#pragma once

#include <mll/_export_rules.h>
#include <mll/utils/timer.h>
#include <mll/debug/renderer.h>
#include <mll/algebra/point3.h>
#include <mll/algebra/vector2.h>
#include <mll/algebra/vector3.h>
#include <mll/algebra/matrix3.h>
#include <mll/algebra/quaternion.h>
#include <mll/geometry/ray2.h>
#include <mll/geometry/ray3.h>
#include <mll/geometry/aabb2.h>
#include <mll/geometry/aabb3.h>
#include <mll/geometry/obb3.h>
#include <mll/geometry/obb2.h>

namespace mll { namespace utils { template<typename T> class oneobj; } }

namespace mll
{
namespace debug
{

class renderer2;

namespace manip
{

namespace stdmanip = ::std;
namespace manip = ::mll::debug::manip;

using mll::algebra::point2;
using mll::algebra::point2i;
using mll::algebra::point3;
using mll::algebra::point3i;
using mll::algebra::vector2;
using mll::algebra::vector2i;
using mll::algebra::vector3;
using mll::algebra::vector3i;

template<typename T, typename Tag> struct manip_helper;

//=====================================================================================//
//                                                                                     //
//                                                                                     //
//              Базовые манипуляторы, взаимодействующие с ядром renderer2              //
//                                                                                     //
//                                                                                     //
//=====================================================================================//
//=====================================================================================//
//                             Установка 2d и 3d ориджинов                             //
//=====================================================================================//
struct pos2_tag;
struct pos3_tag;
typedef manip_helper<point2, pos2_tag> pos2;
typedef manip_helper<point3, pos3_tag> pos3;
MLL_EXPORT renderer2 &operator<<(renderer2 &, const pos2 &);
MLL_EXPORT renderer2 &operator<<(renderer2 &, const pos3 &);

//=====================================================================================//
//                  Проецирование и обратное проецирование ориджинов                   //
//=====================================================================================//
MLL_EXPORT renderer2 &project(renderer2 &);
MLL_EXPORT renderer2 &unproject(renderer2 &);
struct vec2_tag;
typedef manip_helper<point2, vec2_tag> move2;
MLL_EXPORT renderer2 &operator<<(renderer2 &, const move2 &);

//=====================================================================================//
//                          Добавление точек в стек геометрии                          //
//=====================================================================================//
struct pt2_tag;
struct pt3_tag;
typedef manip_helper<point2, pt2_tag> pt2;
typedef manip_helper<point3, pt3_tag> pt3;
MLL_EXPORT renderer2 &operator<<(renderer2 &, const pt2 &);
MLL_EXPORT renderer2 &operator<<(renderer2 &, const pt3 &);

//=====================================================================================//
//                  Трансформация и добавление точек в стек геометрии                  //
//=====================================================================================//
class transformer2_base
{
public:
	virtual ~transformer2_base() = 0 {}
	virtual point2 transform(const point2 &) = 0;
};

class transformer3_base
{
public:
	virtual ~transformer3_base() = 0 {}
	virtual point3 transform(const point3 &) = 0;
};

struct ptt2_tag;
struct ptt3_tag;
typedef manip_helper< std::auto_ptr<transformer2_base>, ptt2_tag> ptt2;
typedef manip_helper< std::auto_ptr<transformer3_base>, ptt3_tag> ptt3;
MLL_EXPORT renderer2 &operator<<(renderer2 &, const ptt2 &);
MLL_EXPORT renderer2 &operator<<(renderer2 &, const ptt3 &);

//=====================================================================================//
//                        Добавление ориджинов в стек геометрии                        //
//=====================================================================================//
MLL_EXPORT renderer2 &ppos2(renderer2 &);
MLL_EXPORT renderer2 &ppos3(renderer2 &);

//=====================================================================================//
//                           Низкоуровневое рисование линий                            //
//=====================================================================================//
struct pline2_tag;
struct pline3_tag;
typedef manip_helper<int,pline2_tag> pline2;
typedef manip_helper<int,pline3_tag> pline3;
MLL_EXPORT renderer2 &operator<<(renderer2 &, const pline2 &);
MLL_EXPORT renderer2 &operator<<(renderer2 &, const pline3 &);

//=====================================================================================//
//                                   Установить цвет                                   //
//=====================================================================================//
struct color
{
	unsigned int value;

