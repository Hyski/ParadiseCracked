/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   08.10.2001

************************************************************************/
#ifndef _SS_EPISODE_3_H_
#define _SS_EPISODE_3_H_

namespace Episode3
{

/***********************************************************************************
> Scene1

  После того как на уровне cybercenter робот Gem толкнет первую фразу – 
  на на уровне открываются все двери (желательно чтобы при этом камера 
  показала хотя бы одну из них)
	door_1,door_2,...,door_9
	door_entr_01
	door_entr_02
	door_entr_03
	door_entr_04
	центроваться на door_2

************************************************************************************/
class Scene1 : public ScriptScene
{
private:
	enum {MAX_DOOR = 9,MAX_ENTR = 4};
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	// объекты для работы с дверьми
	ScriptSceneAPI::Object* m_Doors[MAX_DOOR];
	// объекты для работы с дверьми
	ScriptSceneAPI::Object* m_Entrs[MAX_ENTR];
	//	объект для работы с таймером
	ScriptSceneAPI::Timer* m_Timer;
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

  После того как играющий поговорит с HELEN на уровне RADAR,
  на уровень вбегает агент и три specops.

************************************************************************************/
class Scene2 : public ScriptScene
{
	enum {MAX_SPECCOP = 5};
	static const ipnt2_t m_UpLeftCorner;
	static const int m_Length;
	static const ipnt2_t m_FinishPoint[MAX_SPECCOP];
private:
	enum STAGE {ST_START,ST_ONE_GROUP,ST_SKIP,ST_FINISH};
	STAGE m_Stage;
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	//	объект для работы с таймером
	ScriptSceneAPI::Timer* m_Timer;
	//	объект для работы со спецкопами
	ScriptSceneAPI::Human* m_SpecCops[MAX_SPECCOP];
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
	bool CheckAllSpecCopsStoped(void);
	bool CheckAllSpecCopsNoThink(void);

};

/***********************************************************************************
> Scene3

1) В момент смерти доктора Мортимера (Mortimer), Камера должна отцентроваться на роботе
с системным именем Gem и он должен взорваться.
2) Если в команде есть Тони и выполняется квест quest_03_09, то Тони подбегает к
месту где погиб доктор Мортимер (Mortimer) и говорит фразу, затем говорит Хакер,
потом снова Тони. Если в команде есть Джоб, то после Тони он тоже говорит фразу и
Тони ему отвечает.

***********************************************************************************/
class Scene3 : public ScriptScene
{
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	//	объект для работы с таймером
	ScriptSceneAPI::Timer* m_Timer;
	// объект для работы с хакером
	ScriptSceneAPI::Human* m_Hacker;
	// объект для работы с Тони
	ScriptSceneAPI::Human* m_Tony;
	// объект для работы с Джобом
	ScriptSceneAPI::Human* m_Job;
	// объект для работы с техникой
	ScriptSceneAPI::Vehicle* m_Gem;
	//	координаты смерти Mortimer
	ipnt2_t m_MortimerDeath;
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
///////////////////////////    class Scene4    //////////////////////////
/////////////////////////////////////////////////////////////////////////
//
//	На уровне spaceport, эпизод 3. При смерти gann должна открываться 
//	дверь FField_01.
//
/////////////////////////////////////////////////////////////////////////
class Scene4 : public ScriptScene
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
	Scene4();
	virtual ~Scene4();
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
//	мультик по окончании третьего эпизода
ScriptScene* FinalEpisodeVideo(void);
/////////////////////////////////////////////////////////////////////////


}

#endif	//_S_S_EPISODE_3_H_