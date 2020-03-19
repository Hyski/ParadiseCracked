/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   06.07.2001

************************************************************************/
#ifndef _SS_EPISODE_1_H_
#define _SS_EPISODE_1_H_

#include "ScriptSceneManager.h"

class ScriptSceneAPI::Human;
class ScriptSceneAPI::Camera;

namespace Episode1
{

/***********************************************************************************
> Scene1

Начинается при первом запуске игры (new game).
Bartolomiu произносит свою фразу, после того как игрок нажимает back хакер бежит (именно бежит) к зоне выхода. Пока он бежит по нему стреляет полицейский. Когда он добегает до зоны выхода, то СРАЗУ начинается загрузка china_town. 
Камера задается в максе.

Резюме - после сцены считается что хакер поговорил с bartolomiu. Загружается china_town.

************************************************************************************/
class Scene1 : public ScriptScene
{
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	// объект для работы с хакером
	ScriptSceneAPI::Human* m_Hacker;
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

Когда хакер впервые попадает на china_town (то есть сразу после первой скриптовой сцены).
То Стартует заранее заданная камера которая едет по уровню и залетает в дом к sam lee.
По пути камеры мимо проходит prostitute и soldier.
Камера заезжает в дом к sam lee и показывает его, при этом sam lee ходит внутри дома туда-сюда.
В пределах трех хексов от места его первоначального рождения.
Камера делается в максе
Резюмируя - в сцене ничего не происходит, ее предназначение показать играющему где находится sam lee.

************************************************************************************/
class Scene2 : public ScriptScene
{
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	//	объект для работы с таймером
	ScriptSceneAPI::Timer* m_Timer;
	// объект для работы с хакером
	ScriptSceneAPI::Human* m_Hacker;
	// объект для работы с SamLee
	ScriptSceneAPI::Trader* m_SamLee;
private:
	bool m_FirstCameraAction;
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
	bool OnTick(void) {return true;}
};

/***********************************************************************************
> Scene3
Когда игрок выходит с уровня china_town на factory. 
Загружается спец уровень china_town_upgrade.
Раставляется все персонажы из команады на крыше дома.
Прилетает самолет и садится на площадку, хакер и все кто в его комаде исчезают, самолет взлетает, загружается factory.
Камера делается в максе
Резюмируя - загружается уровень factory.

************************************************************************************/
class Scene3 : public ScriptScene
{
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	// объект для работы с анимацией
	ScriptSceneAPI::Animation* m_Jumper;
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
	bool OnTick(void) {return true;}
};

/***********************************************************************************
> Scene4
Когда играющий убивает Loris то камера центруется на ней,
показывается ее смерть после чего камера медленно вращаясь
риближается к ее трупу на расстояние где то порядка 2 метров
центруясь на выпавшем из лорис кристалле. 
Как только камера приблизилась на 2 м. 
Камера центруется на хакере и тот толкает фразу. 
После нажатия кнопки back игроком сцена заканчивается. 
Камера управляется программой.
Резюмируя - сцена ничего не меняет для игрока.

************************************************************************************/
class Scene4 : public ScriptScene
{
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	//	объект для работы с таймером
	ScriptSceneAPI::Timer* m_Timer;
	// объект для работы с хакером
	ScriptSceneAPI::Human* m_Hacker;
	//	координаты смерти Loris
	point3 m_LorisDeath;
public:
	Scene4();
	virtual ~Scene4();
public:
	//	начало скриптовой сцены
	bool OnStart(void);
	//	окончание скриптовой сцены
	bool OnFinish(void) {return true;}
	//	прерывание скриптовой сцены
	bool OnSkip(void) {return true;}
	//	окончание действия объекта скриптовой сцены
	bool OnObjectNoActive(Object* object);
	//	передача управления скриптовой сцене
	bool OnTick(void) {return true;}
};

/***********************************************************************************
> Scene5

Когда играющий впервые попадает на уровень china_town  в фазе 2 или 3,
то показывается сцена - из-за края уровня выходят четыре штурмовика,
сзади них двигается cyber_spider. Первый штурмовик толкает фразу,
после нажатия кнопки back Сцена прекращается камера снова центруется на хакере.
Камера задается в Максе.
Резюме - никакого значения не имеет,
так как после сцены все штурмовики и кибер спайдеры расставляются в назначенные места рождения.

************************************************************************************/
class Scene5 : public ScriptScene
{
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	// объекты для работы с персонажами
	ScriptSceneAPI::Human* m_Stormtroopers[3];
public:
	Scene5();
	virtual ~Scene5();
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
	bool OnTick(void) {return true;}
};

/***********************************************************************************
> Scene6

Когда играющий говорит с troy (на cityhall) то открывается условная дверь
(которая до этого была закрыта, и оттуда выбегает chill, камера при этом едет за ним,
chill должен подбежать на оближайший свободный хекс к мэру и повернуться к ближайшему
 персонажу играющего. После этого troy толкает фразу. Если в команде есть job то тот
 тоже толкает фразу.
Камера управляется программой.
Результат - считается что играющий поговорил с мэром и соответственно получил квест
даваемый им. Кроме того, остается открытой условная дверь в комнату где был chill,
chill появляется на ближайшем свободном хексе к мэру.

************************************************************************************/
class Scene6 : public ScriptScene
{
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	// объект для работы с объектом
	ScriptSceneAPI::Object* m_Door1;
	ScriptSceneAPI::Object* m_Door2;
	ScriptSceneAPI::Object* m_Door3;
	//	объект для работы с таймером
	ScriptSceneAPI::Timer* m_Timer;
	// объекты для работы с персонажами
	ScriptSceneAPI::Human* m_Troy;
	ScriptSceneAPI::Human* m_Chill;
private:
	enum STATE {S_FIRST_DOOR,S_SECOND_DOOR,S_THIRD_DOOR,
				S_RUNNED_TO_TROY,S_TROY,S_ROTATE_TO_TROY,
				S_FINISH,S_ON_SKIP};
	STATE m_State;
public:
	Scene6();
	virtual ~Scene6();
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
	void ChillToTroy(void);
};


/***********************************************************************************
> Scene7
************************************************************************************/
class Scene7 : public ScriptScene
{
private:
	// объект для работы с камерой
	ScriptSceneAPI::Camera* m_Camera;
	// объект для работы с объектом
	ScriptSceneAPI::Object* m_Door1;
	ScriptSceneAPI::Object* m_Door2;
	// объекты для работы с персонажами
	ScriptSceneAPI::Human* m_Moru;
	ScriptSceneAPI::Human* m_Tomas;
	ScriptSceneAPI::Human* m_Hacker;
	//	объект для работы с таймером
	ScriptSceneAPI::Timer* m_Timer;
private:
	enum STEP {S_TO_DOOR1,S_TO_DOOR2,S_TOMAS_GO,S_MORU_LEAVE,S_TOMAS_LEAVE,S_FINISH};
	STEP m_Step;
	bool m_Door1Open;
	bool m_Door2Open;
public:
	Scene7();
	virtual ~Scene7();
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
//	мультик по окончании первого эпизода
ScriptScene* FinalEpisodeVideo(void);
/////////////////////////////////////////////////////////////////////////

}

#endif	//_SS_EPISODE_1_H_