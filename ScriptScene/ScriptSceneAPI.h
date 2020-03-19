/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: интерфейс для скриптовых сцен со стороны основного проекта
				
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#if !defined(__SCRIPT_SCENE_API_H__)
#define __SCRIPT_SCENE_API_H__

#include "ScriptSceneManager.h"

class BaseEntity;
class Activity;
class SkAnim;
class SkSkin;
struct AnimaData;

//**********************************************************************
//	class ScriptSceneAPI
class ScriptSceneAPI
{
public:
	class Human;
	class Trader;
	class Vehicle;
	class Camera;
	class Timer;
	class Animation;
	class Object;
private:
	struct Deleter 
	{
	public:
		ScriptSceneAPI* m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	float m_AnimationSpeed1;
	float m_AnimationSpeed2;
	bool m_RoofVisible;
private:
	ScriptSceneAPI();
	virtual ~ScriptSceneAPI();
public:
	static ScriptSceneAPI *Instance(void);
public:
	//	**** управление скриптовой сценой
	void StartScene(void);
	void FinishScene(void);
	//	получить текущее время работы программы
	float GetTime(void) const;
	//	выйти на другой уровень
	void ExitFromLevel(const ipnt2_t& pt);
	//	получить фразу из xls'ки
	std::string GetSSPhrase(const char* phrase_name);
	//	включить/выключить крыши
	void SetRoofMode(bool visible);
	//	проиграть мультик
	void PlayBink(const char* file_name);
	//	проверить, есть ли человек на уровне
	bool IsHumanExist(const char* name);
	//	проверить активирование определенного квеста
	bool IsQuestExecute(const char* name);
	//	проверить успешное завершение определенного квеста
	bool IsQuestOk(const char* name);
};

inline ScriptSceneAPI* ScriptSceneAPI::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new ScriptSceneAPI();
	return m_Deleter.m_pInstance;
}

//**********************************************************************
//	class ScriptSceneAPI::Vehicle
class ScriptSceneAPI::Vehicle : public ScriptScene::Object
{
private:
	BaseEntity* m_Entity;
	Activity* m_Activity;
/*	bool m_RunMode;
	bool m_SitMode;*/
public:
	Vehicle(const char* name);
	virtual ~Vehicle();
public:
	//	существует ли такой объект
	static bool IsExist(const char* name);
	//	ехать в точку
	void MoveTo(const ipnt2_t& pnt);
/*	// идти в точку
	void WalkTo(const ipnt2_t& pnt);
	void WalkTo(ScriptSceneAPI::Object* object);
	void WalkTo(ScriptSceneAPI::Human* human);
	// бежать в точку
	void RunTo(const ipnt2_t& pnt);
	void RunTo(ScriptSceneAPI::Object* object);
	void RunTo(ScriptSceneAPI::Human* human);
	//	установить доступность точки
	bool IsAccess(ScriptSceneAPI::Object* object);
	bool IsAccess(ScriptSceneAPI::Human* human);
	// заставить человека повернуться
	void RotateToPoint(const point3& pnt);*/
	//	сказать фразу
	void Talk(const std::string& phrase);
	//	остановить технику
	void Stop(void);
	// мгновенно переместить человека в нужную позицию
	void SetCoords(const ipnt2_t& pt,const float angle);
	//	показать/спрятать персонаж
	void Show(bool show);
	//	взорвать технику
	void ToBlast(void);
/*	//	класс стрельбы
	enum SHOT_TYPE {ST_AIMSHOT,ST_SNAPSHOT,ST_AUTOSHOT};
	// выстрелить в точку
	void Shoot(const point3& target, const float accuracy, const SHOT_TYPE st);*/
	//	включить/выключить изменение свойств персонажа
	void LockTraits(bool lock);
	//	получить трехмерные координаты человека
	point3 GetCoords3(void);
	//	получить двумерные (хексовые) координаты человека
	ipnt2_t GetCoords2(void);
	//	вернуть указатель на существо
	BaseEntity* Entity(void) {return m_Entity;}
public:
	//	объект думает (false - объект перестал думать)
	bool OnThink(void);
	//	тип объекта
	const char* Type(void) const {return "vehicle";}
private:
	//	сгенерировать имя
//	std::string GenerateName(const char* name);
};


