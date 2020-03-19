#if !defined(__FRUSTUM_INCLUDED__)
#define __FRUSTUM_INCLUDED__

/////////////////////////////////////////////////////////////////////////
//	Standard library files
#include <assert.h>
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//	MiST land Library files
#include <mll/algebra/matrix3.h>
#include <mll/algebra/point3.h>
#include <mll/geometry/plane.h>
#include <mll/geometry/aabb3.h>
#include <mll/geometry/obb3.h>
/////////////////////////////////////////////////////////////////////////

namespace mll 
{ 

namespace geometry
{

//=====================================================================================//
//                                 class base_frustum                                  //
//
// этот класс представляет объем, видимый камерой
// направление взора совпадает с положительным направлением оси Z
// горизонтальная составляющая - X
// вертикальная составляющая - Y
//
//=====================================================================================//
template<class T>
class base_frustum
{
public:
	typedef mll::algebra::base_matrix3<T> matrix;

	typedef mll::algebra::base_point3<T> point;
	typedef mll::geometry::base_aabb3<T> aabb;
	typedef mll::geometry::base_obb3<T> obb;

	enum classify_result_type
	{
		outside		= 0,
		inside		= 1,
		intersect	= 2
	};	 //	можно использовать (bool)classify_result_type чтобы узнать снаружи ли объект

public:
	base_frustum();
	base_frustum(const base_frustum<T> &f);
	base_frustum(T znear,T zfar,T hfov,T aspect);
	~base_frustum();
public:
	//	оператор копирования
	base_frustum<T>& operator=(const base_frustum<T>& f);
	//	установить позицию фрастума (как вариант передавать инвертированую матрицу вида)
	void set_pos(const mll::algebra::base_matrix3<T>& world);
	//	определить местоположение точки относительно фрастума
	classify_result_type classify(const mll::algebra::base_point3<T>& pt) const;
	//	определить местоположение бокса относительно фрастума
	classify_result_type classify(const aabb& box) const;
	//	определить местоположение бокса относительно фрастума
	classify_result_type classify(const obb &p) const;
	//? для отладочных целей, возможно, надо переделать на более красивый варант
	// порядок:
	// near_top_left,near_bottom_left, near_bottom_right, near_top_right
	// far_top_left,far_bottom_left, far_bottom_right, far_top_right
	enum corner_name	// GvozdodeR
	{
		near_top_left		= 0,
		near_bottom_left	= 1,
		near_bottom_right	= 2,
		near_top_right		= 3,
		far_top_left		= 4,
		far_bottom_left		= 5,
		far_bottom_right	= 6,
		far_top_right		= 7,
	};
	mll::algebra::base_point3<T> operator[](unsigned int num) const;

private:
	//	для перевода точек в локальную искаженную систему координат объема (и обратно):
	matrix	m_frustum; 
	matrix	m_frustum_inv; 
	//	для перевода точек в локальную систему координат объема (и обратно):
	matrix	m_pos;
	matrix	m_pos_inv;
	//	для использования
	matrix	m_compound;
	matrix	m_compound_inv;
	//	плоскости
	base_plane<T> m_planes[6];
	//	z-координаты инициализации
	T m_near;
	T m_far;

public:
	const matrix &get_proj() const { return m_frustum; } 
	const matrix &get_proj_inv() const { return m_frustum_inv; }
	const matrix &get_pos() const { return m_pos; }
	const matrix &get_pos_inv() const { return m_pos_inv; }
	const matrix &get_full() const { return m_compound; }
	const matrix &get_full_inv() const { return m_compound_inv; }

private:
	//	проверить на попадание Z координаты
	bool check_z(const mll::algebra::base_point3<T>& pt) const;
	//	проверить на попадание X,Y координат
	bool check_xy(const mll::algebra::base_point3<T>& pt) const;
	//	сравнить бокс с плоскостью
	classify_result_type check_box(const base_plane<T>& p, const aabb& box) const;
	//	сравнить точку с плоскостью
	classify_result_type check_point(const base_plane<T>& p, const mll::algebra::base_point3<T>& point) const;
};

//=====================================================================================//
//                       inline base_frustum<T>::base_frustum()                        //
//=====================================================================================//
template<class T>
inline base_frustum<T>::base_frustum()
{
}

//=====================================================================================//
//                       inline base_frustum<T>::base_frustum()                        //
//=====================================================================================//
template<class T>
inline base_frustum<T>::base_frustum(const base_frustum<T>& f)
{
	*this = f;
}

//=====================================================================================//
//                       inline base_frustum<T>::base_frustum()                        //
//=====================================================================================//
template<class T>
inline base_frustum<T>::base_frustum(T znear,T zfar,T hfov,T aspect)
{
	m_pos = m_pos_inv = mll::algebra::base_matrix3<T>::identity();
	m_frustum = algebra::perspective_lh(znear,zfar,hfov,aspect);
	(m_frustum_inv = m_frustum).invert();

	m_compound = m_frustum;
	m_compound_inv = m_frustum_inv;

	m_near = znear;
	m_far = zfar;
}

template<class T>
inline base_frustum<T>::~base_frustum()
{
}
//=====================================================================================//
//                inline base_frustum<T>& base_frustum<T>::operator=()                 //
//=====================================================================================//
template<class T>
inline base_frustum<T>& base_frustum<T>::operator=(const base_frustum<T>& f)
{
	if(this != &f)
	{
		m_frustum = f.m_frustum;  
		m_frustum_inv = f.m_frustum_inv; 
		m_pos = f.m_pos;
		m_pos_inv = f.m_pos_inv;
		m_compound = f.m_compound;
		m_compound_inv = f.m_compound_inv;

		for(unsigned int i=0;i<6;++i)
			m_planes[i] = f.m_planes[i];

		m_near = f.m_near;
		m_far = f.m_far;
	}

	return *this;
}

//=====================================================================================//
//                      inline void base_frustum<T>::set_pos();                        //
//=====================================================================================//
template<class T>
inline void base_frustum<T>::set_pos(const mll::algebra::base_matrix3<T>& world)
{
	m_pos_inv =  world;
	(m_pos = world).invert();

	m_compound = m_pos*m_frustum;
	m_compound_inv = m_frustum_inv*m_pos_inv;

	const algebra::base_point3<T> points[6] = 
	{
		algebra::base_point3<T>(-1.0f,-1.0f, 0.0f)*m_compound_inv,
		algebra::base_point3<T>(-1.0f, 1.0f, 0.0f)*m_compound_inv,
		algebra::base_point3<T>( 1.0f,-1.0f, 0.0f)*m_compound_inv,
		algebra::base_point3<T>(-1.0f, 1.0f, 1.0f)*m_compound_inv,
		algebra::base_point3<T>( 1.0f,-1.0f, 1.0f)*m_compound_inv,
		algebra::base_point3<T>( 1.0f, 1.0f, 1.0f)*m_compound_inv
	};

	//	плоскость z =  0 
	m_planes[0] = base_plane<T>(points[0],points[2],points[1]);
	//	плоскость z =  1 
	m_planes[1] = base_plane<T>(points[3],points[5],points[4]);
	//	плоскость x = -1 
	m_planes[2] = base_plane<T>(points[0],points[1],points[3]);
	//	плоскость x =  1 
	m_planes[3] = base_plane<T>(points[2],points[4],points[5]);
	//	плоскость y = -1 
	m_planes[4] = base_plane<T>(points[0],points[4],points[2]);
	//	плоскость y =  1 
	m_planes[5] = base_plane<T>(points[1],points[5],points[3]);

}

//=====================================================================================//
//          inline base_frustum<T>::classify_result_type base_frustum<T>::classify()   //
//=====================================================================================//
template<class T>
inline typename base_frustum<T>::classify_result_type base_frustum<T>::classify(const mll::algebra::base_point3<T>& pt) const
{
//	return (check_z(pt) && check_xy(pt))?inside:outside;
	if(check_point(m_planes[0], pt) != outside)
	{
		for(int i = 1; i < 6; ++i)
		{
			const classify_result_type local_result = check_point(m_planes[i], pt);
			if (local_result == outside)
				return outside;
		}
		return inside;
	}
	return outside;
}

//=====================================================================================//
//          inline base_frustum<T>::classify_result_type base_frustum<T>::classify()   //
//=====================================================================================//
template<class T>
inline typename base_frustum<T>::classify_result_type base_frustum<T>::classify(const aabb& box) const
{
	classify_result_type result = check_box(m_planes[0],box);

	if(result != outside)
	{
		for(int i=1;i<6;++i)
		{
			const classify_result_type local_result = check_box(m_planes[i],box);
			if(local_result == outside) return outside;
			if(result != intersect)
				result = local_result;
		}
	}

	return result;
}

//=====================================================================================//
//                             base_frustum<T>::classify()                             //
//=====================================================================================//
template<class T>
inline typename base_frustum<T>::classify_result_type base_frustum<T>::classify(const obb& box) const
{
	classify_result_type result = outside;

	if(!box.is_degenerated())
	{
		const mll::algebra::base_vector3<T> i = box.x_dir();
		const mll::algebra::base_vector3<T> j = box.y_dir();
		const mll::algebra::base_vector3<T> k = box.z_dir();
		const mll::algebra::base_point3<T>  o = box.origin();

		const matrix m(	i.x,	    j.x,		k.x,		0.0f,
			i.y,	    j.y,		k.y,		0.0f,
			i.z,	    j.z,		k.z,		0.0f,
			-o.dot(i),	-o.dot(j),	-o.dot(k),	1.0f	);

		const aabb monic = aabb(0,0,0,box.x_length(),box.y_length(),box.z_length());
		result = check_box(m_planes[0]*m,monic);

		if(result != outside)
		{
			for(int i=1;i<6;++i)
			{
				const classify_result_type local_result = check_box(m_planes[i]*m,monic);
				if(local_result == outside) return outside;
				if(result != intersect)
					result = local_result;
			}
		}
	}

	return result;
}

//=====================================================================================//
//          inline base_frustum<T>::point base_frustum<T>::operator[]() const          //
//=====================================================================================//
template<class T>
inline mll::algebra::base_point3<T> base_frustum<T>::operator[](unsigned int num) const
{
	static const unsigned int points_num = 8;

	static const mll::algebra::base_point3<T> points[points_num] =
	{
		point(-1,-1,0),
		point(-1,1,0),
		point(1,1,0),
		point(1,-1,0),
		point(-1,-1,1),
		point(-1,1,1),
		point(1,1,1),
		point(1,-1,1)
	}; 

	assert( num < points_num );

	return points[num]*m_compound_inv;
}
//	проверить на попадания Z координаты
template<class T>
inline bool base_frustum<T>::check_z(const mll::algebra::base_point3<T>& pt) const
{
	const mll::algebra::numerical_traits<T>::quotient_type pos_denom = pt.x*m_pos._03 + pt.y*m_pos._13 + pt.z*m_pos._23 + m_pos._33;
	const T pt_z = (m_pos._02*pt.x+m_pos._12*pt.y+m_pos._22*pt.z+m_pos._32)/pos_denom;

	return (pt_z<=m_far) && (pt_z>=m_near);
}

template<class T>
inline bool base_frustum<T>::check_xy(const mll::algebra::base_point3<T>& pt) const
{
	const mll::algebra::numerical_traits<T>::quotient_type compound_denom = pt.x*m_compound._03 + pt.y*m_compound._13 + pt.z*m_compound._23 + m_compound._33;
	const T pt_x = (m_compound._00*pt.x+m_compound._10*pt.y+m_compound._20*pt.z+m_compound._30)/compound_denom;
	const T pt_y = (m_compound._01*pt.x+m_compound._11*pt.y+m_compound._21*pt.z+m_compound._31)/compound_denom;

	return pt_x>=-1.0f && pt_y>=-1.0f && pt_x<=1.0f  && pt_y<=1.0f;
}
//	сравнить бокс с плоскостью
template<class T>
inline typename base_frustum<T>::classify_result_type base_frustum<T>::check_box(const base_plane<T>& p,const aabb& box) const
{
	using namespace mll::algebra;

/*	classify_result_type result;

	if(p.classify(box[0]) >= 0) result = inside;
	else result = outside;

	for(int i=1;i<8;++i)
	{
		if(p.classify(box[i]) >= 0)
		{
			if(result == outside)
			{
				result = intersect;
				break;
			}
		//	else result = inside;
		}
		else
		{
			if(result == inside)
			{
				result = intersect;
				break;
			}
		//	else result = outside;
		}
	} 
  return result;  */
	typedef base_vector3<T> vec;
	typedef base_point3<T> pnt;

	unsigned octant = (
		(p.m_normal.x > mll::algebra::numerical_traits<T>::get_null()	? 0x01:0x00)
		| (p.m_normal.y > mll::algebra::numerical_traits<T>::get_null()? 0x02:0x00))
		^ (p.m_normal.z > mll::algebra::numerical_traits<T>::get_null()	? 0x00:0x03);

	 T r1,r2;
	switch(octant)
	{
	case 0:
		r1 = p.classify(  pnt(box.min().x, box.min().y, box.max().z)  );
		r2 = p.classify(  pnt(box.max().x, box.max().y, box.min().z)  );
		break;
	case 1:
		r1 = p.classify(  pnt(box.max().x, box.min().y, box.max().z)  );
		r2 = p.classify(  pnt(box.min().x, box.max().y, box.min().z)  );
		break;
	case 2:
		r1 = p.classify(  pnt(box.min().x, box.max().y, box.max().z)  );
		r2 = p.classify(  pnt(box.max().x, box.min().y, box.min().z)  );
		break;
	case 3:
		r1 = p.classify(  pnt(box.max().x, box.max().y, box.max().z)  );
		r2 = p.classify(  pnt(box.min().x, box.min().y, box.min().z)  );
		break;
	};
	if(r1>=mll::algebra::numerical_traits<T>::get_null() && r2>=mll::algebra::numerical_traits<T>::get_null())
		return inside;
	else if(r1<mll::algebra::numerical_traits<T>::get_null() && r2<mll::algebra::numerical_traits<T>::get_null())
		return outside;
	return intersect;
}
//	сравнить точку с плоскостью
template<class T>
inline typename base_frustum<T>::classify_result_type base_frustum<T>::check_point(const base_plane<T>& p, const mll::algebra::base_point3<T>& point) const
{
	T dist = p.classify(point);
	if (dist >= mll::algebra::numerical_traits<T>::get_null())
		return inside;
	return outside;
}

typedef base_frustum<float> frustumf;
typedef base_frustum<float> frustum;

}	//	namespace geometry

}	//	namespace mll

#endif