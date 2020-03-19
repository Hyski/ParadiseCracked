/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ��������� ������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#pragma warning(disable:4786)
// precompiled header
#include "logicdefs.h"

#include "EnemyAI.h"

#include "AIAPI.h"
#include "AIUtils.h"
#include "PathUtils.h"

// ������������ ���� ��� ����� - ��� �������-������ ����� �����
namespace
{
	// �������� ��������� ����� ��������
	const float angle_eps = TORAD(6);
	// ����������, �� ������� ���� ��������� �� ������ � ��������
	const float alarm_dist = 5.0f;
	// ����������, �� ������� ���������� ������ ����� �������
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
	// ������������ ���� �� ���� �� ���� ��
	float norm_angle(float angle);
	// �������� � ���� ������ �������� (�� ������ ��������) ����� ����� �����
	// ������� �������� eid_t
	void setDifferenceBetweenLists(const std::list<eid_t>& lista, const std::list<eid_t>& listb, std::list<eid_t>* outlist);
};

/////////////////////////////////////////////////////////////////////////////
//
// ����� - ���� ���������� ���� ������
//
/////////////////////////////////////////////////////////////////////////////
AIEnemyNode::AIEnemyNode(): m_subteam(0)
{
}

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ���������� �����������
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

//� ���� ������� ���� ������ ��������
float SubteamNode::Think(state_type* st)
{
    node_lst_t::iterator itor = m_nodes.begin();
    while(itor != m_nodes.end())
	{
		// ������� ������� ����
		if((*itor)->need2Delete())
		{
			// ������ ���� ����
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

    //���� ������ ��� ���� �����
    if(m_cur == m_nodes.end())
	{     
        *st = ST_FINISHED;
        return m_complexity;
    }

    //����� �������� �������� ����
    float tmp_complexity = (*m_cur)->Think(st);

    //���� ���� �������� �������� � ����������
    if(*st == ST_FINISHED)
	{
		m_complexity += (*m_cur)->getComplexity();
		tmp_complexity = 0.0f;
        ++m_cur;
	}
    
    *st = ST_INCOMPLETE;
	return m_complexity + tmp_complexity;
}
  
//����������/������������� ����������
void SubteamNode::MakeSaveLoad(SavSlot& st)
{
    //��������/����������� ��������� ����������
	if(st.IsSaving())
	{
		// �����������
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
	// ��������
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

//��������� ������ ����
void SubteamNode::Update(subject_t subj, event_t event, info_t info)
{
    if(static_cast<turn_info*>(info)->m_player == PT_ENEMY)
        m_cur = m_nodes.begin();
	m_complexity = 0.0f;
}

// ��������� ��������� ���� ������ ������� (����� ����)
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

// ������ ������ ��������� ����
float SubteamNode::getComplexity() const
{
	float complexity = 0.0f;
	// �������� ����� ����� ���������� ���������
	node_lst_t::const_iterator i = m_nodes.begin();
	node_lst_t::const_iterator end = m_nodes.end();
	while(i != end)
	{
		complexity += (*i)->getComplexity();
		++i;
	}
	return complexity;
}

// ������ ����������� �� �������� ����� � �������� ID � ��������� ��
// ������� ���� ����� �� ������ ������
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
			// ����!
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
	// �������� ����� ������� ���� �����
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
    if(m_activity)//��������� ��������
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
	if(!st)// ���� �� ��� ��� - �� �����
	{
		m_turn_state = TS_NONE;
		return true;
	}
	if(m_turn_state == TS_NONE)	// ���� m_turn_state == TS_NONE, �� ��� ������ ������ ����
	{
		m_turn_state = TS_START;
		// ������ �������������
		PanicPlayer::GetInst()->Init(AIAPI::getInst()->getPtr(m_entity));
		m_initialising = true;
	}
	
	if(m_initialising)// �������� ���� �������������
	{
		if(PanicPlayer::GetInst()->Execute())
		{	// ����� ���������� ������� �������������
			*st = ST_INCOMPLETE;
			return true;
		}
		// ������������� ���������
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
// ���������, ����� �� ��� ������� � ���� �� � ��
bool CommonEnemyNode::needAndSeeMedkit(ipnt2_t* target)
{
	// �������� ���������� ������� � �����
	if(AIAPI::getInst()->getThingCount(m_entity, TT_MEDIKIT) >= 2)
		return false;
	// ��������� ���� �� �� ���� � ����� �������
	if(AIAPI::getInst()->getThingLocation(m_entity, target, TT_MEDIKIT))
		return true;
	return false;
}

bool CommonEnemyNode::SelfCure()
{
	if(AIAPI::getInst()->getHealthPercent(m_entity) < 100.0f)
	{
		AIAPI::getInst()->print(m_entity, " �������� �����������!");
		if(AIAPI::getInst()->haveMedikit(m_entity))
		{
			// � ���� ���� �������
			// ������ �� � ����
			AIAPI::getInst()->takeMedikit(m_entity);
			// ������ ����
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
		// �� ������� � ����������� ������ - ������� ��� ������
		AIAPI::getInst()->print(m_entity, "�� ������� � ����������� ������ - ������� ��� ������");
		// ������� ��� �������
		AIAPI::getInst()->pickupAllNearAmmo(m_entity);
		// �������� ���� �� ������ � ��� ���������� ������
		if( (!AIAPI::getInst()->takeGrenade(m_entity)) &&
			(!AIAPI::getInst()->takeBestWeapon(m_entity, comparator))	)
		{
			// ���������� ����� ��� ��� ��� ����� ��������� ����� �� �����,
			// ���� ��� �� �������� ��� �� ������ ����������
			while(AIAPI::getInst()->pickupNearWeapon(m_entity))
			{
				if((AIAPI::getInst()->takeBestWeapon(m_entity, comparator)) ||
					(AIAPI::getInst()->takeGrenade(m_entity)))
					break;
			}
			// ������� ��� ������
			AIAPI::getInst()->dropUselessWeapon(m_entity, comparator);
		}
		// ��������� � ��������� ��������������
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	return false;
}
void CommonEnemyNode::SendCureMessage(float dist)
{
	if(AIAPI::getInst()->getHealthPercent(m_entity) < 50.0f)
	{
		AIAPI::getInst()->print(m_entity, "���� ���� ������!");
		getSubteam()->sendMessage(this,	AIEnemyNode::MT_NEEDCURE,&m_entity,	dist);
	}
}
bool CommonEnemyNode::MedkitTake(const ipnt2_t &m_target_pnt, state_type* st)
{
	// 1. �������� �� ������� �� �� � �������
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// �� ������� � ������� ������� - �������, ���� ������
		AIAPI::getInst()->print(m_entity, "thinkMedkitTake: �� ������� � ������� - �������, ���� ������");
		// ������� ��� �������
		AIAPI::getInst()->pickupAllNearMedikit(m_entity);
		// ��������� � ��������� �������� � ������� �����
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 2. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "thinkMedkitTake: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return true;
	}
	
	// ��������� ���� � ��������
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
	if(path.empty())
	{
		// � �������� ������ ������� - ������� �� ����
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return true;

}
/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ������������� �����
//
/////////////////////////////////////////////////////////////////////////////
DCTOR_IMP(FixedEnemyNode)
// ����������� - id �������� � �������, ��� �� ����� ������
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
    // ���� �������� - ����� ��������
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
		// �����������
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
        st << static_cast<int>(m_lookround_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());

		st << m_basepnt.x << m_basepnt.y << m_basedir << m_turn << m_target_dir
			<< m_target_pnt.x << m_target_pnt.y << m_prev_dir;

		return;
    }
	// ��������
	int tmp;
	st >> tmp; m_mode = static_cast<FixedEnemyMode>(tmp);
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> tmp; m_lookround_state = static_cast<LookroundState>(tmp);
	st >> m_entity;
	st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));

	st >> m_basepnt.x >> m_basepnt.y >> m_basedir >> m_turn >> m_target_dir
		>> m_target_pnt.x >> m_target_pnt.y >> m_prev_dir;
	
	// ��, ��� ����������������� ��� ������
	m_activity = 0;
	AIAPI::getInst()->getBaseField(m_basepnt, 10, &m_base_field);
	m_initialising = false;
}


//��������� ��������� � �����������
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

// ��������� ��������� �� �������� ��� ���������
void FixedEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	BaseEntity* be = AIAPI::getInst()->getPtr(m_entity);
	if( be && (be == who) ) return; // ��� � ��� ��������

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (be == victim))
	{	// ����� ����������� - ������ �� ��������� ���
		m_turn = fixed_turns-1;
	}
}

// ��������� ������ Base
bool FixedEnemyNode::thinkBase(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(fixed_cure_dist);

	// 2. ���� � ��� ��� ������ - �������� � ��� ������
	if( (!AIAPI::getInst()->takeBestWeapon(m_entity, FixedWeaponComparator())) &&	(!AIAPI::getInst()->takeGrenade(m_entity))	)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkBase: ���� ������ �����!");
		m_mode = FEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_turn = 0;
		return false;
	}
	
	// 3. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity)) 	)
		{
			AIAPI::getInst()->print(m_entity, "|F|thinkBase: � ������ �����!");
			// ������ ������ �������� �����, ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,	&enemy,	fixed_enemy_dist);
			// ������� � ��������� �����, ������� ������ ���� � ������� 10 ������ �� ������� �����:
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			// ������� ������ ��������� ������ �������� ����
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 4. ���� ��� ���������� ������� ����� - ������� � �����
	enemies.clear();
	if(AIAPI::getInst()->getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies)  )
	{
		// ������� - ������� ������ ��������
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity)))
		{
			AIAPI::getInst()->print(m_entity, "|F|thinkBase: ��� ������� ������� �����!");
			// ������ ������ �������� �����, ������� � ��������� �����
			// ������� ������ ���� � ������� 10 ������ �� ������� �����:
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			// ������� ������ ��������� ������ �������� ����
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),m_target_pnt, path);
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 5. ������ ���
	// 5.1 ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 5.2 ���� ���� ������� � ��� ��� ����� - ������ �� ���
	if(needAndSeeMedkit(&m_target_pnt))
	{
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
		if(!path.empty())
		{
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVEIGNORE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_mode = FEM_MEDKITTAKE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 5.3 ���� ����� ������������� - ��������
	if(++m_turn < fixed_turns)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkBase: ��� �� ����� ����������");
		// ��� �� ����� �������������
		m_turn_state = TS_END;
		return false;
	}
	// ���� ����������
	AIAPI::getInst()->print(m_entity, "|F|thinkBase: ���� �������������");
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

// ��������� ������ LookRound
bool FixedEnemyNode::thinkLookRound(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(fixed_cure_dist);

	// 2. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{	// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|F|thinkLookRound: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,&enemy,fixed_enemy_dist);
			// ������� � ��������� �����, ������� ������ ���� � ������� 10 ������ �� ������� �����:
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			// ������� ������ ��������� ������ �������� ����
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 3. ���� ��� ���������� ������� ����� - ������� � �����
	enemies.clear();
	if(AIAPI::getInst()->getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies))
	{
		// ������� - ������� ������ ��������
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|F|thinkLookRound: ��� ������� ������� �����!");
			// ������ ������ �������� �����, ������� � ��������� �����
			// ������� ������ ���� � ������� 10 ������ �� ������� �����:
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			// ������� ������ ��������� ������ �������� ����
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 4. ������� ������� ����, �� ������� ��������� ��������
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if(fabs(m_target_dir - cur_angle) < angle_eps)
	{
		// �������� ��������� ����� ���������
		switch(m_lookround_state)
		{
		case LS_FIRST : 
			{
				AIAPI::getInst()->print(m_entity, "|F|thinkLookRound: �������� ������ ����� ���������");
				// �������� ������ ����� ���������
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
				AIAPI::getInst()->print(m_entity, "|F|thinkLookRound: �������� ������ ����� ���������");
				// �������� �������������� ����� ���������
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
				AIAPI::getInst()->print(m_entity, "|F|thinkLookRound: ��������� �������� ��������");
				// ��������� � ������� ���������
				m_mode = FEM_BASE;
				m_turn_state = TS_END;
				break;
			}
		default : break;
		}
		return false;
	}

	// 5. ���� �� ������� ���������� - ��������� �� �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkLookRound: �� ������� ���������� - ��������� �� �������� ���");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// ��������� ��������
	m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir,ActivityFactory::CT_ENEMY_LOOKROUND);
	
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_SPOTTED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ Attack
bool FixedEnemyNode::thinkAttack(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(fixed_cure_dist);

	// 2. ���� � ��� ��� ������ - �������� � ��� ������
	if( (!AIAPI::getInst()->takeBestWeapon(m_entity, FixedWeaponComparator())) &&
		(!AIAPI::getInst()->takeGrenade(m_entity))	)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkAttack: ���� ������ �����!");
		m_mode = FEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::getInst()->setStandPose(m_entity);
		return false;
	}

	// 3. ������� �����
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// ��� ����� �����
		// ���� ��� ������ ���� ��� ������� - ������� ������ �������������
		if(m_turn_state == TS_START)
		{
			m_ignored_enemies.clear();
			// � ������ �������� ���� ��� �� �����
			AIAPI::getInst()->setSitPose(m_entity);
		}
		// ��������� ����� �������� ������� ���� ������ � �������������� ������
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty()) // ������� ������ �������� ����� �� ����
			enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		else // ������� ������ �������� ����� �� �������������������
			enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));
	}
	
	if(!enemy)
	{
		// ��� ����� �� �����
		enemies.clear();
		if(AIAPI::getInst()->getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies)	)
		{
			// ���� ����� ��� ����������
			// ���� ��� ������ ���� ��� ������� - ������� ������ �������������
			if(m_turn_state == TS_START)
			{
				m_ignored_enemies.clear();
				// � ������ �������� ���� ��� �� �����
				AIAPI::getInst()->setSitPose(m_entity);
			}
			// ��������� ����� �������� ������� ���� ������ � �������������� ������
			EntityList_t diff_list;
			setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
			if(diff_list.empty())// ������� ������ �������� ����� �� ����
				enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
			else// ������� ������ �������� ����� �� �������������������
				enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));
		}
	}
	
	// 4. ���� ��� ������ - ������� � ������� �����
	if(!enemy)
	{
		// ������ ��� ������ - ������� � ��������� ������� � ������� �����
		AIAPI::getInst()->print(m_entity, "|F|thinkAttack: ������ ��� - �� ����");
		m_mode = FEM_RETURN2BASE;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::getInst()->setStandPose(m_entity);
		return false;
	}
	// ����� ����...

	// 5. ���� ��� ������ ������ ����
	if(m_turn_state == TS_START)
	{
		// �������� ������ ����� � ���� � ���
		AIAPI::getInst()->setStandPose(m_entity);
		AIAPI::getInst()->print(m_entity, "|F|thinkAttack: �������� ������ ����� � ���� � ���");
		// ������� ������ ���� � ������� 10 ������ �� ������� �����:
		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		// ������� ������ ��������� ������ �������� ����
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
		// ������� ������ ����
		m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_ENEMY_REACTED);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_ENEMY_SPOTTED);
		
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 6. ��� ����������� ����

	// 6.1 ���� ����� �� ������� ���������� - �� �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkAttack: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}
	
	// 6.2 ���� ��������� ������� ��� ������ ������ ����� - ���������
	//       ������ ����
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED) 	)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkAttack: ��������� ������� ��� ������ ������ �����");
		m_activity_observer.clear();
		// ��������� ������ ����
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return false;
	}

	// 6.3 ���� ��� ����� ���� - ��������� ����� � ������ �������������
	// ������, ���� ����� ����� ��� ����� ��������� � ������ �������������
	// �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		AIAPI::getInst()->print(m_entity, "|F|thinkAttack: ��� ����� ���� - ��������� ����� � ������ �������������");
		m_activity_observer.clear();
		// ������� � ������ �������������
		m_ignored_enemies.push_back(enemy);
		m_ignored_enemies.unique();
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);

		if(!diff_list.empty())
		{
			// ���������� ��� ������ ������ ������
			m_turn_state = TS_INPROGRESS;
			return true;
		}
		
		if(AIAPI::getInst()->isSitPose(m_entity))
		{
			// �������
			AIAPI::getInst()->setStandPose(m_entity);
			// ������� ������� ����������������� ������
			m_ignored_enemies.clear();
			// ��������� �������� ������ ��� ���
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// �������� ���
		AIAPI::getInst()->setSitPose(m_entity);
		m_turn_state = TS_END;
		return false;
	}

	// 6.4 ���� ����� ����� ������ ����� - �������� ������ ����, ��������� �� ����������
	eid_t weak_enemy = AIAPI::getInst()->getLeastDangerousEnemy(enemies, EnemyComparator(m_entity));
	if(enemy != weak_enemy)
	{
		// ������ ����� ������ - ��������� ������ � ������� ������
		if(AIAPI::getInst()->takeShield(m_entity))
		{
			AIAPI::getInst()->print(m_entity, "|F|thinkAttack: � �� ��� ������!");
			AIAPI::getInst()->throwShield(m_entity, weak_enemy, &m_activity);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 6.5 ���� �� �������� � ����� - �������������� � ����������� ��� ��� ����
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// ����� �������������� � �����
		AIAPI::getInst()->setStandPose(m_entity);
		m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),	m_target_dir,ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// 6.6 �������� � ����� - ���� ������ ����� ���������, ��������
	AIAPI::getInst()->setSitPose(m_entity);
	
	AIAPI::getInst()->print(m_entity, "|F|thinkAttack: ����� ��������, ���� ��� ��������!");
	// ���� ���� - ��������� �� ���� ���� ��� ��������
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
		std::string str = "|F|thinkAttack: �������� ������: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		AIAPI::getInst()->setSitPose(m_entity);
		m_turn_state = TS_END;
	}
