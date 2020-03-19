/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: механизм определения врага
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#pragma warning(disable:4786)

#include "logicdefs.h"
#include "EnemyDetection.h"
#include "entity.h"
#include "xlsreader.h"
#include "spawn.h"

#include "AIContext.h"

namespace
{
    const char* alignment_xls_name = "scripts/logic/alignment.txt";
    enum alignment_column_type
	{
         ACT_SUBTEAM,
         ACT_ENEMIES,
		 ACT_FRIENDS,
		 ACT_NAMES,
            
         ACT_MAX_COLUMNS,            
    };

    column alignment_columns[ACT_MAX_COLUMNS] = 
    {
        column( "subteam", ACT_SUBTEAM),
        column( "enemies", ACT_ENEMIES),
        column( "friends", ACT_FRIENDS),
		column("name", ACT_NAMES)
    };

	class EnemyDetectionObserver : public SpawnObserver
	{
	public:
		virtual void Update(Spawner* spawner, event_t type, info_t info)
		{
			// TODO: написать код, соответствующий загрузкe уровня
			EnemyDetector::getInst()->clearCurLevelInfo();
		}
	};

};

//////////////////////////////////////////////////////////////////////////////
//
// класс для хранения зарегистрированных наблюдателей и уведомления их
//
//////////////////////////////////////////////////////////////////////////////

// зарегистрировать наблюдателя
void RelationsObserversContainer::attach(RelationsObserver* observer, RelationsObserver::event_type type)
{
	switch(type)
	{
	case RelationsObserver::ET_RELATION2PLAYER_CHANGED :
		   {
			   m_player_observers.insert(observer);
			   break;
		   }
	case RelationsObserver::ET_RELATIONS_CHANGED :
		   {
			   m_anybody_observers.insert(observer);
			   break;
		   }
	}
}

// отсоединить наблюдателя
void RelationsObserversContainer::detach(RelationsObserver* observer)
{
	m_player_observers.erase(observer);
	m_anybody_observers.erase(observer);
}

// деструктор
RelationsObserversContainer::~RelationsObserversContainer()
{
	m_player_observers.clear();
	m_anybody_observers.clear();
}

// уведомление о том, что юнит поменял отношение к игроку
void RelationsObserversContainer::notifyEntityChangedRelationWithPlayer(eid_t entity)
{
	Observers::iterator i = m_player_observers.begin();
	Observers::iterator end = m_player_observers.end();
	while(i != end)
	{
		RelationsObserver::EntityDesc ed(entity);
		(*i)->changed(RelationsObserver::ET_RELATION2PLAYER_CHANGED, &ed);
		++i;
	}
}

// уведомление о том, что подкоманда стала врагом игрока
void RelationsObserversContainer::notifySubteamChangedRelationWithPlayer(const std::string& name)
{
	EntityPool::iterator i = EntityPool::GetInst()->begin(ET_ALL_ENTS,
		PT_ENEMY | PT_CIVILIAN);
	EntityPool::iterator end = EntityPool::GetInst()->end();
	while(i != end)
	{
		if(BaseEntity* be = &*i)
		{
			// существо есть
			if(AIContext* context = be->GetAIContext())
			{
				// контекст есть - получим имя подкоманды
				std::string str = context->getSubteam();
				if(str == name)
				{
					// существо стало врагом
					notifyEntityChangedRelationWithPlayer(be->GetEID());
					be->Notify(EntityObserver::EV_RELATION_CHANGE);
				}
			}
		}
		++i;
	}
}

// уведомление о том, что подкоманда изменила отношение к другой подкоманде
void RelationsObserversContainer::notifySubteam1ChangedRelationWithSubteam2(BaseEntity* entity1, BaseEntity* entity2, RelationType type)
{
	std::string subteam1;
	std::string subteam2;

	// определим подкоманды существ

	// первое существо
	switch(entity1->GetPlayer())
	{
	case PT_PLAYER : 
		{
			subteam1 = "player";
			break;
		}
	case PT_ENEMY : case PT_CIVILIAN :
		{
			if(AIContext* context = entity1->GetAIContext())
			{
				subteam1 = context->getSubteam();
			}
			break;
		}
	default:;
	}

	// второе существо
	switch(entity2->GetPlayer())
	{
	case PT_PLAYER : 
		{
			subteam2 = "player";
			break;
		}
	case PT_ENEMY : case PT_CIVILIAN :
		{
			if(AIContext* context = entity2->GetAIContext())
			{
				subteam2 = context->getSubteam();
			}
			break;
		}
	default:;
	}
	// пошлем сообщение
	notifySubteam1ChangedRelationWithSubteam2(subteam1, subteam2, type);
}

