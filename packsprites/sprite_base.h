#pragma once

#include <cstddef>
#include <memory>

#include "rgba.h"
#include "image.h"

class rect;
class TiXmlElement;

struct sprite_base
{
	sprite_base(const image<rgba<int>>& im);
	virtual ~sprite_base();

	size_t width() const
	{ return im_.width; }

	size_t height() const
	{ return im_.height; }

	virtual void serialize(TiXmlElement *el) const = 0;

	image<rgba<int>> im_;
};