//**********************************************************************
//	class ScriptSceneAPI::Human
class ScriptSceneAPI::Human : public ScriptScene::Object
{
private:
	static std::map<std::string,int> m_Registration;
	std::string m_SystemName;
private:
	BaseEntity* m_Entity;
	Activity* m_Activity;
	bool m_RunMode;
	bool m_SitMode;
public:
	enum CREATE_MODE {CM_ALREADY_EXIST,CM_NEW};
	Human(const char* name,CREATE_MODE cm = CM_ALREADY_EXIST);		
	virtual ~Human();
public:
	// идти в точку
	void WalkTo(const ipnt2_t& pnt);
	void WalkTo(ScriptSceneAPI::Object* object);
	void WalkTo(ScriptSceneAPI::Human* human);
	// бежать в точку
	void RunTo(const ipnt2_t& pnt);
	void RunTo(ScriptSceneAPI::Object* object);
	void RunTo(ScriptSceneAPI::Human* human);
	//	установить доступность точки
	bool IsAccess(ScriptSceneAPI::Object* object);
	bool IsAccess(ScriptSceneAPI::Human* human);
	//	установить свободна ли эта точка от каких-либо объектов
	bool IsBusy(const ipnt2_t& pnt);
	// заставить человека повернуться
	void RotateToPoint(const point3& pnt);
	//	сказать фразу
	void Talk(const std::string& phrase);
	//	остановить персонажа
	void Stop(void);
	// мгновенно переместить человека в нужную позицию
	void SetCoords(const ipnt2_t& pt,const float angle);
	//	показать/спрятать персонаж
	void Show(bool show);
	//	класс стрельбы
	enum SHOT_TYPE {ST_AIMSHOT,ST_SNAPSHOT,ST_AUTOSHOT};
	// выстрелить в точку
	void Shoot(const point3& target, const float accuracy, const SHOT_TYPE st);
	//	посадить/поставить персонажа
	void SetSitMode(bool sit);
	//	определить модель поведения
	void SetBehaviourModel(const char* label);
	//	дать человеку предмет
	enum PACK_TYPE {PK_HEAD,PK_BODY,PK_HANDS,PK_LKNEE,PK_RKNEE,PK_IMPLANTS,PK_BACKPACK};
	bool GiveWeapon(const char* weapon_name,const char* ammo_name,int ammo_count,PACK_TYPE pt);
	bool GiveArmor(const char* armor_name,PACK_TYPE pt);
	bool GiveAmmo(const char* ammo_name,int ammo_count,PACK_TYPE pt);
	bool GiveGrenade(const char* grenade_name,PACK_TYPE pt);
	bool GiveImplant(const char* implant_name,PACK_TYPE pt);
	//	установить команду для человека
	enum TEAM_TYPE {TT_NONE,TT_PLAYER,TT_ENEMY};
	void SetTeam(TEAM_TYPE tt);
	//	поменять команду для человека
	void ChangeTeam(TEAM_TYPE tt);
	//	включить/выключить изменение свойств персонажа
	void LockTraits(bool lock);
	//	получить трехмерные координаты человека
	point3 GetCoords3(void);
	//	получить двумерные (хексовые) координаты человека
	ipnt2_t GetCoords2(void);
	//	вернуть указатель на существо
	BaseEntity* Entity(void) {return m_Entity;}
	//	уничтожить существо
	void Destroy(void);
	//	сбросить все вещи на землю
	void ItemsToGround(const ipnt2_t& pt);
public:
	//	объект думает (false - объект перестал думать)
	bool OnThink(void);
	//	тип объекта
	const char* Type(void) const {return "human";}
private:
	//	инициализировать сущность
	std::string InitEntity(const char* system_name,CREATE_MODE cm);
	unsigned int PackTypeToHumanPackType(PACK_TYPE pt);
};

