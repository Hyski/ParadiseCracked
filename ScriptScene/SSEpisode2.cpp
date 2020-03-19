/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   03.09.2001

************************************************************************/
#include "scriptpch.h"
#include "ScriptSceneAPI.h"
#include "SSEpisode2.h"

using namespace Episode2;

//**********************************************************************************
//	Scene1
Scene1::Scene1() : ScriptScene(),m_mtAction(MT_TO_PRIMARY_POINT)
{
}

Scene1::~Scene1()
{
}
//	начало скриптовой сцены
bool Scene1::OnStart(void)
{
	if(ScriptSceneAPI::Vehicle::IsExist("mini_truck"))
	{
		//	создаем действующие лица
		AddObject(m_MiniTruck = new ScriptSceneAPI::Vehicle("mini_truck"));
		AddObject(m_Camera = new ScriptSceneAPI::Camera());
		AddObject(m_Gate1 = new ScriptSceneAPI::Object("door_entr_01"));
		AddObject(m_Gate2 = new ScriptSceneAPI::Object("door_entrance02"));
		AddObject(m_Timer = new ScriptSceneAPI::Timer("timer"));
		
		//	проверяем в какой половине находится MiniTrack
		ipnt2_t pt = m_MiniTruck->GetCoords2();
		if(pt.x < 32) m_MiniTruck->SetCoords(ipnt2_t(13,67),1);
		else m_MiniTruck->SetCoords(ipnt2_t(50,67),1);
		//	направляем MiniTruck в точку
		m_MiniTruck->MoveTo(ipnt2_t(32,56));
		m_MiniTruck->Activated(true);

		m_Camera->MoveBySpline("Camera01");
		m_Camera->Activated(true);

		return true;
	}

	return m_Erase = false;
}
//	окончание скриптовой сцены
bool Scene1::OnFinish(void)
{
	if(IsValid())
	{
		if(m_mtAction == MT_ON_SKIP)
		{
			m_MiniTruck->SetCoords(ipnt2_t(32,50),0);
			m_Gate1->ToBlast(1000);
			m_Gate2->ToBlast(1000);
			m_Camera->FocusOn(m_MiniTruck->GetCoords3(),0);
		}
	}

	return true;
}
//	прерывание скриптовой сцены
bool Scene1::OnSkip(void)
{
	if(IsValid())
	{
		m_mtAction = MT_ON_SKIP;
		m_MiniTruck->Stop();
		if(!m_MiniTruck->OnThink())
		{
			m_MiniTruck->SetCoords(ipnt2_t(32,50),0);
			m_Gate1->ToBlast(1000);
			m_Gate2->ToBlast(1000);
			return true;
		}
		return false;
	}

	return true;
}
//	передача управления скриптовой сцене
bool Scene1::OnTick(void)
{
	return true;
}
//	окончание действия объекта скриптовой сцены
bool Scene1::OnObjectNoActive(Object* object)
{
	if(!strcmp(object->Type(),"camera"))
	{
		m_Camera->Activated(false);
		return m_MiniTruck->IsActivated()||m_Timer->IsActivated();
	}
	if(!strcmp(object->Type(),"timer"))
	{
		m_Timer->Activated(false);

		return true;
	}
	if(!strcmp(object->Type(),"vehicle"))
	{
		switch(m_mtAction)
		{
		case MT_TO_PRIMARY_POINT:
			m_Gate1->ToBlast(1000);
			m_MiniTruck->MoveTo(ipnt2_t(32,50));
			m_Timer->SetTime(m_Gate2->SwitchState()-Timer::GetSeconds());
			m_Timer->Start();
			m_Timer->Activated(true);
			m_mtAction = MT_TO_SECONDARY_POINT;
			return true;
		case MT_TO_SECONDARY_POINT:
			m_MiniTruck->Activated(false);
			return m_Camera->IsActivated();
		case MT_ON_SKIP:
			return false;
		}
	}

	return true;
}

//**********************************************************************************
//	Scene2
Scene2::Scene2() : ScriptScene(),m_Phase(P_ONE),m_Skipped(false)
{
}

