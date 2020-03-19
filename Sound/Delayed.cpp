#include "precomp.h"
#include "Delayed.h"
#include "Segment.h"

void cc_SndDelayedCache::onActivateSegment(cc_Segment *segment)
{
	segment->addRef();
}

void cc_SndDelayedCache::onDeactivateSegment(cc_Segment *segment)
{
	segment->release();
}

void cc_SndDelayedCache::clean()
{
	deactivate();
	m_Segments.clear();
	activate();
}

void cc_SndDelayedCache::addSegment(cc_Segment *seg)
{
	files(seg->getName().c_str());
	onActivateSegment(seg);
}