#include "precomp.h"
#include "DynamicSoundBuffer.h"
#include "dsound.h"
#include "SoundBufferNotify.h"
#include "SoundBufferLock.h"
#include "VolumeMgr.h"
#include "Muffle.h"

//=====================================================================================//
//                      DynamicSoundBuffer::DynamicSoundBuffer()                       //
//=====================================================================================//
DynamicSoundBuffer::DynamicSoundBuffer(Module *module, Format fmt,
									   unsigned size, bool b3d, ISound::Channel channel,
									   unsigned pieceCount)
:	m_format(fmt),
	m_size(size),
	m_pieceCount(pieceCount)
{
	IDirectSoundBuffer *buf;
	WAVEFORMATEX wfmt;
	DSBUFFERDESC desc;

	DWORD initialFlags = (b3d?DSBCAPS_CTRL3D:0)|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLPOSITIONNOTIFY|
								DSBCAPS_GETCURRENTPOSITION2;

	desc.dwSize = sizeof(desc);
	desc.dwFlags = initialFlags|DSBCAPS_LOCHARDWARE;
	desc.dwBufferBytes = m_size * m_pieceCount * sizeof(short) * m_format.channels;
	desc.dwReserved = 0;
	desc.lpwfxFormat = &wfmt;
	desc.guid3DAlgorithm = (b3d?DS3DALG_HRTF_FULL:GUID_NULL);

	wfmt.cbSize = sizeof(wfmt);
	wfmt.nChannels = m_format.channels;
	wfmt.nSamplesPerSec = m_format.rate;
	wfmt.wBitsPerSample = sizeof(short)*8;
	wfmt.nBlockAlign = sizeof(short)*m_format.channels;
	wfmt.wFormatTag = WAVE_FORMAT_PCM;
	wfmt.nAvgBytesPerSec = m_format.rate * wfmt.nBlockAlign;

	IDirectSoundBuffer8 *buf8;
#if !defined(__MUFFLE_NO_HARDWARE_BUFFERS__)
	if(FAILED(module->getDirectSound()->dsound()->CreateSoundBuffer(&desc,&buf,NULL)))
	{
#endif
		desc.dwFlags = initialFlags|DSBCAPS_LOCSOFTWARE;
		if(FAILED(module->getDirectSound()->dsound()->CreateSoundBuffer(&desc,&buf,NULL)))
		{
			throw sound_error("Failed to create sound buffer");
		}
		else
		{
			m_isHard = false;
			addSoft();
		}
#if !defined(__MUFFLE_NO_HARDWARE_BUFFERS__)
	}
	else
	{
		m_isHard = true;
		addHard();
	}
#endif

	SAFE_CALL(buf->QueryInterface(IID_IDirectSoundBuffer8,(void**)(&buf8)));
	buf->Release();

	m_buffer.attach(buf8);
	m_notify.reset(new SoundBufferNotify(module,this));

	m_volume = module->getVolumeMgr()->addVolume(module->getMuffle()->getVolumeObject(channel));
	m_volume->attach(this);
}

//=====================================================================================//
//                      DynamicSoundBuffer::~DynamicSoundBuffer()                      //
//=====================================================================================//
DynamicSoundBuffer::~DynamicSoundBuffer()
{
	if(m_isHard)
	{
		subHard();
	}
	else
	{
		subSoft();
	}
	m_volume->detach();
}

//=====================================================================================//
//                  unsigned DynamicSoundBuffer::getAvailSize() const                  //
//=====================================================================================//
unsigned DynamicSoundBuffer::getAvailSize() const
{
	return m_size - m_writePos;
}

//=====================================================================================//
//                           void DynamicSoundBuffer::feed()                           //
//=====================================================================================//
void DynamicSoundBuffer::feed(const short *data, unsigned count)
{
	assert( count <= getAvailSize() );
	unsigned block = sizeof(short) * m_format.channels;
	memcpy(m_lock->lockArea() + m_writePos*m_format.channels, data, count * block);
	m_writePos += count;
}

//=====================================================================================//
//                           void DynamicSoundBuffer::lock()                           //
//=====================================================================================//
void DynamicSoundBuffer::lock(unsigned piece)
{
	unsigned start = m_size * piece;
	assert( m_lock.get() == 0 );
	m_lock.reset(new SoundBufferLock(this,start,m_size));
	m_writePos = 0;
}

//=====================================================================================//
//                          void DynamicSoundBuffer::unlock()                          //
//=====================================================================================//
void DynamicSoundBuffer::unlock()
{
	m_lock.reset();
}