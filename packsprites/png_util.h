#pragma once

#include <string>
#include <memory>

#include "rgba.h"
#include "image.h"

using rgba_image = image<uint32_t>;
using rgba_image_ptr = std::unique_ptr<rgba_image>;

void
png_write(const rgba_image& im, const std::string& path);

rgba_image_ptr
png_read(const std::string& path);