// уведомление о том, что подкоманда изменила отношение к другой подкоманде
void RelationsObserversContainer::notifySubteam1ChangedRelationWithSubteam2(const std::string& subteam1, const std::string& subteam2, RelationType type)
{
	Observers::iterator i = m_anybody_observers.begin();
	Observers::iterator end = m_anybody_observers.end();
	while(i != end)
	{
		RelationsObserver::RelationDesc rd;
		rd.m_first_subteam = subteam1;
		rd.m_second_subteam = subteam2;
		rd.m_relation_type = type;
		(*i)->changed(RelationsObserver::ET_RELATIONS_CHANGED, &rd);
		++i;
	}
}

// очистить наблюдателей
void RelationsObserversContainer::clear()
{
	m_player_observers.clear();
	m_anybody_observers.clear();
}

///////////////////////////////////////////////////////////////////////////////
//
// класс, предоставляющий интерфейс для определения врага
//
///////////////////////////////////////////////////////////////////////////////

// здесь будет лежать статическая переменная класса
EnemyDetector::Container EnemyDetector::m_container;
// получить указатель на единственный объект класса
EnemyDetector* EnemyDetector::getInst()
{
	// если объект не создан - создадим его
	if(!m_container.m_detector)
	{
		m_container.m_detector = new EnemyDetector;
	}

	return m_container.m_detector;
}

// проинициализировать класс определения врагов
void EnemyDetector::init()
{
	m_next_subteam_number = 1;
	loadAlignment();
}


// конструктор
EnemyDetector::EnemyDetector() :
m_loadlevel_observer(new EnemyDetectionObserver)
{
	// зарегистрируемся на получение сообщений о загрузке уровня
	Spawner::Attach(m_loadlevel_observer, SpawnObserver::ET_ENTRY_LEVEL);
	// зарегистрируемся на получение сообщений о ранении существ
    GameEvMessenger::GetInst()->Attach(this, EV_HURT);
    GameEvMessenger::GetInst()->Attach(this, EV_KILL);
}

// деструктор
EnemyDetector::~EnemyDetector()
{
	// откажемся от получения сообщений о загрузке уровня

	//Punch: если Spawner уничтожается раньше м.б.вылет    
    //Punch: Spawner::Detach(m_loadlevel_observer);

	delete m_loadlevel_observer;

    // откажемся от получения сообщений о ранении существ

	//Punch: если GameEvMessenger уничтожается раньше м.б.вылет    
	//Punch: GameEvMessenger::GetInst()->Detach(this);
}

