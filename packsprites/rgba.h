#pragma once

template <typename T>
struct rgba
{
	rgba()
	: r { 0 }, g { 0 }, b { 0 }, a { 0 }
	{ }

	rgba(T r, T g, T b, T a)
	: r { r }, g { g }, b { b }, a { a }
	{ }

	rgba(uint32_t v)
	: r { v & 0xff }
	, g { (v >> 8) & 0xff }
	, b { (v >> 16) & 0xff }
	, a { v >> 24 }
	{ }

	rgba<T>&
	operator+=(const rgba<T>& o)
	{
		r += o.r;
		g += o.g;
		b += o.b;
		a += o.a;

		return o;
	}

	const rgba<T>
	operator+(const rgba<T>& o) const
	{
		return rgba<T> { r + o.r, g + o.g, b + o.b, a + o.a };
	}

	rgba<T>&
	operator-=(const rgba<T>& o)
	{
		r -= o.r;
		g -= o.g;
		b -= o.b;
		a -= o.a;

		return o;
	}

	const rgba<T>
	operator-(const rgba<T>& o) const
	{
		return rgba<T> { r - o.r, g - o.g, b - o.b, a - o.a };
	}

	template <typename S>
	const rgba<T>
	operator*(S s) const
	{
		return rgba<T> { static_cast<T>(r*s), static_cast<T>(g*s), static_cast<T>(b*s), static_cast<T>(a*s) };
	}

	operator int32_t() const
	{
		return
			static_cast<int32_t>(r) +
			(static_cast<int32_t>(g) << 8) +
			(static_cast<int32_t>(b) << 16) +
			(static_cast<int32_t>(a) << 24);
	}

	T r, g, b, a;
};

template <typename S, typename T>
const rgba<T>
operator*(S s, const rgba<T>& c)
{
	return rgba<T> { static_cast<T>(c.r*s), static_cast<T>(c.g*s), static_cast<T>(c.b*s), static_cast<T>(c.a*s) };
}
