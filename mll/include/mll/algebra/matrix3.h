#if !defined(__MATRIX3_INCLUDED__)
#define __MATRIX3_INCLUDED__

#include <cassert>

/////////////////////////////////////////////////////////////////////////
//	MiST land Lybrary files
#include <mll/algebra/_typedefs.h>
#include <mll/algebra/vector3.h>
#include <mll/algebra/point3.h>
#include <mll/io/iutils.h>
/////////////////////////////////////////////////////////////////////////

namespace mll
{

namespace algebra
{

/////////////////////////////////////////////////////////////////////////
////////////////////////    class base_matrix3    ///////////////////////
/////////////////////////////////////////////////////////////////////////
template<class T>
class base_matrix3
{
public:

	/////////////////////////////////////////////////////////////////////////
	union
	{
		struct
		{
			T _00, _01, _02, _03;
			T _10, _11, _12, _13;
			T _20, _21, _22, _23;
			T _30, _31, _32, _33;
		};
		T x[4][4];
	};
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	конструктор по умолчанию
	base_matrix3();
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	конструктор копирования
	base_matrix3(const base_matrix3& m);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	конструирование нулевой матрицы с элементом v по диагонали
	explicit base_matrix3(const T& v);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	конструирование единичной матрицы
	base_matrix3(const base_identity &);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	конструирование матрицы по параметрам
	base_matrix3(T x11,T x12,T x13,T x14,
				 T x21,T x22,T x23,T x24,
				 T x31,T x32,T x33,T x34,
				 T x41,T x42,T x43,T x44);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	оператор копирования
	base_matrix3& operator=(const base_matrix3& m);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	оператор копирования для единичной матрицы
	base_matrix3& operator=(const base_identity& m);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	арифмитические поэлементные операции
	const base_matrix3& operator+=(const base_matrix3& m);
	const base_matrix3& operator-=(const base_matrix3& m);
	const base_matrix3& operator*=(const T& f);
	const base_matrix3& operator/=(const T& f);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	перемножение матриц
	const base_matrix3& operator*=(const base_matrix3& m);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	оператор доступа по индексу
	T* operator[](int index);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	константный оператор доступа по индексу
	const T* operator[](int index) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	интвертирование матрицы (возвращает измененцю матрицу)
	base_matrix3& invert(void);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	транспонирование матрицы (возвращает измененцю матрицу)
	base_matrix3& transpose(void);
	/////////////////////////////////////////////////////////////////////////

	bool is_orth() const;
	base_matrix3 &invert_orth();
   /*
	/////////////////////////////////////////////////////////////////////////
	//	внешние поэлементные арифмитические операторы
	template<typename R>
	friend base_matrix3<R> operator+(const base_matrix3<R>& a,const base_matrix3<R>& b);
	template<typename R>
	friend base_matrix3<R> operator-(const base_matrix3<R>& a,const base_matrix3<R>& b);
//	template<typename R>
//	friend base_matrix3<R> operator*=(const base_matrix3<R>& a,const R& f);
//	template<typename R>
//	friend base_matrix3<R> operator/=(const base_matrix3<R>& a,const R& f);
	/////////////////////////////////////////////////////////////////////////

	
	//////////////////////////////////////////////////////////////////////////
	//	внешний оператор перемножения матриц
	template<typename R>
	friend base_matrix3<R> operator*(const base_matrix3<R>& a,const base_matrix3<R>& b);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	внешний оператор перемножения вектора и матрицы
	template<typename R>
	friend base_vector3<R> operator*(const base_vector3<R>& v,const base_matrix3<R>& m);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	внешний оператор перемножения точки и матрицы
	template<typename R>
	friend base_point3<R>  operator*(const base_point3<R>& v,const base_matrix3<R>& m);
	/////////////////////////////////////////////////////////////////////////

	template<typename R>
	friend bool operator == (const base_matrix3<R> &, const base_matrix3<R> &);
			*/
	/////////////////////////////////////////////////////////////////////////
	//	получение единичной матрицы
	static base_matrix3 identity(void);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	получение нулевой матрицы
	static base_matrix3 zero(void);
	/////////////////////////////////////////////////////////////////////////

private:

