#if !defined(__DISCARDED_SOUND_CACHE_INCLUDED__)
#define __DISCARDED_SOUND_CACHE_INCLUDED__

#include "Common.h"

class cc_SndDiscardedCache : public cc_SndCache
{
protected:
	void onActivateSegment(cc_Segment *);
	void onDeactivateSegment(cc_Segment *);

public:
	cc_SndDiscardedCache(cc_SegmentMgr *mgr) : cc_SndCache(mgr) {}
};

#endif