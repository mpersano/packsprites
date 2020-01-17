#pragma once

#include <cmath>
#include <algorithm>
#include <vector>
#include <numeric>

template <typename T>
struct image
{
	image(size_t width, size_t height)
	: width { width }
	, height { height }
	, pixels(width*height)
	{ }

	template <typename InputIt>
	image(size_t width, size_t height, InputIt first)
	: width { width }
	, height { height }
	, pixels(width*height)
	{
		std::copy(first, first + width*height, std::begin(pixels));
	}

	template <typename U>
	image(const image<U>& rv)
	: width { rv.width }
	, height { rv.height }
	, pixels(width*height)
	{
		std::copy(std::begin(rv.pixels), std::end(rv.pixels), std::begin(pixels));
	}

	const T& operator()(int r, int c) const
	{
		return pixels[r*width + c];
	}

	T& operator()(int r, int c)
	{
		return pixels[r*width + c];
	}

	template <typename U>
	image<T>& operator*=(U rv)
	{
		std::transform(
			std::begin(pixels),
			std::end(pixels),
			std::begin(pixels),
			[=](const T& v) { return v*rv; });
		return *this;
	}

	template <typename U>
	friend inline
	image<T> operator*(const image<T>& lv, U rv)
	{ return image<T>(lv) *= rv; }

	template <typename U>
	friend inline
	image<T> operator*(U lv, const image<T>& rv)
	{ return image<T>(rv) *= lv; }

	void
	copy(const image<T>& other, int dr, int dc)
	{
		int r0 = std::max(dr, 0);
		int r1 = std::min(dr + other.height, height);

		int c0 = std::max(dc, 0);
		int c1 = std::min(dc + other.width, width);

		for (int r = r0; r < r1; r++)
			std::copy(&other(r - dr, c0 - dc), &other(r - dr, c1 - dc), &(*this)(r, c0));
	}

	void
	gaussian_blur(int radius)
	{
		// generate kernel

		std::vector<float> kernel(2*radius + 1);

		for (int i = 0; i < kernel.size(); i++) {
			float f = i - radius;
			kernel[i] = expf(-f*f/30.);
		}

		float s = std::accumulate(std::begin(kernel), std::end(kernel), 0.f);

		std::transform(
			std::begin(kernel),
			std::end(kernel),
			std::begin(kernel),
			[=](float v) { return v/s; });

		// blur horizontally to temp

		std::vector<T> temp(width*height);

		for (int i = 0; i < height; i++) {
			T *src = &pixels[i*width];
			T *dest = &temp[i*width];

			for (int j = 0; j < width; j++) {
				T s = 0;

				for (int k = 0; k < kernel.size(); k++) {
					if (j + k - radius < 0 || j + k - radius >= width)
						continue;
					s += kernel[k]*src[k - radius];
				}

				*dest++ = s;
				++src;
			}
		}

		// blur vertically from temp

		for (int i = 0; i < height; i++) {
			T *src = &temp[i*width];
			T *dest = &pixels[i*width];

			for (int j = 0; j < width; j++) {
				T s = 0;

				for (int k = 0; k < kernel.size(); k++) {
					if (i + k - radius < 0 || i + k - radius >= height)
						continue;
					s += kernel[k]*src[(k - radius)*width];
				}

				*dest++ = s;
				++src;
			}
		}
	}

	size_t width;
	size_t height;
	std::vector<T> pixels;
};
