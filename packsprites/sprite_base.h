#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "image.h"

class TiXmlElement;

struct sprite_base
{
	sprite_base(std::unique_ptr<image<uint32_t>> im);
	virtual ~sprite_base();

	size_t width() const
	{ return image_->width; }

	size_t height() const
	{ return image_->height; }

	virtual void serialize(TiXmlElement *el) const = 0;

	std::unique_ptr<image<uint32_t>> image_;
};
