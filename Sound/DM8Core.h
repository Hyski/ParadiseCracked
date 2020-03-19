#if !defined(__DIRECT_MUSIC_8_CORE_INCLUDED__)
#define __DIRECT_MUSIC_8_CORE_INCLUDED__

#include "coreDS8.h"
#include "coreDM8.h"
#include "ScriptMgr.h"
#include "Spy.h"
#include "SegmentMgr.h"
#include "ThemeMgr2.h"
#include "PathMgr.h"
#include "EmitMgr.h"
#include "CacheMgr.h"
#include "SegmentStateMgr.h"

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// class cc_DM8Core ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class cc_DM8Core
{
	friend class cc_DX8Sound;

	cc_DirectSound m_dsound;
	cc_DirectMusic m_dmusic;
	cc_SegmentMgr m_segmentMgr;
	cc_ScriptMgr m_scriptMgr;
	cc_SndSpy m_spy;
	cc_PathMgr m_pathMgr;
	cc_EmitMgr m_emitMgr;
	cc_SegmentStateMgr m_sstateMgr;
	cc_ThemeMgr2 m_themeMgr;
	cc_CacheMgr m_cacheMgr;

	cc_DM8Core(HWND);
	~cc_DM8Core();

public:
	DECLARE_LOG_MEMBER(m_log);

//	inline cc_ScriptMgr &scriptMgr() { return m_scriptMgr; }
	inline cc_DirectMusic &music() { return m_dmusic; }
	inline cc_DirectSound &sound() { return m_dsound; }
/*	inline cc_SndSpy &spy() { return m_spy; }
	inline cc_SegmentMgr &segmentMgr() { return m_segmentMgr; }
	inline cc_ThemeMgr &themeMgr() { return m_themeMgr; }
	inline cc_EmitMgr &emitMgr() { return m_emitMgr; }
	inline cc_CacheMgr &cacheMgr() { return m_cacheMgr; }
	inline cc_PathMgr &pathMgr() { return m_pathMgr; }*/
};

#define DMCORE_LOG	C_HOMELOG(cc_DM8Core::m_log)

#endif