//	m_turn_state = TS_END;
	return false;
}

// ��������� ������ Return2Base
bool FixedEnemyNode::thinkReturn2Base(state_type* st)
{
	// 1. �������� ������ �� ��� � ������� ����� ��� ���
	if(m_basepnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// ������ - ������� � ������� ���������
		AIAPI::getInst()->print(m_entity, "|F|thinkReturn2Base: ������ � ������� �����");
		m_mode = FEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// 2. �� ����������� �� ���������?
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// ����������� - ������� ���
		AIAPI::getInst()->print(m_entity, "|F|thinkReturn2Base: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// �� ��������� �� � ������� �����
	// 3. ��������, ����� �� ������ ����� �� ������� �����
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_basepnt, path);
	if(!path.empty())
	{
		// ����� - ������ � ���
		m_target_pnt = m_basepnt;
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// � ������� ����� ����� ������
	// 4. ��������� �� �� �� ������� ����?
	if(AIAPI::getInst()->isEntityInBaseField(m_entity, m_base_field))
	{
		// �� - ������� � ������� ���������
		AIAPI::getInst()->print(m_entity, "|F|thinkReturn2Base: ������ � ������� ����");
		m_mode = FEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// �� ��� �������� ����
	// 5. ����� �� ����� �� �����-������ ����� � ������� ����?
	pnt_vec_t vec;
	AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
	if(vec.empty())
	{
		// ������ - ������� � ������� ���������
		AIAPI::getInst()->print(m_entity, "|F|thinkReturn2Base: �� ���� ����� ���� �� �������� ����!");
		m_mode = FEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// �����
	// ������� ��������� �����
	m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
	// ������ � ���
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer,		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ WeaponSearch
bool FixedEnemyNode::thinkWeaponSearch(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(fixed_cure_dist);
	// 2. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getInst()->getWeaponLocation(m_entity, &m_target_pnt))
	{
		// �������� ���� ������ - ������� � ��� �������
		pnt_vec_t base, vec, path;
		AIAPI::getInst()->getBaseField(m_target_pnt, 1, &base);
		AIAPI::getInst()->getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);

		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		if(path.empty())// � ������ ������ ������� - ������� ���
		{
			m_turn_state = TS_END;
			return false;
		}
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		m_mode = FEM_WEAPONTAKE;
		return false;
	}

	// ������ ����� ��� - �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|F|thinkWeaponSearch: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}
	
	// ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// ��������� ���� - ������� ��������� ����� � ��������� ����
	pnt_vec_t vec, path;
	AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
	m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);

	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),m_target_pnt, path);
	if(path.empty())
	{
		// ������ ���� - ��������� ���
		m_turn_state = TS_END;
		return false;
	}
	// ����� ����
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ WeaponTake
bool FixedEnemyNode::thinkWeaponTake(state_type* st)
{
	// 1. �������� �� ������� �� �� � ������
	if(PickUpWeaponAndAmmo(m_target_pnt, FixedWeaponComparator()))
	{
		m_mode = FEM_RETURN2BASE;
		return false;
	}

	// 2. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|F|thinkWeaponTake: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getInst()->getWeaponLocation(m_entity, &m_target_pnt))
	{
		// �������� ���� ������ - ������ � ����
		pnt_vec_t base, vec, path;
		AIAPI::getInst()->getBaseField(m_target_pnt, 1, &base);
		AIAPI::getInst()->getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);

		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),m_target_pnt, path);
		if(path.empty())
		{
			// � ������ ������ ������� - ������� �� ����
			m_mode = FEM_RETURN2BASE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// ������ ��� - ������� �� ����
	m_mode = FEM_RETURN2BASE;
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ MedkitTake
bool FixedEnemyNode::thinkMedkitTake(state_type* st)
{
	if(!MedkitTake(m_target_pnt,st))	m_mode = FEM_RETURN2BASE;
	return false;

}

// ����!
bool FixedEnemyNode::die()
{
	m_mode = FEM_KILLED;
	return need2Delete();
}

// ����� �� ������� ����
bool FixedEnemyNode::need2Delete() const
{
	if( (m_mode == FEM_KILLED) && !m_activity) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ����������� �����
//
/////////////////////////////////////////////////////////////////////////////
DCTOR_IMP(PatrolEnemyNode)

// ����������� - id ��������
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
    // ���� �������� - ����� ��������
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
		// �����������
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
	// ��������
	int tmp;
	st >> tmp; m_mode = static_cast<PatrolEnemyMode>(tmp);
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> m_entity;
	st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));

	st >> m_enemy_id >> m_enemy_pnt.x >> m_enemy_pnt.y >> m_cure_id
		>> m_cure_pnt.x >> m_cure_pnt.y >> m_shoot_pnt.x >> m_shoot_pnt.y
		>> m_shooted >> m_target_pnt.x >> m_target_pnt.y >> m_target_dir
		>> m_last_enemy_pnt.x >> m_last_enemy_pnt.y;
	
	// ��, ��� ����������������� ��� ������
	m_activity = 0;
	m_initialising = false;
}

