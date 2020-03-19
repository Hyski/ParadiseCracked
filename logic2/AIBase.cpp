/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: �������� ��������� AI
                                                                                
                                                                                
   Author:   Alexander Garanin (Punch)
				Mikhail L. Lepakhin (Flif)
***************************************************************/                
#pragma warning(disable:4786)

#include "logicdefs.h"
#include "PathUtils.h"
#include "HexUtils.h"
#include "HexGrid.h"
#include "Activity.h"
#include "AIBase.h"
#include "AIUtils.h"
#include "thing.h"
#include "aicontext.h"
#include "strategy.h"
#include "Entity.h"
#include "player.h"
#include "DirtyLinks.h"
#include "Graphic.h"
#include "EnemyAI.h"
#include "CivilianAI.h"
#include "xlsreader.h"
#include "..\Interface\Console.h"

DCTOR_IMP(BusNode)
DCTOR_IMP(CivilRoot)
DCTOR_IMP(EnemyRoot)
DCTOR_IMP(FixedNPCNode)
DCTOR_IMP(FeralCarNode)
DCTOR_IMP(CivilianNode)

AIFactory AIFactory::m_factory;

namespace
{
    const char* ai_xls_name = "scripts/spawn/ai.txt";
    enum ai_column_type
	{
         ACT_LABEL,
         ACT_TYPE,
         ACT_SUBTEAM,
            
         ACT_MAX_COLUMNS,            
    };

    column ai_columns[ACT_MAX_COLUMNS] = 
    {
        column(  "label",    ACT_LABEL),
        column(   "type",    ACT_TYPE),
        column("subteam",    ACT_SUBTEAM),
    };

	// �������� �� ����� �������� ���� AI
	void getAINodeDesc(const std::string& label, std::string& type, std::string& subteam);
	// ������� � �������
	void Print(const std::string& str);
};

//=========================================

AINode* AIFactory::CreateCivilianAI()
{
    return new CivilRoot();
}

AINode* AIFactory::CreateEnemyAI()
{
    return new EnemyRoot();
}

//=========================================

CivilRoot::CivilRoot() 
{
    m_cur = m_nodes.end();
    
    Spawner::GetInst()->Attach(this, ET_ENTITY_SPAWN);
    GameEvMessenger::GetInst()->Attach(this, EV_TURN);
	GameEvMessenger::GetInst()->Attach(this, EV_KILL);

	m_complexity = 0.0f;
}

CivilRoot::~CivilRoot()
{
    Spawner::GetInst()->Detach(this);
    GameEvMessenger::GetInst()->Detach(this);

    node_lst_t::iterator itor = m_nodes.begin();
    while(itor != m_nodes.end()){
        delete *itor;
        ++itor;
    }
}

float CivilRoot::Think(state_type* st)
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
    if(m_cur == m_nodes.end()){     
        *st = ST_FINISHED;
		Print("CivilRoot: ��� ������ ������� ��������.");
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

void CivilRoot::Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info)
{
    BaseEntity* entity = static_cast<spawn_info_s*>(info)->m_entity;

    if(entity && entity->GetPlayer() == PT_CIVILIAN){

        if(AINode* node = CreateNode(entity))
            m_nodes.push_back(node);
    }
}

void CivilRoot::Update(GameObserver::subject_t subj, GameObserver::event_t type, GameObserver::info_t ptr)
{
    switch(type)
	{
    case EV_TURN:
        {
			if(static_cast<turn_info*>(ptr)->m_player == PT_CIVILIAN)
			{
				m_cur = m_nodes.begin();
				m_complexity = 0.0f;
			}
        }
        break;

    case EV_KILL:
        {
			kill_info* info = static_cast<kill_info*>(ptr);
			deleteChildren(info->m_victim->GetEID());
        }
        break;
    }
}

