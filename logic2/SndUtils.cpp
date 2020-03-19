/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Switching music themes depending on game situations
                
   Author : Lepakhin Mikhail a.k.a Flif
   Edited by: Klimov Alexander a.k.a. DiSpeLL
***************************************************************/
#pragma warning(disable:4786)

#include "logicdefs.h"

#include "../sound/ISound.h"
#include "../common/DataMgr/TxtFile.h"

#include "spawn.h"
#include "SndUtils.h"
#include "xlsreader.h"
#include "DirtyLinks.h"
#include "GameObserver.h"

#pragma warning (disable: 4503)

namespace{

    const char* logic_sounds_xls = "scripts/Logic/logic_sounds.txt";

    //
    // читать название звука из файла
    //
    class SndStrReader{
    public:

        SndStrReader()
        {
            enum column_type{
                CT_EVENT,
                CT_SOUND,
                    
                MAX_COLUMNS,
            };

            column columns[MAX_COLUMNS] = 
            {
                column("event", CT_EVENT),
                column("sound", CT_SOUND),                    
            };

            TxtFilePtr txt(logic_sounds_xls);
            txt.ReadColumns(columns, columns + MAX_COLUMNS);
            
            std::string str;
            
            //проиниц массив звуков
            for(int line = 0; line < txt->SizeY(); line++){

                //прочтем системное имя события
                txt->GetCell(line, columns[CT_EVENT].m_index, str);

                //найдем ячейку этого события
                for(int k = 0; k < SndUtils::MAX_SOUND; k++){
                    if(m_desc[k].m_event == str){
                        txt->GetCell(line, columns[CT_SOUND].m_index, m_desc[k].m_sndstr);
                        break;
                    }
                }
            }
        }

        const std::string& GetStr(SndUtils::sound_type type) const
        {
            if(type != m_desc[type].m_type){

                std::ostringstream ostr;

                ostr << "SndStrReader: Попытка проиграть звук неверного типа!" << std::endl << std::endl;

                for(int k = 0; k < SndUtils::MAX_SOUND; k++){
                    if(m_desc[k].m_type == type){
                        ostr << "событие:\t\t" << m_desc[k].m_event << std::endl;
                        break;
                    }
                }

                ostr << "неверный звук:\t" << m_desc[type].m_sndstr << std::endl;
                throw CASUS(ostr.str());
            }

            return m_desc[type].m_sndstr;
        }

    private:
        
        struct snd_desc{
            const char*  m_event;
            std::string  m_sndstr;

            SndUtils::sound_type m_type;

            snd_desc(const char* event, SndUtils::sound_type type) :
                m_event(event), m_type(type) {}
        };

        static snd_desc m_desc[SndUtils::MAX_SOUND]; 
    };
    
    SndStrReader::snd_desc SndStrReader::m_desc [SndUtils::MAX_SOUND] =
    {
        SndStrReader::snd_desc("drag_from_head", SndUtils::SND_DRAG_FROM_HEAD), 
        SndStrReader::snd_desc("drop_to_head", SndUtils::SND_DROP_TO_HEAD),  
        SndStrReader::snd_desc("drag_from_body", SndUtils::SND_DRAG_FROM_BODY),  
        SndStrReader::snd_desc("drop_to_body", SndUtils::SND_DROP_TO_BODY),  
        SndStrReader::snd_desc("drag_from_hands", SndUtils::SND_DRAG_FROM_HANDS ),  
        SndStrReader::snd_desc("drop_to_hands", SndUtils::SND_DROP_TO_HANDS),  
        SndStrReader::snd_desc("drag_from_knees", SndUtils::SND_DRAG_FROM_KNEES),  
        SndStrReader::snd_desc("drop_to_knees", SndUtils::SND_DROP_TO_KNEES),  
        SndStrReader::snd_desc("drag_from_box", SndUtils::SND_DRAG_FROM_BOX),  
        SndStrReader::snd_desc("drop_to_box", SndUtils::SND_DROP_TO_BOX),  
        SndStrReader::snd_desc("drag_from_ground", SndUtils::SND_DRAG_FROM_GROUND),  
        SndStrReader::snd_desc("drop_to_ground", SndUtils::SND_DROP_TO_GROUND),  
        SndStrReader::snd_desc("drag_from_market", SndUtils::SND_DRAG_FROM_MARKET),  
        SndStrReader::snd_desc("drop_to_market", SndUtils::SND_DROP_TO_MARKET ),  
        SndStrReader::snd_desc("drag_from_implants", SndUtils::SND_DRAG_FROM_IMPLANTS),  
        SndStrReader::snd_desc("drop_to_implants", SndUtils::SND_DROP_TO_IMPLANTS),  
        SndStrReader::snd_desc("drag_from_backpack", SndUtils::SND_DRAG_FROM_BACKPACK),  
        SndStrReader::snd_desc("drop_to_backpack", SndUtils::SND_DROP_TO_BACKPACK),  
        SndStrReader::snd_desc("unsuitable_parameter", SndUtils::SND_UNSUITABLE_PARAMETER),  
        SndStrReader::snd_desc("incompatible_suit_and_weapon", SndUtils::SND_INCOMPATIBLE_SUIT_AND_WEAPON),  

        SndStrReader::snd_desc("buy_from_trader_succeed", SndUtils::SND_BUY_FROM_TRADER_SUCCEEDED),  
        SndStrReader::snd_desc("buy_from_trader_failed", SndUtils::SND_BUY_FROM_TRADER_FAILED),  
        SndStrReader::snd_desc("sell_to_trader", SndUtils::SND_SELL_TO_TRADER),  

        SndStrReader::snd_desc("no_any_quest", SndUtils::SND_NO_ANY_QUEST),  
        SndStrReader::snd_desc("new_quest", SndUtils::SND_NEW_QUEST),  
        SndStrReader::snd_desc("levelup", SndUtils::SND_LEVELUP),  
        SndStrReader::snd_desc("logic_denied", SndUtils::SND_LOGIC_DENIED),  
        SndStrReader::snd_desc("journal_was_updated", SndUtils::SND_JOURNAL_WAS_UPDATED),  
    };
}

