/*------------------------------------------------------*/
//     Class Name: CNhtThread
//     Coder: Naughty
//     Date: 18.03.2000 - ..
//     Company: Naughty
/*------------------------------------------------------*/
#ifndef _NHT_THREAD_H_
#define _NHT_THREAD_H_

class CNhtThread
{
private:
	HANDLE m_hThread;
	BOOL m_bThread;
public:
	CNhtThread();
	virtual ~CNhtThread();
public:
	BOOL Run(void);
	void Stop(void);
	inline BOOL CanRun(void);
public:
	virtual unsigned int Main(void);
};

BOOL CNhtThread::CanRun(void)
{
	return m_bThread;
}

#endif