/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: интеллект врагов
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__ENEMY_AI_H__)
#define __ENEMY_AI_H__

#ifndef _PUNCH_GAMEOBSERVER_H_
#include "gameobserver.h"
#endif

#ifndef _PUNCH_AIBASE_H_
#include "AIBase.h"
#endif 

#ifndef _PUNCH_ACTIVITY_H_
#include "activity.h"
#endif

class AINode;

/////////////////////////////////////////////////////////////////////////////
//
// класс - наблюдатель за активностью существа
//
/////////////////////////////////////////////////////////////////////////////
class EnemyActivityObserver : public ActivityObserver
{
public:
	EnemyActivityObserver() { m_last_event = EV_NONE; }
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
// класс - узел интеллекта всех врагов
//
/////////////////////////////////////////////////////////////////////////////
class AIEnemyNode: public AINode
{
public:
	// тип сообщения
	enum MessageType
	{
		MT_ENEMYSPOTTED,
		MT_NEEDCURE
	};

	AIEnemyNode();

	virtual ~AIEnemyNode(){}
	// получить сообщение от другого члена подкоманды
	virtual void recieveMessage(MessageType type, void * data) = 0;
	// получить текущую позицию существа, связанного с узлом
	virtual point3 getPosition() const = 0;
	// получить указатель на подкоманду
	SubteamNode* getSubteam() const { return m_subteam; }
	// установить указатель на подкоманду
	void setSubteam(SubteamNode* subteam) { m_subteam = subteam; }
	// расчет полной сложности узла
	virtual float getComplexity() const { return 1.0f; }
private:
	// указатель на подкоманду
	SubteamNode* m_subteam;
};

/////////////////////////////////////////////////////////////////////////////
//
// узел для управления подкомандой
//
/////////////////////////////////////////////////////////////////////////////
class SubteamNode: public AINode,
				   private GameObserver
{
public:

	SubteamNode();
	~SubteamNode();

    DCTOR_DEF(SubteamNode)

    //в этой функции узел должен подумать
    float Think(state_type* st);
    //сохранение/востановление подкоманды
    void MakeSaveLoad(SavSlot& st);
	// расчет полной сложности узла
	virtual float getComplexity() const;

	// добавить узел в список узлов
	void addNode(AIEnemyNode* node) { m_nodes.push_back(node); }

	// разослать сообщение всем членам команды (кроме себя)
	void sendMessage(const AIEnemyNode * sender, AIEnemyNode::MessageType type, void * data, float radius = 0);

	// получить имя подкоманды
	std::string getName() const { return m_name; }

	// установить имя подкоманды
	void setName(const std::string& str)  { m_name = str; }

	// удалить всех детей с заданным ID и заставить их сделать тоже самое
	// со своими детьми
	virtual void deleteChildren(eid_t ID);

private:

	// имя подкоманды
	std::string m_name;

    //обработка начала хода
    void Update(subject_t subj, event_t event, info_t info);
    
    //список вложенных узлов
    typedef std::list<AIEnemyNode*> node_lst_t;
    node_lst_t m_nodes;

    node_lst_t::iterator m_cur;
	float m_complexity;
};


class WeaponComparator;

class CommonEnemyNode:public AIEnemyNode,
					  protected GameObserver
{
public:
	// перечисление состояний тура
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};
	
	// перечисление приемов разворота
	enum LookroundState
	{
		LS_FIRST,
		LS_SECOND,
		LS_2BASE
	};
public:
	CommonEnemyNode(eid_t id);
	// выдать собственную позицию
	virtual point3 getPosition() const;
	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return m_entity; }

	bool ThinkShell(state_type* st);
    void CheckFinished(state_type *st,float *comp);
	bool needAndSeeMedkit(ipnt2_t* target);
	bool SelfCure();
    void SendCureMessage(float dist);
	bool PickUpWeaponAndAmmo(const ipnt2_t &m_target_pnt, WeaponComparator &comparator);
	bool MedkitTake(const ipnt2_t &m_target_pnt, state_type* st);





protected:
	// текущее состояние тура
	TurnState m_turn_state;
	// текущая активность
    Activity* m_activity;
		// текущее состояние разворота
	LookroundState m_lookround_state;
	// идентификатор существа
    eid_t  m_entity;
	// признак того, что юнит находится в процессе инициализации
	bool m_initialising;

	EnemyActivityObserver m_activity_observer;	// наблюдатель за активностью существа
};
/////////////////////////////////////////////////////////////////////////////
//
// узел для стационарного врага
//
/////////////////////////////////////////////////////////////////////////////
class FixedEnemyNode: public CommonEnemyNode
{
public:
	// конструктор - id существа
	FixedEnemyNode(eid_t id = 0);
	~FixedEnemyNode();