// проверить является ли второе существо для первого существа врагом
bool EnemyDetector::isHeEnemy4Me(BaseEntity * me, BaseEntity * he)
{
	// попробуем определить подкоманду (или eid цивилиана если он сам по себе) me
	int my_number = 0;
	int my_eid = 0;
	switch(me->GetPlayer())
	{
	case PT_PLAYER : 
		my_number = m_subteams["player"];
		break;
	case PT_ENEMY :
		// если у врага нет контекста, то он вообще никто!
		if(!me->GetAIContext()) return false;
		my_number = m_subteams[me->GetAIContext()->getSubteam()];
		break;
	case PT_CIVILIAN :
		if(!me->GetAIContext())
		{
			// у мирного жителя нет контекста, нет подкоманды
			// если это человек или торговец запомним его номер
			if((me->Cast2Human()) ||
				(me->Cast2Trader())
				) my_eid = static_cast<int>(me->GetEID());
			else return false;
			break;
		}
		my_number = m_subteams[me->GetAIContext()->getSubteam()];
		break;
	default:;
	}

	// попробуем определить подкоманду (или eid цивилиана если он сам по себе) he
	int his_number = 0;
	int his_eid = 0;
	switch(he->GetPlayer())
	{
	case PT_PLAYER : 
		his_number = m_subteams["player"];
		break;
	case PT_ENEMY : 
		// если у врага нет контекста, то он вообще никто!
		if(!he->GetAIContext()) return false;
		his_number = m_subteams[he->GetAIContext()->getSubteam()];
		break;
	case PT_CIVILIAN :
		if(!he->GetAIContext())
		{
			// у мирного жителя нет контекста, нет подкоманды
			if((he->Cast2Human()) ||
				(he->Cast2Trader())
				) his_eid = static_cast<int>(he->GetEID());
			else return false;
			break;
		}
		his_number = m_subteams[he->GetAIContext()->getSubteam()];
		break;
	default:;
	}

	if(my_number && his_number)
	{
		// и me, и he принадлежат определенным подкомандам
		// проверим считает ли me врагом подкоманду he

		// если множества врагов me пустое, это не враг
		if(m_relations[my_number].empty()) return false;
		// проверим есть ли такой номер подкоманды he среди врагов me
		subteam_set_t::iterator it = m_relations[my_number].find(his_number);
		if(it == m_relations[my_number].end()) return false;
		// значит есть, - он мой враг
		return true;
	}
	if(my_number && his_eid)
	{
		// у me есть подкоманда, а he является мирным жителем не принадлежащим
		// ни одной из подкоманд

		// проверим есть ли у моей подкоманды враги среди мирных жителей
		if(m_subteam_vs_civilians[my_number].empty()) return false;

		// есть, проверим есть ли среди них с номером he_eid
		subteam_set_t::iterator it = m_subteam_vs_civilians[my_number].find(his_eid);
		if(it == m_subteam_vs_civilians[my_number].end()) return false;
		// значит есть, - он мой враг
		return true;
	}

	if(my_eid && his_number)
	{
		// я - мирный житель, не принадлежащий ни одной подкоманде,
		// а у he есть подкоманда

		// проверим есть ли у меня враги среди подкоманд
		if(m_civilian_vs_subteams[my_eid].empty()) return false;

		// есть, проверим есть ли среди них с номером his_number
		subteam_set_t::iterator it = m_civilian_vs_subteams[my_eid].find(his_number);
		if(it == m_civilian_vs_subteams[my_eid].end()) return false;
		// значит есть, - он мой враг
		return true;
	}

	if(my_eid && his_eid)
	{
		// и me, и he являются мирными жителями, не принадлежащими ни одной из подкоманд

		// проверим есть ли у меня враги среди мирных жителей
		if(m_civilian_vs_civilians[my_eid].empty()) return false;

		// есть, проверим есть ли среди них с номером his_eid
		subteam_set_t::iterator it = m_civilian_vs_civilians[my_eid].find(his_eid);
		if(it == m_civilian_vs_civilians[my_eid].end()) return false;
		// значит есть, - он мой враг
		return true;
	}
	return false;
}

// вернуть тип отношений между игроком и существом
RelationType EnemyDetector::getRelationBetweenPlayerAndHe(BaseEntity * he)
{
	RelationType rt = RT_NEUTRAL;
	int player = m_subteams["player"];
	// попробуем определить подкоманду (или eid цивилиана если он сам по себе) he
	int his_number = 0;
	int his_eid = 0;
	switch(he->GetPlayer())
	{
	case PT_PLAYER : 
		{
			// из своей команды
			rt = RT_FRIEND;
			return rt; 
		}
	case PT_ENEMY : 
		// если у врага нет контекста, то он вообще никто!
		if(!he->GetAIContext()) 
		{
			return rt;
		}
		his_number = m_subteams[he->GetAIContext()->getSubteam()];
		break;
	case PT_CIVILIAN :
		if(!he->GetAIContext())
		{
			// у мирного жителя нет контекста, нет подкоманды
			if((he->Cast2Human()) ||
				(he->Cast2Trader())
				) 
			{
				his_eid = static_cast<int>(he->GetEID());
			}
			else 
			{
				return rt;
			}
			break;
		}
		his_number = m_subteams[he->GetAIContext()->getSubteam()];
		break;
	default:;
	}

	if(his_number)
	{
		// у него есть подкоманда - проверим поочереди отношения

		// на врага
		if(!m_relations[player].empty())
		{
			// проверим есть ли такой номер подкоманды he среди врагов player
			subteam_set_t::iterator it = m_relations[player].find(his_number);
			if(it != m_relations[player].end())
			{
				// значит есть, - он враг player'a
				rt = RT_ENEMY;
				return rt;
			}
		}

		// на друга
		if(!m_friends[player].empty())
		{
			// проверим есть ли такой номер подкоманды he среди друзей player
			subteam_set_t::iterator it = m_friends[player].find(his_number);
			if(it != m_friends[player].end())
			{
				// значит есть, - он друг player'a
				rt = RT_FRIEND;
				return rt;
			}
		}

		// значит нейтральный
		return rt;
	}

	if(his_eid)
	{
		// he является мирным жителем не принадлежащим ни одной из подкоманд

		// проверим есть ли у подкоманды player враги среди мирных жителей
		if(!m_subteam_vs_civilians[player].empty())
		{
			// есть, проверим есть ли среди них с номером he_eid
			subteam_set_t::iterator it = m_subteam_vs_civilians[player].find(his_eid);
			if(it != m_subteam_vs_civilians[player].end())
			{
				// значит есть, - он враг player'a
				rt = RT_ENEMY;
				return rt;
			}
		}
		// нейтральный
		return rt;
	}

	// по идее сюда нельзя попасть
	return rt;
}

