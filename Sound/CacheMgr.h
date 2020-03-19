#if !defined(__CACHE_MANAGER_INCLUDED__)
#define __CACHE_MANAGER_INCLUDED__

#include <map>

class cc_SndCache;

class cc_CacheMgr
{
	static const char *m_LogFile;

	typedef std::map<std::string,cc_SndCache *> caches_t;
	caches_t m_Caches;

public:
	void init();
	void shut();

	cc_SndCache *getCache(const char *);
	void addCache(const char *, cc_SndCache *);
};

#endif