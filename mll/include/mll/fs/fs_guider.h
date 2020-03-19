#if !defined(__FS_GUIDER_H_INCLUDED_7552370787048739__)
#define __FS_GUIDER_H_INCLUDED_7552370787048739__


#include "_export_rules.h"
#include "_choose_lib.h"
#include "fs_path.h"
#include "system.h"


namespace mll
{

namespace fs
{

	
class directory;


//=====================================================================================//
//                                   class fs_guider                                   //
//=====================================================================================//
class FILE_SYSTEM_EXPORT fs_guider
{
private:
	system::iterator m_itor;
	fs_path m_path;
	fs_path::const_iterator m_dir;

	void _search();

public:
	fs_guider(const fs_path &path);
	fs_guider(const fs_guider &another);
	~fs_guider() {}

public:
	bool is_done() const { return m_itor == system::instance()->end(); }
	directory *item() const { return *m_itor; }
	system::iterator iter() const { return m_itor; }
	void next();
};

}

}

#endif // !defined(__FS_GUIDER_H_INCLUDED_7552370787048739__)