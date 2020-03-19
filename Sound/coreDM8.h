#if !defined(__DMUSIC_8_CORE_INCLUDED__)
#define __DMUSIC_8_CORE_INCLUDED__

#include <common/COMPtr.h>

class cc_DirectSound;

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// class cc_DirectMusic //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class cc_DirectMusic
{
	cc_COMPtr<IDirectMusic8> m_dmusic;
	cc_COMPtr<IDirectMusicPerformance8> m_perf;

public:
	cc_DirectMusic(HWND,cc_DirectSound *);
	~cc_DirectMusic();

	inline IDirectMusicPerformance8 *performance() { return m_perf; }
};

#endif