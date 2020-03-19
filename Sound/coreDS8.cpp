#include "precomp.h"
#include "coreDS8.h"
#include "DM8Core.h"

#include <list>

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// cc_DirectSound::cc_DirectSound() ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_DirectSound::cc_DirectSound(HWND mainWnd)
{
	TRACK_FUNC(cc_DirectSound::cc_DirectSound());
	CoInitialize(NULL);

	HRESULT hr = DirectSoundCreate8(
		&DSDEVID_DefaultPlayback, // ТЕСТ: нужно сделать выбор устройства
		m_dsound.ptr(),NULL);

	if (FAILED(hr))
	{
		// Звук не был проинициализирован
		DMCORE_LOG << STR_FULL_STAMP << "Cannot initialize DirectSound8 object due to following reason:\n";
		DMCORE_LOG << "\t" << cc_DM8Error::getDescription(hr) << std::endl;
		throw cc_SoundError(hr,"Cannot initialize DirectSound8 object due to following reason");
	}

	DMCORE_LOG << STR_FULL_STAMP << "Initialized DirectSound8 object\n";

	// Установим уровень доступа
	hr = m_dsound->SetCooperativeLevel(mainWnd, DSSCL_PRIORITY);
	if (FAILED(hr))
	{
		DMCORE_LOG << STR_FULL_STAMP << "Cannot set cooperative level for DirectSound object due to following reason:\n";
		DMCORE_LOG << "\t" << cc_DM8Error::getDescription(hr) << std::endl;
		throw cc_SoundError(hr,"Cannot set cooperative level for DirectSound object due to following reason");
	}

	DMCORE_LOG << STR_FULL_STAMP << "Set cooperative level to DSSCL_PRIORITY\n";

	DSBUFFERDESC desc;

	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
	desc.dwReserved = 0;
	desc.lpwfxFormat = NULL;
	desc.dwBufferBytes = 0;
	desc.guid3DAlgorithm = GUID_NULL;

	SAFE_CALL(m_dsound->CreateSoundBuffer(&desc, m_primary.ptr(), NULL));
	SAFE_CALL(m_primary->QueryInterface(IID_IDirectSound3DListener8, m_listener.asVoidPtr()));
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// cc_DirectSound::~cc_DirectSound() ///////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_DirectSound::~cc_DirectSound()
{
	m_listener.Release();
	m_primary.Release();
	m_dsound.Release();
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// cc_DirectSound::logDSoundCaps() ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_DirectSound::logDSoundCaps()
{
	TRACK_FUNC(cc_DirectSound::logDSoundCaps());
	DMCORE_LOG << STR_FULL_STAMP << "The DirectSound8 object capabilities are:\n";
	DSCAPS dscaps;

	dscaps.dwSize = sizeof(DSCAPS);

	m_dsound->GetCaps(&dscaps);

	// Выведем dwFlags
	std::list<std::string> flags;
	if (dscaps.dwFlags & DSCAPS_CERTIFIED) flags.push_back("CERTIFIED");
	if (dscaps.dwFlags & DSCAPS_CONTINUOUSRATE) flags.push_back("CONTINUOUSRATE");
	if (dscaps.dwFlags & DSCAPS_EMULDRIVER) flags.push_back("EMULDRIVER");
	if (dscaps.dwFlags & DSCAPS_PRIMARY16BIT) flags.push_back("PRIMARY16BIT");
	if (dscaps.dwFlags & DSCAPS_PRIMARY8BIT) flags.push_back("PRIMARY8BIT");
	if (dscaps.dwFlags & DSCAPS_PRIMARYMONO) flags.push_back("PRIMARYMONO");
	if (dscaps.dwFlags & DSCAPS_PRIMARYSTEREO) flags.push_back("PRIMARYSTEREO");
	if (dscaps.dwFlags & DSCAPS_SECONDARY16BIT) flags.push_back("SECONDARY16BIT");
	if (dscaps.dwFlags & DSCAPS_SECONDARY8BIT) flags.push_back("SECONDARY8BIT");
	if (dscaps.dwFlags & DSCAPS_SECONDARYMONO) flags.push_back("SECONDARYMONO");
	if (dscaps.dwFlags & DSCAPS_SECONDARYSTEREO) flags.push_back("SECONDARYSTEREO");

	for (std::list<std::string>::iterator itor = flags.begin(); itor != flags.end(); ++itor)
	{
		if (itor == flags.begin())
		{
			DMCORE_LOG << "\tDSCAPS::dwFlags = ";
		}
		else
		{
			DMCORE_LOG << " | ";
		}

		DMCORE_LOG << *itor;
	}
	DMCORE_LOG << std::endl;

	DMCORE_LOG << "\tDSCAPS::dwMinSecondarySampleRate = " << dscaps.dwMinSecondarySampleRate << "\n";
	DMCORE_LOG << "\tDSCAPS::dwMaxSecondarySampleRate = " << dscaps.dwMaxSecondarySampleRate << "\n";
	DMCORE_LOG << "\tDSCAPS::dwPrimaryBuffers = " << dscaps.dwPrimaryBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwMaxHwMixingAllBuffers = " << dscaps.dwMaxHwMixingAllBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwMaxHwMixingStaticBuffers = " << dscaps.dwMaxHwMixingStaticBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwMaxHwMixingStreamingBuffers = " << dscaps.dwMaxHwMixingStreamingBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwFreeHwMixingAllBuffers = " << dscaps.dwFreeHwMixingAllBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwFreeHwMixingStaticBuffers = " << dscaps.dwFreeHwMixingStaticBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwFreeHwMixingStreamingBuffers = " << dscaps.dwFreeHwMixingStreamingBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwMaxHw3DAllBuffers = " << dscaps.dwMaxHw3DAllBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwMaxHw3DStaticBuffers = " << dscaps.dwMaxHw3DStaticBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwMaxHw3DStreamingBuffers = " << dscaps.dwMaxHw3DStreamingBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwFreeHw3DAllBuffers = " << dscaps.dwFreeHw3DAllBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwFreeHw3DStaticBuffers = " << dscaps.dwFreeHw3DStaticBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwFreeHw3DStreamingBuffers = " << dscaps.dwFreeHw3DStreamingBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwTotalHwMemBytes = " << dscaps.dwTotalHwMemBytes << "\n";
	DMCORE_LOG << "\tDSCAPS::dwFreeHwMemBytes = " << dscaps.dwFreeHwMemBytes << "\n";
	DMCORE_LOG << "\tDSCAPS::dwMaxContigFreeHwMemBytes = " << dscaps.dwMaxContigFreeHwMemBytes << "\n";
	DMCORE_LOG << "\tDSCAPS::dwUnlockTransferRateHwBuffers = " << dscaps.dwUnlockTransferRateHwBuffers << "\n";
	DMCORE_LOG << "\tDSCAPS::dwPlayCpuOverheadSwBuffers = " << dscaps.dwPlayCpuOverheadSwBuffers << "\n";

	DMCORE_LOG << STR_FULL_STAMP << "End of DirectSound capabilities list\n";
}