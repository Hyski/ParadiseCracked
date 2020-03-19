#if !defined(__AABB2_INCLUDED__)
#define __AABB2_INCLUDED__

#include <mll/algebra/vector2.h>
#include <mll/algebra/point2.h>
#include <mll/geometry/ray2.h>
#include <limits>

namespace mll
{
///набор классов, реализующих геометрические понятия.
namespace geometry
{

///обрамляющий прямоугольник
/**
	Этот класс реализует обрамляющий прямоугольник, выровненный по осям X и Y.
	\param T тип координаты
*/
template<typename T>
class base_aabb2
{
public:
	typedef algebra::base_vector2<T> vector_type;
	typedef algebra::base_point2<T> point_type;
	typedef base_ray2<T> ray_type;

	enum classify_result_type
	{
		crt_outside,
		crt_inside,
		crt_intersect
	};

private:
	bool m_degenerated;		///< флаг вырожденности
	point_type m_min;		///< точка с минимальными координатами, входящими в aabb2
	point_type m_max;		///< точка с максимальными координатами, входящими в aabb2

public:
	base_aabb2() : m_degenerated(true) {}

	///конструирование по указанным граничным координатам
	base_aabb2(T minx, T miny, T maxx, T maxy) : m_degenerated(true)
	{
		grow(point_type(minx,miny));
		grow(point_type(maxx,maxy));
	}

///конструирование по двум угловым точкам
	/**
	порядок точек не важен. главное, чтобы они располагались на диагонали.
	*/
	base_aabb2(const point_type &minpt, const point_type &maxpt) : m_degenerated(true)
	{
		grow(minpt);
		grow(maxpt);
	}
///конструирование по точке и диагонали.
	base_aabb2(const point_type &minpt, const vector_type &diag) : m_degenerated(true)
	{
		grow(minpt);
		grow(minpt+diag);
	}
///конструктор копирования
	base_aabb2(const base_aabb2 &t)
	:	m_degenerated(t.m_degenerated),
		m_min(t.m_min),
		m_max(t.m_max) {}

	/// проверка на вырожденность
	bool is_degenerated() const { return m_degenerated; }
	// делает aabb2 вырожденным
	base_aabb2 &degenerate() { m_degenerated = true; return *this; }

	/// расширяет aabb2 до заданной точки
	base_aabb2 &grow(const point_type &p);
	/// расширяет aabb2, чтобы он включал в себя b
	base_aabb2 &grow(const base_aabb2 &b);

	///получение точки с минимальными координатами
	point_type min() const { return m_min; }
	///получение точки с максимальными координатами
	point_type max() const { return m_max; }
	///получение диагонали
	vector_type diag() const { return m_max - m_min; }

	/// Сдвинуть на вектор.
	base_aabb2 operator + (const vector_type & displacement) const;

	/// проверка, содержится ли точка внутри aabb2
	bool inside(const point_type &pt) const;

	/// возвращает i-ю угловую точку aabb2
	point_type operator[](int i) const;

	classify_result_type intersect_circle(T rad, const point_type &center);

	/// проверка на пересечение луча с aabb2
	/**
	 возвращает пару точек в координатах луча
	 ВНИМАНИЕ: отрицательные координаты указывают на отсутствие пересечения
	*/
	std::pair<T,T> full_intersect(const ray_type &ray) const;

	/// возвращает координату ближайшего пересечения луча ray с aabb2 в координатах луча
	T intersect(const ray_type &ray) const;

