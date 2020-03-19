/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   06.07.2001

************************************************************************/
#include "scriptpch.h"
#include "ScriptSceneAPI.h"
#include "SSEpisode1.h"

//---------- Лог файл ------------
//#define _DEBUG_SS 
#ifdef _DEBUG_SS
#include "../Common/Log/Log.h"
CLog ss_log;
#define ss	ss_log["ss.log"]
#else
#define ss_log	/##/
#endif
//--------------------------------

using namespace Episode1;

//**********************************************************************************
//	Scene1
Scene1::Scene1() : ScriptScene()
{
}

Scene1::~Scene1()
{
}
//	начало скриптовой сцены
bool Scene1::OnStart(void)
{
	//	создаем действующие лица
	AddObject(m_Camera = new ScriptSceneAPI::Camera());
	AddObject(m_Hacker = new ScriptSceneAPI::Human("player"));
	m_Hacker->RunTo(ipnt2_t(8,15));
	m_Hacker->Activated(true);
	m_Camera->FocusOn(m_Hacker->GetCoords3(),0);
	GetApi()->SetRoofMode(false);

	return true;
}
//	окончание скриптовой сцены
bool Scene1::OnFinish(void)
{
	if(IsValid())
	{
		m_Hacker->SetCoords(ipnt2_t(8,15),0);
		GetApi()->ExitFromLevel(m_Hacker->GetCoords2());
	}

	return true;
}
//	прерывание скриптовой сцены
bool Scene1::OnSkip(void)
{
	if(IsValid())
	{
		m_Hacker->Stop();
		if(!m_Hacker->OnThink())
		{
			m_Hacker->SetCoords(ipnt2_t(8,15),0);
			GetApi()->ExitFromLevel(m_Hacker->GetCoords2());
		}
		else return false;
	}

	return true;
}
//	передача управления скриптовой сцене
bool Scene1::OnTick(void)
{
	m_Camera->FocusOn(m_Hacker->GetCoords3(),0);
	return true;
}
//	окончание действия объекта скриптовой сцены
bool Scene1::OnObjectNoActive(Object* object)
{
	if(!strcmp(object->Name(),"player0"))
	{
		return false;
	}

	return true;
}

