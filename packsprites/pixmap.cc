#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <algorithm>

#include <png.h>

#include "pixmap.h"
#include "panic.h"

pixmap::pixmap(const char *png_path)
{
	FILE *in;

	if ((in = fopen(png_path, "rb")) == 0)
		panic("failed to open `%s': %s", png_path, strerror(errno));

	png_structp png_ptr;
	if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) == 0)
		panic("png_create_read_struct failed");

	png_infop info_ptr;
	if ((info_ptr = png_create_info_struct(png_ptr)) == 0)
		panic("png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		panic("some kind of png error");

	png_init_io(png_ptr, in);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);

	int color_type = png_get_color_type(png_ptr, info_ptr);
	int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	if (bit_depth != 8)
		panic("invalid bit depth in PNG");

	type_ = INVALID;

	switch (color_type) {
		case PNG_COLOR_TYPE_GRAY:
			type_ = GRAY;
			break;

		case PNG_COLOR_TYPE_GRAY_ALPHA:
			type_ = GRAY_ALPHA;
			break;

		case PNG_COLOR_TYPE_RGB:
			type_ = RGB;
			break;

		case PNG_COLOR_TYPE_RGBA:
			type_ = RGB_ALPHA;
			break;

		default:
			panic("invalid color type in PNG");
	}

	width_ = png_get_image_width(png_ptr, info_ptr);
	height_ = png_get_image_height(png_ptr, info_ptr);
	bits_.resize(width_*height_*get_pixel_size(type_));

	png_bytep *rows = png_get_rows(png_ptr, info_ptr);

	const size_t stride = width_*get_pixel_size(type_);

	for (size_t i = 0; i < height_; i++)
		memcpy(&bits_[i*stride], rows[i], stride);

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	fclose(in);
}

pixmap::pixmap(int width, int height, type pixmap_type)
: width_(width)
, height_(height)
, type_(pixmap_type)
{
	bits_.resize(width*height*get_pixel_size(type_));
}

pixmap::~pixmap()
{ }

#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

void
pixmap::save(const std::string& path) const
{
	FILE *fp;

	if ((fp = fopen(path.c_str(), "wb")) == NULL)
		panic("fopen %s for write failed: %s", strerror(errno));

	png_structp png_ptr;

	if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL)) == NULL)
		panic("png_create_write_struct");

	png_infop info_ptr;

	if ((info_ptr = png_create_info_struct(png_ptr)) == NULL)
		panic("png_create_info_struct");

	if (setjmp(png_jmpbuf(png_ptr)))
		panic("png error");

	png_init_io(png_ptr, fp);

	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	int color_type;

	switch (type_) {
		case GRAY:
			color_type = PNG_COLOR_TYPE_GRAY;
			break;

		case GRAY_ALPHA:
			color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
			break;

		case RGB:
			color_type = PNG_COLOR_TYPE_RGB;
			break;

		case RGB_ALPHA:
		default:
			color_type = PNG_COLOR_TYPE_RGBA;
			break;
	}

	png_set_IHDR(
		png_ptr,
		info_ptr,
		width_, height_,
		8,
		color_type,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);

	const size_t stride = width_*get_pixel_size();

	for (size_t i = 0; i < height_; i++)
		png_write_row(png_ptr, const_cast<png_bytep>(&bits_[i*stride]));

	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
}

size_t
pixmap::get_pixel_size() const
{
	return get_pixel_size(type_);
}

size_t
pixmap::get_pixel_size(type pixmap_type)
{
	switch (pixmap_type) {
		case GRAY:
			return 1;

		case GRAY_ALPHA:
			return 2;

		case RGB:
			return 3;

		case RGB_ALPHA:
			return 4;

		default:
			assert(0);
			return 0; // XXX
	}
}
