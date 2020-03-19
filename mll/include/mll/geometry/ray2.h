#if !defined(__RAY2_INCLUDED__)
#define __RAY2_INCLUDED__

/////////////////////////////////////////////////////////////////////////
//	MiST land Library files
//#include <mll/algebra/matrix2.h>
#include <mll/algebra/vector2.h>
#include <mll/algebra/point2.h>
/////////////////////////////////////////////////////////////////////////

namespace mll 
{ 

namespace geometry
{

///обобщенный класс луча в 2-х мерном пространстве
template<class T>
class base_ray2
{
public:

//	typedef mll::algebra::base_matrix2<T> matrix_type;
	typedef mll::algebra::base_vector2<T> vector_type;
	typedef mll::algebra::base_point2<T> point_type;

	vector_type direction;		///<	направление луча
	point_type origin;			///<	позиция луча

public:

	///	конструктор по умолчанию
	base_ray2();

	///	конструктор копирования
	base_ray2(const base_ray2<T>& r);

	///	конструктор из точки и направления
	base_ray2(const point_type& origin,const vector_type& direction);

	~base_ray2();

	///получение точки на луче по указанномц расстоянию от начала луча
	point_type at(T factor) const { assert( factor >= static_cast<T>(0.0) ); return origin + direction*factor; }
	///нормализация направления
	base_ray2 &normalize() { direction.normalize(); return *this; }

	///	оператор присваивания
	const base_ray2& operator=(const base_ray2<T>& r);

	/////////////////////////////////////////////////////////////////////////
	//	оператор трансформирования луча при помощи матрицы
//	base_ray2& operator*=(const matrix_type& m);
	/////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////
template<class T>
inline base_ray2<T>::base_ray2()
{
}
/////////////////////////////////////////////////////////////////////////
template<class T>
inline base_ray2<T>::base_ray2(const base_ray2<T>& r) : origin(r.origin),
													    direction(r.direction)
{
}
/////////////////////////////////////////////////////////////////////////
template<class T>
inline base_ray2<T>::base_ray2(const point_type& origin,const vector_type& direction) : origin(origin),
																						direction(direction)
{
}
/////////////////////////////////////////////////////////////////////////
template<class T>
inline base_ray2<T>::~base_ray2()
{
}
/////////////////////////////////////////////////////////////////////////
template<class T>
inline const base_ray2<T>& base_ray2<T>::operator=(const base_ray2<T>& r)
{
	origin = r.origin;
	direction = r.direction;

	return *this;
}
///чтение из бинарного потока
template<typename T>
inline io::ibinstream &operator>>(io::ibinstream &stream, base_ray2<T> &r)
{
	return stream >> r.direction >> r.origin;
}

///вывод в бинарный поток
template<typename T>
inline io::obinstream &operator<<(io::obinstream &stream, const base_ray2<T> &r)
{
	return stream << r.direction << r.origin;
}
/////////////////////////////////////////////////////////////////////////
//	оператор трансформирования луча при помощи матрицы
//template<class T>
//inline base_ray2<T>& base_ray2<T>::operator*=(const matrix_type& m)
//{
//	origin = origin*m;
//	direction = direction*m;
//
//	return *this;
//}
/////////////////////////////////////////////////////////////////////////
//	внешний оператор трансформирования луча при помощи матрицы
//template<class T>
//inline base_ray2<T> operator*(const base_ray2<T>& r,const base_ray2<T>::matrix_type& m)
//{
//	base_ray2 rv(r);
//	rv *= m;
//	return rv;
//}
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//	наиболее часто используемые определения
typedef base_ray2<float> ray2;  ///<луч с координатами с плавающей точкой одинарной точности
typedef base_ray2<float> ray2f; ///<луч с координатами с плавающей точкой одинарной точности
/////////////////////////////////////////////////////////////////////////

}	//	namespace debug

}	//	namespace mll

#endif