	/////////////////////////////////////////////////////////////////////////
	//	нахождение детерминанта матрицы 3х3
	T det3x3(const T& a00,const T& a01,const T& a02,
	         const T& a10,const T& a11,const T& a12,
			 const T& a20,const T& a21,const T& a22) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	нахождение алгебраического дополнения
	T algebraic_adjunction(int r,int c) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	нахождение детерминанта
	T determinant(void) const;
	/////////////////////////////////////////////////////////////////////////

};

template<class T>
inline base_matrix3<T>::base_matrix3()
{
}

template<class T>
inline base_matrix3<T>::base_matrix3(const base_identity &)
{
	_00 = numerical_traits<T>::get_one();
	_01 = numerical_traits<T>::get_null();
	_02 = numerical_traits<T>::get_null();
	_03 = numerical_traits<T>::get_null();

	_10 = numerical_traits<T>::get_null();
	_11 = numerical_traits<T>::get_one();
	_12 = numerical_traits<T>::get_null();
	_13 = numerical_traits<T>::get_null();

	_20 = numerical_traits<T>::get_null();
	_21 = numerical_traits<T>::get_null();
	_22 = numerical_traits<T>::get_one();
	_23 = numerical_traits<T>::get_null();

	_30 = numerical_traits<T>::get_null();
	_31 = numerical_traits<T>::get_null();
	_32 = numerical_traits<T>::get_null();
	_33 = numerical_traits<T>::get_one();
}


template<class T>
inline base_matrix3<T>::base_matrix3(const base_matrix3& m)
{
	x[0][0]=m.x[0][0]; x[0][1]=m.x[0][1]; x[0][2]=m.x[0][2]; x[0][3]=m.x[0][3];
	x[1][0]=m.x[1][0]; x[1][1]=m.x[1][1]; x[1][2]=m.x[1][2]; x[1][3]=m.x[1][3];
	x[2][0]=m.x[2][0]; x[2][1]=m.x[2][1]; x[2][2]=m.x[2][2]; x[2][3]=m.x[2][3];
	x[3][0]=m.x[3][0]; x[3][1]=m.x[3][1]; x[3][2]=m.x[3][2]; x[3][3]=m.x[3][3];
}

template<class T>
inline base_matrix3<T>::base_matrix3(const T& v)
{
	x[0][0] = x[1][1] = x[2][2] = v;
	x[3][3] = numerical_traits<T>::get_one();

	x[0][1] = x[0][2] = x[0][3] = 
	x[1][0] = x[1][2] = x[1][3] =
	x[2][0] = x[2][1] = x[2][3] =
	x[3][0] = x[3][1] = x[3][2] = numerical_traits<T>::get_null();
}

template<class T>
inline base_matrix3<T>::base_matrix3(T x11,T x12,T x13,T x14,
									 T x21,T x22,T x23,T x24,
									 T x31,T x32,T x33,T x34,
									 T x41,T x42,T x43,T x44)
{
	x[0][0] = x11; x[0][1] = x12; x[0][2] = x13; x[0][3] = x14;
	x[1][0] = x21; x[1][1] = x22; x[1][2] = x23; x[1][3] = x24;
	x[2][0] = x31; x[2][1] = x32; x[2][2] = x33; x[2][3] = x34;
	x[3][0] = x41; x[3][1] = x42; x[3][2] = x43; x[3][3] = x44;
}

template<class T>
inline base_matrix3<T>& base_matrix3<T>::operator=(const base_matrix3<T>& m)
{
	x[0][0]=m.x[0][0]; x[0][1]=m.x[0][1]; x[0][2]=m.x[0][2]; x[0][3]=m.x[0][3];
	x[1][0]=m.x[1][0]; x[1][1]=m.x[1][1]; x[1][2]=m.x[1][2]; x[1][3]=m.x[1][3];
	x[2][0]=m.x[2][0]; x[2][1]=m.x[2][1]; x[2][2]=m.x[2][2]; x[2][3]=m.x[2][3];
	x[3][0]=m.x[3][0]; x[3][1]=m.x[3][1]; x[3][2]=m.x[3][2]; x[3][3]=m.x[3][3];
	
	return *this;
}

template<class T>
inline base_matrix3<T>& base_matrix3<T>::operator=(const base_identity &)
{
	_00 = numerical_traits<T>::get_one();
	_01 = numerical_traits<T>::get_null();
	_02 = numerical_traits<T>::get_null();
	_03 = numerical_traits<T>::get_null();

	_10 = numerical_traits<T>::get_null();
	_11 = numerical_traits<T>::get_one();
	_12 = numerical_traits<T>::get_null();
	_13 = numerical_traits<T>::get_null();

	_20 = numerical_traits<T>::get_null();
	_21 = numerical_traits<T>::get_null();
	_22 = numerical_traits<T>::get_one();
	_23 = numerical_traits<T>::get_null();

	_30 = numerical_traits<T>::get_null();
	_31 = numerical_traits<T>::get_null();
	_32 = numerical_traits<T>::get_null();
	_33 = numerical_traits<T>::get_one();
	
	return *this;
}

template<class T>
inline const base_matrix3<T>& base_matrix3<T>::operator+=(const base_matrix3<T>& m)
{
	x[0][0]+=m.x[0][0]; x[0][1]+=m.x[0][1];	x[0][2]+=m.x[0][2]; x[0][3]+=m.x[0][3];
	x[1][0]+=m.x[1][0]; x[1][1]+=m.x[1][1];	x[1][2]+=m.x[1][2]; x[1][3]+=m.x[1][3];
	x[2][0]+=m.x[2][0]; x[2][1]+=m.x[2][1];	x[2][2]+=m.x[2][2]; x[2][3]+=m.x[2][3];
	x[3][0]+=m.x[3][0]; x[3][1]+=m.x[3][1];	x[3][2]+=m.x[3][2]; x[3][3]+=m.x[3][3];

	return *this;
}

template<class T>
inline const base_matrix3<T>& base_matrix3<T>::operator-=(const base_matrix3<T>& m)
{
	x[0][0]-=m.x[0][0]; x[0][1]-=m.x[0][1];	x[0][2]-=m.x[0][2]; x[0][3]-=m.x[0][3];
	x[1][0]-=m.x[1][0]; x[1][1]-=m.x[1][1];	x[1][2]-=m.x[1][2]; x[1][3]-=m.x[1][3];
	x[2][0]-=m.x[2][0]; x[2][1]-=m.x[2][1];	x[2][2]-=m.x[2][2]; x[2][3]-=m.x[2][3];
	x[3][0]-=m.x[3][0]; x[3][1]-=m.x[3][1];	x[3][2]-=m.x[3][2]; x[3][3]-=m.x[3][3];

	return *this;
}

template<class T>
inline const base_matrix3<T>& base_matrix3<T>::operator*=(const T& f)
{
	x[0][0]*=f; x[0][1]*=f;	x[0][2]*=f; x[0][3]*=f;
	x[1][0]*=f; x[1][1]*=f;	x[1][2]*=f; x[1][3]*=f;
	x[2][0]*=f; x[2][1]*=f;	x[2][2]*=f; x[2][3]*=f;
	x[3][0]*=f; x[3][1]*=f;	x[3][2]*=f; x[3][3]*=f;

	return *this;
}

template<class T>
inline const base_matrix3<T>& base_matrix3<T>::operator/=(const T& f)
{
	x[0][0]/=f; x[0][1]/=f;	x[0][2]/=f; x[0][3]/=f;
	x[1][0]/=f; x[1][1]/=f;	x[1][2]/=f; x[1][3]/=f;
	x[2][0]/=f; x[2][1]/=f;	x[2][2]/=f; x[2][3]/=f;
	x[3][0]/=f; x[3][1]/=f;	x[3][2]/=f; x[3][3]/=f;

	return *this;
}

template<class T>
inline const base_matrix3<T>& base_matrix3<T>::operator*=(const base_matrix3<T>& m)
{
	base_matrix3<T> tmp(

	x[0][0]*m.x[0][0]+x[0][1]*m.x[1][0]+x[0][2]*m.x[2][0]+x[0][3]*m.x[3][0],
	x[0][0]*m.x[0][1]+x[0][1]*m.x[1][1]+x[0][2]*m.x[2][1]+x[0][3]*m.x[3][1],
	x[0][0]*m.x[0][2]+x[0][1]*m.x[1][2]+x[0][2]*m.x[2][2]+x[0][3]*m.x[3][2],
	x[0][0]*m.x[0][3]+x[0][1]*m.x[1][3]+x[0][2]*m.x[2][3]+x[0][3]*m.x[3][3],
	
	x[1][0]*m.x[0][0]+x[1][1]*m.x[1][0]+x[1][2]*m.x[2][0]+x[1][3]*m.x[3][0],
	x[1][0]*m.x[0][1]+x[1][1]*m.x[1][1]+x[1][2]*m.x[2][1]+x[1][3]*m.x[3][1],
	x[1][0]*m.x[0][2]+x[1][1]*m.x[1][2]+x[1][2]*m.x[2][2]+x[1][3]*m.x[3][2],
	x[1][0]*m.x[0][3]+x[1][1]*m.x[1][3]+x[1][2]*m.x[2][3]+x[1][3]*m.x[3][3],
	
	x[2][0]*m.x[0][0]+x[2][1]*m.x[1][0]+x[2][2]*m.x[2][0]+x[2][3]*m.x[3][0],
	x[2][0]*m.x[0][1]+x[2][1]*m.x[1][1]+x[2][2]*m.x[2][1]+x[2][3]*m.x[3][1],
	x[2][0]*m.x[0][2]+x[2][1]*m.x[1][2]+x[2][2]*m.x[2][2]+x[2][3]*m.x[3][2],
	x[2][0]*m.x[0][3]+x[2][1]*m.x[1][3]+x[2][2]*m.x[2][3]+x[2][3]*m.x[3][3],
	
	x[3][0]*m.x[0][0]+x[3][1]*m.x[1][0]+x[3][2]*m.x[2][0]+x[3][3]*m.x[3][0],
	x[3][0]*m.x[0][1]+x[3][1]*m.x[1][1]+x[3][2]*m.x[2][1]+x[3][3]*m.x[3][1],
	x[3][0]*m.x[0][2]+x[3][1]*m.x[1][2]+x[3][2]*m.x[2][2]+x[3][3]*m.x[3][2],
	x[3][0]*m.x[0][3]+x[3][1]*m.x[1][3]+x[3][2]*m.x[2][3]+x[3][3]*m.x[3][3]);

	return *this = tmp;
}

template<class T>
inline T* base_matrix3<T>::operator[](int index)
{
	return &x[index][0];
}

template<class T>
inline const T* base_matrix3<T>::operator[](int index) const
{
	return &x[index][0];
}

template<class T>
inline base_matrix3<T> operator+(const base_matrix3<T>& a,const base_matrix3<T>& b)
{
	base_matrix3<T> res(a);

	res += b;

	return res;
}

template<class T>
inline base_matrix3<T> operator-(const base_matrix3<T>& a,const base_matrix3<T>& b)
{
	base_matrix3<T> res(a);

	res -= b;

	return res;
}

template<class T>
inline base_matrix3<T> operator*(const base_matrix3<T>& a,const T& f)
{
	base_matrix3<T> res(a);

	res *= f;

	return res;
}

template<class T>
inline base_matrix3<T> operator/(const base_matrix3<T>& a,const T& f)
{
	base_matrix3<T> res(a);

	res /= f;

	return res;
}

template<class T>
inline base_matrix3<T> operator*(const base_matrix3<T>& a,const base_matrix3<T>& b)
{
	base_matrix3<T> res(a);

	res *= b;

	return res;
}

template<class T>
inline base_vector3<T> operator*(const base_vector3<T>& v,const base_matrix3<T>& m)
{
	base_vector3<T> res;

	res.x = v.x*m.x[0][0] + v.y*m.x[1][0] + v.z*m.x[2][0];
	res.y = v.x*m.x[0][1] + v.y*m.x[1][1] + v.z*m.x[2][1];
	res.z = v.x*m.x[0][2] + v.y*m.x[1][2] + v.z*m.x[2][2];

	return res;
}

template<class T>
inline base_point3<T> operator*(const base_point3<T>& v,const base_matrix3<T>& m)
{
	base_point3<T> res;

	res.x = v.x*m.x[0][0] + v.y*m.x[1][0] + v.z*m.x[2][0] + m.x[3][0];
	res.y = v.x*m.x[0][1] + v.y*m.x[1][1] + v.z*m.x[2][1] + m.x[3][1];
	res.z = v.x*m.x[0][2] + v.y*m.x[1][2] + v.z*m.x[2][2] + m.x[3][2];

	numerical_traits<T>::quotient_type denom = v.x*m.x[0][3] + v.y*m.x[1][3] + v.z*m.x[2][3] + m.x[3][3];

	res /= denom;

	return res;
}

template<class T>
inline base_matrix3<T>& base_matrix3<T>::invert(void)
{
    base_matrix3<T> out;

	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++)
			out.x[i][j] = algebraic_adjunction(i,j);

