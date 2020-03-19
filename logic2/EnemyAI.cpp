/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: интеллект врагов
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#pragma warning(disable:4786)
// precompiled header
#include "logicdefs.h"

#include "EnemyAI.h"

#include "AIAPI.h"
#include "AIUtils.h"
#include "PathUtils.h"

// пространство имен без имени - для функций-утилит этого файла
namespace
{
	// точность сравнения углов поворота
	const float angle_eps = TORAD(6);
	// расстояние, на котором юнит реагирует на взрывы и выстрелы
	const float alarm_dist = 5.0f;
	// расстояние, на которое посылается сигнал своей команде
	const float signal_dist = 100.0f;
	const float fixed_enemy_dist = 34.0f;
	const float fixed_cure_dist = 0.0f;
	const int fixed_turns = 3;
	const float patrol_cure_dist = 0.0f;
	const float patrol_enemy_dist = 22.0f;
	const unsigned int patrol_enemy_noLOF_move = 6;
	const float assault_cure_dist = 0.0f;
	const float assault_enemy_dist = 22.0f;
	const int assault_turns = 2;
	const float fixed_tech_enemy_dist = 22.0f;
	const float patrol_tech_enemy_dist = 22.0f;
	const float assault_tech_enemy_dist = 22.0f;
	// нормирование угла от нуля до двух пи
	float norm_angle(float angle);
	// получить в виде списка разность (из теории множеств) между двумя двумя
	// другими списками eid_t
	void setDifferenceBetweenLists(const std::list<eid_t>& lista, const std::list<eid_t>& listb, std::list<eid_t>* outlist);
};

/////////////////////////////////////////////////////////////////////////////
//
// класс - узел интеллекта всех врагов
//
/////////////////////////////////////////////////////////////////////////////
AIEnemyNode::AIEnemyNode(): m_subteam(0)
{
}

/////////////////////////////////////////////////////////////////////////////
//
// узел для управления подкомандой
//
/////////////////////////////////////////////////////////////////////////////
DCTOR_IMP(SubteamNode)

SubteamNode::SubteamNode()
{
    m_cur = m_nodes.end();
    
	GameEvMessenger::GetInst()->Attach(this, EV_TURN);
	m_complexity = 0.0f;
}

SubteamNode::~SubteamNode()
{
	GameEvMessenger::GetInst()->Detach(this);

    node_lst_t::iterator itor = m_nodes.begin();
    while(itor != m_nodes.end())
	{
        delete *itor;
        ++itor;
    }
}

//в этой функции узел должен подумать
float SubteamNode::Think(state_type* st)
{
    node_lst_t::iterator itor = m_nodes.begin();
    while(itor != m_nodes.end())
	{
		// удаляем мертвые узлы
		if((*itor)->need2Delete())
		{
			// удалим этот узел
			if(itor == m_cur) m_cur++;
			node_lst_t::iterator tmp = itor;
			itor++;
			delete (*tmp);
			m_nodes.erase(tmp);
			continue;
		}
        if(itor != m_cur) (*itor)->Think(0);
        ++itor;
    }

    if(st == 0) return m_complexity;

    //если обошли все узлы выход
    if(m_cur == m_nodes.end())
	{     
        *st = ST_FINISHED;
        return m_complexity;
    }

    //дадим подумать текушему узлу
    float tmp_complexity = (*m_cur)->Think(st);

    //если узел закончил перейдем к следующему
    if(*st == ST_FINISHED)
	{
		m_complexity += (*m_cur)->getComplexity();
		tmp_complexity = 0.0f;
        ++m_cur;
	}
    
    *st = ST_INCOMPLETE;
	return m_complexity + tmp_complexity;
}
  
//сохранение/востановление подкоманды
void SubteamNode::MakeSaveLoad(SavSlot& st)
{
    //сохраним/восстановим структуру подкоманды
	if(st.IsSaving())
	{
		// сохраняемся
		st << m_name;
		st << m_nodes.size();
		node_lst_t::iterator it = m_nodes.begin();
		node_lst_t::iterator end = m_nodes.end();
		while(it != end)
		{
			DynUtils::SaveObj(st, *it);
			(*it)->MakeSaveLoad(st);
			++it;
		}
		return;
	}
	// читаемся
	st >> m_name;
	AIEnemyNode* node;
	int num;
	st >> num;
	for(int i = 0; i < num; i++)
	{
		DynUtils::LoadObj(st, node);
		node->MakeSaveLoad(st);
		node->setSubteam(this);
		m_nodes.push_back(node);
	}
}

//обработка начала хода
void SubteamNode::Update(subject_t subj, event_t event, info_t info)
{
    if(static_cast<turn_info*>(info)->m_player == PT_ENEMY)
        m_cur = m_nodes.begin();
	m_complexity = 0.0f;
}

// разослать сообщение всем членам команды (кроме себя)
void SubteamNode::sendMessage(const AIEnemyNode * sender, AIEnemyNode::MessageType type, void * data, float radius/*=0*/)
{
	node_lst_t::iterator i = m_nodes.begin();
	node_lst_t::iterator end = m_nodes.end();
	while(i != end)
	{
		if((*i) == sender) { ++i; continue; }
		if(radius)
		{
			float dist = ((*i)->getPosition() - sender->getPosition()).Length();
			if(radius < dist) {++i; continue;}
		}
		(*i)->recieveMessage(type, data);
		++i;
	}
}

// расчет полной сложности узла
float SubteamNode::getComplexity() const
{
	float complexity = 0.0f;
	// заставим своих детей рассчитать сложность
	node_lst_t::const_iterator i = m_nodes.begin();
	node_lst_t::const_iterator end = m_nodes.end();
	while(i != end)
	{
		complexity += (*i)->getComplexity();
		++i;
	}
	return complexity;
}

// пошлем уведомление об убийстве детей с заданным ID и заставить их
// сделать тоже самое со своими детьми
void SubteamNode::deleteChildren(eid_t ID)
{
	if(!ID) return;
	node_lst_t::iterator i = m_nodes.begin();
	node_lst_t::iterator end = m_nodes.end();
	while(i != end)
	{
		AIEnemyNode* node = (*i);
		if(node->getEntityEID() == ID)
		{
			// умри!
			if(node->die())
			{
				if(m_cur == i) m_cur++;
				node_lst_t::iterator j = i;
				i++;
				m_nodes.erase(j);
				delete node;
				continue;
			}
		}
		i++;
	}
	// заставим детей сделать тоже самое
	i = m_nodes.begin();
	while(i != end)
	{
		AIEnemyNode* node = (*i);
		node->deleteChildren(ID);
		i++;
	}
}

point3 CommonEnemyNode::getPosition() const
	{
	return AIAPI::getInst()->getPos3(m_entity);
	}



CommonEnemyNode::CommonEnemyNode(eid_t id)
:m_turn_state(TS_NONE), m_lookround_state(LS_FIRST),
m_entity(id), m_activity(0), m_initialising(false)
{
}

bool CommonEnemyNode::ThinkShell(state_type* st)
{
    if(m_activity)//проиграть действие
	{
        if(st) *st = ST_INCOMPLETE;
        if(!m_activity->Run(AC_TICK))
		{
			m_activity->Detach(&m_activity_observer);
            delete m_activity;
            m_activity = 0;
        }
        return true;
    }
	if(!st)// если не наш ход - на выход
	{
		m_turn_state = TS_NONE;
		return true;
	}
	if(m_turn_state == TS_NONE)	// если m_turn_state == TS_NONE, то это начало нашего хода
	{
		m_turn_state = TS_START;
		// начнем инициализацию
		PanicPlayer::GetInst()->Init(AIAPI::getInst()->getPtr(m_entity));
		m_initialising = true;
	}
	
	if(m_initialising)// проверим флаг инициализации
	{
		if(PanicPlayer::GetInst()->Execute())
		{	// нужно продолжать процесс инициализации
			*st = ST_INCOMPLETE;
			return true;
		}
		// инициализация закончена
		m_initialising = false;
	}
	return false;
}
void CommonEnemyNode::CheckFinished(state_type *st,float *comp)
{
	if(m_turn_state == TS_END)
	{
		*st = ST_FINISHED;
		m_turn_state = TS_START;
		*comp = 1.0f;
	}
	else *st = ST_INCOMPLETE;
}
// проверить, нужна ли мне аптечка и вижу ли я ее
bool CommonEnemyNode::needAndSeeMedkit(ipnt2_t* target)
{
	// проверим количество аптечек у юнита
	if(AIAPI::getInst()->getThingCount(m_entity, TT_MEDIKIT) >= 2)
		return false;
	// посмотрим есть ли на виду у юнита аптечки
	if(AIAPI::getInst()->getThingLocation(m_entity, target, TT_MEDIKIT))
		return true;
	return false;
}

bool CommonEnemyNode::SelfCure()
{
	if(AIAPI::getInst()->getHealthPercent(m_entity) < 100.0f)
	{
		AIAPI::getInst()->print(m_entity, " попробую самолечение!");
		if(AIAPI::getInst()->haveMedikit(m_entity))
		{
			// у меня есть аптечка
			// возьму ее в руки
			AIAPI::getInst()->takeMedikit(m_entity);
			// полечу себя
			AIAPI::getInst()->cure(m_entity, m_entity);
			return true;
		}
	}
	return false;
}
bool CommonEnemyNode::PickUpWeaponAndAmmo(const ipnt2_t &m_target_pnt, WeaponComparator &comparator)
{
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// мы подошли к валяющемуся оружию - возьмем что сможем
		AIAPI::getInst()->print(m_entity, "мы подошли к валяющемуся оружию - возьмем что сможем");
		// возьмем все патроны
		AIAPI::getInst()->pickupAllNearAmmo(m_entity);
		// проверим есть ли теперь у нас заряженное оружие
		if( (!AIAPI::getInst()->takeGrenade(m_entity)) &&
			(!AIAPI::getInst()->takeBestWeapon(m_entity, comparator))	)
		{
			// заряженной пушки все еще нет будем поднимать пушки по одной,
			// пока они не кончатся или не найдем подходящую
			while(AIAPI::getInst()->pickupNearWeapon(m_entity))
			{
				if((AIAPI::getInst()->takeBestWeapon(m_entity, comparator)) ||
					(AIAPI::getInst()->takeGrenade(m_entity)))
					break;
			}
			// выкинем все лишнее
			AIAPI::getInst()->dropUselessWeapon(m_entity, comparator);
		}
		// переходим в состояние патрулирования
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	return false;
}
void CommonEnemyNode::SendCureMessage(float dist)
{
	if(AIAPI::getInst()->getHealthPercent(m_entity) < 50.0f)
	{
		AIAPI::getInst()->print(m_entity, "меня надо лечить!");
		getSubteam()->sendMessage(this,	AIEnemyNode::MT_NEEDCURE,&m_entity,	dist);
	}
}
bool CommonEnemyNode::MedkitTake(const ipnt2_t &m_target_pnt, state_type* st)
{
	// 1. проверим не подошли ли мы к аптечке
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// мы подошли к лежащей аптечке - возьмем, если сможем
		AIAPI::getInst()->print(m_entity, "thinkMedkitTake: мы подошли к аптечке - возьмем, если сможем");
		// возьмем все аптечки
		AIAPI::getInst()->pickupAllNearMedikit(m_entity);
		// переходим в состояние возврата в базовую точку
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 2. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "thinkMedkitTake: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return true;
	}
	
	// продолжим идти к аптечкам
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
	if(path.empty())
	{
		// к аптечкам нельзя подойти - возврат на базу
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return true;

}
/////////////////////////////////////////////////////////////////////////////
//
// узел для стационарного врага
//
/////////////////////////////////////////////////////////////////////////////
DCTOR_IMP(FixedEnemyNode)
// конструктор - id существа и позиция, где он будет стоять
FixedEnemyNode::FixedEnemyNode(eid_t id) :
CommonEnemyNode(id),
m_mode(FEM_BASE),  m_basepnt(0, 0),m_basedir(0.0f), m_turn(0),
m_target_dir(0.0f), m_target_pnt(0, 0),m_prev_dir(0.0f)
{
    GameEvMessenger::GetInst()->Attach(this, EV_SHOOT);

	if(!m_entity) return;
	OnSpawnMsg();
}

FixedEnemyNode::~FixedEnemyNode()
{
    GameEvMessenger::GetInst()->Detach(this);
	delete m_activity;
}

float FixedEnemyNode::Think(state_type* st)
{
	float complexity = 0.0f;
	if (ThinkShell(st)) return complexity;
    // юнит свободен - можно подумать
	bool flag = true;
    while(flag)
	{
        switch(m_mode)
		{
        case FEM_BASE:		  flag = thinkBase(st);        break;
        case FEM_LOOKROUND:   flag = thinkLookRound(st);   break;
        case FEM_ATTACK:      flag = thinkAttack(st);      break;
        case FEM_RETURN2BASE: flag = thinkReturn2Base(st); break;
        case FEM_WEAPONSEARCH:flag = thinkWeaponSearch(st);break;
        case FEM_WEAPONTAKE:  flag = thinkWeaponTake(st);  break;
        case FEM_MEDKITTAKE:  flag = thinkMedkitTake(st);  break;
        }
    }

	CheckFinished(st, &complexity);
	return complexity;
}

void FixedEnemyNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
        st << static_cast<int>(m_lookround_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());

		st << m_basepnt.x << m_basepnt.y << m_basedir << m_turn << m_target_dir
			<< m_target_pnt.x << m_target_pnt.y << m_prev_dir;

		return;
    }
	// читаемся
	int tmp;
	st >> tmp; m_mode = static_cast<FixedEnemyMode>(tmp);
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> tmp; m_lookround_state = static_cast<LookroundState>(tmp);
	st >> m_entity;
	st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));

	st >> m_basepnt.x >> m_basepnt.y >> m_basedir >> m_turn >> m_target_dir
		>> m_target_pnt.x >> m_target_pnt.y >> m_prev_dir;
	
	// то, что восстанавливается без чтения
	m_activity = 0;
	AIAPI::getInst()->getBaseField(m_basepnt, 10, &m_base_field);
	m_initialising = false;
}


//обработка сообщения о расстановке
void FixedEnemyNode::OnSpawnMsg()
{	
	m_basepnt = AIAPI::getInst()->getPos2(m_entity);
	m_basedir = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	AIAPI::getInst()->getBaseField(m_basepnt, 10, &m_base_field);
}

