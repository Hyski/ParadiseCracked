#include "precomp.h"
#include "PathMgr.h"
#include "DX8Sound.h"
#include "coreDM8.h"

#include <assert.h>

const char *cc_PathMgr::m_LogFile = "gvz_sound_emit.log";

#if defined(TURN_ON_ALL_DEBUG_STUFF)
const char *cc_AudioPath::m_LogFile = "gvz_sound_audio_path.log";
unsigned cc_AudioPath::m_Counter = 0;
int cc_AudioPath::m_PathCount = 0;
#endif

cc_AudioPath::cc_AudioPath(cc_PathMgr *pathMgr, IDirectMusicAudioPath8 *path, bool b2d)
:	m_Path(path),
	m_2d(b2d),
	m_pathMgr(pathMgr)
{
	TRACK_FUNC(cc_AudioPath::cc_AudioPath());

#if defined(TURN_ON_ALL_DEBUG_STUFF)
	char buffer[64];
	sprintf(buffer,"apath%u",m_Counter++);
	m_name = buffer;
	CORE_LOG(m_LogFile, "<"+STR_TIME_STAMP+"> "+STR_FULL_STAMP+"Created AudioPath ["+m_name+"]\n");
	m_PathCount++;
#endif

	m_Path->AddRef();
}

void cc_AudioPath::setPosition(const snd_vector &pos)
{
	TRACK_FUNC(cc_AudioPath::setPosition());
	assert(!m_2d);
	
	IDirectSound3DBuffer8 *buffer;
	DWORD dwBuffer = 0;

	while (S_OK == m_Path->GetObjectInPath(DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, dwBuffer++, GUID_NULL, 0, IID_IDirectSound3DBuffer8, reinterpret_cast<void * *>(&buffer)))
	{
		buffer->SetPosition(pos.x, pos.y, pos.z, DS3D_IMMEDIATE);
		buffer->Release();
	}
}

cc_AudioPath::~cc_AudioPath()
{
	TRACK_FUNC(cc_AudioPath::~cc_AudioPath());
	m_pathMgr->releasePath(m_Path);
	m_Path.Release();
#if defined(TURN_ON_ALL_DEBUG_STUFF)
	CORE_LOG(m_LogFile, "<"+STR_TIME_STAMP+"> "+STR_FULL_STAMP+"Destroyed ["+m_name+"]\n");
	--m_PathCount;
#endif
}

cc_PathMgr::cc_PathMgr(cc_DirectMusic *music)
:	m_music(music)
{
	TRACK_FUNC(cc_PathMgr::init());
	DWORD type = DMUS_APATH_DYNAMIC_STEREO;
	SAFE_CALL(m_music->performance()->CreateStandardAudioPath(type,1,TRUE,m_2DAmbientPath.ptr()));
	SAFE_CALL(m_music->performance()->CreateStandardAudioPath(type,1,TRUE,m_2DEffectsPath.ptr()));
	SAFE_CALL(m_music->performance()->CreateStandardAudioPath(type,1,TRUE,m_2DMusicPath.ptr()));
}

cc_PathMgr::~cc_PathMgr()
{
	TRACK_FUNC(cc_PathMgr::shut());
	m_2DAmbientPath.Release();
	m_2DEffectsPath.Release();
	m_2DMusicPath.Release();
}

cc_AudioPath *cc_PathMgr::getAudioPath(const PathParameters &pp, ci_Sound::Channel channel)
{
	TRACK_FUNC(cc_PathMgr::getAudioPath());

	DWORD type;

	if (pp.m_Params!=PathParameters::use3D)
	{
		if (pp.m_Params==PathParameters::themes)
		{
			EMIT_LOG(m_LogFile,STR_FULL_STAMP+"Returned 2D music path\n");
			return new cc_AudioPath(this,m_2DMusicPath,true);
		}
		else if (pp.m_Params==PathParameters::ambient)
		{
			EMIT_LOG(m_LogFile,STR_FULL_STAMP+"Returned 2D ambient path\n");
			return new cc_AudioPath(this,m_2DAmbientPath,true);
		}
		else if (pp.m_Params==PathParameters::effects)
		{
			EMIT_LOG(m_LogFile,STR_FULL_STAMP+"Returned 2D effects path\n");
			return new cc_AudioPath(this,m_2DEffectsPath,true);
		}
		else
		{
			EMIT_LOG(m_LogFile,STR_FULL_STAMP+"Returned 2D effects path\n");
			return new cc_AudioPath(this,m_2DEffectsPath,true);
		}
	}
	else
	{
		type = DMUS_APATH_DYNAMIC_3D;
	}

	IDirectMusicAudioPath8 *path;

	// Создадим путь
	SAFE_CALL(m_music->performance()->CreateStandardAudioPath(type,1,TRUE,&path));
	cc_AudioPath *pth = new cc_AudioPath(this,path,false);
	m_Pathes[path] = channel;

	EMIT_LOG(m_LogFile,STR_FULL_STAMP+"Returned 3D path\n");

	path->Release();

	adjustVolume(ci_Sound::cMaster);

	return pth;
}

void cc_PathMgr::releasePath(IDirectMusicAudioPath8 *path)
{
	pathes_t::iterator itor = m_Pathes.begin();

	for (; itor != m_Pathes.end(); ++itor)
	{
		if (itor->first == path)
		{
			m_Pathes.erase(itor);
			break;
		}
	}
}

void cc_PathMgr::adjustVolume(ci_Sound::Channel channel)
{
	pathes_t::iterator itor = m_Pathes.begin();

	if (channel == ci_Sound::cMaster)
	{
		for (; itor != m_Pathes.end(); itor++)
		{
			long vol = cc_DX8Sound::normToDecibel(cc_DX8Sound::m_Sound->getVolume(itor->second));
			itor->first->SetVolume(vol,0);
		}
	}
	else
	{
		for (; itor != m_Pathes.end(); itor++)
		{
			if (itor->second == channel)
			{
				long vol = cc_DX8Sound::normToDecibel(cc_DX8Sound::m_Sound->getVolume(itor->second));
				itor->first->SetVolume(vol,0);
			}
		}
	}

	if ((channel == ci_Sound::cAmbient) || (channel == ci_Sound::cMaster))
	{
		long vol = cc_DX8Sound::normToDecibel(cc_DX8Sound::m_Sound->getVolume(ci_Sound::cAmbient));
		m_2DAmbientPath->SetVolume(vol,0);
	}

	if ((channel == ci_Sound::cEffects) || (channel == ci_Sound::cMaster))
	{
		long vol = cc_DX8Sound::normToDecibel(cc_DX8Sound::m_Sound->getVolume(ci_Sound::cEffects));
		m_2DEffectsPath->SetVolume(vol,0);
	}

	if ((channel == ci_Sound::cThemes) || (channel == ci_Sound::cMaster))
	{
		long vol = cc_DX8Sound::normToDecibel(cc_DX8Sound::m_Sound->getVolume(ci_Sound::cThemes));
		m_2DMusicPath->SetVolume(vol,0);
	}
}