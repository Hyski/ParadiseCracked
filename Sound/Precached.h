#if !defined(__PRECACHED_CACHE_INCLUDED__)
#define __PRECACHED_CACHE_INCLUDED__

#include "Common.h"

class cc_SndPrecachedCache : public cc_SndCache
{
protected:
	void onActivateSegment(cc_Segment *);
	void onDeactivateSegment(cc_Segment *);

public:
	cc_SndPrecachedCache(cc_SegmentMgr *mgr) : cc_SndCache(mgr) {}
};

#endif