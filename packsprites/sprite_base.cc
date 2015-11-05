#include "sprite_base.h"

sprite_base::sprite_base(const image<rgba<int>>& im)
: im_ { im }
{ }

sprite_base::~sprite_base() = default;
