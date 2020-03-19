#include "precomp.h"
#include "SoundBuffer.h"

long SoundBuffer::m_softwareBuffers = 0;
long SoundBuffer::m_hardwareBuffers = 0;

//=====================================================================================//
//                             void SoundBuffer::addSoft()                             //
//=====================================================================================//
void SoundBuffer::addSoft()
{
	InterlockedIncrement(&m_softwareBuffers);
}

//=====================================================================================//
//                             void SoundBuffer::subSoft()                             //
//=====================================================================================//
void SoundBuffer::subSoft()
{
	InterlockedDecrement(&m_softwareBuffers);
}

//=====================================================================================//
//                             void SoundBuffer::addHard()                             //
//=====================================================================================//
void SoundBuffer::addHard()
{
	InterlockedIncrement(&m_hardwareBuffers);
}

//=====================================================================================//
//                             void SoundBuffer::subHard()                             //
//=====================================================================================//
void SoundBuffer::subHard()
{
	InterlockedDecrement(&m_hardwareBuffers);
}
