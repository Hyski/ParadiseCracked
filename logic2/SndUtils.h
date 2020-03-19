/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Switching music themes depending on game situations
                
   Author : Lepakhin Mikhail a.k.a Flif
   Edited by: Klimov Alexander a.k.a. DiSpeLL
***************************************************************/

//
// утилиты дл€ работы со звуком
//

#ifndef _PUNCH_DISPELL_SNDUTILS_H_
#define _PUNCH_DISPELL_SNDUTILS_H_

class ThemesManager;
class ISndThemeSession;

namespace SndUtils{

    // типы звуков
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // ¬Ќ»ћјЌ»≈: 
    //     при изменении пор€дка звуков в enum'е необходимо 
    //     изменить пор€док звуков в массиве SndStrReader::m_desc
    //
    enum sound_type{
        SND_DRAG_FROM_HEAD,
        SND_DROP_TO_HEAD,
        SND_DRAG_FROM_BODY,
        SND_DROP_TO_BODY,
        SND_DRAG_FROM_HANDS,
        SND_DROP_TO_HANDS,
        SND_DRAG_FROM_KNEES,
        SND_DROP_TO_KNEES,
        SND_DRAG_FROM_BOX,
        SND_DROP_TO_BOX,
        SND_DRAG_FROM_GROUND,
        SND_DROP_TO_GROUND,
        SND_DRAG_FROM_MARKET,
        SND_DROP_TO_MARKET,
        SND_DRAG_FROM_IMPLANTS,
        SND_DROP_TO_IMPLANTS,
        SND_DRAG_FROM_BACKPACK,
        SND_DROP_TO_BACKPACK,
        SND_UNSUITABLE_PARAMETER,
        SND_INCOMPATIBLE_SUIT_AND_WEAPON,
        SND_BUY_FROM_TRADER_SUCCEEDED,
        SND_BUY_FROM_TRADER_FAILED,
        SND_SELL_TO_TRADER,
        SND_NO_ANY_QUEST,
        SND_NEW_QUEST,
        SND_LEVELUP,
        SND_LOGIC_DENIED,
        SND_JOURNAL_WAS_UPDATED,

        MAX_SOUND,
    };

    //перевести тип звука в его название
    const std::string& Snd2Str(sound_type sound);

    //превратить название файла в корректное им€ звукового файла
    std::string MakeSoundFileName(const std::string& file_name);

    //пуста€ звукова€ сесси€
    class EmptySoundSession{
    public:

        EmptySoundSession();
        ~EmptySoundSession();

    private:

        ISndThemeSession* m_session;
    };
};

//
// проигрыватель звуков
//
class SndPlayer{
public:

    virtual ~SndPlayer(){}

    //singleton
    static SndPlayer* GetInst();

    //инициализаци€/деинициализаци€
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //можно ли проигрывать звук
    virtual bool CanPlay() const = 0;

    //проиграть звук
    virtual void Play(const std::string& snd) = 0;
    //проиграть звук в опред точке уровн€
    virtual void Play(const point3& from, const std::string& snd) = 0;
};

//
// стратеги€ проигрывани€ музыкальных тем
//
class MusicChanger{
public:

    virtual ~MusicChanger(){};

    enum music_type{
        MT_NONE,
        MT_QUIET,
        MT_UNEASY,
        MT_STRAINED, 
    };

    //функци€ анализа игровой ситуации
    virtual music_type GetMusic() = 0;
    //нужно ли сменить музыку?
    virtual bool NeedUpdate() const = 0;
};

//
// проигрыватель музыки - музыкальный автомат
//
class JukeBox{  
public:

    //singleton
    static JukeBox* GetInst();

    ~JukeBox();

    //установить новую стратегию смены музыки
    void SetChanger(MusicChanger* changer);

    //уведомить о смене уровн€
    void Reset();

    //инициализаци€ автомата
    void Init();
    //уведомить автомат о начале нового кадра
    void Tick();

    //возобновить проигрывание темы
    void Resume();
    //остановить проигрывание музыки
    void Suspend();

protected:

    JukeBox();

private:

    void SwitchTheme(MusicChanger::music_type type, bool fsmooth = true);

private:

    float m_update_time;   //врем€ дл€ обновлени€ менежджера
    float m_switch_time;   //врем€ переключени€ темы

    const float m_update_delta;  //интервал обновлени€
    const float m_quiet_delta;   //врем€ перехода на более спокойную тему
    const float m_danger_delta;  //врем€ перехода на интенсивную музыку

    MusicChanger::music_type m_music;

	// player
	ThemesManager*  m_player;
	// interface to the music changer
    MusicChanger*   m_changer;
};

#endif //_PUNCH_DISPELL_SNDUTILS_H_

/**************************************************************************

                                 END OF FILE

***************************************************************************/