// загрузить первоначальные сведения о расстановке сил из xls'ки
void EnemyDetector::loadAlignment()
{
	TxtFilePtr txt(alignment_xls_name);
	
	txt.ReadColumns(alignment_columns, alignment_columns + ACT_MAX_COLUMNS);

	for(int i = 1; i < txt->SizeY(); ++i)
	{
		// прочтем название подкоманды
		std::string subteam_name;
		txt->GetCell(i, alignment_columns[ACT_SUBTEAM].m_index, subteam_name);
		// проверим, есть ли такая подкоманда в карте номеров подкоманд
		if(m_subteams.find(subteam_name) == m_subteams.end())
		{
			// такой подкоманды еще не встречалось - добавим ее в карту номеров
			m_subteams[subteam_name] = m_next_subteam_number++;
		}
		
		// прочтем обычное имя подкоманды
		std::string usual_name;
		txt->GetCell(i, alignment_columns[ACT_NAMES].m_index, usual_name);
		// добавим его в карту конвертации
		m_convert_map[subteam_name] = usual_name;

		// прочтем названия подкоманд - врагов текущей
		std::string enemies;  // названия подкоманд через запятую и пробел
		txt->GetCell(i, alignment_columns[ACT_ENEMIES].m_index, enemies);
		std::istringstream stream(enemies);
		std::string word;
		subteam_set_t enemy_set;
		// пройдемся по всем названием подкоманд-врагов
		while((stream >> word) && (word != "none"))
		{
			// word содержит имя подкоманды-врага - проверим есть ли такая в
			// нашей карте номеров подкоманд
			if(m_subteams.find(word) == m_subteams.end())
			{
				// такой подкоманды еще не встречалось - добавим ее в карту номеров
				m_subteams[word] = m_next_subteam_number++;
			}

			// добавим номер подкоманды-врага в список
			enemy_set.insert(m_subteams[word]);
		}
		// добавим в карту отношений между подкомандами созданный список
		// ВНИМАНИЕ: если там уже есть подкоманда с таким же именем, то
		// информация будет перезаписана
		m_relations[m_subteams[subteam_name]] = enemy_set;

		// прочтем названия подкоманд - друзей текущей
		txt->GetCell(i, alignment_columns[ACT_FRIENDS].m_index, enemies);
		std::istringstream stream2(enemies);
		enemy_set.clear();
		// пройдемся по всем названием подкоманд-друзей
		while((stream2 >> word) && (word != "none"))
		{
			// word содержит имя подкоманды-друга - проверим есть ли такая в
			// нашей карте номеров подкоманд
			if(m_subteams.find(word) == m_subteams.end())
			{
				// такой подкоманды еще не встречалось - добавим ее в карту номеров
				m_subteams[word] = m_next_subteam_number++;
			}

			// добавим номер подкоманды-друга во множество
			enemy_set.insert(m_subteams[word]);
		}
		// добавим в карту друзей созданный список
		// ВНИМАНИЕ: если там уже есть подкоманда с таким же именем, то
		// информация будет перезаписана
		m_friends[m_subteams[subteam_name]] = enemy_set;
	}
}

