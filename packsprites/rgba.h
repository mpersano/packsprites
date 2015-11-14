#pragma once

template <template <typename> class Base, typename T>
struct vec_ops
{
	template <typename U>
	Base<T>& operator*=(U rv)
	{
		Base<T>& lv = static_cast<Base<T> &>(*this);
		for (size_t i = 0; i < Base<T>::size; i++)
			lv[i] *= rv;
		return lv;
	}

	template <typename U>
	Base<T>& operator+=(const Base<U>& rv)
	{
		Base<T>& lv = static_cast<Base<T> &>(*this);
		for (size_t i = 0; i < Base<T>::size; i++)
			lv[i] += rv[i];
		return lv;
	}

	template <typename U>
	Base<T>& operator-=(const Base<U>& rv)
	{
		Base<T>& lv = static_cast<Base<T> &>(*this);
		for (size_t i = 0; i < Base<T>::size; i++)
			lv[i] -= rv[i];
		return lv;
	}

	template <typename U>
	friend inline
	Base<T> operator*(const Base<T>& lv, U rv)
	{ return Base<T>(lv) *= rv; }

	template <typename U>
	friend inline
	Base<T> operator*(U lv, const Base<T>& rv)
	{ return Base<T>(rv) *= lv; }

	template <typename U>
	friend inline
	Base<T> operator+(const Base<T>& lv, const Base<U>& rv)
	{ return Base<T>(lv) += rv; }

	template <typename U>
	friend inline
	Base<T> operator-(const Base<T>& lv, const Base<U>& rv)
	{ return Base<T>(lv) -= rv; }
};

template <typename T>
struct rgb : vec_ops<rgb, T>
{
	rgb()
	: r(0), g(0), b(0)
	{ }

	template <typename U>
	rgb(const rgb<U>& rv)
	: r(rv.r), g(rv.g), b(rv.b)
	{ }

	template <typename R, typename G, typename B>
	rgb(R r, G g, B b)
	: r(r), g(g), b(b)
	{ }

	const T& operator[](size_t i) const
	{
		return v[i];
	}

	T& operator[](size_t i)
	{
		return v[i];
	}

	static const size_t size = 3;

	union {
		struct { T r, g, b; };
		T v[size];
	};
};

template <typename T>
struct rgba : vec_ops<rgba, T>
{
	rgba()
	: r(0), g(0), b(0), a(0)
	{ }

	template <typename U>
	rgba(const rgba<U>& rv)
	: r(rv.r), g(rv.g), b(rv.b), a(rv.a)
	{ }

	template <typename R, typename G, typename B, typename A>
	rgba(R r, G g, B b, A a)
	: r(r), g(g), b(b), a(a)
	{ }

	template <typename U, typename A>
	rgba(const rgb<U>& v, A a)
	: r(v.r), g(v.g), b(v.b), a(a)
	{ }

	// unpacking

	rgba(uint32_t v)
	: r { static_cast<T>(v & 0xff) }
	, g { static_cast<T>((v >> 8) & 0xff) }
	, b { static_cast<T>((v >> 16) & 0xff) }
	, a { static_cast<T>(v >> 24) }
	{ }

	// packing

	operator uint32_t() const
	{
		return static_cast<uint32_t>(r) | (static_cast<uint32_t>(g) << 8) | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(a) << 24);
	}

	const T& operator[](size_t i) const
	{
		return v[i];
	}

	T& operator[](size_t i)
	{
		return v[i];
	}

	static const size_t size = 4;

	union {
		struct { T r, g, b, a; };
		T v[size];
	};
};
