#if !defined(__QUATERNION_INCLUDED__)
#define __QUATERNION_INCLUDED__

/////////////////////////////////////////////////////////////////////////
//	MiST land Library files
#include <mll/algebra/matrix3.h>
#include <mll/io/iutils.h>
#include <mll/patterns/type_traits.h>
#include <mll/debug/static_assert.h>
/////////////////////////////////////////////////////////////////////////

namespace mll
{

namespace algebra
{

template<typename T>
class base_axis_angle;

/////////////////////////////////////////////////////////////////////////
//////////////////////    class base_quaternion    //////////////////////
/////////////////////////////////////////////////////////////////////////
template<class T>
class base_quaternion
{
public:
	MLL_STATIC_ASSERT( patterns::is_float<T>::value );

	typedef typename numerical_traits<T>::quotient_type quotient_type;

	T w,x,y,z;

	/////////////////////////////////////////////////////////////////////////
	//	конструктор по умолчанию
	base_quaternion();
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	конструктор по параметрам
	base_quaternion(T w,T x,T y,T z);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	конструкотор копирования
	base_quaternion(const base_quaternion& q);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	конструтор квартениона вращения 
	base_quaternion(const base_vector3<T>& axis,T angle);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	конструтор квартениона из матрицы
	explicit base_quaternion(const base_matrix3<T>& m);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	деструктор
	~base_quaternion();
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	оператор присваивания
	base_quaternion& operator=(const base_quaternion& q);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	арифмитические поэлементтные операции
    base_quaternion& operator+=(const base_quaternion& q);
    base_quaternion& operator-=(const base_quaternion& q);
    base_quaternion& operator*=(T f);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	оператор перемножения кватернионов
    base_quaternion& operator*=(const base_quaternion& q);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	нахождение квадратичной длины кватерниона
    T sqr_length(void) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	нахождение длины (размера) кватерниона
    quotient_type length(void) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	нормализовать кватернион
	base_quaternion &normalize(void);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	преобразовать квартенион в матрицу
	base_matrix3<T> as_matrix(void) const;
	/////////////////////////////////////////////////////////////////////////

	base_quaternion &invert();
	base_axis_angle<T> to_axis_angle() const;

	/////////////////////////////////////////////////////////////////////////
	//  сферическая линейная интерполяция кватернионов
	base_quaternion slerp(const base_quaternion &, T factor) const;
	/////////////////////////////////////////////////////////////////////////

	static base_quaternion<T> identity();
};

//=====================================================================================//
//                                  class axis_angle                                   //
//=====================================================================================//
template<typename T>
class base_axis_angle
{
public:
	MLL_STATIC_ASSERT( patterns::is_float<T>::value );

	base_vector3<T> m_axis;
	T m_angle;