	DCTOR_DEF(FixedEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// обработка внутрикомандных сообщений
	virtual void recieveMessage(MessageType type, void * data) {}

	virtual bool die();
	virtual bool need2Delete() const;
private:

    //обработка игровых сообщений
    void Update(subject_t subj, event_t event, info_t info);
    void OnSpawnMsg();	//обработка сообщения о расстановке

	// обработка сообщения об выстреле или попадании
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);
	
	bool thinkBase(state_type* st);				// обработка режима Base
	bool thinkLookRound(state_type* st);		// обработка режима LookRound
	bool thinkAttack(state_type* st);			// обработка режима Attack
	bool thinkReturn2Base(state_type* st);		// обработка режима Return2Base
	bool thinkWeaponSearch(state_type* st);		// обработка режима WeaponSearch
	bool thinkWeaponTake(state_type* st);		// обработка режима WeaponTake
	bool thinkMedkitTake(state_type* st);		// обработка режима MedkitTake

	// перечисление режимов, в которых может находится стационарный враг
	enum FixedEnemyMode
	{
		FEM_BASE,
		FEM_LOOKROUND,
		FEM_ATTACK,
		FEM_RETURN2BASE,
		FEM_WEAPONSEARCH,
		FEM_WEAPONTAKE,
		FEM_KILLED,
		FEM_MEDKITTAKE,
	};

	// текущий режим
	FixedEnemyMode m_mode;

	ipnt2_t m_basepnt;			// базовое положение существа
	float m_basedir;			// базовое направление существа
	
	int m_turn;				// номер турна в состоянии, когда враг не виден
	float m_target_dir;		// направление на врага, к которому нужно повернуться
	ipnt2_t m_target_pnt;	// точка, куда нужно дойти или куда нужно повернуться
	float m_prev_dir;		// предыдущее направление юнита при очередном приеме разворота
	pnt_vec_t m_base_field;	// вектор хексов базового поля
	typedef std::list<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;		  	// список врагов, которых юнит будет игнорировать в этом туре  из-за отсутствия линии огня

};

/////////////////////////////////////////////////////////////////////////////
//
// узел для патрульного врага
//
/////////////////////////////////////////////////////////////////////////////
class PatrolEnemyNode: public CommonEnemyNode
{
public:
	// конструктор - id существа
	PatrolEnemyNode(eid_t id = 0);
	~PatrolEnemyNode();

	DCTOR_DEF(PatrolEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// обработка внутрикомандных сообщений
	virtual void recieveMessage(MessageType type, void * data);

	virtual bool die();
	virtual bool need2Delete() const;

private:
    //обработка игровых сообщений
    void Update(subject_t subj, event_t event, info_t ptr);

    void OnSpawnMsg() {}	//обработка сообщения о расстановке
	// обработка сообщения об выстреле или попадании
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);
    void OnKillEv(BaseEntity* killer, BaseEntity* victim);//обработка собщения об убийстве

	
	bool thinkPatrol(state_type* st);		// обработка режима Patrol
	bool thinkAttack(state_type* st);		// обработка режима Attack
	bool thinkWeaponSearch(state_type* st);	// обработка режима WeaponSearch
	bool thinkThingTake(state_type* st);	// обработка режима ThingTake
	bool thinkWeaponTake(state_type* st);	// обработка режима WeaponTake
	bool thinkCure(state_type* st);			// обработка режима Cure
	bool thinkPursuit(state_type* st);		// обработка режима Pursuit

	// перечисление режимов, в которых может находится стационарный враг
	enum PatrolEnemyMode
	{
		PEM_PATROL,
		PEM_ATTACK,
		PEM_WEAPONSEARCH,
		PEM_THINGTAKE,
		PEM_WEAPONTAKE,
		PEM_CURE,
		PEM_PURSUIT,
		PEM_KILLED,
	};
	
	PatrolEnemyMode m_mode;		// текущий режим
	
	eid_t m_enemy_id;		// идентификатор врага, о котором сообщила подкоманда
	ipnt2_t m_enemy_pnt;	// положение врага, о котором сообщила подкоманда
	eid_t m_cure_id;		// идентификатор юнита из своей подкоманды, которого нужно лечить
	ipnt2_t m_cure_pnt;		// положение юнита из своей подкоманды, которого нужно лечить
	ipnt2_t m_shoot_pnt;	// положение точки, откуда по мне стреляли
	bool m_shooted;			// флаг, показывающий, что в меня стреляля
	ipnt2_t m_target_pnt;	// заданная точка
	float m_target_dir;		// заданное направление (угол)
	ipnt2_t m_last_enemy_pnt;	// точка, где я последний раз видел врага

	typedef std::list<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;		  	// список врагов, которых юнит будет игнорировать в этом туре
											// из-за отсутствия линии огня
};

