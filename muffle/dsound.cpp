#include "precomp.h"
#include "dsound.h"
#include "caps.h"

//=====================================================================================//
//                             DirectSound::DirectSound()                              //
//=====================================================================================//
DirectSound::DirectSound(const GUID &device, HWND wnd)
:	m_device(device)
{
	KERNEL_LOG("������ IDirectSound8 ������ � GUID " << device << "\n");

	IDirectSound8 *snd;
	HRESULT result;

	if(FAILED(result = DirectSoundCreate8(&device,&snd,NULL)))
	{
		KERNEL_LOG("\t�������� �� ������� �� ��������� �������:\n\t"
			<< get_error_text(result) << "\n");
		std::ostringstream sstr;
		sstr << "Cannot create DirectSound8 due to following problem:\n"
			 << get_error_text(result) << "\n";
		throw sound_error(sstr.str());
	}

	KERNEL_LOG("\t�������� �������. ��������� ����� " << snd << "\n");

	m_dsound.attach(snd);

	SAFE_CALL(m_dsound->SetCooperativeLevel(wnd,DSSCL_PRIORITY));

	dumpCaps(*this);
}

//=====================================================================================//
//                             DirectSound::~DirectSound()                             //
//=====================================================================================//
DirectSound::~DirectSound()
{
	KERNEL_LOG("��������� ������ DirectSound8\n");
}