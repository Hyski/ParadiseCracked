#if !defined(__VECTOR2_INCLUDED__)
#define __VECTOR2_INCLUDED__

/////////////////////////////////////////////////////////////////////////
//	MiST land Lybrary files
#include <mll/algebra/numerical_traits.h>
#include <mll/algebra/_typedefs.h>
#include <mll/io/binstream.h>
#include <mll/io/iutils.h>
/////////////////////////////////////////////////////////////////////////

#include <ostream>

namespace mll
{

namespace algebra
{

/////////////////////////////////////////////////////////////////////////
////////////////////////    class base_vector2    ////////////////////////
/////////////////////////////////////////////////////////////////////////
template<class T>
class base_vector2
{
public:

	typedef typename numerical_traits<T>::quotient_type quotient_type;

	union
	{
		struct
		{
			T x,y;
		};
		T v[2];
	};

	base_vector2() {}
	base_vector2(const base_vector2& v) : x(v.x),y(v.y) {}
	base_vector2(const T * _v) : x(_v[0]), y(_v[1]) {}
	base_vector2(T _x,T _y) : x(_x),y(_y) {}

	/////////////////////////////////////////////////////////////////////////
	//	конструирование нулевого вектора
	base_vector2(base_null_vector) : x(numerical_traits<T>::get_null()),
									 y(numerical_traits<T>::get_null()) {}
	/////////////////////////////////////////////////////////////////////////

	const base_vector2& operator=(const base_vector2& v)
	{
		x = v.x;
		y = v.y;

		return *this;
	}

	base_vector2 operator+(void) const
	{
		return *this;
	}

	base_vector2 operator-(void) const
	{
		return base_vector2(-x,-y);
	}

	const base_vector2& operator+=(const base_vector2& v)
	{
		x += v.x;
		y += v.y;

		return *this;
	}

	const base_vector2& operator-=(const base_vector2& v)
	{
		x -= v.x;
		y -= v.y;

		return *this;
	}

	const base_vector2& operator*=(const T& v)
	{
		x *= v;
		y *= v;

		return *this;
	}

	const base_vector2& operator/=(const T& v)
	{
		const quotient_type iv = numerical_traits<T>::inverse(numerical_traits<T>::to_quotient_type(v));

		x *= iv;
		y *= iv;

		return *this;
	}

	bool operator==(const base_vector2& v) const 
	{
		return x==v.x && y==v.y;
	}

	T operator[](int index) const
	{
		return v[index];
	}

	T sqr_length(void) const
	{
		return x*x+y*y;
	}

	quotient_type length(void) const 
	{
		return numerical_traits<T>::sqrt(sqr_length());
	}

	base_vector2 normalize(void)
	{
		if(!numerical_traits<T>::is_null(sqr_length()))
		{
			const quotient_type iv = numerical_traits<T>::inverse(length());
			x *= iv;
			y *= iv;
		}

		return *this;
	}

	T dot(const base_vector2& v) const
	{
		return x*v.x + y*v.y;
	}
};

template<class T>
inline base_vector2<T> operator+(const base_vector2<T>& u,const base_vector2<T>& v)
{
	return base_vector2<T>(u.x+v.x,u.y+v.y);
}

template<class T>
inline base_vector2<T> operator-(const base_vector2<T>& u,const base_vector2<T>& v)
{
	return base_vector2<T>(u.x-v.x,u.y-v.y);
}

template<class T>
inline base_vector2<T> operator*(const T& f, const base_vector2<T>& v)
{
	return base_vector2<T>(v.x*f,v.y*f);
}

template<class T>
inline base_vector2<T> operator*(const base_vector2<T>& v, const T& f)
{
	return base_vector2<T>(v.x*f,v.y*f);
}

template<class T>
inline base_vector2<T> operator/(const base_vector2<T>& v, const T& f)
{
	return base_vector2<T>(v.x/f,v.y/f);
}

template<class T>
inline bool operator!=(const base_vector2<T>& u,const base_vector2<T>& v)
{
	return (u.x!=v.x) || (u.y!=v.y);
}

typedef base_vector2<float> vector2;
typedef base_vector2<float> vector2f;
typedef base_vector2<int> vector2i;

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// operator<< () /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline ::std::ostream &operator<< (::std::ostream &stream, const base_vector2<T> &pt)
{
	stream << "vector2(" << pt.x << "," << pt.y << ")";
	return stream;
}

/*=====================================================================================*\
 *                                    operator>> ()                                    * 
\*=====================================================================================*/
template<typename T>
inline ::std::istream &operator>> (::std::istream &stream, base_vector2<T> &pt)
{
	stream >> std::ws >> io::verify("vector2") >> std::ws >> io::verify("(");
	stream >> std::ws >> pt.x >> std::ws >> io::verify(",");
	stream >> std::ws >> pt.y >> std::ws >> io::verify(")");
	return stream;
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
template<typename T>
inline io::ibinstream &operator>>(io::ibinstream &stream, base_vector2<T> &pt)
{
	return io::do_raw_input(stream,pt);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
template<typename T>
inline io::obinstream &operator<<(io::obinstream &stream, const base_vector2<T> &pt)
{
	return io::do_raw_output(stream,pt);
}


}	//	namesapce algebra

}	//	namesapce mll


#endif