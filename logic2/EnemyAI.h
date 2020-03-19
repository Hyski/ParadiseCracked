/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ��������� ������
                                                                                
                                                                                
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
// ����� - ����������� �� ����������� ��������
//
/////////////////////////////////////////////////////////////////////////////
class EnemyActivityObserver : public ActivityObserver
{
public:
	EnemyActivityObserver() { m_last_event = EV_NONE; }
    //���������� ����������� � �������
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
// ����� - ���� ���������� ���� ������
//
/////////////////////////////////////////////////////////////////////////////
class AIEnemyNode: public AINode
{
public:
	// ��� ���������
	enum MessageType
	{
		MT_ENEMYSPOTTED,
		MT_NEEDCURE
	};

	AIEnemyNode();

	virtual ~AIEnemyNode(){}
	// �������� ��������� �� ������� ����� ����������
	virtual void recieveMessage(MessageType type, void * data) = 0;
	// �������� ������� ������� ��������, ���������� � �����
	virtual point3 getPosition() const = 0;
	// �������� ��������� �� ����������
	SubteamNode* getSubteam() const { return m_subteam; }
	// ���������� ��������� �� ����������
	void setSubteam(SubteamNode* subteam) { m_subteam = subteam; }
	// ������ ������ ��������� ����
	virtual float getComplexity() const { return 1.0f; }
private:
	// ��������� �� ����������
	SubteamNode* m_subteam;
};

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ���������� �����������
//
/////////////////////////////////////////////////////////////////////////////
class SubteamNode: public AINode,
				   private GameObserver
{
public:

	SubteamNode();
	~SubteamNode();

    DCTOR_DEF(SubteamNode)

    //� ���� ������� ���� ������ ��������
    float Think(state_type* st);
    //����������/������������� ����������
    void MakeSaveLoad(SavSlot& st);
	// ������ ������ ��������� ����
	virtual float getComplexity() const;

	// �������� ���� � ������ �����
	void addNode(AIEnemyNode* node) { m_nodes.push_back(node); }

	// ��������� ��������� ���� ������ ������� (����� ����)
	void sendMessage(const AIEnemyNode * sender, AIEnemyNode::MessageType type, void * data, float radius = 0);

	// �������� ��� ����������
	std::string getName() const { return m_name; }

	// ���������� ��� ����������
	void setName(const std::string& str)  { m_name = str; }

	// ������� ���� ����� � �������� ID � ��������� �� ������� ���� �����
	// �� ������ ������
	virtual void deleteChildren(eid_t ID);

private:

	// ��� ����������
	std::string m_name;

    //��������� ������ ����
    void Update(subject_t subj, event_t event, info_t info);
    
    //������ ��������� �����
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
	// ������������ ��������� ����
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};
	
	// ������������ ������� ���������
	enum LookroundState
	{
		LS_FIRST,
		LS_SECOND,
		LS_2BASE
	};
public:
	CommonEnemyNode(eid_t id);
	// ������ ����������� �������
	virtual point3 getPosition() const;
	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	bool ThinkShell(state_type* st);
    void CheckFinished(state_type *st,float *comp);
	bool needAndSeeMedkit(ipnt2_t* target);
	bool SelfCure();
    void SendCureMessage(float dist);
	bool PickUpWeaponAndAmmo(const ipnt2_t &m_target_pnt, WeaponComparator &comparator);
	bool MedkitTake(const ipnt2_t &m_target_pnt, state_type* st);





protected:
	// ������� ��������� ����
	TurnState m_turn_state;
	// ������� ����������
    Activity* m_activity;
		// ������� ��������� ���������
	LookroundState m_lookround_state;
	// ������������� ��������
    eid_t  m_entity;
	// ������� ����, ��� ���� ��������� � �������� �������������
	bool m_initialising;

	EnemyActivityObserver m_activity_observer;	// ����������� �� ����������� ��������
};
/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ������������� �����
//
/////////////////////////////////////////////////////////////////////////////
class FixedEnemyNode: public CommonEnemyNode
{
public:
	// ����������� - id ��������
	FixedEnemyNode(eid_t id = 0);
	~FixedEnemyNode();

	DCTOR_DEF(FixedEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// ��������� ��������������� ���������
	virtual void recieveMessage(MessageType type, void * data) {}

	virtual bool die();
	virtual bool need2Delete() const;
private:

    //��������� ������� ���������
    void Update(subject_t subj, event_t event, info_t info);
    void OnSpawnMsg();	//��������� ��������� � �����������

	// ��������� ��������� �� �������� ��� ���������
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);
	
