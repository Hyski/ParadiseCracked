#if !defined(__OBB3_H_INCLUDED_7552370787048739__)
#define __OBB3_H_INCLUDED_7552370787048739__

#include <assert.h>
#include <utility>
#include <mll/algebra/vector3.h>
#include <mll/algebra/point3.h>
#include <mll/geometry/ray3.h>
#include <mll/geometry/aabb3.h>
#include <mll/algebra/matrix3.h>
#include <mll/algebra/numerical_traits.h>

namespace mll
{
namespace geometry
{

//=====================================================================================//
//                                   class base_obb3                                   //
//=====================================================================================//
template<typename T>
class base_obb3
{
public:
	typedef algebra::numerical_traits<T> traits_type;
	typedef algebra::base_vector3<T> vector_type;
	typedef algebra::base_point3<T> point_type;
	typedef base_ray3<T> ray_type;
	typedef algebra::base_matrix3<T> matrix_type;
	typedef T number_type;

	enum
	{
		ptf_x		=	1<<0,
		ptf_y		=	1<<1,
		ptf_z		=	1<<2,

		pt_origin	=	0,
		pt_x		=	ptf_x,
		pt_xy		=	ptf_x|ptf_y,
		pt_xz		=	ptf_x|ptf_z,
		pt_xyz		=	ptf_x|ptf_y|ptf_z,
		pt_y		=	ptf_y,
		pt_yz		=	ptf_y|ptf_z,
		pt_z		=	ptf_z,

		pt_count		=	8,
	};

private:
	bool m_degenerated;
	vector_type m_x[3];
	number_type m_factor[3];
	point_type m_origin;

public:
	base_obb3() : m_degenerated(true) {}
	base_obb3(const vector_type &, const vector_type &, const vector_type &, const point_type &);
	base_obb3(const base_obb3 &);
	base_obb3(const base_aabb3<T> &);

	bool is_degenerated() const { return m_degenerated; }
	void degenerate() { m_degenerated = true; }

	base_obb3 &grow(const point_type &);
	base_obb3 &grow(const base_obb3 &);

	point_type operator[](unsigned) const;

	std::pair<T,T> full_intersect(const ray_type &ray) const;
	T intersect(const ray_type &ray) const;
	bool intersect(const base_obb3<T> &box) const;

	bool inside(const point_type &) const;

	base_obb3 &shrink() { m_factor[0] = m_factor[1] = m_factor[2] = traits_type::get_null(); return *this; }

	static T bad_result() {return mll::algebra::numerical_traits<T>::get_bad_ray_intersection();}

	point_type origin() const { return m_origin; }

	vector_type x() const { return m_x[0] * m_factor[0]; }
	vector_type y() const { return m_x[1] * m_factor[1]; }
	vector_type z() const { return m_x[2] * m_factor[2]; }

	const vector_type &x_dir() const { return m_x[0]; }
	const vector_type &y_dir() const { return m_x[1]; }
	const vector_type &z_dir() const { return m_x[2]; }

	number_type x_length() const { return m_factor[0]; }
	number_type y_length() const { return m_factor[1]; }
	number_type z_length() const { return m_factor[2]; }

	number_type x_inv_length() const { return traits_type::get_one()/m_factor[0]; }
	number_type y_inv_length() const { return traits_type::get_one()/m_factor[1]; }
	number_type z_inv_length() const { return traits_type::get_one()/m_factor[2]; }

	base_obb3 &operator*=(const matrix_type &);

	//friend base_obb3 operator*(const base_obb3 &, const matrix_type &);
	//friend base_obb3 operator*(const matrix_type &, const base_obb3 &);

private:
	static void _grow(const point_type &, number_type &, vector_type &, point_type &);
	static number_type _validate(vector_type &, const vector_type &, const vector_type &);
	static vector_type _perp(const vector_type &, const vector_type &);
	static vector_type _perp_vec(const vector_type &);