void FixedEnemyNode::Update(subject_t subj, event_t event, info_t ptr)
{
    switch(event){
    case EV_SHOOT:
        {
            shoot_info* info = static_cast<shoot_info*>(ptr);
            OnShootEv(info->m_actor, info->m_victim, info->m_point);
        }
        break;
    }
}

// обработка сообщения об выстреле или попадании
void FixedEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	BaseEntity* be = AIAPI::getInst()->getPtr(m_entity);
	if( be && (be == who) ) return; // это я сам натворил

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (be == victim))
	{	// будем реагировать - оборот на следующий ход
		m_turn = fixed_turns-1;
	}
}

// обработка режима Base
bool FixedEnemyNode::thinkBase(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(fixed_cure_dist);

	// 2. если у нас нет оружия - перейдем к его поиску
	if( (!AIAPI::getInst()->takeBestWeapon(m_entity, FixedWeaponComparator())) &&	(!AIAPI::getInst()->takeGrenade(m_entity))	)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkBase: надо искать пушку!");
		m_mode = FEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_turn = 0;
		return false;
	}
	
	// 3. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity)) 	)
		{
			AIAPI::getInst()->print(m_entity, "|F|thinkBase: я увидел врага!");
			// выбрал самого опасного врага, пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,	&enemy,	fixed_enemy_dist);
			// переход в состояние атаки, выберем лучший хекс в радиусе 10 хексов от базовой точки:
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			// получим вектор доступных хексов базового поля
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 4. если моя подкоманда засекла врага - перейду в атаку
	enemies.clear();
	if(AIAPI::getInst()->getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies)  )
	{
		// засекла - выберем самого опасного
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity)))
		{
			AIAPI::getInst()->print(m_entity, "|F|thinkBase: моя команда засекла врага!");
			// выбрал самого опасного врага, переход в состояние атаки
			// выберем лучший хекс в радиусе 10 хексов от базовой точки:
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			// получим вектор доступных хексов базового поля
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 5. врагов нет
	// 5.1 если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 5.2 если вижу аптечку и она мне нужна - побегу за ней
	if(needAndSeeMedkit(&m_target_pnt))
	{
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
		if(!path.empty())
		{
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVEIGNORE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_mode = FEM_MEDKITTAKE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 5.3 если время оборачиваться - обернусь
	if(++m_turn < fixed_turns)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkBase: еще не время обернуться");
		// еще не время оборачиваться
		m_turn_state = TS_END;
		return false;
	}
	// пора обернуться
	AIAPI::getInst()->print(m_entity, "|F|thinkBase: пора оборачиваться");
	m_turn = 0;
	m_turn_state = TS_INPROGRESS;
	m_mode = FEM_LOOKROUND;
	m_lookround_state = LS_FIRST;
	m_prev_dir = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	m_target_dir = norm_angle(m_prev_dir + 2*PId3);
	m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir,	ActivityFactory::CT_ENEMY_LOOKROUND);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_SPOTTED);
	return false;
}

// обработка режима LookRound
bool FixedEnemyNode::thinkLookRound(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(fixed_cure_dist);

	// 2. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{	// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|F|thinkLookRound: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,&enemy,fixed_enemy_dist);
			// переход в состояние атаки, выберем лучший хекс в радиусе 10 хексов от базовой точки:
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			// получим вектор доступных хексов базового поля
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 3. если моя подкоманда засекла врага - перейду в атаку
	enemies.clear();
	if(AIAPI::getInst()->getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies))
	{
		// засекла - выберем самого опасного
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|F|thinkLookRound: моя команда засекла врага!");
			// выбрал самого опасного врага, переход в состояние атаки
			// выберем лучший хекс в радиусе 10 хексов от базовой точки:
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			// получим вектор доступных хексов базового поля
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 4. получим текущий угол, на который повернуто существо
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if(fabs(m_target_dir - cur_angle) < angle_eps)
	{
		// закончен очередной прием разворота
		switch(m_lookround_state)
		{
		case LS_FIRST : 
			{
				AIAPI::getInst()->print(m_entity, "|F|thinkLookRound: закончен первый прием разворота");
				// начинаем второй прием разворота
				m_turn_state = TS_INPROGRESS;
				m_lookround_state = LS_SECOND;
				m_target_dir = norm_angle(m_prev_dir + 4*PId3);
				m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir,	ActivityFactory::CT_ENEMY_LOOKROUND);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
				break;
			}
		case LS_SECOND : 
			{
				AIAPI::getInst()->print(m_entity, "|F|thinkLookRound: закончен второй прием разворота");
				// начинаем заключительный прием разворота
				m_turn_state = TS_INPROGRESS;
				m_lookround_state = LS_2BASE;
				m_target_dir = m_basedir;
				m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),	m_target_dir,ActivityFactory::CT_ENEMY_LOOKROUND);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
				break;
			}
		case LS_2BASE : 
			{
				AIAPI::getInst()->print(m_entity, "|F|thinkLookRound: полностью закончил разворот");
				// переходим в базовое состояние
				m_mode = FEM_BASE;
				m_turn_state = TS_END;
				break;
			}
		default : break;
		}
		return false;
	}

	// 5. если не хватило мувпоинтов - продолжим на следющий ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkLookRound: не хватило мувпоинтов - продолжим на следющий ход");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// продолжим действие
	m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir,ActivityFactory::CT_ENEMY_LOOKROUND);
	
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_SPOTTED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Attack
bool FixedEnemyNode::thinkAttack(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(fixed_cure_dist);

	// 2. если у нас нет оружия - перейдем к его поиску
	if( (!AIAPI::getInst()->takeBestWeapon(m_entity, FixedWeaponComparator())) &&
		(!AIAPI::getInst()->takeGrenade(m_entity))	)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkAttack: надо искать пушку!");
		m_mode = FEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::getInst()->setStandPose(m_entity);
		return false;
	}

	// 3. выберем врага
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// сам видит врага
		// если это начало хода или реакция - сбросим список игнорирования
		if(m_turn_state == TS_START)
		{
			m_ignored_enemies.clear();
			// и заодно присядем если уже не сидим
			AIAPI::getInst()->setSitPose(m_entity);
		}
		// попробуем найти разность списков всех врагов и игнорированных врагов
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty()) // выберем самого опасного врага из всех
			enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		else // выберем самого опасного врага из непроигнорированных
			enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));
	}
	
	if(!enemy)
	{
		// сам врага не видит
		enemies.clear();
		if(AIAPI::getInst()->getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies)	)
		{
			// зато видит его подкоманда
			// если это начало хода или реакция - сбросим список игнорирования
			if(m_turn_state == TS_START)
			{
				m_ignored_enemies.clear();
				// и заодно присядем если уже не сидим
				AIAPI::getInst()->setSitPose(m_entity);
			}
			// попробуем найти разность списков всех врагов и игнорированных врагов
			EntityList_t diff_list;
			setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
			if(diff_list.empty())// выберем самого опасного врага из всех
				enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
			else// выберем самого опасного врага из непроигнорированных
				enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));
		}
	}
	
	// 4. если нет врагов - возврат в базовую точку
	if(!enemy)
	{
		// больше нет врагов - переход в состояние возврат в базовую точку
		AIAPI::getInst()->print(m_entity, "|F|thinkAttack: врагов нет - на базу");
		m_mode = FEM_RETURN2BASE;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::getInst()->setStandPose(m_entity);
		return false;
	}
	// враги есть...

	// 5. если это начало нового хода
	if(m_turn_state == TS_START)
	{
		// выбираем лучшую точку и идем в нее
		AIAPI::getInst()->setStandPose(m_entity);
		AIAPI::getInst()->print(m_entity, "|F|thinkAttack: выбираем лучшую точку и идем в нее");
		// выберем лучший хекс в радиусе 10 хексов от базовой точки:
		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		// получим вектор доступных хексов базового поля
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
		// получим лучший хекс
		m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_ENEMY_REACTED);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_ENEMY_SPOTTED);
		
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 6. это продолжение хода

	// 6.1 если юниту не хватило мувпоинтов - он передает ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkAttack: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}
	
	// 6.2 если сработала реакция или увидел нового врага - имитируем
	//       начало хода
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED) 	)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkAttack: сработала реакция или увидел нового врага");
		m_activity_observer.clear();
		// имитируем начало хода
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return false;
	}

	// 6.3 если нет линии огня - добавляем врага в список игнорирования
	// причем, если после этого все враги оказались в списке игнорирования
	// передаем ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkAttack: нет линии огня - добавляем врага в список игнорирования");
		m_activity_observer.clear();
		// добавим в список игнорирования
		m_ignored_enemies.push_back(enemy);
		m_ignored_enemies.unique();
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);

		if(!diff_list.empty())
		{
			// зациклимся для поиска других врагов
			m_turn_state = TS_INPROGRESS;
			return true;
		}
		
		if(AIAPI::getInst()->isSitPose(m_entity))
		{
			// встанем
			AIAPI::getInst()->setStandPose(m_entity);
			// обнулим спискок проигнорированных врагов
			m_ignored_enemies.clear();
			// попробуем поискать врагов еще раз
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// передаем ход
		AIAPI::getInst()->setSitPose(m_entity);
		m_turn_state = TS_END;
		return false;
	}

	// 6.4 если видит более одного врага - пытается кинуть шилд, защищаясь от слабейшего
	eid_t weak_enemy = AIAPI::getInst()->getLeastDangerousEnemy(enemies, EnemyComparator(m_entity));
	if(enemy != weak_enemy)
	{
		// врагов более одного - попробуем кинуть в слабого шилдом
		if(AIAPI::getInst()->takeShield(m_entity))
		{
			AIAPI::getInst()->print(m_entity, "|F|thinkAttack: а мы его шилдом!");
			AIAPI::getInst()->throwShield(m_entity, weak_enemy, &m_activity);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 6.5 Если не повернут к врагу - поворачивается в направлении где был враг
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// нужно поворачиваться к врагу
		AIAPI::getInst()->setStandPose(m_entity);
		m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),	m_target_dir,ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// 6.6 повернут к врагу - если сможем будем атаковать, присядем
	AIAPI::getInst()->setSitPose(m_entity);
	
	AIAPI::getInst()->print(m_entity, "|F|thinkAttack: будем стрелять, если это возможно!");
	// враг есть - выстрелим по нему если это возможно
	if(!AIAPI::getInst()->takeBestWeapon(m_entity, FixedWeaponComparator()))
	{
		AIAPI::getInst()->takeGrenade(m_entity);
	}
	AIAPI::getInst()->setShootType(m_entity, 80.0f, 15.0f);
	std::string reason;
	if(AIAPI::getInst()->shoot(m_entity, enemy, &m_activity, &reason))
	{
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
	}
	else
	{
		std::string str = "|F|thinkAttack: стрелять нельзя: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		AIAPI::getInst()->setSitPose(m_entity);
		m_turn_state = TS_END;
	}
//	m_turn_state = TS_END;
	return false;
}

