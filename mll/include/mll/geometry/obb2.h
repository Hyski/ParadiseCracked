#pragma once

#include <mll/algebra/vector2.h>
#include <mll/algebra/point2.h>
#include <mll/geometry/aabb2.h>
#include <mll/geometry/obb3.h>
#include <mll/algebra/matrix3.h>
#include <mll/algebra/numerical_traits.h>

namespace mll
{
namespace geometry
{

template<typename T>
class base_obb2
{
public:
	typedef algebra::numerical_traits<T> traits_type;
	typedef algebra::base_vector2<T> vector_type;
	typedef algebra::base_vector3<T> vector_type3;
	typedef algebra::base_point2<T> point_type;
	typedef algebra::base_point3<T> point_type3;
	typedef algebra::base_matrix3<T> matrix_type;
	typedef base_ray2<T> ray_type;
	typedef T number_type;

	enum
	{
		ptf_x		=	1<<0,
		ptf_y		=	1<<1,

		pt_origin	=	0,
		pt_x		=	ptf_x,
		pt_xy		=	ptf_x|ptf_y,
		pt_y		=	ptf_y,

		pt_count	=	4
	};

private:

	bool m_degenerated;
	vector_type m_x[2];
	number_type m_factor[2];
	point_type m_origin;

public:
	base_obb2() : m_degenerated(true) {}
	base_obb2(const vector_type &, const vector_type &, const point_type &);
	base_obb2(const base_obb2<T> &);
	base_obb2(const base_obb3<T> &);
	base_obb2(const base_aabb2<T> &);

	point_type operator[](unsigned) const;
	bool is_degenerated() const { return m_degenerated; }
	void degenerate() { m_degenerated = true; }

	base_obb2 &grow(const point_type &);
	const vector_type &x_dir() const { return m_x[0]; }
	const vector_type &y_dir() const { return m_x[1]; }
	number_type x_inv_length() const { return traits_type::get_one()/m_factor[0]; }
	number_type y_inv_length() const { return traits_type::get_one()/m_factor[1]; }
	point_type origin() const { return m_origin; }
	vector_type x() const { return m_x[0] * m_factor[0]; }
	vector_type y() const { return m_x[1] * m_factor[1]; }
	bool intersect(const base_aabb2<T>& aabb) const;
	T intersect(const ray_type &ray) const;
	/// Имеет пересечение с отрезком луча.
	bool intersects(const ray_type &ray) const;

	bool inside(const point_type &) const;

private:

	static T bad_result() {return mll::algebra::numerical_traits<T>::get_bad_ray_intersection();}
	static void _grow(const point_type &, number_type &, vector_type &, point_type &);
	static number_type _validate(vector_type &, const vector_type &);
	void _construct(const vector_type &x, const vector_type &y);
	static vector_type _perp(const vector_type &);

	bool _is_valid() const;
	