void CivilRoot::MakeSaveLoad(SavSlot& st)
{
    //��������/����������� ��������� AI
    if(st.IsSaving()){
        
        node_lst_t::iterator itor = m_nodes.begin();

        while(itor != m_nodes.end()){
            st << 1;

            DynUtils::SaveObj(st, *itor);
            (*itor)->MakeSaveLoad(st);

            ++itor;
        }

        st << 0;
        
    }else{

        int     flag;
        AINode* node;

        st >> flag;
        while(flag){
            
            DynUtils::LoadObj(st, node);
            node->MakeSaveLoad(st);

            m_nodes.push_back(node);
            st >> flag;
        }
    }
}

void CivilRoot::CutWord(std::string& word, std::string& str)
{
    //������ �������
	int k;
    for(k = 0; k < str.size() && (isspace(str[k]) || str[k] == ')' || str[k] == '(' || str[k] == ','); k++);
    str.erase(str.begin(), str.begin()+k);

    word.clear();
    for(k = 0; k < str.size() && iscsym(str[k]); k++){
        word += str[k];
    }

    str.erase(str.begin(), str.begin()+k);
}

void CivilRoot::ToLower(std::string& str)
{
    for(int i = 0; i < str.size(); i++){
        str[i] = tolower(str[i]);
    }
}

AINode* CivilRoot::CreateNode(BaseEntity* entity)
{
	std::string param;
	std::string subteam;
	getAINodeDesc(entity->GetEntityContext()->GetAIModel(),
		param,
		subteam);
	
	if((subteam != "none") && (subteam != ""))
	{
        entity->SetAIContext(AIContextFactory::GetInst()->Create(AIContextFactory::AI_CIVILIAN, subteam));
	}
    
    if(param.empty()) return 0;
	
    ToLower(param);
	
    //���� �������, �� ���� ������� ���� ����������, ���� ��������� ���,
	// ���� �������� ���, ���� ��������� ���
    if(entity->Cast2Vehicle())
	{
        
        std::string schema;
        CutWord(schema, param);
		
        if(schema == "bus"){
			
            //������ �������
            std::string line;
            CutWord(line, param);
			
            //������ ��� ��������
            std::string type;
            CutWord(type, param);
			
            return new BusNode(entity->GetEID(), DirtyLinks::CreateBusRoute(line), type == "nonstop" ?  BusNode::BT_BUSY : BusNode::BT_USUAL);                
        }
		
        if(schema == "car"){
            std::string line;
            CutWord(line, param);
			
            return new FeralCarNode(entity->GetEID(), DirtyLinks::CreateCheckPoints(line));
        }
		
        if(schema == "a_bot"){
            return new AdvertisingBotNode(entity->GetEID());
        }
		
        if(schema == "t_bot"){
            return new TradingBotNode(entity->GetEID());
        }
		
		if(schema == "r_bot"){
            return new RepairBotNode(entity->GetEID());
        }
	}
	
    //���� ������� �� ���� ������ ���� ������ ��� �����.
    if(entity->Cast2Trader()){
		
        std::string schema;
        CutWord(schema, param);
		
        if(schema == "fixednpc") return new FixedNPCNode(entity->GetEID());
    }
	
    //���� �������, �� ���� �������, ���� �������������, ���� �������������
    if(entity->Cast2Human()){
		
        std::string schema;
        CutWord(schema, param);
		
        if(schema == "setednpc"){
            std::string line;
            CutWord(line, param);
			
            return new CivilianNode(entity->GetEID(), DirtyLinks::CreateCheckPoints(line));
        }
        
        if(schema == "randomnpc"){
            std::string radius;
            CutWord(radius, param);
			
            return new CivilianNode(entity->GetEID(), DirtyLinks::CreateCirclePoints(entity, atoi(radius.c_str())));
        }
		
        if(schema == "fixednpc") return new FixedNPCNode(entity->GetEID());

		if(schema == "indifferent") return new IndifferentNode(entity->GetEID());
    }
	
    return 0;
}

// ������ ������ ��������� ����
float CivilRoot::getComplexity() const
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
void CivilRoot::deleteChildren(eid_t ID)
{
	if(!ID) return;
	node_lst_t::iterator i = m_nodes.begin();
	node_lst_t::iterator end = m_nodes.end();
	while(i != end)
	{
		AINode* node = (*i);
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
		AINode* node = (*i);
		node->deleteChildren(ID);
		i++;
	}
}

