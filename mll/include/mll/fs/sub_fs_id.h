#if !defined(__SUB_FS_ID_H_INCLUDED_8093570542662288__)
#define __SUB_FS_ID_H_INCLUDED_8093570542662288__


#include <mll/utils/lcrn_tree.h>

#include <list>


namespace mll
{

namespace fs
{

	
class directory;
class sub_fs;


//=====================================================================================//
//                                   class sub_fs_id                                   //
//=====================================================================================//
class sub_fs_id
{
private:
	typedef mll::utils::lcrn_tree<directory *>::iterator tree_iter;
	typedef std::list<sub_fs *>::iterator dir_iter;
	
private:
	tree_iter m_tree_iter;
	dir_iter m_dir_iter;
	
public:
	sub_fs_id() {}
	sub_fs_id(tree_iter it1, dir_iter it2)
		: m_tree_iter(it1), m_dir_iter(it2) {}
	sub_fs_id(const sub_fs_id &a)
		:	m_tree_iter(a.m_tree_iter), m_dir_iter(a.m_dir_iter) {}
	~sub_fs_id() {}

public:
	sub_fs_id &operator=(const sub_fs_id &a)
	{
		if (this != &a)
		{
			this->~sub_fs_id();
			new(this) sub_fs_id(a);
		}
		return *this;
	}

public:
	tree_iter get_tree_iter() { return m_tree_iter; }
	dir_iter get_dir_iter() { return m_dir_iter; }
};

}

}

#endif // !defined(__SUB_FS_ID_H_INCLUDED_8093570542662288__)