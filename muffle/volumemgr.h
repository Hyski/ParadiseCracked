#if !defined(__VOLUMEMGR_H_INCLUDED_1107552477397067__)
#define __VOLUMEMGR_H_INCLUDED_1107552477397067__

#include <mll/utils/lcrn_tree.h>

class Volume;

//=====================================================================================//
//                                   class VolumeMgr                                   //
//=====================================================================================//
class VolumeMgr : private noncopyable
{
	typedef mll::utils::lcrn_tree<Volume *> Volumes_t;
	Volumes_t m_tree;

	CriticalSection m_treeGuard;

public:
	typedef Volumes_t::iterator iterator;

public:
	VolumeMgr();
	~VolumeMgr();

	std::auto_ptr<Volume> addVolume(Volume * = 0);
	void removeVolume(Volume *);
	CriticalSection &volumeGuard() { return m_treeGuard; }
};

#endif // !defined(__VOLUMEMGR_H_INCLUDED_1107552477397067__)