//========================= EnemyRoot =================================

EnemyRoot::EnemyRoot()
{
    m_cur = m_subteams.end();

    GameEvMessenger::GetInst()->Attach(this, EV_TURN);
	GameEvMessenger::GetInst()->Attach(this, EV_KILL);
    Spawner::GetInst()->Attach(this, ET_ENTITY_SPAWN);

	m_complexity = 0.0f;
}

EnemyRoot::~EnemyRoot()
{
    Spawner::GetInst()->Detach(this);
    GameEvMessenger::GetInst()->Detach(this);

    node_map_t::iterator itor = m_subteams.begin();
    while(itor != m_subteams.end())
	{
        delete (*itor).second;
        ++itor;
    }
}

float EnemyRoot::Think(state_type* st)
{
    node_map_t::iterator itor = m_subteams.begin();
    while(itor != m_subteams.end())
	{
		// ������� ������� ����
		if((*itor).second->need2Delete())
		{
			// ������ ���� ����
			if(itor == m_cur) m_cur++;
			node_map_t::iterator tmp = itor;
			itor++;
			delete (*tmp).second;
			m_subteams.erase(tmp);
			continue;
		}
        if(itor != m_cur) (*itor).second->Think(0);
        ++itor;
    }

    if(st == 0) return m_complexity;

    //���� ������ ��� ���� �����
    if(m_cur == m_subteams.end()){     
        *st = ST_FINISHED;
		Print("EnemyRoot: ��� ������ ��������.");
        return m_complexity;
    }

    //����� �������� �������� ����
    float tmp_complexity = (*m_cur).second->Think(st);

    //���� ���� �������� �������� � ����������
    if(*st == ST_FINISHED)
	{
		m_complexity += (*m_cur).second->getComplexity();
		tmp_complexity = 0.0f;
        ++m_cur;
	}
    
    *st = ST_INCOMPLETE;
	return m_complexity + tmp_complexity;
}
    
//��������� ����������� ���������
void EnemyRoot::Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info)
{
    BaseEntity* entity = static_cast<spawn_info_s*>(info)->m_entity;
	
    if(entity && entity->GetPlayer() == PT_ENEMY)
	{
		// ���� ������ ��� ������� ������� ��� ��������������
        std::string type;
        std::string subteam;
        getAINodeDesc(entity->GetEntityContext()->GetAIModel(),
            type,
            subteam);
        
        // �������� ����� ���������������� ����
        if(AIEnemyNode* node = CreateNode(entity, type))
		{
            // ��������, ���������� �� ����������, � ������� ����� ���������� ����
            node_map_t::iterator it = m_subteams.find(subteam);
            SubteamNode* sn = 0;
            if(it == m_subteams.end())
            {
                // ����� ������� ��� ��� - �������� ��
                sn = new SubteamNode;
                sn->setName(subteam);
                m_subteams[subteam] = sn;
            }
            else
            {
                sn = (*it).second;
            }
            
            // ������ ������� ����� ����������, ������� ����� � ��� �������
            entity->SetAIContext(AIContextFactory::GetInst()->Create(AIContextFactory::AI_CIVILIAN, subteam));
            node->setSubteam(sn);
            m_subteams[subteam]->addNode(node);
        }
    }
}

//��������� ������ ����
void EnemyRoot::Update(GameObserver::subject_t subj, GameObserver::event_t type, GameObserver::info_t ptr)
{
    switch(type)
	{
    case EV_TURN:
        {
			if(static_cast<turn_info*>(ptr)->m_player == PT_ENEMY)
			{
				m_cur = m_subteams.begin();
				m_complexity = 0.0f;
			}
        }
        break;

    case EV_KILL:
        {
			kill_info* info = static_cast<kill_info*>(ptr);
			deleteChildren(info->m_victim->GetEID());
        }
        break;
    }
}