// ��������� ��������������� ���������
void PatrolEnemyNode::recieveMessage(MessageType type, void * data)
{
	eid_t id = *static_cast<eid_t*>(data);
	switch(type)
	{
	case MT_ENEMYSPOTTED :
		{
			if(m_enemy_id)
			{
				// ��� �������� � �����, �������� ��� �� ������ �����
				float old_dist = dist(AIAPI::getInst()->getPos2(m_entity),	m_enemy_pnt);
				float new_dist = dist(AIAPI::getInst()->getPos2(m_entity),	AIAPI::getInst()->getPos2(id));
				if(new_dist < old_dist)
				{  	// ������ �����
					m_enemy_id = id;
					m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
				}
				return;
			}
			// ��� ������ ����, � ������� ��������
			m_enemy_id = id;
			m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
			return;
		}
	case MT_NEEDCURE :
		{
			if(m_cure_id)
			{
				// ��� ������� � ������, �������� ��� �� �������� �����
				float old_dist = dist(AIAPI::getInst()->getPos2(m_entity),	m_cure_pnt);
				float new_dist = dist(AIAPI::getInst()->getPos2(m_entity),	AIAPI::getInst()->getPos2(id));
				if(new_dist < old_dist)
				{	// ������ ��������� � ������
					m_cure_id = id;
					m_cure_pnt = AIAPI::getInst()->getPos2(m_cure_id);
				}
				return;
			}
			// ��� ������ ��������� � ������
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

//��������� �������� �� ��������
void PatrolEnemyNode::OnKillEv(BaseEntity* killer, BaseEntity* victim)
{
	// �������� ������ �� �����
	if(victim == AIAPI::getInst()->getPtr(m_enemy_id)) m_enemy_id = 0;
	if(victim == AIAPI::getInst()->getPtr(m_cure_id)) m_cure_id = 0;
}

// ��������� ��������� �� �������� ��� ���������
void PatrolEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// �� ����� ����������� � ������,
	// ���� ���� ��������� � ������������ ����������
	switch(m_mode)
	{
	case PEM_ATTACK: case PEM_WEAPONSEARCH: case PEM_WEAPONTAKE : return;
	default: break;
	}

	if(AIAPI::getInst()->getPtr(m_entity) == who) return; // ��� � ��� ��������

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (AIAPI::getInst()->getPtr(m_entity) == victim) )
	{
		// ����� �����������
		m_shoot_pnt = AIAPI::getInst()->getPos2(who);
		m_shooted = true;
	}
}

// ��������� ������ Patrol
bool PatrolEnemyNode::thinkPatrol(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(patrol_cure_dist);

	// 2. ���� � ��� ��� ������ - �������� � ��� ������
	if( (!AIAPI::getInst()->takeGrenade(m_entity)) &&
		(!AIAPI::getInst()->takeBestWeapon(m_entity, PatrolWeaponComparator()))	)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: ���� ������ �����!");
		m_mode = PEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 3. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,	&enemy,	patrol_enemy_dist);
			// ������� � ��������� �����
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
			AIAPI::getInst()->setRun(m_entity);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� ������� aim, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	SHT_AIMSHOT);
			// ������ ��������� �� ������� � �����
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);

			if(!steps) // ����� ������� ��� ����� ��� ����, ������ ��������� �, �����
				return false;
			// ���������� ���� ������������
			pnt_vec_t field, vec, path;
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
			// ������� ������ ��������� ������
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,PatrolHexComparator(m_entity, enemy));
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 4. ���� � ���� �������� - ����� ������������
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: � ���� �������� - ����� ������������");
		// � ���� ��� ����� �� ���� ��������
		m_shooted = false;
		// ������� ����� � ������� 3 ������ �� ��������, ������� ��������
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		pnt_vec_t field, vec, path;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		AIAPI::getInst()->getBaseField(m_shoot_pnt,	3, &field);
		// ������� ������ ��������� ������
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. ���� ��� �������� � ����� - ������ � ����
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: ��� �������� � �����");
		// ��������
		m_enemy_id = 0;
		// ������� ����� � ������� 1 ����� �� �����
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,	1, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 6. ���� ���� ���� ������ - ��������� � ��������� �������
	if(m_cure_id)
	{
		// �������� �������� �����
		if( (AIAPI::getInst()->getHealthPercent(m_cure_id) < 50.0f) &&
			(AIAPI::getInst()->haveMedikit(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: ����� ������");
			// ��� ����� ������
			m_mode = PEM_CURE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// ��� �� ����� ������
		m_cure_id = 0;
	}

	// 7. ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 8. ���� ������� �� ������ ������, ������� ��� ������� - 
	// ������� � ��������� ������ ����
	ipnt2_t pos;
	if(AIAPI::getInst()->getThingLocation(m_entity,	&pos,TT_WEAPON|TT_AMMO|TT_MEDIKIT)	)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: ������ �� ������ ������, ������� ��� �������");
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
			// ����� ����
			m_target_pnt = pos;
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			m_mode = PEM_THINGTAKE;
			return false;
		}
	}

	// 9. ���� ����� �� �������� ����� - �������� �����
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: ����� �� �������� ����� - �������� �����");
		// ���������� ���� ������������
		pnt_vec_t allhexes;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, &allhexes, 0);
		if(allhexes.empty())
		{
			// �������� ��� - ���� ������
			AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: ��� ������ ������!!!");
			m_turn_state = TS_END;
			return false;
		}
		// ������� ��������� �����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(allhexes);
	}

	// 10. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 10. ���� � �������� �����
	AIAPI::getInst()->print(m_entity, "|P|thinkPatrol: ���� � �������� �����");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ Attack
bool PatrolEnemyNode::thinkAttack(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(patrol_cure_dist);
	// 2. ���� � ��� ��� ������ - �������� � ��� ������
	if( (!AIAPI::getInst()->takeGrenade(m_entity)) &&
		(!AIAPI::getInst()->takeBestWeapon(m_entity, PatrolWeaponComparator()))	)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkAttack: ���� ������ �����!");
		m_mode = PEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::getInst()->setWalk(m_entity);
		return false;
	}

	// 3. ������� �����
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// ��� ����� �����, ���� ��� ������ ���� ��� ������� - ������� ������ �������������
		if(m_turn_state == TS_START)	m_ignored_enemies.clear();

		// ��������� ����� �������� ������� ���� ������ � �������������� ������
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);

		if(diff_list.empty())// ������� ������ �������� ����� �� ����
			enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		else// ������� ������ �������� ����� �� �������������������
			enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));

		m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
	}
	
	// 4. ���� ��� ������ - ��������� � ��������� �������������
	if(!enemy)
	{
		// ������ ��� - ����� ������������ ����������
		AIAPI::getInst()->print(m_entity, "|P|thinkAttack: ������ ��� - ����� ������������ ����������");
		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		AIAPI::getInst()->setWalk(m_entity);
		return false;
	}
	// ����� ����...

	// 5. ���� ��� ������ ������ ����
	if(m_turn_state == TS_START)
	{
		// �������� ������ ����� � ���� � ���
		AIAPI::getInst()->print(m_entity, "|P|thinkAttack: �������� ������ ����� � ���� � ���");

		// ������� ������ ���� ���, ����� ����� ������� ����������
		// �� ���� ������� aim, ���� ����� ������ ��� - �������
		// ������ ����, � ������� �����.
		
		m_turn_state = TS_INPROGRESS;
		AIAPI::getInst()->setRun(m_entity);
		int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
		movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	SHT_AIMSHOT);
		// ������ ��������� �� ������� � �����
		movpoints -= MPS_FOR_ROTATE*4;
		int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);

		if(!steps)// ����� ������� ��� ����� ��� ����, ������ ��������� �, �����
			return false;

		// ���������� ���� ������������
		pnt_vec_t field, vec, path;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
		// ������� ������ ��������� ������
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// ������� ������ ����
		m_target_pnt = AIAPI::getInst()->getBestPoint(vec, 	PatrolHexComparator(m_entity, enemy));

		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_SPOTTED);
		return false;
	}
	// 6. ��� ����������� ����
	// 6.1 ���� ����� �� ������� ���������� - �� �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// ����������� - �������� ���
		AIAPI::getInst()->print(m_entity, "|P|thinkAttack: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6.2 ���� ��������� ������� ��� ������ ������ ����� - ���������
	//       ������ ����
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)	)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkAttack: ��������� ������� ��� ������ ������ �����");
		m_activity_observer.clear();
		// ��������� ������ ����
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 6.3 ���� ��� ����� ���� - ��������� ����� � ������ �������������
	// ������, ���� ����� ����� ��� ����� ��������� � ������ �������������
	// ��������� ������� � ������ �� �������������� ������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// ����������� - �������� ���
		AIAPI::getInst()->print(m_entity, "|P|thinkAttack: ��� ����� ���� - ��������� ����� � ������ �������������");
		m_activity_observer.clear();
		// ������� � ������ �������������
		m_ignored_enemies.push_back(enemy);
		m_ignored_enemies.unique();
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// ��������� ������� ������� � �����
			AIAPI::getInst()->print(m_entity, "|P|thinkAttack: �������� �������");
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, PathUtils::F_CALC_NEAR_PNTS);
			EntityList_t::iterator i = enemies.begin();
			// ������ ��������, � �������� ����� �������
			bool found = false;
			while(i != enemies.end())
			{
				// ��������, ���������� �� ����� ����� � ���������
				if(PathUtils::GetInst()->IsNear(AIAPI::getInst()->getPtr(*i)))
				{
					// � ����� ����� �������
					found = true;
					m_target_pnt = PathUtils::GetInst()->GetNearPnt(AIAPI::getInst()->getPtr(*i)).m_pnt;
					break;
				}
				++i;
			}
			pnt_vec_t path;
			if(!found)
			{
				// ����������� ����� ��� - ������������ ��������� �������
				pnt_vec_t base, vec;
				AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	patrol_enemy_noLOF_move,&base);
				AIAPI::getInst()->getReachableField(base, &vec, true);
				m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			}
			else
			{
				// ���� ����, � �������� ����� ������� - �������� �� �������� ����
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
				// ������� �������� ����
				path.erase(path.begin(), path.begin() + (path.size() / 2));
			}
			if(path.empty())
			{
				// ��������� ���
				m_turn_state = TS_END;
				return false;
			}
			// ����� ����
			AIAPI::getInst()->setWalk(m_entity);
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
			
			m_turn_state = TS_INPROGRESS;
			return false;
		} 
		// ���������� ��� ������ ������ ������
		m_turn_state = TS_INPROGRESS;
		return true;
	}

	// 6.4 ���� �� �������� � ����� - �������������� � ����������� ��� ��� ����
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// ����� �������������� � �����
		m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir,ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 6.5 �������� � ����� - ���� ������ ����� ���������
	AIAPI::getInst()->print(m_entity, "|P|thinkAttack: ����� ��������, ���� ��� ��������!");
	// ���� ���� - ��������� �� ���� ���� ��� ��������

	// ������� ��������� �������� (���� ���� � ���� ���������)
	if(AIAPI::getInst()->takeGrenade(m_entity))
	{
		// ������� � ����� - ��������� ������ ��
		std::string reason;
		if(AIAPI::getInst()->shoot(m_entity, enemy, &m_activity, &reason))
		{
			// ������� ������ ������ �������
			m_turn_state = TS_INPROGRESS;
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
			return false;
		}
		// ������ ������ ������� �� �������
		std::string str = "|P|thinkAttack: ������ ������ �������: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
	}

	// ������� ��� ���, ��� ������� �� �� �������
	// ������ ��������� ���������� �� ������
	if(!AIAPI::getInst()->takeBestWeapon(m_entity, PatrolWeaponComparator()))
	{
		m_turn_state = TS_END;
		return false;
	}
	// ������ � �����
	AIAPI::getInst()->setShootType(m_entity, 5.0f, 10.0f);
	std::string reason;
	if(AIAPI::getInst()->shoot(m_entity, enemy, &m_activity, &reason))
	{
		// ������� ������ ��������
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
	}
	else
	{
		std::string str = "|P|thinkAttack: �������� ������: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

// ��������� ������ WeaponSearch
bool PatrolEnemyNode::thinkWeaponSearch(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(patrol_cure_dist);

	// 2. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getInst()->getWeaponLocation(m_entity, &m_target_pnt))
	{
		// �������� ���� ������ - ������� � ��� �������
		AIAPI::getInst()->print(m_entity, "|P|thinkWeaponSearch: �������� ���� ������ - ������� � ��� �������!");
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
			// � ������ ������ ������� - ������� ���
			m_turn_state = TS_END;
			return false;
		}
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		m_mode = PEM_WEAPONTAKE;
		return false;
	}

	// 3. ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 4. ���� ����� �� �������� ����� - �������� �����
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkWeaponSearch: ����� �� �������� ����� - �������� �����");
		// ���������� ���� ������������
		pnt_vec_t allhexes;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, &allhexes, 0);
		if(allhexes.empty())
		{
			// �������� ��� - ���� ������
			AIAPI::getInst()->print(m_entity, "|P|thinkWeaponSearch: ��� ������ ������!!!");
			m_turn_state = TS_END;
			return false;
		}
		// ������� ��������� �����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(allhexes);
	}

	// 5. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|P|thinkWeaponSearch: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. ���� � �������� �����
	AIAPI::getInst()->print(m_entity, "|P|thinkWeaponSearch: ���� � �������� �����");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ ThingTake
