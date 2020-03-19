/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: модели поведения человека в панике
                                                                                
                                                                                
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
// поведение в шоке
//
////////////////////////////////////////////////////////////////////////////////
class ShockBehaviour : public PanicBehaviour
{
public:

    //инициализация модели поведения
    virtual void Init(HumanEntity* human);

    // проиграть панику для человека
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:  
    //           true   -  необходим повторный вызов 
    //           false  -  модель поведения проиграна полностью
    //
    // ВНИМАНИЕ: 
    //           по окончанию паники кол-во movepoints д.б. == 0
    //
    virtual bool Panic(HumanEntity* human);
	ShockBehaviour();

};

ShockBehaviour::ShockBehaviour()
{
}

//инициализация модели поведения
void ShockBehaviour::Init(HumanEntity* human)
{
}

// проиграть панику для человека
bool ShockBehaviour::Panic(HumanEntity* human)
{
	// обнулим человеку мувпоинты и закончим на этом
	if(human)
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
//
// поведение в обычной панике
//
////////////////////////////////////////////////////////////////////////////////
class UsualPanicBehaviour : public PanicBehaviour
{
public:

    //инициализация модели поведения
    virtual void Init(HumanEntity* human);

    // проиграть панику для человека
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:  
    //           true   -  необходим повторный вызов 
    //           false  -  модель поведения проиграна полностью
    //
    // ВНИМАНИЕ: 
    //           по окончанию паники кол-во movepoints д.б. == 0
    //
    virtual bool Panic(HumanEntity* human);

	UsualPanicBehaviour();
	~UsualPanicBehaviour();
private:
	// текущая активность
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
//инициализация модели поведения
void UsualPanicBehaviour::Init(HumanEntity* human)
{
	delete m_activity;
	m_activity = 0;
}

// проиграть панику для человека
bool UsualPanicBehaviour::Panic(HumanEntity* human)
{
	// если человек нулевой (может его убили) - на выход
	if(!human) return false;

    //проиграть действие
    if(m_activity)
	{
        if(!m_activity->Run(AC_TICK))
		{
            delete m_activity;
            m_activity = 0;
        }
        return true;
    }

	// пусть человек побегает
	AIAPI::getInst()->setRun(human->GetEID());

    // 1. если у человека нет шагов - на выход
	if(AIAPI::getInst()->getSteps(human->GetEID()) < 1)
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
		return false;
	}

	// посмотрим можно ли куда-нибудь побежать
	
	// рассчитаем поле проходимости
	PathUtils::GetInst()->CalcPassField(human,
		0, 0, 0);
	// получим вектор хексов до которых можно будет добежать с моими шагами
	pnt_vec_t field;
	AIAPI::getInst()->getBaseField(human->GetGraph()->GetPos2(),
		AIAPI::getInst()->getSteps(human->GetEID()),
		&field);
	// получим вектор доступных хексов поля
	pnt_vec_t vec;
	AIAPI::getInst()->getPanicReachableField(field, &vec);
	// получим случайный хекс
	ipnt2_t target = AIAPI::getInst()->getRandomPoint(vec);
	// рассчитаем путь
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(human,
		target, path);

	// 2. если бежать нельзя - на выход
	if(path.empty())
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
		return false;
	}

	// 3. можно бежать - продолжение следует
	m_activity = ActivityFactory::GetInst()->CreateMove(human,
		path,
		ActivityFactory::CT_ENEMY_MOVEIGNORE);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//
// поведение в берсерке
//
////////////////////////////////////////////////////////////////////////////////
class BerserkBehaviour : public PanicBehaviour
{
public:

    //инициализация модели поведения
    virtual void Init(HumanEntity* human);

    // проиграть панику для человека
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:  
    //           true   -  необходим повторный вызов 
    //           false  -  модель поведения проиграна полностью
    //
    // ВНИМАНИЕ: 
    //           по окончанию паники кол-во movepoints д.б. == 0
    //
    virtual bool Panic(HumanEntity* human);

	BerserkBehaviour();
	~BerserkBehaviour();
private:
	// текущая активность
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

//инициализация модели поведения
void BerserkBehaviour::Init(HumanEntity* human)
{
	delete m_activity;
	m_activity = 0;
}

// проиграть панику для человека
bool BerserkBehaviour::Panic(HumanEntity* human)
{
	// если человек нулевой (может его убили) - на выход
	if(!human) return false;

    //проиграть действие
    if(m_activity)
	{
        if(!m_activity->Run(AC_TICK))
		{
            delete m_activity;
            m_activity = 0;
        }
        return true;
    }

	// все стреляют в автоматическом режиме оружия (autoshot)
	AIAPI::getInst()->setShootType(human->GetEID(), 0.0f, 0.0f);

    if(!AIAPI::getInst()->takeBestWeapon(human->GetEID(), AssaultWeaponComparator()))
    {
        if(!AIAPI::getInst()->takeGrenade(human->GetEID()))
        {
            human->GetEntityContext()->GetTraits()->SetMovepnts(0);
            return false;
        }
    }
    // 1. если у человека нет мувпоинтов на выстрел - на выход
	if(AIAPI::getInst()->getMovePoints(human->GetEID()) <
		AIAPI::getInst()->getShootMovePoints(human->GetEID(), SHT_AUTOSHOT))
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
		return false;
	}

	// 2. выстрелим если сможем
	if(AIAPI::getInst()->shootRandom(human->GetEID(), &m_activity))
	{
		// получилось выстрелить - продолжение следует
		return true;
	}
	// выстрелить не получилось - на выход
	human->GetEntityContext()->GetTraits()->SetMovepnts(0);
	return false;
}

//////////////////////////////////////////////////////////////////////////////////
//
// фабрика моделей поведения в панике
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

//создать шоковое поведение
PanicBehaviour* PanicBehaviourFactory::CreateShockBehaviour()
{
	return new ShockBehaviour;
}

//создать обычную панику
PanicBehaviour* PanicBehaviourFactory::CreateUsualBehaviour()
{
	return new UsualPanicBehaviour;
}

//создать панику берсерка
PanicBehaviour* PanicBehaviourFactory::CreateBerserkBehaviour()
{
	return new BerserkBehaviour;
}