	bool _is_overlap(const vector_type& axis, 
					 const vector_type& diag1, 
					 const vector_type& diag1n, 
					 const vector_type& diag2, 
					 const vector_type& diag2n, 
					 const point_type& center1, 
					 const point_type& center2) const;


};

template<typename T>
inline base_obb2<T>::base_obb2(const vector_type &x, const vector_type &y,
							   const point_type &o)
:	m_origin(o), 
	m_degenerated(false)
{
	_construct(x,y);
	assert( _is_valid() );
}


template<typename T>
inline base_obb2<T>::base_obb2(const base_obb2 &rhs)
:	m_degenerated(rhs.m_degenerated)
{
	if(!is_degenerated())
	{
		m_origin = rhs.m_origin;
		for(unsigned i = 0; i < 2; ++i)
		{
			m_x[i] = rhs.m_x[i];
			m_factor[i] = rhs.m_factor[i];
		}
	}
}

template<typename T>
inline base_obb2<T>::base_obb2(const base_obb3<T> &src)
{
	point_type3& o = src.origin();
	vector_type3& x = src.x();
	vector_type3& y = src.y();
	vector_type3& z = src.z();

	point_type3 p3[8];
	p3[0] = o;
	p3[1] = o+x;
	p3[2] = o+y;
	p3[3] = o+z;
	p3[4] = o+x+y;
	p3[5] = o+x+z;
	p3[6] = o+y+z;
	p3[7] = o+x+y+z;

	point2 p2[8];
	p2[0] = point2(p3[0].x, p3[0].y);
	p2[1] = point2(p3[1].x, p3[1].y);
	p2[2] = point2(p3[2].x, p3[2].y);
	p2[3] = point2(p3[3].x, p3[3].y);
	p2[4] = point2(p3[4].x, p3[4].y);
	p2[5] = point2(p3[5].x, p3[5].y);
	p2[6] = point2(p3[6].x, p3[6].y);
	p2[7] = point2(p3[7].x, p3[7].y);

	vector2 side1(p2[1] - p2[0]); 
	vector2 side2(-side1.y, side1.x); 
	{
		float dist = abs((p2[2].x-p2[0].x)*(p2[1].y-p2[0].y)-(p2[2].y-p2[0].y)*(p2[1].x-p2[0].x))/
					 sqrt((p2[1].x-p2[0].x)*(p2[1].x-p2[0].x)+(p2[1].y-p2[0].y)*(p2[1].y-p2[0].y));

		side2.normalize();
		side2 = side2 *  dist;
	}
	obb2 dst(side1, side2, p2[0]);
	//dst.grow(p2[1]);
	//dst.grow(p2[2]);
	dst.grow(p2[3]);
	dst.grow(p2[4]);
	dst.grow(p2[5]);
	dst.grow(p2[6]);
	dst.grow(p2[7]);

	*this = dst;
}

template<typename T>
inline base_obb2<T>::base_obb2(const base_aabb2<T> &rhs)
:	m_degenerated(rhs.is_degenerated())
{
	if(!is_degenerated())
	{
		const T tzero = traits_type::get_null();
		const T tone = traits_type::get_one();
		const vector_type diag = rhs.diag();

		m_origin = rhs.min();
		m_x[0] = vector_type(tone,tzero);
		m_x[1] = vector_type(tzero,tone);

		m_factor[0] = diag.x;
		m_factor[1] = diag.y;
	}

	assert( _is_valid() );
}

template<typename T>
inline void base_obb2<T>::_construct(const vector_type &x, const vector_type &y)
{
	m_x[0] = x;
	m_x[1] = y;

	m_factor[0] = _validate(m_x[0], m_x[1]);
	m_factor[1] = _validate(m_x[1], m_x[0]);
}

template<typename T>
inline bool base_obb2<T>::inside(const point_type &pt) const
{
	if(is_degenerated()) return false;

	const vector_type tv = pt-m_origin;

	const number_type dotx = tv.dot(x_dir())*x_inv_length();
	if( dotx > traits_type::get_one() || dotx < traits_type::get_null() ) return false;

	const number_type doty = tv.dot(y_dir())*y_inv_length();
	if( doty > traits_type::get_one() || doty < traits_type::get_null() ) return false;

	return true;
}

template<typename T>
inline base_obb2<T> &base_obb2<T>::grow(const point_type &pt)
{
	if(is_degenerated())
	{
		const T tzero = traits_type::get_null();
		const vector_type zero(tzero,tzero);
		m_x[0] = m_x[1] = zero;
		m_factor[0] = _validate(m_x[0], zero);
		m_factor[1] = _validate(m_x[1], m_x[0]);
		m_origin = pt;
		m_degenerated = false;
	}
	else
	{
		for(unsigned i = 0; i < 2; ++i) _grow(pt,m_factor[i],m_x[i],m_origin);
	}

	assert( _is_valid() );
	assert( _is_valid() );

	return *this;
}

template<typename T>
inline typename base_obb2<T>::number_type base_obb2<T>::_validate(	vector_type &x, const vector_type &y)
{
	const number_type epsilon = static_cast<number_type>(1e-5);

	
	if(x.length() == traits_type::get_null())
	{
		const vector_type newx = _perp(y).normalize();
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

template<typename T>
inline void base_obb2<T>::_grow(const point_type &pt, number_type &f, vector_type &x, point_type &o)
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

template<typename T>
inline typename base_obb2<T>::vector_type base_obb2<T>::_perp(const vector_type &v)
{
	const number_type lv = v.sqr_length();

	if(lv == traits_type::get_null())
	{
		return vector_type(traits_type::get_one(),traits_type::get_null());
	}

	return vector_type(-v.y, v.x);
}

//=====================================================================================//
//          inline base_obb3<T>::point_type base_obb3<T>::operator[]() const           //
//=====================================================================================//
template<typename T>
inline typename base_obb2<T>::point_type base_obb2<T>::operator[](unsigned i) const
{
	const T tzero = traits_type::get_null();
	const vector_type zero(tzero,tzero);
	return m_origin +
		(i&ptf_x ? x() : zero) +
		(i&ptf_y ? y() : zero);
}


template<typename T>
inline bool base_obb2<T>::_is_valid() const
{
	if(is_degenerated()) return true;

	const T one = traits_type::get_one();
	const T epsilon = static_cast<T>(1e-5);

	for(unsigned i = 0; i < 2; ++i)
	{
		if( traits_type::abs(m_x[i].sqr_length() - one) > epsilon ) return false;
	}

	if( traits_type::abs(m_x[0].dot(m_x[1])) > epsilon ) return false;

	return true;
}

template<typename T>
inline bool base_obb2<T>::_is_overlap(	const vector_type& axis, 
								 		const vector_type& diag1, 
										const vector_type& diag1n, 
										const vector_type& diag2, 
										const vector_type& diag2n, 
										const point_type& center1, 
										const point_type& center2) const
{
	/// проецируем стороны на ось
	vector_type side1proj;
	{
		float dp1 = diag1.dot(axis);
		float dp2 = diag1n.dot(axis);

		float dp = (fabsf(dp1) > fabsf(dp2))?dp1:dp2;
		side1proj = vector_type(dp*axis.x, dp*axis.y);
	}

	vector_type side2proj;
	{
		float dp1 = diag2.dot(axis);
		float dp2 = diag2n.dot(axis);

		float dp = (fabsf(dp1) > fabsf(dp2))?dp1:dp2;
		side2proj = vector_type(dp*axis.x, dp*axis.y);
	}

	/// проецируем центры боксов на ось (полагаем что ось проходит через ценрт первого бокса)
	const point_type& center1proj = center1;

	point_type center2proj;

	{
		vector_type v1 = center2 - center1;
		vector_type v2 = axis;
		float t = v2.dot(v1);

		center2proj = center1 + v2 * t;
	}

	float cl = (center2proj-center1proj).sqr_length();
	float sl = (side1proj.length() + side2proj.length())*(side1proj.length() + side2proj.length()) * 0.25f;

	return cl < sl;

}

template<typename T>
inline bool base_obb2<T>::intersect(const base_aabb2<T>& aabb) const
{
	vector_type axes[4]; 

	axes[0] = vector_type(1.0f, 0.0f);
	axes[1] = vector_type(0.0f, 1.0f);
	axes[2] = vector_type(x()).normalize();
	axes[3] = vector_type(y()).normalize();

	vector_type& diag1 = aabb.diag();
	vector_type diag1n = vector_type(-diag1.y, diag1.x);
	vector_type diag2 = x() + y();
	vector_type diag2n = x() - y();
	point_type center1 = aabb.min()+diag1*0.5f;
	point_type center2 = origin() + diag2*0.5f;

	for(size_t i = 0; i != 4; ++i)
	{
		if(!_is_overlap(axes[i], diag1, diag1n, diag2, diag2n, center1, center2))
		{
			return false;
		}
	}

	return true;
}

//=====================================================================================//
//             inline std::pair<T,T> base_obb3<T>::full_intersect() const              //
//=====================================================================================//
template<typename T>
inline T base_obb2<T>::intersect(const ray_type &ray) const
{
	const T tzero = traits_type::get_null();
	const number_type no_result = -1.0f;

	if(is_degenerated() || ray.direction == vector_type(tzero,tzero))
	{
		return no_result;
	}

	point_type sides[4][2] = 
	{ 
		{ m_origin, m_origin+x() },
		{ m_origin+x(), m_origin+x()+y() },
		{ m_origin+x()+y(), m_origin+y() },
		{ m_origin+y(), m_origin }
	};

	const number_type& x21 = ray.origin.x; 
	const number_type& y21 = ray.origin.y;  
	const number_type& x22 = ray.origin.x + ray.direction.x; 
	const number_type& y22 = ray.origin.y + ray.direction.y;
	T dx2 = x22-x21, dy2 = y22-y21; // Длина проекций второй линии на ось x и y

	for(size_t i = 0; i < 4; ++i)
	{
		const number_type& x11 = sides[i][0].x; 
		const number_type& y11 = sides[i][0].y; 
		const number_type& x12 = sides[i][1].x; 
		const number_type& y12 = sides[i][1].y; 

		T dx1 = x12-x11, dy1 = y12-y11; // Длина проекций первой линии на ось x и y
		T dxx = x11-x21, dyy = y11-y21;
		T div, mul, ub;

		if ((div = (dy2*dx1-dx2*dy1)) <= 0.0f) 
		{
			continue;
		}
		else
		{
			if ((mul = (float)(dx2*dyy-dy2*dxx)) < 0.0f || mul > div)
			{
				continue; // Первый отрезок пересекается за своими границами...
			}
			if ((mul = (float)(dx1*dyy-dy1*dxx)) < 0.0f)
			{
				continue; // Луч пересекается раньше своего оригина...
			}

			ub = mul / div;
		}

		point_type sect(x21 + (x22-x21) * ub, y21 + (y22-y21) * ub);

		return (sect - ray.origin).length() / ray.direction.length(); 
	}

	return no_result;
}

//=====================================================================================//
//                     inline bool base_obb2<T>::intersects() const                    //
//=====================================================================================//
template<typename T>
inline bool base_obb2<T>::intersects(const ray_type &ray) const
{
	const T tzero = traits_type::get_null();
	
	if(is_degenerated() || ray.direction == vector_type(tzero,tzero))
	{
		return false;
	}

	point_type sides[4][2] = 
	{ 
		{ m_origin, m_origin+x() },
		{ m_origin+x(), m_origin+x()+y() },
		{ m_origin+x()+y(), m_origin+y() },
		{ m_origin+y(), m_origin }
	};

	const number_type& x21 = ray.origin.x; 
	const number_type& y21 = ray.origin.y;  
	const number_type& x22 = ray.origin.x + ray.direction.x; 
	const number_type& y22 = ray.origin.y + ray.direction.y;
	T dx2 = x22-x21, dy2 = y22-y21; // Длина проекций второй линии на ось x и y

	for(size_t i = 0; i < 4; ++i)
	{
		const number_type& x11 = sides[i][0].x; 
		const number_type& y11 = sides[i][0].y; 
		const number_type& x12 = sides[i][1].x; 
		const number_type& y12 = sides[i][1].y; 

		T dx1 = x12-x11, dy1 = y12-y11; // Длина проекций первой линии на ось x и y
		T dxx = x11-x21, dyy = y11-y21;
		T div, mul, ub;

		if ((div = (dy2*dx1-dx2*dy1)) <= 0.0f) 
		{
			continue;
		}
		else
		{
			if ((mul = (float)(dx2*dyy-dy2*dxx)) < 0.0f || mul > div)
			{
				continue; // Первый отрезок пересекается за своими границами...
			}
			if ((mul = (float)(dx1*dyy-dy1*dxx)) < 0.0f)
			{
				continue; // Луч пересекается раньше своего оригина...
			}

			ub = mul / div;
		}

		point_type sect(x21 + (x22-x21) * ub, y21 + (y22-y21) * ub);

		return ((sect - ray.origin).sqr_length() < ray.direction.sqr_length()); 
	}

	return false;
}


//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
template<typename T>
inline io::obinstream& operator<<(io::obinstream &o, const base_obb2<T> &box)
{
	o << box.is_degenerated();
	if (!box.is_degenerated())
	{
		o<<box.x()<<box.y()<<box.origin();
	}

	return o;
}
//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
template<typename T>
inline io::ibinstream& operator>>(io::ibinstream &i, base_obb2<T> &box)
{
	box.degenerate();
	typename base_obb2<T>::point_type p;
	typename base_obb2<T>::vector_type vx,vy;
	bool degenerated = false;
	i>>degenerated;
	if (!degenerated)
	{
		i>>vx>>vy>>p;
		box = base_obb2<T>(vx,vy,p);
	}
	return i;
}


typedef base_obb2<float> obb2;

}
}