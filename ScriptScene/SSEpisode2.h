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

����� � ����������
����� �������� �������� �� ������� Prison, ��� ���� � ���� ����
�������� (mini_truck), � �� ������ ��� ����� ������ (������� � ������� Door_Entr_01 �
Door_Entrance02), �� ���������� ��������� - �������� ���� � �����
����� ������ (���� � ������������ 32,47) � ������ ����� ���� ����
�����������, ����� ������ ����������, �� �������������� � �����
�������� ���� � ���� 32,40 � ��� ���������������, ����� �����
Door_Entrance02 �����������.


************************************************************************************/
class Scene1 : public ScriptScene
{
private:
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	// ������ ��� ������ � ��������
	ScriptSceneAPI::Vehicle* m_MiniTruck;
	// ������ ��� ������ � ��������
	ScriptSceneAPI::Object* m_Gate1;
	// ������ ��� ������ � ��������
	ScriptSceneAPI::Object* m_Gate2;
	//	������ ��� ������ � ��������
	ScriptSceneAPI::Timer* m_Timer;
	//	����-���� ��������� �� ������ �����
	enum MINI_TRACK_ACTION {MT_TO_PRIMARY_POINT,MT_TO_SECONDARY_POINT,MT_ON_SKIP};
	MINI_TRACK_ACTION m_mtAction;
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
�� ������ ������� ������� lift �� ���������� ������ 1_11_quest ���������� ��������:
Tom �������� �� ������, ������������ �� ���� (������ ����� � ���� ������ 4,21) � 
������ �������� � ������
� ���� � ��������� {����������} ���������� 6 soldier���, � ��� ����� soldier���
����� � player�� � ���������� ������ ����. (�� �������� ��������� ����� � ������� 3 
������ �� ����, ���� ����� ���, ������ ����������� �� 4, ���� ��� �� 5 � �.�.)

************************************************************************************/
class Scene2 : public ScriptScene
{
	enum {MAX_SOLDIER = 6};
	enum PHASE {P_ONE,P_SECOND};
private:
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	//	������ ��� ������ � ��������
	ScriptSceneAPI::Timer* m_Timer;
	// ������ ��� ������ � �������
	ScriptSceneAPI::Human* m_Hacker;
	// ������ ��� ������ � Tom
	ScriptSceneAPI::Trader* m_Tom;
	//	������ ��� ������ �� ���������
	ScriptSceneAPI::Human* m_Soldiers[MAX_SOLDIER];
	PHASE m_Phase;
	bool m_Skipped;
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
	bool IsAllSoldiersInactive(void);
	bool CheckAllSoldiersNoThink(void);
	void SpawnSoldier(void);
	void SetTraits(ScriptSceneAPI::Human* soldier);
};

/////////////////////////////////////////////////////////////////////////
///////////////////////////    class Scene3    //////////////////////////
/////////////////////////////////////////////////////////////////////////
//
//	�� ������ science ��� ������ RG ������ ����������� ����� Door_01.
//
/////////////////////////////////////////////////////////////////////////
class Scene3 : public ScriptScene
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
//	������� �� ��������� ������� �������
ScriptScene* FinalEpisodeVideo1(void);
ScriptScene* FinalEpisodeVideo2(void);
/////////////////////////////////////////////////////////////////////////

}

#endif	//_SS_EPISODE_2_H_