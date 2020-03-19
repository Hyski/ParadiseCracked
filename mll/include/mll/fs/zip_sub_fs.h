#if !defined(__ZIP_SUB_FS_H_INCLUDED_7552370787048739__)
#define __ZIP_SUB_FS_H_INCLUDED_7552370787048739__


#include "_export_rules.h"
#include "_choose_lib.h"

#include <mll/fs/sub_fs.h>


namespace ml_encrypted_zip { class zip_file; }


namespace mll
{

namespace fs
{


//=====================================================================================//
//                                  class zip_sub_fs                                   //
//=====================================================================================//
class FILE_SYSTEM_EXPORT zip_sub_fs: public sub_fs
{
private:
	std::auto_ptr<ml_encrypted_zip::zip_file> m_zip;
	std::string m_real_path;
	std::auto_ptr<sub_fs::priority_handler> m_priority_handler;

public:
	zip_sub_fs(const std::string &real_path, const std::string &pass = "");
	zip_sub_fs(std::istream &file, const std::string &pass = "");
	~zip_sub_fs();
	void set_priority_handler(std::auto_ptr<sub_fs::priority_handler> handler);

private:
	zip_sub_fs(const zip_sub_fs &a);
	zip_sub_fs &operator=(const zip_sub_fs &a);

public:
	const std::string& disk_path() const;
	bool system_readonly() const;
	bool file_exists(const fs_path &name) const;
	std::auto_ptr<sub_fs_iter> new_iterator(const fs_path &) const;
	unsigned file_size(const fs_path &name) const;
	bool is_readonly(const fs_path &name) const;
	time_t date(const fs_path &name) const;
	bool is_dir(const fs_path &name) const;
	std::auto_ptr<std::istream> r_open_file(const fs_path &path, bool seekable) const;
	std::auto_ptr<std::ostream> w_open_file(const fs_path &path, bool append,
		bool seekable) const;
	void delete_file(const fs_path &name) const;
	void make_dir(const fs_path &path) const;
	sub_fs::sized_data read_file(const fs_path &path) const;

public:		//	up_to_subsystem_priority
	int priority() const;
};

}

}

#endif // !defined(__ZIP_SUB_FS_H_INCLUDED_7552370787048739__)