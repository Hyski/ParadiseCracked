#if !defined(__DYNAMIC_SOUND_BUFFER_H_INCLUDED_0683147347475120__)
#define __DYNAMIC_SOUND_BUFFER_H_INCLUDED_0683147347475120__

#include "SoundBuffer.h"

//=====================================================================================//
//                              class DynamicSoundBuffer                               //
//=====================================================================================//
class DynamicSoundBuffer : public SoundBuffer
{
	Format m_format;
	ComPtr<IDirectSoundBuffer8> m_buffer;
	unsigned m_pieceCount;
	unsigned m_size;
	unsigned m_writePos;
	std::auto_ptr<SoundBufferNotify> m_notify;
	std::auto_ptr<class SoundBufferLock> m_lock;
	std::auto_ptr<Volume> m_volume;
	bool m_isHard;

public:
	DynamicSoundBuffer(Module *, Format, unsigned size, bool b3d,
					   ISound::Channel, unsigned partCount);
	virtual ~DynamicSoundBuffer();

	virtual IDirectSoundBuffer8 *getBuffer() const { return m_buffer; }
	virtual Format getFormat() const { return m_format; }
	virtual unsigned getAvailSize() const;
	virtual void feed(const short *data, unsigned count);
	virtual SoundBufferNotify *getNotify() { return m_notify.get(); }
	virtual Volume *getVolume() { return m_volume.get(); }

	void lock(unsigned part);
	void unlock();
};

#endif // !defined(__DYNAMIC_SOUND_BUFFER_H_INCLUDED_0683147347475120__)