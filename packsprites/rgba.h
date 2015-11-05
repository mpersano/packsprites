#pragma once

template <typename T>
struct rgba
{
	rgba()
	: r(0), g(0), b(0), a(0)
	{ }

	template <typename R, typename G, typename B, typename A>
	rgba(R r, G g, B b, A a)
	: r(r), g(g), b(b), a(a)
	{ }

	template <typename U>
	rgba(const rgba<U>& rv)
	: r(rv.r), g(rv.g), b(rv.b), a(rv.a)
	{ }

	template <typename U>
	rgba<T>& operator*=(U rv)
	{
		r *= rv;
		g *= rv;
		b *= rv;
		a *= rv;
		return *this;
	}

	template <typename U>
	rgba<T>& operator+=(const rgba<U>& rv)
	{
		r += rv.r;
		g += rv.g;
		b += rv.b;
		a += rv.a;
		return *this;
	}

	template <typename U>
	rgba<T>& operator-=(const rgba<U>& rv)
	{
		r -= rv.r;
		g -= rv.g;
		b -= rv.b;
		a -= rv.a;
		return *this;
	}

	template <typename U>
	friend inline
	rgba<T> operator*(const rgba<T>& lv, U rv)
	{ return rgba<T>(lv) *= rv; }

	template <typename U>
	friend inline
	rgba<T> operator*(U lv, const rgba<T>& rv)
	{ return rgba<T>(rv) *= lv; }

	template <typename U>
	friend inline
	rgba<T> operator+(const rgba<T>& lv, const rgba<U>& rv)
	{ return rgba<T>(lv) += rv; }

	template <typename U>
	friend inline
	rgba<T> operator-(const rgba<T>& lv, const rgba<U>& rv)
	{ return rgba<T>(lv) -= rv; }

	T r, g, b, a;
};

using rgbaf = rgba<float>;
using rgbai = rgba<int>;
