#if !defined(__FS_FILES_H_INCLUDED_7552370787048739__)
#define __FS_FILES_H_INCLUDED_7552370787048739__


#include "file_list.h"
#include "file_id.h"
#include "fs_path.h"

#include <boost/shared_ptr.hpp>
#include <list>


namespace mll
{

namespace fs
{


//=====================================================================================//
//                     template<class T> class reference_container                     //
//=====================================================================================//
template<class T>
class reference_container
{
private:
	typedef T container_type;
	const container_type &m_list;

public:
	reference_container(const container_type &list): m_list(list) {}
	~reference_container() {}

public:
	typedef typename container_type::const_iterator iterator;

public:
	///	получить итератор на начало
	iterator begin() { return m_list.begin(); }
	///	получить итератор на конец
	iterator end() { return m_list.end(); }
	///	получить размер контейнера
	unsigned size() const { return m_list.size(); }
};


//=====================================================================================//
//                       template<class T> class value_container                       //
//=====================================================================================//
template<class T>
class value_container
{
private:
	typedef T container_type;
	container_type m_list;

public:
	value_container(const container_type &list): m_list(list) {}
	value_container(const value_container &another): m_list(another.m_list) {}
	~value_container() {}

public:	
	typedef typename container_type::const_iterator iterator;

public:
	///	получить итератор на начало
	iterator begin() { return m_list.begin(); }
	///	получить итератор на конец
	iterator end() { return m_list.end(); }
	///	получить размер контейнера
	unsigned size() const { return m_list.size(); }
};



//	этим нужно пользоваться
typedef std::list<file_id> file_t;
typedef reference_container<file_t> file_container;
typedef value_container<file_t> id_container;











//=====================================================================================//
//                                   class fs_files                                    //
//=====================================================================================//
template <class Pred = date_heuristic>
class fs_files
{
public:
	typedef file_t::const_iterator iterator;

private:
	boost::shared_ptr<file_list<Pred> > m_data; 
	boost::shared_ptr<file_t> m_list;

public:
	fs_files() {}
	fs_files(const fs_path &path, unsigned param);
	fs_files(const fs_files<Pred> &another);
	~fs_files() {}

	fs_files<Pred> &operator=(const fs_files<Pred> &);

public:
	file_container get_file_container() const;
	id_container get_id_container(const file_id &id) const;

public:
	///	получить итератор на начало контейнера предпочтительных вариантов фалов
	iterator begin() const { return m_list->begin(); }
	///	получить итератор на конец
	iterator end() const { return m_list->end(); }
	///	получить размер контейнера
	unsigned int size() const { return m_list->size(); }
};



template<class Pred>
fs_files<Pred>::fs_files(const fs_path &path, unsigned param)
{
	m_data = std::auto_ptr<file_list<Pred> >(new file_list<Pred>(path, param));
	file_t *tmp = new file_t();
	for (file_list<Pred>::iterator i = m_data->begin(); i != m_data->end(); ++i)
	{
		tmp->push_back(*i->begin());
	}
	m_list = std::auto_ptr<file_t>(tmp);
}


template<class Pred>
fs_files<Pred>::fs_files(const fs_files &another)
{
	m_data = another.m_data;
	m_list = another.m_list;
}


template<class Pred>
fs_files<Pred> &fs_files<Pred>::operator=(const fs_files<Pred> &another)
{
	m_data = another.m_data;
	m_list = another.m_list;
	return *this;
}


template<class Pred>
file_container fs_files<Pred>::get_file_container() const
{
	return file_container(*m_list.get());
}


template<class Pred>
id_container fs_files<Pred>::get_id_container(const file_id &id) const
{
	file_t tmp;
	for (file_list<Pred>::iterator i = m_data->begin(); i != m_data->end(); ++i)
	{
		if (id.path() == (*i->begin()).path())
		{
			for (std::set<file_id, Pred>::iterator j = i->begin(); j != i->end(); ++j)
			{
				tmp.push_back(*j);
			}
		}
	}
	return id_container(tmp);
}






}

}

#endif // !defined(__FS_FILES_H_INCLUDED_7552370787048739__)