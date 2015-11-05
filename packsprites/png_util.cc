#include <cstring>

#include <png.h>

#include "panic.h"
#include "png_util.h"

#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

void
png_write(const rgba_image&im, const std::string& path)
{
	struct file {
		file(const std::string& path)
		: s { fopen(path.c_str(), "wb") }
		{
			if (!s)
				panic("fopen %s for write failed: %s", strerror(errno));
		}

		~file()
		{ fclose(s); }

		FILE *s;
	} f { path };

	png_structp png_ptr;

	if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL)) == NULL)
		panic("png_create_write_struct");

	png_infop info_ptr;

	if ((info_ptr = png_create_info_struct(png_ptr)) == NULL)
		panic("png_create_info_struct");

	if (setjmp(png_jmpbuf(png_ptr)))
		panic("png error");

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
