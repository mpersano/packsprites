#pragma once

#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "rgba.h"
#include "sprite_base.h"

struct glyph : sprite_base
{
	glyph(wchar_t code, int left, int top, int advance_x, const image<rgba<int>>& im);

	void serialize(TiXmlElement *el) const override;

	wchar_t code_;
	int left_, top_, advance_x_;
};

class color_fn
{
public:
	virtual ~color_fn() = default;

	virtual rgba<int> operator()(float t) const = 0;
};

class font
{
public:
	font(const char *path);
	virtual ~font();

	void set_char_size(int size);
	void set_outline_radius(int v);
	void set_shadow_offset(int dx, int dy);
	void set_shadow_opacity(float v);
	void set_shadow_blur_radius(int v);

	std::unique_ptr<sprite_base> render_glyph(
			const wchar_t code,
			const color_fn& inner_color,
			const color_fn& outline_color);

private:
	FT_Face face_;
	int outline_radius_;
	int shadow_dx_, shadow_dy_;
	float shadow_opacity_;
	int shadow_blur_radius_;
};
