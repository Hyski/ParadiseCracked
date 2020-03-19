#include "precomp.h"
#include "Segment.h"
#include "SegmentMgr.h"
#include "DX8Sound.h"
#include "Delayed.h"
#include <assert.h>

#include "DM8Core.h"

const char *cc_Segment::m_LogFile = "gvz_sound_segments.log";
static const char *g_failedSegmentsLog = "failed_to_load.log";

unsigned cc_Segment::m_segCount = 0;

//////////////////////////// cc_Segment::cc_Segment() //////////////////////////////////
cc_Segment::cc_Segment(cc_SegmentMgr *mgr)
:	m_mgr(mgr),
	m_refCount(0)
{
	TRACK_FUNC(cc_Segment::cc_Segment());
	static char buffer[128];
	sprintf(buffer,"gsndBuf%u",m_segCount++);
	m_name = buffer;
	CORE_LOG(m_LogFile,"<"+STR_TIME_STAMP+">"+STR_FULL_STAMP+"Created segment ["+m_name+"] ");
}

//////////////////////////// cc_Segment::~cc_Segment() /////////////////////////////////
cc_Segment::~cc_Segment()
{
	TRACK_FUNC(cc_Segment::~cc_Segment());
	CORE_LOG(m_LogFile,"<"+STR_TIME_STAMP+">"+STR_FULL_STAMP+"Destroyed segment ["+m_name+"]\n");
}

//////////////////////////// cc_Segment::addRef() //////////////////////////////////////
void cc_Segment::addRef()
{
	TRACK_FUNC(cc_Segment::addRef());
	++m_refCount;
	CORE_LOG(m_LogFile,"<"+STR_TIME_STAMP+"> Increased reference counter for ["+m_name+"]\n");
}

//////////////////////////// cc_Segment::release() /////////////////////////////////////
void cc_Segment::release()
{
	TRACK_FUNC(cc_Segment::release());
	m_refCount--;
	CORE_LOG(m_LogFile,"<"+STR_TIME_STAMP+"> Decreased reference counter for ["+m_name+"]\n");
}

//////////////////////////// cc_Segment::rename() //////////////////////////////////////
void cc_Segment::rename(const char *name)
{
	TRACK_FUNC(cc_Segment::rename());
	CORE_LOG(m_LogFile,"<"+STR_TIME_STAMP+"> Segment ["+m_name+"] ");
	m_name = name;
	CORE_LOG(m_LogFile,"renamed to ["+m_name+"]\n");
}

//////////////////////////// cc_SegFromFile::cc_SegFromFile() //////////////////////////
cc_SegFromFile::cc_SegFromFile(cc_SegmentMgr *mgr, const char *file)
:	cc_Segment(mgr),
	m_bInCache(false)
{
	TRACK_FUNC(cc_SegFromFile::cc_SegFromFile());
	CORE_LOG(m_LogFile,"from file\n");
	rename(file);
}

//////////////////////////// cc_SegFromFile::addRef() //////////////////////////////////
void cc_SegFromFile::addRef()
{
	TRACK_FUNC(cc_SegFromFile::addRef());
	cc_Segment::addRef();
	if (getRefCount() == 1) load();
}

//////////////////////////// cc_SegFromFile::release() /////////////////////////////////
void cc_SegFromFile::release()
{
	TRACK_FUNC(cc_SegFromFile::release());
	cc_Segment::release();
	if (getRefCount() == 0) unload();
}

//////////////////////////// cc_SegFromFile::addedToCache() ////////////////////////////
void cc_SegFromFile::addedToCache()
{
	TRACK_FUNC(cc_SegFromFile::addedToCache());
	assert(!m_bInCache);
	m_bInCache = true;
	CORE_LOG(m_LogFile,"<"+STR_TIME_STAMP+">"+STR_FULL_STAMP+"Placed ["+getName()+"] in cache\n");
}

//////////////////////////// cc_SegFromFile::kickedFromCache() /////////////////////////
void cc_SegFromFile::kickedFromCache()
{
	TRACK_FUNC(cc_SegFromFile::kickedFromCache());
	assert(m_bInCache);
	m_bInCache = false;
	CORE_LOG(m_LogFile,"<"+STR_TIME_STAMP+">"+STR_FULL_STAMP+"Removed ["+getName()+"] in cache\n");
}

//////////////////////////// cc_SegFromFile::load() ////////////////////////////////////
void cc_SegFromFile::load()
{
	TRACK_FUNC(cc_SegFromFile::load());
	cc_SegmentMgr *sm = m_mgr;

	WCHAR buffer[DMUS_MAX_FILENAME];
	MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,getName().c_str(), -1, buffer, DMUS_MAX_FILENAME);

	if (FAILED(sm->getLoader()->LoadObjectFromFile(CLSID_DirectMusicSegment,
		IID_IDirectMusicSegment8,buffer,m_segment.asVoidPtr())))
	{
		cc_EnvWrapper::log(g_failedSegmentsLog,"Can't load file "+getName()+"\n");
		CORE_WARN_LOG(m_LogFile,"<"+STR_TIME_STAMP+">"+STR_FULL_STAMP+"Failed to load segment ["+getName()+"]\n");
	}
	else
	{
		CORE_LOG(m_LogFile,"<"+STR_TIME_STAMP+">"+STR_FULL_STAMP+"Loaded segment ["+getName()+"]\n");
	}
}

//////////////////////////// cc_SegFromFile::unload() //////////////////////////////////
void cc_SegFromFile::unload()
{
	TRACK_FUNC(cc_SegFromFile::unload());
	if (m_segment)
	{
		m_segment.Release();
		CORE_LOG(m_LogFile,"<"+STR_TIME_STAMP+">"+STR_FULL_STAMP+"Released segment ["+getName()+"]\n");
	}
}

//////////////////////////// cc_SegGenerated::cc_SegGenerated() ////////////////////////
cc_SegGenerated::cc_SegGenerated(cc_SegmentMgr *mgr, IDirectMusicSegment8 *segment)
:	cc_Segment(mgr)
{
	TRACK_FUNC(cc_SegGenerated::cc_SegGenerated());
	CORE_LOG(m_LogFile,"from scratch\n");
	m_segment = segment;
	m_segment->AddRef();
}

//////////////////////////// cc_SegGenerated::~cc_SegGenerated() ///////////////////////
cc_SegGenerated::~cc_SegGenerated()
{
}