//������� ���� AI �� ������ ����������
AIEnemyNode* EnemyRoot::CreateNode(BaseEntity* entity, const std::string type)
{
	if(type == "fixed")
	{
		// ��� ���� ������������� ����
		return new FixedEnemyNode(entity->GetEID());
	}
	if(type == "patrol")
	{
		// ��� ���� ����������� ����
		return new PatrolEnemyNode(entity->GetEID());
	}
	if(type == "assault")
	{
		// ��� ���������
		return new AssaultEnemyNode(entity->GetEID());
	}
	if(type == "fixedtech")
	{
		// ��� ��������� ������� ������������� ����
		return new FixedTechEnemyNode(entity->GetEID());
	}
	if(type == "patroltech")
	{
		// ��� ��������� ������� ����������� ����
		return new PatrolTechEnemyNode(entity->GetEID());
	}
	if(type == "assaulttech")
	{
		// ��� ��������� ������� ���������� ����
		return new AssaultTechEnemyNode(entity->GetEID());
	}
	return 0;
}

void EnemyRoot::MakeSaveLoad(SavSlot& st)
{
    //��������/����������� ��������� AI
	if(st.IsSaving())
	{
		// �����������
		st << m_subteams.size();
		node_map_t::iterator it = m_subteams.begin();
		node_map_t::iterator end = m_subteams.end();
		while(it != end)
		{
			st << (*it).first;
			DynUtils::SaveObj(st, (*it).second);
			(*it).second->MakeSaveLoad(st);
			++it;
		}
		return;
	}
	// ��������
	SubteamNode* node;
	int num;
	st >> num;
	for(int i = 0; i < num; i++)
	{
		std::string str;
		st >> str;
		DynUtils::LoadObj(st, node);
		node->MakeSaveLoad(st);
		m_subteams[str] = node;
	}
}

// ������ ������ ��������� ����
float EnemyRoot::getComplexity() const
{
	float complexity = 0.0f;
	// �������� ����� ����� ���������� ���������
	node_map_t::const_iterator i = m_subteams.begin();
	node_map_t::const_iterator end = m_subteams.end();
	while(i != end)
	{
		complexity += (*i).second->getComplexity();
		++i;
	}
	return complexity;
}

// ������ ����������� �� �������� ����� � �������� ID � ��������� ��
// ������� ���� ����� �� ������ ������
void EnemyRoot::deleteChildren(eid_t ID)
{
	if(!ID) return;
	node_map_t::iterator i = m_subteams.begin();
	node_map_t::iterator end = m_subteams.end();
	while(i != end)
	{
		SubteamNode* subteam = (*i).second;
		if(subteam->getEntityEID() == ID)
		{
			// ����!
			if(subteam->die())
			{
				if(m_cur == i) m_cur++;
				node_map_t::iterator j = i;
				i++;
				m_subteams.erase(j);
				delete subteam;
				continue;
			}
		}
		i++;
	}
	// �������� ����� ������� ���� �����
	i = m_subteams.begin();
	while(i != end)
	{
		SubteamNode* subteam = (*i).second;
		subteam->deleteChildren(ID);
		i++;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// ������������ ���� ��� ����� - ��� �������-������ ����� �����
//
//////////////////////////////////////////////////////////////////////////////
namespace
{
	//
	// �������� �� ����� �������� ���� AI
	//
	void getAINodeDesc(const std::string& label, std::string& type, std::string& subteam)
	{
		TxtFilePtr txt(ai_xls_name);

		txt.ReadColumns(ai_columns, ai_columns + ACT_MAX_COLUMNS);
		std::string str;

		// ������ ������ � ����� ������
		for(int i = 0; i < txt->SizeY(); ++i)
		{
			txt->GetCell(i, ai_columns[ACT_LABEL].m_index, str);
			if(str == label)
			{
				// �����
				txt->GetCell(i, ai_columns[ACT_TYPE].m_index, type);
				txt->GetCell(i, ai_columns[ACT_SUBTEAM].m_index, subteam);
				return;
			}
		}

		// �� �����
		str = "getAINodeDesc: �� ������� ����� ";
		str += label;
		throw CASUS(str);
	}
	// ������� � �������
	void Print(const std::string& str)
	{
		Console::AddString(str.c_str());
	}
};

