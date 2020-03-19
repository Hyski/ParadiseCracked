#if !defined(__COMPTR_H_INCLUDED_1747580201310565__)
#define __COMPTR_H_INCLUDED_1747580201310565__

//=====================================================================================//
//                                    class ComPtr                                     //
//=====================================================================================//
template<typename T>
class ComPtr
{
	T *m_ptr;

	void _doAddRef() { if(m_ptr) m_ptr->AddRef(); }
	void _doRelease() { if(m_ptr) { m_ptr->Release(); m_ptr = 0; } }

public:
	ComPtr() : m_ptr(0) {}
	ComPtr(T *ptr) : m_ptr(ptr) { _doAddRef(); }
	ComPtr(const ComPtr &ptr) : m_ptr(ptr.m_ptr) { _doAddRef(); }
	~ComPtr() { _doRelease(); }

	void attach(T *ptr)
	{
		if(ptr != m_ptr)
		{
			_doRelease();
			m_ptr = ptr;
		}
	}

	T *detach()
	{
		T *tmp = m_ptr;
		m_ptr = 0;
		return tmp;
	}

	void release()
	{
		_doRelease();
	}

	operator T*() const
	{
		return m_ptr;
	}

	bool operator <(T *ptr) const { return m_ptr < ptr; }
	bool operator !() const { return m_ptr == 0; }
	T **operator &() { return &m_ptr; }
	T &operator *() const { return *m_ptr; }
	T *operator ->() const { return m_ptr; }
	bool operator ==(T *ptr) const { return m_ptr == ptr; }
	T *operator =(T *ptr) { _doRelease(); m_ptr = ptr; _doAddRef(); return m_ptr; }
};

#endif // !defined(__COMPTR_H_INCLUDED_1747580201310565__)