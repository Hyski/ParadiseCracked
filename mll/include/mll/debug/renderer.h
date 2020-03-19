#if !defined(__RENDERER_INCLUDED__)
#define __RENDERER_INCLUDED__

/////////////////////////////////////////////////////////////////////////
//	STL Library files
#include <string>
#include <list>
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//	MiST land Library files
#include <mll/_choose_lib.h>
#include <mll/_export_rules.h>
#include <mll/algebra/point2.h>
#include <mll/algebra/point3.h>
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
using mll::algebra::point2;
using mll::algebra::point3;
/////////////////////////////////////////////////////////////////////////

namespace mll
{

namespace geometry
{
	template<class T> class base_frustum;
	template<class T> class base_aabb3;
	template<class T> class base_obb3;
	template<class T> class base_ray3;
}

namespace debug
{
/*
class marker;
*/
//=====================================================================================//
//                                class render_strategy                                //
//=====================================================================================//
class render_strategy
{
public:
	virtual ~render_strategy() {}
	///	отобразить двухмерную линию
	virtual void put_line(const point2& begin,const point2& end,unsigned int color) = 0;
	///	отобразить трехмерную линию
	virtual void put_line(const point3& begin,const point3& end,unsigned int color) = 0;
	///	отобразить двухмерный текст
	virtual void put_text(const point2& origin,const std::string& text,unsigned int color) = 0;
	///	отобразить трехмерный текст
	virtual void put_text(const point3& origin,const std::string& text,unsigned int color) = 0;
	///	спроецировать трехмерную точку на экранную плоскость
	virtual point2 to_screen(const point3& pt) = 0;
	///	отрисовать все буферизированные данные (если таковые имеются)
	virtual void flush(void) {}

};

//=====================================================================================//
//                                   class renderer                                    //
//=====================================================================================//
class MLL_EXPORT renderer
{
public:

	/////////////////////////////////////////////////////////////////////////
	class command;
	/////////////////////////////////////////////////////////////////////////

private:

	typedef std::list<command*> commands_t;				//	тип контейнера команд

	render_strategy* m_rs;								//	указатель на стратегию отрисовки
	commands_t m_commands;								//	контейнер комманд
	unsigned int m_color;								//	текущий цвет
	point3 m_origin3;									//	текущее начало коордиинат для 3D
	point2 m_origin2;									//	текущее начало коордиинат для 2D
	float m_meter;										//	размер метра (масштаб)

private:
	renderer();
	~renderer();
public:
	///	отобразить фрастум
	renderer& operator<<(const geometry::base_frustum<float>& f);
	///	отобразить ориентированный по осям орграничивающий бокс
	renderer& operator<<(const geometry::base_aabb3<float>& b);
	///	отобразить ориентированный орграничивающий бокс
	renderer& operator<<(const geometry::base_obb3<float>& b);
	/// вывести луч
	renderer& operator<<(const geometry::base_ray3<float>& r);
	///	вывести строку текста
	renderer& operator<<(const std::string& text);

public:
	///	установить стратегию отрисовки (вернуть предыдущую)
	render_strategy* set_strategy(render_strategy* strategy);
	///	вызвать функцию через оператор сдвига
	renderer& operator<<(renderer& (*f)(renderer&));
	///	выполнить все накопленные команды
	void flush(void);

public:
	///	установить текущую позицию отрисовки 3D объектов
	void set_origin3(const point3& origin);
	///	получить текущую позицию отрисовки 3D объектов
	const point3& get_origin3(void) const;
	///	установить текущую позицию отрисовки 2D объектов
	void set_origin2(const point2& origin);
	///	получить текущую позицию отрисовки 2D объектов
	const point2& get_origin2(void) const;
	///	установить текущий цвет отображения
	void set_color(unsigned int color);
	///	получить текущий цвет отображения
	unsigned int get_color(void) const;
	///	установить текущее значение метра
	void set_meter(float m);
	///	получить текущее значение метра
	float get_meter(void) const;
	///	получить стратегию отображения
	render_strategy* get_rs(void);


public:
	///	установить команду в очередь (предназначено для использования в манипуляторах)
	void insert_command(command* c);
	///	получить инстанцию класса
	static renderer* instance(void);

