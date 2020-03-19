#pragma once

#include "file_id.h"

#include <deque>


namespace mll
{

namespace fs
{


//=====================================================================================//
//                                     class cache                                     //
//=====================================================================================//
class FILE_SYSTEM_EXPORT cache
{
private:
	typedef std::deque<file_id> single_query_cache_t;
	single_query_cache_t m_single_query_data;

public:		//	кэширование данных
	///	добавить данные последнего запроса на операцию с одним файлом (r_open, w_open, exists)
	void add_single_query(file_id id);
	///	файл удаляется, нужно сбросить его из всех кэшей
	void drop_file(file_id id);
	///	сбросить все кэши (например, обновилась структура подсистем)
	void clear_all();

public:		//	получение данных
	///	посмотреть наличие в кэше id, полученного ранее по такому же пути
	file_id single_query_stored(const std::string& path);
};

}

}