bool PatrolEnemyNode::thinkThingTake(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(patrol_cure_dist);

	// 2. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies, EnemyComparator(m_entity)) )
		{
			AIAPI::getInst()->print(m_entity, "|P|thinkThingTake: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this, AIEnemyNode::MT_ENEMYSPOTTED, &enemy, patrol_enemy_dist);
			// ������� � ��������� �����
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
			AIAPI::getInst()->setRun(m_entity);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� ������� aim, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	SHT_AIMSHOT);
			// ������ ��������� �� ������� � �����
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// ����� ������� ��� ����� ��� ����, ������ ��������� �
				// �����
				return false;
			}
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
			// ������� ������ ��������� ������
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	PatrolHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 3. �������� �� ������� �� �� � ����
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// �� ������� � ���������� ����� - ������� ��� ������
		AIAPI::getInst()->print(m_entity, "|P|thinkThingTake: �� ������� � ���������� ����� - ������� ��� ������");
		AIAPI::getInst()->pickupAllNearAmmo(m_entity);
		AIAPI::getInst()->pickupAllNearMedikit(m_entity);
		AIAPI::getInst()->pickupAllNearWeapon(m_entity);
		AIAPI::getInst()->dropUselessWeapon(m_entity, PatrolWeaponComparator());
		// ��������� � ��������� ��������������
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. ���� � ���� �������� - ����� ������������
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkThingTake: � ���� �������� - ����� ������������");
		// � ���� ��� ����� �� ���� ��������
		m_shooted = false;
		// ������� ����� � ������� 3 ������ �� ��������, ������� ��������
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_shoot_pnt,	3, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. ���� ��� �������� � ����� - ������ � ����
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkThingTake: ��� �������� � �����");
		// ��������
		m_enemy_id = 0;
		// ������� ����� � ������� 1 ����� �� �����
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,	1, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 6. ���� ���� ���� ������ - ��������� � ��������� �������
	if(m_cure_id)
	{
		// �������� �������� �����
		if( (AIAPI::getInst()->getHealthPercent(m_cure_id) < 50.0f) &&	(AIAPI::getInst()->haveMedikit(m_entity)))
		{
			AIAPI::getInst()->print(m_entity, "|P|thinkThingTake: ����� ������");
			// ��� ����� ������
			m_mode = PEM_CURE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// ��� �� ����� ������
		m_cure_id = 0;
	}
	
	// 7. ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 8. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|P|thinkThingTake: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 9. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getInst()->getThingLocation(m_entity, &m_target_pnt,	TT_AMMO|TT_WEAPON|TT_MEDIKIT)	)
	{
		// �������� ���� ������ - ������ � ����
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
			// � ������ ������ ������� - ������� � ��������������
			m_mode = PEM_PATROL;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// ������ ��� - ������� � ��������������
	m_mode = PEM_PATROL;
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ WeaponTake
bool PatrolEnemyNode::thinkWeaponTake(state_type* st)
{
	SendCureMessage(patrol_cure_dist);

	// 2. �������� �� ������� �� �� � ������
	if(PickUpWeaponAndAmmo(m_target_pnt, PatrolWeaponComparator()))
	{
		m_mode = PEM_PATROL;
		return false;
	}

	// 3. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|P|thinkWeaponTake: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getInst()->getWeaponLocation(m_entity, &m_target_pnt))
	{
		// �������� ���� ������ - ������ � ����
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
			// � ������ ������ ������� - ������� � ��������������
			m_mode = PEM_PATROL;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// ������ ��� - ������� � ��������������
	m_mode = PEM_PATROL;
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ Cure
bool PatrolEnemyNode::thinkCure(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(patrol_cure_dist);

	// 2. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::getInst()->print(m_entity, "|P|thinkCure: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_enemy_dist);
			// ������� � ��������� �����
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
			AIAPI::getInst()->setRun(m_entity);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� ������� aim, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
				SHT_AIMSHOT);
			// ������ ��������� �� ������� � �����
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// ����� ������� ��� ����� ��� ����, ������ ��������� �
				// �����
				return false;
			}
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
				steps, &field);
			// ������� ������ ��������� ������
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,
				PatrolHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
				m_target_pnt, path);
			// ����� ����
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

	// 3. ���� � ���� �������� - ����� ������������
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkCure: � ���� �������� - ����� ������������");
		// � ���� ��� ����� �� ���� ��������
		m_shooted = false;
		// ������� ����� � ������� 3 ������ �� ��������, ������� ��������
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_shoot_pnt,
			3, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
			m_target_pnt, path);
		// ����� ����
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

	// 4. ���� ��� �������� � ����� - ������ � ����
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkCure: ��� �������� � �����");
		// ��������
		m_enemy_id = 0;
		// ������� ����� � ������� 1 ����� �� �����
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,
			1, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
			m_target_pnt, path);
		// ����� ����
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

	// 5. ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 6. ���� ������ ��� ����� ������
	if( (!m_cure_id) ||
		(AIAPI::getInst()->getHealthPercent(m_cure_id) >= 50.0f) ||
		(!AIAPI::getInst()->haveMedikit(m_entity))
		)
	{
		AIAPI::getInst()->print(m_entity, "|P|thinkCure: ������ ��� ����� ������");
		// ������ ��� ����� ������ - ������� � ��������������
		m_cure_id = 0;
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		AIAPI::getInst()->setWalk(m_entity);
		return false;
	}

	// 7. ���� ������� � ����� - ����� ���
	if(dist(AIAPI::getInst()->getPos2(m_entity),
		AIAPI::getInst()->getPos2(m_cure_id)) <= 1.5f)
	{
		// ������� - �����
		AIAPI::getInst()->print(m_entity, "|P|thinkCure: �����");
		// ������� � ���� �������
		AIAPI::getInst()->takeMedikit(m_entity);
		// ������� �����
		AIAPI::getInst()->cure(m_entity, m_cure_id);
		// ������� � ���� ������
		if(!AIAPI::getInst()->takeGrenade(m_entity))
		{
			AIAPI::getInst()->takeBestWeapon(m_entity, PatrolWeaponComparator());
		}
		// ���������
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	
	// 8. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|P|thinkCure: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 9. ���� � �����, �������� ����� ������
	
	// ���������� ���� ������������
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
		0, 0, PathUtils::F_CALC_NEAR_PNTS);
	// ��������, ���������� �� ����� ����� � ���������
	if(!PathUtils::GetInst()->IsNear(AIAPI::getInst()->getPtr(m_cure_id)))
	{
		// � ����� ������ �������
		m_cure_id = 0;
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	m_target_pnt = PathUtils::GetInst()->GetNearPnt(AIAPI::getInst()->getPtr(m_cure_id)).m_pnt;
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
		m_target_pnt, path);
	// ����� ����
	AIAPI::getInst()->setRun(m_entity);
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
		path,
		ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ Pursuit
bool PatrolEnemyNode::thinkPursuit(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(patrol_cure_dist);

	// 2. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::getInst()->print(m_entity, "|P|thinkPursuit: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_enemy_dist);
			// ������� � ��������� �����
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
			AIAPI::getInst()->setRun(m_entity);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� ������� aim, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
				SHT_AIMSHOT);
			// ������ ��������� �� ������� � �����
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// ����� ������� ��� ����� ��� ����, ������ ��������� �
				// �����
				return false;
			}
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
				steps, &field);
			// ������� ������ ��������� ������
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,
				PatrolHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
				m_target_pnt, path);
			// ����� ����
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

	// 3. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|P|thinkPursuit: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. �������� �� ������ �� �� � �������� �����
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// �� ������
		// 4.1 ��������, �������� �� � �����, ��� ��� ����
		m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->convertPos2ToPos3(m_last_enemy_pnt) - getPosition()));
		float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
		if( fabs(m_target_dir - cur_angle) >= angle_eps )
		{
			// ����� �������������� � �����
			m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),
				m_target_dir,
				ActivityFactory::CT_ENEMY_ROTATE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 4.2 �������� - ������� � ��������� ��������������
		AIAPI::getInst()->print(m_entity, "|P|thinkPursuit: ������ �� ������ - ����� �������������");
		// ��������� � ��������� ��������������
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. ���� � �������� �����
	AIAPI::getInst()->print(m_entity, "|P|thinkPursuit: ���� � �������� �����");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
		0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
		path,
		ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ����!
bool PatrolEnemyNode::die()
{
	m_mode = PEM_KILLED;
	return need2Delete();
}

// ����� �� ������� ����
bool PatrolEnemyNode::need2Delete() const
{
	if( (m_mode == PEM_KILLED) && !m_activity) return true;
	return false;
}


/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ����������
//
/////////////////////////////////////////////////////////////////////////////

// ����������� - id ��������
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
    //��������� ��������
	if (ThinkShell(st)) return complexity;
    // ���� �������� - ����� ��������
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
		// �����������
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
	// ��������
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
	
	
	// ��, ��� ����������������� ��� ������
	m_activity = 0;
	m_initialising = false;
}

// ��������� ��������������� ���������
void AssaultEnemyNode::recieveMessage(MessageType type, void * data)
{
	eid_t id = *static_cast<eid_t*>(data);
	if(type == MT_ENEMYSPOTTED)
	{
		if(m_enemy_id)
		{
			// ��� �������� � �����
			// �������� ��� �� ������ �����
			float old_dist = dist(AIAPI::getInst()->getPos2(m_entity),
				m_enemy_pnt);
			float new_dist = dist(AIAPI::getInst()->getPos2(m_entity),
				AIAPI::getInst()->getPos2(id));
			if(new_dist < old_dist)
			{
				// ������ �����
				m_enemy_id = id;
				m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
			}
			return;
		}
		// ��� ������ ����, � ������� ��������
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

//��������� �������� �� ��������
void AssaultEnemyNode::OnKillEv(BaseEntity* killer, BaseEntity* victim)
{
	// �������� ������ �� �����
	if(victim == AIAPI::getInst()->getPtr(m_enemy_id)) m_enemy_id = 0;
}

// ��������� ��������� �� �������� ��� ���������
void AssaultEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// �� ����� ����������� � ������,
	// ���� ���� ��������� � ������������ ����������
	switch(m_mode)
	{
	case AEM_ATTACK: case AEM_WEAPONSEARCH: case AEM_WEAPONTAKE : return;
	default: break;
	}

	if(AIAPI::getInst()->getPtr(m_entity) == who) return; // ��� � ��� ��������

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (AIAPI::getInst()->getPtr(m_entity) == victim) )
	{
		// ����� �����������
		m_shoot_pnt = AIAPI::getInst()->getPos2(who);
		m_shooted = true;
	}
}