// обработка сообщения о ранении существа на уровне
void EnemyDetector::Update(subject_t subj, event_t event, info_t ptr)
{
    hurt_info* info = static_cast<hurt_info*>(ptr);

    if(    !info->m_killer || !info->m_victim || info->m_killer == info->m_victim)
        return;

    // попробуем определить подкоманду (или eid цивилиана если он сам по себе) убийцы 
	int killer_number = 0;
	int killer_eid = 0;
	switch(info->m_killer->GetPlayer())
	{
	case PT_PLAYER : 
			killer_number = m_subteams["player"];
			break;
	case PT_ENEMY : 
		{
			if(!info->m_killer->GetAIContext())	// у врага нет контекста!
				return;
			// контекст есть, проверим есть ли у него подкоманда
			std::string str = info->m_killer->GetAIContext()->getSubteam();
			if( str=="" || str=="none" )// подкоманды нет - ничего не делаем
				break;
			// есть подкоманда
			killer_number = m_subteams[str];
			break;
		}
	case PT_CIVILIAN :
		{
			if(!info->m_killer->GetAIContext())
			{
				// у мирного жителя нет контекста - 
				// если это человек или торговец - запомним его номер
				if((info->m_killer->Cast2Human()) ||
					(info->m_killer->Cast2Trader())
					) killer_eid = static_cast<int>(info->m_killer->GetEID());
				else return;
				break;
			}
			// контекст есть, проверим есть ли у него подкоманда
			std::string str = info->m_killer->GetAIContext()->getSubteam();
			if( str=="" || str=="none" )// подкоманды нет - ничего не делаем
				break;
			// есть подкоманда
			killer_number = m_subteams[str];
			break;
		}
	default: return;
	}
	
	// попробуем определить подкоманду (или eid цивилиана если он сам по себе) жертвы
	int victim_number = 0;
	int victim_eid = 0;
	switch(info->m_victim->GetPlayer())
	{
	case PT_PLAYER : 
			// так как жертва - игрок, то никакие отношения не меняются
			return;
	case PT_ENEMY : 
		{
			if(!info->m_victim->GetAIContext())	// у врага нет контекста!
				return;

			// контекст есть, проверим есть ли у него подкоманда
			std::string str = info->m_victim->GetAIContext()->getSubteam();
			if( str=="" || str=="none" )// подкоманды нет - ничего не делаем
				break;
			// есть подкоманда
			victim_number = m_subteams[str];
			break;
		}
	case PT_CIVILIAN :
		{
			if(!info->m_victim->GetAIContext())	// у мирного жителя нет контекста -
			{
				// если это человек или торговец - запомним его номер
				if((info->m_victim->Cast2Human()) || info->m_victim->Cast2Trader())
					victim_eid = static_cast<int>(info->m_victim->GetEID());
				else return;
				break;
			}
			// контекст есть, проверим есть ли у него подкоманда
			std::string str = info->m_victim->GetAIContext()->getSubteam();
			if( str=="" || str=="none" )// подкоманды нет - ничего не делаем
				break;
			// есть подкоманда
			victim_number = m_subteams[str];
			break;
		}
	default: return;
	}

	if(killer_number && victim_number)
	{
		if(killer_number == victim_number) return; // попал кто-то из своих
		// и убийца, и жертва принадлежат определенным подкомандам

		// если подкоманда жертвы очень дружит с подкомандой врага, то она
		// их простит
		if(event!=EV_KILL || info->m_victim->GetPlayer()!=PT_ENEMY || info->m_killer->GetPlayer()!=PT_PLAYER)
		{
			if(m_friends[victim_number].find(killer_number) != m_friends[victim_number].end())
			{
				return;
			}
		}

		// сделаем, чтобы подкоманда жертвы теперь считала подкоманду
		// убийцы своим врагом
		std::pair<subteam_set_t::iterator, bool> res = m_relations[victim_number].insert(killer_number);
		if(res.second) // произошло ли реальное добавление
		{
			// пошлем уведомление об изменении отношений между подкомандами
			m_observers_container.notifySubteam1ChangedRelationWithSubteam2(
				info->m_victim,
				info->m_killer,
				RT_ENEMY);
			// посмотрим, нужно ли посылать уведомление связанное с игроком
			if(info->m_victim->GetPlayer() == PT_PLAYER)
			{
				// пошлем уведомление
				info->m_killer->Notify(EntityObserver::EV_RELATION_CHANGE);
				m_observers_container.notifySubteamChangedRelationWithPlayer(info->m_killer->GetAIContext()->getSubteam());
			}
		}

		// если убийца - игрок - сделаем чтобы подкоманда жертвы стала его врагом
		if(info->m_killer->GetPlayer() == PT_PLAYER)
		{
			std::pair<subteam_set_t::iterator, bool> res = m_relations[killer_number].insert(victim_number);
			if(res.second) // добавление реально произошло
			{
				// пошлем уведомление
				m_observers_container.notifySubteam1ChangedRelationWithSubteam2(
					info->m_killer,
					info->m_victim,
					RT_ENEMY);
				// и еще парочку
				info->m_victim->Notify(EntityObserver::EV_RELATION_CHANGE);
				m_observers_container.notifySubteamChangedRelationWithPlayer(info->m_victim->GetAIContext()->getSubteam());
			}
		}
		return;
	}

	if(killer_number && victim_eid)
	{
		// у убийцы есть подкоманда, а жертва является мирным жителем не принадлежащим
		// ни одной из подкоманд - сделаем подкоманду врагом мирного жителя
		m_civilian_vs_subteams[victim_eid].insert(killer_number);
		// если убийца - игрок, то сделаем мирного жителя врагом команды
		if(info->m_killer->GetPlayer() == PT_PLAYER)
		{
			std::pair<subteam_set_t::iterator, bool> res = m_subteam_vs_civilians[killer_number].insert(victim_eid);
			if(res.second) // добавление произошло
			{
				// пошлем уведомление
				info->m_victim->Notify(EntityObserver::EV_RELATION_CHANGE);
				m_observers_container.notifyEntityChangedRelationWithPlayer(victim_eid);
			}
		}
		return;
	}

	if(killer_eid && victim_number)
	{
		// убийца - мирный житель, не принадлежащий ни одной подкоманде,
		// а у жертвы есть подкоманда. сделаем мирного жителя врагом подкоманды
		std::pair<subteam_set_t::iterator, bool> res = m_subteam_vs_civilians[victim_number].insert(killer_eid);
		// посмотрим, нужно ли посылать уведомление
		if(info->m_victim->GetPlayer() == PT_PLAYER)
		{
			if(res.second) // добавление произошло
			{
				// пошлем уведомление
				info->m_killer->Notify(EntityObserver::EV_RELATION_CHANGE);
				m_observers_container.notifyEntityChangedRelationWithPlayer(killer_eid);
			}
		}
		return;
	}

	if(killer_eid && victim_eid)
	{
		// и убийца, и жертва являются мирными жителями, не принадлежащими ни одной из подкоманд
		// сделаем убийцу личными врагом жертвы
		m_civilian_vs_civilians[victim_eid].insert(killer_eid);
		return;
	}

	// пока в отладке:
	throw CASUS("EnemyDetector::OnHurtEv: непредвиденный случай нанесения повреждений");
}

