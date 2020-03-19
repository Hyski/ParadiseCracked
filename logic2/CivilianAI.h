/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: интеллект мирных жителей
                                                                                
                                                                                
   Author: Alexander Garanin (Punch)
		   Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__CIVILIAN_AI_H__)
#define __CIVILIAN_AI_H__

#ifndef _PUNCH_GAMEOBSERVER_H_
#include "gameobserver.h"
#endif

#ifndef _PUNCH_AIBASE_H_
#include "AIBase.h"
#endif 

class Activity;
class BusRoute;
class CheckPoints;
class AINode;

/////////////////////////////////////////////////////////////////////////////
//
// узел на автобус
//
/////////////////////////////////////////////////////////////////////////////
class BusNode: public AINode
{
public:

    enum bus_type{
        BT_BUSY,    //автобус в кот нельзя сесть
        BT_USUAL,   //обычный автобус
    };

    BusNode(eid_t ent_id = 0, BusRoute* route = 0, bus_type type = BT_USUAL);
    ~BusNode();

    DCTOR_DEF(BusNode)

    float Think(state_type* st);
    void MakeSaveLoad(SavSlot& st);

	// расчет полной сложности узла
	virtual float getComplexity() const { return 1.0f; }

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return m_entity; }

	// умри!
	virtual bool die();
	// нужно ли удалять узел
	virtual bool need2Delete() const;
private:
    //спрятать автобус
    void Hide();

    //поставить автобус на маршрут
    void Spawn();

    //инициализация
    bool OnInit();
    //обработка движения
    bool OnMove();
    //respawn
    bool OnRespawn();
    //обработка остановки
    bool OnStop();
    //обработка конечной остановки
    bool OnDest();

    //можем перейти на след точку пути?
    bool CanMove() const;

private:

    enum bus_mode{
        BM_MOVE,
        BM_STOP,
        BM_DEST,
        BM_INIT,
        BM_QUIT,    
        BM_SPAWN,
		BM_KILLED,
    };
 
    bus_mode  m_mode;
    
    int       m_delay;
    ipnt2_t   m_to;
    eid_t     m_entity;
 
    BusRoute* m_route;
    Activity* m_activity;
};

/////////////////////////////////////////////////////////////////////////////
//
// Узел для машин с нецивилизованным движением
//
/////////////////////////////////////////////////////////////////////////////
class FeralCarNode : public AINode, private GameObserver{
public:

    FeralCarNode(eid_t ent = 0, CheckPoints* points = 0);
    ~FeralCarNode();

    DCTOR_DEF(FeralCarNode)

    float Think(state_type* st);
    void MakeSaveLoad(SavSlot& st);
	// расчет полной сложности узла
	virtual float getComplexity() const { return 1.0f; }

private:

    void Update(subject_t subj, event_t event, info_t info);

    //спрятать машину
    void Hide();
    //инициализация
    bool OnInit();
    //respawn
    bool OnSpawn();
    //respawn delay
    bool OnDelay();

private:

    enum car_mode{
        CM_INIT,
        CM_QUIT,
        CM_SPAWN,
        CM_DELAY,
    };

    car_mode m_mode;

    eid_t     m_ent; 
    int          m_delay;
    Activity*    m_activity;
    CheckPoints* m_checks;  
};

/////////////////////////////////////////////////////////////////////////////
//
// узел для одного существа
//
//
/////////////////////////////////////////////////////////////////////////////
class CivilianNode: public AINode, private GameObserver
{
public:

    CivilianNode(eid_t ent_id = 0, CheckPoints* checks = 0);
    ~CivilianNode();

    DCTOR_DEF(CivilianNode)

    float Think(state_type* st);
    void MakeSaveLoad(SavSlot& st);
	// расчет полной сложности узла
	virtual float getComplexity() const { return 1.0f; }

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return m_entity; }

	// умри!
	virtual bool die();
	// нужно ли удалять узел
	virtual bool need2Delete() const;