// ��������� ������ Fixed
bool AssaultEnemyNode::thinkFixed(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(fixed_cure_dist);

	// 2. ���� � ��� ��� ������ - �������� � ��� ������
	if( (!AIAPI::getInst()->takeBestWeapon(m_entity, AssaultWeaponComparator())) &&
		(!AIAPI::getInst()->takeGrenade(m_entity))
		)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkFixed: ���� ������ �����!");
		m_mode = AEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_turn = 0;
		return false;
	}
	
	// 3. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::getInst()->print(m_entity, "|A|thinkFixed: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				assault_enemy_dist);
			// ������� � ��������� �����
			m_mode = AEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� ������� aim, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
				SHT_AIMSHOT);
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			// ������ ��������� �� ������� � �����
			movpoints -= MPS_FOR_ROTATE*4;
			// ������ ��������� �� ����������
			movpoints -= MPS_FOR_POSE_CHANGE;
			if(!steps)
			{
				// ����� ������� ��� ����� ��� ����, ������ ��������� �
				// �����
				return false;
			}
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
				steps, &field);
			// ������� ������ ��������� ������
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	AssaultHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
				path,
				ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 4. ���� � ���� �������� - ����� ������������
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkFixed: � ���� �������� - ����� ������������");
		// � ���� ��� ����� �� ���� ��������
		m_shooted = false;
		// ������� ����� � ������� 3 ������ �� ��������, ������� ��������
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_shoot_pnt,	3, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),m_target_pnt, path);
		// ����� ����
		AIAPI::getInst()->setRun(m_entity);
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = AEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_turn = 0;
		return false;
	}

	// 5. ���� ��� �������� � ����� - ������ � ����
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkFixed: ��� �������� � �����");
		// ��������
		m_enemy_id = 0;
		// ������� ����� � ������� 1 ����� �� �����
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,	1, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// ����� ����
		AIAPI::getInst()->setRun(m_entity);
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = AEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_turn = 0;
		return false;
	}

	// 6. ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 7. ���� ���� ������� � ��� ��� ����� - ������ �� ���
	if(needAndSeeMedkit(&m_target_pnt))
	{
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		if(!path.empty())
		{
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVEIGNORE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_mode = AEM_MEDKITTAKE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 8. ����� ���� ������ �� ����� � ���� ����� ������������� - ��������
	if(++m_turn < assault_turns)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkFixed: ��� �� ����� ����������");
		// ��� �� ����� �������������
		m_turn_state = TS_END;
		return false;
	}
	// ���� ����������
	AIAPI::getInst()->print(m_entity, "|A|thinkFixed: ���� �������������");
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

// ��������� ������ Lookround
bool AssaultEnemyNode::thinkLookround(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(assault_cure_dist);

	// 2. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,	&enemy,	assault_enemy_dist);
			// ������� � ��������� �����
			m_mode = AEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� ������� aim, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	SHT_AIMSHOT);
			// ������ ��������� �� ������� � �����
			movpoints -= MPS_FOR_ROTATE*4;
			// ������ ��������� �� ����������
			movpoints -= MPS_FOR_POSE_CHANGE;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// ����� ������� ��� ����� ��� ����, ������ ��������� �
				// �����
				return false;
			}
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
			// ������� ������ ��������� ������
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,	AssaultHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 3. ���� � ���� �������� - ����� ������������
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: � ���� �������� - ����� ������������");
		// � ���� ��� ����� �� ���� ��������
		m_shooted = false;
		// ������� ����� � ������� 3 ������ �� ��������, ������� ��������
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_shoot_pnt,	3, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),m_target_pnt, path);
		// ����� ����
		AIAPI::getInst()->setRun(m_entity);
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = AEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. ���� ��� �������� � ����� - ������ � ����
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: ��� �������� � �����");
		// ��������
		m_enemy_id = 0;
		// ������� ����� � ������� 1 ����� �� �����
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,	1, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
		// ����� ����
		AIAPI::getInst()->setRun(m_entity);
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = AEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. ������� ������� ����, �� ������� ��������� ��������
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if(fabs(m_target_dir - cur_angle) < angle_eps)
	{
		// �������� ��������� ����� ���������
		switch(m_lookround_state)
		{
		case LS_FIRST : 
			{
				AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: �������� ������ ����� ���������");
				// �������� ������ ����� ���������
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
				AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: �������� ������ ����� ���������");
				// �������� �������������� ����� ���������
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
				AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: ��������� �������� ��������");
				// ��������� � ������� ���������
				m_mode = AEM_FIXED;
				m_turn_state = TS_END;
				break;
			}
		default : break;
		}
		return false;
	}

	// 6. ���� �� ������� ���������� - ������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkLookRound: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// ��������� ��������
	m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir,ActivityFactory::CT_ENEMY_LOOKROUND);
	
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ Attack
bool AssaultEnemyNode::thinkAttack(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(assault_cure_dist);

	// 2. ���� � ��� ��� ������ - �������� � ��� ������
	if( (!AIAPI::getInst()->takeBestWeapon(m_entity, AssaultWeaponComparator())) &&
		(!AIAPI::getInst()->takeGrenade(m_entity))
		)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkAttack: ���� ������ �����!");
		m_mode = AEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::getInst()->setStandPose(m_entity);
		return false;
	}

	// 3. ������� �����
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// ��� ����� �����

		// ���� ��� ������ ���� ��� ������� - ������� ������ �������������
		if(m_turn_state == TS_START)
		{
			m_ignored_enemies.clear();
		}
		// ��������� ����� �������� ������� ���� ������ � �������������� ������
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// ������� ������ �������� ����� �� ����
			enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
				EnemyComparator(m_entity));
		}
		else
		{
			// ������� ������ �������� ����� �� �������������������
			enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));
		}
		m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
	}
	
	// 4. ���� ��� ������ - ��������� � ��������� �������������
	if(!enemy)
	{
		// ������ ��� - ����� ������������ ����������
		AIAPI::getInst()->print(m_entity, "|A|thinkAttack: ������ ��� - ����� ������������ ����������");
		m_mode = AEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		AIAPI::getInst()->setStandPose(m_entity);
		return false;
	}
	// ����� ����...

	// 5. ���� ��� ������ ������ ���� ��� ��������� �������
	if(m_turn_state == TS_START)
	{
		// �������� ������ ����� � ���� � ���
		AIAPI::getInst()->print(m_entity, "|A|thinkAttack: �������� ������ ����� � ���� � ���");

		// ������� ������ ���� ���, ����� ����� ������� ����������
		// �� ���� ������� aim, ���� ����� ������ ��� - �������
		// ������ ����, � ������� �����.
		
		m_turn_state = TS_INPROGRESS;
		// ������� ����� ���-�� ���������� ����������
		int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
		// ������ ���������� ���������� ����������� ��� ��������
		movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	SHT_AIMSHOT);
		// ������ ��������� �� ������� � �����
		movpoints -= MPS_FOR_ROTATE*4;
		// ������ ��������� �� ����������
		movpoints -= MPS_FOR_POSE_CHANGE;
		int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
		if(!steps)
		{
			// ����� ������� ��� ����� ��� ����, ������ ��������� � �����
			return false;
		}
		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec, true);
		// ������� ������ ����
		m_target_pnt = AIAPI::getInst()->getBestPoint(vec, AssaultHexComparator(m_entity, enemy));
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,	ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
		return false;
	}

	// 6. ��� ����������� ����

	// 6.1 ���� ����� �� ������� ���������� - �� �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// ����������� - �������� ���
		AIAPI::getInst()->print(m_entity, "|A|thinkAttack: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6.2 ���� ��������� ������� ��� ������ ������ ����� - ���������
	//       ������ ����
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)	)
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkAttack: ��������� ������� ��� ������ ������ �����");
		m_activity_observer.clear();
		// ��������� ������ ����
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 6.3 ���� ��� ����� ���� - ��������� ����� � ������ �������������
	// ������, ���� ����� ����� ��� ����� ��������� � ������ �������������
	// ��������� ������� � ������ �� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// ����������� - �������� ���
		AIAPI::getInst()->print(m_entity, "|A|thinkAttack: ��� ����� ���� - ��������� ����� � ������ �������������");
		m_activity_observer.clear();
		// ������� � ������ �������������
		m_ignored_enemies.push_back(enemy);
		m_ignored_enemies.unique();
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// ��������� ������� ������� � �����
			AIAPI::getInst()->print(m_entity, "|A|thinkAttack: �������� �������");
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, PathUtils::F_CALC_NEAR_PNTS);
			EntityList_t::iterator i = enemies.begin();
			// ������ ��������, � �������� ����� �������
			bool found = false;
			while(i != enemies.end())
			{
				// ��������, ���������� �� ����� ����� � ���������
				if(PathUtils::GetInst()->IsNear(AIAPI::getInst()->getPtr(*i)))
				{
					// � ����� ����� �������
					found = true;
					m_target_pnt = PathUtils::GetInst()->GetNearPnt(AIAPI::getInst()->getPtr(*i)).m_pnt;
					break;
				}
				++i;
			}
			pnt_vec_t path;
			if(!found)
			{
				// ����������� ����� ��� - ������������ ��������� �������
				pnt_vec_t base, vec;
				AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	patrol_enemy_noLOF_move,&base);
				AIAPI::getInst()->getReachableField(base, &vec, true);
				m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			}
			else
			{
				// ���� ����, � �������� ����� ������� - �������� �� �������� ����
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
				// ������� �������� ����
				path.erase(path.begin(), path.begin() + (path.size() / 2));
			}
			if(path.empty())
			{
				// ��������� ���
				m_turn_state = TS_END;
				return false;
			}
			// ����� ����
			AIAPI::getInst()->setWalk(m_entity);
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
			
			m_turn_state = TS_INPROGRESS;
			return false;
		} // if(diff_list.empty())
		// ���������� ��� ������ ������ ������
		m_turn_state = TS_INPROGRESS;
		return true;
	}

	// 6.4 ���� �� �������� � ����� - �������������� � ����������� ��� ��� ����
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// ����� �������������� � �����
		m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),
			m_target_dir,
			ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 6.5 �������� � ����� - ���� ������ ����� ���������
	AIAPI::getInst()->print(m_entity, "|A|thinkAttack: ����� ��������, ���� ��� ��������!");
	// ���� ���� - ��������� �� ���� ���� ��� ��������
	// �������� � ������������ 60%
	if(rand() < 19662)
	{
		AIAPI::getInst()->setSitPose(m_entity);
	}
	else
	{
		AIAPI::getInst()->setStandPose(m_entity);
	}

	// ���� ���� ������� � ����� ��������� ��������� ������ ����� -
	// ��������� ������ ��
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
		// ������� � ����� - ��������� ������ ��
		std::string reason;
		if(AIAPI::getInst()->shoot(m_entity, enemy, &m_activity, &reason))
		{
			// ������ ������
			m_turn_state = TS_INPROGRESS;
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_NO_LOF);
			return false;
		}
		// ������ �� �������
		std::string str = "|A|thinkAttack: ������ ������ �������: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}

	// ������ ��������� ���������� �� ������
	if(AIAPI::getInst()->takeBestWeapon(m_entity, AssaultWeaponComparator()))
	{
		// ������ � �����
		AIAPI::getInst()->setShootType(m_entity, 30.0f, 15.0f);
		std::string reason;
		if(AIAPI::getInst()->shoot(m_entity, enemy, &m_activity, &reason))
		{
			// �������� ��������
			m_turn_state = TS_INPROGRESS;
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_NO_LOF);
			return false;
		}
		// ���������� �� �������
		std::string str = "|A|thinkAttack: �������� ������: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	m_turn_state = TS_END;
	return false;
}

// ��������� ������ WeaponSearch
bool AssaultEnemyNode::thinkWeaponSearch(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(assault_cure_dist);

	// 2. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getInst()->getWeaponLocation(m_entity, &m_target_pnt))
	{
		// �������� ���� ������ - ������� � ��� �������
		AIAPI::getInst()->print(m_entity, "|A|thinkWeaponSearch: �������� ���� ������ - ������� � ��� �������!");
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
			// � ������ ������ ������� - ������� ���
			m_turn_state = TS_END;
			return false;
		}
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		m_mode = AEM_WEAPONTAKE;
		return false;
	}

	// 3. ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 4. ���� ����� �� �������� ����� - �������� �����
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|A|thinkWeaponSearch: ����� �� �������� ����� - �������� �����");
		// ���������� ���� ������������
		pnt_vec_t allhexes;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, &allhexes, 0);
		if(allhexes.empty())
		{
			// �������� ��� - ���� ������
			AIAPI::getInst()->print(m_entity, "|A|thinkWeaponSearch: ��� ������ ������!!!");
			m_turn_state = TS_END;
			return false;
		}
		// ������� ��������� �����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(allhexes);
	}

	// 5. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|A|thinkWeaponSearch: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. ���� � �������� �����
	AIAPI::getInst()->print(m_entity, "|A|thinkWeaponSearch: ���� � �������� �����");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
		0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	// ����� ����
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
		path,
		ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ WeaponTake
