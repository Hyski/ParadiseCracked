#include "precomp.h"
#include "CacheMgr.h"
#include "Common.h"

const char *cc_CacheMgr::m_LogFile = "gvz_sound_core.log";

void cc_CacheMgr::init()
{
	CORE_LOG(m_LogFile, "cc_CacheMgr::init() - Cache manager initialized and ready to cache\n");
}

void cc_CacheMgr::shut()
{
	caches_t::iterator itor = m_Caches.begin();

	for (; itor != m_Caches.end(); itor++)
	{
		CORE_LOG(m_LogFile, ("cc_CacheMgr::shut() - Deleted cache "+itor->first+"\n").c_str());
		delete itor->second;
	}

	m_Caches.clear();

	CORE_LOG(m_LogFile, "cc_CacheMgr::shut() - Cache manager uninitialized\n");
}

cc_SndCache *cc_CacheMgr::getCache(const char *name)
{
	caches_t::iterator itor = m_Caches.find(name);
	if (itor == m_Caches.end()) return itor->second;
	return 0;
}

void cc_CacheMgr::addCache(const char *name, cc_SndCache *cache)
{
	if (m_Caches.find(name) != m_Caches.end())
	{
		CORE_WARN_LOG(m_LogFile, "cc_CacheMgr::addCache() - Duplicating cache names. Cache not added\n");
		return;
	}

	m_Caches[name] = cache;
	CORE_LOG(m_LogFile, ("cc_CacheMgr::addCache() - Added cache "+std::string(name)+"\n").c_str());
}