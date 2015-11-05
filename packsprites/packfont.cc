#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cassert>

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

#include <vector>
#include <string>
#include <memory>
#include <algorithm>

#include "font.h"
#include "sprite_packer.h"
#include "panic.h"

namespace {

class flat_color_fn : public color_fn
{
public:
	flat_color_fn(const rgba<int>& color)
	: color_(color)
	{ }

	rgba<int> operator()(float t) const override
	{ return color_; }

private:
	rgba<int> color_;
};

// lame linear gradient

class gradient_color_fn : public color_fn
{
public:
	gradient_color_fn(const rgba<int>& from, const rgba<int>& to)
	: from_(from), to_(to)
	{ }

	rgba<int> operator()(float t) const override
	{ return from_ + (to_ - from_)*t; }

private:
	rgba<int> from_, to_;
};

// quadratic bezier gradient

class bezier_color_fn : public color_fn
{
public:
	bezier_color_fn(const rgba<int>& c0, const rgba<int>& c1, const rgba<int>& c2)
	: c0_(c0), c1_(c1), c2_(c2)
	{ }

	rgba<int> operator()(float u) const override
	{
		const float w0 = (1 - u)*(1 - u);
		const float w1 = 2*u*(1 - u);
		const float w2 = u*u;

		return c0_*w0 + c1_*w1 + c2_*w2;
	}

private:
	rgba<int> c0_, c1_, c2_;
};

void
usage(const char *argv0)
{
	fprintf(stderr,
		"usage: packfont [options] font sheetname range...\n"
		"\n"
		"options:\n"
		"-b	size in pixels of border around the packed sprites (default: 2)\n"
		"-w	spritesheet width (default: 256)\n"
		"-h	spritesheet height (default: 256)\n"
		"-s	font size (default: 16)\n"
		"-g	outline radius, in pixels (default: 2)\n"
		"-i	font color\n"
		"-o	outline color\n"
		"-S	drop shadow opacity, between 0 and 1 (default: .2)\n"
		"-B	drop shadow gaussian blur radius, in pixels (default: 0)\n"
		"-d	drop shadow x offset, in pixels (default: 0)\n"
		"-e	drop shadow y offset, in pixels (default: 0)\n");
	exit(EXIT_FAILURE);
}

int
parse_int(const char *str)
{
	return *str == 'x' || *str == 'X' ? strtol(str + 1, 0, 16) : strtol(str, 0, 10);
}

rgba<int>
parse_color(const char *str)
{
	rgba<int> c;
	sscanf(str, "%02x%02x%02x%02x", &c.a, &c.r, &c.g, &c.b);
	return c;
}

std::unique_ptr<color_fn>
parse_color_fn(char *str)
{
	if (char *p = strchr(str, '-')) {
		*p = '\0';

		if (char *q = strchr(p + 1, '-')) {
			*q = '\0';

			return std::unique_ptr<color_fn>
				{
					new bezier_color_fn {
						parse_color(str),
						parse_color(p + 1),
						parse_color(q + 1) }
				};
		} else {
			return std::unique_ptr<color_fn>
				{
					new gradient_color_fn {
						parse_color(str),
						parse_color(p + 1) }
				};
		}
	} else {
		return std::unique_ptr<color_fn> { new flat_color_fn { parse_color(str) } };
	}
}

} // (anonymous namespace)

int
main(int argc, char *argv[])
{
	int border = 2;
	int font_size = 16;
	int sheet_width = 256;
	int sheet_height = 256;
	int outline_radius = 2;
	int shadow_dx = 0;
	int shadow_dy = 0;
	float shadow_opacity = .2;
	int shadow_blur_radius = 0;
	std::string texture_path_base = ".";

	std::unique_ptr<color_fn> inner_color { new flat_color_fn { rgba<int> { 255, 255, 255, 255 } } };
	std::unique_ptr<color_fn> outline_color { new flat_color_fn { rgba<int> { 0, 0, 0, 255 } } };

	int c;

	while ((c = getopt(argc, argv, "b:s:w:h:g:t:i:o:S:d:e:B:")) != EOF) {
		switch (c) {
			case 'b':
				border = atoi(optarg);
				break;

			case 's':
				font_size = atoi(optarg);
				break;

			case 'w':
				sheet_width = atoi(optarg);
				break;

			case 'h':
				sheet_height = atoi(optarg);
				break;

			case 'g':
				outline_radius = atoi(optarg);
				break;

			case 't':
				texture_path_base = optarg;
				break;

			case 'i':
				inner_color = std::move(parse_color_fn(optarg));
				break;

			case 'o':
				outline_color = std::move(parse_color_fn(optarg));
				break;

			case 'S':
				shadow_opacity = atof(optarg);
				break;

			case 'B':
				shadow_blur_radius = atoi(optarg);
				break;

			case 'd':
				shadow_dx = atoi(optarg);
				break;

			case 'e':
				shadow_dy = atoi(optarg);
				break;
		}
	}

	if (argc - optind < 3)
		usage(*argv);

	const char *font_name = argv[optind];
	const char *sheet_name = argv[optind + 1];

	std::vector<sprite_base *> sprites;

	font f(font_name);
	f.set_char_size(font_size);

	for (int i = optind + 2; i < argc; i++) {
		char *range = argv[i];

		int from = parse_int(range), to;

		if (char *dash = strchr(range, '-')) {
			*dash = '\0';
			to = parse_int(dash + 1);
		} else {
			to = from;
		}

		for (int j = from; j <= to; j++) {
			sprites.push_back(
				f.render_glyph(
					j,
					outline_radius,
					*inner_color.get(),
					*outline_color.get(),
					shadow_dx,
					shadow_dy,
					shadow_opacity,
					shadow_blur_radius));
		}
	}

	pack_sprites(sprites,
			sheet_name,
			sheet_width, sheet_height,
			border,
			pixmap::RGB_ALPHA,
			texture_path_base.c_str());
}
