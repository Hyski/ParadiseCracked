#include "precomp.h"
#include "DX8Sound.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Подключим заголовочный файл Win32 системы
#if defined(_UNDER_CARCASS_)
#include <interfaces/sysdep/IWin32System.h>
#endif

#include <list>
#include <string>
#include <stdio.h>

#include "SndScriptWrapper.h"
#include "Emitter.h"
#include "Script.h"
#include "Common.h"
#include "Session.h"
#include "Segment.h"
#include "SegmentState.h"

#include "DM8Core.h"

cc_DX8Sound *cc_DX8Sound::m_Sound = 0;
const char *cc_DX8Sound::m_LogFile = "gvz_sound_core.log";

#if !defined(_UNDER_CARCASS_)
struct cc_SndInit
{
	HWND m_Wnd;
	ci_VFileSystem *m_FileSystem;
};
#endif

//#define __MUTE_ALL_SOUNDS__

cc_DX8Sound::cc_DX8Sound ()
:	m_Dir(0.0f,0.0f,1.0f),
	m_Up(0.0f,1.0f,0.0f),
	m_Right(0.0f,-1.0f,0.0f),
	m_Origin(0.0f,0.0f,0.0f)
{
	for (int i = 0; i < 256; i++)
	{
		m_Volumes[i] = 1.0f;
		m_Muted[i] = false;
	}

#if defined(__MUTE_ALL_SOUNDS__)
	for (int j = 1; j < 256; j++)
	{
		m_Muted[j] = true;
	}
#endif

#if defined(TURN_ON_ALL_DEBUG_STUFF)
	m_Muted[cDebug] = false;
#else
	m_Muted[cDebug] = true;
#endif
}

cc_DX8Sound::~cc_DX8Sound() {}

#if defined(_UNDER_CARCASS_)
void cc_DX8Sound::Tie (bool)
{
	init(0);
}

void cc_DX8Sound::Untie (bool)
{
}
#endif // _UNDER_CARCASS_

void cc_DX8Sound::Release ()
{
	shut();
	delete this;
}

class cc_InitXcpt
{
};

void cc_DX8Sound::init (cc_SndInit *initData)
{
	TRACK_FUNC(cc_DX8Sound::init());
	HWND mainWnd = NULL;

#if defined(_UNDER_CARCASS_)
	if (!CARCASS->GetSystem()->IsSystem(ci_System::SYSTEM_WIN32))
		throw SND_EXCEPTION("Не могу работать на системе, отличной от Win32");

	ci_Win32System *win32 = static_cast<ci_Win32System *>(CARCASS->GetSystem());
	mainWnd = win32->GetHWND();

	m_FileSystem = CARCASS->GetFileSystem();
#else
	mainWnd = initData->m_Wnd;
	m_FileSystem = initData->m_FileSystem;
#endif

	// ВНИМАНИЕ: Небезопасный код
	// Нужен для того, чтобы объекты из cc_DM8Core могли узнать MUSIC_TIME
	m_core = reinterpret_cast<cc_DM8Core*>(::operator new(sizeof(cc_DM8Core)));
	m_core = new(m_core) cc_DM8Core(mainWnd);

//	cc_SndEmitter *emitter = new cc_SndEmitter(m_ScrMgr.getScript("anotherEffect"));
//	emitter->play();

/*	for (int i = 0; i < 10; i++)
	{
		cc_SndEmitter *emitter = new cc_SndEmitter(m_ScrMgr.getScript("tankEngine"));
		emitter->play();
		emitter->setPosition(snd_vector(i*100.0f,0.0f,0.0f));

		Sleep(100);
		manage();
	}*/

//	snd_vector dir(sqrt(2.0f)*0.5f,0.0f,-sqrt(2.0f)*0.5f);
//	snd_vector up(sqrt(2.0f)*0.5f,0.0f,sqrt(2.0f)*0.5f);
/*	float s2d2 = sqrt(2.0f)*0.5f;
	snd_vector dir(5.0f,0.0f,1.0f);
	snd_vector up(-1.0f,0.0f,5.0f);

	for (float arg = 0.0f; arg < 100.0f; arg+=0.1f)
	{
		float targ = 3.14f*sin(arg);
		setCamera
		(
			snd_vector(sin(targ),0.0f,cos(targ)),
			snd_vector(-cos(targ),0.0f,sin(targ)),
			snd_vector(0.0f,0.0f,0.0f),
			snd_vector(0.0f,13.85f,0.813f)
		);
		Sleep(100);
	}*/

/*	{
		IDirectMusicAudioPath8 *path;
		SAFE_CALL(m_Perf->CreateStandardAudioPath(DMUS_APATH_DYNAMIC_3D, 1, TRUE, &path));
		cc_Segment *seg = m_SegMgr.getSegment("sound\\tankEngine.wav");
		seg->addRef();
		SAFE_CALL(seg->getSegment()->Download(path));
		SAFE_CALL(m_Perf->PlaySegmentEx(seg->getSegment(),NULL,NULL,DMUS_SEGF_SECONDARY,0,NULL,NULL,path));
	}
	{
		IDirectMusicAudioPath8 *path;
		SAFE_CALL(m_Perf->CreateStandardAudioPath(DMUS_APATH_DYNAMIC_3D, 1, TRUE, &path));
		cc_Segment *seg = m_SegMgr.getSegment("sound\\e1.wav");
		seg->addRef();
		SAFE_CALL(seg->getSegment()->Download(path));
		SAFE_CALL(m_Perf->PlaySegmentEx(seg->getSegment(),NULL,NULL,DMUS_SEGF_SECONDARY,0,NULL,NULL,path));
	}*/
//	for(int i = 0; i < 10; ++i)
//	{
//	SAFE_CALL(m_Perf->PlaySegmentEx(seg->getSegment(),NULL,NULL,DMUS_SEGF_SECONDARY,0,NULL,NULL,path));
//	}*/
}