	explicit color() : value(0xFFFFFFFF) {}
	explicit color(float r, float g, float b)
	{
		unsigned int rr = static_cast<unsigned int>(std::max(std::min(r*255.0f,255.0f),0.0f));
		unsigned int gg = static_cast<unsigned int>(std::max(std::min(g*255.0f,255.0f),0.0f));
		unsigned int bb = static_cast<unsigned int>(std::max(std::min(b*255.0f,255.0f),0.0f));
		value = 0xFF000000 | (rr << 16) | (gg << 8) | bb;
	}
	explicit color(float r, float g, float b, float a)
	{
		unsigned int rr = static_cast<unsigned int>(std::max(std::min(r*255.0f,255.0f),0.0f));
		unsigned int gg = static_cast<unsigned int>(std::max(std::min(g*255.0f,255.0f),0.0f));
		unsigned int bb = static_cast<unsigned int>(std::max(std::min(b*255.0f,255.0f),0.0f));
		unsigned int aa = static_cast<unsigned int>(std::max(std::min(a*255.0f,255.0f),0.0f));
		value = (aa << 24) | (rr << 16) | (gg << 8) | bb;
	}
	explicit color(unsigned int c) : value(c) {}
};
MLL_EXPORT renderer2 &operator<<(renderer2 &, const color &);

//=====================================================================================//
//                 Убрать несколько последних точек из стека геометрии                 //
//=====================================================================================//
struct cancel2_tag;
struct cancel3_tag;
typedef manip_helper<int,cancel2_tag> cancel2;
typedef manip_helper<int,cancel3_tag> cancel3;
MLL_EXPORT renderer2 &operator<<(renderer2 &, const cancel2 &);
MLL_EXPORT renderer2 &operator<<(renderer2 &, const cancel3 &);

//=====================================================================================//
//                      Сохранить/восстановить значение ориджинов                      //
//=====================================================================================//
MLL_EXPORT renderer2 &ppush(renderer2 &);
MLL_EXPORT renderer2 &ppop(renderer2 &);

//=====================================================================================//
//                     Получить параметры камеры в стек геометрии                      //
//=====================================================================================//
MLL_EXPORT renderer2 &campos(renderer2 &);
MLL_EXPORT renderer2 &camdir(renderer2 &);
MLL_EXPORT renderer2 &camright(renderer2 &);
MLL_EXPORT renderer2 &camup(renderer2 &);

//=====================================================================================//
//                             Сбросить текст на рендеринг                             //
//=====================================================================================//
MLL_EXPORT renderer2 &flush(renderer2 &r);
}

//=====================================================================================//
//                   class render_strategy2 : public render_strategy                   //
//=====================================================================================//
class render_strategy2 : public render_strategy
{
public:
	/// Возвращает размер окна рендеринга
	virtual point2 get_screen_size() = 0;
	///	Спроецировать трехмерную точку на экранную плоскость. Если z < 0.0f,
	/// то точка сзади плоскости экрана, иначе - спереди
	virtual point3 to_screen2(const point3& pt) = 0;
	/// Возвращает высоту фонта
	virtual float get_font_height() = 0;
	/// Возвращает видовую матрицу
	virtual mll::algebra::matrix3 get_view() = 0;
};

//=====================================================================================//
//                                   class renderer2                                   //
//                        Развитие вспомогательного рендеринга                         //
//=====================================================================================//
class MLL_EXPORT renderer2
{
	class impl;
	std::auto_ptr<impl> m_pimpl;

	renderer2();
	~renderer2();

public:
	/// Возвращает значение атрибута
	double get_attr(const std::string &n) const;
	void set_attr(const std::string &n, double val);

	/// Возвращает стандартный поток ввода/вывода
	std::ostream &get_stream();

	/// Установить стратегию
	void set_rs(render_strategy *strategy);
	/// Установить стратегию
	void set_rs(render_strategy2 *strategy);
	/// Обнулить указатель на стратегию
	void reset_rs();

	/// Отрисовать все, что накопил
	void render();
	/// Очистить накопленную очередь
	void clear();

	static renderer2 &inst();

private:
	friend utils::oneobj<renderer2>;

