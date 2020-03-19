#if !defined(__THEME_MANAGER_2_INCLUDED__)
#define __THEME_MANAGER_2_INCLUDED__

#include <dmusici.h>
#include <list>
#include COMPTR_H

class cc_ThemeSession;

class cc_Segment;

/*//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// cc_ThemePart ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
class cc_ThemePart
{
	cc_COMPtr<IDirectMusicSegmentState8> m_segState;
//	bool m_alreadyStarted;

public:
	cc_ThemePart(IDirectMusicSegmentState8 *segState)
	:	m_segState(segState), 
		m_alreadyStarted(false) {}

	virtual ~cc_ThemePart() {}
	bool isFinished();
};

//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// cc_SegmentBased ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
class cc_SegmentBased : public cc_ThemePart
{
	cc_Segment *m_segment;

public:
	cc_SegmentBased(cc_Segment *, IDirectMusicSegmentState8 *);
	~cc_SegmentBased();
};

//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// cc_AutoBased ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
class cc_AutoBased : public cc_ThemePart
{
	cc_COMPtr<IDirectMusicSegment8> m_segment;

public:
	cc_AutoBased(IDirectMusicSegment8 *, IDirectMusicSegmentState8 *);
	~cc_AutoBased();
};*/

class cc_SegmentState;
class cc_AudioPath;
class cc_CS;
class cc_PathMgr;
class cc_SegmentStateMgr;
class cc_SegmentMgr;
class cc_ScriptMgr;

//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// cc_ThemeMgr2 ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// Реализует плавную смену тем
class cc_ThemeMgr2
{
//	static const char *m_LogFile;
	static cc_ThemeMgr2 *m_instance;

	DECLARE_LOG_MEMBER(m_log);
	cc_PathMgr *m_pathMgr;
	cc_SegmentStateMgr *m_sstateMgr;
	cc_ScriptMgr *m_scriptMgr;
	cc_SegmentMgr *m_segmentMgr;

	typedef std::list<cc_ThemeSession *> sessions_t;
	sessions_t m_Stack;

	cc_ThemeSession *m_Active;	// Активная сессия
	cc_Segment *m_Segment;		// Играющий сегмент
	cc_Segment *m_NextSegment;	// Следующий сегмент
	cc_AudioPath *m_Path;		// AudioPath, на котором все играется
	cc_SegmentState *m_State;	// Стейт, играющий на данный момент

	static cc_CS *m_dataLock;
	static void __cdecl	transThread(void *);
	static HANDLE m_transThread;

	enum {silence, theme, transitionAsc, transitionDesc} m_playState;

	void beginTheme(cc_ThemeSession *);
	void beginTransition(cc_ThemeSession *);
	void changeTarget(cc_ThemeSession *);
	void redoTransition(cc_ThemeSession *);

	float m_volume;

public:
	cc_ThemeMgr2(cc_PathMgr *, cc_SegmentStateMgr *, cc_ScriptMgr *, cc_SegmentMgr *);
	~cc_ThemeMgr2();

//	void init();
	void manage();
//	void shut();

	void noticeThemeChange(cc_ThemeSession *);
	void deleteSession(cc_ThemeSession *);

//	IDirectMusicAudioPath8 *getPath() {return m_Path->;}

	cc_ThemeSession *beginSession();

	inline cc_PathMgr *pathMgr() { return m_pathMgr; }
	inline cc_SegmentStateMgr *sstateMgr() { return m_sstateMgr; }
	inline cc_ScriptMgr *scriptMgr() { return m_scriptMgr; }
	inline cc_SegmentMgr *segmentMgr() { return m_segmentMgr; }
};

#endif