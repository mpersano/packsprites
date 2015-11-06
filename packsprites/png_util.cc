#include <cstring>

#include <png.h>

#include "panic.h"
#include "png_util.h"

namespace {

struct file {
	file(const std::string& path, const std::string& mode)
	: s { fopen(path.c_str(), mode.c_str()) }
	{
		if (!s)
			panic("fopen %s for write failed: %s", strerror(errno));
	}

	~file()
	{ fclose(s); }

	FILE *s;
};

}

#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

void
png_write(const rgba_image&im, const std::string& path)
{
	png_structp png_ptr;

	if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL)) == NULL)
		panic("png_create_write_struct");

	png_infop info_ptr;

	if ((info_ptr = png_create_info_struct(png_ptr)) == NULL)
		panic("png_create_info_struct");

	if (setjmp(png_jmpbuf(png_ptr)))
		panic("png error");

	file f { path, "wb" };

	png_init_io(png_ptr, f.s);

	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	png_set_IHDR(
		png_ptr,
		info_ptr,
		im.width, im.height,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);

	std::vector<png_byte> row(4*im.width);

	for (size_t i = 0; i < im.height; i++) {
		auto it = std::begin(row);

		for (size_t j = 0; j < im.width; j++) {
			const auto& c = im(i, j);

			*it++ = c.r;
			*it++ = c.g;
			*it++ = c.b;
			*it++ = c.a;
		}

		png_write_row(png_ptr, &row[0]);
	}

	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);
}

rgba_image
png_read(const std::string& path)
{
	file f { path, "rb" };

	png_structp png_ptr;
	if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) == 0)
		panic("png_create_read_struct failed");

	png_infop info_ptr;
	if ((info_ptr = png_create_info_struct(png_ptr)) == 0)
		panic("png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		panic("some kind of png error");

	png_init_io(png_ptr, f.s);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);

	auto color_type = png_get_color_type(png_ptr, info_ptr);
	auto bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	if (bit_depth != 8 || color_type != PNG_COLOR_TYPE_RGBA)
		panic("invalid bit depth in PNG");

	// TODO conversion for other color types

	auto width = png_get_image_width(png_ptr, info_ptr);
	auto height = png_get_image_height(png_ptr, info_ptr);

	rgba_image im(width, height);

	png_bytep *rows = png_get_rows(png_ptr, info_ptr);

	for (size_t i = 0; i < height; i++) {
		auto src = rows[i];
		auto dest = &im(i, 0);

		for (size_t j = 0; j < width; j++) {
			*dest++ = rgba<int>(src[0], src[1], src[2], src[3]);
			src += 4;
		}
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	return im;
}
