#if !defined(__SOUND_BUFFER_LOCK_H_INCLUDED_8811013198571208__)
#define __SOUND_BUFFER_LOCK_H_INCLUDED_8811013198571208__

class SoundBuffer;

//=====================================================================================//
//                                class SoundBufferLock                                //
//=====================================================================================//
class SoundBufferLock : private noncopyable
{
	static long lockCount;

	SoundBuffer *m_sbuffer;
	ComPtr<IDirectSoundBuffer8> m_buffer;
	short *m_ptr;
	unsigned m_count;

public:
	SoundBufferLock(SoundBuffer *, unsigned start, unsigned count);
	~SoundBufferLock();
	short *lockArea() const { return m_ptr; }
};

#endif // !defined(__SOUND_BUFFER_LOCK_H_INCLUDED_8811013198571208__)