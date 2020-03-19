#if !defined(__DIRECTORY_H_INCLUDED_1747580201310565__)
#define __DIRECTORY_H_INCLUDED_1747580201310565__


#include "sub_fs.h"

#include <string>
#include <list>


namespace mll
{

namespace fs
{



//=====================================================================================//
//                                   class directory                                   //
//=====================================================================================//
class directory
{
private:
	typedef std::list<sub_fs *> list_t;
	list_t m_list;
	std::string m_name;

public:
	typedef list_t::iterator iterator;
	typedef list_t::const_iterator const_iterator;

public:
	directory(const std::string &name): m_name(name) {}
	~directory();

	iterator mount(sub_fs *what);
	std::auto_ptr<sub_fs> unmount(iterator what);

	const std::string &name() { return m_name; }

	const_iterator begin() { return m_list.begin(); }
	const_iterator end() { return m_list.end(); }
	unsigned size() const { return m_list.size(); }
};






























//=====================================================================================//
//                                     РЕАЛИЗАЦИЯ                                      //
//=====================================================================================//


/////////////////////////////////////////////////////////////////////////
//	деструктор уничтожает имеющиеся замонтированные подсистемы,
//	и на всякий пожарный случай чистит список
inline directory::~directory()
{
	for (iterator i = m_list.begin(); i != m_list.end(); ++i)
		delete *i;
	m_list.clear();
}


/////////////////////////////////////////////////////////////////////////
//	монтирование подсистемы. Итератор возвращается для формирования
//	sub_fs_id
inline directory::iterator directory::mount(sub_fs *what)
{
	return m_list.insert(m_list.begin(), what);
}


/////////////////////////////////////////////////////////////////////////
//	демонтирование подсистемы. Права на удаление уходят
//	вместе с auto_ptr
inline std::auto_ptr<sub_fs> directory::unmount(iterator what)
{
	std::auto_ptr<sub_fs> tmp(*what);
	m_list.erase(what);
	return tmp;
}



}

}

#endif // !defined(__DIRECTORY_H_INCLUDED_1747580201310565__)