	*this = out.transpose()/determinant();

	return *this;
}

template<class T>
inline base_matrix3<T>& base_matrix3<T>::transpose(void)
{
	T t;

	t = x[0][1]; x[0][1] = x[1][0]; x[1][0] = t;
	t = x[0][2]; x[0][2] = x[2][0];	x[2][0] = t;
	t = x[0][3]; x[0][3] = x[3][0];	x[3][0] = t;
	t = x[1][3]; x[1][3] = x[3][1];	x[3][1] = t;
	t = x[2][3]; x[2][3] = x[3][2];	x[3][2] = t;
	t = x[1][2]; x[1][2] = x[2][1];	x[2][1] = t;

	return *this;
}

//=====================================================================================//
//                                     translate()                                     //
//=====================================================================================//
template<class T>
inline base_matrix3<T> translate(const base_vector3<T>& v)
{
	base_matrix3<T> res(base_matrix3<T>::identity());

	res.x[3][0] = v.x;
	res.x[3][1] = v.y;
	res.x[3][2] = v.z;

	return res;
}

//=====================================================================================//
//                                     translate()                                     //
//=====================================================================================//
template<class T>
inline base_matrix3<T> translate(const base_point3<T> &v)
{
	base_matrix3<T> res(base_matrix3<T>::identity());

	res.x[3][0] = v.x;
	res.x[3][1] = v.y;
	res.x[3][2] = v.z;

	return res;
}

template<class T>
inline base_matrix3<T> scale(const base_vector3<T>& v)
{
	base_matrix3<T> res(base_matrix3<T>::identity());

	res.x[0][0] = v.x;
	res.x[1][1] = v.y;
	res.x[2][2] = v.z;

	return res;
}

template<class T>
inline base_matrix3<T> rotate_x(T angle)
{
	base_matrix3<T> res(base_matrix3<T>::identity());

	const T cosine = numerical_traits<T>::cos(angle);
	const T sine = numerical_traits<T>::sin(angle);

	res.x[1][1] = cosine;
	res.x[1][2] = sine;
	res.x[2][1] = -sine;
	res.x[2][2] = cosine;

	return res;
}

template<class T>
inline base_matrix3<T> rotate_y(T angle)
{
	base_matrix3<T> res(base_matrix3<T>::identity());

	const T cosine = numerical_traits<T>::cos(angle);
	const T sine = numerical_traits<T>::sin(angle);

	res.x[0][0] = cosine;
	res.x[0][2] = sine;
	res.x[2][0] = -sine;
	res.x[2][2] = cosine;

	return res;
}

template<class T>
inline base_matrix3<T> rotate_z(T angle)
{
	base_matrix3<T> res(base_matrix3<T>::identity());

	const T cosine = numerical_traits<T>::cos(angle);
	const T sine = numerical_traits<T>::sin(angle);

	res.x[0][0] = cosine;
	res.x[0][1] = sine;
	res.x[1][0] = -sine;
	res.x[1][1] = cosine;

	return res;
}

//=====================================================================================//
//                       inline base_matrix3<T> perspective_lh()                       //
//=====================================================================================//
template<class T>
inline base_matrix3<T> perspective_lh(T znear,T zfar,T hfov,T aspect)
{
	const T cosine = numerical_traits<T>::cos(hfov*0.5f);
	const T sine = numerical_traits<T>::sin(hfov*0.5f);
	const T c = cosine/sine;
	const T W = aspect * c;
	const T H = c;
	const T Q = zfar/(zfar-znear);
	const T D = -Q*znear;

	return matrix3(   -W, 0.0f, 0.0f, 0.0f,
					0.0f,	 H, 0.0f, 0.0f,
					0.0f, 0.0f,	   Q, 1.0f,
					0.0f, 0.0f,    D, 0.0f  );
/*
	//	вариант Gvozdoder'а - дает не совсем ожидаемые результаты
	const float ctg_hfov = cosf(hfov*0.5f)/sinf(hfov*0.5f);
	const float ctg_vfov = ctg_hfov/aspect;
	const float E = znear * ctg_hfov;
	const float F = znear * ctg_vfov;
	const float C = - (zfar + znear) / (zfar - znear);
	const float D = -1.0f * (zfar * znear) / (zfar - znear);

	return matrix3(   -E, 0.0f, 0.0f, 0.0f,
					0.0f,	F,  0.0f, 0.0f,
					0.0f, 0.0f,	  -C,   -D,
					0.0f, 0.0f, -1.0f,0.0f  )/-D;
*/
}

//=====================================================================================//
//                       inline base_matrix3<T> perspective_rh()                       //
//=====================================================================================//
template<class T>
inline base_matrix3<T> perspective_rh(T znear,T zfar,T hfov,T aspect)
{
	const T cosine = numerical_traits<T>::cos(hfov*0.5f);
	const T sine = numerical_traits<T>::sin(hfov*0.5f);
	const T c = cosine/sine;
	const T W = aspect * c;
	const T H = c;
	const T Q = zfar/(zfar-znear);
	const T D = -Q*znear;

	return matrix3(    W, 0.0f, 0.0f, 0.0f,
					0.0f,	 H, 0.0f, 0.0f,
					0.0f, 0.0f,	   Q, 1.0f,
					0.0f, 0.0f,    D, 0.0f  );
}

//=====================================================================================//
//                       inline base_matrix3<T> perspective_rh()                       //
//=====================================================================================//
template<class T>
inline base_matrix3<T> perspective_lh2(T znear,T zfar,T hfov,T aspect)
{
	const T cosine = numerical_traits<T>::cos(hfov*0.5f);
	const T sine = numerical_traits<T>::sin(hfov*0.5f);
	const T c = cosine/sine;
	const T W = aspect * c;
	const T H = c;
	const T Q = zfar/(zfar-znear);
	const T D = -Q*znear;

	return matrix3(    W, 0.0f, 0.0f, 0.0f,
					0.0f,	 H, 0.0f, 0.0f,
					0.0f, 0.0f,	   Q, 1.0f,
					0.0f, 0.0f,    D, 0.0f  );
}

//=====================================================================================//
//                       inline base_matrix3<T> perspective_lh()                       //
//=====================================================================================//
template<class T>
inline base_matrix3<T> perspective_rh2(T znear,T zfar,T hfov,T aspect)
{
	const T cosine = numerical_traits<T>::cos(hfov*0.5f);
	const T sine = numerical_traits<T>::sin(hfov*0.5f);
	const T c = cosine/sine;
	const T W = aspect * c;
	const T H = c;
	const T Q = zfar/(zfar-znear);
	const T D = -Q*znear;

	return matrix3(   -W, 0.0f, 0.0f, 0.0f,
					0.0f,	 H, 0.0f, 0.0f,
					0.0f, 0.0f,	   Q, 1.0f,
					0.0f, 0.0f,    D, 0.0f  );
}

//=====================================================================================//
//                            inline base_matrix3<T> view()                            //
//=====================================================================================//
template<class T>
inline base_matrix3<T> view(const base_vector3<T>& dir,const base_vector3<T>& top,const base_point3<T>& origin)
{
	const vector3 k = dir;
	const vector3 j = top;
	const vector3 i = j.cross(k);
	const point3 o = origin;

	assert( fabsf(j.dot(k)) < 1.0e-5f ); // Вектора i и k должны быть перпендикулярны

	return matrix3 (      i.x,	      j.x,		  k.x,	0.0f,
					      i.y,	      j.y,		  k.y,	0.0f,
					      i.z,	      j.z,		  k.z,	0.0f,
					-o.dot(i),	-o.dot(j),	-o.dot(k),	1.0f	);
}

template<class T>
inline base_matrix3<T> base_matrix3<T>::identity(void)
{
	return base_matrix3<T>(numerical_traits<T>::get_one());
}

template<class T>
inline base_matrix3<T> base_matrix3<T>::zero(void)
{
	const T z = numerical_traits<T>::get_null();
	return base_matrix3<T>(z,z,z,z,
						   z,z,z,z,
						   z,z,z,z,
						   z,z,z,z);
}

/////////////////////////////////////////////////////////////////////////
//	нахождение детерминанта матрицы 3х3
template<class T>
inline T base_matrix3<T>::det3x3(const T& a00,const T& a01,const T& a02,
								 const T& a10,const T& a11,const T& a12,
								 const T& a20,const T& a21,const T& a22) const
{
	return (a00*a11*a22+a20*a01*a12+a10*a21*a02 - a20*a11*a02-a10*a01*a22-a00*a21*a12);	
}
/////////////////////////////////////////////////////////////////////////
//	нахождение алгебраического дополнения
template<class T>
inline T base_matrix3<T>::algebraic_adjunction(int r,int c) const
{
	const T one = numerical_traits<T>::get_one();
	int k = 0;
	T m[9];

	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			if((i==r)||(j==c)) continue;
			m[k] = x[i][j];
			k++;
		}
	}

	return (((r+c)&1)?-one:one)*det3x3(m[0],m[1],m[2],m[3],m[4],m[5],m[6],m[7],m[8]);
}
/////////////////////////////////////////////////////////////////////////
//	нахождение детерминанта
template<class T>
T base_matrix3<T>::determinant(void) const
{
	return x[0][0]*det3x3(x[1][1], x[1][2], x[1][3],
                          x[2][1], x[2][2], x[2][3],
						  x[3][1], x[3][2], x[3][3]) -
		   x[1][0]*det3x3(x[0][1], x[0][2], x[0][3],
                          x[2][1], x[2][2], x[2][3],
						  x[3][1], x[3][2], x[3][3]) +
		   x[2][0]*det3x3(x[0][1], x[0][2], x[0][3],
                          x[1][1], x[1][2], x[1][3],
						  x[3][1], x[3][2], x[3][3]) -
		   x[3][0]*det3x3(x[0][1], x[0][2], x[0][3],
                          x[1][1], x[1][2], x[1][3],
						  x[2][1], x[2][2], x[2][3]);
}
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// base_matrix3<T>::is_orth() ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<class T>
bool base_matrix3<T>::is_orth() const
{
	base_vector3<T> i(_00,_10,_20);
	base_vector3<T> j(_01,_11,_21);
	base_vector3<T> k(_02,_12,_22);

	const T one = numerical_traits<T>::get_one();
	const T zero = numerical_traits<T>::get_null();

	return i.dot(j) == zero && i.dot(k) == zero && j.dot(k) == zero &&
		i.length() == one && j.length() == one && k.length() == one &&
		_03 == zero && _13 == zero && _23 == zero && _33 == one;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// base_matrix3<T>::invert_orth() /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<class T>
base_matrix3<T> &base_matrix3<T>::invert_orth()
{
	base_vector3<T> translate(_30,_31,_32);
	base_vector3<T> right(_00,_01,_02);
	base_vector3<T> up(_10,_11,_12);
	base_vector3<T> dir(_20,_21,_22);

	base_matrix3<T> tmp(identity());

	tmp._30=-translate.dot(right);
	tmp._31=-translate.dot(up);
	tmp._32=-translate.dot(dir);

	tmp._00=_00; tmp._01=_10; tmp._02=_20;
	tmp._10=_01; tmp._11=_11; tmp._12=_21;
	tmp._20=_02; tmp._21=_12; tmp._22=_22;

	*this = tmp;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// operator == () /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline bool operator == (const base_matrix3<T> &m1, const base_matrix3<T> &m2)
{
	return m1._00 == m2._00 && m1._01 == m2._01 && m1._02 == m2._02 && m1._03 == m2._03 &&
		   m1._10 == m2._10 && m1._11 == m2._11 && m1._12 == m2._12 && m1._13 == m2._13 &&
		   m1._20 == m2._20 && m1._21 == m2._21 && m1._22 == m2._22 && m1._23 == m2._23 &&
		   m1._30 == m2._30 && m1._31 == m2._31 && m1._32 == m2._32 && m1._33 == m2._33;
}

//=====================================================================================//
//                              inline bool operator!=()                               //
//=====================================================================================//
template<typename T>
inline bool operator!=(const base_matrix3<T> &l, const base_matrix3<T> &r)
{
	return !(l == r);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
template<typename T>
inline std::ostream &operator<<(std::ostream &stream, const base_matrix3<T> &m)
{
	stream << "matrix3(" << m._00 << ", " << m._01 << ", " << m._02 << ", " << m._03 << ",\n";
	stream << "        " << m._10 << ", " << m._11 << ", " << m._12 << ", " << m._13 << ",\n";
	stream << "        " << m._20 << ", " << m._21 << ", " << m._22 << ", " << m._23 << ",\n";
	stream << "        " << m._30 << ", " << m._31 << ", " << m._32 << ", " << m._33 << ")";
	return stream;
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
template<typename T>
inline std::istream &operator>>(std::istream &stream, base_matrix3<T> &m)
{
	stream >> std::ws >> io::verify("matrix3") >> std::ws >> io::verify("(");

	stream >> std::ws >> m._00 >> std::ws >> io::verify(",");
	stream >> std::ws >> m._01 >> std::ws >> io::verify(",");
	stream >> std::ws >> m._02 >> std::ws >> io::verify(",");
	stream >> std::ws >> m._03 >> std::ws >> io::verify(",");

	stream >> std::ws >> m._10 >> std::ws >> io::verify(",");
	stream >> std::ws >> m._11 >> std::ws >> io::verify(",");
	stream >> std::ws >> m._12 >> std::ws >> io::verify(",");
	stream >> std::ws >> m._13 >> std::ws >> io::verify(",");

	stream >> std::ws >> m._20 >> std::ws >> io::verify(",");
	stream >> std::ws >> m._21 >> std::ws >> io::verify(",");
	stream >> std::ws >> m._22 >> std::ws >> io::verify(",");
	stream >> std::ws >> m._23 >> std::ws >> io::verify(",");

	stream >> std::ws >> m._30 >> std::ws >> io::verify(",");
	stream >> std::ws >> m._31 >> std::ws >> io::verify(",");
	stream >> std::ws >> m._32 >> std::ws >> io::verify(",");
	stream >> std::ws >> m._33 >> std::ws >> io::verify(")");
	return stream;
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
template<typename T>
inline io::ibinstream &operator>>(io::ibinstream &stream, base_matrix3<T> &pt)
{
	return io::do_raw_input(stream,pt);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
template<typename T>
inline io::obinstream &operator<<(io::obinstream &stream, const base_matrix3<T> &pt)
{
	return io::do_raw_output(stream,pt);
}

/////////////////////////////////////////////////////////////////////////
//	наиболее часто используемые определения
typedef base_matrix3<float> matrix3;
typedef base_matrix3<float> matrix3f;
typedef base_matrix3<int> matrix3i;
/////////////////////////////////////////////////////////////////////////

}	//	namesapce algebra

}	//	namesapce mll

#endif