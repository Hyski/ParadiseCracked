#include "precomp.h"
#include "SoundBufferLock.h"
#include "SoundBuffer.h"

long SoundBufferLock::lockCount = 0;

//=====================================================================================//
//                         SoundBufferLock::SoundBufferLock()                          //
//=====================================================================================//
SoundBufferLock::SoundBufferLock(SoundBuffer *buffer, 
								 unsigned start, unsigned count)
:	m_sbuffer(buffer),
	m_buffer(buffer->getBuffer()),
	m_count(count*sizeof(short)*buffer->getFormat().channels)
{
	short *ptr;
	unsigned long size1, size2;

	SAFE_CALL(m_buffer->Lock(start*sizeof(short)*buffer->getFormat().channels,
		m_count,(void **)&m_ptr,&size1,(void **)&ptr,&size2,0));

	assert( ptr == 0 );
	assert( size1 == m_count );

	InterlockedIncrement(&lockCount);
}

//=====================================================================================//
//                         SoundBufferLock::~SoundBufferLock()                         //
//=====================================================================================//
SoundBufferLock::~SoundBufferLock()
{
	InterlockedDecrement(&lockCount);
	SAFE_CALL(m_buffer->Unlock(m_ptr,m_count,0, 0));
}