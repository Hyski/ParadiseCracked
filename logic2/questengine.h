#ifndef QUESTENGINE_H
#define QUESTENGINE_H

//#define QUESTS_LOGGING

#ifdef QUESTS_LOGGING
#define QuestLog logFile["quest_test.log"]
#define JournalLog logFile["quest_journal.log"]
#else
#define QuestLog //logFile["quest_test.log"]
#define JournalLog //logFile["quest_journal.log"]
#endif
//fixme:убрать!
//#include "questManager.h"

struct phrase_s;

class Quest;

/*
class LogicServer_
  {
  public:

    //
    // служба построения отчета по законченному quest'y
    //
    class ReportBuilder{
    public:

       virtual ~ReportBuilder(){}
          
       //начать отчет для квеста
       virtual void Start() = 0;
       //закончить отчет (вывести отчет на экран)
       virtual void Finish() = 0;
       
       enum event_type{
           ET_NONE,
           ET_START,  //начало квеста
           ET_FINISH, //завершение квеста
       };

       //квест завершен
       virtual void AddEvent(event_type type) = 0;
       //собрать статистику по опыту
       virtual void AddExperience(int experience) = 0;
       //собрать статистику по выданным предметам
       virtual void AddItems(const std::string& sys_name) = 0;
    };
      
    //получить доступ к службе составления отчета
    virtual ReportBuilder* GetReportBuilder() = 0;

    enum quest_event_type{
        QET_NEW_QUEST_START,  //начало нового (не системного) квеста
    };

    //уведомление о событии в квестах
    virtual void Notify(quest_event_type event) = 0;
    //узнать отношение существа к команде игрока 
    virtual int GetEntityAlignment(const std::string& actor) const = 0;
    //узнать имя персонажа
    virtual std::string GetHeroName(const std::string& actor) = 0;

    //показать MessageBox 
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //возврат:
    //         true  -  была нажата кнопка OK(Yes)
    //         false -  была нажата кнопка Cancel(No) 
    //
    virtual bool ShowMessageBox(const std::string& text) = 0;

    virtual bool CanGetItems(const std::string &itemset_name)const=0; //можно ли забрать набор предметов у команды
    virtual void GiveItems(const std::string &actor, const std::string &itemset_name)=0;//дать предметы команде
    virtual void TakeItems(const std::string &itemset_name)=0;//отобрать предметы у команды
    virtual bool IsSceneFinished(const std::string &scene_name)const=0; //закончилась ли скриптовая сцена
    virtual bool IsHeroInTeam(const std::string &sys_name)const=0;//есть ли в команде указанный участник
    virtual int  GetCurrentPhase()const=0; //возвращает текущую игровую фазу
    virtual void GiveExperience(const std::string &actor,int value)=0;//дать команде опыт
    virtual void AddHeroInTeam(const std::string &sys_name)=0;      //предложение добавить в команду человека
    virtual void RemoveHeroFromTeam(const std::string &sys_name)=0; //убрать из команды человека
    virtual void EnableLevel(const std::string &lev_name, bool enable)=0; //разрешить/запретить выход на уровень
    virtual void ChangeAlignment(const std::string &side_a, const std::string &side_b, int type)=0;// изменить отношение сторон друг к другу
    virtual void ChangePhase(int Phase)=0; //изменить игровую фазу
    virtual void AddJournalRecord(const std::string& theme ,const std::string& text)=0; //Добавляет запись в журнал
    virtual void RemoveJournalTheme(const std::string& theme)=0; //полностью стирает запись из журнала
    virtual void AddNews(const std::string& sys_name)=0; //добавляет новость в журнал
    virtual void SayPhrase(const std::string &actor, const std::string &text, const std::string &wav_name)=0;
    virtual void SayPhrase(const std::string &actor, const phrase_s& phrase) = 0;

    enum PHRASE_TYPE{PT_USUAL, PT_FIRST};
    //virtual void SayActorPhrase(const std::string &actor, PHRASE_TYPE type)=0;//сказать стандартную фразу на Юз
  };
*/

class LogicServer1{
public:

    virtual ~LogicServer1(){}
    
    //
    // служба построения отчета по законченному quest'y
    //
    class ReportBuilder{
    public:
        
        virtual ~ReportBuilder(){}
        
        //начать отчет для квеста
        virtual void Start() = 0;
        //закончить отчет (вывести отчет на экран)
        virtual void Finish() = 0;
        
        enum event_type{
            ET_NONE,
            ET_START,  //начало квеста
            ET_FINISH, //завершение квеста
        };
        
        //квест завершен
        virtual void AddEvent(event_type type) = 0;
        //собрать статистику по опыту
        virtual void AddExperience(int experience) = 0;
        //собрать статистику по выданным предметам
        virtual void AddItems(const std::string& sys_name) = 0;
    };
    
    //получить доступ к службе составления отчета
    virtual ReportBuilder* GetReportBuilder() = 0;
    
    enum quest_event_type{
        QET_QUEST_FINISHED,   //окончание квеста 
        QET_NEW_QUEST_START,  //начало нового (не системного) квеста
    };

    //информация о квесте передается вместе с QET_QUEST_FINISHED 
    struct quest_info{
        std::string m_quest;

        quest_info(const std::string& sys_name) : m_quest(sys_name) {}
    };
        
    //уведомление о событии в квестах
    virtual void Notify(quest_event_type event, void* info = 0) = 0;

    //
    // класс предоставляющий средства для работы с существом
    //
    class EntityManipulator{
    public:

        virtual ~EntityManipulator(){}

