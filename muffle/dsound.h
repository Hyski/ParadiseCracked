#if !defined(__DSOUND_H_INCLUDED_5698059465543420__)
#define __DSOUND_H_INCLUDED_5698059465543420__

//=====================================================================================//
//                                  class DirectSound                                  //
//=====================================================================================//
class DirectSound
{
	GUID					m_device;
	ComPtr<IDirectSound8>	m_dsound;

public:
	DirectSound(const GUID &device, HWND);
	~DirectSound();

	IDirectSound8 *dsound() const { return m_dsound; }
};

#endif // !defined(__DSOUND_H_INCLUDED_5698059465543420__)