class ThemesManager
{
	ISndThemeSession *m_Session;

public:
	// определение типа темы и префиксного оператора++ для него
	enum ThemesType{
        none,
		quiet,
		uneasy,
		strained
	};

private:

	// флаг, который показывает, следует ли вообще проигрывать темы
	bool muted;

	// название текущей проигрываемой темы
	std::string cur_theme_name;
	// тип напряженности текущей темы
	ThemesType cur_type;
	// название дескришина для тем
	std::string desc_name;

	typedef std::map<ThemesType,   std::string> CurThemesMap;
	typedef std::map<std::string, CurThemesMap> AllThemesMap;

	CurThemesMap cur_themes;
	AllThemesMap all_themes;

public:

	ThemesManager();
	virtual ~ThemesManager(){};

	// GvozdodeR's additions
	void InitSession();
	void ShutSession();
	// End of GvozdodeR's additions

	// первоначальная загрузка тем из xls
	void LoadThemes();
	// установить новый уровень и эпизод
	void SetLevel(const std::string& LevelName, const std::string& EpisodeName);
	// переключение темы
	void SwitchTheme(ThemesType Type, bool fsmooth = true);

private:

	ThemesManager(const ThemesManager&){};
	const ThemesManager& operator=(const ThemesManager&){};
};

//////////////////////////////////////////////////////////////////////
//
// класс менеджера музыкальных тем
//
//////////////////////////////////////////////////////////////////////

inline ThemesManager::ThemesManager():desc_name("THEME"),
							   cur_theme_name(""),
							   cur_type(quiet),
							   muted(true),
							   m_Session(0)
{
}

void ThemesManager::InitSession()
{
	if (!m_Session) m_Session = ISound::instance()->beginSession();
}

void ThemesManager::ShutSession()
{
	if (m_Session)
	{
		m_Session->Release();
		m_Session = 0;
	}
}

inline ThemesManager::ThemesType& operator++(ThemesManager::ThemesType& t)
{
	switch(t)
	{
    case ThemesManager::none:
        t = ThemesManager::quiet;
        break;
	case ThemesManager::quiet: 
		t = ThemesManager::uneasy; 
		break;
	case ThemesManager::uneasy: 
		t = ThemesManager::strained; 
		break;
	case ThemesManager::strained: 
		t = ThemesManager::quiet; 
		break;
	}

	return t;
}

//
// установить новый уровень и эпизод
//
void ThemesManager::SetLevel(const std::string& LevelName, const std::string& EpisodeName)
{
	// если музыка заглушена - выходим
	if(muted) return;

#pragma SMART_WARNING("Old sound lib")
	if(!this->cur_theme_name.empty())
	{
//		sound::tray tr(&desc_name, &cur_theme_name);
//		sound::dev->SmoothMusicDecay(tr);
		this->cur_theme_name.erase();
	}

	AllThemesMap::const_iterator	 iter = this->all_themes.find(LevelName);
	AllThemesMap::const_iterator end_iter = this->all_themes.end();

	if(iter==end_iter)
	{
		iter=this->all_themes.find(EpisodeName);
		// используем имя эпизода
		if(iter!=end_iter)this->cur_themes=iter->second;
	}
	else
	{
		// используем имя уровня
		this->cur_themes=iter->second;
	}

	CurThemesMap::const_iterator cur_themes_iter     = cur_themes.find(quiet);
	CurThemesMap::const_iterator end_cur_themes_iter = cur_themes.end();
    
//	if(cur_themes_iter!=end_cur_themes_iter)
//	{
// #pragma SMART_WARNING("Old sound lib")
// /*		sound::tray tr(&desc_name, &cur_themes_iter->second);
//		sound::dev->SmoothPlay(tr);*/
//		if (m_Session) m_Session->changeTheme(desc_name.c_str(), cur_themes_iter->second.c_str());
//		
//		this->cur_theme_name = cur_themes_iter->second;
//		this->cur_type       = quiet;
//	}
}