        //узнать тип персонажа
		//    ET_NONE ,ET_HUMAN,ET_VEHICLE ,ET_TRADER, ET_ALL_ENTS 
        virtual entity_type GetType() const = 0;
        //узнать игрока к которому принадлежит существо
		//    PT_NONE,PT_ENEMY,PT_PLAYER,PT_CIVILIAN ,PT_ALL_PLAYERS
        virtual player_type GetPlayer() const = 0;
        //узнать отношение существа к команде игрока 
		//	  RT_NEUTRAL, RT_ENEMY, RT_FRIEND
        virtual RelationType GetRelation2Player() const = 0;

        //узнать имя персонажа
        virtual const std::string& GetName() const = 0;
        //узнать системное имя персонажа
        virtual const std::string& GetSysName() const = 0;

        //дать команде опыт
        virtual void GiveExperience(int value) = 0;
        //дать предметы команде
        virtual void GiveItems(const std::string &itemset_name) = 0;

        enum phrase_type{
            PT_USE,     //фраза на use
            PT_FIRST,   //первая фраза
        };
        
        //сказать фразу
        virtual void SayPhrase(phrase_type type) = 0;
        virtual void SayPhrase(const phrase_s& phrase) = 0;

        //перевести существо в другую команду
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        //если new_ai.empty() == true, то у существа остается 
        //та модель ai с которой оно было расставлено на уровне
        virtual void ChangePlayer(player_type new_player, const std::string& new_ai = std::string()) = 0;
    };

    //
    // сервис для итерации по масииву существ
    //
    class EntityIterator{
    public:

        virtual ~EntityIterator(){}

        //установить параметры итерирования, перейти к первому сущетсву
        virtual void First(const std::string& sys_name, player_type player) = 0;

        //перейти к следущему существу
        virtual void Next() = 0;
        //закончена ли итерация
        virtual bool IsDone() const = 0;

        //получить сылку на манипулятор существа
        virtual EntityManipulator* Get() = 0;
    };

    //получить ссылку на объект итератор
    virtual EntityIterator* GetEntityIterator() = 0;
    
    //показать MessageBox 
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //возврат:
    //         true  -  была нажата кнопка OK(Yes)
    //         false -  была нажата кнопка Cancel(No) 
    //
    virtual bool ShowMessageBox(const std::string& text) = 0;
    
    //отобрать предметы у команды
    virtual void TakeItems(const std::string &itemset_name) = 0;
    //можно ли забрать набор предметов у команды
    virtual bool CanGetItems(const std::string &itemset_name) const = 0;

    //закончилась ли скриптовая сцена
    virtual bool IsSceneFinished(const std::string &scene_name) const = 0;

    //изменить игровую фазу
    virtual void ChangePhase(int Phase) = 0;
    //возвращает текущую игровую фазу
    virtual int  GetCurrentPhase() const = 0;
    //разрешить/запретить выход на уровень
    virtual void EnableLevel(const std::string &lev_name, bool enable) = 0;

    // изменить отношение сторон друг к другу
    virtual void ChangeAlignment(const std::string &side_a, const std::string &side_b, RelationType type) = 0;
    
    //добавляет новость в журнал
    virtual void AddNews(const std::string& sys_name) = 0;
    //полностью стирает запись из журнала
    virtual void RemoveJournalTheme(const std::string& theme) = 0;
    //Добавляет запись в журнал
    virtual void AddJournalRecord(const std::string& theme, const std::string& text) = 0;
	//проверить был ли уничтожен уникальный человек (герой, антигерой, торговец)
	virtual bool IsHeroDead(const std::string &sys_name) const = 0;
};

/*
class QuestMessage_
  {
  public:
    enum MESSAGE_TYPE {MT_USE, MT_KILL, MT_FIRSTUSE};
    enum PHRASE_TYPE {PT_NONE, PT_FIRST, PT_USUAL};
    std::string Actor;
    std::string Subject;
    MESSAGE_TYPE Type;
    LogicServer_ *serv;

    PHRASE_TYPE m_SayPhraseType;
    QuestMessage_():m_SayPhraseType(PT_NONE){};
  };*/
class QuestMessage1
  {
  public:
    enum MESSAGE_TYPE {MT_USE, MT_KILL, MT_FIRSTUSE};

		LogicServer1::EntityManipulator *Subject; //на кого действуют
		LogicServer1::EntityManipulator *Actor;		//кто действует     

    MESSAGE_TYPE Type;
    LogicServer1 *serv;

    QuestMessage1(){};
  };
    enum QUEST_STATE {
      QS_NOTSTARTED,      //еще не начат
      QS_STARTED,         //начат
      QS_FAILED,          //провален  (в ходе исполнения)
      QS_COMPLETE,        //выполнен
      QS_PASSED,          //кончился (нельзя было закончить еще до старта)
      QS_PASSED_OK,       //кончился (был выполнен до момента старта)
      QS_MAXSTATES
      };
class QuestEngine
  {
  public:
    typedef std::vector<Quest *> QuestList;
//   static void  SendMessage (message_t& event);					// Send message to all quests

   static  Quest *Get(const std::string &sys_name);
   static  QUEST_STATE GetQuestState(const std::string &sys_name);
   static  void Init();
   static  void Close();
   static  void Clear();
   static void MakeSaveLoad(SavSlot &slot);

   static  void HandleMessage(QuestMessage1 &mes);
   static  void ChangeEpisode(int num, LogicServer1 *serv);


  private:
   static  QuestList Quests;
   static  int m_Episode;
  };
class QuestEngineJournal
  {
  public:
    friend class QuestEngine;
    friend class Quest;
  protected:
    static void AddRecord(LogicServer1 *serv, const std::string &QuestName, const std::string &Description);
    static void RemoveTheme(LogicServer1 *serv, const std::string &QuestName);
  };
#endif