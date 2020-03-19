#if !defined(__DELAYED_SOUND_CACHE_INCLUDED__)
#define __DELAYED_SOUND_CACHE_INCLUDED__

#include "Common.h"

class cc_SndDelayedCache : public cc_SndCache
{
protected:
	void onActivateSegment(cc_Segment *);
	void onDeactivateSegment(cc_Segment *);

public:
	cc_SndDelayedCache(cc_SegmentMgr *mgr) : cc_SndCache(mgr) {}
	void addSegment(cc_Segment *);
	void clean();
};

#endif