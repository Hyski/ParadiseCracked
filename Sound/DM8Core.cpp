#include "precomp.h"
#include "DM8Core.h"

LOG_MEMBER_IMPL(cc_DM8Core,m_log,"sound/core.log");

cc_DM8Core::cc_DM8Core(HWND mainWnd)
:	m_dsound(mainWnd),
	m_dmusic(mainWnd,&m_dsound),
	m_segmentMgr(),
	m_scriptMgr(&m_segmentMgr),
	m_spy(&m_dmusic),
	m_pathMgr(&m_dmusic),
	m_emitMgr(&m_segmentMgr,&m_pathMgr,&m_sstateMgr),
	m_sstateMgr(&m_spy,&m_emitMgr,&m_dmusic),
	m_themeMgr(&m_pathMgr,&m_sstateMgr,&m_scriptMgr,&m_segmentMgr),
	m_cacheMgr()
{
	TRACK_FUNC(cc_DM8Core::cc_DM8Core());
	DMCORE_LOG << STR_FULL_STAMP << "Core fully initialized\n";
}

cc_DM8Core::~cc_DM8Core()
{
	DMCORE_LOG << STR_FULL_STAMP << "Starting core destruction\n";
}