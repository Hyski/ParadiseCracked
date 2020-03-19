#include "precomp.h"
#include "Segment.h"
#include "SegmentMgr.h"
#include "Delayed.h"
#include "DM8Core.h"

//const char *cc_SegmentMgr::m_LogFile = "gvz_sound_core.log";

cc_SegmentMgr::cc_SegmentMgr()
{
	TRACK_FUNC(cc_SegmentMgr::cc_SegmentMgr());
	HRESULT hr;

	hr = CoCreateInstance(CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, IID_IDirectMusicLoader8, m_Loader.asVoidPtr());

	if (FAILED(hr))
	{
		DMCORE_LOG << STR_FULL_STAMP << "Cannot instantiate DirectMusicLoader8\n";
		throw cc_SoundError(hr,"Cannot instantiate DirectMusicLoader8 due to following reason");
	}

	DMCORE_LOG << STR_FULL_STAMP << "Created DirectMusicLoader8 instance\n";

	SAFE_CALL(m_Loader->EnableCache(GUID_DirectMusicAllTypes,FALSE));
	SAFE_CALL(m_Loader->ClearCache(GUID_DirectMusicAllTypes));

	m_Delayed = new cc_SndDelayedCache(this);

	DMCORE_LOG << STR_FULL_STAMP << "Segment Manager initialized\n";
}

cc_SegmentMgr::~cc_SegmentMgr()
{
	TRACK_FUNC(cc_SegmentMgr::~cc_SegmentMgr());
	delete m_Delayed;
	m_Loader.Release();

	segments_t::iterator itor = m_Segments.begin();
	for(; itor != m_Segments.end(); ++itor)
	{
		delete itor->second;
	}

	DMCORE_LOG << STR_FULL_STAMP << "Segment Manager closed\n";
}

/*void cc_SegmentMgr::init()
{
	TRACK_FUNC(cc_SegmentMgr::init());
	if (FAILED(CoCreateInstance(CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, IID_IDirectMusicLoader8, m_Loader.asVoidPtr())))
	{
		CORE_WARN_LOG(m_LogFile, STR_FULL_STAMP+"Cannot instantiate DirectMusicLoader8\n");
	}
	else
	{
		CORE_LOG(m_LogFile, STR_FULL_STAMP+"Created DirectMusicLoader8 instance\n");
	}

	SAFE_CALL(m_Loader->EnableCache(GUID_DirectMusicAllTypes,FALSE));
	SAFE_CALL(m_Loader->ClearCache(GUID_DirectMusicAllTypes));

	m_Delayed = new cc_SndDelayedCache;

	CORE_LOG(m_LogFile, STR_FULL_STAMP+"Segment Manager initialized\n");
}

void cc_SegmentMgr::shut()
{
	TRACK_FUNC(cc_SegmentMgr::shut());
	delete m_Delayed;
	m_Loader.Release();

	segments_t::iterator itor = m_Segments.begin();
	for(; itor != m_Segments.end(); ++itor)
	{
		delete itor->second;
	}

	CORE_LOG(m_LogFile, STR_FULL_STAMP+"Segment Manager closed\n");
}*/

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SegmentMgr::getSegment() //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_SegmentMgr::ListedSegment *cc_SegmentMgr::getSegment(const char *name)
{
	// Переведем строку в нижний регистр
	std::string tmp(name);
	std::use_facet< std::ctype<char> >(std::locale()).tolower(tmp.begin(),tmp.end());
	std::replace(tmp.begin(), tmp.end(), '\\', '/');

	segments_t::iterator itor = m_Segments.find(tmp);
	if (itor != m_Segments.end()) return itor->second;

	ListedSegment *seg = m_Segments[tmp] = new ListedSegment(this,tmp.c_str());
	m_Delayed->addSegment(seg);
	return seg;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// cc_SegmentMgr::getSegmentRef() /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_SegmentMgr::ListedSegment *cc_SegmentMgr::getSegmentRef(const char *name)
{
	// Переведем строку в нижний регистр
	std::string tmp(name);
	std::use_facet< std::ctype<char> >(std::locale()).tolower(tmp.begin(),tmp.end());
	std::replace(tmp.begin(), tmp.end(), '\\', '/');

	segments_t::iterator itor = m_Segments.find(tmp);
	if (itor != m_Segments.end()) return itor->second;

	ListedSegment *seg = m_Segments[tmp] = new ListedSegment(this,tmp.c_str());
	return seg;
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SegmentMgr::getLoader () //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
IDirectMusicLoader8 *cc_SegmentMgr::getLoader ()
{
	return m_Loader;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// cc_SegmentMgr::getDefaultCache() ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_SndDelayedCache *cc_SegmentMgr::getDefaultCache()
{
	return m_Delayed;
}