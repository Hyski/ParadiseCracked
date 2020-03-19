/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   08.10.2001

************************************************************************/
#include "scriptpch.h"
#include "ScriptSceneAPI.h"
#include "SSEpisode3.h"

using namespace Episode3;

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
	int i;
	char buff[50];

	//	создаем действующие лица
	AddObject(m_Camera = new ScriptSceneAPI::Camera());
	AddObject(m_Timer = new ScriptSceneAPI::Timer("timer"));
	for(i=0;i<MAX_DOOR;i++)
	{
		sprintf(buff,"door_%d",i+1);
		AddObject(m_Doors[i] = new ScriptSceneAPI::Object(buff));
		if(i == 1) m_Timer->SetTime(m_Doors[1]->SwitchState()-Timer::GetSeconds());
		else m_Doors[i]->SwitchState();
	}
	for(i=0;i<MAX_ENTR;i++)
	{
		sprintf(buff,"door_entr_0%d",i+1);
		AddObject(m_Entrs[i] = new ScriptSceneAPI::Object(buff));
		m_Entrs[i]->SwitchState();
	}
	m_Camera->FocusOn(m_Doors[1]->GetCoords3(),0);
	m_Timer->Start();
	m_Timer->Activated(true);

	return true;
}
//	окончание скриптовой сцены
bool Scene1::OnFinish(void)
{
	return true;
}
//	прерывание скриптовой сцены
bool Scene1::OnSkip(void)
{
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
	if(!strcmp(object->Type(),"timer"))
	{
		object->Activated(false);
		return false;
	}

	return true;
}

//**********************************************************************************
//	Scene2
const ipnt2_t Scene2::m_UpLeftCorner = ipnt2_t(54,34);
const int Scene2::m_Length = 40;
const ipnt2_t Scene2::m_FinishPoint[MAX_SPECCOP] = 
{
	ipnt2_t(15,36),
	ipnt2_t(24,9),
	ipnt2_t(25,20),
	ipnt2_t(46,50),
	ipnt2_t(46,65)
};
Scene2::Scene2() : ScriptScene(),m_Stage(ST_START)
{
}