	base_aabb2 &operator = (const base_aabb2 &);
	
public:
	static T bad_result()
	{
		return mll::algebra::numerical_traits<T>::get_bad_ray_intersection();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// base_aabb2<T>::inside() ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline bool base_aabb2<T>::inside(const point_type &pt) const
{
	return	pt.x >= m_min.x && pt.x <= m_max.x &&
			pt.y >= m_min.y && pt.y <= m_max.y;
}

//=====================================================================================//
//        base_aabb2<T>::classify_result_type base_aabb2<T>::intersect_circle()        //
//=====================================================================================//
template<typename T>
typename base_aabb2<T>::classify_result_type base_aabb2<T>::intersect_circle(T rad, const point_type &center)
{
   T d(0), a;

   // проходим по осям X,Y
   for (int i=0; i<2; i++)
   {
      // если центр сферы лежит перед AABB,
      if (center.v[i] < m_min.v[i])
      {
         // то вычисляем квадрат расстояния по этой оси
         a = center.v[i] - m_min.v[i];
         d += a * a;
      }

      // если центр сферы лежит после AABB,
      if (center.v[i] > m_max.v[i])
      {
         // то вычисляем квадрат расстояния по этой оси
         a = center.v[i] - m_max.v[i];
         d += a * a;
      }
   }

   return d <= (rad * rad) ? crt_intersect : crt_outside;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// base_aabb2<T>::full_intersect() ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline std::pair<T,T> base_aabb2<T>::full_intersect(const ray_type &ray) const
{
	typedef std::pair<T,T> pair_t;

	const T bad = bad_result();
	const pair_t no_result(bad,bad);
	const T zero = mll::algebra::numerical_traits<T>::get_null();

	typedef mll::algebra::numerical_traits<T> ntraits;

	if(ray.direction.x == zero && ray.direction.y == zero)
	{
		return pair_t(zero,mll::algebra::numerical_traits<T>::get_one());
	}

	if(is_degenerated())
	{
		return no_result;
	}

	if(ray.direction.x == zero)
	{
		if(ray.origin.y < m_min.y || ray.origin.y > m_max.y)
		{
			return no_result;
		}
		else
		{
			T inv = ntraits::inverse(ray.direction.y);
			pair_t p((m_min.y - ray.origin.y)*inv,(m_max.y - ray.origin.y)*inv);

			if(p.first > p.second) std::swap(p.first,p.second);
			return p;
		}
	}
	else if(ray.direction.y == zero)
	{
		if(ray.origin.x < m_min.x || ray.origin.x > m_max.x)
		{
			return no_result;
		}
		else
		{
			T inv = ntraits::inverse(ray.direction.x);
			pair_t p((m_min.x - ray.origin.x)*inv,(m_max.x - ray.origin.x)*inv);

			if(p.first > p.second) std::swap(p.first,p.second);
			return p;
		}
	}

	T invx = ntraits::inverse(ray.direction.x);
	T invy = ntraits::inverse(ray.direction.y);

	T tmin1 = (m_min.x - ray.origin.x) * invx;
	T tmax1 = (m_max.x - ray.origin.x) * invx;

	T tmin2 = (m_min.y - ray.origin.y) * invy;
	T tmax2 = (m_max.y - ray.origin.y) * invy;

	if(tmin1 > tmax1) std::swap(tmin1,tmax1);
	if(tmin2 > tmax2) std::swap(tmin2,tmax2);
	
	tmin1 = std::max(tmin1,tmin2);
	tmax1 = std::min(tmax1,tmax2);

	if(tmax1 < zero || tmin1 > tmax1) return no_result;

	return pair_t(tmin1,tmax1);
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// base_aabb2<T>::intersect() ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline T base_aabb2<T>::intersect(const ray_type &ray) const
{
	std::pair<T,T> t(full_intersect(ray));
	if(t.first < algebra::numerical_traits<T>::get_null()) return t.second;
	return t.first;
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// base_aabb2<T>::operator = () //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline base_aabb2<T> &base_aabb2<T>::operator = (const base_aabb2<T> &t)
{
	m_degenerated = t.m_degenerated;
	m_min = t.m_min;
	m_max = t.m_max;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// base_aabb2<T>::operator[]() //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline typename base_aabb2<T>::point_type base_aabb2<T>::operator[](int i) const
{
	const T x = i&2 ? m_max.x : m_min.x;
	const T y = i&1 ? m_max.y : m_min.y;

	return point_type(x,y);
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// base_aabb2<T>::operator+() ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline base_aabb2<T> base_aabb2<T>::operator + (const vector_type& displacement) const
{
	if (is_degenerated())
	{
		return base_aabb2<T>();
	}
	return base_aabb2<T>(m_min + displacement, m_max + displacement);
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// base_aabb2<T>::grow() /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline base_aabb2<T> &base_aabb2<T>::grow(const base_aabb2 &b)
{
	if(is_degenerated())
	{
		return *this = b;
	}
	m_min.x = std::min(m_min.x,b.m_min.x);
	m_min.y = std::min(m_min.y,b.m_min.y);
	m_max.x = std::max(m_max.x,b.m_max.x);
	m_max.y = std::max(m_max.y,b.m_max.y);
	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// base_aabb2<T>::grow() /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline base_aabb2<T> &base_aabb2<T>::grow(const point_type &p)
{
	if(is_degenerated())
	{
		m_min=m_max=p;
		m_degenerated = false;
		return *this;
	}
	m_min.x = std::min(m_min.x,p.x);
	m_min.y = std::min(m_min.y,p.y);
	m_max.x = std::max(m_max.x,p.x);
	m_max.y = std::max(m_max.y,p.y);
	return *this;
}

typedef base_aabb2<float> aabb2;
typedef base_aabb2<float> aabb2f;
typedef base_aabb2<int> aabb2i;

///вывод в поток stl
template<typename T>
inline ::std::ostream &operator<< (::std::ostream &stream, const base_aabb2<T> &pt)
{
	stream << "aabb2(" << (pt.degenerated()?"1":"0") "," << pt.min() << "," << pt.max() << ")";
	return stream;
}

///чтение из потока stl
template<typename T>
inline ::std::istream &operator>> (::std::istream &stream, base_aabb2<T> &pt)
{
	int degen = 0;
	base_point2<T> ptmin, ptmax;
	stream >> io::verify("aabb2") >> std::ws >> io::verify("(");
	stream >> std::ws >> degen >> std::ws >> io::verify(",");
	stream >> std::ws >> ptmin >> std::ws >> io::verify(",");
	stream >> std::ws >> ptmax >> std::ws >> io::verify(")");
	pt = degen ? base_aabb2<T>() : base_aabb2<T>(ptmin,ptmax);
	return stream;
}

///чтение из бинарного потока
template<typename T>
inline io::ibinstream &operator>>(io::ibinstream &stream, base_aabb2<T> &pt)
{
	return io::do_raw_input(stream,pt);
}

///вывод в бинарный поток
template<typename T>
inline io::obinstream &operator<<(io::obinstream &stream, const base_aabb2<T> &pt)
{
	return io::do_raw_output(stream,pt);
}


} // namespace 'geometry'
} // namespace 'mll'


#endif