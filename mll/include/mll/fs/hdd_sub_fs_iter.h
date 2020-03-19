#if !defined(__HDD_SUB_FS_ITER_H_INCLUDED_6491963142910359__)
#define __HDD_SUB_FS_ITER_H_INCLUDED_6491963142910359__


#include "_export_rules.h"
#include "_choose_lib.h"

#include <mll/fs/sub_fs_iter.h>

#include <io.h>


namespace mll
{

namespace fs
{


//=====================================================================================//
//                                class hdd_sub_fs_iter                                //
//=====================================================================================//
class FILE_SYSTEM_EXPORT hdd_sub_fs_iter: public sub_fs_iter
{
private:
	_finddata_t m_buf;
	long m_hFind;
	std::string m_search_path;
	std::string m_rel_path;
	const sub_fs *m_sub_fs;
	
public:
	hdd_sub_fs_iter(const std::string& disk_path,
					const std::string& rel_path,
					const sub_fs *sfs);
	~hdd_sub_fs_iter();
	
	bool is_done();
	virtual sub_fs_iter &operator++();
	file_id get_id();
};

}

}

#endif // !defined(__HDD_SUB_FS_ITER_H_INCLUDED_6491963142910359__)