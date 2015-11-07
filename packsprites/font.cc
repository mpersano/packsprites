#include <algorithm>
#include <cmath>
#include <array>

#include <tinyxml.h>

#include "rgba.h"
#include "image.h"
#include "panic.h"
#include "font.h"

namespace {

template <typename T>
image<T>
dilate(const image<T>& im, int radius)
{
	float kernel[2*radius + 1][2*radius + 1];

	for (int i = 0; i < 2*radius + 1; i++) {
		for (int j = 0; j < 2*radius + 1; j++) {
			const int dr = i - radius;
			const int dc = j - radius;

			const float l = sqrtf(dr*dr + dc*dc);

			if (l <= radius)
				kernel[i][j] = 1.;
			else if (l < radius + 1.)
				kernel[i][j] = 1. - (l - radius);
			else
				kernel[i][j] = 0.;
		}
	}

	image<T> rv(im.width, im.height);

	auto dest = std::begin(rv.pixels);

	for (int i = 0; i < im.height; i++) {
		for (int j = 0; j < im.width; j++) {
			float v = 0;

			for (int dr = -radius; dr <= radius; dr++) {
				for (int dc = -radius; dc <= radius; dc++) {
					int r = i + dr;
					int c = j + dc;

					if (r >= 0 && r < im.height && c >= 0 && c < im.width) {
						const float w = kernel[dr + radius][dc + radius];
						v = std::max(v, w*im.pixels[r*im.width + c]);
					}
				}
			}

			*dest++ = v;
		}
	}

	return rv;
}

} // (anonymous namespace)

glyph::glyph(wchar_t code, int left, int top, int advance_x, const image<rgba<int>>& im)
: sprite_base(im)
, code_(code)
, left_(left)
, top_(top)
, advance_x_(advance_x)
{ }

void
glyph::serialize(TiXmlElement *el) const
{
	el->SetAttribute("code", code_);
	el->SetAttribute("left", left_);
	el->SetAttribute("top", top_);
	el->SetAttribute("advancex", advance_x_);
}

class ft_library
{
public:
	static FT_Library get_instance();

private:
	ft_library();
	~ft_library();

	FT_Library library_;
};

ft_library::ft_library()
{
	if (FT_Init_FreeType(&library_) != 0)
		panic("FT_Init_FreeType");
}

ft_library::~ft_library()
{
	FT_Done_FreeType(library_);
}

FT_Library
ft_library::get_instance()
{
	static ft_library instance;
	return instance.library_;
}

font::font(const char *path)
: outline_radius_ { 2 }
, inner_color_fn_ { [](float) { return rgba<int> { 255, 255, 255, 255 }; } }
, outer_color_fn_ { [](float) { return rgba<int> { 0, 0, 0, 255 }; } }
, shadow_dx_ { 0 }
, shadow_dy_ { 0 }
, shadow_opacity_ { .2 }
, shadow_blur_radius_ { 0 }
{
	if (FT_New_Face(ft_library::get_instance(), path, 0, &face_) != 0)
		panic("FT_New_Face");
}

font::~font()
{
	FT_Done_Face(face_);
}

void
font::set_char_size(int size)
{
	if (FT_Set_Char_Size(face_, size << 6, 0, 100, 0) != 0)
		panic("FT_Set_Char_Size");
}

void
font::set_outline_radius(int v)
{
	outline_radius_ = v;
}

void
font::set_inner_color_fn(const color_fn& fn)
{
	inner_color_fn_ = fn;
}

void
font::set_outer_color_fn(const color_fn& fn)
{
	outer_color_fn_ = fn;
}

void
font::set_shadow_offset(int dx, int dy)
{
	shadow_dx_ = dx;
	shadow_dy_ = dy;
}

void
font::set_shadow_opacity(float v)
{
	shadow_opacity_ = v;
}

void
font::set_shadow_blur_radius(int v)
{
	shadow_blur_radius_ = v;
}

std::unique_ptr<sprite_base>
font::render_glyph(wchar_t code)
{
	if ((FT_Load_Char(face_, code, FT_LOAD_RENDER)) != 0)
		panic("FT_Load_Char");

	FT_GlyphSlot slot = face_->glyph;

	FT_Bitmap *bitmap = &slot->bitmap;

	const int src_height = bitmap->rows;
	const int src_width = bitmap->pitch;

	// figure out final size

	int dest_height = src_height + 2*outline_radius_;
	int dest_width = src_width + 2*outline_radius_;

	int offset_x = outline_radius_;
	int offset_y = outline_radius_;

	if (shadow_dx_ < 0) {
		dest_width += -shadow_dx_ + shadow_blur_radius_;
		offset_x += -shadow_dx_ + shadow_blur_radius_;
	} else if (shadow_dx_ > 0) {
		dest_width += shadow_dx_ + shadow_blur_radius_;
	}

	if (shadow_dy_ < 0) {
		dest_height += -shadow_dy_ + shadow_blur_radius_;
		offset_y += -shadow_dy_ + shadow_blur_radius_;
	} else if (shadow_dy_ > 0) {
		dest_height += shadow_dy_ + shadow_blur_radius_;
	}

	// copy grayscale channel

	image<float> orig(src_width, src_height, bitmap->buffer);
	orig *= 1.f/255;

	image<float> lum(dest_width, dest_height);
	lum.copy(orig, offset_y, offset_x);

	// create outline with dilation morphological filter

	image<float> outline = dilate(lum, outline_radius_);

	// add some happy colors

	image<rgba<float>> color_glyph(dest_width, dest_height);

	for (int i = 0; i < dest_height; i++) {
		const float t = static_cast<float>(i)/dest_height; // XXX: should sub outline radius for inner color

		const float f = 1.f/255;
		auto c0 = rgba<float>(inner_color_fn_(t))*f;
		auto c1 = rgba<float>(outer_color_fn_(t))*f;

		for (int j = 0; j < dest_width; j++) {
			auto l = lum(i, j);

			auto c = c0*l + c1*(1 - l);
			c.a *= outline(i, j);

			color_glyph(i, j) = c;
		}
	}

	// drop shadow

	if (shadow_dx_ || shadow_dy_ || shadow_blur_radius_) {
		image<float> alpha(dest_width, dest_height);
		std::transform(
			std::begin(color_glyph.pixels),
			std::end(color_glyph.pixels),
			std::begin(alpha.pixels),
			[](const rgba<float>& c) { return c.a; });


		image<float> shadow(dest_width, dest_height);
		shadow.copy(alpha, shadow_dy_, shadow_dx_);
		shadow *= shadow_opacity_;

		shadow.gaussian_blur(shadow_blur_radius_);

		for (size_t i = 0u; i < dest_width*dest_height; i++) {
			auto s = shadow.pixels[i];

			auto& c = color_glyph.pixels[i];
			auto a = c.a;

			if (auto da = s*(1.f - a)) {
				// https://en.wikipedia.org/wiki/Alpha_compositing

				auto t = a/(a + da);

				c.r *= t;
				c.g *= t;
				c.b *= t;

				c.a += da;
			}
		}
	}

	const FT_Glyph_Metrics *metrics = &slot->metrics;
	const int left = metrics->horiBearingX >> 6;
	const int top = metrics->horiBearingY >> 6;
	const int advance_x = metrics->horiAdvance >> 6;

	return std::unique_ptr<sprite_base> { new glyph { code, left, top, advance_x, color_glyph*255.f } };
}
