#if !defined(__FS_EXISTENDER_H_INCLUDED_7552370787048739__)
#define __FS_EXISTENDER_H_INCLUDED_7552370787048739__

#include <mll/fs/_export_rules.h>
#include <mll/fs/_choose_lib.h>

#include "directory.h"
#include "fs_path.h"
#include "fs_guider.h"


namespace mll
{

namespace fs
{

	
class sub_fs;


//=====================================================================================//
//                                 class fs_existender                                 //
//=====================================================================================//
class fs_existender
{
private:
	const sub_fs *m_sub_fs;
	fs_path m_sub_path;
	
public:
	fs_existender(const fs_path &path);
	~fs_existender() {}

public:
	const sub_fs *get_sub_fs() const { return m_sub_fs; }
	const fs_path &get_sub_path() const { return m_sub_path; }
};






























//=====================================================================================//
//                                     –≈јЋ»«ј÷»я                                      //
//=====================================================================================//


/////////////////////////////////////////////////////////////////////////
//	конструктор перебирает поступивший path с начала, и вытаскивает 
//	последнюю sub_fs, "вис€щую" на пути. ¬ m_sub_path остаетс€ 
//	"остаток" path.
inline fs_existender::fs_existender(const fs_path &path)
:	m_sub_fs(0), m_sub_path("root/" + path.to_str())
{
	//	найти sub_fs, чтобы бќльша€ часть path приходилась на виртуальные директории
	directory *tmp_dir;
	fs_guider guid(path);
	for(; !guid.is_done(); guid.next())
	{
		tmp_dir = guid.item();
		m_sub_path.pop_front();
		for (directory::const_iterator i = tmp_dir->begin(); i != tmp_dir->end(); ++i)
		{
			if (!(*i)->system_readonly())
				m_sub_fs = *i;
		}
	}
	if (!m_sub_fs)
		throw no_writable("fs: no writable subsystems, can't create file");
}



}

}

#endif // !defined(__FS_EXISTENDER_H_INCLUDED_7552370787048739__)