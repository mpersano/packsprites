#include <cstring>

#include <tinyxml.h>

#include "sprite.h"

sprite::sprite(const std::string& name, std::unique_ptr<image<uint32_t>> im)
: sprite_base { std::move(im) }
, name_ { name }
{ }

void
sprite::serialize(TiXmlElement *el) const
{
	el->SetAttribute("name", name_);
}