	bool thinkBase(state_type* st);				// ��������� ������ Base
	bool thinkLookRound(state_type* st);		// ��������� ������ LookRound
	bool thinkAttack(state_type* st);			// ��������� ������ Attack
	bool thinkReturn2Base(state_type* st);		// ��������� ������ Return2Base
	bool thinkWeaponSearch(state_type* st);		// ��������� ������ WeaponSearch
	bool thinkWeaponTake(state_type* st);		// ��������� ������ WeaponTake
	bool thinkMedkitTake(state_type* st);		// ��������� ������ MedkitTake

	// ������������ �������, � ������� ����� ��������� ������������ ����
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

	// ������� �����
	FixedEnemyMode m_mode;

	ipnt2_t m_basepnt;			// ������� ��������� ��������
	float m_basedir;			// ������� ����������� ��������
	
	int m_turn;				// ����� ����� � ���������, ����� ���� �� �����
	float m_target_dir;		// ����������� �� �����, � �������� ����� �����������
	ipnt2_t m_target_pnt;	// �����, ���� ����� ����� ��� ���� ����� �����������
	float m_prev_dir;		// ���������� ����������� ����� ��� ��������� ������ ���������
	pnt_vec_t m_base_field;	// ������ ������ �������� ����
	typedef std::list<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;		  	// ������ ������, ������� ���� ����� ������������ � ���� ����  ��-�� ���������� ����� ����

};

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ����������� �����
//
/////////////////////////////////////////////////////////////////////////////
class PatrolEnemyNode: public CommonEnemyNode
{
public:
	// ����������� - id ��������
	PatrolEnemyNode(eid_t id = 0);
	~PatrolEnemyNode();

	DCTOR_DEF(PatrolEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// ��������� ��������������� ���������
	virtual void recieveMessage(MessageType type, void * data);

	virtual bool die();
	virtual bool need2Delete() const;

private:
    //��������� ������� ���������
    void Update(subject_t subj, event_t event, info_t ptr);

    void OnSpawnMsg() {}	//��������� ��������� � �����������
	// ��������� ��������� �� �������� ��� ���������
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);
    void OnKillEv(BaseEntity* killer, BaseEntity* victim);//��������� �������� �� ��������

	
	bool thinkPatrol(state_type* st);		// ��������� ������ Patrol
	bool thinkAttack(state_type* st);		// ��������� ������ Attack
	bool thinkWeaponSearch(state_type* st);	// ��������� ������ WeaponSearch
	bool thinkThingTake(state_type* st);	// ��������� ������ ThingTake
	bool thinkWeaponTake(state_type* st);	// ��������� ������ WeaponTake
	bool thinkCure(state_type* st);			// ��������� ������ Cure
	bool thinkPursuit(state_type* st);		// ��������� ������ Pursuit

	// ������������ �������, � ������� ����� ��������� ������������ ����
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
	
	PatrolEnemyMode m_mode;		// ������� �����
	
	eid_t m_enemy_id;		// ������������� �����, � ������� �������� ����������
	ipnt2_t m_enemy_pnt;	// ��������� �����, � ������� �������� ����������
	eid_t m_cure_id;		// ������������� ����� �� ����� ����������, �������� ����� ������
	ipnt2_t m_cure_pnt;		// ��������� ����� �� ����� ����������, �������� ����� ������
	ipnt2_t m_shoot_pnt;	// ��������� �����, ������ �� ��� ��������
	bool m_shooted;			// ����, ������������, ��� � ���� ��������
	ipnt2_t m_target_pnt;	// �������� �����
	float m_target_dir;		// �������� ����������� (����)
	ipnt2_t m_last_enemy_pnt;	// �����, ��� � ��������� ��� ����� �����

	typedef std::list<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;		  	// ������ ������, ������� ���� ����� ������������ � ���� ����
											// ��-�� ���������� ����� ����
};

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ����������
//
/////////////////////////////////////////////////////////////////////////////
class AssaultEnemyNode: public CommonEnemyNode
{
public:
	// ����������� - id ��������
	AssaultEnemyNode(eid_t id = 0);
	~AssaultEnemyNode();

	DCTOR_DEF(AssaultEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// ��������� ��������������� ���������
	virtual void recieveMessage(MessageType type, void * data);

	virtual bool die();
	virtual bool need2Delete() const;
private:
    void Update(subject_t subj, event_t event, info_t ptr);	//��������� ������� ���������
    void OnSpawnMsg() {}	// ��������� ��������� � �����������
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);	// ��������� ��������� �� �������� ��� ���������
    void OnKillEv(BaseEntity* killer, BaseEntity* victim);//��������� �������� �� ��������
	
