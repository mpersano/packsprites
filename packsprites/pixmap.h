#pragma once

#include <cstdint>
#include <string>
#include <vector>

class pixmap
{
public:
	enum type { GRAY, GRAY_ALPHA, RGB, RGB_ALPHA, INVALID };

	pixmap(int width, int height, type pixmap_type);
	pixmap(const char *png_path);

	virtual ~pixmap();

	size_t get_width() const
	{ return width_; }

	size_t get_height() const
	{ return height_; }

	const uint8_t *get_bits() const
	{ return &bits_[0]; }

	uint8_t *get_bits()
	{ return &bits_[0]; }

	type get_type() const
	{ return type_; }

	size_t get_pixel_size() const;

	static size_t get_pixel_size(type pixmap_type);

	void save(const std::string& path) const;

private:
	size_t width_;
	size_t height_;
	std::vector<uint8_t> bits_;
	type type_;

	pixmap(const pixmap&); // disable copy ctor
	pixmap& operator=(const pixmap&); // disable assignment
};
