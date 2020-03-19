/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: �������� ��������� AI
                                                                                
                                                                                
   Author:   Alexander Garanin (Punch)
				Mikhail L. Lepakhin (Flif)
***************************************************************/                

#ifndef _PUNCH_AIBASE_H_
#define _PUNCH_AIBASE_H_

#ifndef _PUNCH_GAMEOBSERVER_H_
#include "gameobserver.h"
#endif

#ifndef _PUNCH_SPAWN_H_
#include "Spawn.h"
#endif

class Activity;
class BusRoute;
class CheckPoints;
class SubteamNode;

class AINode;
class AIEnemyNode;
//
// ������� AI
//
class AIFactory{
public:

    //�������� ������ �� ������
    static AIFactory* GetInst(){return &m_factory;}

    //������� AI ��� �������
    AINode* CreateCivilianAI();
    //������� AI �����
    AINode* CreateEnemyAI();

protected:

    //������ �� ���������
    AIFactory(){}

private:

    static AIFactory m_factory;
};

//
// ��� AI ������������ � ���� ������ �� AINode
//
class AINode{
public:

    virtual ~AINode(){}

    DCTOR_ABS_DEF(AINode)

    //� ���� ������� ���� ������ ��������
    virtual float Think(state_type* st) = 0;
    //����������/������������� AI
    virtual void MakeSaveLoad(SavSlot& st) = 0;
	// ������ ������ ��������� ����
	virtual float getComplexity() const = 0;

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return 0; }
	// ������ ����������� �� �������� ����� � �������� ID � ��������� ��
	// ������� ���� ����� �� ������ ������
	virtual void deleteChildren(eid_t ID) { }
	// ����� �� ������� ����
	virtual bool need2Delete() const { return false; }
	// ����!
	virtual bool die() { return false; }
};

//
// ������ ������ ��� ������
//
class EnemyRoot: public AINode, 
                 private GameObserver, 
                 private SpawnObserver
{
public:
	EnemyRoot();
	~EnemyRoot();

    DCTOR_DEF(EnemyRoot)

    //� ���� ������� ���� ������ ��������
    float Think(state_type* st);
    //����������/������������� AI
    void MakeSaveLoad(SavSlot& st);
	// ������ ������ ��������� ����
	virtual float getComplexity() const;

	// ������� ���� ����� � �������� ID � ��������� �� ������� ���� �����
	// �� ������ ������
	virtual void deleteChildren(eid_t ID);

private:
    
    //��������� ����������� ���������
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //��������� ������ ����
    void Update(GameObserver::subject_t subj, GameObserver::event_t type, GameObserver::info_t info);

    //������� ���� AI �� ������ ����������
    AIEnemyNode* CreateNode(BaseEntity* entity, const std::string type);

    // ����� ��������� �����
    typedef std::map<std::string, SubteamNode*> node_map_t;
    node_map_t m_subteams;

    node_map_t::iterator m_cur;
	float m_complexity;
};

//
// ������ ������ ��� ��������
//
class CivilRoot:    public AINode, 
                    private GameObserver, 
                    private SpawnObserver
{
public:

    CivilRoot();
    ~CivilRoot();

    DCTOR_DEF(CivilRoot)

    //� ���� ������� ���� ������ ��������
    float Think(state_type* st);
    //����������/������������� AI
    void MakeSaveLoad(SavSlot& st);
	// ������ ������ ��������� ����
	virtual float getComplexity() const;
	// ������� ���� ����� � �������� ID � ��������� �� ������� ���� �����
	// �� ������ ������
	virtual void deleteChildren(eid_t ID);

private:
    
    //��������� ����������� ���������
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //��������� ������ ����
    void Update(GameObserver::subject_t subj, GameObserver::event_t type, GameObserver::info_t info);

    //������� ���� AI �� ������ ����������
    AINode* CreateNode(BaseEntity* entity);

    //��������� ��� � ������ �������
    void ToLower(std::string& str);
    //�������� ����� �  ������
    void CutWord(std::string& word, std::string& str);

private:
    
    //������ ��������� �����
    typedef std::list<AINode*> node_lst_t;
    node_lst_t m_nodes;

    node_lst_t::iterator m_cur;

	float m_complexity;
};

#endif // _PUNCH_AIBASE_H_