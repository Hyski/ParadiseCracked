/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   03.09.2001

************************************************************************/
#ifndef _SS_EPISODE_2_H_
#define _SS_EPISODE_2_H_

#include "ScriptSceneManager.h"

/*class ScriptSceneAPI::Human;
class ScriptSceneAPI::Camera;*/

namespace Episode2
{

/***********************************************************************************
> Scene1

Сцена с грузовиком
Когда играющий попадает на уровень Prison, при этом у него есть
грузовик (mini_truck), а на уровне еще целые ворота (объекты с именами Door_Entr_01 и
Door_Entrance02), то происходит следующее - грузовик едет к точке
перед дверью (хекс с координатами 32,47) в момент когда этот хекс
достигается, дверь должна взорваться, не останавливаясь в хексе
грузовик едет в хекс 32,40 и там останавливается, после этого
Door_Entrance02 открывается.


************************************************************************************/
class Scene1 : public ScriptScene
{
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	// объект для работы с техникой
	ScriptSceneAPI::Vehicle* m_MiniTruck;
	// объект для работы с объектом
	ScriptSceneAPI::Object* m_Gate1;
	// объект для работы с объектом
	ScriptSceneAPI::Object* m_Gate2;
	//	объект для работы с таймером
	ScriptSceneAPI::Timer* m_Timer;
	//	мини-трак двигается на вторую точку
	enum MINI_TRACK_ACTION {MT_TO_PRIMARY_POINT,MT_TO_SECONDARY_POINT,MT_ON_SKIP};
	MINI_TRACK_ACTION m_mtAction;
public:
	Scene1();
	virtual ~Scene1();
public:
	//	начало скриптовой сцены
	bool OnStart(void);
	//	окончание скриптовой сцены
	bool OnFinish(void);
	//	прерывание скриптовой сцены
	bool OnSkip(void);
	//	окончание действия объекта скриптовой сцены
	bool OnObjectNoActive(Object* object);
	//	передача управления скриптовой сцене
	bool OnTick(void);
};

/***********************************************************************************
> Scene2
Во втором эпизоде уровень lift По завершении квеста 1_11_quest происходит следущее:
Tom выбегает из здания, заворачивает за угол (короче бежит к зоне выхода 4,21) и 
просто исчезает с уровня
В зоне с названием {координаты} появляются 6 soldier’ов, и эти шесть soldier’ов
Бегут к player’у и становятся вокруг него. (ну выбирают доступные хексы в радиусе 3 
хексов от него, если таких нет, радиус расширяется до 4, если нет до 5 и т.д.)

************************************************************************************/
class Scene2 : public ScriptScene
{
	enum {MAX_SOLDIER = 6};
	enum PHASE {P_ONE,P_SECOND};
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	//	объект для работы с таймером
	ScriptSceneAPI::Timer* m_Timer;
	// объект для работы с хакером
	ScriptSceneAPI::Human* m_Hacker;
	// объект для работы с Tom
	ScriptSceneAPI::Trader* m_Tom;
	//	объект для работы со солдатами
	ScriptSceneAPI::Human* m_Soldiers[MAX_SOLDIER];
	PHASE m_Phase;
	bool m_Skipped;
public:
	Scene2();
	virtual ~Scene2();
public:
	//	начало скриптовой сцены
	bool OnStart(void);
	//	окончание скриптовой сцены
	bool OnFinish(void);
	//	прерывание скриптовой сцены
	bool OnSkip(void);
	//	окончание действия объекта скриптовой сцены
	bool OnObjectNoActive(Object* object);
	//	передача управления скриптовой сцене
	bool OnTick(void);
private:
	bool IsAllSoldiersInactive(void);
	bool CheckAllSoldiersNoThink(void);
	void SpawnSoldier(void);
	void SetTraits(ScriptSceneAPI::Human* soldier);
};

/////////////////////////////////////////////////////////////////////////
///////////////////////////    class Scene3    //////////////////////////
/////////////////////////////////////////////////////////////////////////
//
//	На уровне science при смерти RG должна открываться дверь Door_01.
//
/////////////////////////////////////////////////////////////////////////
class Scene3 : public ScriptScene
{
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	// объект для работы с дверью
	ScriptSceneAPI::Object* m_Door;
private:
	enum PART_NAME {PN_FIRST,PN_SECOND};
	bool m_CameraStepOne;
	bool m_TimerStepOne;
	bool m_SkipGemBlast;
	PART_NAME m_Part;
public:
	Scene3();
	virtual ~Scene3();
public:
	//	начало скриптовой сцены
	bool OnStart(void);
	//	окончание скриптовой сцены
	bool OnFinish(void);
	//	прерывание скриптовой сцены
	bool OnSkip(void);
	//	окончание действия объекта скриптовой сцены
	bool OnObjectNoActive(Object* object);
	//	передача управления скриптовой сцене
	bool OnTick(void);
};

/////////////////////////////////////////////////////////////////////////
//	мультик по окончании второго эпизода
ScriptScene* FinalEpisodeVideo1(void);
ScriptScene* FinalEpisodeVideo2(void);
/////////////////////////////////////////////////////////////////////////

}

#endif	//_SS_EPISODE_2_H_