// обработка режима Return2Base
bool FixedEnemyNode::thinkReturn2Base(state_type* st)
{
	// 1. проверим пришли мы уже в базовую точку или нет
	if(m_basepnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// пришли - переход в базовое состояние
		AIAPI::getInst()->print(m_entity, "|F|thinkReturn2Base: пришли в базовую точку");
		m_mode = FEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// 2. не закончились ли мувпоинты?
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - отдадим ход
		AIAPI::getInst()->print(m_entity, "|F|thinkReturn2Base: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// мы находимся не в базовой точке
	// 3. проверим, можно ли вообще дойти до базовой точки
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_basepnt, path);
	if(!path.empty())
	{
		// можно - пойдем в нее
		m_target_pnt = m_basepnt;
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// в базовую точку дойти нельзя
	// 4. находимся ли мы на базовом поле?
	if(AIAPI::getInst()->isEntityInBaseField(m_entity, m_base_field))
	{
		// да - переход в базовое состояние
		AIAPI::getInst()->print(m_entity, "|F|thinkReturn2Base: пришли в базовое поле");
		m_mode = FEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// мы вне базового поля
	// 5. можно ли дойти до какой-нибудь точки в базовом поле?
	pnt_vec_t vec;
	AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
	if(vec.empty())
	{
		// нельзя - переход в базовое состояние
		AIAPI::getInst()->print(m_entity, "|F|thinkReturn2Base: не могу дойти даже до базового поля!");
		m_mode = FEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно
	// выберем случайную точку
	m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
	// пойдем в нее
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer,		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима WeaponSearch
bool FixedEnemyNode::thinkWeaponSearch(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(fixed_cure_dist);
	// 2. проверим есть ли где-нить рядом оружие
	if(AIAPI::getInst()->getWeaponLocation(m_entity, &m_target_pnt))
	{
		// недалеко есть оружие - переход к его захвату
		pnt_vec_t base, vec, path;
		AIAPI::getInst()->getBaseField(m_target_pnt, 1, &base);
		AIAPI::getInst()->getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);

		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		if(path.empty())// к оружию нельзя подойти - отдадим ход
		{
			m_turn_state = TS_END;
			return false;
		}
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		m_mode = FEM_WEAPONTAKE;
		return false;
	}

	// оружия рядом нет - проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|F|thinkWeaponSearch: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}
	
	// если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// мувпоинты есть - выберем случайную точку и ломанемся туда
	pnt_vec_t vec, path;
	AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
	m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);

	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),m_target_pnt, path);
	if(path.empty())
	{
		// некуда идти - передадим ход
		m_turn_state = TS_END;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима WeaponTake
bool FixedEnemyNode::thinkWeaponTake(state_type* st)
{
	// 1. проверим не подошли ли мы к оружию
	if(PickUpWeaponAndAmmo(m_target_pnt, FixedWeaponComparator()))
	{
		m_mode = FEM_RETURN2BASE;
		return false;
	}

	// 2. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|F|thinkWeaponTake: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. проверим есть ли где-нить рядом оружие
	if(AIAPI::getInst()->getWeaponLocation(m_entity, &m_target_pnt))
	{
		// недалеко есть оружие - пойдем к нему
		pnt_vec_t base, vec, path;
		AIAPI::getInst()->getBaseField(m_target_pnt, 1, &base);
		AIAPI::getInst()->getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);

		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),m_target_pnt, path);
		if(path.empty())
		{
			// к оружию нельзя подойти - возврат на базу
			m_mode = FEM_RETURN2BASE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// оружия нет - возврат на базу
	m_mode = FEM_RETURN2BASE;
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима MedkitTake
bool FixedEnemyNode::thinkMedkitTake(state_type* st)
{
	if(!MedkitTake(m_target_pnt,st))	m_mode = FEM_RETURN2BASE;
	return false;

}

// умри!
bool FixedEnemyNode::die()
{
	m_mode = FEM_KILLED;
	return need2Delete();
}

// нужно ли удалять узел
bool FixedEnemyNode::need2Delete() const
{
	if( (m_mode == FEM_KILLED) && !m_activity) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// узел для патрульного врага
//
/////////////////////////////////////////////////////////////////////////////
DCTOR_IMP(PatrolEnemyNode)

// конструктор - id существа
PatrolEnemyNode::PatrolEnemyNode(eid_t id) :
CommonEnemyNode(id), m_mode(PEM_PATROL), 
m_enemy_id(0),m_enemy_pnt(0, 0), m_cure_id(0), m_cure_pnt(0, 0), m_shoot_pnt(0, 0),
m_shooted(false), m_target_pnt(0, 0), m_target_dir(0), m_last_enemy_pnt(0, 0)
{
    GameEvMessenger::GetInst()->Attach(this, EV_SHOOT);
    GameEvMessenger::GetInst()->Attach(this, EV_KILL);

	if(!m_entity) return;

	OnSpawnMsg();
}

PatrolEnemyNode::~PatrolEnemyNode()
{
    GameEvMessenger::GetInst()->Detach(this);
	delete m_activity;
}

float PatrolEnemyNode::Think(state_type* st)
{
	float complexity = 0.0f;
	if (ThinkShell(st)) return complexity;
    // юнит свободен - можно подумать
	bool flag = true;
    while(flag)
	{
        switch(m_mode)
		{
        case PEM_PATROL:    flag = thinkPatrol(st);       break;
        case PEM_ATTACK:    flag = thinkAttack(st);       break;
        case PEM_WEAPONSEARCH: flag = thinkWeaponSearch(st);     break;
        case PEM_THINGTAKE:  flag = thinkThingTake(st);         break;
        case PEM_WEAPONTAKE:  flag = thinkWeaponTake(st);        break;
        case PEM_CURE:       flag = thinkCure(st);       break;
        case PEM_PURSUIT:    flag = thinkPursuit(st);   break;
        }
    }

	CheckFinished(st, &complexity);
	return complexity;
}

void PatrolEnemyNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());
		st << m_enemy_id << m_enemy_pnt.x << m_enemy_pnt.y << m_cure_id
			<< m_cure_pnt.x << m_cure_pnt.y << m_shoot_pnt.x << m_shoot_pnt.y
			<< m_shooted << m_target_pnt.x << m_target_pnt.y << m_target_dir
			<< m_last_enemy_pnt.x << m_last_enemy_pnt.y;
		return;
    }
	// читаемся
	int tmp;
	st >> tmp; m_mode = static_cast<PatrolEnemyMode>(tmp);
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> m_entity;
	st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));

	st >> m_enemy_id >> m_enemy_pnt.x >> m_enemy_pnt.y >> m_cure_id
		>> m_cure_pnt.x >> m_cure_pnt.y >> m_shoot_pnt.x >> m_shoot_pnt.y
		>> m_shooted >> m_target_pnt.x >> m_target_pnt.y >> m_target_dir
		>> m_last_enemy_pnt.x >> m_last_enemy_pnt.y;
	
	// то, что восстанавливается без чтения
	m_activity = 0;
	m_initialising = false;
}

// обработка внутрикомандных сообщений
void PatrolEnemyNode::recieveMessage(MessageType type, void * data)
{
	eid_t id = *static_cast<eid_t*>(data);
	switch(type)
	{
	case MT_ENEMYSPOTTED :
		{
			if(m_enemy_id)
			{
				// уже сообщали о враге, проверим кто из врагов ближе
				float old_dist = dist(AIAPI::getInst()->getPos2(m_entity),	m_enemy_pnt);
				float new_dist = dist(AIAPI::getInst()->getPos2(m_entity),	AIAPI::getInst()->getPos2(id));
				if(new_dist < old_dist)
				{  	// сменим врага
					m_enemy_id = id;
					m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
				}
				return;
			}
			// это первый враг, о котором сообщили
			m_enemy_id = id;
			m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
			return;
		}
	case MT_NEEDCURE :
		{
			if(m_cure_id)
			{
				// уже просили о помощи, проверим кто из просящих ближе
				float old_dist = dist(AIAPI::getInst()->getPos2(m_entity),	m_cure_pnt);
				float new_dist = dist(AIAPI::getInst()->getPos2(m_entity),	AIAPI::getInst()->getPos2(id));
				if(new_dist < old_dist)
				{	// сменим просящего о помощи
					m_cure_id = id;
					m_cure_pnt = AIAPI::getInst()->getPos2(m_cure_id);
				}
				return;
			}
			// это первое сообщение о помощи
			m_cure_id = id;
			m_cure_pnt = AIAPI::getInst()->getPos2(m_cure_id);
			return;
		}
	default : return;
	}
}


void PatrolEnemyNode::Update(subject_t subj, event_t event, info_t ptr)
{
    switch(event)
	{
    case EV_SHOOT:
        {
            shoot_info* info = static_cast<shoot_info*>(ptr);
            OnShootEv(info->m_actor, info->m_victim, info->m_point);
        }
        break;
    case EV_KILL:
        {
            kill_info* info = static_cast<kill_info*>(ptr);
            OnKillEv(info->m_killer, info->m_victim);
        }
        break;
    }
}

//обработка собщения об убийстве
void PatrolEnemyNode::OnKillEv(BaseEntity* killer, BaseEntity* victim)
{
	// почистим ссылки на людей
	if(victim == AIAPI::getInst()->getPtr(m_enemy_id)) m_enemy_id = 0;
	if(victim == AIAPI::getInst()->getPtr(m_cure_id)) m_cure_id = 0;
}

// обработка сообщения об выстреле или попадании
void PatrolEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// не будем реагировать в случае,
	// если юнит находится в определенных состояниях
	switch(m_mode)
	{
	case PEM_ATTACK: case PEM_WEAPONSEARCH: case PEM_WEAPONTAKE : return;
	default: break;
	}

	if(AIAPI::getInst()->getPtr(m_entity) == who) return; // это я сам натворил

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (AIAPI::getInst()->getPtr(m_entity) == victim) )
	{
		// будем реагировать
		m_shoot_pnt = AIAPI::getInst()->getPos2(who);
		m_shooted = true;
	}
}

// обработка режима Patrol
bool PatrolEnemyNode::thinkPatrol(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(patrol_cure_dist);

	// 2. если у нас нет оружия - перейдем к его поиску
	if( (!AIAPI::getInst()->takeGrenade(m_entity)) &&
		(!AIAPI::getInst()->takeBestWeapon(m_entity, PatrolWeaponComparator()))	)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: надо искать пушку!");
		m_mode = PEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 3. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,	&enemy,	patrol_enemy_dist);
			// переход в состояние атаки
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
			AIAPI::getInst()->setRun(m_entity);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел aim, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	SHT_AIMSHOT);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);

			if(!steps) // будем считать что стоим где надо, просто переходим в, атаку
				return false;
			// рассчитаем поле проходимости
			pnt_vec_t field, vec, path;
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
			// получим вектор доступных хексов
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,PatrolHexComparator(m_entity, enemy));
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 4. Если в меня стреляли - будем преследовать
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: в меня стреляли - будем преследовать");
		// в меня или рядом со мной стреляли
		m_shooted = false;
		// выберем точку в радиусе 3 хексов от существа, которое стреляло
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		pnt_vec_t field, vec, path;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		AIAPI::getInst()->getBaseField(m_shoot_pnt,	3, &field);
		// получим вектор доступных хексов
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. если мне сообщили о враге - пойдем к нему
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: мне сообщили о враге");
		// сообщили
		m_enemy_id = 0;
		// выберем точку в радиусе 1 хекса от врага
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,	1, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 6. если есть кого лечить - переходим в состояние лечения
	if(m_cure_id)
	{
		// проверим здоровье юнита
		if( (AIAPI::getInst()->getHealthPercent(m_cure_id) < 50.0f) &&
			(AIAPI::getInst()->haveMedikit(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: нужно лечить");
			// его нужно лечить
			m_mode = PEM_CURE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// его не нужно лечить
		m_cure_id = 0;
	}

	// 7. если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 8. если увидели на уровне оружие, патроны или аптечку - 
	// переход в состояние захват вещи
	ipnt2_t pos;
	if(AIAPI::getInst()->getThingLocation(m_entity,	&pos,TT_WEAPON|TT_AMMO|TT_MEDIKIT)	)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: увидел на уровне оружие, патроны или аптечку");
		pnt_vec_t base;
		pnt_vec_t vec;
		AIAPI::getInst()->getBaseField(pos, 1, &base);
		AIAPI::getInst()->getReachableField(base, &vec, true);
		pos = AIAPI::getInst()->getRandomPoint(vec);
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	pos, path);
		if(!path.empty())
		{
			// можно идти
			m_target_pnt = pos;
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			m_mode = PEM_THINGTAKE;
			return false;
		}
	}

	// 9. если дошел до заданной точки - выбирает новую
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: дошел до заданной точки - выбирает новую");
		// рассчитаем поле проходимости
		pnt_vec_t allhexes;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, &allhexes, 0);
		if(allhexes.empty())
		{
			// передаем ход - идти некуда
			AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: мне некуда ходить!!!");
			m_turn_state = TS_END;
			return false;
		}
		// получим случайную точку
		m_target_pnt = AIAPI::getInst()->getRandomPoint(allhexes);
	}

	// 10. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 10. идем в заданную точку
	AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: идем в заданную точку");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Attack
bool PatrolEnemyNode::thinkAttack(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(patrol_cure_dist);
	// 2. если у нас нет оружия - перейдем к его поиску
	if( (!AIAPI::getInst()->takeGrenade(m_entity)) &&
		(!AIAPI::getInst()->takeBestWeapon(m_entity, PatrolWeaponComparator()))	)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkAttack: надо искать пушку!");
		m_mode = PEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::getInst()->setWalk(m_entity);
		return false;
	}

	// 3. выберем врага
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// сам видит врага, если это начало хода или реакция - сбросим список игнорирования
		if(m_turn_state == TS_START)	m_ignored_enemies.clear();

		// попробуем найти разность списков всех врагов и игнорированных врагов
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);

		if(diff_list.empty())// выберем самого опасного врага из всех
			enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		else// выберем самого опасного врага из непроигнорированных
			enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));

		m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
	}
	
	// 4. если нет врагов - переходим в состояние преследования
	if(!enemy)
	{
		// врагов нет - будем преследовать последнего
		AIAPI::getInst()->print(m_entity, "|P|thinkAttack: врагов нет - будем преследовать последнего");
		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		AIAPI::getInst()->setWalk(m_entity);
		return false;
	}
	// враги есть...

	// 5. если это начало нового хода
	if(m_turn_state == TS_START)
	{
		// выбираем лучшую точку и идем в нее
		AIAPI::getInst()->print(m_entity, "|P|thinkAttack: выбираем лучшую точку и идем в нее");

		// выберем лучший хекс так, чтобы потом хватило мувпоинтов
		// на один выстрел aim, если таких хексов нет - считает
		// лучшим хекс, в котором стоит.
		
		m_turn_state = TS_INPROGRESS;
		AIAPI::getInst()->setRun(m_entity);
		int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
		movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	SHT_AIMSHOT);
		// вычтем мувпоинты на поворот к врагу
		movpoints -= MPS_FOR_ROTATE*4;
		int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);

		if(!steps)// будем считать что стоим где надо, просто переходим в, атаку
			return false;

		// рассчитаем поле проходимости
		pnt_vec_t field, vec, path;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
		// получим вектор доступных хексов
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// получим лучший хекс
		m_target_pnt = AIAPI::getInst()->getBestPoint(vec, 	PatrolHexComparator(m_entity, enemy));

		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_SPOTTED);
		return false;
	}
	// 6. это продолжение хода
	// 6.1 если юниту не хватило мувпоинтов - он передает ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		AIAPI::getInst()->print(m_entity, "|P|thinkAttack: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6.2 если сработала реакция или увидел нового врага - имитируем
	//       начало хода
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)	)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkAttack: сработала реакция или увидел нового врага");
		m_activity_observer.clear();
		// имитируем начало хода
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 6.3 если нет линии огня - добавляем врага в список игнорирования
	// причем, если после этого все враги оказались в списке игнорирования
	// попробуем подойти к одному из игнорированных врагов
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// закончились - закончим ход
		AIAPI::getInst()->print(m_entity, "|P|thinkAttack: нет линии огня - добавляем врага в список игнорирования");
		m_activity_observer.clear();
		// добавим в список игнорирования
		m_ignored_enemies.push_back(enemy);
		m_ignored_enemies.unique();
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// попробуем подойти поближе к врагу
			AIAPI::getInst()->print(m_entity, "|P|thinkAttack: подойдем поближе");
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, PathUtils::F_CALC_NEAR_PNTS);
			EntityList_t::iterator i = enemies.begin();
			// найдем существо, к которому можно подойти
			bool found = false;
			while(i != enemies.end())
			{
				// проверим, существуют ли точки рядом с существом
				if(PathUtils::GetInst()->IsNear(AIAPI::getInst()->getPtr(*i)))
				{
					// к юниту можно подойти
					found = true;
					m_target_pnt = PathUtils::GetInst()->GetNearPnt(AIAPI::getInst()->getPtr(*i)).m_pnt;
					break;
				}
				++i;
			}
			pnt_vec_t path;
			if(!found)
			{
				// подходящего врага нет - переместимся случайным образом
				pnt_vec_t base, vec;
				AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	patrol_enemy_noLOF_move,&base);
				AIAPI::getInst()->getReachableField(base, &vec, true);
				m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			}
			else
			{
				// есть враг, к которому можно подойти - подойдем на половину пути
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
				// отрежем половину пути
				path.erase(path.begin(), path.begin() + (path.size() / 2));
			}
			if(path.empty())
			{
				// передадим ход
				m_turn_state = TS_END;
				return false;
			}
			// можно идти
			AIAPI::getInst()->setWalk(m_entity);
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
			
			m_turn_state = TS_INPROGRESS;
			return false;
		} 
		// зациклимся для поиска других врагов
		m_turn_state = TS_INPROGRESS;
		return true;
	}

	// 6.4 Если не повернут к врагу - поворачивается в направлении где был враг
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// нужно поворачиваться к врагу
		m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir,ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 6.5 повернут к врагу - если сможем будем атаковать
	AIAPI::getInst()->print(m_entity, "|P|thinkAttack: будем стрелять, если это возможно!");
	// враг есть - выстрелим по нему если это возможно

	// сначала попробуем гранатой (если есть и если получится)
	if(AIAPI::getInst()->takeGrenade(m_entity))
	{
		// граната в руках - попробуем кинуть ее
		std::string reason;
		if(AIAPI::getInst()->shoot(m_entity, enemy, &m_activity, &reason))
		{
			// удалось начать бросок гранаты
			m_turn_state = TS_INPROGRESS;
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
			return false;
		}
		// начать бросок гранаты не удалось
		std::string str = "|P|thinkAttack: нельзя кинуть гранату: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
	}

	// гранаты или нет, или бросить ее не удалось
	// теперь попробуем пострелять из оружия
	if(!AIAPI::getInst()->takeBestWeapon(m_entity, PatrolWeaponComparator()))
	{
		m_turn_state = TS_END;
		return false;
	}
	// оружие в руках
	AIAPI::getInst()->setShootType(m_entity, 5.0f, 10.0f);
	std::string reason;
	if(AIAPI::getInst()->shoot(m_entity, enemy, &m_activity, &reason))
	{
		// удалось начать стрельбу
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
	}
	else
	{
		std::string str = "|P|thinkAttack: стрелять нельзя: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

// обработка режима WeaponSearch
bool PatrolEnemyNode::thinkWeaponSearch(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(patrol_cure_dist);

	// 2. проверим есть ли где-нить рядом оружие
	if(AIAPI::getInst()->getWeaponLocation(m_entity, &m_target_pnt))
	{
		// недалеко есть оружие - переход к его захвату
		AIAPI::getInst()->print(m_entity, "|P|thinkWeaponSearch: недалеко есть оружие - переход к его захвату!");
		pnt_vec_t base;
		pnt_vec_t vec;
		AIAPI::getInst()->getBaseField(m_target_pnt, 1, &base);
		AIAPI::getInst()->getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		if(path.empty())
		{
			// к оружию нельзя подойти - отдадим ход
			m_turn_state = TS_END;
			return false;
		}
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		m_mode = PEM_WEAPONTAKE;
		return false;
	}

	// 3. если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 4. если дошел до заданной точки - выбирает новую
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkWeaponSearch: дошел до заданной точки - выбирает новую");
		// рассчитаем поле проходимости
		pnt_vec_t allhexes;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, &allhexes, 0);
		if(allhexes.empty())
		{
			// передаем ход - идти некуда
			AIAPI::getInst()->print(m_entity, "|P|thinkWeaponSearch: мне некуда ходить!!!");
			m_turn_state = TS_END;
			return false;
		}
		// получим случайную точку
		m_target_pnt = AIAPI::getInst()->getRandomPoint(allhexes);
	}

	// 5. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|P|thinkWeaponSearch: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. идем в заданную точку
	AIAPI::getInst()->print(m_entity, "|P|thinkWeaponSearch: идем в заданную точку");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима ThingTake
