#if !defined(__NOTIFY_THREAD_H_INCLUDED_6553975471027936__)
#define __NOTIFY_THREAD_H_INCLUDED_6553975471027936__

#include <list>
#include <map>

class NotifyReceiver;
class NotifyCluster;

//=====================================================================================//
//                                 class NotifyThread                                  //
//=====================================================================================//
class NotifyThread
{
	CriticalSection m_clustersGuard;

	typedef std::list<NotifyCluster *> Clusters_t;
	typedef std::map<HANDLE,Clusters_t::iterator> HandleMap_t;

	Clusters_t m_clusters;
	HandleMap_t m_handleMap;

public:
	NotifyThread();
	~NotifyThread();

	void addNotify(HANDLE,NotifyReceiver*);
	void removeNotify(HANDLE);

	unsigned getEventCount();
	unsigned getThreadCount();
};

#endif // !defined(__NOTIFY_THREAD_H_INCLUDED_6553975471027936__)