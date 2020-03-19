#if !defined(__AABB3_INCLUDED__)
#define __AABB3_INCLUDED__

#include <utility>
#include <mll/algebra/vector3.h>
#include <mll/algebra/point3.h>
#include <mll/geometry/ray3.h>
#include <mll/io/binstream.h>
#include <limits>

namespace mll
{
namespace geometry
{

///обрамляющий параллелепипед.
/**
	Этот класс реализует обрамляющий параллелепипед, выровненный по осям X, Y и Z.
	\param T тип координаты
*/
template<typename T>
class base_aabb3
{
public:
	typedef algebra::base_vector3<T> vector_type;
	typedef algebra::base_point3<T> point_type;
	typedef base_ray3<T> ray_type;

	enum quadrant_e
	{
		right=0,
		left=1,
		middle=2
	};

	enum classify_result_type
	{
		crt_outside,
		crt_inside,
		crt_intersect
	};

private:
	bool m_degenerated;			///< флаг вырожденности
	point_type m_min;           ///< точка с минимальными координатами, входящими в aabb3
	point_type m_max;           ///< точка с максимальными координатами, входящими в aabb3

public:
	base_aabb3() : m_degenerated(true) {}

	///конструирование по указанным граничным координатам
	base_aabb3(T minx, T miny, T minz, T maxx, T maxy, T maxz)
									:	m_degenerated(false),
									m_min(std::min(minx,maxx),std::min(miny,maxy),std::min(minz,maxz)),
									m_max(std::max(minx,maxx),std::max(miny,maxy),std::max(minz,maxz))
												{}

///конструирование по двум угловым точкам
	/**
	порядок точек не важен. главное, чтобы они располагались на диагонали.
	*/
	base_aabb3(const point_type &minpt, const point_type &maxpt)
									:	m_degenerated(true)
									{grow(minpt).grow(maxpt);}

///конструирование по точке и диагонали.
	base_aabb3(const point_type &minpt, const vector_type &diag)
									:	m_degenerated(true)
									{grow(minpt).grow(minpt+diag);}

///конструктор копирования
	base_aabb3(const base_aabb3 &t)
									:	m_degenerated(t.m_degenerated),
										m_min(t.m_min),
										m_max(t.m_max) {}

	/// проверка на вырожденность
	bool is_degenerated() const { return m_degenerated; }
	// делает aabb3 вырожденным
	void degenerate() { m_degenerated = true; }
	/// расширяет aabb3 до заданной точки
	base_aabb3& grow(const point_type &p)
		{
		if(is_degenerated())
			{
			m_min=m_max=p;
			m_degenerated = false;
			return *this;
			}
		m_min.x = std::min(m_min.x,p.x);
		m_min.y = std::min(m_min.y,p.y);
		m_min.z = std::min(m_min.z,p.z);
		m_max.x = std::max(m_max.x,p.x);
		m_max.y = std::max(m_max.y,p.y);
		m_max.z = std::max(m_max.z,p.z);
		return *this;
		}
	/// расширяет aabb3, чтобы он включал в себя b
	base_aabb3& grow(const base_aabb3 &b)
		{
		if(is_degenerated())
			{
			*this = b;
			return *this;
			}
		m_min.x = std::min(m_min.x,b.m_min.x);
		m_min.y = std::min(m_min.y,b.m_min.y);
		m_min.z = std::min(m_min.z,b.m_min.z);
		m_max.x = std::max(m_max.x,b.m_max.x);
		m_max.y = std::max(m_max.y,b.m_max.y);
		m_max.z = std::max(m_max.z,b.m_max.z);
		return *this;
		}
	///получение точки с минимальными координатами
	point_type min() const { return m_min; }
	///получение точки с максимальными координатами
	point_type max() const { return m_max; }
	///получение диагонали
	vector_type diag() const { return m_max - m_min; }
	/// получить центр бокса
	point_type center() const { return (m_max-algebra::origin+m_min)*0.5f; }

	/// проверка, содержится ли точка внутри aabb3
	bool inside(const point_type &pt) const
	{
		if( pt.x >= m_min.x && pt.x <= m_max.x &&
			pt.y >= m_min.y && pt.y <= m_max.y &&
			pt.z >= m_min.z && pt.z <= m_max.z) return true;
		return false;
	}

