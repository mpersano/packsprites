#include <cstdio>

#include <vector>

template <typename PixelType>
struct image
{
	image(int width, int height)
	: width_ { width }
	, height_ { height }
	, data_(width_*height_)
	{ }

	std::vector<PixelType> pixels_;
};

int
main()
{
}
