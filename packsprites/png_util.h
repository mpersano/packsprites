#pragma once

#include <string>

#include "rgba.h"
#include "image.h"

using rgba_image = image<rgba<int>>;

void
png_write(const rgba_image& im, const std::string& path);
