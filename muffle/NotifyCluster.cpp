#include "precomp.h"
#include "NotifyCluster.h"
#include "NotifyReceiver.h"
#include <process.h>

typedef void (*ThreadFunc_t)(void*);

//=====================================================================================//
//                           NotifyCluster::NotifyCluster()                            //
//=====================================================================================//
NotifyCluster::NotifyCluster()
:	m_thread(NULL)
{
	m_handles.reserve(MAXIMUM_WAIT_OBJECTS);
	m_receivers.reserve(MAXIMUM_WAIT_OBJECTS);

	doInsertReceiver(m_updateContent.handle(),0);
	doInsertReceiver(m_exitThread.handle(),0);

	m_thread = (HANDLE)_beginthread((ThreadFunc_t)threadStarter,0,this);
	SetThreadPriority(m_thread,THREAD_PRIORITY_TIME_CRITICAL);
}

//=====================================================================================//
//                           NotifyCluster::~NotifyCluster()                           //
//=====================================================================================//
NotifyCluster::~NotifyCluster()
{
}

//=====================================================================================//
//                            void NotifyCluster::release()                            //
//=====================================================================================//
void NotifyCluster::release()
{
	SetEvent(m_exitThread.handle());
}

//=====================================================================================//
//                       void NotifyCluster::doInsertReceiver()                        //
//=====================================================================================//
void NotifyCluster::doInsertReceiver(HANDLE handle, NotifyReceiver *recv)
{
	Handles_t::iterator it = std::lower_bound(m_handles.begin(),m_handles.end(),handle);
	const int index = it-m_handles.begin();
	m_handles.insert(it,handle);
	m_receivers.insert(m_receivers.begin()+index,recv);
}

//=====================================================================================//
//                           bool NotifyCluster::hasEvents()                           //
//=====================================================================================//
bool NotifyCluster::hasEvents()
{
	return m_handles.size() > 2;
}

//=====================================================================================//
//                          bool NotifyCluster::canAddMore()                           //
//=====================================================================================//
bool NotifyCluster::canAddMore()
{
	return m_handles.size() < MAXIMUM_WAIT_OBJECTS;
}

//=====================================================================================//
//                           void NotifyCluster::addNotify()                           //
//=====================================================================================//
void NotifyCluster::addNotify(HANDLE handle, NotifyReceiver *recv)
{
	assert( m_thread != NULL );
	SetEvent(m_updateContent.handle());

	{
		Entry guard(m_vectorsGuard);
		doInsertReceiver(handle,recv);
	}

	SetEvent(m_doneUpdate.handle());
}

//=====================================================================================//
//                         void NotifyCluster::removeNotify()                          //
//=====================================================================================//
void NotifyCluster::removeNotify(HANDLE handle)
{
	assert( m_thread != NULL );
	SetEvent(m_updateContent.handle());

	{
		Entry guard(m_vectorsGuard);
		Handles_t::iterator it = std::lower_bound(m_handles.begin(),m_handles.end(),handle);
		assert( it != m_handles.end() );
		assert( *it == handle );
		int index = it-m_handles.begin();
		m_handles.erase(it);
		m_receivers.erase(m_receivers.begin()+index);
	}

	SetEvent(m_doneUpdate.handle());
}

//=====================================================================================//
//                         void NotifyCluster::threadStarter()                         //
//=====================================================================================//
void NotifyCluster::threadStarter(NotifyCluster *This)
{
	try
	{
		This->threadFunc();
	}
	catch(const std::exception &e)
	{
		MessageBox(NULL,e.what(),"Paradise Cracked", MB_OK);
	}
}

//=====================================================================================//
//                          void NotifyCluster::threadFunc()                           //
//=====================================================================================//
void NotifyCluster::threadFunc()
{
	std::auto_ptr<Entry> guard(new Entry(m_vectorsGuard));

	while(true)
	{
		assert( m_handles.size() <= MAXIMUM_WAIT_OBJECTS );

		unsigned result = WaitForMultipleObjects(
			m_handles.size(),
			&m_handles[0],
			FALSE,INFINITE);

		if(result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + m_handles.size())
		{
			result -= WAIT_OBJECT_0;
		}
		else
		{
			throw sound_error("Error in WaitForMultipleObject call");
		}

		if(m_handles[result] == m_updateContent.handle())
		{
			guard.reset();
			WaitForSingleObject(m_doneUpdate.handle(),INFINITE);
			guard.reset(new Entry(m_vectorsGuard));
		}
		else if(m_handles[result] == m_exitThread.handle())
		{
			guard.reset();
			delete this;
			break;
		}
		else
		{
			NotifyReceiver *recv = m_receivers[result];
			guard.reset();
			recv->notify();
			guard.reset(new Entry(m_vectorsGuard));
		}
	}
}