	base_axis_angle() {}
	base_axis_angle(const base_vector3<T> &axis, float angle) : m_axis(axis), m_angle(angle) {}
	base_axis_angle(const base_quaternion<T> &q) { *this = q.to_axis_angle(); }
};

/////////////////////////////////////////////////////////////////////////
//	конструктор по умолчанию
template<class T>
inline base_quaternion<T>::base_quaternion()
{
}
/////////////////////////////////////////////////////////////////////////
//	конструктор по параметрам
template<class T>
inline base_quaternion<T>::base_quaternion(T _w,T _x,T _y,T _z) : w(_w),x(_x),y(_y),z(_z)
{
}
/////////////////////////////////////////////////////////////////////////
//	конструкотор копирования
template<class T>
inline base_quaternion<T>::base_quaternion(const base_quaternion& q) : w(q.w),x(q.x),y(q.y),z(q.z)
{
}
/////////////////////////////////////////////////////////////////////////
//	конструтор квартениона вращения 
template<class T>
inline base_quaternion<T>::base_quaternion(const base_vector3<T>& axis,T angle) : w(numerical_traits<T>::cos(angle/2))
{
	const T sine = numerical_traits<T>::sin(angle/2);

	x = sine*axis.x;
	y = sine*axis.y;
	z = sine*axis.z;
}

/////////////////////////////////////////////////////////////////////////
//	конструтор квартениона из матрицы
template<class T>
inline base_quaternion<T>::base_quaternion(const base_matrix3<T>& m)
{
	quotient_type s;
	T tr;

	tr = m[0][0] + m[1][1] + m[2][2];
	
	if(tr > T(0.0))
	{
		s = numerical_traits<T>::sqrt(tr + numerical_traits<T>::get_one());
		w = T(0.5) * s;
		s = T(0.5) / s;
		x = (m[1][2] - m[2][1]) * s;
		y = (m[2][0] - m[0][2]) * s;
		z = (m[0][1] - m[1][0]) * s;
	}
	else
	{
		const int nxt[3] = {1,2,0};
		int i,j,k;
		T q[4];

		i = 0;
		if (m[1][1] > m[0][0]) i = 1;
		if (m[2][2] > m[i][i]) i = 2;
		j = nxt[i];
		k = nxt[j];
		
		s = numerical_traits<T>::sqrt((m[i][i] - (m[j][j] + m[k][k])) + numerical_traits<T>::get_one());
		
		q[i] = s * T(0.5);
		
		if (s != T(0.0)) s = T(0.5) / s;
		
		q[3] = (m[j][k] - m[k][j]) * s;
		q[j] = (m[i][j] + m[j][i]) * s;
		q[k] = (m[i][k] + m[k][i]) * s;
		
		x = q[0];
		y = q[1];
		z = q[2];
		w = q[3];
	}
}

/////////////////////////////////////////////////////////////////////////
//	деструктор
template<class T>
inline base_quaternion<T>::~base_quaternion()
{
}
/////////////////////////////////////////////////////////////////////////
//	оператор присваивания
template<class T>
inline base_quaternion<T>& base_quaternion<T>::operator=(const base_quaternion<T>& q)
{
	w = q.w;
	x = q.x;
	y = q.y;
	z = q.z;

	return *this;
}

//=====================================================================================//
//                              inline bool operator!=()                               //
//=====================================================================================//
template<typename T>
inline bool operator!=(const base_quaternion<T> &a, const base_quaternion<T> &b)
{
	return a.w != b.w ||
		   a.x != b.x ||
		   a.y != b.y ||
		   a.z != b.z;
}

//=====================================================================================//
//                              inline bool operator==()                               //
//=====================================================================================//
template<typename T>
inline bool operator==(const base_quaternion<T> &a, const base_quaternion<T> &b)
{
	return !(a!=b);
}

/////////////////////////////////////////////////////////////////////////
//	арифмитические поэлементтные операции
template<class T>
inline base_quaternion<T>& base_quaternion<T>::operator+=(const base_quaternion<T>& q)
{
	w += q.w;
	x += q.x;
	y += q.y;
	z += q.z;

	return *this;
}

template<class T>
inline base_quaternion<T>& base_quaternion<T>::operator-=(const base_quaternion<T>& q)
{
	w -= q.w;
	x -= q.x;
	y -= q.y;
	z -= q.z;

	return *this;
}

template<class T>
inline base_quaternion<T>& base_quaternion<T>::operator*=(const base_quaternion<T>& q)
{
	base_quaternion<T> tmp(	w*q.w-x*q.x-y*q.y-z*q.z,
							w*q.x+x*q.w+y*q.z-z*q.y,
							w*q.y+y*q.w+z*q.x-x*q.z,
							w*q.z+z*q.w+x*q.y-y*q.x);

	return *this = tmp;
}

template<class T>
inline base_quaternion<T>& base_quaternion<T>::operator*=(T f)
{
	w *= f;
	x *= f;
	y *= f;
	z *= f;

	return *this;
}
/////////////////////////////////////////////////////////////////////////
//	внешние арифмитические поэлементтные операции
template<class T>
inline base_quaternion<T> operator+(const base_quaternion<T>& a,const base_quaternion<T>& b)
{
	return base_quaternion<T>(a.w + b.w,
							  a.x + b.x,
							  a.y + b.y,
							  a.z + b.z);
}

template<class T>
inline base_quaternion<T> operator-(const base_quaternion<T>& a,const base_quaternion<T>& b)
{
	return base_quaternion<T>(a.w - b.w,
							  a.x - b.x,
							  a.y - b.y,
							  a.z - b.z);
}

template<class T>
inline base_quaternion<T> operator*(const base_quaternion<T>& a,const base_quaternion<T>& b)
{
	base_quaternion<T> result(a);
	result *= b;
	return result;
}

template<class T>
inline base_quaternion<T> operator*(T f,const base_quaternion<T>& a)
{
	return base_quaternion<T>(f*a.w,
							  f*a.x,
							  f*a.y,
							  f*a.z);
}

template<class T>
inline base_quaternion<T> operator*(const base_quaternion<T>& a,T f)
{
	return base_quaternion<T>(f*a.w,
							  f*a.x,
							  f*a.y,
							  f*a.z);
}

/////////////////////////////////////////////////////////////////////////
//	операция умножения вектора на кватернион
template<class T>
inline base_vector3<T> operator*(const base_vector3<T>& v,const base_quaternion<T>& q)
{
	return v*q.as_matrix();
}
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//	нахождение квадратичной длины кватерниона
template<class T>
inline T base_quaternion<T>::sqr_length(void) const
{
	return w*w+x*x+y*y+z*z;
}
/////////////////////////////////////////////////////////////////////////
//	нахождение длины (размера) кватерниона
template<class T>
inline typename base_quaternion<T>::quotient_type base_quaternion<T>::length(void) const
{
	return numerical_traits<T>::sqrt(sqr_length());
}
/////////////////////////////////////////////////////////////////////////
//	нормализовать кватернион
template<class T>
inline base_quaternion<T> &base_quaternion<T>::normalize(void)
{
	const quotient_type len = length();
	
	if(!numerical_traits<T>::is_null(sqr_length()))
	{
		const quotient_type iv = numerical_traits<T>::inverse(len);
		w *= iv;
		x *= iv;
		y *= iv;
		z *= iv;
	}
	
	return *this;
}
/////////////////////////////////////////////////////////////////////////
//	преобразовать квартенион в матрицу
template<class T>
inline base_matrix3<T> base_quaternion<T>::as_matrix(void) const
{
	T wx,wy,wz,xx,yy,yz,xy,xz,zz,x2,y2,z2;
	const T one = numerical_traits<T>::get_one();
	base_matrix3<T> m;
	
	x2 = x + x;
	y2 = y + y;
	z2 = z + z;
	xx = x * x2;   xy = x * y2;   xz = x * z2;
	yy = y * y2;   yz = y * z2;   zz = z * z2;
	wx = w * x2;   wy = w * y2;   wz = w * z2;
	
	m[0][0]=one-(yy+zz); m[0][1]=xy+wz;       m[0][2]=xz-wy;
	m[1][0]=xy-wz;       m[1][1]=one-(xx+zz); m[1][2]=yz+wx;
	m[2][0]=xz+wy;       m[2][1]=yz-wx;       m[2][2]=one-(xx+yy);
	
	m[0][3] = m[1][3] = m[2][3] = 0;
	m[3][0] = m[3][1] = m[3][2] = 0;
	m[3][3] = one;

	return m;
}
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// base_quaternion<T>::invert() //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline base_quaternion<T> &base_quaternion<T>::invert()
{
	w = -w;
	return *this;
}

template<class T>
inline base_quaternion<T> base_quaternion<T>::slerp(const base_quaternion<T> &q, T factor) const
{
    static const T delta = T(0.001);

    bool flip = false;
    T inv_factor;

    T norm = x * q.x + y * q.y + z * q.z + w * q.w;

    if (norm < numerical_traits<T>::get_null())
    {
        norm = -norm;
        flip = true;
    }

    if (numerical_traits<T>::get_one() - norm < delta)
    {
        inv_factor = numerical_traits<T>::get_one() - factor;
    }
    else
    {
	    const T theta = numerical_traits<T>::acos(norm);
        const T s = numerical_traits<T>::get_one() / numerical_traits<T>::sin(theta);
        
        inv_factor = numerical_traits<T>::sin((numerical_traits<T>::get_one() - factor) * theta) * s;
        factor = numerical_traits<T>::sin(factor * theta) * s;
    }

    if (flip)
    {
        factor = -factor;
    }

    return *this * inv_factor + q * factor;
}

template<typename T>
inline base_quaternion<T> base_quaternion<T>::identity()
{
	return base_quaternion<T>(
		numerical_traits<T>::get_one(),
		numerical_traits<T>::get_null(),
		numerical_traits<T>::get_null(),
		numerical_traits<T>::get_null());
}

//=====================================================================================//
//         inline base_axis_angle<T> base_quaternion<T>::to_axis_angle() const         //
//=====================================================================================//
template<typename T>
inline base_axis_angle<T> base_quaternion<T>::to_axis_angle() const
{
	typedef numerical_traits<T> nt;

	base_axis_angle<T> result;
	result.m_angle = 2*nt::acos(w);
	if(result.m_angle < static_cast<T>(1e-5))
	{
		result.m_axis = base_vector3<T>(0,0,0);
		result.m_angle = nt::get_null();
	}
	else
	{
		const T inv_sine = nt::inverse(nt::sin(result.m_angle*static_cast<T>(0.5)));
		result.m_axis = base_vector3<T>(x,y,z) * inv_sine;
	}

	return result;
}

/*=====================================================================================*\
 *                                    operator<< ()                                    * 
\*=====================================================================================*/
template<typename T>
inline ::std::ostream &operator<< (::std::ostream &stream, const base_quaternion<T> &pt)
{
	stream << "quaternion(" << pt.x << "," << pt.y << "," << pt.z << "," << pt.w<< ")";
	return stream;
}

/*=====================================================================================*\
 *                                    operator>> ()                                    * 
\*=====================================================================================*/
template<typename T>
inline ::std::istream &operator>> (::std::istream &stream, base_quaternion<T> &pt)
{
	stream >> std::ws >> io::verify("quaternion") >> std::ws >> io::verify("(");
	stream >> std::ws >> pt.x >> std::ws >> io::verify(",");
	stream >> std::ws >> pt.y >> std::ws >> io::verify(",");
	stream >> std::ws >> pt.z >> std::ws >> io::verify(",");
	stream >> std::ws >> pt.w >> std::ws >> io::verify(")");
	return stream;
}


//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
template<typename T>
inline io::ibinstream &operator>>(io::ibinstream &stream, base_quaternion<T> &pt)
{
	return io::do_raw_input(stream,pt);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
template<typename T>
inline io::obinstream &operator<<(io::obinstream &stream, const base_quaternion<T> &pt)
{
	return io::do_raw_output(stream,pt);
}







/////////////////////////////////////////////////////////////////////////
//	наиболее часто используемые определения
typedef base_quaternion<float> quaternion;
typedef base_quaternion<float> quaternionf;
typedef base_axis_angle<float> axis_angle;
typedef base_axis_angle<float> axis_anglef;
/////////////////////////////////////////////////////////////////////////


}	//	namesapce algebra

}	//	namesapce mll


#endif