	friend std::auto_ptr<renderer>;
};

//	вызвать функцию через оператор сдвига
inline renderer& renderer::operator<<(renderer& (*f)(renderer&))
{
	return f(*this);
}
//	получить текущее начало 3D координат
inline const point3& renderer::get_origin3(void) const
{
	return m_origin3;
}
//	получить текущее начало 2D координат
inline const point2& renderer::get_origin2(void) const
{
	return m_origin2;
}
//	получить текущий цвет отображения
inline unsigned int renderer::get_color(void) const
{
	return m_color;
}
//	получить текущее нзначение метра
inline float renderer::get_meter(void) const
{
	return m_meter;
}
//	установить текущее начало 3D координат
inline void renderer::set_origin3(const point3& origin)
{
	m_origin3 = origin;
}
//	установить текущее начало 2D координат
inline void renderer::set_origin2(const point2& origin)
{
	m_origin2 = origin;
}
//	установить текущий цвет отображения
inline void renderer::set_color(unsigned int color)
{
	m_color = color;
}
//	установить текущее значение метра
inline void renderer::set_meter(float m)
{
	m_meter = m;
}
//	получить стратегию отображения
inline render_strategy* renderer::get_rs(void)
{
	return m_rs;
}

//=====================================================================================//
//                               коллекция манипуляторов                               //
//=====================================================================================//
//=====================================================================================//
//                                   struct strategy                                   //
//=====================================================================================//
///	установить стратегию отрисовки
struct strategy
{
	mll::debug::render_strategy* m_rs;
	strategy(mll::debug::render_strategy* rs) : m_rs(rs) {}
	friend mll::debug::renderer& operator<<(mll::debug::renderer& r,const mll::debug::strategy& s);
};

inline mll::debug::renderer& operator<<(mll::debug::renderer& r,const mll::debug::strategy& s)
{
	r.set_strategy(s.m_rs);
	return r;
}

//=====================================================================================//
//                            mll::debug::renderer& flush()                            //
//=====================================================================================//
///	отрисовать все примитивы
inline mll::debug::renderer& flush(mll::debug::renderer& r)
{
	r.flush();
	return r;
}

//=====================================================================================//
//                                    struct color                                     //
//=====================================================================================//
///	установить текущий цвет
struct color
{
	unsigned int m_value;
	color(unsigned int value) : m_value(value) {}
	friend MLL_EXPORT mll::debug::renderer& operator<<(mll::debug::renderer& r,const mll::debug::color& c);
};

//=====================================================================================//
//                                   struct moveto2                                    //
//=====================================================================================//
///	установить текущее начало 2D координат
struct moveto2
{
	point2 m_value;
	moveto2(const point2& value) : m_value(value) {}
	moveto2(float x,float y) : m_value(x,y) {}
	friend MLL_EXPORT mll::debug::renderer& operator<<(mll::debug::renderer& r,const mll::debug::moveto2& o);
};

//=====================================================================================//
//                                   struct moveto3                                    //
//=====================================================================================//
///	установить текущее начало 3D координат
struct moveto3
{
	point3 m_value;
	moveto3(const point3& value) : m_value(value) {}
	moveto3(float x,float y,float z) : m_value(x,y,z) {}
	friend MLL_EXPORT mll::debug::renderer& operator<<(mll::debug::renderer& r,const mll::debug::moveto3& o);
};

//=====================================================================================//
//                                    struct text3                                     //
//=====================================================================================//
///	вывести трех мернай текст
struct text3
{
	const std::string m_value;
	text3(const std::string& value) : m_value(value) {}
	friend MLL_EXPORT mll::debug::renderer& operator<<(mll::debug::renderer& r,const mll::debug::text3& o);
};

//=====================================================================================//
//                        макросы для использования отрисовщика                        //
//=====================================================================================//
#if defined(MLL_ENABLE_DEBUG_RENDERER)
#define MLL_DEBUG_RENDER(List)				*mll::debug::renderer::instance() << List
#else
#define MLL_DEBUG_RENDER(List)				
#endif

/*
/////////////////////////////////////////////////////////////////////////
//////////////////////////    class renderer    //////////////////////////
/////////////////////////////////////////////////////////////////////////
class MLL_EXPORT renderer
{
public:

	/////////////////////////////////////////////////////////////////////////
	class command;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	///////////////////////////    class meter    ///////////////////////////
	/////////////////////////////////////////////////////////////////////////
	class meter
	{
	public:
		float value;
		meter() : value(1) {}
		meter(const meter& m) : value(m.value) {}
		meter(float v) : value(v) {}
		~meter() {}
		const meter& operator=(const meter& m) { value=m.value; return *this; }
	};
	/////////////////////////////////////////////////////////////////////////

private:

	/////////////////////////////////////////////////////////////////////////
	typedef std::list<command*> commands_t;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	commands_t m_commands;					//	список комманд
	render_strategy* m_rs;					//	ссылка на стратегию отрисовки
	point2 m_origin2;						//	начало коордиинат для 2D
	point3 m_origin3;						//	начало коордиинат для 3D
	color m_color;							//	цвет по умолчанию
	meter m_meter;							//	значение метра
	/////////////////////////////////////////////////////////////////////////

public:
	renderer(render_strategy* rs);
	~renderer();
public:
	//	отобразить маркер
	renderer& operator<<(const marker& m);
	//	отобразить ориентированный по осям орграничивающий бокс
	renderer& operator<<(const geometry::base_aabb3<float> &r);
	//	отобразить фрастум
	renderer& operator<<(const geometry::base_frustum<float> &r);
	//	отобразить линию
	renderer& operator<<(const line &l);
	//	вывести двумерный текст
	renderer& operator<<(const text2& t);
	//	отобразить луч
	renderer& operator<<(const geometry::base_ray3<float>& r);
	//	установить цвет по умолчанию
	renderer& operator<<(const color& c);
	//	установить значение метра по умолчанию
	renderer& operator<<(const meter& m);
	//	отобразить все
	void flush(void);

public:

	/////////////////////////////////////////////////////////////////////////
	//	установить текущий цвет отображения
	void set_color(const color& c);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	получить текущий цвет отображения
	const color& get_color(void) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	установить текущее начало 2D координат
	void set_origin2(const point2& origin);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	получить текущее начало 2D координат
	const point2& get_origin2(void) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	установить текущее начало 3D координат
	void set_origin3(const point3& origin);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	получить текущее начало 3D координат
	const point3& get_origin3(void) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	установить текущее значение метра
	void set_meter(const meter& m);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	получить текущее нзначение метра
	const meter& get_meter(void) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	получить стратегию отображения
	render_strategy* get_rs(void);
	/////////////////////////////////////////////////////////////////////////

};

/////////////////////////////////////////////////////////////////////////
//	получить текущий цвет отображения
inline const color& renderer::get_color(void) const
{
	return m_color;
}
/////////////////////////////////////////////////////////////////////////
//	получить текущее начало 2D координат
inline const point2& renderer::get_origin2(void) const
{
	return m_origin2;
}
/////////////////////////////////////////////////////////////////////////
//	получить текущее начало 3D координат
inline const point3& renderer::get_origin3(void) const
{
	return m_origin3;
}
/////////////////////////////////////////////////////////////////////////
//	получить текущее нзначение метра
inline const renderer::meter& renderer::get_meter(void) const
{
	return m_meter;
}
/////////////////////////////////////////////////////////////////////////
//	получить стратегию отображения
inline render_strategy* renderer::get_rs(void)
{
	return m_rs;
}
/////////////////////////////////////////////////////////////////////////

//=====================================================================================//
//                                     class text2                                     //
//=====================================================================================//
class MLL_EXPORT text2
{
public:

	mll::algebra::point2 m_pt;						//	точка вывода
	std::string m_text;								//	текст

public:
	text2();
	text2(const char* text,...);
	text2(int x,int y,const char* text,...);
	text2(const mll::algebra::point2& pt,const char* text,...);

};
*/
}	//	namespace debug

}	//	namespace mll



#endif