	bool thinkFixed(state_type* st);			// ��������� ������ Fixed
	bool thinkLookround(state_type* st);		// ��������� ������ Lookround
	bool thinkAttack(state_type* st);			// ��������� ������ Attack
	bool thinkWeaponSearch(state_type* st);		// ��������� ������ WeaponSearch
	bool thinkWeaponTake(state_type* st);		// ��������� ������ WeaponTake
	bool thinkPursuit(state_type* st);			// ��������� ������ Pursuit
	bool thinkMedkitTake(state_type* st);		// ��������� ������ MedkitTake

	// ������������ �������, � ������� ����� ��������� ������������ ����
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

	AssaultEnemyMode m_mode;		// ������� �����
	
	int m_turn;				// ����� ����� � ���������, ����� ���� �� �����
	eid_t m_enemy_id;		// ������������� �����, � ������� �������� ����������
	ipnt2_t m_enemy_pnt;	// ��������� �����, � ������� �������� ����������
	ipnt2_t m_shoot_pnt;	// ��������� �����, ������ �� ��� ��������
	bool m_shooted;			// ����, ������������, ��� � ���� ��������
	ipnt2_t m_target_pnt;	// �������� �����
	float m_target_dir;		// �������� ����������� (����)
	ipnt2_t m_last_enemy_pnt;	// �����, ��� � ��������� ��� ����� �����
	float m_prev_dir;		// ���������� ����������� ����� ��� ��������� ������ ���������

	typedef std::list<eid_t> EntityList_t;	
	EntityList_t m_ignored_enemies;			// ������ ������, ������� ���� ����� ������������ � ���� ����
											// ��-�� ���������� ����� ����
	
};

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ������������ ��������� �������
//
/////////////////////////////////////////////////////////////////////////////
class FixedTechEnemyNode: public AIEnemyNode
{
public:
	// ����������� - id ��������
	FixedTechEnemyNode(eid_t id = 0);
	~FixedTechEnemyNode();

	DCTOR_DEF(FixedTechEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// ��������� ��������������� ���������
	virtual void recieveMessage(MessageType type, void * data) {}

	// ������ ����������� �������
	virtual point3 getPosition() const;

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	// ����!
	virtual bool die();
	// ����� �� ������� ����
	virtual bool need2Delete() const;
private:

    //��������� ��������� � �����������
    void OnSpawnMsg();

	// ��������� ������ Base
	bool thinkBase(state_type* st);
	// ��������� ������ Attack
	bool thinkAttack(state_type* st);
	// ��������� ������ Rotate2Base
	bool thinkRotate2Base(state_type* st);

	// ������������ �������, � ������� ����� ��������� ������������ ��������� �������
	enum FixedTechEnemyMode
	{
		FTEM_BASE,
		FTEM_ATTACK,
		FTEM_ROTATE2BASE,
		FTEM_KILLED,
	};

	// ������������ ��������� ����
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};

	// ������� �����
	FixedTechEnemyMode m_mode;
	// ������� ��������� ����
	TurnState m_turn_state;

	// ������������� ��������
    eid_t  m_entity;

	// ������� ����������
    Activity* m_activity;

	// ����������� �� ����������� ��������
	EnemyActivityObserver m_activity_observer;

	// ������� ��������� ��������
	ipnt2_t m_basepnt;
	// ������� ����������� ��������
	float m_basedir;

