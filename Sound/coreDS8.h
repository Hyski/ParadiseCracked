#if !defined(__CORE_DIRECT_SOUND_8_INCLUDED__)
#define __CORE_DIRECT_SOUND_8_INCLUDED__

#include <common/COMPtr.h>

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// class cc_DirectSound //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class cc_DirectSound
{
	cc_COMPtr<IDirectSound8> m_dsound;
	cc_COMPtr<IDirectSoundBuffer> m_primary;
	cc_COMPtr<IDirectSound3DListener8> m_listener;

	void logDSoundCaps();

public:
	cc_DirectSound(HWND);
	~cc_DirectSound();

	inline IDirectSound8 *dsound() { return m_dsound; }
	inline IDirectSound3DListener8 *listener() { return m_listener; }
};

#endif