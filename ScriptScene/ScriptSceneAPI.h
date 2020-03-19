/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ��������� ��� ���������� ���� �� ������� ��������� �������
				
                                                                                
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
	//	**** ���������� ���������� ������
	void StartScene(void);
	void FinishScene(void);
	//	�������� ������� ����� ������ ���������
	float GetTime(void) const;
	//	����� �� ������ �������
	void ExitFromLevel(const ipnt2_t& pt);
	//	�������� ����� �� xls'��
	std::string GetSSPhrase(const char* phrase_name);
	//	��������/��������� �����
	void SetRoofMode(bool visible);
	//	��������� �������
	void PlayBink(const char* file_name);
	//	���������, ���� �� ������� �� ������
	bool IsHumanExist(const char* name);
	//	��������� ������������� ������������� ������
	bool IsQuestExecute(const char* name);
	//	��������� �������� ���������� ������������� ������
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
	//	���������� �� ����� ������
	static bool IsExist(const char* name);
	//	����� � �����
	void MoveTo(const ipnt2_t& pnt);
/*	// ���� � �����
	void WalkTo(const ipnt2_t& pnt);
	void WalkTo(ScriptSceneAPI::Object* object);
	void WalkTo(ScriptSceneAPI::Human* human);
	// ������ � �����
	void RunTo(const ipnt2_t& pnt);
	void RunTo(ScriptSceneAPI::Object* object);
	void RunTo(ScriptSceneAPI::Human* human);
	//	���������� ����������� �����
	bool IsAccess(ScriptSceneAPI::Object* object);
	bool IsAccess(ScriptSceneAPI::Human* human);
	// ��������� �������� �����������
	void RotateToPoint(const point3& pnt);*/
	//	������� �����
	void Talk(const std::string& phrase);
	//	���������� �������
	void Stop(void);
	// ��������� ����������� �������� � ������ �������
	void SetCoords(const ipnt2_t& pt,const float angle);
	//	��������/�������� ��������
	void Show(bool show);
	//	�������� �������
	void ToBlast(void);
/*	//	����� ��������
	enum SHOT_TYPE {ST_AIMSHOT,ST_SNAPSHOT,ST_AUTOSHOT};
	// ���������� � �����
	void Shoot(const point3& target, const float accuracy, const SHOT_TYPE st);*/
	//	��������/��������� ��������� ������� ���������
	void LockTraits(bool lock);
	//	�������� ���������� ���������� ��������
	point3 GetCoords3(void);
	//	�������� ��������� (��������) ���������� ��������
	ipnt2_t GetCoords2(void);
	//	������� ��������� �� ��������
	BaseEntity* Entity(void) {return m_Entity;}
public:
	//	������ ������ (false - ������ �������� ������)
	bool OnThink(void);
	//	��� �������
	const char* Type(void) const {return "vehicle";}
private:
	//	������������� ���
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
	// ���� � �����
	void WalkTo(const ipnt2_t& pnt);
	void WalkTo(ScriptSceneAPI::Object* object);
	void WalkTo(ScriptSceneAPI::Human* human);
	// ������ � �����
	void RunTo(const ipnt2_t& pnt);
	void RunTo(ScriptSceneAPI::Object* object);
	void RunTo(ScriptSceneAPI::Human* human);
	//	���������� ����������� �����
	bool IsAccess(ScriptSceneAPI::Object* object);
	bool IsAccess(ScriptSceneAPI::Human* human);
	//	���������� �������� �� ��� ����� �� �����-���� ��������
	bool IsBusy(const ipnt2_t& pnt);
	// ��������� �������� �����������
	void RotateToPoint(const point3& pnt);
	//	������� �����
	void Talk(const std::string& phrase);
	//	���������� ���������
	void Stop(void);
	// ��������� ����������� �������� � ������ �������
	void SetCoords(const ipnt2_t& pt,const float angle);
	//	��������/�������� ��������
	void Show(bool show);
	//	����� ��������
	enum SHOT_TYPE {ST_AIMSHOT,ST_SNAPSHOT,ST_AUTOSHOT};
	// ���������� � �����
	void Shoot(const point3& target, const float accuracy, const SHOT_TYPE st);
	//	��������/��������� ���������
	void SetSitMode(bool sit);
	//	���������� ������ ���������
	void SetBehaviourModel(const char* label);
	//	���� �������� �������
	enum PACK_TYPE {PK_HEAD,PK_BODY,PK_HANDS,PK_LKNEE,PK_RKNEE,PK_IMPLANTS,PK_BACKPACK};
	bool GiveWeapon(const char* weapon_name,const char* ammo_name,int ammo_count,PACK_TYPE pt);
	bool GiveArmor(const char* armor_name,PACK_TYPE pt);
	bool GiveAmmo(const char* ammo_name,int ammo_count,PACK_TYPE pt);
	bool GiveGrenade(const char* grenade_name,PACK_TYPE pt);
	bool GiveImplant(const char* implant_name,PACK_TYPE pt);
	//	���������� ������� ��� ��������
	enum TEAM_TYPE {TT_NONE,TT_PLAYER,TT_ENEMY};
	void SetTeam(TEAM_TYPE tt);
	//	�������� ������� ��� ��������
	void ChangeTeam(TEAM_TYPE tt);
	//	��������/��������� ��������� ������� ���������
	void LockTraits(bool lock);
	//	�������� ���������� ���������� ��������
	point3 GetCoords3(void);
	//	�������� ��������� (��������) ���������� ��������
	ipnt2_t GetCoords2(void);
	//	������� ��������� �� ��������
	BaseEntity* Entity(void) {return m_Entity;}
	//	���������� ��������
	void Destroy(void);
	//	�������� ��� ���� �� �����
	void ItemsToGround(const ipnt2_t& pt);
