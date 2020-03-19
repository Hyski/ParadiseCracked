#ifndef QUESTENGINELOC
#define QUESTENGINELOC
   
       class Quest;

/*
»нтерфейсы дл€ квестов
*/

//абстракци€ задачи квеста
class QuestTask
  {
  public:
    virtual bool Complete(LogicServer1 *serv)const=0;
    virtual bool CantComplete(LogicServer1 *serv)const {return false;};
    virtual void Execute(LogicServer1 *serv) {;};
		virtual bool PlayerPermit(LogicServer1 *serv){return true;};
  };
//абстракци€ зависимости начала квеста
class QuestDependency
  {
  public:
    virtual bool Passed(LogicServer1 *serv)const=0;   //если текущее условие выполнено
    virtual bool CantPass(LogicServer1 *serv)const {return false;}; //если текущее условие невозможно выполнить (пр: нужно quest=notstarted, а он started)
  };
//абстракци€ вознаграждени€ квеста
class QuestBonus
  {
  public:
    virtual void GiveIt(LogicServer1::EntityManipulator *actor, LogicServer1 *serv)const=0;
    virtual void QuestGone(LogicServer1 *serv)const {};//квест завершилс€, либо кончилс€ эпизод
  };



/*
реализации классов
*/

//класс, хран€щий информацию о квесте
//Ёто текстовые строки, звуковые файлы...
 class QuestInfo
  {
   public:
  //ќ—ќЅ≈ЌЌќ—“№:
  //перва€ строка в массиве - текст фразы
  //втора€ строка в массиве - им€ .wav файла озвучки
  std::string StartSpeech[2];
  std::string FailedSpeech[2];
  std::string CompleteSpeech[2];
  std::string PassedSpeech[2];
  std::string PassedOKSpeech[2];
  std::string InProgressSpeech[2];
  std::string JournalRecord;
  std::string Description;
  std::string Name;
   public:
   void CutSoundNames();
  //void MakeSaveLoad(SavSlot &slot);//нет сохранени€, т.к. все заново читаетс€ из таблицы
  };

class Quest
  {
  public:
    friend class QuestEngine;

    QUEST_STATE GetState() const;
    typedef std::list<QuestBonus*> BonusList;
    typedef std::list<QuestDependency*> DepList;
    typedef std::list<QuestTask*> TaskList;


  public:
    QUEST_STATE GetState(){return m_CurState;};
    void HandleMessage(QuestMessage1 &mes);
    Quest():m_CurState(QS_NOTSTARTED),m_AwaitingStatus(AS_WAIT){};
    ~Quest();

    void CheckDependencies(QuestMessage1 *mes);     //проверить услови€ старта (и стартовать если нужно)
    bool CheckState(QuestMessage1 *mes);            //проверить выполнение задач (сменить состо€ние если нужно)
                                                   //возвращает true если была сказана фраза.

  private:
    void GiveBonuses(QUEST_STATE state, QuestMessage1 *mes);
    void MakeSaveLoad(SavSlot &slot);
    bool GoToState(QUEST_STATE state, QuestMessage1 *mes); //дл€ переход€ между состо€ни€ми использовать только эту функцию

    BonusList m_Bonuses[QS_MAXSTATES];
    DepList   m_Dependencies;
    TaskList  m_Tasks;

  private:
    std::string m_SysName;
    std::string m_Holder;
    bool m_HasHolder;
    bool m_AutoStart;
    bool m_AutoFinish;

    QuestInfo m_Speeches;

    QUEST_STATE m_CurState;
  private:
    enum AWAITING_STATUS{AS_MAYSTART,AS_WAIT,AS_NEVER,AS_STARTED};
    AWAITING_STATUS m_AwaitingStatus;
  };


class MessageBase
  {
  public:
    typedef std::map<std::string, int> KilledList;
    typedef std::map<std::string, int> DeadList;
    typedef std::set<std::string> UsedList;
    typedef std::map<std::string,std::string> HeroList;
    static void Clear();
    static void AddMessage(QuestMessage1 &mes);
    static bool IsKilled(const std::string sys_name);
    static bool IsDead(const std::string sys_name);
    static bool IsUsed(const std::string sys_name);
    static QUEST_STATE GetQuestState(const std::string sys_name);
    static void MakeSaveLoad(SavSlot &slot);

    static void StartBonusMode(const std::string &StartBonusQuest);//fixme: заплатка дл€ отсоединени€ героев данных на один квест
    static void HeroWantsJoin(const std::string &sys_name);
    static bool isHeroWantsJoin(const std::string &sys_name);
    static void HeroWalkout(const std::string &sys_name);

  private:
    static std::string StartBonusQuest;
    static KilledList m_Killed;
    static DeadList m_Dead;
    static UsedList      m_Used;
    static HeroList   m_Heroes;

  };

class Q2LUtilities
	{
	public:
		static 	bool IsHeroInTeam(LogicServer1::EntityManipulator *hero)
			{
			return hero->GetPlayer()==PT_PLAYER;
			}
		static 	bool IsHeroInTeam(const std::string &name, LogicServer1 *serv)
			{
			LogicServer1::EntityIterator *it=serv->GetEntityIterator();
			it->First(name,PT_PLAYER);
			if(!it->IsDone())
				return true;
			return false;
			}
		static bool isNotFoe(const std::string &name, LogicServer1 *serv)
			{
			LogicServer1::EntityIterator *it=serv->GetEntityIterator();
			it->First(name,PT_ENEMY);
			while(!it->IsDone())
				{
				if(it->Get() && it->Get()->GetRelation2Player()==RT_ENEMY)
					return false;
				it->Next();
				}
			
			return true;
			}
		static std::string GetHeroName(const std::string &name, LogicServer1 *serv)
			{
			LogicServer1::EntityIterator *it=serv->GetEntityIterator();
			it->First(name,PT_ALL_PLAYERS);
			if(!it->IsDone() && it->Get())
				return it->Get()->GetName();
			return "";
			}
	static void AddHeroInTeam(const std::string &name, LogicServer1 *serv)
		{
		if(IsHeroInTeam(name,serv)) return;

			LogicServer1::EntityIterator *it=serv->GetEntityIterator();
			it->First(name,PT_ALL_PLAYERS);
			if(!it->IsDone() && it->Get())
				{
				it->Get()->ChangePlayer(PT_PLAYER);
				}
		}
	static void RemoveHeroFromTeam(const std::string &name, LogicServer1 *serv)
		{
		if(!IsHeroInTeam(name,serv)) return;
			LogicServer1::EntityIterator *it=serv->GetEntityIterator();
			it->First(name,PT_ALL_PLAYERS);
			if(!it->IsDone() && it->Get())
				{
				it->Get()->ChangePlayer(PT_CIVILIAN);
				}
		}
	};

#endif