/////////////////////////////////////////////////////////////////////////////
//
// узел для штурмовика
//
/////////////////////////////////////////////////////////////////////////////
class AssaultEnemyNode: public CommonEnemyNode
{
public:
	// конструктор - id существа
	AssaultEnemyNode(eid_t id = 0);
	~AssaultEnemyNode();

	DCTOR_DEF(AssaultEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// обработка внутрикомандных сообщений
	virtual void recieveMessage(MessageType type, void * data);

	virtual bool die();
	virtual bool need2Delete() const;
private:
    void Update(subject_t subj, event_t event, info_t ptr);	//обработка игровых сообщений
    void OnSpawnMsg() {}	// обработка сообщения о расстановке
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);	// обработка сообщения об выстреле или попадании
    void OnKillEv(BaseEntity* killer, BaseEntity* victim);//обработка собщения об убийстве
	
	bool thinkFixed(state_type* st);			// обработка режима Fixed
	bool thinkLookround(state_type* st);		// обработка режима Lookround
	bool thinkAttack(state_type* st);			// обработка режима Attack
	bool thinkWeaponSearch(state_type* st);		// обработка режима WeaponSearch
	bool thinkWeaponTake(state_type* st);		// обработка режима WeaponTake
	bool thinkPursuit(state_type* st);			// обработка режима Pursuit
	bool thinkMedkitTake(state_type* st);		// обработка режима MedkitTake

	// перечисление режимов, в которых может находится стационарный враг
	enum AssaultEnemyMode
	{
		AEM_FIXED,
		AEM_LOOKROUND,
		AEM_ATTACK,
		AEM_WEAPONSEARCH,
		AEM_WEAPONTAKE,
		AEM_PURSUIT,
		AEM_KILLED,
		AEM_MEDKITTAKE,
	};

	AssaultEnemyMode m_mode;		// текущий режим
	
	int m_turn;				// номер турна в состоянии, когда враг не виден
	eid_t m_enemy_id;		// идентификатор врага, о котором сообщила подкоманда
	ipnt2_t m_enemy_pnt;	// положение врага, о котором сообщила подкоманда
	ipnt2_t m_shoot_pnt;	// положение точки, откуда по мне стреляли
	bool m_shooted;			// флаг, показывающий, что в меня стреляля
	ipnt2_t m_target_pnt;	// заданная точка
	float m_target_dir;		// заданное направление (угол)
	ipnt2_t m_last_enemy_pnt;	// точка, где я последний раз видел врага
	float m_prev_dir;		// предыдущее направление юнита при очередном приеме разворота

	typedef std::list<eid_t> EntityList_t;	
	EntityList_t m_ignored_enemies;			// список врагов, которых юнит будет игнорировать в этом туре
											// из-за отсутствия линии огня
	
};

/////////////////////////////////////////////////////////////////////////////
//
// узел для стационарной вражеской техники
//
/////////////////////////////////////////////////////////////////////////////
class FixedTechEnemyNode: public AIEnemyNode
{
public:
	// конструктор - id существа
	FixedTechEnemyNode(eid_t id = 0);
	~FixedTechEnemyNode();

