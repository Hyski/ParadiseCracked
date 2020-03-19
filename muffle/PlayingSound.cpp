#include "precomp.h"
#include "PlayingSound.h"
#include "Decoder.h"
#include "Script.h"
#include "StaticDecodeStream.h"
#include "DynamicDecodeStream.h"

//=====================================================================================//
//                            PlayingSound::PlayingSound()                             //
//=====================================================================================//
PlayingSound::PlayingSound(Module *module, const Script &script,
						   std::auto_ptr<Decoder> decoder, Emitter *emitter)
:	m_module(module),
	m_script(script),
	m_emitter(emitter),
	m_onStop(this,&PlayingSound::onStop)
{
	if(decoder->getLength() < max_buffer_size)
	{
		m_stream.reset(new StaticDecodeStream(m_module,m_script,decoder));
	}
	else
	{
		m_stream.reset(new DynamicDecodeStream(m_module,m_script,decoder));
	}

	m_stream->buffer()->getNotify()->addStopReceiver(&m_onStop);
	setPosition(m_emitter->position());
	setVelocity(m_emitter->velocity());
	setDistances(m_script.minDist(), m_script.maxDist());
	m_stream->play();
}

//=====================================================================================//
//                            PlayingSound::~PlayingSound()                            //
//=====================================================================================//
PlayingSound::~PlayingSound()
{
}

//=====================================================================================//
//                             void PlayingSound::onStop()                             //
//=====================================================================================//
void PlayingSound::onStop()
{
	m_stream->buffer()->getNotify()->removeReceiver(&m_onStop);
	m_emitter->onStop(this);
}

//=====================================================================================//
//                          void PlayingSound::setPosition()                           //
//=====================================================================================//
void PlayingSound::setPosition(const snd_vector &pos)
{
	if(!m_script.is3d()) return;
	IDirectSound3DBuffer8 *buf;
	SAFE_CALL(m_stream->buffer()->getBuffer()->QueryInterface(IID_IDirectSound3DBuffer8,(void**)&buf));
	buf->SetPosition(pos.x,pos.y,pos.z,DS3D_IMMEDIATE);
	buf->Release();
}

//=====================================================================================//
//                          void PlayingSound::setVelocity()                           //
//=====================================================================================//
void PlayingSound::setVelocity(const snd_vector &vel)
{
	if(!m_script.is3d()) return;
	IDirectSound3DBuffer8 *buf;
	SAFE_CALL(m_stream->buffer()->getBuffer()->QueryInterface(IID_IDirectSound3DBuffer8,(void**)&buf));
	buf->SetVelocity(vel.x,vel.y,vel.z,DS3D_IMMEDIATE);
	buf->Release();
}

//=====================================================================================//
//                          void PlayingSound::setDistances()                          //
//=====================================================================================//
void PlayingSound::setDistances(float min, float max)
{
	if(!m_script.is3d()) return;
	IDirectSound3DBuffer8 *buf;
	SAFE_CALL(m_stream->buffer()->getBuffer()->QueryInterface(IID_IDirectSound3DBuffer8,(void**)&buf));
	buf->SetMinDistance(min,DS3D_IMMEDIATE);
	buf->SetMaxDistance(max,DS3D_IMMEDIATE);
	buf->Release();
}

//=====================================================================================//
//                           void PlayingSound::setVolume()                            //
//=====================================================================================//
void PlayingSound::setVolume(float vol)
{
	m_stream->buffer()->getVolume()->change(vol);
}