public:
	//	������ ������ (false - ������ �������� ������)
	bool OnThink(void);
	//	��� �������
	const char* Type(void) const {return "human";}
private:
	//	���������������� ��������
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
	// ��������� ����������� �������� � ������ �������
	void SetCoords(const ipnt2_t& pt,const float angle);
	//	��������/�������� ��������
	void Show(bool show);
	//	��������/��������� ��������� ������� ���������
	void LockTraits(bool lock);
	//	�������� ���������� ���������� ��������
	point3 GetCoords3(void);
	//	�������� ��������� (��������) ���������� ��������
	ipnt2_t GetCoords2(void);
	//	���������� ��������
	void Destroy(void);
public:
	//	������ ������ (false - ������ �������� ������)
	bool OnThink(void);
	//	��� �������
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
	// ��������� ������ �� �������
	void MoveBySpline(const std::string& spline_name);
	// ������������� ������ �� ������������ �����
	void FocusOn(const point3& pt);
	// ������������� ������ �� ������������ �����
	void FocusOn(const point3& pt, const float time);
public:
	//	������ ������ (false - ������ �������� ������)
	bool OnThink(void);
	//	��� �������
	const char* Type(void) const {return "camera";}
};

//**********************************************************************
//	class ScriptSceneAPI::Timer
class ScriptSceneAPI::Timer : public ScriptScene::Object
{
private:
	float m_Seconds;					//	������� �����
	float m_SecondsStart;				//	����� ������ ����
	float m_SecondsLeft;				//	����� ��������
public:
	Timer(const char* name);
	virtual ~Timer();
public:
	//	���������� ����� � ��������	
	void SetTime(float seconds);
	//	��������� ������
	void Start(void);
public:
	//	������ ������ (false - ������ �������� ������)
	bool OnThink(void);
	//	��� �������
	const char* Type(void) const {return "timer";}
};

//**********************************************************************
//	class ScriptSceneAPI::Animation
class ScriptSceneAPI::Animation : public ScriptScene::Object
{
private:
	static const char* m_AnimaPath;		//	���� �� ��������
	static const char* m_SkinPath;		//	���� �� ����
private:
	SkAnim* m_Animation;			//	���������� �� �����
	SkSkin* m_Skin;					//	���������� ����� �������������
	AnimaData* m_AnimaData;			//	������, ����������� ��� ��������
public:
	Animation(const char* name);
	virtual ~Animation();
public:
	void Start(void);
public:
	//	������ ������ (false - ������ �������� ������)
	bool OnThink(void);
	//	��� �������
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
	//	����������� �������� ������� (���������� ����� �� ������� ����� ���������)
	float SwitchState(void);
	//	������ ��������� �������
	bool GetState(void);
	//	���������� ������
	void ToErase(void);
	//	�������� ������
	void ToBlast(float damage);
	//	�������� ���������� ������ �������
	point3 GetCoords3(void);
public:
	//	������ ������ (false - ������ �������� ������)
	bool OnThink(void);
	//	��� �������
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
	//	������ ���������� �����
	bool OnStart(void);
	//	��������� ���������� �����
	bool OnFinish(void);
	//	���������� ���������� �����
	bool OnSkip(void);
	//	��������� �������� ������� ���������� �����
	bool OnObjectNoActive(Object* object);
	//	�������� ���������� ���������� �����
	bool OnTick(void) {return true;}
};


#endif // !defined(__SCRIPT_SCENE_API_H__)
