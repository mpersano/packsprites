#include "sprite_base.h"

sprite_base::sprite_base(const image<rgba<int>>& image)
: image_ { image }
{ }

sprite_base::~sprite_base() = default;
