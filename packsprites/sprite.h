#pragma once

#include <string>

#include "sprite_base.h"

struct sprite : sprite_base
{
	sprite(const std::string& name, std::unique_ptr<image<uint32_t>> im);

	void serialize(TiXmlElement *el) const override;

	std::string name_;
};
