#include "precomp.h"
#include "ThemeMgr2.h"
#include "DX8Sound.h"

#include "Session.h"
#include "Segment.h"
#include "SegmentState.h"
#include "PathMgr.h"

#include "CriticalSection.h"

#include <assert.h>
#include <process.h>
#include <sstream>

#include "DM8Core.h"

cc_CS *cc_ThemeMgr2::m_dataLock = 0;
HANDLE cc_ThemeMgr2::m_transThread;
cc_ThemeMgr2 *cc_ThemeMgr2::m_instance = 0;
//const char *cc_ThemeMgr2::m_LogFile = "gvz_sound_themes.log";
LOG_MEMBER_IMPL(cc_ThemeMgr2,m_log,"sound/themes.log");
#define THM_LOG		C_HOMELOG(m_log)

#define MACRO_TM_STAMP	"<" << STR_TIME_STAMP << "> " << STR_FULL_STAMP

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// cc_ThemeMgr2::cc_ThemeMgr2() //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_ThemeMgr2::cc_ThemeMgr2(cc_PathMgr *pathMgr, cc_SegmentStateMgr *sstateMgr, cc_ScriptMgr *scriptMgr, cc_SegmentMgr *segmentMgr)
:	m_pathMgr(pathMgr),
	m_sstateMgr(sstateMgr),
	m_segmentMgr(segmentMgr),
	m_scriptMgr(scriptMgr)
{
	TRACK_FUNC(cc_ThemeMgr2::~cc_ThemeMgr2());

	m_instance = this;

	m_Active = 0;
	m_Segment = 0;
	m_NextSegment = 0;

	cc_PathMgr::PathParameters pp;
	pp.m_Params = cc_PathMgr::PathParameters::themes;

	m_Path = pathMgr->getAudioPath(pp,ci_Sound::cThemes);

	m_State = 0;
	m_playState = silence;

	m_transThread = NULL;
	m_dataLock = cc_CSEntry::initializeCriticalSection();
	THM_LOG << MACRO_TM_STAMP << "Initialized themes manager\n";
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// cc_ThemeMgr2::~cc_ThemeMgr2() /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_ThemeMgr2::~cc_ThemeMgr2()
{
	TRACK_FUNC(cc_ThemeMgr2::~cc_ThemeMgr2());

	if(!m_Stack.empty())
	{
		std::ostringstream sstr;
		sstr << MACRO_TM_STAMP << "Не завершены все музыкальные сессии" << std::ends;
		throw SND_EXCEPTION(sstr.str().c_str());
	}

	for(;;)
	{
		{
			cc_CSEntry guard(m_dataLock);
			if ((m_playState != transitionAsc) && (m_playState != transitionDesc)) break;
		}

		THM_LOG << MACRO_TM_STAMP << "Waiting transThread to stop...\n";
		WaitForSingleObject(this->m_transThread,INFINITE);
		THM_LOG << MACRO_TM_STAMP << "TransThread stopped!\n";

		break;
	}

	if (m_State)
	{
		m_State->release();
		m_State = 0;
	}

	delete m_Path;
	cc_CSEntry::deleteCriticalSection(m_dataLock);
	THM_LOG << MACRO_TM_STAMP << "Shut down themes manager\n";
}

/*void cc_ThemeMgr2::init()
{
	TRACK_FUNC(cc_ThemeMgr2::init());

	m_Active = 0;
	m_Segment = 0;
	m_NextSegment = 0;

	cc_PathMgr::PathParameters pp;
	pp.m_Params = cc_PathMgr::PathParameters::themes;

	m_Path = cc_DX8Sound::m_Sound->getPathMgr()->getAudioPath(pp,ci_Sound::cThemes);

	m_State = 0;
	m_playState = silence;

	m_transThread = NULL;
	m_dataLock = cc_CSEntry::initializeCriticalSection();
	CORE_LOG(m_LogFile,MACRO_TM_STAMP+"Initialized themes manager\n");
}*/

/*void cc_ThemeMgr2::shut()
{
	TRACK_FUNC(cc_ThemeMgr2::shut());

	if(!m_Stack.empty()) throw SND_EXCEPTION((MACRO_TM_STAMP+"Не завершены все музыкальные сессии").c_str());

	for(;;)
	{
		{
			cc_CSEntry guard(m_dataLock);
			if ((m_playState != transitionAsc) && (m_playState != transitionDesc)) break;
		}

		CORE_LOG(m_LogFile,MACRO_TM_STAMP+"Waiting transThread to stop...\n");
		WaitForSingleObject(this->m_transThread,INFINITE);
		CORE_LOG(m_LogFile,MACRO_TM_STAMP+"TransThread stopped!\n");

		break;
	}

	if (m_State)
	{
		m_State->release();
		m_State = 0;
	}

	delete m_Path;
	cc_CSEntry::deleteCriticalSection(m_dataLock);
	CORE_LOG(m_LogFile,MACRO_TM_STAMP+"Shut down themes manager\n");
}*/

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// cc_ThemeMgr2::noticeThemeChange() ///////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_ThemeMgr2::noticeThemeChange(cc_ThemeSession *session)
{
	TRACK_FUNC(cc_ThemeMgr2::noticeThemeChange());
	if (session != m_Active) return;

	THM_LOG << MACRO_TM_STAMP << "Theme changed\n";
	if (!session)
	{
		THM_LOG << "\t\t\t...session == 0\n";
	}

	{
		cc_CSEntry guard(m_dataLock);
		switch (m_playState)
		{
			case silence: beginTheme(session); break;
			case theme: beginTransition(session); break;
			case transitionDesc: changeTarget(session); break;
			case transitionAsc: redoTransition(session); break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_ThemeMgr2::beginTheme() ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_ThemeMgr2::beginTheme(cc_ThemeSession *session)
{
	TRACK_FUNC(cc_ThemeMgr2::beginTheme());

	if (!session) return;
	if (!session->getPlayingSegment()) return;

	THM_LOG << MACRO_TM_STAMP << "Starting theme from silence\n";

	m_State = new cc_SegmentState(session->getPlayingSegment(),m_sstateMgr);
	{
		float vol = cc_DX8Sound::m_Sound->getVolume(ci_Sound::cThemes);
		m_Path->getPath()->SetVolume(cc_DX8Sound::normToDecibel(vol),0);
	}

	if(!m_State->play(m_Path->getPath()))
	{
		m_State->release();
		m_State = 0;
		m_playState = silence;
	}
	else
	{
		m_playState = theme;
	}

	m_NextSegment = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// cc_ThemeMgr2::beginTransition() ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_ThemeMgr2::beginTransition(cc_ThemeSession *session)
{
	TRACK_FUNC(cc_ThemeMgr2::beginTransition());
	if (!session)
	{
		m_NextSegment = 0;
	}
	else
	{
		m_NextSegment = session->getPlayingSegment();
	}

	THM_LOG << MACRO_TM_STAMP << "Starting theme from other theme\n";
	m_playState = transitionDesc;
	m_transThread = reinterpret_cast<HANDLE>(_beginthread(transThread, 0, reinterpret_cast<void*>(this)));
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// cc_ThemeMgr2::changeTarget() //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_ThemeMgr2::changeTarget(cc_ThemeSession *session)
{
	TRACK_FUNC(cc_ThemeMgr2::changeTarget());

	THM_LOG << MACRO_TM_STAMP << "Changing target theme\n";

	if (!session)
	{
		m_NextSegment = 0;
	}
	else
	{
		m_NextSegment = session->getPlayingSegment();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// cc_ThemeMgr2::redoTransition() /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_ThemeMgr2::redoTransition(cc_ThemeSession *session)
{
	TRACK_FUNC(cc_ThemeMgr2::redoTransition());
	THM_LOG << MACRO_TM_STAMP << "Redoing transition\n";
	m_playState = transitionDesc;
	m_NextSegment = session->getPlayingSegment();
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_ThemeMgr2::transThread() //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void __cdecl cc_ThemeMgr2::transThread(LPVOID ptr)
{
	cc_ThemeMgr2 *mgr = reinterpret_cast<cc_ThemeMgr2*>(ptr);

	mgr->m_volume = 1.0f;

	for(;;)
	{
		{
			cc_CSEntry guard(m_dataLock);

			if (mgr->m_playState == transitionAsc)
			{
				mgr->m_volume += 0.1f;
				float vol = cc_DX8Sound::m_Sound->getVolume(ci_Sound::cThemes)*mgr->m_volume;

				mgr->m_Path->getPath()->SetVolume(cc_DX8Sound::normToDecibel(vol),0);

				if (mgr->m_volume >= 1.0f)
				{
					mgr->m_playState = theme;
					m_transThread = NULL;

					float vol = cc_DX8Sound::m_Sound->getVolume(ci_Sound::cThemes);
					mgr->m_Path->getPath()->SetVolume(cc_DX8Sound::normToDecibel(vol),0);

					break;
				}
			}
			else if (mgr->m_playState == transitionDesc)
			{
				mgr->m_volume -= 0.1f;
				float vol = cc_DX8Sound::m_Sound->getVolume(ci_Sound::cThemes)*mgr->m_volume;

				mgr->m_Path->getPath()->SetVolume(cc_DX8Sound::normToDecibel(vol),0);

				if (mgr->m_volume <= 0.0f)	// достигли нуля -- переключим тему
				{
					mgr->m_State->stop();
					mgr->m_State->release();
					mgr->m_State = 0;

					mgr->m_Segment = mgr->m_NextSegment;
					mgr->m_NextSegment = 0;

					if (!mgr->m_Segment)
					{
						mgr->m_playState = silence;
						m_transThread = NULL;

						float vol = cc_DX8Sound::m_Sound->getVolume(ci_Sound::cThemes);
						mgr->m_Path->getPath()->SetVolume(cc_DX8Sound::normToDecibel(vol),0);

						break;
					}

					mgr->m_State = new cc_SegmentState(mgr->m_Segment,m_instance->m_sstateMgr);
					if(!mgr->m_State->play(mgr->m_Path->getPath()))
					{
						mgr->m_State->release();
						mgr->m_State = 0;
						mgr->m_playState = silence;

						float vol = cc_DX8Sound::m_Sound->getVolume(ci_Sound::cThemes);
						mgr->m_Path->getPath()->SetVolume(cc_DX8Sound::normToDecibel(vol),0);

						break;
					}
					else
					{
						mgr->m_playState = transitionAsc;
					}
				}
			}
		}

		Sleep(100);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// cc_ThemeMgr2::manage() /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_ThemeMgr2::manage()
{
	TRACK_FUNC(cc_ThemeMgr2::manage());
	if (m_State && m_State->isFinished())
	{
		noticeThemeChange(m_Active);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// cc_ThemeMgr2::beginSession() //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_ThemeSession *cc_ThemeMgr2::beginSession()
{
	TRACK_FUNC(cc_ThemeMgr::beginSession());
	cc_ThemeSession *session = new cc_ThemeSession(this);
	m_Stack.push_front(session);
	m_Active = session;
	noticeThemeChange(m_Active);
	return session;
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// cc_ThemeMgr2::deleteSession() /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_ThemeMgr2::deleteSession(cc_ThemeSession *session)
{
	TRACK_FUNC(cc_ThemeMgr2::deleteSession());

	if (session == m_Active)
	{
		m_Stack.erase(m_Stack.begin());
		if (m_Stack.empty())
		{
			m_Active = 0;
		}
		else
		{
			m_Active = *m_Stack.begin();
		}

		noticeThemeChange(m_Active);
	}
	else
	{
		for (sessions_t::iterator itor = m_Stack.begin(); itor != m_Stack.end(); itor++)
		{
			if (*itor == session)
			{
				m_Stack.erase(itor);
				break;
			}
		}
	}
}