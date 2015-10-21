#pragma once

#include <cstddef>
#include <memory>

#include "pixmap.h"

class rect;
class TiXmlElement;

struct sprite_base
{
	sprite_base(pixmap *pm);
	virtual ~sprite_base();

	size_t width() const
	{ return pm_->get_width(); }

	size_t height() const
	{ return pm_->get_height(); }

	virtual void serialize(TiXmlElement *el) const = 0;

	std::unique_ptr<pixmap> pm_;
};
