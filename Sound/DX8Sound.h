/***********************************************************************

                          DX8Sound component

                       Copyright by MiST land 2001

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Sergei V. Zagursky (GvozdodeR)

************************************************************************/
#if !defined(__SOUND_H_INCLUDED__01)
#define __SOUND_H_INCLUDED__01

#if defined(_UNDER_CARCASS_)
#include <common/COMPtr.h>
#include <mll/algebra/vector3.h>
#include <mll/algebra/matrix3.h>
#else
#include "COMPtr.h"
#include "../common/3D/geometry.h"
#endif

// DirectAudio заголовки 
#include <dsound.h>
#include <dmusicc.h>
#include <dmusici.h>

#include "ScriptMgr.h"
#include "EmitMgr.h"
#include "SegmentMgr.h"
#include "CacheMgr.h"
#include "Spy.h"
#include "PathMgr.h"
#include "ThemeMgr2.h"

class cc_DM8Core;

class cc_DX8Sound : public ci_Sound
{
	float m_Volumes[256];
	bool m_Muted[256];

	static const char *m_LogFile;
//	void logDSoundCaps();

	ci_VFileSystem *m_FileSystem;

	cc_DM8Core *m_core;

//	cc_ScriptMgr m_ScrMgr;
//	cc_EmitMgr m_EmitMgr;
//	cc_SegmentMgr m_SegMgr;
//	cc_CacheMgr m_CacheMgr;
//	cc_SndSpy m_SndSpy;
//	cc_PathMgr m_PathMgr;
//	cc_ThemeMgr2 m_ThemeMgr;

	snd_vector m_Dir, m_Up, m_Right, m_Origin;

public:
//	cc_COMPtr<IDirectSound8> m_DSound;
//	cc_COMPtr<IDirectSoundBuffer> m_Primary;
//	cc_COMPtr<IDirectSound3DListener8> m_Listener;
//	cc_COMPtr<IDirectMusic8> m_DMusic;
//	cc_COMPtr<IDirectMusicPerformance8> m_Perf;

	cc_DX8Sound ();
	~cc_DX8Sound ();

	MUSIC_TIME getTime();

#if defined(_UNDER_CARCASS_)
	void Tie (bool);
	void Untie (bool);
#endif // _UNDER_CARCASS_

	void Release ();

	static cc_DX8Sound *m_Sound;

//	cc_SegmentMgr *getSegmentMgr ();
//	cc_EmitMgr *getEmitMgr ();
//	cc_ScriptMgr *getScriptMgr ();
//	cc_CacheMgr *getCacheMgr ();
//	cc_SndSpy *getSpy ();
//	cc_PathMgr *getPathMgr();
//	cc_ThemeMgr2 *getThemeMgr();

	ci_VFileSystem *getFileSystem();

	// Проинициализировать устройства
	void init (cc_SndInit *);

	// Деинициализировать устройства
	void shut ();

	// Очистить все кеши звуков и тем
	void clean () {};

	void manage ();

	// Начать музыкальную сессию
	ci_SndThemeSession *beginSession ();

	// Создать источник звука
	ci_SndEmitter *createEmitter (ci_SndScript *, const char *wave);
	ci_SndEmitter *createEmitter (ci_SndScript *);
	ci_SndEmitter *createEmitter (const char *scriptName);

	// Проиграть звук. После запуска звука с помощью этих функций 
	// его никак проконтролировать нельзя. Если звук зациклен, то он
	// может быть остановлен вместе со всеми остальными звуками с
	// помощью функции clean()
	void emitSound (ci_SndScript *, const char *wave);
	void emitSound (ci_SndScript *);
	void emitSound (const char *scriptName);

	// Возвращает хэндл на скрипт источника
	ci_SndScript *getScript (const char *scriptName);

	// Возвращает указатель на кеш
	ci_SndCache *getCache (const char *cacheName);

	// Установить громкость звука для определенного канала
	void setVolume (Channel, float volume);

	// Получить громкость звука для определенного канала
	float getVolume (Channel) const;

	// Приглушить канал
	void muteChannel(ci_Sound::Channel,bool);

	void setCamera (const snd_vector &dir,
					const snd_vector &up,
					const snd_vector &right,
					const snd_vector &origin);

	snd_vector getFront() const;
	snd_vector getTop() const;
	snd_vector getOrigin() const;

	// Перевести позицию звука в координаты относительно камеры
	void translatePosition(snd_vector &absPos);

	// Свойства среды
	// ...

	static long normToDecibel(float);
};

#endif