//**********************************************************************
//	class ScriptSceneAPI::Trader
class ScriptSceneAPI::Trader : public ScriptScene::Object
{
private:
	BaseEntity * m_Entity;
public:
	Trader(const char* name);
	virtual ~Trader();
public:
	// мгновенно переместить человека в нужную позицию
	void SetCoords(const ipnt2_t& pt,const float angle);
	//	показать/спрятать персонаж
	void Show(bool show);
	//	включить/выключить изменение свойств персонажа
	void LockTraits(bool lock);
	//	получить трехмерные координаты человека
	point3 GetCoords3(void);
	//	получить двумерные (хексовые) координаты человека
	ipnt2_t GetCoords2(void);
	//	уничтожить существо
	void Destroy(void);
public:
	//	объект думает (false - объект перестал думать)
	bool OnThink(void);
	//	тип объекта
	const char* Type(void) const {return "trader";}
};

//**********************************************************************
//	class ScriptSceneAPI::Camera
class ScriptSceneAPI::Camera : public ScriptScene::Object
{
public:
	Camera();
	virtual ~Camera();
public:
	// запустить камеру по сплайну
	void MoveBySpline(const std::string& spline_name);
	// сфокусировать камеру на определенную точку
	void FocusOn(const point3& pt);
	// сфокусировать камеру на определенную точку
	void FocusOn(const point3& pt, const float time);
public:
	//	объект думает (false - объект перестал думать)
	bool OnThink(void);
	//	тип объекта
	const char* Type(void) const {return "camera";}
};

//**********************************************************************
//	class ScriptSceneAPI::Timer
class ScriptSceneAPI::Timer : public ScriptScene::Object
{
private:
	float m_Seconds;					//	текущее время
	float m_SecondsStart;				//	скока секунд надо
	float m_SecondsLeft;				//	скока осталось
public:
	Timer(const char* name);
	virtual ~Timer();
public:
	//	установить время в секундах	
	void SetTime(float seconds);
	//	запустить таймер
	void Start(void);
public:
	//	объект думает (false - объект перестал думать)
	bool OnThink(void);
	//	тип объекта
	const char* Type(void) const {return "timer";}
};

//**********************************************************************
//	class ScriptSceneAPI::Animation
class ScriptSceneAPI::Animation : public ScriptScene::Object
{
private:
	static const char* m_AnimaPath;		//	путь до анимации
	static const char* m_SkinPath;		//	путь до кожи
private:
	SkAnim* m_Animation;			//	уничтожать не нужно
	SkSkin* m_Skin;					//	уничтожать после использования
	AnimaData* m_AnimaData;			//	данные, необходимые для анимации
public:
	Animation(const char* name);
	virtual ~Animation();
public:
	void Start(void);
public:
	//	объект думает (false - объект перестал думать)
	bool OnThink(void);
	//	тип объекта
	const char* Type(void) const {return "animation";}
};

//**********************************************************************
//	class ScriptSceneAPI::Object
class ScriptSceneAPI::Object : public ScriptScene::Object
{
public:
	Object(const char* name);
	virtual ~Object();
public:
	//	переключить сотояние объекта (возвращает время за которое дверь откроется)
	float SwitchState(void);
	//	узнать состояние объекта
	bool GetState(void);
	//	уничтожить объект
	void ToErase(void);
	//	взорвать объект
	void ToBlast(float damage);
	//	получить координаты сентра объекта
	point3 GetCoords3(void);
public:
	//	объект думает (false - объект перестал думать)
	bool OnThink(void);
	//	тип объекта
	const char* Type(void) const {return "object";}
};

/////////////////////////////////////////////////////////////////////////
//////////////////////    class VideoScriptScene    /////////////////////
/////////////////////////////////////////////////////////////////////////
class VideoScriptScene : public ScriptScene
{
private:
	std::string m_Video;
public:
	VideoScriptScene(const char* video);
	virtual ~VideoScriptScene();
private:
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


#endif // !defined(__SCRIPT_SCENE_API_H__)