bool PatrolEnemyNode::thinkThingTake(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(patrol_cure_dist);

	// 2. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies, EnemyComparator(m_entity)) )
		{
			AIAPI::getInst()->print(m_entity, "|P|thinkThingTake: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this, AIEnemyNode::MT_ENEMYSPOTTED, &enemy, patrol_enemy_dist);
			// переход в состояние атаки
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
			AIAPI::getInst()->setRun(m_entity);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел aim, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	SHT_AIMSHOT);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// будем считать что стоим где надо, просто переходим в
				// атаку
				return false;
			}
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
			// получим вектор доступных хексов
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	PatrolHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 3. проверим не подошли ли мы к вещи
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// мы подошли к валяющимся вещам - возьмем что сможем
		AIAPI::getInst()->print(m_entity, "|P|thinkThingTake: мы подошли к валяющимся вещам - возьмем что сможем");
		AIAPI::getInst()->pickupAllNearAmmo(m_entity);
		AIAPI::getInst()->pickupAllNearMedikit(m_entity);
		AIAPI::getInst()->pickupAllNearWeapon(m_entity);
		AIAPI::getInst()->dropUselessWeapon(m_entity, PatrolWeaponComparator());
		// переходим в состояние патрулирования
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. Если в меня стреляли - будем преследовать
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkThingTake: в меня стреляли - будем преследовать");
		// в меня или рядом со мной стреляли
		m_shooted = false;
		// выберем точку в радиусе 3 хексов от существа, которое стреляло
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_shoot_pnt,	3, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. если мне сообщили о враге - пойдем к нему
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkThingTake: мне сообщили о враге");
		// сообщили
		m_enemy_id = 0;
		// выберем точку в радиусе 1 хекса от врага
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,	1, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 6. если есть кого лечить - переходим в состояние лечения
	if(m_cure_id)
	{
		// проверим здоровье юнита
		if( (AIAPI::getInst()->getHealthPercent(m_cure_id) < 50.0f) &&	(AIAPI::getInst()->haveMedikit(m_entity)))
		{
			AIAPI::getInst()->print(m_entity, "|P|thinkThingTake: нужно лечить");
			// его нужно лечить
			m_mode = PEM_CURE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// его не нужно лечить
		m_cure_id = 0;
	}
	
	// 7. если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 8. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|P|thinkThingTake: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 9. проверим есть ли где-нить рядом оружие
	if(AIAPI::getInst()->getThingLocation(m_entity, &m_target_pnt,	TT_AMMO|TT_WEAPON|TT_MEDIKIT)	)
	{
		// недалеко есть оружие - пойдем к нему
		pnt_vec_t base;
		pnt_vec_t vec;
		AIAPI::getInst()->getBaseField(m_target_pnt, 1, &base);
		AIAPI::getInst()->getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		if(path.empty())
		{
			// к оружию нельзя подойти - возврат к патрулированию
			m_mode = PEM_PATROL;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// оружия нет - возврат к патрулированию
	m_mode = PEM_PATROL;
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима WeaponTake
bool PatrolEnemyNode::thinkWeaponTake(state_type* st)
{
	SendCureMessage(patrol_cure_dist);

	// 2. проверим не подошли ли мы к оружию
	if(PickUpWeaponAndAmmo(m_target_pnt, PatrolWeaponComparator()))
	{
		m_mode = PEM_PATROL;
		return false;
	}

	// 3. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|P|thinkWeaponTake: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. проверим есть ли где-нить рядом оружие
	if(AIAPI::getInst()->getWeaponLocation(m_entity, &m_target_pnt))
	{
		// недалеко есть оружие - пойдем к нему
		pnt_vec_t base;
		pnt_vec_t vec;
		AIAPI::getInst()->getBaseField(m_target_pnt, 1, &base);
		AIAPI::getInst()->getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
			m_target_pnt, path);
		if(path.empty())
		{
			// к оружию нельзя подойти - возврат к патрулированию
			m_mode = PEM_PATROL;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// оружия нет - возврат к патрулированию
	m_mode = PEM_PATROL;
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Cure
bool PatrolEnemyNode::thinkCure(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(patrol_cure_dist);

	// 2. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::getInst()->print(m_entity, "|P|thinkCure: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_enemy_dist);
			// переход в состояние атаки
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
			AIAPI::getInst()->setRun(m_entity);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел aim, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
				SHT_AIMSHOT);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// будем считать что стоим где надо, просто переходим в
				// атаку
				return false;
			}
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
				steps, &field);
			// получим вектор доступных хексов
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,
				PatrolHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
				m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
				path,
				ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 3. Если в меня стреляли - будем преследовать
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkCure: в меня стреляли - будем преследовать");
		// в меня или рядом со мной стреляли
		m_shooted = false;
		// выберем точку в радиусе 3 хексов от существа, которое стреляло
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_shoot_pnt,
			3, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
			m_target_pnt, path);
		// можно идти
		AIAPI::getInst()->setWalk(m_entity);
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. если мне сообщили о враге - пойдем к нему
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkCure: мне сообщили о враге");
		// сообщили
		m_enemy_id = 0;
		// выберем точку в радиусе 1 хекса от врага
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,
			1, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
			m_target_pnt, path);
		// можно идти
		AIAPI::getInst()->setWalk(m_entity);
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 6. если некого или нечем лечить
	if( (!m_cure_id) ||
		(AIAPI::getInst()->getHealthPercent(m_cure_id) >= 50.0f) ||
		(!AIAPI::getInst()->haveMedikit(m_entity))
		)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkCure: некого или нечем лечить");
		// некого или нечем лечить - переход к патрулированию
		m_cure_id = 0;
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		AIAPI::getInst()->setWalk(m_entity);
		return false;
	}

	// 7. если подошел к юниту - лечим его
	if(dist(AIAPI::getInst()->getPos2(m_entity),
		AIAPI::getInst()->getPos2(m_cure_id)) <= 1.5f)
	{
		// подошел - лечим
		AIAPI::getInst()->print(m_entity, "|P|thinkCure: лечим");
		// возьмем в руки аптечку
		AIAPI::getInst()->takeMedikit(m_entity);
		// полечим юнита
		AIAPI::getInst()->cure(m_entity, m_cure_id);
		// возьмем в руки оружие
		if(!AIAPI::getInst()->takeGrenade(m_entity))
		{
			AIAPI::getInst()->takeBestWeapon(m_entity, PatrolWeaponComparator());
		}
		// продолжим
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	
	// 8. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|P|thinkCure: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 9. идем к юниту, которого нужно лечить
	
	// рассчитаем поле проходимости
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
		0, 0, PathUtils::F_CALC_NEAR_PNTS);
	// проверим, существуют ли точки рядом с существом
	if(!PathUtils::GetInst()->IsNear(AIAPI::getInst()->getPtr(m_cure_id)))
	{
		// к юниту нельзя подойти
		m_cure_id = 0;
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	m_target_pnt = PathUtils::GetInst()->GetNearPnt(AIAPI::getInst()->getPtr(m_cure_id)).m_pnt;
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
		m_target_pnt, path);
	// можно идти
	AIAPI::getInst()->setRun(m_entity);
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
		path,
		ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Pursuit
bool PatrolEnemyNode::thinkPursuit(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(patrol_cure_dist);

	// 2. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::getInst()->print(m_entity, "|P|thinkPursuit: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_enemy_dist);
			// переход в состояние атаки
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
			AIAPI::getInst()->setRun(m_entity);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел aim, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
				SHT_AIMSHOT);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// будем считать что стоим где надо, просто переходим в
				// атаку
				return false;
			}
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
				steps, &field);
			// получим вектор доступных хексов
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,
				PatrolHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
				m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
				path,
				ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 3. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|P|thinkPursuit: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. проверим не пришли ли мы в заданную точку
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// мы пришли
		// 4.1 проверим, повернут ли в точку, где был враг
		m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->convertPos2ToPos3(m_last_enemy_pnt) - getPosition()));
		float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
		if( fabs(m_target_dir - cur_angle) >= angle_eps )
		{
			// нужно поворачиваться к врагу
			m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),
				m_target_dir,
				ActivityFactory::CT_ENEMY_ROTATE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 4.2 повернут - переход в состояние патрулирования
		AIAPI::getInst()->print(m_entity, "|P|thinkPursuit: никого не догнал - будем патрулировать");
		// переходим в состояние патрулирования
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. идем в заданную точку
	AIAPI::getInst()->print(m_entity, "|P|thinkPursuit: идем в заданную точку");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
		0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
		path,
		ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// умри!
bool PatrolEnemyNode::die()
{
	m_mode = PEM_KILLED;
	return need2Delete();
}

// нужно ли удалять узел
bool PatrolEnemyNode::need2Delete() const
{
	if( (m_mode == PEM_KILLED) && !m_activity) return true;
	return false;
}


/////////////////////////////////////////////////////////////////////////////
//
// узел для штурмовика
//
/////////////////////////////////////////////////////////////////////////////

// конструктор - id существа
AssaultEnemyNode::AssaultEnemyNode(eid_t id) :
CommonEnemyNode(id), m_mode(AEM_FIXED),
m_turn(0), m_enemy_id(0), m_enemy_pnt(0, 0), m_shoot_pnt(0, 0),
m_shooted(false), m_target_pnt(0, 0), m_target_dir(0), m_last_enemy_pnt(0, 0),
m_prev_dir(0)
{
    GameEvMessenger::GetInst()->Attach(this, EV_SHOOT);
    GameEvMessenger::GetInst()->Attach(this, EV_KILL);

    if(!m_entity) return;

	OnSpawnMsg();
}

AssaultEnemyNode::~AssaultEnemyNode()
{
    GameEvMessenger::GetInst()->Detach(this);
    delete m_activity;
}

DCTOR_IMP(AssaultEnemyNode)

float AssaultEnemyNode::Think(state_type* st)
{
	float complexity = 0.0f;
    //проиграть действие
	if (ThinkShell(st)) return complexity;
    // юнит свободен - можно подумать
	bool flag = true;
    while(flag)
	{
        switch(m_mode)
		{
        case AEM_FIXED:		flag = thinkFixed(st);  break;
        case AEM_LOOKROUND: flag = thinkLookround(st); break;
        case AEM_ATTACK:    flag = thinkAttack(st);    break;
        case AEM_WEAPONSEARCH: flag = thinkWeaponSearch(st);   break;
        case AEM_WEAPONTAKE:   flag = thinkWeaponTake(st);     break;
        case AEM_PURSUIT:      flag = thinkPursuit(st);        break;
        case AEM_MEDKITTAKE:   flag = thinkMedkitTake(st);     break;
        }
    }

	CheckFinished(st, &complexity);
	return complexity;
}

void AssaultEnemyNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
        st << static_cast<int>(m_lookround_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());
		st << m_turn;
		st << m_enemy_id;
		st << m_enemy_pnt.x;
		st << m_enemy_pnt.y;
		st << m_shoot_pnt.x;
		st << m_shoot_pnt.y;
		st << m_shooted;
		st << m_target_pnt.x;
		st << m_target_pnt.y;
		st << m_target_dir;
		st << m_last_enemy_pnt.x;
		st << m_last_enemy_pnt.y;
		st << m_prev_dir;
		return;
    }
	// читаемся
	int tmp;
	st >> tmp; m_mode = static_cast<AssaultEnemyMode>(tmp);
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> tmp; m_lookround_state = static_cast<LookroundState>(tmp);
	st >> m_entity;
	st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));
	st >> m_turn;
	st >> m_enemy_id;
	st >> m_enemy_pnt.x;
	st >> m_enemy_pnt.y;
	st >> m_shoot_pnt.x;
	st >> m_shoot_pnt.y;
	st >> m_shooted;
	st >> m_target_pnt.x;
	st >> m_target_pnt.y;
	st >> m_target_dir;
	st >> m_last_enemy_pnt.x;
	st >> m_last_enemy_pnt.y;
	st >> m_prev_dir;
	
	
	// то, что восстанавливается без чтения
	m_activity = 0;
	m_initialising = false;
}

// обработка внутрикомандных сообщений
void AssaultEnemyNode::recieveMessage(MessageType type, void * data)
{
	eid_t id = *static_cast<eid_t*>(data);
	if(type == MT_ENEMYSPOTTED)
	{
		if(m_enemy_id)
		{
			// уже сообщали о враге
			// проверим кто из врагов ближе
			float old_dist = dist(AIAPI::getInst()->getPos2(m_entity),
				m_enemy_pnt);
			float new_dist = dist(AIAPI::getInst()->getPos2(m_entity),
				AIAPI::getInst()->getPos2(id));
			if(new_dist < old_dist)
			{
				// сменим врага
				m_enemy_id = id;
				m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
			}
			return;
		}
		// это первый враг, о котором сообщили
		m_enemy_id = id;
		m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
	}
}


void AssaultEnemyNode::Update(subject_t subj, event_t event, info_t ptr)
{
    switch(event){
    case EV_SHOOT:
        {
            shoot_info* info = static_cast<shoot_info*>(ptr);
            OnShootEv(info->m_actor, info->m_victim, info->m_point);
        }
        break;
    case EV_KILL:
        {
            kill_info* info = static_cast<kill_info*>(ptr);
            OnKillEv(info->m_killer, info->m_victim);
        }
        break;
    }
}

//обработка собщения об убийстве
void AssaultEnemyNode::OnKillEv(BaseEntity* killer, BaseEntity* victim)
{
	// почистим ссылки на людей
	if(victim == AIAPI::getInst()->getPtr(m_enemy_id)) m_enemy_id = 0;
}

// обработка сообщения об выстреле или попадании
void AssaultEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// не будем реагировать в случае,
	// если юнит находится в определенных состояниях
	switch(m_mode)
	{
	case AEM_ATTACK: case AEM_WEAPONSEARCH: case AEM_WEAPONTAKE : return;
	default: break;
	}

	if(AIAPI::getInst()->getPtr(m_entity) == who) return; // это я сам натворил

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (AIAPI::getInst()->getPtr(m_entity) == victim) )
	{
		// будем реагировать
		m_shoot_pnt = AIAPI::getInst()->getPos2(who);
		m_shooted = true;
	}
}

// обработка режима Fixed
bool AssaultEnemyNode::thinkFixed(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(fixed_cure_dist);

	// 2. если у нас нет оружия - перейдем к его поиску
	if( (!AIAPI::getInst()->takeBestWeapon(m_entity, AssaultWeaponComparator())) &&
		(!AIAPI::getInst()->takeGrenade(m_entity))
		)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkFixed: надо искать пушку!");
		m_mode = AEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_turn = 0;
		return false;
	}
	
	// 3. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::getInst()->print(m_entity, "|A|thinkFixed: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				assault_enemy_dist);
			// переход в состояние атаки
			m_mode = AEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел aim, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
				SHT_AIMSHOT);
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= MPS_FOR_ROTATE*4;
			// вычтем мувпоинты на приседание
			movpoints -= MPS_FOR_POSE_CHANGE;
			if(!steps)
			{
				// будем считать что стоим где надо, просто переходим в
				// атаку
				return false;
			}
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
				steps, &field);
			// получим вектор доступных хексов
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	AssaultHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
				path,
				ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 4. Если в меня стреляли - будем преследовать
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkFixed: в меня стреляли - будем преследовать");
		// в меня или рядом со мной стреляли
		m_shooted = false;
		// выберем точку в радиусе 3 хексов от существа, которое стреляло
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_shoot_pnt,	3, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),m_target_pnt, path);
		// можно идти
		AIAPI::getInst()->setRun(m_entity);
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = AEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_turn = 0;
		return false;
	}

	// 5. если мне сообщили о враге - пойдем к нему
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkFixed: мне сообщили о враге");
		// сообщили
		m_enemy_id = 0;
		// выберем точку в радиусе 1 хекса от врага
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,	1, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// можно идти
		AIAPI::getInst()->setRun(m_entity);
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = AEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_turn = 0;
		return false;
	}

	// 6. если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 7. если вижу аптечку и она мне нужна - побегу за ней
	if(needAndSeeMedkit(&m_target_pnt))
	{
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		if(!path.empty())
		{
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVEIGNORE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_mode = AEM_MEDKITTAKE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 8. стало быть врагов не видно и если время оборачиваться - обернусь
	if(++m_turn < assault_turns)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkFixed: еще не время обернуться");
		// еще не время оборачиваться
		m_turn_state = TS_END;
		return false;
	}
	// пора обернуться
	AIAPI::getInst()->print(m_entity, "|A|thinkFixed: пора оборачиваться");
	m_turn = 0;
	m_turn_state = TS_INPROGRESS;
	m_mode = AEM_LOOKROUND;
	m_lookround_state = LS_FIRST;
	m_prev_dir = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	m_target_dir = norm_angle(m_prev_dir + 2*PId3);
	m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir, ActivityFactory::CT_ENEMY_LOOKROUND);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
	return false;
}

// обработка режима Lookround
bool AssaultEnemyNode::thinkLookround(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(assault_cure_dist);

	// 2. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,	&enemy,	assault_enemy_dist);
			// переход в состояние атаки
			m_mode = AEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел aim, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	SHT_AIMSHOT);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= MPS_FOR_ROTATE*4;
			// вычтем мувпоинты на приседание
			movpoints -= MPS_FOR_POSE_CHANGE;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// будем считать что стоим где надо, просто переходим в
				// атаку
				return false;
			}
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
			// получим вектор доступных хексов
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	AssaultHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 3. Если в меня стреляли - будем преследовать
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: в меня стреляли - будем преследовать");
		// в меня или рядом со мной стреляли
		m_shooted = false;
		// выберем точку в радиусе 3 хексов от существа, которое стреляло
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_shoot_pnt,	3, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),m_target_pnt, path);
		// можно идти
		AIAPI::getInst()->setRun(m_entity);
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = AEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. если мне сообщили о враге - пойдем к нему
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: мне сообщили о враге");
		// сообщили
		m_enemy_id = 0;
		// выберем точку в радиусе 1 хекса от врага
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,	1, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// можно идти
		AIAPI::getInst()->setRun(m_entity);
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = AEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. получим текущий угол, на который повернуто существо
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if(fabs(m_target_dir - cur_angle) < angle_eps)
	{
		// закончен очередной прием разворота
		switch(m_lookround_state)
		{
		case LS_FIRST : 
			{
				AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: закончен первый прием разворота");
				// начинаем второй прием разворота
				m_turn_state = TS_INPROGRESS;
				m_lookround_state = LS_SECOND;
				m_target_dir = norm_angle(m_prev_dir + 4*PId3);
				m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir,	ActivityFactory::CT_ENEMY_LOOKROUND);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
				break;
			}
		case LS_SECOND : 
			{
				AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: закончен второй прием разворота");
				// начинаем заключительный прием разворота
				m_turn_state = TS_INPROGRESS;
				m_lookround_state = LS_2BASE;
				m_target_dir = m_prev_dir;
				m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir,	ActivityFactory::CT_ENEMY_LOOKROUND);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
				break;
			}
		case LS_2BASE : 
			{
				AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: полностью закончил разворот");
				// переходим в базовое состояние
				m_mode = AEM_FIXED;
				m_turn_state = TS_END;
				break;
			}
		default : break;
		}
		return false;
	}

	// 6. если не хватило мувпоинтов - отдадим ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// продолжим действие
	m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir,ActivityFactory::CT_ENEMY_LOOKROUND);
	
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Attack
bool AssaultEnemyNode::thinkAttack(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(assault_cure_dist);

	// 2. если у нас нет оружия - перейдем к его поиску
	if( (!AIAPI::getInst()->takeBestWeapon(m_entity, AssaultWeaponComparator())) &&
		(!AIAPI::getInst()->takeGrenade(m_entity))
		)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkAttack: надо искать пушку!");
		m_mode = AEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::getInst()->setStandPose(m_entity);
		return false;
	}

	// 3. выберем врага
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// сам видит врага

		// если это начало хода или реакция - сбросим список игнорирования
		if(m_turn_state == TS_START)
		{
			m_ignored_enemies.clear();
		}
		// попробуем найти разность списков всех врагов и игнорированных врагов
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// выберем самого опасного врага из всех
			enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
				EnemyComparator(m_entity));
		}
		else
		{
			// выберем самого опасного врага из непроигнорированных
			enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));
		}
		m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
	}
	
	// 4. если нет врагов - переходим в состояние преследования
	if(!enemy)
	{
		// врагов нет - будем преследовать последнего
		AIAPI::getInst()->print(m_entity, "|A|thinkAttack: врагов нет - будем преследовать последнего");
		m_mode = AEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		AIAPI::getInst()->setStandPose(m_entity);
		return false;
	}
	// враги есть...

	// 5. если это начало нового хода или сработала реакция
	if(m_turn_state == TS_START)
	{
		// выбираем лучшую точку и идем в нее
		AIAPI::getInst()->print(m_entity, "|A|thinkAttack: выбираем лучшую точку и идем в нее");

		// выберем лучший хекс так, чтобы потом хватило мувпоинтов
		// на один выстрел aim, если таких хексов нет - считает
		// лучшим хекс, в котором стоит.
		
		m_turn_state = TS_INPROGRESS;
		// получим общее кол-во оставшихся мувпонитов
		int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
		// вычтем количество мувпоинтов необходимых для выстрела
		movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	SHT_AIMSHOT);
		// вычтем мувпоинты на поворот к врагу
		movpoints -= MPS_FOR_ROTATE*4;
		// вычтем мувпоинты на приседание
		movpoints -= MPS_FOR_POSE_CHANGE;
		int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
		if(!steps)
		{
			// будем считать что стоим где надо, просто переходим в атаку
			return false;
		}
		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// получим лучший хекс
		m_target_pnt = AIAPI::getInst()->getBestPoint(vec, AssaultHexComparator(m_entity, enemy));
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
		return false;
	}

	// 6. это продолжение хода

	// 6.1 если юниту не хватило мувпоинтов - он передает ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		AIAPI::getInst()->print(m_entity, "|A|thinkAttack: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6.2 если сработала реакция или увидел нового врага - имитируем
	//       начало хода
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)	)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkAttack: сработала реакция или увидел нового врага");
		m_activity_observer.clear();
		// имитируем начало хода
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 6.3 если нет линии огня - добавляем врага в список игнорирования
	// причем, если после этого все враги оказались в списке игнорирования
	// попробуем подойти к одному из них
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// закончились - закончим ход
		AIAPI::getInst()->print(m_entity, "|A|thinkAttack: нет линии огня - добавляем врага в список игнорирования");
		m_activity_observer.clear();
		// добавим в список игнорирования
		m_ignored_enemies.push_back(enemy);
		m_ignored_enemies.unique();
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// попробуем подойти поближе к врагу
			AIAPI::getInst()->print(m_entity, "|A|thinkAttack: подойдем поближе");
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, PathUtils::F_CALC_NEAR_PNTS);
			EntityList_t::iterator i = enemies.begin();
			// найдем существо, к которому можно подойти
			bool found = false;
			while(i != enemies.end())
			{
				// проверим, существуют ли точки рядом с существом
				if(PathUtils::GetInst()->IsNear(AIAPI::getInst()->getPtr(*i)))
				{
					// к юниту можно подойти
					found = true;
					m_target_pnt = PathUtils::GetInst()->GetNearPnt(AIAPI::getInst()->getPtr(*i)).m_pnt;
					break;
				}
				++i;
			}
			pnt_vec_t path;
			if(!found)
			{
				// подходящего врага нет - переместимся случайным образом
				pnt_vec_t base, vec;
				AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	patrol_enemy_noLOF_move,&base);
				AIAPI::getInst()->getReachableField(base, &vec, true);
				m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			}
			else
			{
				// есть враг, к которому можно подойти - подойдем на половину пути
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
				// отрежем половину пути
				path.erase(path.begin(), path.begin() + (path.size() / 2));
			}
			if(path.empty())
			{
				// передадим ход
				m_turn_state = TS_END;
				return false;
			}
			// можно идти
			AIAPI::getInst()->setWalk(m_entity);
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
			
			m_turn_state = TS_INPROGRESS;
			return false;
		} // if(diff_list.empty())
		// зациклимся для поиска других врагов
		m_turn_state = TS_INPROGRESS;
		return true;
	}

	// 6.4 Если не повернут к врагу - поворачивается в направлении где был враг
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// нужно поворачиваться к врагу
		m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),
			m_target_dir,
			ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 6.5 повернут к врагу - если сможем будем атаковать
	AIAPI::getInst()->print(m_entity, "|A|thinkAttack: будем стрелять, если это возможно!");
	// враг есть - выстрелим по нему если это возможно
	// присядем в вероятностью 60%
	if(rand() < 19662)
	{
		AIAPI::getInst()->setSitPose(m_entity);
	}
	else
	{
		AIAPI::getInst()->setStandPose(m_entity);
	}

	// если есть граната и можно атаковать несколько врагов сразу -
	// попробуем кинуть ее
	bool have_ready_gun = AIAPI::getInst()->takeBestWeapon(m_entity,
		AssaultWeaponComparator());
	if(AIAPI::getInst()->takeGrenade(m_entity) &&
		
		( (!have_ready_gun)  ||  
		AIAPI::getInst()->canAttackByGrenadeMultipleTarget(m_entity,
		enemy,
		enemies)
		)
		)
	{
		// граната в руках - попробуем кинуть ее
		std::string reason;
		if(AIAPI::getInst()->shoot(m_entity, enemy, &m_activity, &reason))
		{
			// начали бросок
			m_turn_state = TS_INPROGRESS;
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_NO_LOF);
			return false;
		}
		// кинуть не удалось
		std::string str = "|A|thinkAttack: нельзя кинуть гранату: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}

	// теперь попробуем стрельнуть из оружия
	if(AIAPI::getInst()->takeBestWeapon(m_entity, AssaultWeaponComparator()))
	{
		// оружие в руках
		AIAPI::getInst()->setShootType(m_entity, 30.0f, 15.0f);
		std::string reason;
		if(AIAPI::getInst()->shoot(m_entity, enemy, &m_activity, &reason))
		{
			// стрельба началась
			m_turn_state = TS_INPROGRESS;
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_NO_LOF);
			return false;
		}
		// стрельнуть не удалось
		std::string str = "|A|thinkAttack: стрелять нельзя: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	m_turn_state = TS_END;
	return false;
}

