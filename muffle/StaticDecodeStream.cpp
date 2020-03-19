#include "precomp.h"
#include "StaticDecodeStream.h"
#include "StaticSoundBuffer.h"
#include "DecodeMgr.h"
#include "filemgmt.h"
#include "Decoder.h"
#include "Script.h"
#include "SoundBufferNotify.h"

//=====================================================================================//
//                      StaticDecodeStream::StaticDecodeStream()                       //
//=====================================================================================//
StaticDecodeStream::StaticDecodeStream(Module *module, const Script &script,
									   std::auto_ptr<Decoder> decoder)
:	m_module(module),
	m_script(script)
{
	m_buffer.reset	(
						new StaticSoundBuffer
						(
							m_module,
							decoder->getFormat(),
							decoder->getLength(),
							m_script.is3d(),
							m_script.channel()
						)
					);

	m_buffer->lock();
	decoder->decode(*m_buffer);
	m_buffer->unlock();
}

//=====================================================================================//
//                      StaticDecodeStream::~StaticDecodeStream()                      //
//=====================================================================================//
StaticDecodeStream::~StaticDecodeStream()
{
}

//=====================================================================================//
//                           void StaticDecodeStream::play()                           //
//=====================================================================================//
void StaticDecodeStream::play()
{
	m_buffer->getNotify()->prepareForPlaying();
	SAFE_CALL(m_buffer->getBuffer()->Play(0,0,m_script.repeat()?DSBPLAY_LOOPING:0));
}

//=====================================================================================//
//                           void StaticDecodeStream::stop()                           //
//=====================================================================================//
void StaticDecodeStream::stop()
{
	SAFE_CALL(m_buffer->getBuffer()->Stop());
}