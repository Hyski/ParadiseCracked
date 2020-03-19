/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ ��������� �������� � ������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#pragma warning(disable:4786)

#include "logicdefs.h"

#include "PanicBehaviour.h"
#include "entity.h"
#include "Activity.h"
#include "AIAPI.h"
#include "PathUtils.h"
#include "Graphic.h"

PanicBehaviourFactory::Container PanicBehaviourFactory::m_container;

////////////////////////////////////////////////////////////////////////////////
//
// ��������� � ����
//
////////////////////////////////////////////////////////////////////////////////
class ShockBehaviour : public PanicBehaviour
{
public:

    //������������� ������ ���������
    virtual void Init(HumanEntity* human);

    // ��������� ������ ��� ��������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:  
    //           true   -  ��������� ��������� ����� 
    //           false  -  ������ ��������� ��������� ���������
    //
    // ��������: 
    //           �� ��������� ������ ���-�� movepoints �.�. == 0
    //
    virtual bool Panic(HumanEntity* human);
	ShockBehaviour();

};

ShockBehaviour::ShockBehaviour()
{
}

//������������� ������ ���������
void ShockBehaviour::Init(HumanEntity* human)
{
}

// ��������� ������ ��� ��������
bool ShockBehaviour::Panic(HumanEntity* human)
{
	// ������� �������� ��������� � �������� �� ����
	if(human)
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
//
// ��������� � ������� ������
//
////////////////////////////////////////////////////////////////////////////////
class UsualPanicBehaviour : public PanicBehaviour
{
public:

    //������������� ������ ���������
    virtual void Init(HumanEntity* human);

    // ��������� ������ ��� ��������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:  
    //           true   -  ��������� ��������� ����� 
    //           false  -  ������ ��������� ��������� ���������
    //
    // ��������: 
    //           �� ��������� ������ ���-�� movepoints �.�. == 0
    //
    virtual bool Panic(HumanEntity* human);

	UsualPanicBehaviour();
	~UsualPanicBehaviour();
private:
	// ������� ����������
    Activity* m_activity;
};

UsualPanicBehaviour::UsualPanicBehaviour()
{
	m_activity = 0;
}

UsualPanicBehaviour::~UsualPanicBehaviour()
{
	delete m_activity;
}
//������������� ������ ���������
void UsualPanicBehaviour::Init(HumanEntity* human)
{
	delete m_activity;
	m_activity = 0;
}

// ��������� ������ ��� ��������
bool UsualPanicBehaviour::Panic(HumanEntity* human)
{
	// ���� ������� ������� (����� ��� �����) - �� �����
	if(!human) return false;

    //��������� ��������
    if(m_activity)
	{
        if(!m_activity->Run(AC_TICK))
		{
            delete m_activity;
            m_activity = 0;
        }
        return true;
    }

	// ����� ������� ��������
	AIAPI::getInst()->setRun(human->GetEID());

    // 1. ���� � �������� ��� ����� - �� �����
	if(AIAPI::getInst()->getSteps(human->GetEID()) < 1)
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
		return false;
	}

	// ��������� ����� �� ����-������ ��������
	
	// ���������� ���� ������������
	PathUtils::GetInst()->CalcPassField(human,
		0, 0, 0);
	// ������� ������ ������ �� ������� ����� ����� �������� � ����� ������
	pnt_vec_t field;
	AIAPI::getInst()->getBaseField(human->GetGraph()->GetPos2(),
		AIAPI::getInst()->getSteps(human->GetEID()),
		&field);
	// ������� ������ ��������� ������ ����
	pnt_vec_t vec;
	AIAPI::getInst()->getPanicReachableField(field, &vec);
	// ������� ��������� ����
	ipnt2_t target = AIAPI::getInst()->getRandomPoint(vec);
	// ���������� ����
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(human,
		target, path);

	// 2. ���� ������ ������ - �� �����
	if(path.empty())
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
		return false;
	}

	// 3. ����� ������ - ����������� �������
	m_activity = ActivityFactory::GetInst()->CreateMove(human,
		path,
		ActivityFactory::CT_ENEMY_MOVEIGNORE);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//
// ��������� � ��������
//
////////////////////////////////////////////////////////////////////////////////
class BerserkBehaviour : public PanicBehaviour
{
public:

    //������������� ������ ���������
    virtual void Init(HumanEntity* human);

    // ��������� ������ ��� ��������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:  
    //           true   -  ��������� ��������� ����� 
    //           false  -  ������ ��������� ��������� ���������
    //
    // ��������: 
    //           �� ��������� ������ ���-�� movepoints �.�. == 0
    //
    virtual bool Panic(HumanEntity* human);

	BerserkBehaviour();
	~BerserkBehaviour();
private:
	// ������� ����������
    Activity* m_activity;
};

BerserkBehaviour::BerserkBehaviour()
{
	m_activity = 0;
}

BerserkBehaviour::~BerserkBehaviour()
{
	delete m_activity;
}

//������������� ������ ���������
void BerserkBehaviour::Init(HumanEntity* human)
{
	delete m_activity;
	m_activity = 0;
}

// ��������� ������ ��� ��������
bool BerserkBehaviour::Panic(HumanEntity* human)
{
	// ���� ������� ������� (����� ��� �����) - �� �����
	if(!human) return false;

    //��������� ��������
    if(m_activity)
	{
        if(!m_activity->Run(AC_TICK))
		{
            delete m_activity;
            m_activity = 0;
        }
        return true;
    }

	// ��� �������� � �������������� ������ ������ (autoshot)
	AIAPI::getInst()->setShootType(human->GetEID(), 0.0f, 0.0f);

    if(!AIAPI::getInst()->takeBestWeapon(human->GetEID(), AssaultWeaponComparator()))
    {
        if(!AIAPI::getInst()->takeGrenade(human->GetEID()))
        {
            human->GetEntityContext()->GetTraits()->SetMovepnts(0);
            return false;
        }
    }
    // 1. ���� � �������� ��� ���������� �� ������� - �� �����
	if(AIAPI::getInst()->getMovePoints(human->GetEID()) <
		AIAPI::getInst()->getShootMovePoints(human->GetEID(), SHT_AUTOSHOT))
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
		return false;
	}

	// 2. ��������� ���� ������
	if(AIAPI::getInst()->shootRandom(human->GetEID(), &m_activity))
	{
		// ���������� ���������� - ����������� �������
		return true;
	}
	// ���������� �� ���������� - �� �����
	human->GetEntityContext()->GetTraits()->SetMovepnts(0);
	return false;
}

//////////////////////////////////////////////////////////////////////////////////
//
// ������� ������� ��������� � ������
//
//////////////////////////////////////////////////////////////////////////////////

//singleton
PanicBehaviourFactory* PanicBehaviourFactory::GetInst()
{
	if(!m_container.m_instance)
	{
		m_container.m_instance = new PanicBehaviourFactory;
	}
	return m_container.m_instance;
}

//������� ������� ���������
PanicBehaviour* PanicBehaviourFactory::CreateShockBehaviour()
{
	return new ShockBehaviour;
}

//������� ������� ������
PanicBehaviour* PanicBehaviourFactory::CreateUsualBehaviour()
{
	return new UsualPanicBehaviour;
}

//������� ������ ��������
PanicBehaviour* PanicBehaviourFactory::CreateBerserkBehaviour()
{
	return new BerserkBehaviour;
}
