#if !defined(__STATIC_SOUND_BUFFER_H_INCLUDED_2520816929844783__)
#define __STATIC_SOUND_BUFFER_H_INCLUDED_2520816929844783__

#include "SoundBuffer.h"

class DirectSound;
class SoundBufferNotify;

//=====================================================================================//
//                       class StaticSoundBuffer : public Buffer                       //
//=====================================================================================//
class StaticSoundBuffer : public SoundBuffer
{
	Format m_format;
	ComPtr<IDirectSoundBuffer8> m_buffer;
	unsigned m_size;
	unsigned m_writePos;
	std::auto_ptr<class SoundBufferLock> m_lock;
	std::auto_ptr<SoundBufferNotify> m_notify;
	std::auto_ptr<Volume> m_volume;
	bool m_isHard;

public:
	StaticSoundBuffer(Module *, Format, unsigned size, bool b3d, ISound::Channel);
	virtual ~StaticSoundBuffer();

	void lock();
	void unlock();

	virtual IDirectSoundBuffer8 *getBuffer() const { return m_buffer; }
	virtual Format getFormat() const { return m_format; }
	virtual unsigned getAvailSize() const;
	virtual void feed(const short *data, unsigned count);
	virtual SoundBufferNotify *getNotify() { return m_notify.get(); }
	virtual Volume *getVolume() { return m_volume.get(); }
};

#endif // !defined(__STATIC_SOUND_BUFFER_H_INCLUDED_2520816929844783__)