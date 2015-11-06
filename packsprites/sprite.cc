#include <cstring>

#include <tinyxml.h>

#include "sprite.h"

sprite::sprite(const std::string& name, const image<rgba<int>>& im)
: sprite_base(im)
, name_(name)
{ }

void
sprite::serialize(TiXmlElement *el) const
{
	el->SetAttribute("name", name_);
}