//**********************************************************************************
//	Scene2
Scene2::Scene2() : ScriptScene(),m_FirstCameraAction(true)
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
	AddObject(m_SamLee = new ScriptSceneAPI::Trader("samlee"));
	//	задаем начальные действия
	m_Camera->FocusOn(m_Hacker->GetCoords3(),0);
	m_Timer->SetTime(2);
	m_Timer->Start();
	m_Timer->Activated(true);

	return true;
}
//	окончание скриптовой сцены
bool Scene2::OnFinish(void)
{
	return true;
}
//	прерывание скриптовой сцены
bool Scene2::OnSkip(void)
{
	if(IsValid()) m_Camera->FocusOn(m_Hacker->GetCoords3(),0);
	return true;
}
//	окончание действия объекта скриптовой сцены
bool Scene2::OnObjectNoActive(Object* object)
{
	if(!strcmp(object->Type(),"timer"))
	{
		m_Camera->MoveBySpline("Camera01");
		m_Camera->Activated(true);
		m_Timer->Activated(false);

		return true;
	}
	if(!strcmp(object->Type(),"camera"))
	{
		if(m_FirstCameraAction)
		{
			m_Camera->FocusOn(m_Hacker->GetCoords3(),3.f);
			m_FirstCameraAction = false;
		}
		else
		{
			return false;
		}
	}

	return true;
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
	//	создаем действующие лица
	AddObject(m_Camera = new ScriptSceneAPI::Camera());
	AddObject(m_Jumper = new ScriptSceneAPI::Animation("jumper"));
	//	задаем начальные действия
	m_Jumper->Start();
	m_Jumper->Activated(true);
	m_Camera->MoveBySpline("Camera02");
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
//	окончание действия объекта скриптовой сцены
bool Scene3::OnObjectNoActive(Object* object)
{
	return false;
}

//**********************************************************************************
//	Scene4
Scene4::Scene4() : ScriptScene()
{
	// объект для работы с Loris
	ScriptSceneAPI::Human* loris = new ScriptSceneAPI::Human("loris");
	m_LorisDeath = loris->GetCoords3();
	delete loris;
}

Scene4::~Scene4()
{
}
//	начало скриптовой сцены
bool Scene4::OnStart(void)
{
	//	создаем действующие лица
	AddObject(m_Camera = new ScriptSceneAPI::Camera());
	AddObject(m_Timer = new ScriptSceneAPI::Timer("timer"));
	AddObject(m_Hacker = new ScriptSceneAPI::Human("player"));
	//	задаем начальные действия
	m_Camera->FocusOn(m_LorisDeath,2.0f);
	m_Camera->Activated(true);
	GetApi()->SetRoofMode(false);

	return true;
}
//	окончание действия объекта скриптовой сцены
bool Scene4::OnObjectNoActive(Object* object)
{
	static bool first = true;

	if(!strcmp(object->Type(),"camera"))
	{
		if(first)
		{
			m_Camera->Activated(false);
			m_Timer->SetTime(1);
			m_Timer->Start();
			m_Timer->Activated(true);
			first = false;

			return true;
		}
		else
		{
			return false;
		}
	}
	if(!strcmp(object->Type(),"timer"))
	{
		m_Timer->Activated(false);
		m_Camera->FocusOn(m_Hacker->GetCoords3(),2.0f);
		m_Camera->Activated(true);
	}

	return true;
}


//**********************************************************************************
//	Scene5
Scene5::Scene5() : ScriptScene()
{
}

Scene5::~Scene5()
{
}
//	начало скриптовой сцены
bool Scene5::OnStart(void)
{
	//	создаем действующие лица
	AddObject(m_Camera = new ScriptSceneAPI::Camera());
	AddObject(m_Stormtroopers[0] = new ScriptSceneAPI::Human("soldier"));
	AddObject(m_Stormtroopers[1] = new ScriptSceneAPI::Human("soldier"));
	AddObject(m_Stormtroopers[2] = new ScriptSceneAPI::Human("soldier"));
	//	задаем начальные действия

	return true;
}
//	окончание скриптовой сцены
bool Scene5::OnFinish(void)
{
	return true;
}
//	прерывание скриптовой сцены
bool Scene5::OnSkip(void)
{
	return true;
}
//	окончание действия объекта скриптовой сцены
bool Scene5::OnObjectNoActive(Object* object)
{
	return false;
}


//**********************************************************************************
//	Scene6
Scene6::Scene6() : ScriptScene(),m_State(S_FIRST_DOOR)
{
}

Scene6::~Scene6()
{
}
//	начало скриптовой сцены
bool Scene6::OnStart(void)
{
	if(GetApi()->IsHumanExist("chill"))
	{
		GetApi()->SetRoofMode(false);
		//	создаем действующие лица
		AddObject(m_Camera = new ScriptSceneAPI::Camera());
		AddObject(m_Door1 = new ScriptSceneAPI::Object("mhdoor_2l"));
		AddObject(m_Door2 = new ScriptSceneAPI::Object("mhdoor_5r"));
		AddObject(m_Door3 = new ScriptSceneAPI::Object("mhdoor_4l"));
		AddObject(m_Troy = new ScriptSceneAPI::Human("troy"));
		AddObject(m_Chill = new ScriptSceneAPI::Human("chill"));
		AddObject(m_Timer = new ScriptSceneAPI::Timer("timer"));
		//	задаем начальные действия
		m_Camera->FocusOn(m_Chill->GetCoords3(),0);
		m_Timer->SetTime(m_Door1->SwitchState()-Timer::GetSeconds());
		m_Timer->Start();
		m_Timer->Activated(true);

		return true;
	}

	return m_Erase = false;
}
//	окончание скриптовой сцены
bool Scene6::OnFinish(void)
{
	if(IsValid() && (m_State == S_ON_SKIP))
	{
		m_Chill->Stop();
		//	Chill должен остановиться
		if(!m_Chill->OnThink())
		{
			ChillToTroy();
			return true;
		}
		else return false;
	}

	return true;
}
//	прерывание скриптовой сцены
bool Scene6::OnSkip(void)
{
	if(IsValid())
	{
		m_State = S_ON_SKIP;
		m_Chill->Stop();
		//	Chill должен остановиться
		if(!m_Chill->OnThink())
		{
			ChillToTroy();
			return true;
		}
		else return false;
	}

	return true;
}
//	передача управления скриптовой сцене
bool Scene6::OnTick(void)
{
	switch(m_State)
	{
	case S_FIRST_DOOR:
	case S_SECOND_DOOR:
	case S_THIRD_DOOR:
	case S_RUNNED_TO_TROY:
		m_Camera->FocusOn(m_Chill->GetCoords3(),0);
		break;
	case S_TROY:
		m_Camera->FocusOn(m_Troy->GetCoords3(),0);
		m_State = S_FINISH;
		break;
	case S_FINISH:
		m_Troy->Talk(ScriptSceneAPI::Instance()->GetSSPhrase("episode1.scene6.troy"));
		return false;
	case S_ON_SKIP:
		return false;

	}
	return true;
}
//	окончание действия объекта скриптовой сцены
bool Scene6::OnObjectNoActive(Object* object)
{
	if(m_State == S_ON_SKIP) return false;

	if(!strcmp(object->Type(),"timer"))
	{
		m_Timer->Activated(false);
		switch(m_State)
		{
		case S_FIRST_DOOR:
			if(m_Chill->IsAccess(m_Troy))
			{//	если можно добраться до Троя
				m_Chill->RunTo(m_Troy);
				m_Chill->Activated(true);
				m_State = S_RUNNED_TO_TROY;
			}
			else
			{
				if(m_Chill->IsAccess(m_Door3))
				{//	если можно добраться до третьей двери
					m_Chill->RunTo(m_Door3);
					m_Chill->Activated(true);
					m_State = S_THIRD_DOOR;
				}
				else
				{
					if(m_Chill->IsAccess(m_Door2))
					{//	если можно добраться до второй двери
						m_Chill->RunTo(m_Door2);
						m_Chill->Activated(true);
						m_State = S_SECOND_DOOR;
					}
					else
					{
						m_State = S_TROY;
					}
				}
			}
			break;
		case S_SECOND_DOOR:
			m_State = S_TROY;
			if(m_Chill->IsAccess(m_Troy))
			{//	если можно добраться до Троя
				m_Chill->RunTo(m_Troy);
				m_Chill->Activated(true);
				m_State = S_RUNNED_TO_TROY;
			}
			else
			{
				if(m_Chill->IsAccess(m_Door3))
				{//	если можно добраться до третьей двери
					m_Chill->RunTo(m_Door3);
					m_Chill->Activated(true);
					m_State = S_THIRD_DOOR;
				}
			}
			break;
		case S_THIRD_DOOR:
			m_State = S_TROY;
			if(m_Chill->IsAccess(m_Troy))
			{//	если можно добраться до Троя
				m_Chill->RunTo(m_Troy);
				m_Chill->Activated(true);
				m_State = S_RUNNED_TO_TROY;
			}
			break;
		}
	}
	if(!strcmp(object->Name(),"chill0"))
	{
		m_Chill->Activated(false);
		switch(m_State)
		{
		case S_SECOND_DOOR:		//	открываем вторую дверь
			if(!m_Door2->GetState())
			{
				m_Timer->SetTime(m_Door2->SwitchState()-Timer::GetSeconds());
				m_Timer->Start();
				m_Timer->Activated(true);
			}
			else
			{//	причина совершенно другая
				m_State = S_TROY;
			}
			break;
		case S_THIRD_DOOR:		//	открываем третью дверь
			if(!m_Door3->GetState())
			{
				m_Timer->SetTime(m_Door3->SwitchState()-Timer::GetSeconds());
				m_Timer->Start();
				m_Timer->Activated(true);
			}
			else
			{//	причина совершенно другая
				m_State = S_TROY;
			}
			break;
		case S_RUNNED_TO_TROY:
			m_Chill->RotateToPoint(m_Troy->GetCoords3());
			m_Chill->Activated(true);
			m_State = S_ROTATE_TO_TROY;
			break;
		case S_ROTATE_TO_TROY:
			m_State = S_TROY;
			break;
		}
	}

	return true;
}

void Scene6::ChillToTroy(void)
{
	const ipnt2_t points[4] = 
	{
		ipnt2_t(m_Troy->GetCoords2()+ipnt2_t(0,1)),
			ipnt2_t(m_Troy->GetCoords2()+ipnt2_t(1,0)),
			ipnt2_t(m_Troy->GetCoords2()+ipnt2_t(1,1)),
			ipnt2_t(m_Troy->GetCoords2()+ipnt2_t(-1,-1))
	};			
	
	for(int i=0;i<4;i++)
	{
		if(!m_Chill->IsBusy(points[i]))
			m_Chill->SetCoords(points[i],0);
	}
}

//**********************************************************************************
//	Scene7
Scene7::Scene7() : ScriptScene(),m_Step(S_TO_DOOR1),m_Door1Open(false),m_Door2Open(false)

{
}

Scene7::~Scene7()
{
}
//	начало скриптовой сцены
bool Scene7::OnStart(void)
{
	if(!GetApi()->IsHumanExist("lor")  && 
		GetApi()->IsHumanExist("moru") &&
		GetApi()->IsHumanExist("player"))
	{
		GetApi()->SetRoofMode(false);
		//	создаем действующие лица
		AddObject(m_Camera = new ScriptSceneAPI::Camera());
		AddObject(m_Door1 = new ScriptSceneAPI::Object("depot_s"));
		AddObject(m_Door2 = new ScriptSceneAPI::Object("depot01a"));
		AddObject(m_Moru = new ScriptSceneAPI::Human("moru"));
		AddObject(m_Tomas = new ScriptSceneAPI::Human("tomas"));
		AddObject(m_Hacker = new ScriptSceneAPI::Human("player"));
		AddObject(m_Timer = new ScriptSceneAPI::Timer("timer"));
		//	задаем начальные действия
		m_Camera->FocusOn(m_Moru->GetCoords3(),0);
		m_Camera->Activated(true);
		m_Tomas->ChangeTeam(ScriptSceneAPI::Human::TT_NONE);
		
		return true;
	}

	return m_Erase = false;
}
//	окончание скриптовой сцены
bool Scene7::OnFinish(void)
{
	return true;
}
//	прерывание скриптовой сцены
bool Scene7::OnSkip(void)
{//	результат:
	if(IsValid())
	{
		//	двери открыты
		if(!m_Door1Open) m_Door1->SwitchState();
		if(!m_Door2Open) m_Door2->SwitchState();
		//	Мору должен остановиться и исчезнуть
		if(GetApi()->IsHumanExist("moru"))
		{
			m_Moru->Stop();
			m_Moru->ItemsToGround(m_Hacker->GetCoords2());
			m_Moru->ChangeTeam(ScriptSceneAPI::Human::TT_NONE);
			m_Moru->Destroy();
		}
		//	Томас должен остановиться и исчезнуть
		if(GetApi()->IsHumanExist("tomas"))
		{
			m_Tomas->Stop();
			m_Tomas->Destroy();
		}
	}
	else m_Erase = false;

	return true;
}
//	передача управления скриптовой сцене
bool Scene7::OnTick(void)
{
	switch(m_Step)
	{
	case S_TO_DOOR2:
	case S_TOMAS_GO:
		m_Camera->FocusOn(m_Moru->GetCoords3(),0);
		break;
	case S_TOMAS_LEAVE:
		m_Camera->FocusOn(m_Tomas->GetCoords3(),0);
		break;
	}

	return true;
}
//	окончание действия объекта скриптовой сцены
bool Scene7::OnObjectNoActive(Object* object)
{
	static bool one_finish = false;

	if(!strcmp(object->Name(),"camera"))
	{
		object->Activated(false);
		switch(m_Step)
		{
		case S_TO_DOOR1:
			m_Moru->RunTo(m_Door1);
			m_Moru->Activated(true);
			m_Camera->FocusOn(m_Door2->GetCoords3(),0);
			m_Step = S_TO_DOOR2;
			break;
		}

		return true;
	}
	if(!strcmp(object->Name(),"moru0"))
	{
		switch(m_Step)
		{
		case S_TO_DOOR2:
			object->Activated(false);
			m_Timer->SetTime(m_Door1->SwitchState()-Timer::GetSeconds());
			m_Timer->Start();
			m_Door1Open = true;
			m_Timer->Activated(true);
			break;
		case S_TOMAS_GO:
			object->Activated(false);
			m_Timer->SetTime(m_Door2->SwitchState()-Timer::GetSeconds());
			m_Timer->Start();
			m_Door2Open = true;
			m_Timer->Activated(true);
			break;
		}

		return true;
	}
	if(!strcmp(object->Name(),"tomas0"))
	{
		switch(m_Step)
		{
		case S_TOMAS_GO:
			object->Activated(false);
			m_Moru->Talk(GetApi()->GetSSPhrase("episode1.scene7.moru1"));
			m_Tomas->Talk(GetApi()->GetSSPhrase("episode1.scene7.tomas1"));
			m_Moru->Talk(GetApi()->GetSSPhrase("episode1.scene7.moru2"));
			m_Tomas->Talk(GetApi()->GetSSPhrase("episode1.scene7.tomas2"));
			m_Moru->Talk(GetApi()->GetSSPhrase("episode1.scene7.moru3"));
			m_Moru->RunTo(ipnt2_t(24,3));
			m_Moru->Activated(true);
			m_Timer->SetTime(1);
			m_Timer->Start();
			m_Timer->Activated(true);
			m_Step = S_MORU_LEAVE;
			break;
		case S_TOMAS_LEAVE:
			if(!m_Moru->OnThink())
			{
				object->Activated(false);
				m_Hacker->Talk(GetApi()->GetSSPhrase("episode1.scene7.hacker"));
				m_Tomas->Destroy();
				m_Moru->ItemsToGround(m_Hacker->GetCoords2());
				m_Moru->ChangeTeam(ScriptSceneAPI::Human::TT_NONE);
				m_Moru->Destroy();
				m_Step = S_FINISH;
				return false;
			}
			break;
		}

		return true;
	}
	if(!strcmp(object->Name(),"timer"))
	{
		switch(m_Step)
		{
		case S_TO_DOOR2:
			m_Timer->Activated(false);
			m_Moru->RunTo(m_Door2);
			m_Moru->Activated(true);
			m_Step = S_TOMAS_GO;
			break;
		case S_TOMAS_GO:
			m_Timer->Activated(false);
			m_Tomas->WalkTo(m_Moru);
			m_Tomas->Activated(true);
			break;
		case S_MORU_LEAVE:
			m_Timer->Activated(false);
			m_Tomas->RunTo(ipnt2_t(24,4));
			m_Tomas->Activated(true);
			m_Step = S_TOMAS_LEAVE;
			break;
		}

		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////
//	мультик по окончании первого эпизода
ScriptScene* Episode1::FinalEpisodeVideo(void)
{
	return new VideoScriptScene("e2_1.bik");
}
/////////////////////////////////////////////////////////////////////////


