#include "precomp.h"
#include "volumemgr.h"
#include "volume.h"

//=====================================================================================//
//                               VolumeMgr::VolumeMgr()                                //
//=====================================================================================//
VolumeMgr::VolumeMgr()
{
}

//=====================================================================================//
//                               VolumeMgr::~VolumeMgr()                               //
//=====================================================================================//
VolumeMgr::~VolumeMgr()
{
	Entry guard(m_treeGuard);
	m_tree.clear();
}

//=====================================================================================//
//                     VolumeMgr::iterator VolumeMgr::addVolume()                      //
//=====================================================================================//
std::auto_ptr<Volume> VolumeMgr::addVolume(Volume *vol)
{
	Entry guard(m_treeGuard);
	iterator parent = vol ? vol->m_me : m_tree.end();
	iterator item = m_tree.insert_son(parent,0);
	*item = new Volume(this,item);
	return std::auto_ptr<Volume>(*item);
}

//=====================================================================================//
//                           void VolumeMgr::removeVolume()                            //
//=====================================================================================//
void VolumeMgr::removeVolume(Volume *vol)
{
	Entry guard(m_treeGuard);
	assert( !vol->m_me.has_son() );
	m_tree.erase(vol->m_me);
}