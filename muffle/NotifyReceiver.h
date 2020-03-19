#if !defined(__NOTIFY_RECEIVER_H_INCLUDED_7552370787048739__)
#define __NOTIFY_RECEIVER_H_INCLUDED_7552370787048739__

//=====================================================================================//
//                                class NotifyReceiver                                 //
//=====================================================================================//
class NotifyReceiver
{
public:
	virtual void notify() = 0;
};

//=====================================================================================//
//                                 class NotifyAdaptor                                 //
//=====================================================================================//
template<typename T>
class NotifyAdaptor : public NotifyReceiver
{
	typedef void (T::* const func_t)();

	T * const m_recv;
	func_t m_func;

public:
	NotifyAdaptor(T *recv, func_t func) : m_recv(recv), m_func(func) {}
	virtual void notify() { (m_recv->*m_func)(); }
};

#endif // !defined(__NOTIFY_RECEIVER_H_INCLUDED_7552370787048739__)