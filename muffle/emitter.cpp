#include "precomp.h"
#include "emitter.h"
#include "SoundBuffer.h"
#include "Decoder.h"
#include "DecodeMgr.h"
#include "Script.h"
#include "FileMgmt.h"
#include "SoundBufferNotify.h"
#include "StaticSoundBuffer.h"
#include "DecodeStream.h"
#include "PlayingSound.h"

long Emitter::m_emittersCount = 0;

CriticalSection Emitter::m_staticListGuard;
CriticalSection Emitter::m_onStopGuard;
Emitter::EmitterList_t Emitter::m_allEmitters;

//=====================================================================================//
//                              void Emitter::ClearAll()                               //
//=====================================================================================//
void Emitter::ClearAll()
{
	Entry guard1(m_onStopGuard);
	Entry guard2(m_staticListGuard);

	EmitterList_t emitterList(m_allEmitters);

	for(EmitterList_t::iterator i = emitterList.begin(); i != emitterList.end(); ++i)
	{
		if((*i)->m_script->channel() != ISound::cThemes)
		{
			(*i)->stop();
			(*i)->Release();
		}
	}
}

//=====================================================================================//
//                            void Emitter::insertToList()                             //
//=====================================================================================//
void Emitter::insertToList()
{
#if MUFFLE_DESTRUCT_STRATEGY==1
	{
		Entry guard(m_staticListGuard);
		m_allEmitters.insert(this);
	}
#elif MUFFLE_DESTRUCT_STRATEGY==2
	if(script.channel() == ISound::cThemes)
	{
		Entry guard(m_staticListGuard);
		m_allEmitters.insert(this);
	}
#else
#error Unknown MUFFLE_DESTRUCT_STRATEGY
#endif
}

//=====================================================================================//
//                           void Emitter::removeFromList()                            //
//=====================================================================================//
void Emitter::removeFromList()
{
#if MUFFLE_DESTRUCT_STRATEGY==1
	{
		Entry guard(m_staticListGuard);
		m_allEmitters.erase(this);
	}
#elif MUFFLE_DESTRUCT_STRATEGY==2
	if(script.channel() == ISound::cThemes)
	{
		Entry guard(m_staticListGuard);
		m_allEmitters.erase(this);
	}
#else
#error Unknown MUFFLE_DESTRUCT_STRATEGY
#endif
}

//=====================================================================================//
//                                 Emitter::Emitter()                                  //
//=====================================================================================//
Emitter::Emitter(Module *module, const Script &script)
:	m_module(module),
	m_script(&script),
	m_wave(m_script->fileName()),
	m_selfDestruct(false),
	m_pos(0.0f,0.0f,0.0f),
	m_vel(0.0f,0.0f,0.0f),
	m_stopping(false)
{
	if( m_wave.length() > 0 )
	{
		NOFILE_LOG("Передано пустое имя файла\n");
	}
	InterlockedIncrement(&m_emittersCount);

	insertToList();
}

//=====================================================================================//
//                                 Emitter::Emitter()                                  //
//=====================================================================================//
Emitter::Emitter(Module *module, const Script &script, const std::string &wave)
:	m_module(module),
	m_script(&script),
	m_wave(wave),
	m_selfDestruct(false),
	m_pos(0.0f,0.0f,0.0f),
	m_vel(0.0f,0.0f,0.0f),
	m_stopping(false)
{
	if( m_wave.length() > 0 )
	{
		NOFILE_LOG("Передано пустое имя файла\n");
	}
	InterlockedIncrement(&m_emittersCount);

	insertToList();
}

//=====================================================================================//
//                                 Emitter::~Emitter()                                 //
//=====================================================================================//
Emitter::~Emitter()
{
	InterlockedDecrement(&m_emittersCount);
	removeFromList();
}