Scene2::~Scene2()
{
}
//	начало скриптовой сцены
bool Scene2::OnStart(void)
{
	int counter = 0;

	//	создаем действующие лица
	AddObject(m_Camera = new ScriptSceneAPI::Camera());
	AddObject(m_Timer = new ScriptSceneAPI::Timer("timer"));
	//	создание ирасстановка копов
	for(int i=0;i<MAX_SPECCOP;i++)
	{
		AddObject(m_SpecCops[i] = new ScriptSceneAPI::Human("specops",ScriptSceneAPI::Human::CM_NEW));
		while(counter != m_Length)
		{
			ipnt2_t xy(m_UpLeftCorner.x+counter/MAX_SPECCOP,m_UpLeftCorner.y+counter%MAX_SPECCOP);

			if(!m_SpecCops[i]->IsBusy(xy))
			{
				m_SpecCops[i]->SetCoords(xy,-1);
				m_SpecCops[i]->Show(true);
				m_SpecCops[i]->GiveWeapon("k16","standart_army_shell",30,ScriptSceneAPI::Human::PK_HANDS);
				m_SpecCops[i]->GiveArmor("nohomoto",ScriptSceneAPI::Human::PK_BODY);
				m_SpecCops[i]->WalkTo(ipnt2_t(xy.x-6,xy.y));
				m_SpecCops[i]->Activated(true);
				break;
			}
			counter++;
		}
	}

	m_Camera->FocusOn(m_SpecCops[2]->GetCoords3(),0);
	GetApi()->SetRoofMode(false);

	return true;
}
//	окончание скриптовой сцены
bool Scene2::OnFinish(void)
{
	if(IsValid())
	{
		for(int i=0;i<MAX_SPECCOP;i++)
		{
			m_SpecCops[i]->SetTeam(ScriptSceneAPI::Human::TT_ENEMY);
			m_SpecCops[i]->SetBehaviourModel("ss_a");
		}
	}

	return true;
}
//	прерывание скриптовой сцены
bool Scene2::OnSkip(void)
{
	if(IsValid())
	{
		int i;
		
		m_Stage = ST_SKIP;
		for(i=0;i<MAX_SPECCOP;i++)
			m_SpecCops[i]->Stop();
		if(CheckAllSpecCopsNoThink())
		{
			for(i=0;i<MAX_SPECCOP;i++)
			{
				if(!m_SpecCops[i]->IsBusy(m_FinishPoint[i])) m_SpecCops[i]->SetCoords(m_FinishPoint[i],-1);
				m_SpecCops[i]->SetTeam(ScriptSceneAPI::Human::TT_ENEMY);
				m_SpecCops[i]->SetBehaviourModel("ss_a");
			}
			m_SpecCops[1]->SetSitMode(true);
			m_SpecCops[2]->SetSitMode(true);
			m_SpecCops[4]->SetSitMode(true);
		}
		else return false;
	}

	return true;
}
//	передача управления скриптовой сцене
bool Scene2::OnTick(void)
{
	switch(m_Stage)
	{
	case ST_START:
		m_Camera->FocusOn(m_SpecCops[2]->GetCoords3(),0);
		break;
	case ST_ONE_GROUP:
		m_Camera->FocusOn(m_SpecCops[0]->GetCoords3(),0);
		break;
	}

	return true;
}
//	окончание действия объекта скриптовой сцены
bool Scene2::OnObjectNoActive(Object* object)
{
	switch(m_Stage)
	{
	case ST_START:
		if(!strcmp(object->Type(),"human"))
		{
			object->Activated(false);
			if(CheckAllSpecCopsStoped())
			{
				m_Timer->SetTime(1.5f);
				m_Timer->Start();
				m_Timer->Activated(true);
			}
		}
		if(!strcmp(object->Type(),"timer"))
		{
			m_Timer->SetTime(0.5f);
			m_Timer->Start();
			m_Stage = ST_ONE_GROUP;
			m_SpecCops[0]->RunTo(m_FinishPoint[0]);
			m_SpecCops[0]->Activated(true);
			m_SpecCops[4]->RunTo(m_FinishPoint[4]);
			m_SpecCops[4]->Activated(true);
		}
		break;
	case ST_ONE_GROUP:
		if(!strcmp(object->Type(),"timer"))
		{
			m_Timer->Activated(false);
			m_SpecCops[1]->RunTo(m_FinishPoint[1]);
			m_SpecCops[1]->Activated(true);
			m_SpecCops[2]->RunTo(m_FinishPoint[2]);
			m_SpecCops[2]->Activated(true);
			m_SpecCops[3]->RunTo(m_FinishPoint[3]);
			m_SpecCops[3]->Activated(true);
		}
		if(!strcmp(object->Type(),"human"))
		{
			object->Activated(false);
			if(object == m_SpecCops[1])
				m_SpecCops[1]->SetSitMode(true);
			if(object == m_SpecCops[2])
				m_SpecCops[2]->SetSitMode(true);
			if(object == m_SpecCops[4])
				m_SpecCops[4]->SetSitMode(true);
			if(CheckAllSpecCopsStoped())
			{
				m_Stage = ST_FINISH;
				return false;
			}
		}
		break;
	case ST_SKIP:
		if(!strcmp(object->Type(),"human"))
		{
			if(!m_Timer->IsActivated())
			{
				m_Timer->SetTime(0.4f);
				m_Timer->Start();
				m_Timer->Activated(true);
			}
		}
		if(!strcmp(object->Type(),"timer"))
		{
			return !OnSkip();
		}
		break;
	}

	return true;
}

bool Scene2::CheckAllSpecCopsStoped(void)
{
	for(int i=0;i<MAX_SPECCOP;i++)
	{
		if(m_SpecCops[i]->IsActivated())
			return false;
	}

	return true;
}

bool Scene2::CheckAllSpecCopsNoThink(void)
{
	for(int i=0;i<MAX_SPECCOP;i++)
	{
		if(m_SpecCops[i]->OnThink())
			return false;
	}

	return true;
}

//**********************************************************************************
//	Scene3
Scene3::Scene3() : ScriptScene(),m_CameraStepOne(true),m_TimerStepOne(true),m_Part(PN_FIRST),m_SkipGemBlast(false)
{
	// объект для работы с Mortimer
	ScriptSceneAPI::Human* mortimer = new ScriptSceneAPI::Human("mortimer");
	m_MortimerDeath = mortimer->GetCoords2();
	delete mortimer;
}