	/// возвращает i-ю угловую точку aabb3
	point_type operator[](int i) const;

	/// проверка, пересекаются ли боксы

    bool intersect_triangle(const point_type &, const point_type &, const point_type &) const;

	classify_result_type classify(const base_aabb3 &aabb3) const
	{
		if ( m_min.x >= aabb3.m_max.x ) return crt_outside;
		if ( m_min.y >= aabb3.m_max.y ) return crt_outside;
		if ( m_min.z >= aabb3.m_max.z ) return crt_outside;
		if ( m_max.x <= aabb3.m_min.x ) return crt_outside;
		if ( m_max.y <= aabb3.m_min.y ) return crt_outside;
		if ( m_max.z <= aabb3.m_min.z ) return crt_outside;

		if ( aabb3.m_min.x >= m_min.x &&
			 aabb3.m_min.y >= m_min.y &&
			 aabb3.m_min.z >= m_min.z &&
			 aabb3.m_max.x <= m_max.x &&
			 aabb3.m_max.y <= m_max.y &&
			 aabb3.m_max.z <= m_max.z ) return crt_inside;

		return crt_intersect;
	}


	/// проверка на пересечение луча с aabb3
	/**
	 возвращает пару точек в координатах луча
	 ВНИМАНИЕ: отрицательные координаты указывают на отсутствие пересечения
	*/
	std::pair<T,T> full_intersect(const ray_type &ray) const;