	// ����������� �� �����, � �������� ����� �����������
	float m_target_dir;
	// ������� ����, ��� ���� ��������� � �������� �������������
	bool m_initialising;
	// ������ ������, ������� ���� ����� ������������ � ���� ����
	// ��-�� ���������� ����� ����
	typedef std::list<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;
};

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ���������� ��������� �������
//
/////////////////////////////////////////////////////////////////////////////
class PatrolTechEnemyNode: public AIEnemyNode,
                           private GameObserver
{
public:
	// ����������� - id ��������
	PatrolTechEnemyNode(eid_t id = 0);
	~PatrolTechEnemyNode();

	DCTOR_DEF(PatrolTechEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// ��������� ��������������� ���������
	virtual void recieveMessage(MessageType type, void * data);

	// ������ ����������� �������
	virtual point3 getPosition() const;

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	// ����!
	virtual bool die();
	// ����� �� ������� ����
	virtual bool need2Delete() const;
private:

    //��������� ������� ���������
    void Update(subject_t subj, event_t event, info_t ptr);

    //��������� ��������� � �����������
    void OnSpawnMsg() {}
	// ��������� ��������� �� �������� ��� ���������
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);

	// ��������� ������ Patrol
	bool thinkPatrol(state_type* st);
	// ��������� ������ Attack
	bool thinkAttack(state_type* st);
	// ��������� ������ Pursuit
	bool thinkPursuit(state_type* st);

	// ������������ �������, � ������� ����� ��������� ���������� ��������� �������
	enum PatrolTechEnemyMode
	{
		PTEM_PATROL,
		PTEM_ATTACK,
		PTEM_PURSUIT,
		PTEM_KILLED,
	};

	// ������������ ��������� ����
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};

	// ������� �����
	PatrolTechEnemyMode m_mode;
	// ������� ��������� ����
	TurnState m_turn_state;

	// ������������� ��������
    eid_t  m_entity;

	// ������� ����������
    Activity* m_activity;

	// ����������� �� ����������� ��������
	EnemyActivityObserver m_activity_observer;

	// ������������� �����, � ������� �������� ����������
	eid_t m_enemy_id;
	// ��������� �����, � ������� �������� ����������
	ipnt2_t m_enemy_pnt;
	// ��������� �����, ������ �� ��� ��������
	ipnt2_t m_shoot_pnt;
	// ����, ������������, ��� � ���� ��������
	bool m_shooted;
	// �������� �����
	ipnt2_t m_target_pnt;
	// �������� ����������� (����)
	float m_target_dir;
	// �����, ��� � ��������� ��� ����� �����
	ipnt2_t m_last_enemy_pnt;
	// ������� ����, ��� ���� ��������� � �������� �������������
	bool m_initialising;
	// ������ ������, ������� ���� ����� ������������ � ���� ����
	// ��-�� ���������� ����� ����
	typedef std::list<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;
};

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ��������� ��������� �������
//
/////////////////////////////////////////////////////////////////////////////
class AssaultTechEnemyNode: public AIEnemyNode,
                            private GameObserver
{
public:
	// ����������� - id ��������
	AssaultTechEnemyNode(eid_t id = 0);
	~AssaultTechEnemyNode();

	DCTOR_DEF(AssaultTechEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// ��������� ��������������� ���������
	virtual void recieveMessage(MessageType type, void * data);

	// ������ ����������� �������
	virtual point3 getPosition() const;

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	// ����!
	virtual bool die();
	// ����� �� ������� ����
	virtual bool need2Delete() const;
private:

    //��������� ������� ���������
    void Update(subject_t subj, event_t event, info_t ptr);
    
    //��������� ��������� � �����������
    void OnSpawnMsg() {}
	// ��������� ��������� �� �������� ��� ���������
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);

	// ��������� ������ Patrol
	bool thinkPatrol(state_type* st);
	// ��������� ������ Attack
	bool thinkAttack(state_type* st);
	// ��������� ������ Pursuit
	bool thinkPursuit(state_type* st);

	// ������������ �������, � ������� ����� ��������� ��������� ��������� �������
	enum AssaultTechEnemyMode
	{
		ATEM_PATROL,
		ATEM_ATTACK,
		ATEM_PURSUIT,
		ATEM_KILLED,
	};

	// ������������ ��������� ����
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};

	// ������� �����
	AssaultTechEnemyMode m_mode;
	// ������� ��������� ����
	TurnState m_turn_state;

	// ������������� ��������
    eid_t  m_entity;

	// ������� ����������
    Activity* m_activity;

	// ����������� �� ����������� ��������
	EnemyActivityObserver m_activity_observer;

	// ������������� �����, � ������� �������� ����������
	eid_t m_enemy_id;
	// ��������� �����, � ������� �������� ����������
	ipnt2_t m_enemy_pnt;
	// ��������� �����, ������ �� ��� ��������
	ipnt2_t m_shoot_pnt;
	// ����, ������������, ��� � ���� ��������
	bool m_shooted;
	// �������� �����
	ipnt2_t m_target_pnt;
	// �������� ����������� (����)
	float m_target_dir;
	// �����, ��� � ��������� ��� ����� �����
	ipnt2_t m_last_enemy_pnt;
	// ������� ����, ��� ���� ��������� � �������� �������������
	bool m_initialising;
	// ������ ������, ������� ���� ����� ������������ � ���� ����
	// ��-�� ���������� ����� ����
	typedef std::list<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;
};

#endif // __ENEMY_AI_H__