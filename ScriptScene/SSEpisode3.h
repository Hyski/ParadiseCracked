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

  ����� ���� ��� �� ������ cybercenter ����� Gem ������� ������ ����� � 
  �� �� ������ ����������� ��� ����� (���������� ����� ��� ���� ������ 
  �������� ���� �� ���� �� ���)
	door_1,door_2,...,door_9
	door_entr_01
	door_entr_02
	door_entr_03
	door_entr_04
	������������ �� door_2

************************************************************************************/
class Scene1 : public ScriptScene
{
private:
	enum {MAX_DOOR = 9,MAX_ENTR = 4};
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	// ������� ��� ������ � �������
	ScriptSceneAPI::Object* m_Doors[MAX_DOOR];
	// ������� ��� ������ � �������
	ScriptSceneAPI::Object* m_Entrs[MAX_ENTR];
	//	������ ��� ������ � ��������
	ScriptSceneAPI::Timer* m_Timer;
public:
	Scene1();
	virtual ~Scene1();
public:
	//	������ ���������� �����
	bool OnStart(void);
	//	��������� ���������� �����
	bool OnFinish(void);
	//	���������� ���������� �����
	bool OnSkip(void);
	//	��������� �������� ������� ���������� �����
	bool OnObjectNoActive(Object* object);
	//	�������� ���������� ���������� �����
	bool OnTick(void);
};

/***********************************************************************************
> Scene2

  ����� ���� ��� �������� ��������� � HELEN �� ������ RADAR,
  �� ������� ������� ����� � ��� specops.

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
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	//	������ ��� ������ � ��������
	ScriptSceneAPI::Timer* m_Timer;
	//	������ ��� ������ �� ����������
	ScriptSceneAPI::Human* m_SpecCops[MAX_SPECCOP];
public:
	Scene2();
	virtual ~Scene2();
public:
	//	������ ���������� �����
	bool OnStart(void);
	//	��������� ���������� �����
	bool OnFinish(void);
	//	���������� ���������� �����
	bool OnSkip(void);
	//	��������� �������� ������� ���������� �����
	bool OnObjectNoActive(Object* object);
	//	�������� ���������� ���������� �����
	bool OnTick(void);
private:
	bool CheckAllSpecCopsStoped(void);
	bool CheckAllSpecCopsNoThink(void);

};

/***********************************************************************************
> Scene3

1) � ������ ������ ������� ��������� (Mortimer), ������ ������ �������������� �� ������
� ��������� ������ Gem � �� ������ ����������.
2) ���� � ������� ���� ���� � ����������� ����� quest_03_09, �� ���� ��������� �
����� ��� ����� ������ �������� (Mortimer) � ������� �����, ����� ������� �����,
����� ����� ����. ���� � ������� ���� ����, �� ����� ���� �� ���� ������� ����� �
���� ��� ��������.

***********************************************************************************/
class Scene3 : public ScriptScene
{
private:
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	//	������ ��� ������ � ��������
	ScriptSceneAPI::Timer* m_Timer;
	// ������ ��� ������ � �������
	ScriptSceneAPI::Human* m_Hacker;
	// ������ ��� ������ � ����
	ScriptSceneAPI::Human* m_Tony;
	// ������ ��� ������ � ������
	ScriptSceneAPI::Human* m_Job;
	// ������ ��� ������ � ��������
	ScriptSceneAPI::Vehicle* m_Gem;
	//	���������� ������ Mortimer
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
	//	������ ���������� �����
	bool OnStart(void);
	//	��������� ���������� �����
	bool OnFinish(void);
	//	���������� ���������� �����
	bool OnSkip(void);
	//	��������� �������� ������� ���������� �����
	bool OnObjectNoActive(Object* object);
	//	�������� ���������� ���������� �����
	bool OnTick(void);
};

/////////////////////////////////////////////////////////////////////////
///////////////////////////    class Scene4    //////////////////////////
/////////////////////////////////////////////////////////////////////////
//
//	�� ������ spaceport, ������ 3. ��� ������ gann ������ ����������� 
//	����� FField_01.
//
/////////////////////////////////////////////////////////////////////////
class Scene4 : public ScriptScene
{
private:
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	// ������ ��� ������ � ������
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
	//	������ ���������� �����
	bool OnStart(void);
	//	��������� ���������� �����
	bool OnFinish(void);
	//	���������� ���������� �����
	bool OnSkip(void);
	//	��������� �������� ������� ���������� �����
	bool OnObjectNoActive(Object* object);
	//	�������� ���������� ���������� �����
	bool OnTick(void);
};

/////////////////////////////////////////////////////////////////////////
//	������� �� ��������� �������� �������
ScriptScene* FinalEpisodeVideo(void);
/////////////////////////////////////////////////////////////////////////


}

#endif	//_S_S_EPISODE_3_H_