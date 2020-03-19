#if !defined(__THEME_MANAGER_H_INCLUDED_8093570542662288__)
#define __THEME_MANAGER_H_INCLUDED_8093570542662288__

#include <list>

class ThemeSession;
class Emitter;
class Script;

//=====================================================================================//
//                                 class ThemeManager                                  //
//=====================================================================================//
class ThemeManager : private noncopyable
{
	Module *m_module;
	CriticalSection m_sessGuard;

	Emitter *m_emitter;
	const Script *m_script;
	std::string m_wave;

	typedef std::list<ThemeSession *> Sessions_t;
	Sessions_t m_sessions;

	void notifyThemeChange();
	bool isThemeChanged();

public:
	ThemeManager(Module *);
	~ThemeManager();

	void addSession(ThemeSession *);
	void removeSession(ThemeSession *);
	void notify(ThemeSession *);
};

#endif // !defined(__THEME_MANAGER_H_INCLUDED_8093570542662288__)