private:

    //инициализация
    bool OnInitMode(state_type* st);
    //проигрывание ходьбы
    bool OnMoveMode(state_type* st);
    //обработка конца маршрута
    bool OnQuitMode(state_type* st);
    //обработка остановки
    bool OnStopMode(state_type* st);
    //обработка атаки
    bool OnAttackMode(state_type* st);
    //обработка конца ходьбы
    bool OnEndMoveMode(state_type* st);
    //обработка убегания
    bool OnRunAwayMode(state_type* st);
    //взять оружие в руки
    bool OnWeapon2Hands(state_type* st);
    //подобрать предмет на уровне
    bool OnThingTakeMode(state_type* st);
    //зарядить пушку
    bool OnLoadWeaponMode(state_type* st);
    //поиск предметов на уровне
    bool OnThingSearchMode(state_type* st);

    //узнать есть ли оружие у unit'а
    bool HaveWeapon() const;
    
    void Update(subject_t subj, event_t event, info_t info);

    //обработка сообщения о расстановке
    void OnSpawnMsg();
    //обработка сообщения о стрельбе
    void OnShootEv(BaseEntity* who, const point3& where);
    //обработка собщения о повреждении
    void OnHurtEv(BaseEntity* killer, BaseEntity* victim);

    //проигрывание анимации в соотв с m_anim
    void PlayAnim(state_type* st);

private:

    enum civilian_mode{
        CM_INIT,        // инициализация
        CM_QUIT,
        CM_STOP,        // проиграть special
        CM_MOVE,        // пойти к следю точке
        CM_RUNAWAY,     // убегание в случае повреждения
        CM_ENDMOVE,
        CM_ATTACK,      // персонаж атаковать врага
        CM_THING_TAKE,  // подобрать предмет
        CM_THING_SEARCH,// осмотримся в поисках оружия или патронов
        CM_WEAPON2HANDS,// взять оружие в руки
        CM_WEAPON_LOAD, // зарядить пушку 
        CM_KILLED, // приготовился умирать 
    };

    civilian_mode m_mode;

    enum anim_type{
        AT_NONE,
        AT_FEAR,
        AT_SPECIAL,
    };

    anim_type m_anim;
    anim_type m_sav_anim;
 
    int    m_duration;
    float  m_time;

    CheckPoints* m_checks;

    bool   m_was_hurt;
    int    m_delay;
    int    m_run;
    point3 m_dir;

    eid_t  m_enemy;
    eid_t  m_entity;

    Activity* m_activity;
};

/////////////////////////////////////////////////////////////////////////////
//
// узел для человека, который стоит и играет анимацию
//
/////////////////////////////////////////////////////////////////////////////
class FixedNPCNode: public  AINode 
{
public:

    FixedNPCNode(eid_t ent = 0);
    ~FixedNPCNode();

    DCTOR_DEF(FixedNPCNode)

    float Think(state_type* st);
    void MakeSaveLoad(SavSlot& st);
	// расчет полной сложности узла
	virtual float getComplexity() const { return 1.0f; }

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return m_entity; }

	// умри!
	virtual bool die() { return true; }
private:
    float m_time;
    eid_t m_entity;
};

/////////////////////////////////////////////////////////////////////////////
//
// класс - наблюдатель за активностью существа
//
/////////////////////////////////////////////////////////////////////////////
class CivilianActivityObserver : public ActivityObserver
{
public:
	CivilianActivityObserver() { m_last_event = EV_NONE; }
    //обработать уведомление о событии
    virtual void Update(Activity* subj, event_t event, info_t info)
	{
		setLastEvent(static_cast<event_type>(event));
	}

	event_type getLastEvent() { return m_last_event; }
	void setLastEvent(event_type event) { m_last_event = event; }
	void clear() { m_last_event = EV_NONE; }
private:
	event_type m_last_event;
};

/////////////////////////////////////////////////////////////////////////////
//
// узел для рекламного бота
//
/////////////////////////////////////////////////////////////////////////////
class AdvertisingBotNode: public AINode
{
public:
	// конструктор - id существа
	AdvertisingBotNode(eid_t id = 0);
	~AdvertisingBotNode();

	DCTOR_DEF(AdvertisingBotNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);
	// расчет полной сложности узла
	virtual float getComplexity() const { return 1.0f; }

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return m_entity; }

	// умри!
	virtual bool die();
	// нужно ли удалять узел
	virtual bool need2Delete() const;
private:
	// обработка режима Searching
	bool thinkSearching(state_type* st);
	// обработка режима Attack
	bool thinkAttack(state_type* st);
	// обработка режима Rest
	bool thinkRest(state_type* st);

	// перечисление режимов, в которых может находится бот
	enum AdvertisingBotMode
	{
		ABM_SEARCHING,
		ABM_ATTACK,
		ABM_REST,
		ABM_KILLED,
	};

	// перечисление состояний тура
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};

	// текущий режим
	AdvertisingBotMode m_mode;
	// текущее состояние тура
	TurnState m_turn_state;

	// идентификатор существа
    eid_t  m_entity;

	// текущая активность
    Activity* m_activity;

	// наблюдатель за активностью существа
	CivilianActivityObserver m_activity_observer;

	// идентификатор существа, за которым буду охотиться
	eid_t m_enemy_id;
	// заданная точка
	ipnt2_t m_target_pnt;
	// признак того, что юнит находится в процессе инициализации
	bool m_initialising;
	// количество туров в состоянии отдыха
	int m_rest_turn;
	// количество туров в состоянии атаки
	int m_attack_turn;
};

