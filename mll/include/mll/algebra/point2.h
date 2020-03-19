#if !defined(__POINT2_INCLUDED__)
#define __POINT2_INCLUDED__

/////////////////////////////////////////////////////////////////////////
//	MiST land Library files
#include <mll/algebra/_typedefs.h>
#include <mll/algebra/vector2.h>
#include <mll/io/iutils.h>
/////////////////////////////////////////////////////////////////////////

#include <ostream>

namespace mll
{


///набор классов, реализующих некоторые понятия из области алгебры
namespace algebra
{

///алгебраическая 2-х мерная точка.
/**
	этот класс шаблонный. тип координат является параметром шаблона.
	\param T тип координаты.
*/
template<class T>
class base_point2
{
public:

	typedef typename numerical_traits<T>::quotient_type quotient_type;

	/// координата
	union
	{
		struct
		{
			T x,y;
		};
		T v[2];
	};

	base_point2() {}
	base_point2(const base_point2& v) : x(v.x),y(v.y) {}
	base_point2(const T * _v) : x(_v[0]), y(_v[1]) {}
	base_point2(T _x,T _y) : x(_x),y(_y) {}
	explicit base_point2(const base_origin&) : x(numerical_traits<T>::get_null()),
											   y(numerical_traits<T>::get_null()) {}

	///присваивание
	const base_point2& operator=(const base_point2& v)
	{
		x = v.x;
		y = v.y;

		return *this;
	}

	///присваивание
	const base_point2& operator=(const base_origin& v)
	{
		x = y = numerical_traits<T>::get_null();

		return *this;
	}

	///унарный +
	base_point2 operator+(void) const
	{
		return *this;
	}
	///унарный -
	base_point2 operator-(void) const
	{
		return base_point2(-x,-y);
	}

	///прибавление с присваиванием
	const base_point2& operator+=(const base_vector2<T>& v)
	{
		x += v.x;
		y += v.y;

		return *this;
	}

	///вычитание с присваиванием
	const base_point2& operator-=(const base_vector2<T>& v)
	{
		x -= v.x;
		y -= v.y;

		return *this;
	}
	///умножение с присваиванием
	const base_point2& operator*=(const T& v)
	{
		x *= v;
		y *= v;

		return *this;
	}
///деление с присваиванием
	const base_point2& operator/=(const T& v)
	{
		const quotient_type iv = numerical_traits<T>::inverse(numerical_traits<T>::to_quotient_type(v));

		x *= iv;
		y *= iv;

		return *this;
	}

///скалярное произведение
	T dot(const base_vector2<T>& v) const
	{
		return x*v.x + y*v.y;
	}

///линейная интерполяция с коэффициентом factor
	base_point2 lerp(const base_point2 &p,quotient_type factor) const
	{
		return *this + (p-*this)*factor;
	}
};

template<class T>
inline base_vector2<T> operator-(const base_point2<T>& u,const base_point2<T>& v)
{
	return base_vector2<T>(u.x-v.x,u.y-v.y);
}

template<class T>
inline base_point2<T> operator+(const base_point2<T>& u,const base_vector2<T>& v)
{
	return base_point2<T>(u.x+v.x,u.y+v.y);
}

template<class T>
inline base_point2<T> operator-(const base_point2<T>& u,const base_vector2<T>& v)
{
	return base_point2<T>(u.x-v.x,u.y-v.y);
}

template<class T>
inline base_point2<T> operator+(const base_vector2<T>& v,const base_point2<T>& u)
{
	return base_point2<T>(u.x+v.x,u.y+v.y);
}

template<class T>
inline base_point2<T> operator-(const base_vector2<T>& v,const base_point2<T>& u)
{
	return base_point2<T>(u.x-v.x,u.y-v.y);
}

template<class T>
inline base_point2<T> operator+(const base_origin&,const base_vector2<T>& v)
{
	return base_point2<T>(v.x,v.y);
}

template<class T>
inline base_point2<T> operator-(const base_origin&,const base_vector2<T>& v)
{
	return base_point2<T>(-v.x,-v.y);
}

template<class T>
inline base_point2<T> operator+(const base_vector2<T>& v,const base_origin&)
{
	return base_point2<T>(v.x,v.y);
}

template<class T>
inline base_point2<T> operator-(const base_vector2<T>& v,const base_origin&)
{
	return base_point2<T>(v.x,v.y);
}

template<class T>
inline base_vector2<T> operator-(const base_point2<T>& v,const base_origin&)
{
	return base_vector2<T>(v.x,v.y);
}

template<class T>
inline bool operator==(const base_point2<T>& u,const base_point2<T>& v)
{
	return u.x==v.x && u.y==v.y;
}

template<class T>
inline bool operator<(const base_point2<T>& u,const base_point2<T>& v)
{
	return u.x<v.x || (u.x==v.x && u.y<v.y);
}
	

template<class T>
inline bool operator!=(const base_point2<T>& u,const base_point2<T>& v)
{
	return (u.x!=v.x) || (u.y!=v.y);
}

template<class T>
inline base_point2<T> operator*(const base_point2<T> &p, T f)
{
	base_point2<T> result(p);
	result *= f;
	return result;
}

template<class T>
inline base_point2<T> operator/(const base_point2<T> &p, T f)
{
	base_point2<T> result(p);
	result /= f;
	return result;
}

template<class T>
inline base_point2<T> operator*(T f, const base_point2<T> &p)
{
	base_point2<T> result(p);
	result *= f;
	return result;
}

typedef base_point2<float> point2; ///< точка с координатами с плавающей точкой одинарной точности
typedef base_point2<float> point2f;///< точка с координатами с плавающей точкой одинарной точности
typedef base_point2<int> point2i;  ///< точка с целочисленными координатами

///вывод точки в поток stl.
template<typename T>
inline ::std::ostream &operator<< (::std::ostream &stream, const base_point2<T> &pt)
{
	stream << "point2(" << pt.x << "," << pt.y << ")";
	return stream;
}

///чтение точки из потока stl.
template<typename T>
inline ::std::istream &operator>> (::std::istream &stream, base_point2<T> &pt)
{
	stream >> std::ws >> io::verify("point2") >> std::ws >> io::verify("(");
	stream >> std::ws >> pt.x >> std::ws >> io::verify(",");
	stream >> std::ws >> pt.y >> std::ws >> io::verify(")");
	return stream;
}

///чтение точки из бинарного потока.
template<typename T>
inline io::ibinstream &operator>>(io::ibinstream &stream, base_point2<T> &pt)
{
	return io::do_raw_input(stream,pt);
}

///вывод точки в бинарный поток
template<typename T>
inline io::obinstream &operator<<(io::obinstream &stream, const base_point2<T> &pt)
{
	return io::do_raw_output(stream,pt);
}

}	//	namesapce algebra

}	//	namesapce mll


#endif