//сохранение/востановление
void EnemyDetector::MakeSaveLoad(SavSlot& st)
{
    //сохраним/восстановим структуру подкоманды
	if(st.IsSaving())
	{
		// сохраняемся

		// сохраним карту номеров подкоманд
		st << m_subteams.size();
		{
			subteam_map_t::iterator i = m_subteams.begin();
			subteam_map_t::iterator end = m_subteams.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second;
				++i;
			}
		}

		// сохраним карту отношений между подкомандами
		st << m_relations.size();
		{
			relation_map_t::iterator i = m_relations.begin();
			relation_map_t::iterator end = m_relations.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second.size();
				subteam_set_t::iterator j = (*i).second.begin();
				subteam_set_t::iterator enD = (*i).second.end();
				while(j != enD)
				{
					st << (*j);
					j++;
				}
				i++;
			}
		}

		// сохраним карту друзей подкоманд
		st << m_friends.size();
		{
			relation_map_t::iterator i = m_friends.begin();
			relation_map_t::iterator end = m_friends.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second.size();
				subteam_set_t::iterator j = (*i).second.begin();
				subteam_set_t::iterator enD = (*i).second.end();
				while(j != enD)
				{
					st << (*j);
					j++;
				}
				i++;
			}
		}

		// сохраним номер для следующей подкоманды
		st << m_next_subteam_number;

		// сохраним карту: ключ - номер подкоманды, значение - список номеров мирных жителей (врагов)
		st << m_subteam_vs_civilians.size();
		{
			relation_map_t::iterator i = m_subteam_vs_civilians.begin();
			relation_map_t::iterator end = m_subteam_vs_civilians.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second.size();
				subteam_set_t::iterator j = (*i).second.begin();
				subteam_set_t::iterator enD = (*i).second.end();
				while(j != enD)
				{
					st << (*j);
					j++;
				}
				i++;
			}
		}

		// сохраним карту: ключ - номер мирного жителя, значение - список номеров подкоманд (врагов)
		st << m_civilian_vs_subteams.size();
		{
			relation_map_t::iterator i = m_civilian_vs_subteams.begin();
			relation_map_t::iterator end = m_civilian_vs_subteams.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second.size();
				subteam_set_t::iterator j = (*i).second.begin();
				subteam_set_t::iterator enD = (*i).second.end();
				while(j != enD)
				{
					st << (*j);
					j++;
				}
				i++;
			}
		}

		// сохраним карту: ключ - номер мирного жителя, значение - список номеров мирных жителей (врагов)
		st << m_civilian_vs_civilians.size();
		{
			relation_map_t::iterator i = m_civilian_vs_civilians.begin();
			relation_map_t::iterator end = m_civilian_vs_civilians.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second.size();
				subteam_set_t::iterator j = (*i).second.begin();
				subteam_set_t::iterator enD = (*i).second.end();
				while(j != enD)
				{
					st << (*j);
					j++;
				}
				i++;
			}
		}

		// сохраним карту конвертации:
		st << m_convert_map.size();
		{
			convert_map_t::iterator i = m_convert_map.begin();
			convert_map_t::iterator end = m_convert_map.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second;
				i++;
			}
		}

		return;
	}
	// читаемся

	// прочитаем карту номеров подкоманд
	int size;
	st >> size;
	m_subteams.clear();
	for(int i = 0; i < size; i++)
	{
		std::string str;
		int num;
		st >> str;
		st >> num;
		m_subteams[str] = num;
	}

	// прочитаем карту отношений между подкомандами
	st >> size;
	m_relations.clear();
	for(int i = 0; i < size; i++)
	{
		int subteam_num;
		st >> subteam_num;
		int set_size;
		st >> set_size;
		subteam_set_t set;
		for(int j = 0; j < set_size; j++)
		{
			int s_num;
			st >> s_num;
			set.insert(s_num);
		}
		m_relations[subteam_num] = set;
	}

	// прочитаем карту друзей подкоманд
	st >> size;
	m_friends.clear();
	for(int i = 0; i < size; i++)
	{
		int subteam_num;
		st >> subteam_num;
		int set_size;
		st >> set_size;
		subteam_set_t set;
		for(int j = 0; j < set_size; j++)
		{
			int s_num;
			st >> s_num;
			set.insert(s_num);
		}
		m_friends[subteam_num] = set;
	}

	// прочитаем номер для следующей подкоманды
	st >> m_next_subteam_number;

	// прочитаем карту: ключ - номер подкоманды, значение - список номеров мирных жителей (врагов)
	st >> size;
	m_subteam_vs_civilians.clear();
	for(int i = 0; i < size; i++)
	{
		int subteam_num;
		st >> subteam_num;
		int set_size;
		st >> set_size;
		subteam_set_t set;
		for(int j = 0; j < set_size; j++)
		{
			int s_num;
			st >> s_num;
			set.insert(s_num);
		}
		m_subteam_vs_civilians[subteam_num] = set;
	}

	// прочитаем карту: ключ - номер мирного жителя, значение - список номеров подкоманд (врагов)
	st >> size;
	m_civilian_vs_subteams.clear();
	for(int i = 0; i < size; i++)
	{
		int subteam_num;
		st >> subteam_num;
		int set_size;
		st >> set_size;
		subteam_set_t set;
		for(int j = 0; j < set_size; j++)
		{
			int s_num;
			st >> s_num;
			set.insert(s_num);
		}
		m_civilian_vs_subteams[subteam_num] = set;
	}

	// прочитаем карту: ключ - номер мирного жителя, значение - список номеров мирных жителей (врагов)
	st >> size;
	m_civilian_vs_civilians.clear();
	for(int i = 0; i < size; i++)
	{
		int subteam_num;
		st >> subteam_num;
		int set_size;
		st >> set_size;
		subteam_set_t set;
		for(int j = 0; j < set_size; j++)
		{
			int s_num;
			st >> s_num;
			set.insert(s_num);
		}
		m_civilian_vs_civilians[subteam_num] = set;
	}
	
	// прочитаем карту конвертации:
	st >> size;
	m_convert_map.clear();
	for(int i = 0; i < size; i++)
	{
		std::string key;
		std::string data;
		st >> key;
		st >> data;
		m_convert_map[key] = data;
	}
}