/////////////////////////////////////////////////////////////////////////////
//
// узел для торгового бота
//
/////////////////////////////////////////////////////////////////////////////
class TradingBotNode: public AINode
{
public:
	// конструктор - id существа
	TradingBotNode(eid_t id = 0);
	~TradingBotNode();

	DCTOR_DEF(TradingBotNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);
	// расчет полной сложности узла
	virtual float getComplexity() const { return 1.0f; }

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return m_entity; }

	// умри!
	virtual bool die();
	// нужно ли удалять узел
	virtual bool need2Delete() const;
private:
    //обработка сообщения о расстановке
    void OnSpawnMsg();

	// обработка режима Searching
	bool thinkSearching(state_type* st);
	// обработка режима Attack
	bool thinkAttack(state_type* st);
	// обработка режима Rest
	bool thinkRest(state_type* st);

	// перечисление режимов, в которых может находится бот
	enum TradingBotMode
	{
		TBM_SEARCHING,
		TBM_ATTACK,
		TBM_REST,
		TBM_KILLED,
	};

	// перечисление состояний тура
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};

	// текущий режим
	TradingBotMode m_mode;
	// текущее состояние тура
	TurnState m_turn_state;

	// идентификатор существа
    eid_t  m_entity;

	// текущая активность
    Activity* m_activity;

	// наблюдатель за активностью существа
	CivilianActivityObserver m_activity_observer;

	// идентификатор существа, за которым буду охотиться
	eid_t m_enemy_id;
	// базовая точка торгового бота
	ipnt2_t m_base_pnt;
	// вектор хексов базового поля
	pnt_vec_t m_base_field;
	// заданная точка
	ipnt2_t m_target_pnt;
	// признак того, что юнит находится в процессе инициализации
	bool m_initialising;
	// количество туров в состоянии отдыха
	int m_rest_turn;
	// количество туров в состоянии атаки
	int m_attack_turn;
};


/////////////////////////////////////////////////////////////////////////////
//
// узел для ремонтного бота
//
/////////////////////////////////////////////////////////////////////////////
class RepairBotNode: public AINode
{
public:
	// конструктор - id существа
	RepairBotNode(eid_t id = 0);
	~RepairBotNode();

	DCTOR_DEF(RepairBotNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);
	// расчет полной сложности узла
	virtual float getComplexity() const { return 1.0f; }

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return m_entity; }

	// умри!
	virtual bool die();
	// нужно ли удалять узел
	virtual bool need2Delete() const;
private:
	// обработка режима Searching
	bool thinkGoing(state_type* st);
	// обработка режима Attack
	bool thinkAttack(state_type* st);

	// перечисление режимов, в которых может находится бот
	enum RepairBotMode
	{
		RBM_GOING,
		RBM_ATTACK,
		RBM_KILLED,
	};

	// перечисление состояний тура
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};

	// текущий режим
	RepairBotMode m_mode;
	// текущее состояние тура
	TurnState m_turn_state;

	// идентификатор существа
    eid_t  m_entity;

	// текущая активность
    Activity* m_activity;

	// наблюдатель за активностью существа
	CivilianActivityObserver m_activity_observer;

	// заданная точка
	ipnt2_t m_target_pnt;
	// признак того, что юнит находится в процессе инициализации
	bool m_initialising;
	// количество туров в состоянии атаки
	int m_attack_turn;
};


/////////////////////////////////////////////////////////////////////////////
//
// узел для пофигиста
//
/////////////////////////////////////////////////////////////////////////////
class IndifferentNode: public AINode
{
public:
	// конструктор - id существа
	IndifferentNode(eid_t id = 0);
	~IndifferentNode();

	DCTOR_DEF(IndifferentNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);
	// расчет полной сложности узла
	virtual float getComplexity() const { return 1.0f; }

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return m_entity; }

	// умри!
	virtual bool die() { return true; }
private:
    //обработка сообщения о расстановке
    void OnSpawnMsg();

	// перечисление состояний тура
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};
	// текущее состояние тура
	TurnState m_turn_state;

	// идентификатор существа
    eid_t  m_entity;
	// признак того, что юнит находится в процессе инициализации
	bool m_initialising;
};

#endif // __CIVILIAN_AI_H__