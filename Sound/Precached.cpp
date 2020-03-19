#include "precomp.h"
#include "Precached.h"
#include "Segment.h"

void cc_SndPrecachedCache::onActivateSegment(cc_Segment *segment)
{
	segment->addRef();
}

void cc_SndPrecachedCache::onDeactivateSegment(cc_Segment *segment)
{
	segment->release();
}