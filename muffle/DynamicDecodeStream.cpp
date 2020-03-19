#include "precomp.h"
#include "DynamicDecodeStream.h"
#include "filemgmt.h"
#include "DecodeMgr.h"
#include "DynamicDecodeStreamQuery.h"
#include "Decoder.h"
#include "Script.h"
#include "SoundBufferNotify.h"

//=====================================================================================//
//                     DynamicDecodeStream::DynamicDecodeStream()                      //
//=====================================================================================//
DynamicDecodeStream::DynamicDecodeStream(Module *module, const Script &script,
										 std::auto_ptr<Decoder> decoder)
:	m_module(module),
	m_script(script),
	m_decoder(decoder),
	m_decodeAdaptor(this,&DynamicDecodeStream::onDecode),
	m_stopAdaptor(this,&DynamicDecodeStream::onStop),
	m_stopActivated(false),
	m_currentFrame(0)
{
	m_buffer.reset	(
						new DynamicSoundBuffer
						(
							m_module,
							m_decoder->getFormat(),
							piece_size,
							m_script.is3d(),
							m_script.channel(),
							piece_count
						)
					);

	for(unsigned i = 0; i < piece_count; ++i)
	{
		DynamicDecodeStreamQuery query(this,m_currentFrame);
		query.start();
		query.decoder()->decode(*query.buffer());
		query.finish();
		m_currentFrame = (m_currentFrame+1)%piece_count;
	}

	m_buffer->getNotify()->addDecodeReceiver(piece_size,piece_count,&m_decodeAdaptor);
	m_buffer->getNotify()->addPosReceiver(m_decoder->getLength()%max_buffer_size,&m_stopAdaptor);
}

//=====================================================================================//
//                     DynamicDecodeStream::~DynamicDecodeStream()                     //
//=====================================================================================//
DynamicDecodeStream::~DynamicDecodeStream()
{
	m_module->getDecodeThread()->removeQuery(m_decoder.get());
	m_buffer->getNotify()->removeReceiver(&m_decodeAdaptor);
	m_buffer->getNotify()->removeReceiver(&m_stopAdaptor);
}

//=====================================================================================//
//                    Format DynamicDecodeStream::getFormat() const                    //
//=====================================================================================//
Format DynamicDecodeStream::getFormat() const
{
	return m_buffer->getFormat();
}

//=====================================================================================//
//                 unsigned DynamicDecodeStream::getAvailSize() const                  //
//=====================================================================================//
unsigned DynamicDecodeStream::getAvailSize() const
{
	return m_samplesLeft;
}

//=====================================================================================//
//                          void DynamicDecodeStream::feed()                           //
//=====================================================================================//
void DynamicDecodeStream::feed(const short *data, unsigned count)
{
	m_buffer->feed(data,count);
}

//=====================================================================================//
//                          void DynamicDecodeStream::play()                           //
//=====================================================================================//
void DynamicDecodeStream::play()
{
	m_buffer->getNotify()->prepareForPlaying();
	SAFE_CALL(m_buffer->getBuffer()->Play(0,0,DSBPLAY_LOOPING));
}

//=====================================================================================//
//                          void DynamicDecodeStream::stop()                           //
//=====================================================================================//
void DynamicDecodeStream::stop()
{
	SAFE_CALL(m_buffer->getBuffer()->Stop());
}

//=====================================================================================//
//                        void DynamicDecodeStream::onDecode()                         //
//=====================================================================================//
void DynamicDecodeStream::onDecode()
{
	DWORD playCurs;
	m_buffer->getBuffer()->GetCurrentPosition(&playCurs,0);
	playCurs /= piece_size*sizeof(short)*m_decoder->getFormat().channels;
	while(playCurs != m_currentFrame)
	{
		m_module->getDecodeThread()->addQuery(
				new DynamicDecodeStreamQuery(this,m_currentFrame)
			);
		m_currentFrame = (m_currentFrame+1)%piece_count;
	}
}

//=====================================================================================//
//                         void DynamicDecodeStream::onStop()                          //
//=====================================================================================//
void DynamicDecodeStream::onStop()
{
	DWORD playCurs;
	buffer()->getBuffer()->GetCurrentPosition(&playCurs,0);
	unsigned block = sizeof(short) * buffer()->getFormat().channels;

	if(m_stopActivated)
	{
		m_buffer->getBuffer()->Stop();
	}
}

//=====================================================================================//
//                      void DynamicDecodeStream::activateStop()                       //
//=====================================================================================//
void DynamicDecodeStream::activateStop()
{
	if(!m_script.repeat())
	{
		m_stopActivated = true;
	}
}