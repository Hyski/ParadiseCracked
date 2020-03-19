/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.10.2001

************************************************************************/
#ifndef _S_S_EPISODE_4_H_
#define _S_S_EPISODE_4_H_

namespace Episode4
{

/***********************************************************************************
> Scene1

  После того, как на уровне reactor робот roby получает последнее попадание
  (в смысле смертельно) – камера центруется на нем, он толкает речь,
  после чего умирает.

************************************************************************************/
class Scene1 : public ScriptScene
{
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	//	объект для работы с таймером
	ScriptSceneAPI::Timer* m_Timer;
	// объекты для работы с дверью
	ScriptSceneAPI::Object* m_Door;
	//	точка
	point3 m_Point;
	enum STAGE {ST_ONE,ST_TWO,ST_THREE};
	STAGE m_Stage;
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


}

#endif	//_S_S_EPISODE_4_H_