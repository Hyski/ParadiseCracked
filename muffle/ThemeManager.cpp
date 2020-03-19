#include "precomp.h"
#include "ThemeManager.h"
#include "ThemeSession.h"
#include "Emitter.h"
#include "Muffle.h"
#include "Script.h"
#include "Fade.h"

namespace
{
	long fadeCount = 0;
}

//=====================================================================================//
//                            ThemeManager::ThemeManager()                             //
//=====================================================================================//
ThemeManager::ThemeManager(Module *module)
:	m_module(module),
	m_emitter(0),
	m_script(0)
{
}

//=====================================================================================//
//                            ThemeManager::~ThemeManager()                            //
//=====================================================================================//
ThemeManager::~ThemeManager()
{
#if !defined(MUFFLE_WAIT_FOR_MUSIC_FADE_OUT)
	Fade::destruct();
#endif
}

//=====================================================================================//
//                         bool ThemeManager::isThemeChanged()                         //
//=====================================================================================//
bool ThemeManager::isThemeChanged()
{
	if(m_sessions.empty())
	{
		if(m_emitter == 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	ThemeSession *session = m_sessions.front();
	if((m_emitter != 0) != session->isMuted()) return true;
	if(m_script != session->script()) return true;
	if(m_wave != session->theme()) return true;

	return false;
}

//=====================================================================================//
//                           void ThemeManager::addSession()                           //
//=====================================================================================//
void ThemeManager::addSession(ThemeSession *session)
{
	Entry guard(m_sessGuard);
	m_sessions.push_front(session);
	notifyThemeChange();
}

//=====================================================================================//
//                         void ThemeManager::removeSession()                          //
//=====================================================================================//
void ThemeManager::removeSession(ThemeSession *session)
{
	Entry guard(m_sessGuard);
	Sessions_t::iterator i = std::find(m_sessions.begin(), m_sessions.end(), session);
	assert( i != m_sessions.end() );

	bool needNotify = (i == m_sessions.begin());
	m_sessions.erase(i);

	if(needNotify)
	{
		notifyThemeChange();
	}
}

//=====================================================================================//
//                       void ThemeManager::notifyThemeChange()                        //
//=====================================================================================//
void ThemeManager::notifyThemeChange()
{
	if(isThemeChanged())
	{
		bool wasPlaying = false;
		if(m_emitter != 0)
		{
			wasPlaying = true;
			Fade *fade = new Fade(m_module,m_emitter,-0.03f,&fadeCount);
			m_emitter = 0;
		}

		if(fadeCount > 0) wasPlaying = true;

		if(!m_sessions.empty())
		{
			m_script = m_sessions.front()->script();
			m_wave = m_sessions.front()->theme();

			if(m_sessions.front()->isPlaying())
			{
				ThemeSession *session = m_sessions.front();
				m_emitter = new Emitter(m_module,*session->script(),session->theme());
				m_emitter->play();
				if(wasPlaying)
				{
					m_emitter->setVolume(0.0f);
					Fade *fade = new Fade(m_module,m_emitter,0.03f);
				}
			}
		}
		else
		{
			m_script = 0;
			m_wave = "";
		}
	}
}

//=====================================================================================//
//                             void ThemeManager::notify()                             //
//=====================================================================================//
void ThemeManager::notify(ThemeSession *session)
{
	Entry guard(m_sessGuard);
	if(session == m_sessions.front())
	{
		notifyThemeChange();
	}
}