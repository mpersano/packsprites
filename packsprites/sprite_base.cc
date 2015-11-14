#include "sprite_base.h"

sprite_base::sprite_base(std::unique_ptr<image<uint32_t>> image)
: image_ { std::move(image) }
{ }

sprite_base::~sprite_base() = default;
