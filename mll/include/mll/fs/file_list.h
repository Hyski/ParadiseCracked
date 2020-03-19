#if !defined(__FILE_LIST_H_INCLUDED_5698059465543420__)
#define __FILE_LIST_H_INCLUDED_5698059465543420__


#include "sub_fs_iter.h"
#include "mask_regex.h"
#include "fs_guider.h"
#include "directory.h"
#include "file_id.h"
#include "system.h"

#include <set>


namespace mll
{

namespace fs
{


//=====================================================================================//
//                                   class file_list                                   //
//=====================================================================================//
template <class Pred = date_heuristic>
class file_list
{
private:
	typedef std::list<std::set<file_id, Pred> > cont_t;
	cont_t m_list;

public:
	typedef typename cont_t::iterator iterator;
	typedef typename cont_t::const_iterator const_iterator;

public:
	file_list(const fs_path &path, unsigned param);
	~file_list() {}

public:
	iterator begin() { return m_list.begin(); }
	iterator end() { return m_list.end(); }
	const_iterator begin() const { return m_list.begin(); }
	const_iterator end() const { return m_list.end(); }
};






























//=====================================================================================//
//                                     –≈јЋ»«ј÷»я                                      //
//=====================================================================================//


/////////////////////////////////////////////////////////////////////////
//	конструктор идет по дереву директорий, по замонтированным
//	sub_fs (переформировыва€ path), и ищет файлы, подход€щие под
//	param. «аносит их в контейнер.
template <class Pred>
file_list<Pred>::file_list(const fs_path &path, unsigned param)
{
	if (!path.length())
		throw incorrect_path("fs: filename is empty");
	//	формируем маску и путь
	std::string mask(path.back());
	mask_regex mreg(mask);

	// перебор по директори€м (directory), содержащимс€ в пути
	fs_guider guid(path);
	fs_path where("root/" + path.to_str());	// where нужно передать с отрезанной виртуальной частью
	for(; !guid.is_done(); guid.next())
	{
		directory *tmp_dir = guid.item();
		where.pop_front();	// тут мы еЄ по кусочкам и отрезаем

		// перебор sub_fs внутри директории
		for(directory::const_iterator i = tmp_dir->begin(); i != tmp_dir->end(); ++i)
		{
			// перебор файлов внутри sub_fs
			std::auto_ptr<sub_fs_iter> it = (*i)->new_iterator(where);
			for(; !it->is_done(); ++(*it))
			{
				// проверка на соответствие введенным параметрам поиска
				file_id tmp_id = it->get_id();

				if ((param & fs::dir) && !(tmp_id.is_dir()))
					if (!(param & fs::files))
						continue;
				if ((param & fs::files) && (tmp_id.is_dir()))
					if (!(param & fs::dir))
						continue;
				if ((param & fs::readonly) && !(tmp_id.is_readonly()))
					continue;
				// проверка маски
				if (mreg.is_valid(tmp_id.name()))
				{
					//	перебираем имеющеес€ "хоз€йство" на предмет совпадени€ имени
					bool founded = false;
					for (iterator j = begin(); j != end(); ++j)
					{
						if (tmp_id.path() == j->begin()->path())
						{
							j->insert(tmp_id);
							founded = true;
						}
					}
					if (!founded)
					{
						std::set<file_id, Pred> tmp_set;
						tmp_set.insert(tmp_id);
						m_list.insert(m_list.end(),tmp_set);
					}
				}
			}
		}
	}
	
}



}

}

#endif // !defined(__FILE_LIST_H_INCLUDED_5698059465543420__)