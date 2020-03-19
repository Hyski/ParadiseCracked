#include "precomp.h"
#include "NotifyThread.h"
#include "NotifyCluster.h"
#include <process.h>
#include <functional>

//=====================================================================================//
//                            NotifyThread::NotifyThread()                             //
//=====================================================================================//
NotifyThread::NotifyThread()
{
}

//=====================================================================================//
//                            NotifyThread::~NotifyThread()                            //
//=====================================================================================//
NotifyThread::~NotifyThread()
{
	while( !m_handleMap.empty() ) Sleep(0);
}

//=====================================================================================//
//                           void NotifyThread::addNotify()                            //
//=====================================================================================//
void NotifyThread::addNotify(HANDLE handle, NotifyReceiver *recv)
{
	Entry guard(m_clustersGuard);

	// Ќайдем первый кластер, в котором есть вакантные места
	Clusters_t::iterator i = 
		std::find_if(m_clusters.begin(),m_clusters.end(),
		std::mem_fun(&NotifyCluster::canAddMore));

	NotifyCluster *cluster;

	if(i == m_clusters.end())
	{
		// “акого кластера не существует -- создадим его
		cluster = new NotifyCluster;
		m_clusters.push_front(cluster);
		i = m_clusters.begin();
	}
	else
	{
		cluster = *i;
	}

	cluster->addNotify(handle,recv);
	m_handleMap[handle] = i;
}

//=====================================================================================//
//                          void NotifyThread::removeNotify()                          //
//=====================================================================================//
void NotifyThread::removeNotify(HANDLE handle)
{
	Entry guard(m_clustersGuard);

	HandleMap_t::iterator i = m_handleMap.find(handle);
	assert( i != m_handleMap.end() );

	Clusters_t::iterator clit = i->second;
	NotifyCluster *cluster = *clit;
	cluster->removeNotify(handle);
	m_handleMap.erase(i);

	if(!cluster->hasEvents())
	{
		cluster->release();
		m_clusters.erase(clit);
	}
}

//=====================================================================================//
//                       unsigned NotifyThread::getEventCount()                        //
//=====================================================================================//
unsigned NotifyThread::getEventCount()
{
	return m_handleMap.size();
}

//=====================================================================================//
//                       unsigned NotifyThread::getThreadCount()                       //
//=====================================================================================//
unsigned NotifyThread::getThreadCount()
{
	return m_clusters.size();
}