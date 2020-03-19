#if !defined(__SOUND_BUFFER_H_INCLUDED_1341602651940427__)
#define __SOUND_BUFFER_H_INCLUDED_1341602651940427__

#include "buffer.h"

class SoundBufferNotify;
class Volume;

//=====================================================================================//
//                          class SoundBuffer : public Buffer                          //
//=====================================================================================//
class SoundBuffer : public Buffer
{
	static long m_hardwareBuffers;
	static long m_softwareBuffers;

public:
	virtual IDirectSoundBuffer8 *getBuffer() const = 0;
	virtual SoundBufferNotify *getNotify() = 0;
	virtual Volume *getVolume() = 0;

	static void addSoft();
	static void subSoft();
	static void addHard();
	static void subHard();

	static long softCount() { return m_softwareBuffers; }
	static long hardCount() { return m_hardwareBuffers; }
};

#endif // !defined(__SOUND_BUFFER_H_INCLUDED_1341602651940427__)