	/// возвращает координату ближайшего пересечения луча ray с aabb3 в координатах луча
	T intersect(const ray_type &ray) const
	{
		std::pair<T,T> t(full_intersect(ray));
		if(t.first < algebra::numerical_traits<T>::get_null()) return t.second;
		return t.first;
	}

public:
	static T bad_result()
	{
		return mll::algebra::numerical_traits<T>::get_bad_ray_intersection();
	}

private:
	T intersect(bool inside,point_type &candidate,quadrant_e *quadrant,const ray_type &ray) const;
};

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// base_aabb3<T>::operator[]() //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline typename base_aabb3<T>::point_type base_aabb3<T>::operator[](int i) const
{
	const T x = ((i&2)>>1)^(i&1) ? m_max.x : m_min.x;
	const T y = i&2 ? m_max.y : m_min.y;
	const T z = i&4 ? m_max.z : m_min.z;

	return point_type(x,y,z);
}

template<typename T>
inline T base_aabb3<T>::intersect(bool inside,point_type &candidate,quadrant_e *quadrant,const ray_type &ray) const
{
	T max=-1;
	int whichPlane = 0;
	vector_type vec;
	// Calculate T distances to candidate planes
	if (inside)
	{
		for (int i=0; i<3; ++i)
		{
			if (quadrant[i] != middle && ray.direction.v[i] !=0)
			{
				float max_new = (candidate.v[i]-ray.origin.v[i]) / ray.direction.v[i];
				if (max==-1 || max_new < max)
				{
					max = max_new;
					whichPlane = i;
				}
			}
		}
	}
	else
	{
		for (int i=0; i<3; ++i)
		{
			if (quadrant[i] != middle && ray.direction.v[i] !=0)
			{
				//max[i] = (candidate.v[i]-ray.origin.v[i]) / ray.direction.v[i];
				float max_new = (candidate.v[i]-ray.origin.v[i]) / ray.direction.v[i];
				if (max_new > max)
				{
					// Get largest of the maxT's for final choice of intersection
					max = max_new;
					whichPlane = i;
				}
			}
		}
	}
	// Check final candidate actually inside box
	if (max < 0)
		return -1;

	for (int i=0; i<3; i++)
	{
		if (whichPlane != i)
		{
			vec.v[i] = max * ray.direction.v[i];
			T c = ray.origin.v[i] + vec.v[i];
			if (c < m_min.v[i] || c > m_max.v[i])
				return -1;
		}
		else
		{
			vec.v[i] = max * ray.direction.v[i];
		}
	}
	return mll::algebra::numerical_traits<T>::sqrt(vec.sqr_length()/ray.direction.sqr_length());
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// base_aabb3<T>::full_intersect() ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline std::pair<T,T> base_aabb3<T>::full_intersect(const ray_type &ray) const
{
	bool inside = true;
	point_type candidate_min;
	point_type candidate_max;
	quadrant_e quadrant[3] = {middle,middle,middle};
	std::pair<T,T> res(-1,-1);
	// Find candidate planes; this loop can be avoided if rays cast all from the eye(assume perpsective view)
	for (int i=0; i<3; ++i)
	{
		if(ray.origin.v[i] < m_min.v[i])
		{
			quadrant[i] = left;
			candidate_min.v[i] = m_min.v[i];
			candidate_max.v[i] = m_max.v[i];
			inside = false;
		}
		else if (ray.origin.v[i] > m_max.v[i])
		{
			quadrant[i] = right;
			candidate_min.v[i] = m_max.v[i];
			candidate_max.v[i] = m_min.v[i];
			inside = false;
		}
	}
	if(inside)
	{
		// Ray origin inside bounding box
		for (int i=0;i<3;++i)
		{
			if (ray.direction.v[i]<0)
			{
				candidate_min.v[i] = m_min.v[i];
				quadrant[i] = right;
			}
			else if (ray.direction.v[i]>0)
			{
				candidate_min.v[i] = m_max.v[i];
				quadrant[i] = left;
			}
		}
		res.first = intersect(true,candidate_min,&quadrant[0],ray);
		return res;
	}
	else
	{
		res.first = intersect(false,candidate_min,&quadrant[0],ray);
		if (res.first<0) 
			return res;
		res.second = intersect(true,candidate_max,&quadrant[0],ray);
		return res;
	}
}

//=====================================================================================//
//                      bool base_aabb3<T>::intersect_triangle()                       //
//=====================================================================================//
template<typename T>
bool base_aabb3<T>::intersect_triangle(const point_type &a, const point_type &b, const point_type &c) const
{
	typedef mll::algebra::numerical_traits<T> traits;

	const T half = traits::inverse(traits::get_one()+traits::get_one());
	const T zero = traits::get_null();
	const T one = traits::get_one();

	// расстояние от pnt1 до центра OBB
	const vector_type dist = a - center();
	const vector_type dg = half*diag();

	// стороны треугольника
	const vector_type edges[3] = {b - a, c - b, a - c};

	// раннее отсечение треугольника по сфере
	{
		// нормаль треугольника
		const vector_type n = edges[0].cross(edges[1]);
		const T n_dot_dist = n.dot(dist);
		if(n_dot_dist*n_dot_dist > dg.sqr_length()*n.sqr_length()) return false;
	}

	// проверка aabb3 треугольника на пересечение
	{
		base_aabb3 triaabb;
		triaabb.grow(a).grow(b).grow(c);
		
		switch(classify(triaabb))
		{
			case crt_inside: return true;
			case crt_outside: return false;
		}
	}

	//const int next_edge[3] = { 1, 2, 0 };
	//const int adj_comps[3][2] = { {1,2}, {0,2}, {0,1} };

	//// "честная" проверка
	//for(int j = 0; j < 3; ++j)
	//{
	//	vector_type orth = vector_type(zero,zero,zero);
	//	orth.v[j] = one;

	//	for(int i = 0; i < 3; ++i)
	//	{
	//		const int ac1 = adj_comps[i][0], ac2 = adj_comps[i][1];

	//		const vector_type n = orth.cross(edges[i]);
	//		const T p = n.dot(dist);
	//		const T d0 = n.dot(edges[next_edge[i]]);

	//		const T r = dg[ac1] * traits::abs(edges[i][ac2])
	//			      + dg[ac2] * traits::abs(edges[i][ac1]);

	//		if(std::min(p,p+d0) > r || std::max(p,p+d0) < -r) return false;
	//	}
	//}

	return true;
}

typedef base_aabb3<float> aabb3;
typedef base_aabb3<float> aabb3f;


//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
inline io::obinstream& operator<<(io::obinstream &o, const aabb3 &box)
{
	o<<box.min()<<box.max();
	return o;
}
//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
inline io::ibinstream& operator>>(io::ibinstream &i, aabb3 &box)
{
	box.degenerate();
	mll::algebra::point3 p1,p2;
	i>>p1>>p2;
	box.grow(p1).grow(p2);
	return i;
}

} // namespace 'geometry'
} // namespace 'mll'


#endif