// обработка режима WeaponSearch
bool AssaultEnemyNode::thinkWeaponSearch(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(assault_cure_dist);

	// 2. проверим есть ли где-нить рядом оружие
	if(AIAPI::getInst()->getWeaponLocation(m_entity, &m_target_pnt))
	{
		// недалеко есть оружие - переход к его захвату
		AIAPI::getInst()->print(m_entity, "|A|thinkWeaponSearch: недалеко есть оружие - переход к его захвату!");
		pnt_vec_t base;
		pnt_vec_t vec;
		AIAPI::getInst()->getBaseField(m_target_pnt, 1, &base);
		AIAPI::getInst()->getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
			m_target_pnt, path);
		if(path.empty())
		{
			// к оружию нельзя подойти - отдадим ход
			m_turn_state = TS_END;
			return false;
		}
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		m_mode = AEM_WEAPONTAKE;
		return false;
	}

	// 3. если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 4. если дошел до заданной точки - выбирает новую
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkWeaponSearch: дошел до заданной точки - выбирает новую");
		// рассчитаем поле проходимости
		pnt_vec_t allhexes;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, &allhexes, 0);
		if(allhexes.empty())
		{
			// передаем ход - идти некуда
			AIAPI::getInst()->print(m_entity, "|A|thinkWeaponSearch: мне некуда ходить!!!");
			m_turn_state = TS_END;
			return false;
		}
		// получим случайную точку
		m_target_pnt = AIAPI::getInst()->getRandomPoint(allhexes);
	}

	// 5. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|A|thinkWeaponSearch: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. идем в заданную точку
	AIAPI::getInst()->print(m_entity, "|A|thinkWeaponSearch: идем в заданную точку");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
		0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
		path,
		ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима WeaponTake
bool AssaultEnemyNode::thinkWeaponTake(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(assault_cure_dist);

	// 2. проверим не подошли ли мы к оружию
	if(PickUpWeaponAndAmmo(m_target_pnt, AssaultWeaponComparator()))
	{
		m_mode = AEM_FIXED;
		return false;
	}

	// 3. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|A|thinkWeaponTake: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. проверим есть ли где-нить рядом оружие
	if(AIAPI::getInst()->getWeaponLocation(m_entity, &m_target_pnt))
	{
		// недалеко есть оружие - пойдем к нему
		pnt_vec_t base, vec, path;
		AIAPI::getInst()->getBaseField(m_target_pnt, 1, &base);
		AIAPI::getInst()->getReachableField(base, &vec, true);

		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),m_target_pnt, path);
		if(path.empty())
		{
			// к оружию нельзя подойти - возврат стационарному состоянию
			m_mode = AEM_FIXED;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// оружия нет - возврат к стационарному состоянию
	m_mode = AEM_FIXED;
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Pursuit
bool AssaultEnemyNode::thinkPursuit(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(assault_cure_dist);

	// 2. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|A|thinkPursuit: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,&enemy,assault_enemy_dist);
			// переход в состояние атаки
			AIAPI::getInst()->setWalk(m_entity);
			m_mode = AEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел aim, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	SHT_AIMSHOT);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= MPS_FOR_ROTATE*4;
			// вычтем мувпоинты на приседание
			movpoints -= MPS_FOR_POSE_CHANGE;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// будем считать что стоим где надо, просто переходим в
				// атаку
				return false;
			}

			pnt_vec_t field, vec, path;
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
			// получим вектор доступных хексов
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec, AssaultHexComparator(m_entity, enemy));
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 3. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|A|thinkPursuit: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. проверим не пришли ли мы в заданную точку
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// мы пришли
		// 4.1 проверим, повернут ли в точку, где был враг
		m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->convertPos2ToPos3(m_last_enemy_pnt) - getPosition()));
		float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
		if( fabs(m_target_dir - cur_angle) >= angle_eps )
		{
			// нужно поворачиваться к врагу
			m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity), m_target_dir,	ActivityFactory::CT_ENEMY_ROTATE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 4.2 повернут - переход стационарное состояние
		AIAPI::getInst()->print(m_entity, "|A|thinkPursuit: никого не догнал - переход стационарное состояние");
		// переходим в стационарное состояние
		m_mode = AEM_FIXED;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. идем в заданную точку
	AIAPI::getInst()->print(m_entity, "|A|thinkPursuit: идем в заданную точку");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	// можно идти
	AIAPI::getInst()->setRun(m_entity);
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима MedkitTake
bool AssaultEnemyNode::thinkMedkitTake(state_type* st)
{
	if(!MedkitTake(m_target_pnt,st))	m_mode = AEM_FIXED;
	return false;
}

// умри!
bool AssaultEnemyNode::die()
{
	m_mode = AEM_KILLED;
	return need2Delete();
}

// нужно ли удалять узел
bool AssaultEnemyNode::need2Delete() const
{
	if( (m_mode == AEM_KILLED) && !m_activity) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// узел для стационарной вражеской техники
//
/////////////////////////////////////////////////////////////////////////////

// конструктор - id существа
FixedTechEnemyNode::FixedTechEnemyNode(eid_t id) : m_mode(FTEM_BASE),
m_turn_state(TS_NONE), m_entity(id), m_activity(0), m_basepnt(0, 0),
m_basedir(0), m_target_dir(0), m_initialising(false)
{
	if(m_entity) OnSpawnMsg();
}

FixedTechEnemyNode::~FixedTechEnemyNode()
{
	delete m_activity;
}

DCTOR_IMP(FixedTechEnemyNode)

float FixedTechEnemyNode::Think(state_type* st)
{
	float complexity = 0.0f;
    //проиграть действие
    if(m_activity){

        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK)){
			m_activity->Detach(&m_activity_observer);
            delete m_activity;
            m_activity = 0;
        }
        return complexity;
    }

	// если не наш ход - на выход
	if(!st)
	{
		m_turn_state = TS_NONE;
		return complexity;
	}

	// если m_turn_state == TS_NONE, то это начало нашего хода
	if(m_turn_state == TS_NONE)
	{
		m_turn_state = TS_START;
		// начнем инициализацию
		PanicPlayer::GetInst()->Init(AIAPI::getInst()->getPtr(m_entity));
		m_initialising = true;
	}

	// проверим флаг инициализации
	if(m_initialising)
	{
		if(PanicPlayer::GetInst()->Execute())
		{
			// нужно продолжать процесс инициализации
			*st = ST_INCOMPLETE;
			return complexity;
		}
		// инициализация закончена
		m_initialising = false;
	}

    // юнит свободен - можно подумать
	bool flag = true;
    while(flag)
	{
        switch(m_mode)
		{
        case FTEM_BASE:    flag = thinkBase(st);         break;
		case FTEM_ATTACK:  flag = thinkAttack(st);       break;
		case FTEM_ROTATE2BASE:  flag = thinkRotate2Base(st);     break;
        }
    }

	if(m_turn_state == TS_END)
	{
		*st = ST_FINISHED;
		m_turn_state = TS_START;
		complexity = 1.0f;
	}
	else *st = ST_INCOMPLETE;
	return complexity;
}

void FixedTechEnemyNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());

		st << m_basepnt.x << m_basepnt.y << m_basedir << m_target_dir;
		return;
    }
	// читаемся
	int tmp;
	st >> tmp; m_mode = static_cast<FixedTechEnemyMode>(tmp);
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> m_entity;
	st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));

	st >> m_basepnt.x >> m_basepnt.y >> m_basedir >> m_target_dir;
	
	// то, что восстанавливается без чтения
	m_activity = 0;
	m_initialising = false;
}

// выдать собственную позицию
point3 FixedTechEnemyNode::getPosition() const
{
	return AIAPI::getInst()->getPos3(m_entity);
}

//обработка сообщения о расстановке
void FixedTechEnemyNode::OnSpawnMsg()
{
	m_basepnt = AIAPI::getInst()->getPos2(m_entity);
	m_basedir = norm_angle(AIAPI::getInst()->getAngle(m_entity));
}

// обработка режима Base
bool FixedTechEnemyNode::thinkBase(state_type* st)
{
	// 1. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|FT|thinkBase: я увидел врага!");
			// выбрал самого опасного врага, пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED, &enemy, fixed_tech_enemy_dist);
			// переход в состояние атаки
			m_mode = FTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 2. если моя подкоманда засекла врага - перейду в атаку
	enemies.clear();
	if(AIAPI::getInst()->getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies) )
	{
		// засекла - выберем самого опасного
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|FT|thinkBase: моя команда засекла врага!");
			// выбрал самого опасного врага, переход в состояние атаки
			m_mode = FTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 3. никого нет
	AIAPI::getInst()->print(m_entity, "|FT|thinkBase: никого нет");
	m_turn_state = TS_END;
	return false;
}

// обработка режима Attack
bool FixedTechEnemyNode::thinkAttack(state_type* st)
{
	// 1. выберем врага
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// сама видит врага
		// если это начало хода или реакция - сбросим список игнорирования
		if( (m_turn_state == TS_START) ||(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED)	)
		{
			m_ignored_enemies.clear();
		}
		// попробуем найти разность списков всех врагов и игнорированных врагов
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// выберем самого опасного врага из всех
			enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		}
		else
		{
			// выберем самого опасного врага из непроигнорированных
			enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list, EnemyComparator(m_entity));
		}
	}
	
	if(!enemy)
	{
		// самa врага не видит
		enemies.clear();
		if(AIAPI::getInst()->getEnemiesVisibleBySubteam(getSubteam()->getName(), &enemies) 	)
		{
			// зато видит ее подкоманда
			// если это начало хода или реакция - сбросим список игнорирования
			if( (m_turn_state == TS_START) ||(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) 	)
			{
				m_ignored_enemies.clear();
			}
			// попробуем найти разность списков всех врагов и игнорированных врагов
			EntityList_t diff_list;
			setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
			if(diff_list.empty())
			{
				// выберем самого опасного врага из всех
				enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies, EnemyComparator(m_entity));
			}
			else
			{
				// выберем самого опасного врага из непроигнорированных
				enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list, EnemyComparator(m_entity));
			}
		}
	}
	
	// 2. если нет врагов - разворот в базовое направление
	if(!enemy)
	{
		// больше нет врагов - разворот в базовое направление
		AIAPI::getInst()->print(m_entity, "|FT|thinkAttack: врагов нет - на базу");
		m_mode = FTEM_ROTATE2BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// враги есть...

	// 3. если технике не хватило мувпоинтов - онa передает ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		AIAPI::getInst()->print(m_entity, "|FT|thinkAttack: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. если нет линии огня - добавляем врага в список игнорирования
	// причем, если после этого все враги оказались в списке игнорирования
	// передаем ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// закончились - закончим ход
		AIAPI::getInst()->print(m_entity, "|FT|thinkAttack: нет линии огня - добавляем врага в список игнорирования");
		m_activity_observer.clear();
		// добавим в список игнорирования
		m_ignored_enemies.push_back(enemy);
		m_ignored_enemies.unique();
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// передаем ход
			m_turn_state = TS_END;
			return false;
		}
		// зациклимся для поиска других врагов
		m_turn_state = TS_INPROGRESS;
		return true;
	}


	// 5 Если не повернут к врагу - поворачивается в направлении где был враг
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// нужно поворачиваться к врагу
		m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir, 	ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 6. повернутa к врагу - если сможем будем атаковать
	AIAPI::getInst()->print(m_entity, "|FT|thinkAttack: будем стрелять, если это возможно!");
	// враг есть - выстрелим по нему если это возможно
	std::string reason;
	if(AIAPI::getInst()->shootByVehicle(m_entity, enemy, &m_activity, &reason))
	{
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
	}
	else
	{
		std::string str = "|FT|thinkAttack: стрелять нельзя: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

// обработка режима Rotate2Base
bool FixedTechEnemyNode::thinkRotate2Base(state_type* st)
{
	// 1. проверим развернута ли техника в базовом направлении
	float cur_angle = AIAPI::getInst()->getAngle(m_entity);
	if( fabs(m_basedir - cur_angle) < angle_eps )
	{
		// развернута - перейдем в базовое состояние
		AIAPI::getInst()->print(m_entity, "|FT|thinkRotate2Base: развернута");
		m_mode = FTEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 2. проверим мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		AIAPI::getInst()->print(m_entity, "|FT|thinkRotate2Base: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. разворот в базовое направление
	AIAPI::getInst()->print(m_entity, "|FT|thinkRotate2Base: разворачиваемся...");
	m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_basedir,	ActivityFactory::CT_ENEMY_ROTATE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// умри!
bool FixedTechEnemyNode::die()
{
	m_mode = FTEM_KILLED;
	return need2Delete();
}

// нужно ли удалять узел
bool FixedTechEnemyNode::need2Delete() const
{
	if( (m_mode == FTEM_KILLED) && !m_activity) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// узел для патрульной вражеской техники
//
/////////////////////////////////////////////////////////////////////////////

// конструктор - id существа
PatrolTechEnemyNode::PatrolTechEnemyNode(eid_t id) : m_mode(PTEM_PATROL),
m_turn_state(TS_NONE), m_entity(id), m_activity(0), m_enemy_id(0),
m_enemy_pnt(0, 0), m_shoot_pnt(0, 0), m_shooted(false), m_target_pnt(0, 0),
m_target_dir(0), m_last_enemy_pnt(0, 0), m_initialising(false)
{
    GameEvMessenger::GetInst()->Attach(this, EV_SHOOT);

	if(!m_entity) return;
	OnSpawnMsg();
}

PatrolTechEnemyNode::~PatrolTechEnemyNode()
{
    GameEvMessenger::GetInst()->Detach(this);
	delete m_activity;
}

DCTOR_IMP(PatrolTechEnemyNode)

float PatrolTechEnemyNode::Think(state_type* st)
{
	float complexity = 0.0f;
    //проиграть действие
    if(m_activity){

        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK)){
			m_activity->Detach(&m_activity_observer);
            delete m_activity;
            m_activity = 0;
        }
        return complexity;
    }

	// если не наш ход - на выход
	if(!st)
	{
		m_turn_state = TS_NONE;
		return complexity;
	}

	// если m_turn_state == TS_NONE, то это начало нашего хода
	if(m_turn_state == TS_NONE)
	{
		m_turn_state = TS_START;
		// начнем инициализацию
		PanicPlayer::GetInst()->Init(AIAPI::getInst()->getPtr(m_entity));
		m_initialising = true;
	}

	// проверим флаг инициализации
	if(m_initialising)
	{
		if(PanicPlayer::GetInst()->Execute())
		{
			// нужно продолжать процесс инициализации
			*st = ST_INCOMPLETE;
			return complexity;
		}
		// инициализация закончена
		m_initialising = false;
	}

    // юнит свободен - можно подумать
	bool flag = true;
    while(flag)
	{
        switch(m_mode)
		{
        case PTEM_PATROL:
            flag = thinkPatrol(st);
            break;
        
		case PTEM_ATTACK:
            flag = thinkAttack(st);
            break;
        
		case PTEM_PURSUIT:
            flag = thinkPursuit(st);
            break;
        }
    }

	if(m_turn_state == TS_END)
	{
		*st = ST_FINISHED;
		m_turn_state = TS_START;
		complexity = 1.0f;
	}
	else *st = ST_INCOMPLETE;
	return complexity;
}

void PatrolTechEnemyNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());
		st << m_enemy_id;
		st << m_enemy_pnt.x;
		st << m_enemy_pnt.y;
		st << m_shoot_pnt.x;
		st << m_shoot_pnt.y;
		st << m_shooted;
		st << m_target_pnt.x;
		st << m_target_pnt.y;
		st << m_target_dir;
		st << m_last_enemy_pnt.x;
		st << m_last_enemy_pnt.y;

		return;
    }
	// читаемся
	int tmp;
	st >> tmp; m_mode = static_cast<PatrolTechEnemyMode>(tmp);
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> m_entity;
	st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));
	st >> m_enemy_id;
	st >> m_enemy_pnt.x;
	st >> m_enemy_pnt.y;
	st >> m_shoot_pnt.x;
	st >> m_shoot_pnt.y;
	st >> m_shooted;
	st >> m_target_pnt.x;
	st >> m_target_pnt.y;
	st >> m_target_dir;
	st >> m_last_enemy_pnt.x;
	st >> m_last_enemy_pnt.y;
	
	// то, что восстанавливается без чтения
	m_activity = 0;
	m_initialising = false;
}

