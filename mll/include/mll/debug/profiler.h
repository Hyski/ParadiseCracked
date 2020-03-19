#if !defined(__PROFILER_H_INCLUDED_7552370787048739__)
#define __PROFILER_H_INCLUDED_7552370787048739__


#include <mll/utils/timer.h>
#include <string>
#include <map>
#include <iostream>

#include <mll/_export_rules.h>
#include <mll/_choose_lib.h>

namespace mll
{
namespace debug
{

//=====================================================================================//
//                                   class profiler                                    //
//=====================================================================================//
/** \class profiler
	\brief Физически содержит информацию о времени выполнения блоков и кол-ве из вызовов.
*/
class MLL_EXPORT profiler
{

public:
	/// первый элемент - суммарное время выполнения вызовов блока, второй - кол-во вызовов
	typedef std::pair<float, int> time_count_t;
	/// получить запись из контейнера.
	/** \param str - строка, которой идентифицирован блок
	*/
	static time_count_t *get_record(const std::string &str);
	/// сгенерировать файл отчета
	/** \param stream - открытый поток на запись
	*/
	static void flush(std::ostream &stream);

private:
	typedef std::map<std::string, time_count_t> times_t; 
	static times_t m_times;
};


//=====================================================================================//
//                                     class query                                     //
//=====================================================================================//
/** \class query
	\brief Создается один раз для каждого блока, заносит информацию о нем в профайлер.
*/
class MLL_EXPORT query
{
	std::string m_name;
	profiler::time_count_t *m_time;
	mll::utils::timer m_timer;

public:
	query(const std::string &);
	~query() {}

	/// добавляет время выполнения блока к существующей записи и инкрементирует счетчик
	/** \param t - время выполнения очередного вызова блока
	*/
	void add_time(float t);
	/// получить время таймера в секундах. Используется для вычисления времени выполнения.
	float get_tick() const;
};


inline query::query(const std::string &str)
:	m_name(str)
{
	m_time = profiler::get_record(m_name);
}

inline float query::get_tick() const
{
	return m_timer.get().seconds();
}

inline void query::add_time(float t)
{
	m_time->first += t;
	++m_time->second;
}


//=====================================================================================//
//                                    class profile                                    //
//=====================================================================================//
/** \class profile
	\brief Следит за конкретным вызовом конкретного блока, т.е. считает время.
*/
/** Профайл засекает время, которое он сам существует. Рекомендуется использовать его на стеке.
*/
class profile
{
	query *const m_query;
	float m_time_var;
	
public:
	profile(query *);
	~profile();
};


inline profile::profile(query *q)
:	m_query(q)
{
	m_time_var = m_query->get_tick();
}

inline profile::~profile()
{
	m_time_var = m_query->get_tick() - m_time_var;
	m_query->add_time(m_time_var);
}


#if defined(USE_PROFILING)
#	define MLL_PROFILE(name)		static mll::debug::query __localQuery(name);		\
								mll::debug::profile __localProfile(&__localQuery)
#	define MLL_PRFLUSH(stream)		mll::debug::profiler::flush(stream)
#else
#	define	MLL_PROFILE(name)
#	define	MLL_PRFLUSH(stream)
#endif

}  //debug
}  //mll


#endif // !defined(__PROFILER_H_INCLUDED_7552370787048739__)