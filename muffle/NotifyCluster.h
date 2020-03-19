#if !defined(__NOTIFY_CLUSTER_H_INCLUDED_7552370787048739__)
#define __NOTIFY_CLUSTER_H_INCLUDED_7552370787048739__

class NotifyReceiver;

//=====================================================================================//
//                                 class NotifyCluster                                 //
//=====================================================================================//
class NotifyCluster
{
	CriticalSection m_vectorsGuard;
	HANDLE m_thread;

	WndHandle m_exitThread;
	WndHandle m_updateContent;
	WndHandle m_doneUpdate;

	typedef std::vector<HANDLE> Handles_t;
	typedef std::vector<NotifyReceiver*> Receivers_t;

	Handles_t m_handles;
	Receivers_t m_receivers;

	void doInsertReceiver(HANDLE,NotifyReceiver *);

	static void __cdecl threadStarter(NotifyCluster *);
	void threadFunc();

	~NotifyCluster();

public:
	NotifyCluster();
	void release();

	void addNotify(HANDLE, NotifyReceiver *);
	void removeNotify(HANDLE);

	bool hasEvents();
	bool canAddMore();
};

#endif // !defined(__NOTIFY_CLUSTER_H_INCLUDED_7552370787048739__)