#pragma once

#include <cstddef>
#include <memory>

#include "rgba.h"
#include "image.h"

class TiXmlElement;

struct sprite_base
{
	sprite_base(const image<rgba<int>>& im);
	virtual ~sprite_base();

	size_t width() const
	{ return image_.width; }

	size_t height() const
	{ return image_.height; }

	virtual void serialize(TiXmlElement *el) const = 0;

	image<rgba<int>> image_;
};
