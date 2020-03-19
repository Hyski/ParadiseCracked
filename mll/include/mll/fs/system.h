#if !defined(__SYSTEM_H_INCLUDED_7552370787048739__)
#define __SYSTEM_H_INCLUDED_7552370787048739__


#include "_export_rules.h"
#include "_choose_lib.h"

#include "sub_fs_id.h"
#include "fs_path.h"

#include <mll/utils/lcrn_tree.h>
#include <mll/debug/exception.h>



namespace mll
{

namespace fs
{

	
class directory;
class sub_fs;
class cache;
class critical_section;


//=====================================================================================//
//                                    class system                                     //
//=====================================================================================//
class FILE_SYSTEM_EXPORT system
{
public:
	
protected:
	static system *m_instance;
	virtual ~system() {}
	
public:
	static void make_fs();
	static void destroy_fs();
	static system *instance();
	
public:
	typedef mll::utils::lcrn_tree<directory *> dir_tree;
	typedef dir_tree::const_iterator const_iterator;
	typedef dir_tree::iterator iterator;

public:
	virtual iterator begin() = 0;
	virtual iterator end() = 0;
	virtual const_iterator begin() const = 0;
	virtual const_iterator end() const = 0;

	virtual void _create_path(const fs_path &path) = 0;
	virtual iterator find_dir(const fs_path &where) const = 0;

	virtual sub_fs_id mount(std::auto_ptr<sub_fs> what, const fs_path &where) = 0;
	virtual std::auto_ptr<sub_fs> unmount(sub_fs_id id) = 0;

	virtual mll::fs::cache* cache() = 0;
	
	virtual critical_section *get_cs() const = 0;
};


//	исключения всякого рода
typedef mll::debug::exception_template<struct fs_failure> failure;
typedef mll::debug::exception_template<struct fs_no_file,failure> no_file;
typedef mll::debug::exception_template<struct fs_stream_fail,failure> stream_fail;
typedef mll::debug::exception_template<struct fs_no_mounted,failure> no_mounted;
typedef mll::debug::exception_template<struct fs_no_writable,failure> no_writable;
typedef mll::debug::exception_template<struct fs_incorrect_path,failure> incorrect_path;
typedef mll::debug::exception_template<struct fs_zip_restrictions,failure> zip_restrictions;


}

}

#endif // !defined(__SYSTEM_H_INCLUDED_7552370787048739__)