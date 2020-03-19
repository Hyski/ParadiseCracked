#if !defined(__THEME_SESSION_H_INCLUDED_8093570542662288__)
#define __THEME_SESSION_H_INCLUDED_8093570542662288__

class Script;

//=====================================================================================//
//                    class ThemeSession : public ISndThemeSession                     //
//=====================================================================================//
class ThemeSession : public ISndThemeSession
{
	Module *m_module;
	const Script *m_script;
	std::string m_theme;
	bool m_mute;

public:
	ThemeSession(Module *);
	~ThemeSession();

	// —менить музыкальную тему
	virtual void changeTheme(const char *scriptName, const char *newTheme);
	// ¬ключить/выключить музыкальную тему
	virtual void mute(bool bMute = true);
	virtual void Release();

	const Script *script() const { return m_script; }
	const std::string &theme() const { return m_theme; }

	bool isPlaying() { return !m_mute && m_script!=0 && m_theme.size() > 0; }
	bool isMuted() { return m_mute; }
};

#endif // !defined(__THEME_SESSION_H_INCLUDED_8093570542662288__)