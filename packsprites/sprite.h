#pragma once

#include <string>

#include "sprite_base.h"

struct sprite : sprite_base
{
	sprite(const std::string& name, const image<rgba<int>>& im);

	void serialize(TiXmlElement *el) const override;

	std::string name_;
};