	DCTOR_DEF(FixedTechEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// обработка внутрикомандных сообщений
	virtual void recieveMessage(MessageType type, void * data) {}

	// выдать собственную позицию
	virtual point3 getPosition() const;

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

	// обработка режима Base
	bool thinkBase(state_type* st);
	// обработка режима Attack
	bool thinkAttack(state_type* st);
	// обработка режима Rotate2Base
	bool thinkRotate2Base(state_type* st);

	// перечисление режимов, в которых может находится стационарная вражеская техника
	enum FixedTechEnemyMode
	{
		FTEM_BASE,
		FTEM_ATTACK,
		FTEM_ROTATE2BASE,
		FTEM_KILLED,
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
	FixedTechEnemyMode m_mode;
	// текущее состояние тура
	TurnState m_turn_state;

	// идентификатор существа
    eid_t  m_entity;

	// текущая активность
    Activity* m_activity;

	// наблюдатель за активностью существа
	EnemyActivityObserver m_activity_observer;

	// базовое положение существа
	ipnt2_t m_basepnt;
	// базовое направление существа
	float m_basedir;

	// направление на врага, к которому нужно повернуться
	float m_target_dir;
	// признак того, что юнит находится в процессе инициализации
	bool m_initialising;
	// список врагов, которых юнит будет игнорировать в этом туре
	// из-за отсутствия линии огня
	typedef std::list<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;
};

/////////////////////////////////////////////////////////////////////////////
//
// узел для патрульной вражеской техники
//
/////////////////////////////////////////////////////////////////////////////
class PatrolTechEnemyNode: public AIEnemyNode,
                           private GameObserver
{
public:
	// конструктор - id существа
	PatrolTechEnemyNode(eid_t id = 0);
	~PatrolTechEnemyNode();

	DCTOR_DEF(PatrolTechEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// обработка внутрикомандных сообщений
	virtual void recieveMessage(MessageType type, void * data);

	// выдать собственную позицию
	virtual point3 getPosition() const;

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return m_entity; }

	// умри!
	virtual bool die();
	// нужно ли удалять узел
	virtual bool need2Delete() const;
private:

    //обработка игровых сообщений
    void Update(subject_t subj, event_t event, info_t ptr);

    //обработка сообщения о расстановке
    void OnSpawnMsg() {}
	// обработка сообщения об выстреле или попадании
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);

	// обработка режима Patrol
	bool thinkPatrol(state_type* st);
	// обработка режима Attack
	bool thinkAttack(state_type* st);
	// обработка режима Pursuit
	bool thinkPursuit(state_type* st);

	// перечисление режимов, в которых может находится патрульная вражеская техника
	enum PatrolTechEnemyMode
	{
		PTEM_PATROL,
		PTEM_ATTACK,
		PTEM_PURSUIT,
		PTEM_KILLED,
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
	PatrolTechEnemyMode m_mode;
	// текущее состояние тура
	TurnState m_turn_state;

	// идентификатор существа
    eid_t  m_entity;

	// текущая активность
    Activity* m_activity;

	// наблюдатель за активностью существа
	EnemyActivityObserver m_activity_observer;

	// идентификатор врага, о котором сообщила подкоманда
	eid_t m_enemy_id;
	// положение врага, о котором сообщила подкоманда
	ipnt2_t m_enemy_pnt;
	// положение точки, откуда по мне стреляли
	ipnt2_t m_shoot_pnt;
	// флаг, показывающий, что в меня стреляля
	bool m_shooted;
	// заданная точка
	ipnt2_t m_target_pnt;
	// заданное направление (угол)
	float m_target_dir;
	// точка, где я последний раз видел врага
	ipnt2_t m_last_enemy_pnt;
	// признак того, что юнит находится в процессе инициализации
	bool m_initialising;
	// список врагов, которых юнит будет игнорировать в этом туре
	// из-за отсутствия линии огня
	typedef std::list<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;
};

/////////////////////////////////////////////////////////////////////////////
//
// узел для штурмовой вражеской техники
//
/////////////////////////////////////////////////////////////////////////////
class AssaultTechEnemyNode: public AIEnemyNode,
                            private GameObserver
{
public:
	// конструктор - id существа
	AssaultTechEnemyNode(eid_t id = 0);
	~AssaultTechEnemyNode();

	DCTOR_DEF(AssaultTechEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// обработка внутрикомандных сообщений
	virtual void recieveMessage(MessageType type, void * data);

	// выдать собственную позицию
	virtual point3 getPosition() const;

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return m_entity; }

	// умри!
	virtual bool die();
	// нужно ли удалять узел
	virtual bool need2Delete() const;
private:

    //обработка игровых сообщений
    void Update(subject_t subj, event_t event, info_t ptr);
    
    //обработка сообщения о расстановке
    void OnSpawnMsg() {}
	// обработка сообщения об выстреле или попадании
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);

	// обработка режима Patrol
	bool thinkPatrol(state_type* st);
	// обработка режима Attack
	bool thinkAttack(state_type* st);
	// обработка режима Pursuit
	bool thinkPursuit(state_type* st);

	// перечисление режимов, в которых может находится штурмовая вражеская техника
	enum AssaultTechEnemyMode
	{
		ATEM_PATROL,
		ATEM_ATTACK,
		ATEM_PURSUIT,
		ATEM_KILLED,
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
	AssaultTechEnemyMode m_mode;
	// текущее состояние тура
	TurnState m_turn_state;

	// идентификатор существа
    eid_t  m_entity;

	// текущая активность
    Activity* m_activity;

	// наблюдатель за активностью существа
	EnemyActivityObserver m_activity_observer;

	// идентификатор врага, о котором сообщила подкоманда
	eid_t m_enemy_id;
	// положение врага, о котором сообщила подкоманда
	ipnt2_t m_enemy_pnt;
	// положение точки, откуда по мне стреляли
	ipnt2_t m_shoot_pnt;
	// флаг, показывающий, что в меня стреляля
	bool m_shooted;
	// заданная точка
	ipnt2_t m_target_pnt;
	// заданное направление (угол)
	float m_target_dir;
	// точка, где я последний раз видел врага
	ipnt2_t m_last_enemy_pnt;
	// признак того, что юнит находится в процессе инициализации
	bool m_initialising;
	// список врагов, которых юнит будет игнорировать в этом туре
	// из-за отсутствия линии огня
	typedef std::list<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;
};

#endif // __ENEMY_AI_H__