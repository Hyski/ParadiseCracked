//=====================================================================================//
//                             Sound interface definitions                             //
//                             Copyright by MiST land 2002                             //
//=====================================================================================//
//  Description:                                                                       //
//=====================================================================================//
//  Author: Sergei V. Zagursky aka GvozdodeR                                           //
//=====================================================================================//

#if !defined(__ISOUND_H_INCLUDED_2023399421164473__)
#define __ISOUND_H_INCLUDED_2023399421164473__

//=====================================================================================//
//                                 struct SndServices                                  //
//=====================================================================================//
struct SndServices
{
	long m_Wnd;
	class ci_VFileSystem *m_FileSystem;
	virtual void dbg_printf(short,short,const char *);
};

#if !defined(__SMART_MESSAGE_MACROS__)
#define MACRO_TO_STR(str)			#str
#define	MACRO_TO_STR2(str)			MACRO_TO_STR(str)
#define	MACRO_PLACE_STAMP			__FILE__"("MACRO_TO_STR2(__LINE__)")"
#define SMART_MESSAGE(msg)			message(MACRO_PLACE_STAMP" : "msg)
#define	SMART_WARNING(msg)			SMART_MESSAGE("warning [Paradise Cracked]: "msg)
#define	SMART_ERROR(msg)			SMART_MESSAGE("error [Paradise Cracked]: "msg)
#define __SMART_MESSAGE_MACROS__
#endif

class point3;

typedef point3 snd_vector;
typedef float snd_matrix[4][4];

#define MUFFLE_SPEC

// Хэндл на скрипт источника звука
//=====================================================================================//
//                                 class ci_SndScript                                  //
//=====================================================================================//
class ISndScript
{
};

//=====================================================================================//
//                     class ci_SndEmitter : public ci_Releasable                      //
//=====================================================================================//
class ISndEmitter
{
public:
	virtual void play() = 0;
	virtual void stop() = 0;

	// Установить позицию звука
	virtual void setPosition(const snd_vector &) = 0;
	virtual void setVelocity(const snd_vector &) = 0;
	// etc.

	// Возвращает состояние источника
	virtual bool isPlaying() = 0;

	virtual void Release() = 0;
};

//=====================================================================================//
//                   class ci_SndThemeSession : public ci_Releasable                   //
//=====================================================================================//
class ISndThemeSession
{
public:
	// Сменить музыкальную тему
	virtual void changeTheme(const char *scriptName, const char *newTheme) = 0;
	// Включить/выключить музыкальную тему
	virtual void mute(bool bMute = true) = 0;
	virtual void Release() = 0;
};

//=====================================================================================//
//                     class ci_Sound : public ci_CommonInterface                      //
//=====================================================================================//
class MUFFLE_SPEC ISound
{
public:
	// Список возможных каналов
	// Громкость канала равна Master*ChannelVolume
	enum Channel
	{
		cMaster				=	0,
		cThemes				=	1,
		cSpeech				=	2,
		cEffects			=	3,
		cMenu				=	4,
		cAmbient			=	5,
		cDebug				=	6,
		cTerminator			=	7,
	};

	virtual void manage() = 0;

	// Начать музыкальную сессию
	virtual ISndThemeSession *beginSession() = 0;

	// Создать источник звука
	virtual ISndEmitter *createEmitter(const ISndScript *, const char *waveFile) = 0;
	virtual ISndEmitter *createEmitter(const ISndScript *) = 0;
	virtual ISndEmitter *createEmitter(const char *scriptName) = 0;

	// Проиграть звук. После запуска звука с помощью этих функций 
	// его никак проконтролировать нельзя. Если звук зациклен, то он
	// может быть остановлен вместе со всеми остальными звуками с
	// помощью функции clean()
	virtual void emitSound(const ISndScript *, const char *waveFile) = 0;
	virtual void emitSound(const ISndScript *) = 0;
	virtual void emitSound(const char *scriptName) = 0;

	// Возвращает указатель на скрипт
	virtual const ISndScript *getScript(const char *scriptName) = 0;

	// Установить громкость звука для определенного канала
	virtual void setVolume(Channel, float volume) = 0;

	// Получить громкость звука для определенного канала
	virtual float getVolume(Channel) const = 0;

	// Включить/выключить определенный канал
	virtual void muteChannel(Channel, bool) = 0;

	// Изменить позицию камеры
	virtual void setCamera(const snd_vector &dir,
						   const snd_vector &up,
						   const snd_vector &right,
						   const snd_vector &origin) = 0;

	// Свойства среды
	// ...

	// Статические функции для инициализации звука извне каркаса
public:
	static ISound *instance();
	static void initInstance(SndServices *);
	static void shutInstance();
	static bool isInitialized();
};

#undef MUFFLE_SPEC

#endif // !defined(__ISOUND_H_INCLUDED_2023399421164473__)