//=====================================================================================//
//                                void Emitter::play()                                 //
//=====================================================================================//
void Emitter::play()
{
	try
	{
		std::auto_ptr<Decoder> decoder = m_module->getDecodeMgr()
						->createDecoder(m_module->getFileMgmt()->createFile(m_wave.c_str()));

		PlayingSound *snd = new PlayingSound(m_module,*m_script,decoder,this);

		{
			Entry guard(m_soundsGuard);
			iterator i = std::lower_bound(m_sounds.begin(),m_sounds.end(),snd);
			m_sounds.insert(i,snd);
		}

		m_stopping = false;
	}
	catch(const cannot_create_decoder &)
	{
		NOFILE_LOG("Невозможно открыть файл " << m_wave.c_str() << "\n");
	}
	catch(const file_not_found &)
	{
		NOFILE_LOG("Не найден файл " << m_wave.c_str() << "\n");
	}
}

//=====================================================================================//
//                                void Emitter::stop()                                 //
//=====================================================================================//
void Emitter::stop()
{
	Entry guard(m_soundsGuard);
	for(iterator i = m_sounds.begin(); i != m_sounds.end(); ++i)
	{
		PlayingSound *pls = *i;
		if(pls->m_stream.get())
		{
			pls->m_stream->buffer()->getBuffer()->Stop();
		}
	}
	m_stopping = true;
}

//=====================================================================================//
//                           void Emitter::setSelfDestruct()                           //
//=====================================================================================//
void Emitter::setSelfDestruct()
{
	if(m_script->repeat() && !m_stopping)
	{
		std::ostringstream sstr;
		sstr << "Cannot self destruct repeating emitter " << m_wave;
		throw sound_error(sstr.str());
	}
	m_selfDestruct = true;
}

//=====================================================================================//
//                             void Emitter::setPosition()                             //
//=====================================================================================//
void Emitter::setPosition(const snd_vector &pos)
{
	if( !m_script->is3d() ) return;
	m_pos = pos;
	Entry guard(m_soundsGuard);
	for(iterator i = m_sounds.begin(); i != m_sounds.end(); ++i)
	{
		(*i)->setPosition(m_pos);
	}
}

//=====================================================================================//
//                             void Emitter::setVelocity()                             //
//=====================================================================================//
void Emitter::setVelocity(const snd_vector &vel)
{
	if( !m_script->is3d() ) return;
	m_vel = vel;
	Entry guard(m_soundsGuard);
	for(iterator i = m_sounds.begin(); i != m_sounds.end(); ++i)
	{
		(*i)->setVelocity(m_vel);
	}
}

//=====================================================================================//
//                              void Emitter::setVolume()                              //
//=====================================================================================//
void Emitter::setVolume(float vol)
{
	Entry guard(m_soundsGuard);
	for(iterator i = m_sounds.begin(); i != m_sounds.end(); ++i)
	{
		(*i)->setVolume(vol);
	}
}

//=====================================================================================//
//                              bool Emitter::isPlaying()                              //
//=====================================================================================//
bool Emitter::isPlaying()
{
//	return m_stream.get() != 0;
	Entry guard(m_soundsGuard);
	return !m_sounds.empty();
}

//=====================================================================================//
//                               void Emitter::onStop()                                //
//=====================================================================================//
void Emitter::onStop(PlayingSound *snd)
{
	Entry guard(m_onStopGuard);

	bool test;

	{
		Entry guard(m_soundsGuard);
		delete snd;
		m_sounds.erase(std::lower_bound(m_sounds.begin(),m_sounds.end(),snd));
		test = m_sounds.empty();
	}

	if(test && m_selfDestruct) delete this;
}

//=====================================================================================//
//                               void Emitter::Release()                               //
//=====================================================================================//
void Emitter::Release()
{
	if(isPlaying())
	{
		setSelfDestruct();
	}
	else
	{
		delete this;
	}
}

//=====================================================================================//
//                        unsigned Emitter::getEmittersCount()                         //
//=====================================================================================//
long Emitter::getEmittersCount()
{
	return m_emittersCount;
}