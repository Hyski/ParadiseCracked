#include "precomp.h"
#include "dsound.h"
#include "caps.h"

//=====================================================================================//
//                             DirectSound::DirectSound()                              //
//=====================================================================================//
DirectSound::DirectSound(const GUID &device, HWND wnd)
:	m_device(device)
{
	KERNEL_LOG("—оздаю IDirectSound8 объект с GUID " << device << "\n");

	IDirectSound8 *snd;
	HRESULT result;

	if(FAILED(result = DirectSoundCreate8(&device,&snd,NULL)))
	{
		KERNEL_LOG("\t—оздание не удалось по следующей причине:\n\t"
			<< get_error_text(result) << "\n");
		std::ostringstream sstr;
		sstr << "Cannot create DirectSound8 due to following problem:\n"
			 << get_error_text(result) << "\n";
		throw sound_error(sstr.str());
	}

	KERNEL_LOG("\t—оздание удалось. ”казатель равен " << snd << "\n");

	m_dsound.attach(snd);

	SAFE_CALL(m_dsound->SetCooperativeLevel(wnd,DSSCL_PRIORITY));

	dumpCaps(*this);
}

//=====================================================================================//
//                             DirectSound::~DirectSound()                             //
//=====================================================================================//
DirectSound::~DirectSound()
{
	KERNEL_LOG("”ничтожаю объект DirectSound8\n");
}