bool AssaultEnemyNode::thinkWeaponTake(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(assault_cure_dist);

	// 2. �������� �� ������� �� �� � ������
	if(PickUpWeaponAndAmmo(m_target_pnt, AssaultWeaponComparator()))
	{
		m_mode = AEM_FIXED;
		return false;
	}

	// 3. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|A|thinkWeaponTake: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getInst()->getWeaponLocation(m_entity, &m_target_pnt))
	{
		// �������� ���� ������ - ������ � ����
		pnt_vec_t base, vec, path;
		AIAPI::getInst()->getBaseField(m_target_pnt, 1, &base);
		AIAPI::getInst()->getReachableField(base, &vec, true);

		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),m_target_pnt, path);
		if(path.empty())
		{
			// � ������ ������ ������� - ������� ������������� ���������
			m_mode = AEM_FIXED;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// ������ ��� - ������� � ������������� ���������
	m_mode = AEM_FIXED;
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ Pursuit
bool AssaultEnemyNode::thinkPursuit(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(assault_cure_dist);

	// 2. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|A|thinkPursuit: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,&enemy,assault_enemy_dist);
			// ������� � ��������� �����
			AIAPI::getInst()->setWalk(m_entity);
			m_mode = AEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� ������� aim, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	SHT_AIMSHOT);
			// ������ ��������� �� ������� � �����
			movpoints -= MPS_FOR_ROTATE*4;
			// ������ ��������� �� ����������
			movpoints -= MPS_FOR_POSE_CHANGE;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// ����� ������� ��� ����� ��� ����, ������ ��������� �
				// �����
				return false;
			}

			pnt_vec_t field, vec, path;
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
			// ������� ������ ��������� ������
			AIAPI::getInst()->getReachableField(field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec, AssaultHexComparator(m_entity, enemy));
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),	m_target_pnt, path);
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 3. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|A|thinkPursuit: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. �������� �� ������ �� �� � �������� �����
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// �� ������
		// 4.1 ��������, �������� �� � �����, ��� ��� ����
		m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->convertPos2ToPos3(m_last_enemy_pnt) - getPosition()));
		float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
		if( fabs(m_target_dir - cur_angle) >= angle_eps )
		{
			// ����� �������������� � �����
			m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity), m_target_dir,	ActivityFactory::CT_ENEMY_ROTATE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 4.2 �������� - ������� ������������ ���������
		AIAPI::getInst()->print(m_entity, "|A|thinkPursuit: ������ �� ������ - ������� ������������ ���������");
		// ��������� � ������������ ���������
		m_mode = AEM_FIXED;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. ���� � �������� �����
	AIAPI::getInst()->print(m_entity, "|A|thinkPursuit: ���� � �������� �����");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	// ����� ����
	AIAPI::getInst()->setRun(m_entity);
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ MedkitTake
bool AssaultEnemyNode::thinkMedkitTake(state_type* st)
{
	if(!MedkitTake(m_target_pnt,st))	m_mode = AEM_FIXED;
	return false;
}

// ����!
bool AssaultEnemyNode::die()
{
	m_mode = AEM_KILLED;
	return need2Delete();
}

// ����� �� ������� ����
bool AssaultEnemyNode::need2Delete() const
{
	if( (m_mode == AEM_KILLED) && !m_activity) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ������������ ��������� �������
//
/////////////////////////////////////////////////////////////////////////////

// ����������� - id ��������
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
    //��������� ��������
    if(m_activity){

        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK)){
			m_activity->Detach(&m_activity_observer);
            delete m_activity;
            m_activity = 0;
        }
        return complexity;
    }

	// ���� �� ��� ��� - �� �����
	if(!st)
	{
		m_turn_state = TS_NONE;
		return complexity;
	}

	// ���� m_turn_state == TS_NONE, �� ��� ������ ������ ����
	if(m_turn_state == TS_NONE)
	{
		m_turn_state = TS_START;
		// ������ �������������
		PanicPlayer::GetInst()->Init(AIAPI::getInst()->getPtr(m_entity));
		m_initialising = true;
	}

	// �������� ���� �������������
	if(m_initialising)
	{
		if(PanicPlayer::GetInst()->Execute())
		{
			// ����� ���������� ������� �������������
			*st = ST_INCOMPLETE;
			return complexity;
		}
		// ������������� ���������
		m_initialising = false;
	}

    // ���� �������� - ����� ��������
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
		// �����������
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());

		st << m_basepnt.x << m_basepnt.y << m_basedir << m_target_dir;
		return;
    }
	// ��������
	int tmp;
	st >> tmp; m_mode = static_cast<FixedTechEnemyMode>(tmp);
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> m_entity;
	st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));

	st >> m_basepnt.x >> m_basepnt.y >> m_basedir >> m_target_dir;
	
	// ��, ��� ����������������� ��� ������
	m_activity = 0;
	m_initialising = false;
}

// ������ ����������� �������
point3 FixedTechEnemyNode::getPosition() const
{
	return AIAPI::getInst()->getPos3(m_entity);
}

//��������� ��������� � �����������
void FixedTechEnemyNode::OnSpawnMsg()
{
	m_basepnt = AIAPI::getInst()->getPos2(m_entity);
	m_basedir = norm_angle(AIAPI::getInst()->getAngle(m_entity));
}

// ��������� ������ Base
bool FixedTechEnemyNode::thinkBase(state_type* st)
{
	// 1. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|FT|thinkBase: � ������ �����!");
			// ������ ������ �������� �����, ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED, &enemy, fixed_tech_enemy_dist);
			// ������� � ��������� �����
			m_mode = FTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 2. ���� ��� ���������� ������� ����� - ������� � �����
	enemies.clear();
	if(AIAPI::getInst()->getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies) )
	{
		// ������� - ������� ������ ��������
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))	)
		{
			AIAPI::getInst()->print(m_entity, "|FT|thinkBase: ��� ������� ������� �����!");
			// ������ ������ �������� �����, ������� � ��������� �����
			m_mode = FTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 3. ������ ���
	AIAPI::getInst()->print(m_entity, "|FT|thinkBase: ������ ���");
	m_turn_state = TS_END;
	return false;
}

// ��������� ������ Attack
bool FixedTechEnemyNode::thinkAttack(state_type* st)
{
	// 1. ������� �����
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// ���� ����� �����
		// ���� ��� ������ ���� ��� ������� - ������� ������ �������������
		if( (m_turn_state == TS_START) ||(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED)	)
		{
			m_ignored_enemies.clear();
		}
		// ��������� ����� �������� ������� ���� ������ � �������������� ������
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// ������� ������ �������� ����� �� ����
			enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		}
		else
		{
			// ������� ������ �������� ����� �� �������������������
			enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list, EnemyComparator(m_entity));
		}
	}
	
	if(!enemy)
	{
		// ���a ����� �� �����
		enemies.clear();
		if(AIAPI::getInst()->getEnemiesVisibleBySubteam(getSubteam()->getName(), &enemies) 	)
		{
			// ���� ����� �� ����������
			// ���� ��� ������ ���� ��� ������� - ������� ������ �������������
			if( (m_turn_state == TS_START) ||(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) 	)
			{
				m_ignored_enemies.clear();
			}
			// ��������� ����� �������� ������� ���� ������ � �������������� ������
			EntityList_t diff_list;
			setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
			if(diff_list.empty())
			{
				// ������� ������ �������� ����� �� ����
				enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies, EnemyComparator(m_entity));
			}
			else
			{
				// ������� ������ �������� ����� �� �������������������
				enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list, EnemyComparator(m_entity));
			}
		}
	}
	
	// 2. ���� ��� ������ - �������� � ������� �����������
	if(!enemy)
	{
		// ������ ��� ������ - �������� � ������� �����������
		AIAPI::getInst()->print(m_entity, "|FT|thinkAttack: ������ ��� - �� ����");
		m_mode = FTEM_ROTATE2BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����...

	// 3. ���� ������� �� ������� ���������� - ��a �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// ����������� - �������� ���
		AIAPI::getInst()->print(m_entity, "|FT|thinkAttack: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. ���� ��� ����� ���� - ��������� ����� � ������ �������������
	// ������, ���� ����� ����� ��� ����� ��������� � ������ �������������
	// �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// ����������� - �������� ���
		AIAPI::getInst()->print(m_entity, "|FT|thinkAttack: ��� ����� ���� - ��������� ����� � ������ �������������");
		m_activity_observer.clear();
		// ������� � ������ �������������
		m_ignored_enemies.push_back(enemy);
		m_ignored_enemies.unique();
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// �������� ���
			m_turn_state = TS_END;
			return false;
		}
		// ���������� ��� ������ ������ ������
		m_turn_state = TS_INPROGRESS;
		return true;
	}


	// 5 ���� �� �������� � ����� - �������������� � ����������� ��� ��� ����
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// ����� �������������� � �����
		m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir, 	ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 6. ��������a � ����� - ���� ������ ����� ���������
	AIAPI::getInst()->print(m_entity, "|FT|thinkAttack: ����� ��������, ���� ��� ��������!");
	// ���� ���� - ��������� �� ���� ���� ��� ��������
	std::string reason;
	if(AIAPI::getInst()->shootByVehicle(m_entity, enemy, &m_activity, &reason))
	{
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
	}
	else
	{
		std::string str = "|FT|thinkAttack: �������� ������: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

// ��������� ������ Rotate2Base
bool FixedTechEnemyNode::thinkRotate2Base(state_type* st)
{
	// 1. �������� ���������� �� ������� � ������� �����������
	float cur_angle = AIAPI::getInst()->getAngle(m_entity);
	if( fabs(m_basedir - cur_angle) < angle_eps )
	{
		// ���������� - �������� � ������� ���������
		AIAPI::getInst()->print(m_entity, "|FT|thinkRotate2Base: ����������");
		m_mode = FTEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 2. �������� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// ����������� - �������� ���
		AIAPI::getInst()->print(m_entity, "|FT|thinkRotate2Base: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. �������� � ������� �����������
	AIAPI::getInst()->print(m_entity, "|FT|thinkRotate2Base: ���������������...");
	m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_basedir,	ActivityFactory::CT_ENEMY_ROTATE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ����!
bool FixedTechEnemyNode::die()
{
	m_mode = FTEM_KILLED;
	return need2Delete();
}

// ����� �� ������� ����
bool FixedTechEnemyNode::need2Delete() const
{
	if( (m_mode == FTEM_KILLED) && !m_activity) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ���������� ��������� �������
//
/////////////////////////////////////////////////////////////////////////////

// ����������� - id ��������
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
    //��������� ��������
    if(m_activity){

        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK)){
			m_activity->Detach(&m_activity_observer);
            delete m_activity;
            m_activity = 0;
        }
        return complexity;
    }

	// ���� �� ��� ��� - �� �����
	if(!st)
	{
		m_turn_state = TS_NONE;
		return complexity;
	}

	// ���� m_turn_state == TS_NONE, �� ��� ������ ������ ����
	if(m_turn_state == TS_NONE)
	{
		m_turn_state = TS_START;
		// ������ �������������
		PanicPlayer::GetInst()->Init(AIAPI::getInst()->getPtr(m_entity));
		m_initialising = true;
	}

	// �������� ���� �������������
	if(m_initialising)
	{
		if(PanicPlayer::GetInst()->Execute())
		{
			// ����� ���������� ������� �������������
			*st = ST_INCOMPLETE;
			return complexity;
		}
		// ������������� ���������
		m_initialising = false;
	}

    // ���� �������� - ����� ��������
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
		// �����������
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
	// ��������
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
	
	// ��, ��� ����������������� ��� ������
	m_activity = 0;
	m_initialising = false;
}

// ��������� ��������������� ���������
void PatrolTechEnemyNode::recieveMessage(MessageType type, void * data)
{
	eid_t id = *static_cast<eid_t*>(data);
	if(type == MT_ENEMYSPOTTED)
	{
		if(m_enemy_id)
		{
			// ��� �������� � �����
			// �������� ��� �� ������ �����
			float old_dist = dist(AIAPI::getInst()->getPos2(m_entity),
				m_enemy_pnt);
			float new_dist = dist(AIAPI::getInst()->getPos2(m_entity),
				AIAPI::getInst()->getPos2(id));
			if(new_dist < old_dist)
			{
				// ������ �����
				m_enemy_id = id;
				m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
			}
			return;
		}
		// ��� ������ ����, � ������� ��������
		m_enemy_id = id;
		m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
		return;
	}
}

// ������ ����������� �������
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

// ��������� ��������� �� �������� ��� ���������
void PatrolTechEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// �� ����� ����������� � ������,
	// ���� ������� ��������� � ������������ ����������
	if(m_mode == PTEM_ATTACK) return;

	if(AIAPI::getInst()->getPtr(m_entity) == who) return; // ��� � ��� ��������

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (AIAPI::getInst()->getPtr(m_entity) == victim))
	{
		// ����� �����������
		m_shoot_pnt = AIAPI::getInst()->getPos2(who);
		m_shooted = true;
	}
}