// очистить информацию, хранимую только для текущего уровня
void EnemyDetector::clearCurLevelInfo()
{
	m_subteam_vs_civilians.clear();
	m_civilian_vs_subteams.clear();
	m_civilian_vs_civilians.clear();
}

// зарегистрировать наблюдателя
void EnemyDetector::attach(RelationsObserver* observer, RelationsObserver::event_type type)
{
	m_observers_container.attach(observer, type);
}

// отсоединить наблюдателя
void EnemyDetector::detach(RelationsObserver* observer)
{
	m_observers_container.detach(observer);
}

// принудительно изменить отношения между двумя подкомандами.
// после вызова подкоманда 1 будет относиться к подкоманде 2 как к new_relation
void EnemyDetector::setRelation(const std::string& subteam1,
								const std::string& subteam2,
								RelationType new_relation)
{
	// узнаем текущее отношение подкоманды 1 к подкоманде 2
	RelationType old_relation = RT_NEUTRAL; // по умолчанию - нейтральное
	int snum1 = m_subteams[subteam1];
	int snum2 = m_subteams[subteam2];
	if( (!snum1) || (!snum2) ) return;
	
	// на врага
	if(!m_relations[snum1].empty())
	{
		// проверим есть ли такой номер подкоманды snum2 среди врагов snum1
		subteam_set_t::iterator it = m_relations[snum1].find(snum2);
		if(it != m_relations[snum1].end())
		{
			// значит есть, - он враг 
			old_relation = RT_ENEMY;
		}
	}
	
	// на друга
	if(!m_friends[snum1].empty())
	{
		// проверим есть ли такой номер подкоманды he среди друзей player
		subteam_set_t::iterator it = m_friends[snum1].find(snum2);
		if(it != m_friends[snum1].end())
		{
			// значит есть, - он друг player'a
			old_relation = RT_FRIEND;
		}
	}

	if(old_relation == new_relation)
	{
		// ничего менять не надо
		return;
	}

	// отношение изменилось
	// удалим подкоманду 2 из множества врагов и друзей подкоманды 1
	clearRelation(subteam1, subteam2);
	switch(new_relation)
	{
	case RT_ENEMY :
		{
			// добавим подкоманду 2 в множество врагов подкоманды 1
			setEnemyRelation(subteam1, subteam2);
			break;
		}
	case RT_FRIEND :
		{
			// добавим подкоманду 2 в множество друзей подкоманды 1
			setFriendRelation(subteam1, subteam2);
			break;
		}
	}

	// пошлем уведомление
	m_observers_container.notifySubteam1ChangedRelationWithSubteam2(subteam1,
		subteam2,
		new_relation);
	// и, если нужно, еще одно
	if(subteam1 == "player")
	{
		// пошлем уведомление для изменения интерфейса
		m_observers_container.notifySubteamChangedRelationWithPlayer(subteam2);
	}
}

