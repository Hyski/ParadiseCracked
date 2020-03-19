#include "precomp.h"
#include "coreDS8.h"
#include "DM8Core.h"
#include "coreDM8.h"

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// cc_DirectMusic::cc_DirectMusic() ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_DirectMusic::cc_DirectMusic(HWND mainWnd, cc_DirectSound *dsound)
{
	TRACK_FUNC(cc_DirectMusic::cc_DirectMusic());
	HRESULT hr;

	hr = CoCreateInstance(
		CLSID_DirectMusic,
		NULL, 
		CLSCTX_INPROC, 
		IID_IDirectMusic8, 
		m_dmusic.asVoidPtr());

	if (FAILED(hr))
	{
		DMCORE_LOG << STR_FULL_STAMP << "Cannot create DirectMusic8 instance due to following reason:\n";
		DMCORE_LOG << "\t" << cc_DM8Error::getDescription(hr) << "\n";
		throw cc_SoundError(hr,"Cannot create DirectMusic8 instance due to following reason");
	}

	DMCORE_LOG << STR_FULL_STAMP << "Initialized DirectMusic8 object\n";

	// Ассоциируем DirectSound8 объект с объектом DirectMusic8
	hr = m_dmusic->SetDirectSound(dsound->dsound(),mainWnd);
	if (FAILED(hr))
	{
		DMCORE_LOG << STR_FULL_STAMP << "Cannot assign DirectSound8 device to DirectMusic8 object\n";
		throw cc_SoundError(hr,"Cannot assign DirectSound8 device to DirectMusic8 object due to following problem");
	}

	hr = CoCreateInstance(CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC, IID_IDirectMusicPerformance8, m_perf.asVoidPtr());
	if (FAILED(hr))
	{
		DMCORE_LOG << STR_FULL_STAMP << "Cannot create DirectMusicPerformance8 object due to following reason:\n";
		DMCORE_LOG << "\t" << cc_DM8Error::getDescription(hr) << "\n";
		throw cc_SoundError(hr,"Cannot create DirectMusicPerformance8 object due to following reason");
	}

	// Проинициализируем перформанс
	cc_COMPtr<IDirectMusic> music;
	cc_COMPtr<IDirectSound> sound;

	hr = m_dmusic->QueryInterface(IID_IDirectMusic,music.asVoidPtr());
	if(FAILED(hr))
	{
		DMCORE_LOG << STR_FULL_STAMP << "DirectMusic8 to DirectMusic typecast failed\n";
		throw cc_SoundError(hr,"DirectMusic8 to DirectMusic typecast failed due to following problem");
	}

	hr = dsound->dsound()->QueryInterface(IID_IDirectSound,sound.asVoidPtr());
	if(FAILED(hr))
	{
		DMCORE_LOG << STR_FULL_STAMP << "DirectSound8 to DirectSound typecast failed\n";
		throw cc_SoundError(hr,"DirectSound8 to DirectSound typecast failed due to following problem");
	}

	DMUS_AUDIOPARAMS dmap;

	dmap.dwSize = sizeof(DMUS_AUDIOPARAMS);
	dmap.fInitNow = TRUE;
	dmap.dwValidData = DMUS_AUDIOPARAMS_VOICES|DMUS_AUDIOPARAMS_FEATURES;
	dmap.dwFeatures = DMUS_AUDIOF_ALL;
	dmap.dwVoices = 1024;

	hr = m_perf->InitAudio(music.ptr(),sound.ptr(),mainWnd,0,/*DMUS_APATH_DYNAMIC_STEREO,*/0,DMUS_AUDIOF_ALL,&dmap);
	if(FAILED(hr))
	{
		DMCORE_LOG << STR_FULL_STAMP << "Cannot initialize DirectMusicPerformance8 object due to following reason:\n";
		DMCORE_LOG << "\t" << cc_DM8Error::getDescription(hr) << "\n";
		throw cc_SoundError(hr,"Cannot initialize DirectMusicPerformance8 object due to following reason");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// cc_DirectMusic::~cc_DirectMusic() ///////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_DirectMusic::~cc_DirectMusic()
{
	m_perf->CloseDown();
	m_perf.Release();
	m_dmusic.Release();
}