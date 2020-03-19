#include "precomp.h"
#include "DecodeThread.h"
#include "Decoder.h"
#include <process.h>

typedef void (*ThreadFunc_t)(void*);

//=====================================================================================//
//                            DecodeThread::DecodeThread()                             //
//=====================================================================================//
DecodeThread::DecodeThread()
:	m_thread(NULL)
{
//	m_thread = (HANDLE)_beginthread((ThreadFunc_t)threadStarter,0,this);
}

//=====================================================================================//
//                            DecodeThread::~DecodeThread()                            //
//=====================================================================================//
DecodeThread::~DecodeThread()
{
//	SetEvent(m_exitThread.handle());
//	WaitForSingleObject(m_thread,INFINITE);
//	m_thread = 0;
}

//=====================================================================================//
//                            void DecodeThread::addQuery()                            //
//=====================================================================================//
void DecodeThread::addQuery(DecodeThreadQuery *query)
{
	query->start();
	query->decoder()->decode(*query->buffer());
	query->finish();
	delete query;
//	{
//		Entry guard(m_queueGuard);
//		m_queue.push_front(query);
//	}
//	SetEvent(m_queueUpdated.handle());
}

//=====================================================================================//
//                     void __cdecl DecodeThread::threadStarter()                      //
//=====================================================================================//
void __cdecl DecodeThread::threadStarter(DecodeThread *This)
{
	This->threadFunc();
}

//=====================================================================================//
//                          void DecodeThread::removeQuery()                           //
//=====================================================================================//
void DecodeThread::removeQuery(Decoder *decoder)
{
	Entry guard(m_queueGuard);
	Queries_t::iterator i = m_queue.begin();
	for(; i != m_queue.end();)
	{
		if((*i)->decoder() == decoder)
		{
			delete *i;
			m_queue.erase(i++);
		}
		else
		{
			++i;
		}
	}
}

//=====================================================================================//
//                           void DecodeThread::threadFunc()                           //
//=====================================================================================//
void DecodeThread::threadFunc()
{
	HANDLE handles[] = { m_exitThread.handle(), m_queueUpdated.handle() };
	unsigned size = sizeof(handles)/sizeof(HANDLE);

	while(true)
	{
		assert( size <= MAXIMUM_WAIT_OBJECTS );

		unsigned result = WaitForMultipleObjects(size,handles,FALSE,INFINITE);

		if(result < WAIT_OBJECT_0 || result >= WAIT_OBJECT_0 + size)
		{
			throw sound_error("Error in WaitForMultipleObject call");
		}

		result -= WAIT_OBJECT_0;

		if(handles[result] == m_exitThread.handle())
		{
			break;
		}
		else if(handles[result] == m_queueUpdated.handle())
		{
			for(;;)
			{
				std::auto_ptr<DecodeThreadQuery> query;

				{
					Entry guard(m_queueGuard);
					if(m_queue.empty()) break;
					query.reset(m_queue.back());
					m_queue.pop_back();
				}

				query->start();
				query->decoder()->decode(*query->buffer());
				query->finish();
			}
		}
	}
}