Scene3::~Scene3()
{
}
//	начало скриптовой сцены
bool Scene3::OnStart(void)
{
	//	создаем действующие лица
	AddObject(m_Hacker = new ScriptSceneAPI::Human("player"));
	AddObject(m_Gem = new ScriptSceneAPI::Vehicle("gem"));
	AddObject(m_Camera = new ScriptSceneAPI::Camera());
	AddObject(m_Timer = new ScriptSceneAPI::Timer("timer"));

	m_Timer->SetTime(1);
	m_Timer->Start();
	m_Timer->Activated(true);

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
	switch(m_Part)
	{
	case PN_FIRST:
		{
			if(!m_SkipGemBlast)
			{
				m_Gem->ToBlast();
				m_Camera->FocusOn(m_Hacker->GetCoords3(),0);
			}
		}
		break;
	case PN_SECOND:
		{
			m_Tony->Stop();
			if(m_Tony->OnThink()) return false;
		}
		break;
	}

	return true;
}
//	передача управления скриптовой сцене
bool Scene3::OnTick(void)
{
	if(m_Part == PN_SECOND)
	{
		m_Camera->FocusOn(m_Tony->GetCoords3(),0);
	}

	return true;
}
//	окончание действия объекта скриптовой сцены
bool Scene3::OnObjectNoActive(Object* object)
{
	switch(m_Part)
	{
	case PN_FIRST:
		{
			if(!strcmp(object->Type(),"timer"))
			{
				if(m_CameraStepOne)
				{
					m_Camera->FocusOn(m_Gem->GetCoords3(),1);
					m_CameraStepOne = false;
				}
				else m_Camera->FocusOn(m_Hacker->GetCoords3(),1);
				m_Camera->Activated(true);
				
				object->Activated(false);
				return true;
			}
			if(!strcmp(object->Type(),"camera"))
			{
				if(m_TimerStepOne)
				{
					m_SkipGemBlast = true;
					m_Gem->ToBlast();
					m_Timer->SetTime(1);
					m_Timer->Start();
					m_Timer->Activated(true);
					m_TimerStepOne = false;
				}
				else
				{
					m_Hacker->Talk(GetApi()->GetSSPhrase("episode3.scene3.hacker1"));
					//	инициализируем вторую часть сцены
					if(GetApi()->IsHumanExist("tony") && GetApi()->IsQuestExecute("quest_03_09"))
					{	
						AddObject(m_Tony = new ScriptSceneAPI::Human("tony"));
						m_Tony->RunTo(m_MortimerDeath);
						m_Tony->Activated(true);

						m_Part = PN_SECOND;
					}
					else return false;
				}
				
				object->Activated(false);
			}
		}
		break;
	case PN_SECOND:
		{
			if(!strcmp(object->Type(),"human"))
			{//	Тони добежал до места гибели Мортимера, теперь будем разговаривать
				m_Tony->Talk(GetApi()->GetSSPhrase("episode3.scene3.tony1"));
				m_Hacker->Talk(GetApi()->GetSSPhrase("episode3.scene3.hacker2"));
				m_Tony->Talk(GetApi()->GetSSPhrase("episode3.scene3.tony2"));
				//	проверяем присутсвие Джоба
				if(GetApi()->IsHumanExist("job"))
				{
					AddObject(m_Job = new ScriptSceneAPI::Human("job"));
					m_Job->Talk(GetApi()->GetSSPhrase("episode3.scene3.job"));
					m_Tony->Talk(GetApi()->GetSSPhrase("episode3.scene3.tony3"));
				}

				return false;
			}
		}
		break;
	}

	return true;
}

//**********************************************************************************
//	Scene4
Scene4::Scene4() : ScriptScene()
{
}

Scene4::~Scene4()
{
}
//	начало скриптовой сцены
bool Scene4::OnStart(void)
{
	//	создаем действующие лица
	AddObject(m_Camera = new ScriptSceneAPI::Camera());
	AddObject(m_Door = new ScriptSceneAPI::Object("ffield_01"));
	m_Door->SwitchState();
	m_Camera->FocusOn(m_Door->GetCoords3(),1);
	m_Camera->Activated(true);

	return true;
}
//	окончание скриптовой сцены
bool Scene4::OnFinish(void)
{
	return true;
}
//	прерывание скриптовой сцены
bool Scene4::OnSkip(void)
{
	return true;
}
//	передача управления скриптовой сцене
bool Scene4::OnTick(void)
{
	return true;
}
//	окончание действия объекта скриптовой сцены
bool Scene4::OnObjectNoActive(Object* object)
{
	if(!strcmp(object->Type(),"camera"))
	{
		object->Activated(false);
		return false;
	}

	return true;
}
/////////////////////////////////////////////////////////////////////////
//	мультик по окончании третьего эпизода
ScriptScene* Episode3::FinalEpisodeVideo(void)
{
	return new VideoScriptScene("e4_1.bik");
}
/////////////////////////////////////////////////////////////////////////
