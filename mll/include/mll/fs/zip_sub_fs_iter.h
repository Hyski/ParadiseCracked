#if !defined(__ZIP_SUB_FS_ITER_H_INCLUDED_8093570542662288__)
#define __ZIP_SUB_FS_ITER_H_INCLUDED_8093570542662288__


#include "_export_rules.h"

#include <mll/fs/sub_fs_iter.h>


namespace ml_encrypted_zip { class zip_file; }


namespace mll
{

namespace fs
{


//=====================================================================================//
//                                class zip_sub_fs_iter                                //
//=====================================================================================//
class FILE_SYSTEM_EXPORT zip_sub_fs_iter: public sub_fs_iter
{
private:
	ml_encrypted_zip::zip_file *m_zip;
	typedef std::list<file_id> files;
	files m_files;
	files::iterator m_it;
	const sub_fs *m_sfs;
	
public:
	zip_sub_fs_iter(ml_encrypted_zip::zip_file *file,
					const std::string &rel_path,
					const sub_fs *sfs);
	~zip_sub_fs_iter() {}
	
	bool is_done();
	virtual sub_fs_iter &operator++();
	file_id get_id();
};

}

}

#endif // !defined(__ZIP_SUB_FS_ITER_H_INCLUDED_8093570542662288__)