Scene2::~Scene2()
{
}
//	начало скриптовой сцены
bool Scene2::OnStart(void)
{
	//	создаем действующие лица
	AddObject(m_Camera = new ScriptSceneAPI::Camera());
	AddObject(m_Timer = new ScriptSceneAPI::Timer("timer"));
	AddObject(m_Hacker = new ScriptSceneAPI::Human("player"));
	AddObject(m_Tom = new ScriptSceneAPI::Trader("tom"));

	SpawnSoldier();

	m_Camera->FocusOn(m_Soldiers[0]->GetCoords3(),1);
	m_Camera->Activated(true);
	//	убираем Тома
	m_Tom->Destroy();

	return true;
}
//	окончание скриптовой сцены
bool Scene2::OnFinish(void)
{
	if(IsValid())
	{
		//	это необходимо для того чтобы Хакер обновил информацию о врагах
		//	другого способа не было времени и возможности придумать
		for(int i=0;i<MAX_SOLDIER;i++)
			m_Soldiers[i]->SetCoords(m_Soldiers[i]->GetCoords2(),1);
	}

	return true;
}
//	прерывание скриптовой сцены
bool Scene2::OnSkip(void)
{
	if(IsValid())
	{
		int i;

		m_Skipped = true;
		//	во первых всех останавливаем
		for(i=0;i<MAX_SOLDIER;i++) m_Soldiers[i]->Stop();
		//	проверяем на активность
		if(CheckAllSoldiersNoThink())
		{
			if(m_Phase == P_ONE)
			{
				//	направляем всю братию к Хакеру
				const ipnt2_t points[MAX_SOLDIER] = 
				{
					ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(0,1)),
						ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(2,0)),
						ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(1,2)),
						ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(-1,-1)),
						ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(-2,-1)),
						ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(-1,1))
				};
				
				for(i=3;i<MAX_SOLDIER;i++)
				{
					if(!m_Soldiers[i]->IsBusy(points[i]))
						m_Soldiers[i]->SetCoords(points[i],1);
				}

				m_Camera->FocusOn(m_Hacker->GetCoords3(),1);
			}
		}
		else return false;
	}
	else
	{
		AddObject(m_Camera = new ScriptSceneAPI::Camera());
		//	создаем Хакера
		AddObject(m_Hacker = new ScriptSceneAPI::Human("player"));
		//	создаем и убираем Тома
		AddObject(m_Tom = new ScriptSceneAPI::Trader("tom"));
		m_Tom->Destroy();
		//	направляем всю братию к Хакеру
		const ipnt2_t points[MAX_SOLDIER] = 
		{
			ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(0,1)),
				ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(2,0)),
				ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(1,2)),
				ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(-1,-1)),
				ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(-2,-1)),
				ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(-1,1))
		};
		//	создаем и расставляем солдат
		SpawnSoldier();

		for(int i=3;i<MAX_SOLDIER;i++)
		{
			if(!m_Soldiers[i]->IsBusy(points[i]))
				m_Soldiers[i]->SetCoords(points[i],1);
		}

		m_Camera->FocusOn(m_Hacker->GetCoords3(),1);
	}

	return true;
}
//	передача управления скриптовой сцене
bool Scene2::OnTick(void)
{
	if(!m_Camera->IsActivated() && (m_Phase == P_ONE))
	{
		for(int i=0;i<MAX_SOLDIER;i++)
		{
			if(m_Soldiers[i]->IsActivated())
			{
				m_Camera->FocusOn(m_Soldiers[i]->GetCoords3(),0);
				break;
			}
		}
	}

	return true;
}
//	окончание действия объекта скриптовой сцены
bool Scene2::OnObjectNoActive(Object* object)
{
	if(m_Skipped)
	{
		return !OnSkip();
	}
	else
	{
		if(!strcmp(object->Type(),"camera"))
		{
			m_Camera->Activated(false);
			//	направляем всю братию к Хакеру
			const ipnt2_t points[MAX_SOLDIER] = 
			{
				ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(0,1)),
					ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(2,0)),
					ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(1,2)),
					ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(-1,-1)),
					ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(2,-1)),
					ipnt2_t(m_Hacker->GetCoords2()+ipnt2_t(-1,1))
			};
			for(int i=0;i<MAX_SOLDIER;i++)
			{
				if(!m_Soldiers[i]->IsBusy(points[i]))
				{
					m_Soldiers[i]->WalkTo(points[i]);
					m_Soldiers[i]->Activated(true);
				}
			}

			GetApi()->SetRoofMode(false);

			if(IsAllSoldiersInactive()) return false;
			
			return true;
		}
		if(!strcmp(object->Type(),"human"))
		{
			object->Activated(false);
			
			switch(m_Phase)
			{
			case P_ONE:
				if(IsAllSoldiersInactive())
				{
					m_Phase = P_SECOND;
					m_Camera->FocusOn(m_Hacker->GetCoords3(),1);
					for(int i=0;i<MAX_SOLDIER;i++)
					{
						m_Soldiers[i]->RotateToPoint(m_Hacker->GetCoords3());
						m_Soldiers[i]->Activated(true);
					}
				}
				break;
			case P_SECOND:
				if(IsAllSoldiersInactive()) return false;
			}
		}
		if(!strcmp(object->Type(),"timer"))
		{
			m_Timer->Activated(false);
		}
	}

	return true;
}