// ��������� ������ Patrol
bool PatrolTechEnemyNode::thinkPatrol(state_type* st)
{
	// 1. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: � ������� �����!");
			// ������a ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_tech_enemy_dist);
			// ������� � ��������� �����
			m_mode = PTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� �������, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
				/*�� ���� �������� ������*/SHT_AIMSHOT);
			// ������ ��������� �� ������� � �����
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// ����� ������� ��� ����� ��� ����, ������ ��������� �
				// �����
				return false;
			}
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
				steps, &field);
			// ������� ������ ��������� ������
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,
				WalkingTechHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
				m_target_pnt, path);
			// ����� ����
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

	// 2. ���� � ���� �������� - ����� ������������
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: � ���� �������� - ����� ������������");
		// � ���� ��� ����� �� ���� ��������
		m_shooted = false;
		// ������� ����� � ������� 3 ������ �� ��������, ������� ��������
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_shoot_pnt,
			3, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
			m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 3. ���� ��� �������� � ����� - ������ � ����
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: ��� �������� � �����");
		// ��������
		m_enemy_id = 0;
		// ������� ����� � ������� 1 ����� �� �����
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,
			1, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
			m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. ���� ����� �� �������� ����� - �������� �����
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: ����� �� �������� ����� - �������� �����");
		// ���������� ���� ������������
		pnt_vec_t allhexes;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, &allhexes, 0);
		if(allhexes.empty())
		{
			// �������� ��� - ���� ������
			AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: ��� ������ ������!!!");
			m_turn_state = TS_END;
			return false;
		}
		// ������� ��������� �����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(allhexes);
	}

	// 5. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. ���� � �������� �����
	AIAPI::getInst()->print(m_entity, "|PT|thinkPatrol: ���� � �������� �����");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
		0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
		path,
		ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ Attack
bool PatrolTechEnemyNode::thinkAttack(state_type* st)
{
	// 1. ���� � ������� ��� ������ - �������� � �������������
	if(!AIAPI::getInst()->haveTechWeapon(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: ��� ������ - ����� ������������");
		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		return false;
	}

	// 2. ������� �����
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// ��� ����� �����

		// ���� ��� ������ ���� ��� ������� - ������� ������ �������������
		if(m_turn_state == TS_START)
		{
			m_ignored_enemies.clear();
		}
		// ��������� ����� �������� ������� ���� ������ � �������������� ������
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// ������� ������ �������� ����� �� ����
			enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
				EnemyComparator(m_entity));
		}
		else
		{
			// ������� ������ �������� ����� �� �������������������
			enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list,
				EnemyComparator(m_entity));
		}
		m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
	}
	
	// 3. ���� ��� ������ - ��������� � ��������� �������������
	if(!enemy)
	{
		// ������ ��� - ����� ������������ ����������
		AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: ������ ��� - ����� ������������ ����������");
		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		return false;
	}
	// ����� ����...

	// 4. ���� ��� ������ ������ ���� ��� ��������� �������
	if(m_turn_state == TS_START)
	{
		// �������� ������ ����� � ���� � ���
		AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: �������� ������ ����� � ���� � ���");

		// ������� ������ ���� ���, ����� ����� ������� ����������
		// �� ���� �������, ���� ����� ������ ��� - �������
		// ������ ����, � ������� �����.
		
		m_turn_state = TS_INPROGRESS;
		// ������� ����� ���-�� ���������� ����������
		int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
		// ������ ���������� ���������� ����������� ��� ��������
		movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
			/*������ �� ���� ��������*/SHT_AIMSHOT);
		// ������ ��������� �� ������� � �����
		movpoints -= MPS_FOR_ROTATE*4;
		int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
		if(!steps)
		{
			// ����� ������� ��� ����� ��� ����, ������ ��������� �
			// �����
			return false;
		}
		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
			steps, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec);
		// ������� ������ ����
		m_target_pnt = AIAPI::getInst()->getBestPoint(vec,
			WalkingTechHexComparator(m_entity, enemy));
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
			m_target_pnt, path);
		// ����� ����
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

	// 5. ��� ����������� ����

	// 5.1 ���� �� ������� ���������� - �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// ����������� - �������� ���
		AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 5.2 ���� ��������� ������� ��� ������ ������ ����� - ���������
	//       ������ ����
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)
		)
	{
		AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: ��������� ������� ��� ������ ������ �����");
		m_activity_observer.clear();
		// ��������� ������ ����
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 5.3 ���� ��� ����� ���� - ��������� ����� � ������ �������������
	// ������, ���� ����� ����� ��� ����� ��������� � ������ �������������
	// ��������� ������� � ������ �� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// ����������� - �������� ���
		AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: ��� ����� ���� - ��������� ����� � ������ �������������");
		m_activity_observer.clear();
		// ������� � ������ �������������
		m_ignored_enemies.push_back(enemy);
		m_ignored_enemies.unique();
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// ��������� ������� ������� � �����
			AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: �������� �������");
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
				0, 0, PathUtils::F_CALC_NEAR_PNTS);
			EntityList_t::iterator i = enemies.begin();
			// ������ ��������, � �������� ����� �������
			bool found = false;
			while(i != enemies.end())
			{
				// ��������, ���������� �� ����� ����� � ���������
				if(PathUtils::GetInst()->IsNear(AIAPI::getInst()->getPtr(*i)))
				{
					// � ����� ����� �������
					found = true;
					m_target_pnt = PathUtils::GetInst()->GetNearPnt(AIAPI::getInst()->getPtr(*i)).m_pnt;
					break;
				}
				++i;
			}
			pnt_vec_t path;
			if(!found)
			{
				// ����������� ����� ��� - ������������ ��������� �������
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
				// ���� ����, � �������� ����� ������� - �������� �� �������� ����
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
					m_target_pnt, path);
				// ������� �������� ����
				path.erase(path.begin(), path.begin() + (path.size() / 2));
			}
			if(path.empty())
			{
				// ��������� ���
				m_turn_state = TS_END;
				return false;
			}
			// ����� ����
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
		// ���������� ��� ������ ������ ������
		m_turn_state = TS_INPROGRESS;
		return true;
	}

	// 5.4 ���� �� ��������a � ����� - �������������� � ����������� ��� ��� ����
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// ����� �������������� � �����
		m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),
			m_target_dir,
			ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 5.5 ��������a � ����� - ���� ������ ����� ���������
	AIAPI::getInst()->print(m_entity, "|PT|thinkAttack: ����� ��������, ���� ��� ��������!");
	// ���� ���� - ��������� �� ���� ���� ��� ��������
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
		std::string str = "|PT|thinkAttack: �������� ������: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

// ��������� ������ Pursuit
bool PatrolTechEnemyNode::thinkPursuit(state_type* st)
{
	// 1. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::getInst()->print(m_entity, "|PT|thinkPursuit: � ������a �����!");
			// ������a ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_tech_enemy_dist);
			// ������� � ��������� �����
			m_mode = PTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� �������, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
				/*������ �� ���� ��������*/SHT_AIMSHOT);
			// ������ ��������� �� ������� � �����
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// ����� ������� ��� ����� ��� ����, ������ ��������� �
				// �����
				return false;
			}
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),
				steps, &field);
			// ������� ������ ��������� ������
			pnt_vec_t vec;
			AIAPI::getInst()->getReachableField(field, &vec);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec,
				WalkingTechHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
				m_target_pnt, path);
			// ����� ����
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

	// 2. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|PT|thinkPursuit: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. �������� �� ������ �� �� � �������� �����
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// �� ������
		// 3.1 ��������, ��������a �� � �����, ��� ��� ����
		m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->convertPos2ToPos3(m_last_enemy_pnt) - getPosition()));
		float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
		if( fabs(m_target_dir - cur_angle) >= angle_eps )
		{
			// ����� �������������� � �����
			m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),
				m_target_dir,
				ActivityFactory::CT_ENEMY_ROTATE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 3.2 ��������a - ������� � ��������� ��������������
		AIAPI::getInst()->print(m_entity, "|PT|thinkPursuit: ������ �� ������a - ����� �������������");
		// ��������� � ��������� ��������������
		m_mode = PTEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. ���� � �������� �����
	AIAPI::getInst()->print(m_entity, "|PT|thinkPursuit: ���� � �������� �����");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),
		0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),
		path,
		ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ����!
bool PatrolTechEnemyNode::die()
{
	m_mode = PTEM_KILLED;
	return need2Delete();
}

// ����� �� ������� ����
bool PatrolTechEnemyNode::need2Delete() const
{
	if( (m_mode == PTEM_KILLED) && !m_activity) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ��������� ��������� �������
//
/////////////////////////////////////////////////////////////////////////////

// ����������� - id ��������
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
    //��������� ��������
    if(m_activity){

        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK)){
			m_activity->Detach(&m_activity_observer);
            delete m_activity;
            m_activity = 0;
        }
        return complexity;
    }

	// ���� �� ��� ��� - �� �����
	if(!st)
	{
		m_turn_state = TS_NONE;
		return complexity;
	}

	// ���� m_turn_state == TS_NONE, �� ��� ������ ������ ����
	if(m_turn_state == TS_NONE)
	{
		m_turn_state = TS_START;
		// ������ �������������
		PanicPlayer::GetInst()->Init(AIAPI::getInst()->getPtr(m_entity));
		m_initialising = true;
	}

	// �������� ���� �������������
	if(m_initialising)
	{
		if(PanicPlayer::GetInst()->Execute())
		{
			// ����� ���������� ������� �������������
			*st = ST_INCOMPLETE;
			return complexity;
		}
		// ������������� ���������
		m_initialising = false;
	}

    // ���� �������� - ����� ��������
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
		// �����������
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
	// ��������
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
	
	// ��, ��� ����������������� ��� ������
	m_activity = 0;
	m_initialising = false;
}

// ��������� ��������������� ���������
void AssaultTechEnemyNode::recieveMessage(MessageType type, void * data)
{
	eid_t id = *static_cast<eid_t*>(data);
	if(type == MT_ENEMYSPOTTED)
	{
		if(m_enemy_id)
		{
			// ��� �������� � �����
			// �������� ��� �� ������ �����
			float old_dist = dist(AIAPI::getInst()->getPos2(m_entity), m_enemy_pnt);
			float new_dist = dist(AIAPI::getInst()->getPos2(m_entity), AIAPI::getInst()->getPos2(id));
			if(new_dist < old_dist)
			{
				// ������ �����
				m_enemy_id = id;
				m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
			}
			return;
		}
		// ��� ������ ����, � ������� ��������
		m_enemy_id = id;
		m_enemy_pnt = AIAPI::getInst()->getPos2(m_enemy_id);
		return;
	}
}

// ������ ����������� �������
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

