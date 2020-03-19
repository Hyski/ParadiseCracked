#if !defined(__KERNEL_H_INCLUDED_6215049273296640__)
#define __KERNEL_H_INCLUDED_6215049273296640__

//=====================================================================================//
//                                    class Kernel                                     //
//=====================================================================================//
class Kernel : private noncopyable
{
	std::auto_ptr<class DirectSound> m_dsound;
	std::auto_ptr<class PrimaryBuffer> m_primary;

public:
	Kernel(const GUID &, HWND);
	~Kernel();

	DirectSound &dsound() const { return *m_dsound; }
	PrimaryBuffer &primary() const { return *m_primary; }
};

#endif // !defined(__KERNEL_H_INCLUDED_6215049273296640__)