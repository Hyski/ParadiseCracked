#pragma once

#ifndef _WINDOWS_
extern "C"
{
__declspec(dllimport) void * __stdcall LoadLibraryA(const char *);
__declspec(dllimport) void * __stdcall GetProcAddress(void *, const char *);
}
#endif

namespace mll
{

namespace utils
{

//=====================================================================================//
//                                     class vtune                                     //
//=====================================================================================//
class vtune
{
	typedef void __cdecl VTPause_t(void);
	typedef void __cdecl VTResume_t(void);

#ifndef _WINDOWS_
	void *m_dll;
#else
	HMODULE m_dll;
#endif

	VTPause_t *m_pause;
	VTResume_t *m_resume;

public:
	vtune()
	:	m_dll(0), m_pause(0), m_resume(0)
	{
		m_dll = LoadLibraryA("VTuneApi.dll");
		m_pause = (VTPause_t*)GetProcAddress(m_dll,"VTPause");
		m_resume = (VTResume_t*)GetProcAddress(m_dll,"VTResume");
	}

	static void pause();
	static void resume();

private:
	static vtune &inst()
	{
		static vtune v;
		return v;
	}
};

//=====================================================================================//
//                                 void vtune::pause()                                 //
//=====================================================================================//
inline void vtune::pause()
{
	if(inst().m_pause) (*inst().m_pause)();
}

//=====================================================================================//
//                                void vtune::resume()                                 //
//=====================================================================================//
inline void vtune::resume()
{
	if(inst().m_resume) (*inst().m_resume)();
}

}

}