// ��������� ��������� �� �������� ��� ���������
void AssaultTechEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// �� ����� ����������� � ������,
	// ���� ������� ��������� � ������������ ����������
	if(m_mode == ATEM_ATTACK) return;

	BaseEntity* be = AIAPI::getInst()->getPtr(m_entity);
	if( be && (be == who) ) return; // ��� � ��� ��������

	float dist = (getPosition() - where).Length();
	if((dist < alarm_dist) || (be == victim) )
	{
		// ����� �����������
		m_shoot_pnt = AIAPI::getInst()->getPos2(who);
		m_shooted = true;
	}
}

// ��������� ������ Patrol
bool AssaultTechEnemyNode::thinkPatrol(state_type* st)
{
	// 1. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: � ������� �����!");
			// ������a ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED, &enemy, assault_tech_enemy_dist);
			// ������� � ��������� �����
			m_mode = ATEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� �������, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	/*�� ���� �������� ������*/SHT_AIMSHOT);
			// ������ ��������� �� ������� � �����
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// ����� ������� ��� ����� ��� ����, ������ ��������� �, �����
				return false;
			}
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			pnt_vec_t field, vec, path;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
			// ������� ������ ��������� ������
			AIAPI::getInst()->getReachableField(field, &vec);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec, WalkingTechHexComparator(m_entity, enemy));
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 2. ���� � ���� �������� - ����� ������������
	if(m_shooted)
	{
		AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: � ���� �������� - ����� ������������");
		// � ���� ��� ����� �� ���� ��������
		m_shooted = false;
		// ������� ����� � ������� 3 ������ �� ��������, ������� ��������
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field, vec, path;
		AIAPI::getInst()->getBaseField(m_shoot_pnt,	3, &field);
		// ������� ������ ��������� ������
		AIAPI::getInst()->getReachableField(field, &vec);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = ATEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 3. ���� ��� �������� � ����� - ������ � ����
	if(m_enemy_id)
	{
		AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: ��� �������� � �����");
		// ��������
		m_enemy_id = 0;
		// ������� ����� � ������� 1 ����� �� �����
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getInst()->getBaseField(m_enemy_pnt,	1, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(field, &vec);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = ATEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. ���� ����� �� �������� ����� - �������� �����
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: ����� �� �������� ����� - �������� �����");
		// ���������� ���� ������������
		pnt_vec_t allhexes;
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, &allhexes, 0);
		if(allhexes.empty())
		{
			// �������� ��� - ���� ������
			AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: ��� ������ ������!!!");
			m_turn_state = TS_END;
			return false;
		}
		// ������� ��������� �����
		m_target_pnt = AIAPI::getInst()->getRandomPoint(allhexes);
	}

	// 5. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. ���� � �������� �����
	AIAPI::getInst()->print(m_entity, "|AT|thinkPatrol: ���� � �������� �����");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ��������� ������ Attack
bool AssaultTechEnemyNode::thinkAttack(state_type* st)
{
	// 1. ���� � ������� ��� ������ - �������� � �������������
	if(!AIAPI::getInst()->haveTechWeapon(m_entity))
	{
		AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: ��� ������ - ����� ������������");
		m_mode = ATEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		return false;
	}

	// 2. ������� �����
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// ���a ����� �����

		// ���� ��� ������ ���� ��� ������� - ������� ������ �������������
		if(m_turn_state == TS_START)
			m_ignored_enemies.clear();
		// ��������� ����� �������� ������� ���� ������ � �������������� ������
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// ������� ������ �������� ����� �� ����
			enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies, EnemyComparator(m_entity));
		}
		else
		{
			// ������� ������ �������� ����� �� �������������������
			enemy = AIAPI::getInst()->getMostDangerousEnemy(diff_list, EnemyComparator(m_entity));
		}
		m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);
	}
	
	// 3. ���� ��� ������ - ��������� � ��������� �������������
	if(!enemy)
	{
		// ������ ��� - ����� ������������ ����������
		AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: ������ ��� - ����� ������������ ����������");
		m_mode = ATEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		return false;
	}
	// ����� ����...

	// 3. ���� ��� ������ ������ ���� ��� ��������� �������
	if(m_turn_state == TS_START)
	{
		m_activity_observer.clear();
		// �������� ������ ����� � ���� � ���
		AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: �������� ������ ����� � ���� � ���");

		// ������� ������ ���� ���, ����� ����� ������� ����������
		// �� ���� �������, ���� ����� ������ ��� - �������
		// ������ ����, � ������� �����.
		
		m_turn_state = TS_INPROGRESS;
		// ������� ����� ���-�� ���������� ����������
		int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
		// ������ ���������� ���������� ����������� ��� ��������
		movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,	/*������ �� ���� ��������*/SHT_AIMSHOT);
		// ������ ��������� �� ������� � �����
		movpoints -= MPS_FOR_ROTATE*4;
		int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
		if(!steps)
		{
			// ����� ������� ��� ����� ��� ����, ������ ��������� �, �����
			return false;
		}
		// ���������� ���� ������������
		PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field, vec, path;
		AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
		// ������� ������ ��������� ������
		AIAPI::getInst()->getReachableField(field, &vec);
		// ������� ������ ����
		m_target_pnt = AIAPI::getInst()->getBestPoint(vec, WalkingTechHexComparator(m_entity, enemy));
		PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
		return false;
	}

	// 5. ��� ����������� ����

	// 5.1 ���� �� ������� ���������� - �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// ����������� - �������� ���
		AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 5.2 ���� ��������� ������� ��� ������ ������ ����� - ���������
	//       ������ ����
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)
		)
	{
		AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: ��������� ������� ��� ������ ������ �����");
		m_activity_observer.clear();
		// ��������� ������ ����
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 5.3 ���� ��� ����� ���� - ��������� ����� � ������ �������������
	// ������, ���� ����� ����� ��� ����� ��������� � ������ �������������
	// ��������� ������� � ������ �� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// ����������� - �������� ���
		AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: ��� ����� ����");
		m_activity_observer.clear();
		// ������� � ������ �������������
		m_ignored_enemies.push_back(enemy);
		m_ignored_enemies.unique();
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// ��������� ������� ������� � �����
			AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: �������� �������");
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, PathUtils::F_CALC_NEAR_PNTS);
			EntityList_t::iterator i = enemies.begin();
			// ������ ��������, � �������� ����� �������
			bool found = false;
			while(i != enemies.end())
			{
				// ��������, ���������� �� ����� ����� � ���������
				if(PathUtils::GetInst()->IsNear(AIAPI::getInst()->getPtr(*i)))
				{
					// � ����� ����� �������
					found = true;
					m_target_pnt = PathUtils::GetInst()->GetNearPnt(AIAPI::getInst()->getPtr(*i)).m_pnt;
					break;
				}
				++i;
			}
			pnt_vec_t path;
			if(!found)
			{
				// ����������� ����� ��� - ������������ ��������� �������
				pnt_vec_t base, vec;
				AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	patrol_enemy_noLOF_move,&base);
				AIAPI::getInst()->getReachableField(base, &vec, true);
				m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
			}
			else
			{
				// ���� ����, � �������� ����� ������� - �������� �� �������� ����
				PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
				// ������� �������� ����
				path.erase(path.begin(), path.begin() + (path.size() / 2));
			}
			if(path.empty())
			{
				// ��������� ���
				m_turn_state = TS_END;
				return false;
			}
			// ����� ����
			AIAPI::getInst()->setWalk(m_entity);
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
			
			m_turn_state = TS_INPROGRESS;
			return false;
		} // if(diff_list.empty())
		// ���������� ��� ������ ������ ������
		m_turn_state = TS_INPROGRESS;
		return true;
	}

	// 5.4 ���� �� ��������a � ����� - �������������� � ����������� ��� ��� ����
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// ����� �������������� � �����
		m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity), m_target_dir,	ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 5.5 ��������a � ����� - ���� ������ ����� ���������
	AIAPI::getInst()->print(m_entity, "|AT|thinkAttack: ����� ��������, ���� ��� ��������!");
	// ���� ���� - ��������� �� ���� ���� ��� ��������
	std::string reason;
	if(AIAPI::getInst()->shootByVehicle(m_entity, enemy, &m_activity, &reason))
	{
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
	}
	else
	{
		std::string str = "|AT|thinkAttack: �������� ������: ";
		str += reason;
		AIAPI::getInst()->print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

// ��������� ������ Pursuit
bool AssaultTechEnemyNode::thinkPursuit(state_type* st)
{
	// 1. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getInst()->getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getInst()->getMostDangerousEnemy(enemies, EnemyComparator(m_entity)))
		{
			AIAPI::getInst()->print(m_entity, "|AT|thinkPursuit: � ������a �����!");
			// ������a ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED, &enemy, assault_tech_enemy_dist);
			// ������� � ��������� �����
			m_mode = ATEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getInst()->getPos2(enemy);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� �������, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getInst()->getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getInst()->getShootMovePoints(m_entity,
				/*������ �� ���� ��������*/SHT_AIMSHOT);
			// ������ ��������� �� ������� � �����
			movpoints -= MPS_FOR_ROTATE*4;
			int steps = AIAPI::getInst()->getSteps(m_entity, movpoints);
			if(!steps)
			{
				// ����� ������� ��� ����� ��� ����, ������ ��������� �
				// �����
				return false;
			}
			// ���������� ���� ������������
			PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
			pnt_vec_t field, vec, path;
			AIAPI::getInst()->getBaseField(AIAPI::getInst()->getPos2(m_entity),	steps, &field);
			// ������� ������ ��������� ������
			AIAPI::getInst()->getReachableField(field, &vec);
			// ������� ������ ����
			m_target_pnt = AIAPI::getInst()->getBestPoint(vec, WalkingTechHexComparator(m_entity, enemy));
			PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
			// ����� ����
			m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 2. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::getInst()->print(m_entity, "|AT|thinkPursuit: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. �������� �� ������ �� �� � �������� �����
	if(m_target_pnt == AIAPI::getInst()->getPos2(m_entity))
	{
		// �� ������
		// 3.1 ��������, ��������a �� � �����, ��� ��� ����
		m_target_dir = norm_angle(Dir2Angle(AIAPI::getInst()->convertPos2ToPos3(m_last_enemy_pnt) - getPosition()));
		float cur_angle = norm_angle(AIAPI::getInst()->getAngle(m_entity));
		if( fabs(m_target_dir - cur_angle) >= angle_eps )
		{
			// ����� �������������� � �����
			m_activity = ActivityFactory::GetInst()->CreateRotate(AIAPI::getInst()->getPtr(m_entity),m_target_dir,	ActivityFactory::CT_ENEMY_ROTATE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 3.2 ��������a - ������� � ��������� ��������������
		AIAPI::getInst()->print(m_entity, "|AT|thinkPursuit: ������ �� ������a - ����� �������������");
		// ��������� � ��������� ��������������
		m_mode = ATEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. ���� � �������� �����
	AIAPI::getInst()->print(m_entity, "|AT|thinkPursuit: ���� � �������� �����");
	PathUtils::GetInst()->CalcPassField(AIAPI::getInst()->getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(AIAPI::getInst()->getPtr(m_entity), m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getInst()->getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����
	m_activity = ActivityFactory::GetInst()->CreateMove(AIAPI::getInst()->getPtr(m_entity),	path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// ����!
bool AssaultTechEnemyNode::die()
{
	m_mode = ATEM_KILLED;
	return need2Delete();
}

// ����� �� ������� ����
bool AssaultTechEnemyNode::need2Delete() const
{
	if( (m_mode == ATEM_KILLED) && !m_activity) return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////////
//
// ������������ ���� ��� ����� - ��� �������-������ ����� �����
//
//////////////////////////////////////////////////////////////////////////////
namespace
{
	// ������������ ���� �� ���� �� ���� ��
	float norm_angle(float angle)
	{
		float fl = fmod(angle, PIm2);
		if(fl < 0) fl += PIm2;
		return fl;
	}
	// �������� � ���� ������ �������� (�� ������ ��������) ����� ����� �����
	// ������� �������� eid_t
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
				// �� ���� ������� ���� � ������ ������ � ����������� �� ������
				(*outlist).push_back(*i);
			}
			++i;
		}
	}
};