//
// переключение темы
//
void ThemesManager::SwitchTheme(ThemesType Type, bool fsmooth)
{
	// если музыка заглушена - выходим
	if(muted) return;

    std::string old_theme = cur_theme_name;
    
    cur_type = Type;
    cur_theme_name.clear();
    
    if(cur_type != none){
        CurThemesMap::const_iterator iter= cur_themes.find(Type);
        if(iter != cur_themes.end()) cur_theme_name = iter->second;
    }

    if(old_theme != cur_theme_name){

#pragma SMART_WARNING("Old sound lib")
//        sound::tray tr_old(&desc_name, &old_theme);
//        sound::tray tr_new(&desc_name, &cur_theme_name);
        
        if(fsmooth){
            
#pragma SMART_WARNING("Old sound lib")
//            sound::SmoothMusicTransmission(tr_old, tr_new);
            
        }else{
        
#pragma SMART_WARNING("Old sound lib")
//            sound::dev->StopEx(tr_old);
//            if(cur_theme_name.size()) sound::dev->Play(tr_new);
        }
    //fixme:
#pragma SMART_WARNING("возникает ошибка при многократном Save/Load")
        if(!m_Session) InitSession();
    //fixme:

		m_Session->changeTheme(desc_name.c_str(), cur_theme_name.c_str());
    }
}

//
// закрытые функции
//
void ThemesManager::LoadThemes()
{
	// прочтем из опций не заглушена ли музыка
    muted = !DirtyLinks::GetIntOpt(DirtyLinks::OT_ENABLE_THEMES);

	// The path to the themes names
	static const char* _path="scripts/ThemesManager/Themes.txt";

	TxtFile themes_file(DataMgr::Load(_path));
	if(themes_file.SizeY()==0)throw CASUS("ThemesManager: Can't Load Info File!(scripts/ThemesManager/Themes.txt)");

	int i = 1;
	while(i < themes_file.SizeY())
	{
		const std::string& levelname = themes_file(i, 0);
		i++;
		if(levelname.empty())continue; 

		// загрузим из файла темы каждого эпизода
		ThemesType tt = quiet;
		for(int j = 0; j < 3; j++, i++, ++tt)
		{
			this->cur_themes[tt] = themes_file(i, 1);
		}

		// положим это в карту всех тем
		this->all_themes[levelname]=this->cur_themes;
	}

	// закроем файл
	DataMgr::Release(_path);
}

//=================================================================

static const ISndScript *g_GlobalScript = 0;
static const ISndScript *g_GlobalScript2d = 0;

const std::string& SndUtils::Snd2Str(sound_type sound)
{
    static SndStrReader reader;
    return reader.GetStr(sound);
}

//проверить правильность имени звукового файла
std::string SndUtils::MakeSoundFileName(const std::string& file_name)
{
    std::string str = file_name;
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    
    if(str.rfind(".wav") == std::string::npos)
        str += ".wav";
    
    return "sounds/" + str;
}

SndUtils::EmptySoundSession::EmptySoundSession() :
    m_session(ISound::instance()->beginSession())
{
}

SndUtils::EmptySoundSession::~EmptySoundSession()
{
    m_session->Release();
}

//=================================================================

namespace{