void cc_DX8Sound::manage()
{
	m_core->m_emitMgr.manage();
	m_core->m_themeMgr.manage();
}

void cc_DX8Sound::shut()
{
	delete m_core;
	m_core = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// cc_DX8Sound::getTime() /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
MUSIC_TIME cc_DX8Sound::getTime()
{
	MUSIC_TIME time;
	m_core->m_dmusic.performance()->GetTime(NULL,&time);
	return time;
}

ci_SndThemeSession *cc_DX8Sound::beginSession()
{
	return m_core->m_themeMgr.beginSession();
}

ci_SndEmitter *cc_DX8Sound::createEmitter (ci_SndScript *script, const char *wave)
{
	return m_core->m_emitMgr.emitSound((cc_SndScript *)script,wave,cc_EmitMgr::pDestroyAfterStop)->clone();
}

ci_SndEmitter *cc_DX8Sound::createEmitter (ci_SndScript *script)
{
	return m_core->m_emitMgr.emitSound((cc_SndScript *)script,cc_EmitMgr::pDestroyAfterStop)->clone();
}

ci_SndEmitter *cc_DX8Sound::createEmitter (const char *name)
{
	return m_core->m_emitMgr.emitSound(m_core->m_scriptMgr.getScript(name),0)->clone();
}

void cc_DX8Sound::emitSound (ci_SndScript *script, const char *wave)
{
	m_core->m_emitMgr.emitSound((cc_SndScript *)script,wave,cc_EmitMgr::pDestroyAfterStop|cc_EmitMgr::pPlay);
}

void cc_DX8Sound::emitSound (ci_SndScript *script)
{
	m_core->m_emitMgr.emitSound((cc_SndScript *)script,cc_EmitMgr::pDestroyAfterStop|cc_EmitMgr::pPlay);
}

void cc_DX8Sound::emitSound (const char *name)
{
	m_core->m_emitMgr.emitSound(m_core->m_scriptMgr.getScript(name),cc_EmitMgr::pPlay);
}

ci_VFileSystem *cc_DX8Sound::getFileSystem()
{
	return m_FileSystem;
}

ci_SndScript *cc_DX8Sound::getScript(const char *scriptName)
{
	return m_core->m_scriptMgr.getScript(scriptName);
}

ci_SndCache *cc_DX8Sound::getCache(const char *cacheName)
{
	return m_core->m_cacheMgr.getCache(cacheName);
}

void cc_DX8Sound::setVolume(Channel channel, float volume)
{
	TRACK_FUNC(cc_DX8Sound::setVolume());
	// Если уровень громкости не изменился
	if (volume == m_Volumes[channel]) return;

#if defined(TURN_ON_ALL_DEBUG_STUFF)
	{
		char buffer[128];
		sprintf(buffer,"Changed volume for channel %d to %f\n",static_cast<int>(channel),volume);
		CORE_LOG(m_LogFile,"<"+STR_TIME_STAMP+"> "+STR_FULL_STAMP+buffer);
	}
#endif

	// Сохраним новый уровень громкости
	m_Volumes[channel] = volume;

	m_core->m_pathMgr.adjustVolume(channel);
}

float cc_DX8Sound::getVolume(Channel channel) const
{
	if (m_Muted[channel] || m_Muted[cMaster]) return 0.0f;
	return (channel == cMaster)?m_Volumes[cMaster]:m_Volumes[cMaster]*m_Volumes[channel];
}

void cc_DX8Sound::muteChannel(Channel channel, bool mute)
{
	if (m_Muted[channel] == mute) return;
	m_Muted[channel] = mute;

	m_core->m_pathMgr.adjustVolume(channel);
}

void cc_DX8Sound::setCamera(const snd_vector &dir,
							const snd_vector &up,
							const snd_vector &right,
							const snd_vector &origin)
{
#if defined(_USE_CARCASS_)
	m_Dir = snd_vector(dir.x,dir.y,dir.z);
#else
	m_Dir = snd_vector(-dir.x,-dir.y,dir.z);
#endif
	m_Up = snd_vector(0.0f,0.0f,1.0f);//up;
	m_Right = right;
	m_Origin = snd_vector(origin.x,origin.y,origin.z);

//	m_EmitMgr.updateOnCameraMove();

	m_core->m_dsound.listener()->SetPosition(m_Origin.x, m_Origin.y, m_Origin.z, DS3D_IMMEDIATE);
	m_core->m_dsound.listener()->SetOrientation(m_Dir.x, m_Dir.y, m_Dir.z, m_Up.x, m_Up.y, m_Up.z, DS3D_IMMEDIATE);
}

void cc_DX8Sound::translatePosition(snd_vector &absPos)
{
//	absPos.x = absPos.x;
//	absPos.y = absPos.y;
//	absPos.z = absPos.z;
}

snd_vector cc_DX8Sound::getFront() const
{
	return m_Dir;
}

snd_vector cc_DX8Sound::getTop() const
{
	return m_Up;
}

snd_vector cc_DX8Sound::getOrigin() const
{
	return m_Origin;
}

long cc_DX8Sound::normToDecibel(float norm)
{
	static const float minimal = powf(expf(1.0f),-9.6f);
	if (norm < minimal) return -9600;
	if (norm > 1.0f) return 0;
	return static_cast<long>(1000.0*log(norm));
}