bool Scene2::IsAllSoldiersInactive(void)
{
	for(int i=0;i<MAX_SOLDIER;i++)
	{
		if(m_Soldiers[i]->IsActivated()) return false;
	}

	return true;
}

bool Scene2::CheckAllSoldiersNoThink(void)
{
	for(int i=0;i<MAX_SOLDIER;i++)
	{
		if(m_Soldiers[i]->OnThink())
			return false;
	}

	return true;
}

void Scene2::SpawnSoldier(void)
{
	//	создание и расстановка солдатов
	const int max_length = 20;
	int counter_y = 0;
	for(int i=0;(i<MAX_SOLDIER) && (counter_y != max_length);i++,counter_y++)
	{
		AddObject(m_Soldiers[i] = new ScriptSceneAPI::Human("soldier",ScriptSceneAPI::Human::CM_NEW));

		while(counter_y != max_length)
		{
			ipnt2_t xy(4,21+counter_y);
			if(!m_Soldiers[i]->IsBusy(xy))
			{
				m_Soldiers[i]->SetCoords(xy,1);
				SetTraits(m_Soldiers[i]);
				break;
			}
			counter_y++;
		}
	}
}

void Scene2::SetTraits(ScriptSceneAPI::Human* soldier)
{
	soldier->Show(true);
	soldier->GiveWeapon("penta2","standart_police_shell",28,ScriptSceneAPI::Human::PK_HANDS);
	soldier->GiveArmor("nohomoto",ScriptSceneAPI::Human::PK_BODY);
	soldier->GiveGrenade("fiery_grenade",ScriptSceneAPI::Human::PK_RKNEE);
	soldier->GiveAmmo("advanced_police_shell",14,ScriptSceneAPI::Human::PK_BACKPACK);
	soldier->GiveAmmo("advanced_police_shell",14,ScriptSceneAPI::Human::PK_RKNEE);
	soldier->GiveAmmo("shock_police_shell",14,ScriptSceneAPI::Human::PK_RKNEE);
	soldier->GiveAmmo("light_police_shell",14,ScriptSceneAPI::Human::PK_RKNEE);
	soldier->GiveImplant("strength_amplifier",ScriptSceneAPI::Human::PK_IMPLANTS);
	soldier->GiveImplant("strength_amplifier",ScriptSceneAPI::Human::PK_IMPLANTS);
	soldier->GiveImplant("strength_amplifier",ScriptSceneAPI::Human::PK_IMPLANTS);
	soldier->GiveImplant("accuracy_implant",ScriptSceneAPI::Human::PK_IMPLANTS);
		
	soldier->SetTeam(ScriptSceneAPI::Human::TT_ENEMY);
	soldier->SetBehaviourModel("s_a");
}

//**********************************************************************************
//	Scene3
Scene3::Scene3() : ScriptScene()
{
}

Scene3::~Scene3()
{
}
//	начало скриптовой сцены
bool Scene3::OnStart(void)
{
	GetApi()->SetRoofMode(false);

	//	создаем действующие лица
	AddObject(m_Camera = new ScriptSceneAPI::Camera());
	AddObject(m_Door = new ScriptSceneAPI::Object("door_01"));
	m_Door->SwitchState();
	m_Camera->FocusOn(m_Door->GetCoords3(),1);
	m_Camera->Activated(true);

	return true;
}
//	окончание скриптовой сцены
bool Scene3::OnFinish(void)
{
	return true;
}
//	прерывание скриптовой сцены
bool Scene3::OnSkip(void)
{
	return true;
}
//	передача управления скриптовой сцене
bool Scene3::OnTick(void)
{
	return true;
}
//	окончание действия объекта скриптовой сцены
bool Scene3::OnObjectNoActive(Object* object)
{
	if(!strcmp(object->Type(),"camera"))
	{
		object->Activated(false);
		return false;
	}

	return true;
}
/////////////////////////////////////////////////////////////////////////
//	мультик по окончании второго эпизода
ScriptScene* Episode2::FinalEpisodeVideo1(void)
{
	return new VideoScriptScene("e3_1.bik");
}

ScriptScene* Episode2::FinalEpisodeVideo2(void)
{
	return new VideoScriptScene("e3_1.bik");
}
/////////////////////////////////////////////////////////////////////////
