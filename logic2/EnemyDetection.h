/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: механизм определения врага
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__ENEMY_DETECTION_H__)
#define __ENEMY_DETECTION_H__

#ifndef _PUNCH_GAMEOBSERVER_H_
#include "gameobserver.h"
#endif

class BaseEntity;
class SpawnObserver;
class RelationsObserver;
class RelationsObserversContainer;

//////////////////////////////////////////////////////////////////////////////
//
// класс, наблюдатель за изменением отношений между существами
//
//////////////////////////////////////////////////////////////////////////////
class RelationsObserver
{
public:
	enum event_type
	{
		ET_RELATION2PLAYER_CHANGED,
		ET_RELATIONS_CHANGED
	};
	struct EntityDesc
	{
		eid_t m_id;
		EntityDesc(eid_t id) : m_id(id) {}
	};
	struct RelationDesc
	{
		std::string m_first_subteam;
		std::string m_second_subteam;
		RelationType m_relation_type;
		bool isFirstPlayer() const { return m_first_subteam == "player"; }
	};
	virtual void changed(event_type et, void* param) = 0;
};


//////////////////////////////////////////////////////////////////////////////
//
// класс для хранения зарегистрированных наблюдателей и уведомления их
//
//////////////////////////////////////////////////////////////////////////////
class RelationsObserversContainer
{
public:
	// зарегистрировать наблюдателя
	void attach(RelationsObserver* observer, RelationsObserver::event_type type);
	// отсоединить наблюдателя
	void detach(RelationsObserver* observer);
	// деструктор
	~RelationsObserversContainer();
	// уведомление о том, что юнит поменял отношение к игроку
	void notifyEntityChangedRelationWithPlayer(eid_t entity);
	// уведомление о том, что подкоманда стала врагом игрока
	void notifySubteamChangedRelationWithPlayer(const std::string& name);
	// уведомление о том, что подкоманда изменила отношение к другой подкоманде
	void notifySubteam1ChangedRelationWithSubteam2(BaseEntity* entity1, BaseEntity* entity2, RelationType type);
	void notifySubteam1ChangedRelationWithSubteam2(const std::string& subteam1, const std::string& subteam2, RelationType type);
	// очистить наблюдателей
	void clear();
private:
	typedef std::set<RelationsObserver*> Observers;
	Observers m_player_observers;
	Observers m_anybody_observers;
};

//////////////////////////////////////////////////////////////////////////////
//
// класс, предоставляющий интерфейс для определения врага
//
//////////////////////////////////////////////////////////////////////////////
class EnemyDetector: private GameObserver
{
public:
	// получить указатель на единственный объект класса
	static EnemyDetector * getInst();

	// деструктор
	~EnemyDetector();

	// проверить является ли второе существо для первого существа врагом
	bool isHeEnemy4Me(BaseEntity * me, BaseEntity * he);

	// вернуть тип отношений между игроком и существом
	RelationType getRelationBetweenPlayerAndHe(BaseEntity * he);

	// проинициализировать класс определения врагов
	void init();

	// очистить информацию, хранимую только для текущего уровня
	void clearCurLevelInfo();

    //сохранение/востановление
    void MakeSaveLoad(SavSlot& st);

	// зарегистрировать наблюдателя
	void attach(RelationsObserver* observer, RelationsObserver::event_type type);
	// отсоединить наблюдателя
	void detach(RelationsObserver* observer);

    //Punch: Это для заплатки в иконках видимости
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  	// явное уведомление о том, что юнит поменял отношение к игроку
	void notifyEntityChangedRelationWithPlayer(eid_t entity)
    { m_observers_container.notifyEntityChangedRelationWithPlayer(entity); }       

	// принудительно изменить отношения между двумя подкомандами.
	// после вызова подкоманда 1 будет относиться к подкоманде 2 как к new_relation
	void setRelation(const std::string& subteam1, const std::string& subteam2, RelationType new_relation);

	// получить имя подкоманды по ее системному имени
	std::string getName(const std::string& sys_name);

private:

	RelationsObserversContainer m_observers_container;

	// загрузить первоначальные сведения о расстановке сил из xls'ки
	void loadAlignment();
	// обработка сообщения о ранении существа на уровне
    void Update(subject_t subj, event_t event, info_t info);

	// удалить подкоманду 2 из множества врагов и друзей подкоманды 1
	void clearRelation(const std::string& subteam1, const std::string& subteam2);
	// сделать подкоманду 2 врагом подкоманды 1
	void setEnemyRelation(const std::string& subteam1, const std::string& subteam2);
	// сделать подкоманду 2 другом подкоманды 1
	void setFriendRelation(const std::string& subteam1, const std::string& subteam2);

	// карта подкоманд и их номеров
	typedef std::map<std::string, int> subteam_map_t;
	// множество номеров подкоманд
	typedef std::set<int> subteam_set_t;
	// карта, которая хранит по номеру подкоманды множество всех номеров ее врагов
	typedef std::map<int, subteam_set_t> relation_map_t;
	// карта, которая конвертирует системное имя команды в обычное
	typedef std::map<std::string, std::string> convert_map_t;

	// все подкоманды
	subteam_map_t m_subteams;
	// отношения между подкомандами
	relation_map_t m_relations;
	// здесь храняться друзья
	relation_map_t m_friends;
	// текущий номер для следующей подкоманды в карте номеров подкоманд
	int m_next_subteam_number;

	// карта: ключ - номер подкоманды, значение - список номеров мирных жителей (врагов)
	relation_map_t m_subteam_vs_civilians;
	// карта: ключ - номер мирного жителя, значение - список номеров подкоманд (врагов)
	relation_map_t m_civilian_vs_subteams;
	// карта: ключ - номер мирного жителя, значение - список номеров мирных жителей (врагов)
	relation_map_t m_civilian_vs_civilians;

	// карта для конвертации системного имени в обычное
	convert_map_t m_convert_map;

	// скроем конструктор чтобы никто не мог создать объект класса сам
	EnemyDetector();

	// указатель на класс для получения сообщений о загрузке нового уровня
	SpawnObserver* m_loadlevel_observer;
	// специальная структура для хранения и уничтожения
	// объектов, находящихся в статических указателях
	struct Container
	{
		// указатель на единственный объект
		EnemyDetector * m_detector;

		~Container()
		{
			delete m_detector;
		}
	};

	static Container m_container;
};

#endif // __ENEMY_DETECTION_H__