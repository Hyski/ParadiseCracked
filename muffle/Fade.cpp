#include "precomp.h"
#include "Fade.h"
#include "Emitter.h"
#include "NotifyThread.h"

#if !defined(MUFFLE_WAIT_FOR_MUSIC_FADE_OUT)
CriticalSection Fade::m_listLock;
CriticalSection Fade::m_destructLock;
Fade::FadeList_t Fade::m_allFades;

//=====================================================================================//
//                                void Fade::destruct()                                //
//=====================================================================================//
void Fade::destruct()
{
	Entry guard1(m_destructLock);
	Entry guard2(m_listLock);
	
	for(FadeList_t::iterator i = m_allFades.begin(); i != m_allFades.end(); ++i)
	{
		(*i)->m_selfDestruct = true;
	}
}

#endif

//=====================================================================================//
//                            inline void Fade::addToList()                            //
//=====================================================================================//
inline void Fade::addToList()
{
#if !defined(MUFFLE_WAIT_FOR_MUSIC_FADE_OUT)
	Entry guard(m_listLock);
	m_allFades.insert(this);
#endif
}

//=====================================================================================//
//                         inline void Fade::removeFromList()                          //
//=====================================================================================//
inline void Fade::removeFromList()
{
#if !defined(MUFFLE_WAIT_FOR_MUSIC_FADE_OUT)
	Entry guard(m_listLock);
	m_allFades.erase(this);
#endif
}

//=====================================================================================//
//                                    Fade::Fade()                                     //
//=====================================================================================//
Fade::Fade(Module *module, Emitter *emitter, float inc, long *fadeCount)
:	m_module(module),
	m_volume(inc<0?1.0f:0.0f),
	m_increment(inc),
	m_emitter(emitter),
	m_fadeCount(fadeCount),
	m_decVolAdaptor(this,&Fade::onDecVol),
	m_selfDestruct(false)
{
	if(m_fadeCount) InterlockedIncrement(m_fadeCount);
	m_htimer = CreateWaitableTimer(NULL,FALSE,NULL);
	setTimer();
	m_module->getNotifyThread()->addNotify(m_htimer,&m_decVolAdaptor);
	addToList();
}

//=====================================================================================//
//                                    Fade::~Fade()                                    //
//=====================================================================================//
Fade::~Fade()
{
#if !defined(MUFFLE_WAIT_FOR_MUSIC_FADE_OUT)
	Entry guard(m_destructLock);
#endif

	if(m_fadeCount) InterlockedDecrement(m_fadeCount);
	if(m_increment < 0.0f)
	{
		m_emitter->stop();
		m_emitter->Release();
	}
	m_module->getNotifyThread()->removeNotify(m_htimer);
	CloseHandle(m_htimer);
	removeFromList();
}

//=====================================================================================//
//                                void Fade::setTimer()                                //
//=====================================================================================//
void Fade::setTimer()
{
	LARGE_INTEGER period;
	period.QuadPart = -1000000;		// Каждую десятую секунды
	if(!SetWaitableTimer(m_htimer,&period,0,NULL,NULL,FALSE))
	{
		throw sound_error("Cannot set waitable timer");
	}
}

//=====================================================================================//
//                                void Fade::onDecVol()                                //
//=====================================================================================//
void Fade::onDecVol()
{
#if !defined(MUFFLE_WAIT_FOR_MUSIC_FADE_OUT)
	Entry guard(m_destructLock);

	if(m_selfDestruct)
	{
		delete this;
		return;
	}
#endif

	m_emitter->setVolume(m_volume += m_increment);

	if((m_volume <= 0.0f && m_increment < 0.0f) 
		|| (m_volume >= 1.0f && m_increment > 0.0f))
	{
		delete this;
	}
	else
	{
		setTimer();
	}
}