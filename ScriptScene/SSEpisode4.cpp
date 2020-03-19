/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.10.2001

************************************************************************/
#include "scriptpch.h"
#include "ScriptSceneAPI.h"
#include "SSEpisode4.h"

using namespace Episode4;

//**********************************************************************************
//	Scene1
Scene1::Scene1() : ScriptScene(),m_Stage(ST_ONE),m_Door(0)
{
	ScriptSceneAPI::Vehicle* roby = new ScriptSceneAPI::Vehicle("roby");

	m_Point = roby->GetCoords3();
	roby->Talk(ScriptSceneAPI::Instance()->GetSSPhrase("episode4.scene1.roby"));

	delete roby;
}

Scene1::~Scene1()
{
}
//	начало скриптовой сцены
bool Scene1::OnStart(void)
{
	//	создаем действующие лица
	AddObject(m_Camera = new ScriptSceneAPI::Camera());
	AddObject(m_Timer = new ScriptSceneAPI::Timer("timer"));
	AddObject(m_Door = new ScriptSceneAPI::Object("door_16"));
	m_Camera->FocusOn(m_Point,2);
	m_Camera->Activated(true);

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
	if(!m_Door) AddObject(m_Door = new ScriptSceneAPI::Object("door_16"));
	if(m_Stage != ST_THREE) m_Door->SwitchState();

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
		switch(m_Stage)
		{
		case ST_ONE:
			m_Camera->FocusOn(m_Door->GetCoords3(),1);
			m_Stage = ST_TWO;
			break;
		case ST_TWO:
			object->Activated(false);
			m_Timer->SetTime(m_Door->SwitchState()-Timer::GetSeconds());
			m_Timer->Start();
			m_Timer->Activated(true);
			m_Stage = ST_THREE;
			break;
		}
		
		return true;
	}
	if(!strcmp(object->Type(),"timer"))
	{
		object->Activated(false);
		return false;
	}


	return true;
}