	/// Дружественные манипуляторы
	friend renderer2 &manip::ppos2(renderer2 &);
	friend renderer2 &manip::ppos3(renderer2 &);
	friend renderer2 &manip::project(renderer2 &);
	friend renderer2 &manip::unproject(renderer2 &);
	friend renderer2 &manip::flush(renderer2 &);
	friend renderer2 &manip::ppush(renderer2 &);
	friend renderer2 &manip::ppop(renderer2 &);
	friend renderer2 &manip::campos(renderer2 &);
	friend renderer2 &manip::camdir(renderer2 &);
	friend renderer2 &manip::camright(renderer2 &);
	friend renderer2 &manip::camup(renderer2 &);
	friend renderer2 &manip::operator<<(renderer2 &, const manip::color &);
	friend renderer2 &manip::operator<<(renderer2 &, const manip::pline2 &);
	friend renderer2 &manip::operator<<(renderer2 &, const manip::pline3 &);
	friend renderer2 &manip::operator<<(renderer2 &, const manip::pt2 &);
	friend renderer2 &manip::operator<<(renderer2 &, const manip::pt3 &);
	friend renderer2 &manip::operator<<(renderer2 &, const manip::ptt2 &);
	friend renderer2 &manip::operator<<(renderer2 &, const manip::ptt3 &);
	friend renderer2 &manip::operator<<(renderer2 &, const manip::pos2 &);
	friend renderer2 &manip::operator<<(renderer2 &, const manip::pos3 &);
	friend renderer2 &manip::operator<<(renderer2 &, const manip::move2 &);
	friend renderer2 &manip::operator<<(renderer2 &, const manip::cancel2 &);
	friend renderer2 &manip::operator<<(renderer2 &, const manip::cancel3 &);
};

typedef renderer2 &(* manip_t)(renderer2 &);

//=====================================================================================//
//                                                                                     //
//                                                                                     //
//                          Фундаментальные операторы сдвига                           //
//                                                                                     //
//                                                                                     //
//=====================================================================================//
inline renderer2 &operator<<(renderer2 &r, manip_t manip) { return (*manip)(r); }

template<typename T> renderer2 &operator<<(renderer2 &r, const T &t)
{
	r.get_stream() << t;
	return r;
}

inline renderer2 &operator<<(renderer2 &r, std::basic_ostream<char>& (*manip)(std::basic_ostream<char>&))
{
	r.get_stream() << manip;
	return r;
}

inline renderer2 &operator<<(renderer2 &r, std::ios_base & (*manip)(std::ios_base &))
{
    r.get_stream() << manip;
    return r;
}

//=====================================================================================//
//                                       RDBG2()                                       //
//                    Вспомогательный макрос для работы с renderer2                    //
//=====================================================================================//
#define MLL_RDBG2(L)																	\
	{																					\
		using namespace ::mll::debug::manip;											\
		using namespace ::std;															\
		using manip::flush;																\
		using manip::tan;																\
		namespace std = ::std;															\
		using namespace ::mll::algebra;													\
		using namespace ::mll::geometry;												\
		using manip::ray2;																\
		using manip::ray3;																\
		using manip::aabb2;																\
		using manip::aabb3;																\
		using manip::obb2_helper;														\
		using manip::make_obb2;															\
		using manip::obb3;																\
		using manip::sphere;															\
		using manip::cilynder;															\
		::mll::debug::renderer2::inst() << L << flush;									\
	}

namespace manip
{

//=====================================================================================//
//                               Вспомогательные классы                                //
//=====================================================================================//
template<typename T, typename Tag>
struct manip_helper
{
	T value;
	explicit manip_helper(const T &value) : value(value) {}
};

template<typename Tag>
struct manip_helper<point2, Tag>
{
	point2 value;
	explicit manip_helper(const point2 &value) : value(value) {}
	explicit manip_helper(float x, float y) : value(x,y) {}
	explicit manip_helper(const point2i &value) : value(static_cast<float>(value.x), static_cast<float>(value.y)) {}
	explicit manip_helper(int x, int y) : value(static_cast<float>(x),static_cast<float>(y)) {}
};

template<typename Tag>
struct manip_helper<point3, Tag>
{
	point3 value;
	explicit manip_helper(const point3 &value) : value(value) {}
	explicit manip_helper(float x, float y, float z) : value(x,y,z) {}
};

template<typename T, typename Tag>
struct manip_helper<std::auto_ptr<T>, Tag>
{
	std::auto_ptr<T> value;
	explicit manip_helper(std::auto_ptr<T> value) : value(value) {}
};

//=====================================================================================//
//                                                                                     //
//                                                                                     //
//                            Манипуляторы второго уровня.                             //
//               Работают с renderer2 только через базовые манипуляторы.               //
//                                                                                     //
//                                                                                     //
//=====================================================================================//

//=====================================================================================//
//                   Нарисовать линию от ориджина до заданной точки                    //
//=====================================================================================//
struct lineto2_tag;
struct lineto3_tag;
typedef manip_helper<point2,lineto2_tag> lineto2;
typedef manip_helper<point3,lineto3_tag> lineto3;

MLL_EXPORT renderer2 &operator<<(renderer2 &r, const lineto2 &l);
MLL_EXPORT renderer2 &operator<<(renderer2 &r, const lineto3 &l);

//=====================================================================================//
//                Нарисовать луч от ориджина по указанному направлению                 //
//=====================================================================================//
struct rayto2_tag;
struct rayto3_tag;
typedef manip_helper<point2,rayto2_tag> rayto2;
typedef manip_helper<point3,rayto3_tag> rayto3;

MLL_EXPORT renderer2 &operator<<(renderer2 &r, const rayto2 &rt);
MLL_EXPORT renderer2 &operator<<(renderer2 &r, const rayto3 &rt);

//=====================================================================================//
//                                   Нарисовать луч                                    //
//=====================================================================================//
struct ray2
{
	mll::geometry::ray2 r;
	ray2() : r(point2(0.0f,0.0f), mll::algebra::vector2(0.0f,0.0f)) {}
	explicit ray2(const point2 &p, const mll::algebra::vector2 &v) : r(p,v) {}
	explicit ray2(const mll::geometry::ray2 &r) : r(r) {}
};

struct ray3
{
	mll::geometry::ray3 r;
	ray3() : r(point3(0.0f,0.0f,0.0f), mll::algebra::vector3(0.0f,0.0f,0.0f)) {}
	explicit ray3(const point3 &p, const mll::algebra::vector3 &v) : r(p,v) {}
	explicit ray3(const mll::geometry::ray3 &r) : r(r) {}
};

MLL_EXPORT renderer2 &operator<<(renderer2 &r, const ray2 &rr);
MLL_EXPORT renderer2 &operator<<(renderer2 &r, const ray3 &rr);

//=====================================================================================//
//                                  Нарисовать маркер                                  //
//=====================================================================================//
enum marker_type
{
	mt_x,
	mt_plus,
	mt_o
};

struct marker3_helper
{
	marker_type type;
	point3 point;
	bool use_origin;

