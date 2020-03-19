/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: основные структуры AI
                                                                                
                                                                                
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
// фабрика AI
//
class AIFactory{
public:

    //получить ссылку на объект
    static AIFactory* GetInst(){return &m_factory;}

    //создать AI для горожан
    AINode* CreateCivilianAI();
    //создать AI врага
    AINode* CreateEnemyAI();

protected:

    //объект не создается
    AIFactory(){}

private:

    static AIFactory m_factory;
};

//
// Все AI представлено в виде дерева из AINode
//
class AINode{
public:

    virtual ~AINode(){}

    DCTOR_ABS_DEF(AINode)

    //в этой функции узел должен подумать
    virtual float Think(state_type* st) = 0;
    //сохранение/востановление AI
    virtual void MakeSaveLoad(SavSlot& st) = 0;
	// расчет полной сложности узла
	virtual float getComplexity() const = 0;

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return 0; }
	// пошлем уведомление об убийстве детей с заданным ID и заставить их
	// сделать тоже самое со своими детьми
	virtual void deleteChildren(eid_t ID) { }
	// нужно ли удалять узел
	virtual bool need2Delete() const { return false; }
	// умри!
	virtual bool die() { return false; }
};

//
// корень дерева для врагов
//
class EnemyRoot: public AINode, 
                 private GameObserver, 
                 private SpawnObserver
{
public:
	EnemyRoot();
	~EnemyRoot();

    DCTOR_DEF(EnemyRoot)

    //в этой функции узел должен подумать
    float Think(state_type* st);
    //сохранение/востановление AI
    void MakeSaveLoad(SavSlot& st);
	// расчет полной сложности узла
	virtual float getComplexity() const;

	// удалить всех детей с заданным ID и заставить их сделать тоже самое
	// со своими детьми
	virtual void deleteChildren(eid_t ID);

private:
    
    //обработка расстановки персонажа
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //обработка начала хода
    void Update(GameObserver::subject_t subj, GameObserver::event_t type, GameObserver::info_t info);

    //создать узел AI по строке параметров
    AIEnemyNode* CreateNode(BaseEntity* entity, const std::string type);

    // карта вложенных узлов
    typedef std::map<std::string, SubteamNode*> node_map_t;
    node_map_t m_subteams;

    node_map_t::iterator m_cur;
	float m_complexity;
};

//
// корень дерева для штатских
//
class CivilRoot:    public AINode, 
                    private GameObserver, 
                    private SpawnObserver
{
public:

    CivilRoot();
    ~CivilRoot();

    DCTOR_DEF(CivilRoot)

    //в этой функции узел должен подумать
    float Think(state_type* st);
    //сохранение/востановление AI
    void MakeSaveLoad(SavSlot& st);
	// расчет полной сложности узла
	virtual float getComplexity() const;
	// удалить всех детей с заданным ID и заставить их сделать тоже самое
	// со своими детьми
	virtual void deleteChildren(eid_t ID);

private:
    
    //обработка расстановки персонажа
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //обработка начала хода
    void Update(GameObserver::subject_t subj, GameObserver::event_t type, GameObserver::info_t info);

    //создать узел AI по строке параметров
    AINode* CreateNode(BaseEntity* entity);

    //перевести все в нижний регистр
    void ToLower(std::string& str);
    //получить слово в  строке
    void CutWord(std::string& word, std::string& str);

private:
    
    //список вложенных узлов
    typedef std::list<AINode*> node_lst_t;
    node_lst_t m_nodes;

    node_lst_t::iterator m_cur;

	float m_complexity;
};

#endif // _PUNCH_AIBASE_H_