// обработка внутрикомандных сообщений
void PatrolTechEnemyNode::recieveMessage(MessageType type, void * data)
{
	eid_t id = *static_cast<eid_t*>(data);
	if(type == MT_ENEMYSPOTTED)
	{
		if(m_enemy_id)
		{
			// уже сообщали о враге
			// проверим кто из врагов ближе
			float old_dist = dist(AIAPI::getInst()->getPos2(m_entity),
				m_enemy_pnt);
			float new_dist = dist(AIAPI::getInst()->getPos2(m_entity),
				AIAPI::getInst()->getPos2(id));
			if(new_dist < old_dist)
			{
				// сменим врага
				m_enemy_id = id;
				m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
			}
			return;
		}
		// это первый враг, о котором сообщили
		m_enemy_id = id;
		m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
		return;
	}
}

// выдать собственную позицию
point3 PatrolTechEnemyNode::getPosition() const
{
	return AIAPI::getInst()->getPos3(m_entity);
}

void PatrolTechEnemyNode::Update(subject_t subj, event_t event, info_t ptr)
{
    switch(event){
    case EV_SHOOT:
        {
            shoot_info* info = static_cast<shoot_info*>(ptr);
            OnShootEv(info->m_actor, info->m_victim, info->m_point);
        }
        break;
    }
}

// обработка сообщения об выстреле или попадании
void PatrolTechEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// не будем реагировать в случае,
	// если техника находится в определенных состояниях
	if(m_mode == PTEM_ATTACK) return;

	if(AIAPI::getInst()->getPtr(m_entity) == who) return; // это я сам натворил

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (AIAPI::getInst()->getPtr(m_entity) == victim))
	{
		// будем реагировать
		m_shoot_pnt = AIAPI::getInst()->getPos2(who);
		m_shooted = true;
	}
}

// обработка режима Patrol
bool PatrolTechEnemyNode::thinkPatrol(state_type* st)
{
	// 1. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: я увидела врага!");
			// выбралa самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_tech_enemy_dist);
			// переход в состояние атаки
			m_mode = PTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
				/*на этот параметр забьет*/SHT_AIMSHOT);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// будем считать что стоим где надо, просто переходим в
				// атаку
				return false;
			}
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
				steps, &field);
			// получим вектор доступных хексов
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,
				WalkingTechHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
				m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
				path,
				ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 2. Если в меня стреляли - будем преследовать
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: в меня стреляли - будем преследовать");
		// в меня или рядом со мной стреляли
		m_shooted = false;
		// выберем точку в радиусе 3 хексов от существа, которое стреляло
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_shoot_pnt,
			3, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
			m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 3. если мне сообщили о враге - пойдем к нему
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: мне сообщили о враге");
		// сообщили
		m_enemy_id = 0;
		// выберем точку в радиусе 1 хекса от врага
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,
			1, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
			m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. если дошла до заданной точки - выбирает новую
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: дошла до заданной точки - выбирает новую");
		// рассчитаем поле проходимости
		pnt_vec_t allhexes;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, &allhexes, 0);
		if(allhexes.empty())
		{
			// передаем ход - идти некуда
			AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: мне некуда ходить!!!");
			m_turn_state = TS_END;
			return false;
		}
		// получим случайную точку
		m_target_pnt = AIAPI::getInst()->getRandomPoint(allhexes);
	}

	// 5. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. идем в заданную точку
	AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: идем в заданную точку");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
		0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
		path,
		ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Attack
bool PatrolTechEnemyNode::thinkAttack(state_type* st)
{
	// 1. если у техники нет оружия - перейдем к преследованию
	if(!AIAPI::getInst()->haveTechWeapon(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: нет оружия - будем преследовать");
		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		return false;
	}

	// 2. выберем врага
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// сам видит врага

		// если это начало хода или реакция - сбросим список игнорирования
		if(m_turn_state == TS_START)
		{
			m_ignored_enemies.clear();
		}
		// попробуем найти разность списков всех врагов и игнорированных врагов
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// выберем самого опасного врага из всех
			enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
				EnemyComparator(m_entity));
		}
		else
		{
			// выберем самого опасного врага из непроигнорированных
			enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list,
				EnemyComparator(m_entity));
		}
		m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
	}
	
	// 3. если нет врагов - переходим в состояние преследования
	if(!enemy)
	{
		// врагов нет - будем преследовать последнего
		AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: врагов нет - будем преследовать последнего");
		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		return false;
	}
	// враги есть...

	// 4. если это начало нового хода или сработала реакция
	if(m_turn_state == TS_START)
	{
		// выбираем лучшую точку и идем в нее
		AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: выбираем лучшую точку и идем в нее");

		// выберем лучший хекс так, чтобы потом хватило мувпоинтов
		// на один выстрел, если таких хексов нет - считает
		// лучшим хекс, в котором стоит.
		
		m_turn_state = TS_INPROGRESS;
		// получим общее кол-во оставшихся мувпонитов
		int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
		// вычтем количество мувпоинтов необходимых для выстрела
		movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
			/*забьет на этот параметр*/SHT_AIMSHOT);
		// вычтем мувпоинты на поворот к врагу
		movpoints -= MPS_FOR_ROTATE*4;
		int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
		if(!steps)
		{
			// будем считать что стоим где надо, просто переходим в
			// атаку
			return false;
		}
		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
			steps, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec);
		// получим лучший хекс
		m_target_pnt = AIAPI::getInst()->getBestPoint(vec,
			WalkingTechHexComparator(m_entity, enemy));
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
			m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_ENEMY_REACTED);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_ENEMY_SPOTTED);
		return false;
	}

	// 5. это продолжение хода

	// 5.1 если не хватило мувпоинтов - передает ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 5.2 если сработала реакция или увидел нового врага - имитируем
	//       начало хода
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)
		)
	{
		AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: сработала реакция или увидел нового врага");
		m_activity_observer.clear();
		// имитируем начало хода
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 5.3 если нет линии огня - добавляем врага в список игнорирования
	// причем, если после этого все враги оказались в списке игнорирования
	// попробуем подойти к одному из них
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// закончились - закончим ход
		AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: нет линии огня - добавляем врага в список игнорирования");
		m_activity_observer.clear();
		// добавим в список игнорирования
		m_ignored_enemies.push_back(enemy);
		m_ignored_enemies.unique();
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// попробуем подойти поближе к врагу
			AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: подойдем поближе");
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
				0, 0, PathUtils::F_CALC_NEAR_PNTS);
			EntityList_t::iterator i = enemies.begin();
			// найдем существо, к которому можно подойти
			bool found = false;
			while(i != enemies.end())
			{
				// проверим, существуют ли точки рядом с существом
				if(PathUtils::GetInst()->IsNear(AIAPI::getInst()->getPtr(*i)))
				{
					// к юниту можно подойти
					found = true;
					m_target_pnt = PathUtils::GetInst()->GetNearPnt(AIAPI::getInst()->getPtr(*i)).m_pnt;
					break;
				}
				++i;
			}
			pnt_vec_t path;
			if(!found)
			{
				// подходящего врага нет - переместимся случайным образом
				pnt_vec_t base, vec;
				AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
					patrol_enemy_noLOF_move,
					&base);
				AIAPI::getInst()->getReachableField(base, &vec, true);
				m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
					m_target_pnt, path);
			}
			else
			{
				// есть враг, к которому можно подойти - подойдем на половину пути
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
					m_target_pnt, path);
				// отрежем половину пути
				path.erase(path.begin(), path.begin() + (path.size() / 2));
			}
			if(path.empty())
			{
				// передадим ход
				m_turn_state = TS_END;
				return false;
			}
			// можно идти
			AIAPI::getInst()->setWalk(m_entity);
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
				path,
				ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_ENEMY_SPOTTED);
			
			m_turn_state = TS_INPROGRESS;
			return false;
		} // if(diff_list.empty())
		// зациклимся для поиска других врагов
		m_turn_state = TS_INPROGRESS;
		return true;
	}

	// 5.4 Если не повернутa к врагу - поворачивается в направлении где был враг
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// нужно поворачиваться к врагу
		m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),
			m_target_dir,
			ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 5.5 повернутa к врагу - если сможем будем атаковать
	AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: будем стрелять, если это возможно!");
	// враг есть - выстрелим по нему если это возможно
	std::string reason;
	if(AIAPI::getInst()->shootByVehicle(m_entity, enemy, &m_activity, &reason))
	{
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_NO_LOF);
	}
	else
	{
		std::string str = "|PT|thinkAttack: стрелять нельзя: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

// обработка режима Pursuit
bool PatrolTechEnemyNode::thinkPursuit(state_type* st)
{
	// 1. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::getInst()->print(m_entity, "|PT|thinkPursuit: я увиделa врага!");
			// выбралa самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_tech_enemy_dist);
			// переход в состояние атаки
			m_mode = PTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
				/*забьет на этот параметр*/SHT_AIMSHOT);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// будем считать что стоим где надо, просто переходим в
				// атаку
				return false;
			}
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
				steps, &field);
			// получим вектор доступных хексов
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,
				WalkingTechHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
				m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
				path,
				ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 2. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|PT|thinkPursuit: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. проверим не пришли ли мы в заданную точку
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// мы пришли
		// 3.1 проверим, повернутa ли в точку, где был враг
		m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->convertPos2ToPos3(m_last_enemy_pnt) - getPosition()));
		float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
		if( fabs(m_target_dir - cur_angle) >= angle_eps )
		{
			// нужно поворачиваться к врагу
			m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),
				m_target_dir,
				ActivityFactory::CT_ENEMY_ROTATE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 3.2 повернутa - переход в состояние патрулирования
		AIAPI::getInst()->print(m_entity, "|PT|thinkPursuit: никого не догналa - будем патрулировать");
		// переходим в состояние патрулирования
		m_mode = PTEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. идем в заданную точку
	AIAPI::getInst()->print(m_entity, "|PT|thinkPursuit: идем в заданную точку");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
		0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
		path,
		ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// умри!
bool PatrolTechEnemyNode::die()
{
	m_mode = PTEM_KILLED;
	return need2Delete();
}

