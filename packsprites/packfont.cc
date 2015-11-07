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

color_fn
parse_color_fn(const char *str)
{
	auto c0 = parse_color(str);

	if (const char *p = strchr(str, '-')) {
		auto c1 = parse_color(p + 1);

		if (const char *q = strchr(p + 1, '-')) {
			auto c2 = parse_color(q + 1);

			// quadratic bezier gradient
			return [=](float u)
				{
					const float w0 = (1 - u)*(1 - u);
					const float w1 = 2*u*(1 - u);
					const float w2 = u*u;

					return c0*w0 + c1*w1 + c2*w2;
				};
		} else {
			// lame linear gradient
			return [=](float u) { return c0*(1.f - u) + c1*u; };
		}
	} else {
		// flat color
		return [=](float) { return c0; };
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
	color_fn inner_color_fn { [](float) { return rgba<int> { 255, 255, 255, 255 }; } };
	color_fn outer_color_fn { [](float) { return rgba<int> { 0, 0, 0, 255 }; } };
	int shadow_dx = 0;
	int shadow_dy = 0;
	float shadow_opacity = .2;
	int shadow_blur_radius = 0;
	std::string texture_path_base = ".";

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
				inner_color_fn = parse_color_fn(optarg);
				break;

			case 'o':
				outer_color_fn = parse_color_fn(optarg);
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

	std::vector<std::unique_ptr<sprite_base>> sprites;

	font f(font_name);

	f.set_outline_radius(outline_radius);

	f.set_char_size(font_size);

	f.set_inner_color_fn(inner_color_fn);
	f.set_outer_color_fn(outer_color_fn);

	f.set_shadow_offset(shadow_dx, shadow_dy);
	f.set_shadow_opacity(shadow_opacity);
	f.set_shadow_blur_radius(shadow_blur_radius);

	for (int i = optind + 2; i < argc; i++) {
		char *range = argv[i];

		int from = parse_int(range), to;

		if (char *dash = strchr(range, '-')) {
			*dash = '\0';
			to = parse_int(dash + 1);
		} else {
			to = from;
		}

		for (int j = from; j <= to; j++)
			sprites.push_back(f.render_glyph(j));
	}

	pack_sprites(sprites,
			sheet_name,
			sheet_width, sheet_height,
			border,
			texture_path_base.c_str());
}