	void _construct(const vector_type &x, const vector_type &y, const vector_type &z);
	bool _is_valid() const;
};

//=====================================================================================//
//                      inline T base_obb3<T>::intersect() const                       //
//=====================================================================================//
template<typename T>
inline T base_obb3<T>::intersect(const base_ray3<T> &ray) const
{
	std::pair<T,T> t = full_intersect(ray);
	if(t.first < algebra::numerical_traits<T>::get_null()) return t.second;
	return t.first;
}

//=====================================================================================//
//                      inline bool base_obb3<T>::intersect() const                       //
//=====================================================================================//

//typedef struct {
//	vector3 pos; // координаты центра
//	vector3 size; // размеры
//	vector3 axis[3]; // локальные оси
//} obb_t; 

template<typename T>
bool base_obb3<T>::intersect(const base_obb3<T> &box) const
{

	vector3 a, b; // размеры уменьшенные вдвое (радиусы)
	vector3 v, T;
	float rmatrix[3][3];
	float ra, rb, t;
	int i, j;

	// делим размеры пополам
	a.x = x_length()/2;
	a.y = y_length()/2;
	a.z = z_length()/2;

	b.x = box.x_length()/2;
	b.y = box.y_length()/2;
	b.z = box.z_length()/2;

	//
	// преобразуем B в систему координат первого OBB'а
	//

	v = (box.origin()+(box.x()+box.y()+box.z())*0.5f) - 
		(origin()+(x()+y()+z())*0.5f);

	T.x = v.dot(m_x[0]);
	T.y = v.dot(m_x[1]);
	T.z = v.dot(m_x[2]);

	// строим матрицу преобразования
	for( i=0 ; i<3 ; i++ )
		for( j=0 ; j<3 ; j++ ) 
		{
			rmatrix[i][j] = m_x[i].dot(box.m_x[j]);
		}

	// сперва проецируем на оси A
	for( i=0 ; i<3 ; i++ ) 
	{
		ra = a.v[i];

		rb =	b.x*fabsf(rmatrix[i][0]) +
				b.y*fabsf(rmatrix[i][1]) +
				b.z*fabsf(rmatrix[i][2]);

		t = fabsf( T[i] );

		if( t > ra + rb ) return false;
	}

	// на оси B
	for( i=0 ; i<3 ; i++ ) 
	{
		ra = a.x*fabsf(rmatrix[0][i]) +
			 a.y*fabsf(rmatrix[1][i]) +
			 a.z*fabsf(rmatrix[2][i]);

		rb = box.m_factor[i];

		t = fabsf(  T.x*rmatrix[0][i] +
					T.y*rmatrix[1][i] +
					T.z*rmatrix[2][i] );

		if (t > ra + rb) return false;
	}

	//L = A0 x B0
	ra = a[1]*fabs(rmatrix[2][0]) + a[2]*fabs(rmatrix[1][0]);
	rb = b[1]*fabs(rmatrix[0][2]) + b[2]*fabs(rmatrix[0][1]);
	t = fabs( T[2]*rmatrix[1][0] - T[1]*rmatrix[2][0]);
	if( t > ra + rb ) return false;

	//L = A0 x B1
	ra = a[1]*fabs(rmatrix[2][1]) + a[2]*fabs(rmatrix[1][1]);
	rb = b[0]*fabs(rmatrix[0][2]) + b[2]*fabs(rmatrix[0][0]);
	t = fabs(T[2]*rmatrix[1][1] - T[1]*rmatrix[2][1]);
	if( t > ra + rb ) return false;

	//L = A0 x B2
	ra = a[1]*fabs(rmatrix[2][2]) + a[2]*fabs(rmatrix[1][2]);
	rb = b[0]*fabs(rmatrix[0][1]) + b[1]*fabs(rmatrix[0][0]);
	t = fabs(T[2]*rmatrix[1][2] - T[1]*rmatrix[2][2]);
	if( t > ra + rb ) return false;

	//L = A1 x B0
	ra = a[0]*fabs(rmatrix[2][0]) + a[2]*fabs(rmatrix[0][0]);
	rb = b[1]*fabs(rmatrix[1][2]) + b[2]*fabs(rmatrix[1][1]);
	t = fabs( T[0]*rmatrix[2][0] - T[2]*rmatrix[0][0] );
	if( t > ra + rb ) return false;

	//L = A1 x B1
	ra = a[0]*fabs(rmatrix[2][1]) + a[2]*fabs(rmatrix[0][1]);
	rb = b[0]*fabs(rmatrix[1][2]) + b[2]*fabs(rmatrix[1][0]);
	t = fabs( T[0]*rmatrix[2][1] - T[2]*rmatrix[0][1] );
	if( t > ra + rb ) return false;

	//L = A1 x B2
	ra = a[0]*fabs(rmatrix[2][2]) + a[2]*fabs(rmatrix[0][2]);
	rb = b[0]*fabs(rmatrix[1][1]) + b[1]*fabs(rmatrix[1][0]);
	t = fabs( T[0]*rmatrix[2][2] - T[2]*rmatrix[0][2] );
	if( t > ra + rb ) return false;

	//L = A2 x B0
	ra = a[0]*fabs(rmatrix[1][0]) + a[1]*fabs(rmatrix[0][0]);
	rb = b[1]*fabs(rmatrix[2][2]) + b[2]*fabs(rmatrix[2][1]);
	t = fabs( T[1]*rmatrix[0][0] - T[0]*rmatrix[1][0] );
	if( t > ra + rb ) return false;

	//L = A2 x B1
	ra = a[0]*fabs(rmatrix[1][1]) + a[1]*fabs(rmatrix[0][1]);
	rb = b[0]*fabs(rmatrix[2][2]) + b[2]*fabs(rmatrix[2][0]);
	t = fabs( T[1]*rmatrix[0][1] - T[0]*rmatrix[1][1] );
	if( t > ra + rb ) return false;

	//L = A2 x B2
	ra = a[0]*fabs(rmatrix[1][2]) + a[1]*fabs(rmatrix[0][2]);
	rb = b[0]*fabs(rmatrix[2][1]) + b[1]*fabs(rmatrix[2][0]);
	t = fabs( T[1]*rmatrix[0][2] - T[0]*rmatrix[1][2] );
	if( t > ra + rb ) return false;

	// если до сих пор не вышли, то боксы пересекаются
	return 1; 
} 




//=====================================================================================//
//             inline base_obb3<T>::vector_type base_obb3<T>::_perp_vec()              //
//=====================================================================================//
template<typename T>
inline typename base_obb3<T>::vector_type base_obb3<T>::_perp_vec(const vector_type &x)
{
	const vector_type tx(traits_type::get_one(),traits_type::get_null(),traits_type::get_null());
	vector_type r = tx.cross(x);

	if(r.sqr_length() == traits_type::get_null())
	{
		const vector_type ty(traits_type::get_null(),traits_type::get_one(),traits_type::get_null());
		r = ty.cross(x);
	}

	return r;
}

//=====================================================================================//
//               inline base_obb3<T>::vector_type base_obb3<T>::_perp()                //
//=====================================================================================//
template<typename T>
inline typename base_obb3<T>::vector_type base_obb3<T>::_perp(const vector_type &x, const vector_type &y)
{
	const number_type lx = x.sqr_length();
	const number_type ly = y.sqr_length();

	if(lx == traits_type::get_null() && ly == traits_type::get_null())
	{
		return vector_type(traits_type::get_one(),traits_type::get_null(),traits_type::get_null());
	}
	else if(lx == traits_type::get_null())
	{
		return _perp_vec(y);
	}
	else if(ly == traits_type::get_null())
	{
		return _perp_vec(x);
	}
	else
	{
		assert( x.cross(y) != vector_type(traits_type::get_null(),traits_type::get_null(),traits_type::get_null()) );
		return x.cross(y);
	}
}

//=====================================================================================//
//                     inline bool base_obb3<T>::_is_valid() const                     //
//=====================================================================================//
template<typename T>
inline bool base_obb3<T>::_is_valid() const
{
	if(is_degenerated()) return true;

	const T one = traits_type::get_one();
	const T epsilon = static_cast<T>(1e-5);

	for(unsigned i = 0; i < 3; ++i)
	{
		if( traits_type::abs(m_x[i].sqr_length() - one) > epsilon ) return false;
	}

	if( traits_type::abs(m_x[0].dot(m_x[1])) > epsilon ) return false;
	if( traits_type::abs(m_x[0].dot(m_x[2])) > epsilon ) return false;
	if( traits_type::abs(m_x[1].dot(m_x[2])) > epsilon ) return false;

	return true;
}

//=====================================================================================//
//                        inline void base_obb3<T>::_validate()                        //
//=====================================================================================//
template<typename T>
inline typename base_obb3<T>::number_type base_obb3<T>::_validate(vector_type &x,
									const vector_type &y, const vector_type &z)
{
	const number_type epsilon = static_cast<number_type>(1e-5);

	if(x.length() == traits_type::get_null() 
		|| traits_type::abs(y.dot(z)) < epsilon )
	{
		const vector_type newx = _perp(y,z).normalize();
		const number_type result = newx.dot(x);
		x = newx;
		return result;
	}
	else
	{
		number_type result = x.length();
		x.normalize();
		return result;
	}
}

//=====================================================================================//
//                       inline void base_obb3<T>::_construct()                        //
//=====================================================================================//
template<typename T>
inline void base_obb3<T>::_construct(const vector_type &x, const vector_type &y, const vector_type &z)
{
	m_x[0] = x;
	m_x[1] = y;
	m_x[2] = z;

	m_factor[0] = _validate(m_x[0],m_x[1],m_x[2]);
	m_factor[1] = _validate(m_x[1],m_x[2],m_x[0]);
	m_factor[2] = _validate(m_x[2],m_x[0],m_x[1]);
}


//=====================================================================================//
//                          inline base_obb3<T>::base_obb3()                           //
//=====================================================================================//
template<typename T>
inline base_obb3<T>::base_obb3(const vector_type &x, const vector_type &y, const vector_type &z,
							   const point_type &o)
:	m_origin(o), m_degenerated(false)
{
	_construct(x,y,z);
	assert( _is_valid() );
}

//=====================================================================================//
//                          inline base_obb3<T>::base_obb3()                           //
//=====================================================================================//
template<typename T>
inline base_obb3<T>::base_obb3(const base_obb3 &rhs)
:	m_degenerated(rhs.m_degenerated)
{
	if(!is_degenerated())
	{
		m_origin = rhs.m_origin;
		for(unsigned i = 0; i < 3; ++i)
		{
			m_x[i] = rhs.m_x[i];
			m_factor[i] = rhs.m_factor[i];
		}
	}
}

//=====================================================================================//
//                          inline base_obb3<T>::base_obb3()                           //
//=====================================================================================//
template<typename T>
inline base_obb3<T>::base_obb3(const base_aabb3<T> &rhs)
:	m_degenerated(rhs.is_degenerated())
{
	if(!is_degenerated())
	{
		const T tzero = traits_type::get_null();
		const T tone = traits_type::get_one();
		const vector_type diag = rhs.diag();

		m_origin = rhs.min();
		m_x[0] = vector_type(tone,tzero,tzero);
		m_x[1] = vector_type(tzero,tone,tzero);
		m_x[2] = vector_type(tzero,tzero,tone);

		m_factor[0] = diag.x;
		m_factor[1] = diag.y;
		m_factor[2] = diag.z;
	}

	assert( _is_valid() );
}

//=====================================================================================//
//          inline base_obb3<T>::point_type base_obb3<T>::operator[]() const           //
//=====================================================================================//
template<typename T>
inline typename base_obb3<T>::point_type base_obb3<T>::operator[](unsigned i) const
{
	const T tzero = traits_type::get_null();
	const vector_type zero(tzero,tzero,tzero);
	return m_origin +
			(i&ptf_x ? x() : zero) +
			(i&ptf_y ? y() : zero) +
			(i&ptf_z ? z() : zero);
}

//=====================================================================================//
//                          inline void base_obb3<T>::_grow()                          //
//=====================================================================================//
template<typename T>
inline void base_obb3<T>::_grow(const point_type &pt, number_type &f, vector_type &x, point_type &o)
{
	const T tzero = traits_type::get_null();
	const vector_type a = pt-o;
	const T dot = a.dot(x); // т. к. x.length() == 1.0, то dot равен длине проекции a на x

	if(dot < tzero)
	{
		o += x*dot;
		f -= dot;
	}
	else if(dot > f)
	{
		f = dot;
	}
}

//=====================================================================================//
//                      inline base_obb3<T> &base_obb3<T>::grow()                      //
//=====================================================================================//
template<typename T>
inline base_obb3<T> &base_obb3<T>::grow(const point_type &pt)
{
	if(is_degenerated())
	{
		const T tzero = traits_type::get_null();
		const vector_type zero(tzero,tzero,tzero);
		m_x[0] = m_x[1] = m_x[2] = zero;
		m_factor[0] = _validate(m_x[0],zero,zero);
		m_factor[1] = _validate(m_x[1],zero,m_x[0]);
		m_factor[2] = _validate(m_x[2],m_x[0],m_x[1]);
		m_origin = pt;
		m_degenerated = false;
	}
	else
	{
		for(unsigned i = 0; i < 3; ++i) _grow(pt,m_factor[i],m_x[i],m_origin);
	}

	assert( _is_valid() );
	assert( _is_valid() );

	return *this;
}

//=====================================================================================//
//                       inline base_obb3 &base_obb3<T>::grow()                        //
//=====================================================================================//
template<typename T>
inline base_obb3<T> &base_obb3<T>::grow(const base_obb3 &obb)
{
	if(is_degenerated())
	{
		*this = obb;
	}
	else if(!obb.is_degenerated())
	{
		for(unsigned i=0; i<8; ++i) grow(obb[i]);
	}

	return *this;
}

//=====================================================================================//
//                      inline bool base_obb3<T>::inside() const                       //
//=====================================================================================//
template<typename T>
inline bool base_obb3<T>::inside(const point_type &pt) const
{
	if(is_degenerated()) return false;

	const vector_type tv = pt-m_origin;

	const number_type dotx = tv.dot(x_dir());
	if( dotx > x_length() || dotx < traits_type::get_null() ) return false;

	const number_type doty = tv.dot(y_dir());
	if( doty > y_length() || doty < traits_type::get_null() ) return false;

	const number_type dotz = tv.dot(z_dir());
	if( dotz > z_length() || dotz < traits_type::get_null() ) return false;

	return true;
}

//=====================================================================================//
//             inline std::pair<T,T> base_obb3<T>::full_intersect() const              //
//=====================================================================================//
template<typename T>
inline std::pair<T,T> base_obb3<T>::full_intersect(const ray_type &ray) const
{
	typedef std::pair<T,T> pair_t;
	const T bad = bad_result();
	const T tzero = traits_type::get_null();
	const pair_t no_result(bad,bad);

	if(is_degenerated()) return no_result;
	if(ray.direction == vector_type(tzero,tzero,tzero)) return no_result;

	const number_type epsilon = static_cast<number_type>(1e-5);
	const number_type inv_epsilon = static_cast<number_type>(1e+5);
	const point_type co = m_origin + x() + y() + z();

	number_type tmax = std::numeric_limits<number_type>::max();
	number_type tmin = -std::numeric_limits<number_type>::max();

	for(unsigned i = 0; i < 3; ++i)
	{
		if(traits_type::abs(m_factor[i]) < epsilon) continue;

		const number_type ddot = m_x[i].dot(ray.direction);
		if(traits_type::abs(ddot) < epsilon) continue;
		
		const number_type invd = traits_type::get_one()/ddot;
		const number_type d = ray.origin.dot(m_x[i]);
		const number_type d1 = m_origin.dot(m_x[i]);
		const number_type d2 = co.dot(m_x[i]);

		const number_type t1 = (d1-d)*invd;
		const number_type t2 = (d2-d)*invd;

		tmin = std::max(tmin,std::min(t1,t2));
		tmax = std::min(tmax,std::max(t1,t2));

		if(tmin > tmax) return no_result;
		if(tmax < traits_type::get_null()) return no_result;
	}

	return pair_t(tmin,tmax);
}

template<typename T>
inline algebra::base_vector3<T> _make_perp(const algebra::base_vector3<T> &a,
										   const algebra::base_vector3<T> &b)
{
	const T epsilon = static_cast<T>(1e-5);
	typedef algebra::numerical_traits<T> nt;

	if(nt::abs(a.dot(b)) >= epsilon)
	{
		return a.cross(b).cross(a).normalize();
	}
	else
	{
		return b;
	}
}

template<typename T>
inline algebra::base_vector3<T> _make_perp(const algebra::base_vector3<T> &a,
										   const algebra::base_vector3<T> &b,
										   const algebra::base_vector3<T> &c)
{
	const T epsilon = static_cast<T>(1e-5);
	typedef algebra::numerical_traits<T> nt;

	if(nt::abs(a.dot(c)) >= epsilon || nt::abs(b.dot(c)) >= epsilon)
	{
		return a.cross(b).normalize();
	}
	else
	{
		return c;
	}
}

//=====================================================================================//
//                   inline base_obb3<T> &base_obb3<T>::operator*=()                   //
//=====================================================================================//
template<typename T>
inline base_obb3<T> &base_obb3<T>::operator*=(const matrix_type &mtr)
{
	if(is_degenerated()) return *this;

	vector_type _x[3] = { m_x[0]*mtr, m_x[1]*mtr, m_x[2]*mtr };
	const point_type o = m_origin*mtr;

	vector_type _nx[3] = { vector_type(_x[0]).normalize(),
						   vector_type(_x[1]).normalize(),
						   vector_type(_x[2]).normalize() };

	_nx[1] = _make_perp(_nx[0],_nx[1]);
	_nx[2] = _make_perp(_nx[0],_nx[1],_nx[2]);

	const number_type _lng[3] = { m_factor[0], m_factor[1], m_factor[2] };

	shrink();
	m_x[0] = _nx[0];
	m_x[1] = _nx[1];
	m_x[2] = _nx[2];
	m_origin = o;

	_x[0] *= _lng[0];
	_x[1] *= _lng[1];
	_x[2] *= _lng[2];

	grow(o);
	grow(o+_x[0]);
	grow(o+_x[1]);
	grow(o+_x[2]);
	grow(o+_x[0]+_x[1]);
	grow(o+_x[0]+_x[2]);
	grow(o+_x[1]+_x[2]);
	grow(o+_x[0]+_x[1]+_x[2]);

	return *this;
}

//=====================================================================================//
//                 inline base_obb3<T> base_obb3<T>::operator*() const                 //
//=====================================================================================//
template<typename T>
inline base_obb3<T> operator*(const base_obb3<T> &obb, const typename base_obb3<T>::matrix_type &mtr)
{
	base_obb3<T> result(obb);
	result *= mtr;
	return result;
}

//=====================================================================================//
//                 inline base_obb3<T> base_obb3<T>::operator*() const                 //
//=====================================================================================//
template<typename T>
inline base_obb3<T> operator*(const base_aabb3<T> &aabb, const typename base_obb3<T>::matrix_type &mtr)
{
    base_obb3<T> result(aabb);
    result *= mtr;
    return result;
}

//=====================================================================================//
//                           friend base_obb3<T> operator*()                           //
//=====================================================================================//
template<typename T>
inline base_obb3<T> operator*(const typename base_obb3<T>::matrix_type &mtr, const base_obb3<T> &obb)
{
	base_obb3<T> result(obb);
	result *= mtr;
	return result;
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
template<typename T>
inline io::obinstream& operator<<(io::obinstream &o, const base_obb3<T> &box)
{
	o << box.is_degenerated();
	if (!box.is_degenerated())
	{
		o<<box.x()<<box.y()<<box.z()<<box.origin();
	}
	
	return o;
}
//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
template<typename T>
inline io::ibinstream& operator>>(io::ibinstream &i, base_obb3<T> &box)
{
	box.degenerate();
	typename base_obb3<T>::point_type p;
	typename base_obb3<T>::vector_type vx,vy,vz;
	bool degenerated = false;
	i>>degenerated;
	if (!degenerated)
	{
		i>>vx>>vy>>vz>>p;
		box = base_obb3<T>(vx,vy,vz,p);
	}
	return i;
}

typedef base_obb3<float> obb3;
typedef base_obb3<float> obb3f;

}	// namespace 'geometry'
}	// namespace 'mll'

#endif // !defined(__OBB3_H_INCLUDED_7552370787048739__)