// нужно ли удалять узел
bool PatrolTechEnemyNode::need2Delete() const
{
	if( (m_mode == PTEM_KILLED) && !m_activity) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// узел для штурмовой вражеской техники
//
/////////////////////////////////////////////////////////////////////////////

// конструктор - id существа
AssaultTechEnemyNode::AssaultTechEnemyNode(eid_t id) : m_mode(ATEM_PATROL),
m_turn_state(TS_NONE), m_entity(id), m_activity(0), m_enemy_id(0),
m_enemy_pnt(0, 0), m_shoot_pnt(0, 0), m_shooted(false), m_target_pnt(0, 0),
m_target_dir(0), m_last_enemy_pnt(0, 0), m_initialising(false)
{
    GameEvMessenger::GetInst()->Attach(this, EV_SHOOT);
	if(!m_entity) return;
	OnSpawnMsg();
}

AssaultTechEnemyNode::~AssaultTechEnemyNode()
{
    GameEvMessenger::GetInst()->Detach(this);
	delete m_activity;
}

DCTOR_IMP(AssaultTechEnemyNode)

float AssaultTechEnemyNode::Think(state_type* st)
{
	float complexity = 0.0f;
    //проиграть действие
    if(m_activity){

        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK)){
			m_activity->Detach(&m_activity_observer);
            delete m_activity;
            m_activity = 0;
        }
        return complexity;
    }

	// если не наш ход - на выход
	if(!st)
	{
		m_turn_state = TS_NONE;
		return complexity;
	}

	// если m_turn_state == TS_NONE, то это начало нашего хода
	if(m_turn_state == TS_NONE)
	{
		m_turn_state = TS_START;
		// начнем инициализацию
		PanicPlayer::GetInst()->Init(AIAPI::getInst()->getPtr(m_entity));
		m_initialising = true;
	}

	// проверим флаг инициализации
	if(m_initialising)
	{
		if(PanicPlayer::GetInst()->Execute())
		{
			// нужно продолжать процесс инициализации
			*st = ST_INCOMPLETE;
			return complexity;
		}
		// инициализация закончена
		m_initialising = false;
	}

    // юнит свободен - можно подумать
	bool flag = true;
    while(flag)
	{
        switch(m_mode)
		{
        case ATEM_PATROL:
            flag = thinkPatrol(st);
            break;
        
		case ATEM_ATTACK:
            flag = thinkAttack(st);
            break;
        
		case ATEM_PURSUIT:
            flag = thinkPursuit(st);
            break;
        }
    }

	if(m_turn_state == TS_END)
	{
		*st = ST_FINISHED;
		m_turn_state = TS_START;
		complexity = 1.0f;
	}
	else *st = ST_INCOMPLETE;
	return complexity;
}

void AssaultTechEnemyNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());
		st << m_enemy_id;
		st << m_enemy_pnt.x;
		st << m_enemy_pnt.y;
		st << m_shoot_pnt.x;
		st << m_shoot_pnt.y;
		st << m_shooted;
		st << m_target_pnt.x;
		st << m_target_pnt.y;
		st << m_target_dir;
		st << m_last_enemy_pnt.x;
		st << m_last_enemy_pnt.y;

		return;
    }
	// читаемся
	int tmp;
	st >> tmp; m_mode = static_cast<AssaultTechEnemyMode>(tmp);
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> m_entity;
	st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));
	st >> m_enemy_id;
	st >> m_enemy_pnt.x;
	st >> m_enemy_pnt.y;
	st >> m_shoot_pnt.x;
	st >> m_shoot_pnt.y;
	st >> m_shooted;
	st >> m_target_pnt.x;
	st >> m_target_pnt.y;
	st >> m_target_dir;
	st >> m_last_enemy_pnt.x;
	st >> m_last_enemy_pnt.y;
	
	// то, что восстанавливается без чтения
	m_activity = 0;
	m_initialising = false;
}

// обработка внутрикомандных сообщений
void AssaultTechEnemyNode::recieveMessage(MessageType type, void * data)
{
	eid_t id = *static_cast<eid_t*>(data);
	if(type == MT_ENEMYSPOTTED)
	{
		if(m_enemy_id)
		{
			// уже сообщали о враге
			// проверим кто из врагов ближе
			float old_dist = dist(AIAPI::getInst()->getPos2(m_entity), m_enemy_pnt);
			float new_dist = dist(AIAPI::getInst()->getPos2(m_entity), AIAPI::getInst()->getPos2(id));
			if(new_dist < old_dist)
			{
				// сменим врага
				m_enemy_id = id;
				m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
			}
			return;
		}
		// это первый враг, о котором сообщили
		m_enemy_id = id;
		m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
		return;
	}
}

// выдать собственную позицию
point3 AssaultTechEnemyNode::getPosition() const
{
	return AIAPI::getInst()->getPos3(m_entity);
}

void AssaultTechEnemyNode::Update(subject_t subj, event_t event, info_t ptr)
{
    switch(event){
    case EV_SHOOT:
        {
            shoot_info* info = static_cast<shoot_info*>(ptr);
            OnShootEv(info->m_actor, info->m_victim, info->m_point);
        }
        break;
    }
}

// обработка сообщения об выстреле или попадании
void AssaultTechEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// не будем реагировать в случае,
	// если техника находится в определенных состояниях
	if(m_mode == ATEM_ATTACK) return;

	BaseEntity* be = AIAPI::getInst()->getPtr(m_entity);
	if( be && (be == who) ) return; // это я сам натворил

	float dist = (getPosition() - where).Length();
	if((dist < alarm_dist) || (be == victim) )
	{
		// будем реагировать
		m_shoot_pnt = AIAPI::getInst()->getPos2(who);
		m_shooted = true;
	}
}

// обработка режима Patrol
bool AssaultTechEnemyNode::thinkPatrol(state_type* st)
{
	// 1. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: я увидела врага!");
			// выбралa самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED, &enemy, assault_tech_enemy_dist);
			// переход в состояние атаки
			m_mode = ATEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	/*на этот параметр забьет*/SHT_AIMSHOT);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// будем считать что стоим где надо, просто переходим в, атаку
				return false;
			}
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			pnt_vec_t field, vec, path;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
			// получим вектор доступных хексов
			AIAPI::getInst()->getReachableField(field, &vec);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec, WalkingTechHexComparator(m_entity, enemy));
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 2. Если в меня стреляли - будем преследовать
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: в меня стреляли - будем преследовать");
		// в меня или рядом со мной стреляли
		m_shooted = false;
		// выберем точку в радиусе 3 хексов от существа, которое стреляло
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field, vec, path;
		AIAPI::getInst()->getBaseField(m_shoot_pnt,	3, &field);
		// получим вектор доступных хексов
		AIAPI::getInst()->getReachableField(field, &vec);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = ATEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 3. если мне сообщили о враге - пойдем к нему
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: мне сообщили о враге");
		// сообщили
		m_enemy_id = 0;
		// выберем точку в радиусе 1 хекса от врага
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,	1, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = ATEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. если дошла до заданной точки - выбирает новую
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: дошла до заданной точки - выбирает новую");
		// рассчитаем поле проходимости
		pnt_vec_t allhexes;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, &allhexes, 0);
		if(allhexes.empty())
		{
			// передаем ход - идти некуда
			AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: мне некуда ходить!!!");
			m_turn_state = TS_END;
			return false;
		}
		// получим случайную точку
		m_target_pnt = AIAPI::getInst()->getRandomPoint(allhexes);
	}

	// 5. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. идем в заданную точку
	AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: идем в заданную точку");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Attack
bool AssaultTechEnemyNode::thinkAttack(state_type* st)
{
	// 1. если у техники нет оружия - перейдем к преследованию
	if(!AIAPI::getInst()->haveTechWeapon(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: нет оружия - будем преследовать");
		m_mode = ATEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		return false;
	}

	// 2. выберем врага
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// самa видит врага

		// если это начало хода или реакция - сбросим список игнорирования
		if(m_turn_state == TS_START)
			m_ignored_enemies.clear();
		// попробуем найти разность списков всех врагов и игнорированных врагов
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// выберем самого опасного врага из всех
			enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies, EnemyComparator(m_entity));
		}
		else
		{
			// выберем самого опасного врага из непроигнорированных
			enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list, EnemyComparator(m_entity));
		}
		m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
	}
	
	// 3. если нет врагов - переходим в состояние преследования
	if(!enemy)
	{
		// врагов нет - будем преследовать последнего
		AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: врагов нет - будем преследовать последнего");
		m_mode = ATEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		return false;
	}
	// враги есть...

	// 3. если это начало нового хода или сработала реакция
	if(m_turn_state == TS_START)
	{
		m_activity_observer.clear();
		// выбираем лучшую точку и идем в нее
		AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: выбираем лучшую точку и идем в нее");

		// выберем лучший хекс так, чтобы потом хватило мувпоинтов
		// на один выстрел, если таких хексов нет - считает
		// лучшим хекс, в котором стоит.
		
		m_turn_state = TS_INPROGRESS;
		// получим общее кол-во оставшихся мувпонитов
		int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
		// вычтем количество мувпоинтов необходимых для выстрела
		movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	/*забьет на этот параметр*/SHT_AIMSHOT);
		// вычтем мувпоинты на поворот к врагу
		movpoints -= MPS_FOR_ROTATE*4;
		int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
		if(!steps)
		{
			// будем считать что стоим где надо, просто переходим в, атаку
			return false;
		}
		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field, vec, path;
		AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
		// получим вектор доступных хексов
		AIAPI::getInst()->getReachableField(field, &vec);
		// получим лучший хекс
		m_target_pnt = AIAPI::getInst()->getBestPoint(vec, WalkingTechHexComparator(m_entity, enemy));
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
		return false;
	}

	// 5. это продолжение хода

	// 5.1 если не хватило мувпоинтов - передает ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 5.2 если сработала реакция или увидел нового врага - имитируем
	//       начало хода
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)
		)
	{
		AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: сработала реакция или увидел нового врага");
		m_activity_observer.clear();
		// имитируем начало хода
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 5.3 если нет линии огня - добавляем врага в список игнорирования
	// причем, если после этого все враги оказались в списке игнорирования
	// попробуем подойти к одному из них
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// закончились - закончим ход
		AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: нет линии огня");
		m_activity_observer.clear();
		// добавим в список игнорирования
		m_ignored_enemies.push_back(enemy);
		m_ignored_enemies.unique();
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// попробуем подойти поближе к врагу
			AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: подойдем поближе");
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, PathUtils::F_CALC_NEAR_PNTS);
			EntityList_t::iterator i = enemies.begin();
			// найдем существо, к которому можно подойти
			bool found = false;
			while(i != enemies.end())
			{
				// проверим, существуют ли точки рядом с существом
				if(PathUtils::GetInst()->IsNear(AIAPI::getInst()->getPtr(*i)))
				{
					// к юниту можно подойти
					found = true;
					m_target_pnt = PathUtils::GetInst()->GetNearPnt(AIAPI::getInst()->getPtr(*i)).m_pnt;
					break;
				}
				++i;
			}
			pnt_vec_t path;
			if(!found)
			{
				// подходящего врага нет - переместимся случайным образом
				pnt_vec_t base, vec;
				AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	patrol_enemy_noLOF_move,&base);
				AIAPI::getInst()->getReachableField(base, &vec, true);
				m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
			}
			else
			{
				// есть враг, к которому можно подойти - подойдем на половину пути
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
				// отрежем половину пути
				path.erase(path.begin(), path.begin() + (path.size() / 2));
			}
			if(path.empty())
			{
				// передадим ход
				m_turn_state = TS_END;
				return false;
			}
			// можно идти
			AIAPI::getInst()->setWalk(m_entity);
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
			
			m_turn_state = TS_INPROGRESS;
			return false;
		} // if(diff_list.empty())
		// зациклимся для поиска других врагов
		m_turn_state = TS_INPROGRESS;
		return true;
	}

	// 5.4 Если не повернутa к врагу - поворачивается в направлении где был враг
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// нужно поворачиваться к врагу
		m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity), m_target_dir,	ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 5.5 повернутa к врагу - если сможем будем атаковать
	AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: будем стрелять, если это возможно!");
	// враг есть - выстрелим по нему если это возможно
	std::string reason;
	if(AIAPI::getInst()->shootByVehicle(m_entity, enemy, &m_activity, &reason))
	{
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
	}
	else
	{
		std::string str = "|AT|thinkAttack: стрелять нельзя: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

// обработка режима Pursuit
bool AssaultTechEnemyNode::thinkPursuit(state_type* st)
{
	// 1. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies, EnemyComparator(m_entity)))
		{
			AIAPI::getInst()->print(m_entity, "|AT|thinkPursuit: я увиделa врага!");
			// выбралa самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED, &enemy, assault_tech_enemy_dist);
			// переход в состояние атаки
			m_mode = ATEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
				/*забьет на этот параметр*/SHT_AIMSHOT);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// будем считать что стоим где надо, просто переходим в
				// атаку
				return false;
			}
			// рассчитаем поле проходимости
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			pnt_vec_t field, vec, path;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
			// получим вектор доступных хексов
			AIAPI::getInst()->getReachableField(field, &vec);
			// получим лучший хекс
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec, WalkingTechHexComparator(m_entity, enemy));
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 2. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::getInst()->print(m_entity, "|AT|thinkPursuit: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. проверим не пришли ли мы в заданную точку
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// мы пришли
		// 3.1 проверим, повернутa ли в точку, где был враг
		m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->convertPos2ToPos3(m_last_enemy_pnt) - getPosition()));
		float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
		if( fabs(m_target_dir - cur_angle) >= angle_eps )
		{
			// нужно поворачиваться к врагу
			m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir,	ActivityFactory::CT_ENEMY_ROTATE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 3.2 повернутa - переход в состояние патрулирования
		AIAPI::getInst()->print(m_entity, "|AT|thinkPursuit: никого не догналa - будем патрулировать");
		// переходим в состояние патрулирования
		m_mode = ATEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. идем в заданную точку
	AIAPI::getInst()->print(m_entity, "|AT|thinkPursuit: идем в заданную точку");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// умри!
bool AssaultTechEnemyNode::die()
{
	m_mode = ATEM_KILLED;
	return need2Delete();
}

// нужно ли удалять узел
bool AssaultTechEnemyNode::need2Delete() const
{
	if( (m_mode == ATEM_KILLED) && !m_activity) return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////////
//
// пространство имен без имени - для функций-утилит этого файла
//
//////////////////////////////////////////////////////////////////////////////
namespace
{
	// нормирование угла от нуля до двух пи
	float norm_angle(float angle)
	{
		float fl = fmod(angle, PIm2);
		if(fl < 0) fl += PIm2;
		return fl;
	}
	// получить в виде списка разность (из теории множеств) между двумя двумя
	// другими списками eid_t
	void setDifferenceBetweenLists(const std::list<eid_t>& lista, const std::list<eid_t>& listb, std::list<eid_t>* outlist)
	{
		(*outlist).clear();
		std::list<eid_t>::const_iterator i = lista.begin();
		std::list<eid_t>::const_iterator end = lista.end();
		while(i != end)
		{
			std::list<eid_t>::const_iterator found = std::find(listb.begin(),	listb.end(), *i );
			if(found == listb.end())
			{
				// то есть элемент есть в первом списке и отсутствует во втором
				(*outlist).push_back(*i);
			}
			++i;
		}
	}
};
