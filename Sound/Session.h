#if !defined(__THEME_SESSION_INCLUDED__)
#define __THEME_SESSION_INCLUDED__

class cc_SndScript;
class cc_Segment;
class cc_ThemeMgr2;

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// class cc_ThemeSession /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class cc_ThemeSession : public ci_SndThemeSession
{
	cc_ThemeMgr2 *m_mgr;
	cc_SndScript *m_Script;
	cc_Segment *m_Segment;
	bool m_bMuted;

public:
	cc_ThemeSession(cc_ThemeMgr2 *);

	void changeTheme(const char *scriptName, const char *newTheme);
	void mute(bool);
	void Release();

	cc_SndScript *getPlayingScript();
	cc_Segment *getPlayingSegment();
};

#endif