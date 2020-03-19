#if !defined(__WND_HANDLE_H_INCLUDED_8093570542662288__)
#define __WND_HANDLE_H_INCLUDED_8093570542662288__

//=====================================================================================//
//                                   class WndHandle                                   //
//=====================================================================================//
class WndHandle : private noncopyable
{
	HANDLE m_handle;

public:
	WndHandle() : m_handle(CreateEvent(NULL,FALSE,FALSE,NULL))
	{
	}

//	WndHandle(const WndHandle &wh) : m_handle(wh.m_handle)
//	{
//		const_cast<WndHandle&>(wh).m_handle = NULL;
//	}

	~WndHandle()
	{
		if(m_handle != NULL)
		{
			CloseHandle(m_handle);
		}
	}

//	WndHandle &operator=(const WndHandle &wh)
//	{
//		if(&wh != this)
//		{
//			this->~WndHandle();
//			new(this) WndHandle(wh);
//		}
//		return *this;
//	}

	HANDLE handle() { return m_handle; }
};

//=====================================================================================//
//                             struct std::less<HANDLE *>                              //
//=====================================================================================//
template<>
struct std::less<HANDLE *>
{
	bool operator()(const HANDLE *&h1, const HANDLE *&h2)
	{
		return *h1 < *h2;
	}
};


#endif // !defined(__WND_HANDLE_H_INCLUDED_8093570542662288__)