// удалить подкоманду 2 из множества врагов и друзей подкоманды 1
void EnemyDetector::clearRelation(const std::string& subteam1, const std::string& subteam2)
{
	// если любой из подкоманд нет в карте номеров - ничего делать не надо
	if(m_subteams.find(subteam1) == m_subteams.end()) return;
	if(m_subteams.find(subteam2) == m_subteams.end()) return;

	// проверим, есть ли среди врагов подкоманды 1 подкоманда 2
	if(m_relations[m_subteams[subteam1]].find(m_subteams[subteam2]) != m_relations[m_subteams[subteam1]].end())
	{
		// есть, удалим запись
		m_relations[m_subteams[subteam1]].erase(m_subteams[subteam2]);
	}

	// проверим, есть ли среди друзей подкоманды 1 подкоманда 2
	if(m_friends[m_subteams[subteam1]].find(m_subteams[subteam2]) != m_friends[m_subteams[subteam1]].end())
	{
		// есть, удалим запись
		m_friends[m_subteams[subteam1]].erase(m_subteams[subteam2]);
	}
}

// сделать подкоманду 2 врагом подкоманды 1
void EnemyDetector::setEnemyRelation(const std::string& subteam1, const std::string& subteam2)
{
	// если любой из подкоманд нет в карте номеров - ничего делать не надо
	if(m_subteams.find(subteam1) == m_subteams.end()) return;
	if(m_subteams.find(subteam2) == m_subteams.end()) return;

	// проверим, есть ли среди врагов подкоманды 1 подкоманда 2
	if(m_relations[m_subteams[subteam1]].find(m_subteams[subteam2]) == m_relations[m_subteams[subteam1]].end())
	{
		// нет, добавим запись
		m_relations[m_subteams[subteam1]].insert(m_subteams[subteam2]);
	}
}

// сделать подкоманду 2 другом подкоманды 1
void EnemyDetector::setFriendRelation(const std::string& subteam1, const std::string& subteam2)
{
	// если любой из подкоманд нет в карте номеров - ничего делать не надо
	if(m_subteams.find(subteam1) == m_subteams.end()) return;
	if(m_subteams.find(subteam2) == m_subteams.end()) return;

	// проверим, есть ли среди друзей подкоманды 1 подкоманда 2
	if(m_friends[m_subteams[subteam1]].find(m_subteams[subteam2]) == m_friends[m_subteams[subteam1]].end())
	{
		// нет, добавим запись
		m_friends[m_subteams[subteam1]].insert(m_subteams[subteam2]);
	}
}

// получить имя подкоманды по ее системному имени
std::string EnemyDetector::getName(const std::string& sys_name)
{
	return m_convert_map[sys_name];
}