    //
    // проигрыватель звуков
    //
    class SndPlayerImp : public  SndPlayer,
                         private GameObserver,
                         private SpawnObserver
    {
    public:

        SndPlayerImp() : m_state(ST_EXIT_LEVEL) {}
        
        void Init()
        {
            m_state = ST_EXIT_LEVEL;

            Spawner::GetInst()->Attach(this, ET_EXIT_LEVEL);

            GameEvMessenger::GetInst()->Attach(this, EV_START_GAME);
            GameEvMessenger::GetInst()->Attach(this, EV_INIT_GAME_LOAD);
        }

        void Shut()
        {
		STACK_GUARD("SndPlayer::Shut");
            Spawner::GetInst()->Detach(this);
            GameEvMessenger::GetInst()->Detach(this);
        }
        
        bool CanPlay() const 
        {
            return m_state == ST_START_GAME;
        }
        
        void Play(const std::string& snd)
        {
            if(!CanPlay()) return;

            std::string tmp = SndUtils::MakeSoundFileName(snd);
            
            if (!g_GlobalScript2d) g_GlobalScript2d = ISound::instance()->getScript("logic_2d");
            ISound::instance()->emitSound(g_GlobalScript2d,tmp.c_str());
        }

        void Play(const point3& from, const std::string& snd)
        {
            if(!CanPlay()) return;

            std::string path = SndUtils::MakeSoundFileName(snd);
            
            if (!g_GlobalScript) g_GlobalScript = ISound::instance()->getScript("onetime");
            ISndEmitter *emitter = ISound::instance()->createEmitter(g_GlobalScript,path.c_str());
            emitter->setPosition(from);
            emitter->play();
            emitter->Release();
        }

    private:

        void Update(SpawnObserver::subject_t subj, SpawnObserver::event_t event, SpawnObserver::info_t info)
        {
            m_state = ST_EXIT_LEVEL;
        }
        
        void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info)
        {
            if(event == EV_START_GAME) m_state = ST_START_GAME;
            if(event == EV_INIT_GAME_LOAD) m_state = ST_LOAD_GAME;
        }

    private:

        enum state_type{
            ST_LOAD_GAME,
            ST_EXIT_LEVEL,
            ST_START_GAME,
        };
        
        state_type  m_state;
    };
}

SndPlayer* SndPlayer::GetInst()
{
    static SndPlayerImp imp;
    return &imp;
}

//=================================================================

JukeBox* JukeBox::GetInst()
{
    static JukeBox imp;
    return &imp;
}

JukeBox::JukeBox() : 
    m_player(new ThemesManager()), m_update_time(0), m_switch_time(0), m_danger_delta(2.0f),
    m_music(MusicChanger::MT_QUIET), m_update_delta(0.5f), m_quiet_delta(5.0f), m_changer(0)
{
}

JukeBox::~JukeBox()
{
    delete m_player;
}

void JukeBox::Tick()
{
STACK_GUARD("JukeBox::Tick");
    //оюновление менеджера тем
    if(m_changer && m_update_time < Timer::GetSeconds()){

        m_update_time = Timer::GetSeconds() + m_update_delta;

        if(m_changer->NeedUpdate()){

            MusicChanger::music_type music = m_changer->GetMusic();

            if(m_music != music){

                float delta = m_music < music ? m_danger_delta : m_quiet_delta;

                m_music = music;
                m_switch_time = Timer::GetSeconds() + delta;
            }
        }
    }

    //переключение тем
    if(m_switch_time && m_switch_time < Timer::GetSeconds()){
        m_switch_time = 0;
        SwitchTheme(m_music);
    }
}

void JukeBox::Reset()
{
    m_changer = 0;
	m_player->ShutSession();

    m_update_time = 0;
    m_switch_time = 0;

    if(Spawner::GetInst()){
        std::string episode = "0";
        episode[0] += Spawner::GetInst()->GetEpisode();
        m_player->SetLevel(DirtyLinks::GetLevelSysName(), episode);
    }

    //SwitchTheme(m_music = MusicChanger::MT_QUIET);
}

void JukeBox::Init()
{
    m_player->LoadThemes();
}

void JukeBox::SetChanger(MusicChanger* changer)
{
	m_player->InitSession();
    if(m_changer = changer) SwitchTheme(m_music = m_changer->GetMusic());
}

void JukeBox::Resume()
{
    m_music = m_changer ? m_changer->GetMusic() : MusicChanger::MT_QUIET;
    SwitchTheme(m_music, false);
}

void JukeBox::Suspend()
{
    SwitchTheme(m_music = MusicChanger::MT_NONE, false);
}

void JukeBox::SwitchTheme(MusicChanger::music_type type, bool fsmooth)
{
    switch(type){
    case MusicChanger::MT_NONE:
        m_player->SwitchTheme(ThemesManager::ThemesType::none, fsmooth);
        break;
    
    case MusicChanger::MT_QUIET:
        m_player->SwitchTheme(ThemesManager::ThemesType::quiet, fsmooth);
        break;
        
    case MusicChanger::MT_UNEASY:
        m_player->SwitchTheme(ThemesManager::ThemesType::uneasy, fsmooth);
        break;
        
    case MusicChanger::MT_STRAINED:
        m_player->SwitchTheme(ThemesManager::ThemesType::strained, fsmooth);
        break;
    }
}

/**************************************************************************

                                 END OF FILE

***************************************************************************/


