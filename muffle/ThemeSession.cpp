#include "precomp.h"
#include "ThemeSession.h"
#include "ThemeManager.h"
#include "ScriptMgr.h"

//=====================================================================================//
//                            ThemeSession::ThemeSession()                             //
//=====================================================================================//
ThemeSession::ThemeSession(Module *module)
:	m_module(module),
	m_script(0),
	m_mute(false)
{
}

//=====================================================================================//
//                            ThemeSession::~ThemeSession()                            //
//=====================================================================================//
ThemeSession::~ThemeSession()
{
}

//=====================================================================================//
//                            void ThemeSession::Release()                             //
//=====================================================================================//
void ThemeSession::Release()
{
	m_module->getThemeManager()->removeSession(this);
	delete this;
}

//=====================================================================================//
//                              void ThemeSession::mute()                              //
//=====================================================================================//
void ThemeSession::mute(bool bmute)
{
	if(m_mute != bmute)
	{
		m_mute = bmute;
		m_module->getThemeManager()->notify(this);
	}
}

//=====================================================================================//
//                          void ThemeSession::changeTheme()                           //
//=====================================================================================//
void ThemeSession::changeTheme(const char *scriptName, const char *newTheme)
{
	m_script = &m_module->getScriptMgr()->get(scriptName);
	if(newTheme)
	{
		m_theme = newTheme;
	}
	else
	{
		m_theme = m_script->fileName();
	}
	m_module->getThemeManager()->notify(this);
}