	explicit marker3_helper(marker_type mt) : type(mt), point(0.0f,0.0f,0.0f), use_origin(true) {}
	explicit marker3_helper(marker_type mt, const point3 &pt) : type(mt), point(pt), use_origin(false) {}
};


MLL_EXPORT renderer2 &operator<<(renderer2 &r, const marker3_helper &m);
MLL_EXPORT renderer2 &marker3(renderer2 &r);
MLL_EXPORT marker3_helper marker3(marker_type mt);
MLL_EXPORT marker3_helper marker3(const point3 &pt);
MLL_EXPORT marker3_helper marker3(float x, float y, float z);
MLL_EXPORT marker3_helper marker3(marker_type mt, const point3 &pt);
MLL_EXPORT marker3_helper marker3(marker_type mt, float x, float y, float z);
MLL_EXPORT renderer2 &operator<<(renderer2 &r, const marker3_helper &m);

//=====================================================================================//
//                                Отрисовать бокс aabb2                                //
//=====================================================================================//
struct aabb2
{
	mll::geometry::aabb2 value;

	explicit aabb2(float x1, float y1, float x2, float y2) : value(x1,y1,x2,y2) {}
	explicit aabb2(const point2 &p1, const point2 &p2) : value(p1,p2) {}
	explicit aabb2(const mll::geometry::aabb2 &b) : value(b) {}
};

MLL_EXPORT renderer2 &operator<<(renderer2 &r, const aabb2 &box);

//=====================================================================================//
//                                Отрисовать окружность                                //
//=====================================================================================//
struct circle2
{
	mll::algebra::point2 pos;
	float r;
	int segments;

	circle2(float x, float y, float radius, int segment_count = 16) : pos(x,y), r(radius), segments(segment_count) {}
	circle2(const point2 &center, float radius, int segment_count = 16) : pos(center), r(radius), segments(segment_count) {}
};

MLL_EXPORT renderer2 &operator<<(renderer2 &r, const circle2 &c);

//=====================================================================================//
//                                Отрисовать сферу                                     //
//=====================================================================================//
struct sphere
{
	point3 pos;
	float  radius;
	bool   use_origin;

	explicit sphere(const point3& p, float r) : pos(p), radius(r), use_origin(true) {}
	//explicit sphere(float r) : pos(0,0,0), radius(r), use_origin(false) {}
};

MLL_EXPORT renderer2 &operator<<(renderer2 &r, const sphere &s);

//=====================================================================================//
//                                Отрисовать цилиндр                                   //
//=====================================================================================//
struct cilynder
{
	point3 pos;
	float radius;
	float height;

	cilynder(const point3& p, float r, float h) : pos(p), radius(r), height(h) {}
	explicit cilynder(const mll::geometry::obb3& box) 
	:	pos(box.origin()), radius(box.x().length()*0.5f), height(box.z().length()) {}
};

MLL_EXPORT renderer2 &operator<<(renderer2 &r, const cilynder &c);

//=====================================================================================//
//                                Отрисовать бокс aabb3                                //
//=====================================================================================//
struct aabb3
{
	mll::geometry::aabb3 value;

	explicit aabb3(float x1, float y1, float z1, float x2, float y2, float z2) : value(x1,y1,z1,x2,y2,z2) {}
	explicit aabb3(const point3 &p1, const point3 &p2) : value(p1,p2) {}
	explicit aabb3(const mll::geometry::aabb3 &b) : value(b) {}
};

MLL_EXPORT renderer2 &operator<<(renderer2 &r, const aabb3 &box);

//=====================================================================================//
//                                Отрисовать бокс obb3                                //
//=====================================================================================//
struct obb3
{
	const mll::geometry::obb3 value;

	explicit obb3(const mll::algebra::vector3 &v1, const mll::algebra::vector3 &v2, const mll::algebra::vector3 &v3, const mll::algebra::point3 &pt): value(v1, v2, v3, pt) {}
	explicit obb3(const mll::geometry::obb3 &obb): value(obb) {}
	explicit obb3(const mll::geometry::aabb3 &aabb): value(aabb) {}

};

MLL_EXPORT renderer2 &operator<<(renderer2 &r, const obb3 &box);

//=====================================================================================//
//                                Отрисовать бокс obb3                                //
//=====================================================================================//
struct obb2_helper
{
	const mll::geometry::obb2 value;
	mll::algebra::point3 pos;

	explicit obb2_helper() {}
	explicit obb2_helper(const mll::algebra::vector2 &v1, const mll::algebra::vector2 &v2, const mll::algebra::point2 &pt): value(v1, v2, pt), pos(0,0,0) {}
	explicit obb2_helper(const mll::geometry::obb2 &obb): value(obb), pos(0,0,0) {}
	
	point3 collect_origin(point3 o) { return pos = o; }
	point3 transform_z(point3 p) { return point3(p.x, p.y, pos.z); }
};

inline obb2_helper make_obb2(const mll::algebra::vector2 &v1, const mll::algebra::vector2 &v2, const mll::algebra::point2 &pt)
{
	return obb2_helper(v1, v2, pt); 
}

inline obb2_helper make_obb2(const mll::geometry::obb2 &obb)
{
	return obb2_helper(obb); 
}

MLL_EXPORT renderer2 &operator<<(renderer2 &r, const obb2_helper &box);

//=====================================================================================//
//                      Вывести в текстовый потоке количество fps                      //
//=====================================================================================//
MLL_EXPORT renderer2 &fps(renderer2 &r);

}

}
}

#include "renderer2_colors.h"