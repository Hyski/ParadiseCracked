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

���������� ��� ������ ������� ���� (new game).
Bartolomiu ���������� ���� �����, ����� ���� ��� ����� �������� back ����� ����� (������ �����) � ���� ������. ���� �� ����� �� ���� �������� �����������. ����� �� �������� �� ���� ������, �� ����� ���������� �������� china_town. 
������ �������� � �����.

������ - ����� ����� ��������� ��� ����� ��������� � bartolomiu. ����������� china_town.

************************************************************************************/
class Scene1 : public ScriptScene
{
private:
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	// ������ ��� ������ � �������
	ScriptSceneAPI::Human* m_Hacker;
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

����� ����� ������� �������� �� china_town (�� ���� ����� ����� ������ ���������� �����).
�� �������� ������� �������� ������ ������� ���� �� ������ � �������� � ��� � sam lee.
�� ���� ������ ���� �������� prostitute � soldier.
������ �������� � ��� � sam lee � ���������� ���, ��� ���� sam lee ����� ������ ���� ����-����.
� �������� ���� ������ �� ����� ��� ��������������� ��������.
������ �������� � �����
��������� - � ����� ������ �� ����������, �� �������������� �������� ��������� ��� ��������� sam lee.

************************************************************************************/
class Scene2 : public ScriptScene
{
private:
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	//	������ ��� ������ � ��������
	ScriptSceneAPI::Timer* m_Timer;
	// ������ ��� ������ � �������
	ScriptSceneAPI::Human* m_Hacker;
	// ������ ��� ������ � SamLee
	ScriptSceneAPI::Trader* m_SamLee;
private:
	bool m_FirstCameraAction;
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
	bool OnTick(void) {return true;}
};

/***********************************************************************************
> Scene3
����� ����� ������� � ������ china_town �� factory. 
����������� ���� ������� china_town_upgrade.
������������ ��� ��������� �� �������� �� ����� ����.
��������� ������� � ������� �� ��������, ����� � ��� ��� � ��� ������ ��������, ������� ��������, ����������� factory.
������ �������� � �����
��������� - ����������� ������� factory.

************************************************************************************/
class Scene3 : public ScriptScene
{
private:
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	// ������ ��� ������ � ���������
	ScriptSceneAPI::Animation* m_Jumper;
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
	bool OnTick(void) {return true;}
};

/***********************************************************************************
> Scene4
����� �������� ������� Loris �� ������ ���������� �� ���,
������������ �� ������ ����� ���� ������ �������� ��������
����������� � �� ����� �� ���������� ��� �� ������� 2 ������
��������� �� �������� �� ����� ���������. 
��� ������ ������ ������������ �� 2 �. 
������ ���������� �� ������ � ��� ������� �����. 
����� ������� ������ back ������� ����� �������������. 
������ ����������� ����������.
��������� - ����� ������ �� ������ ��� ������.

************************************************************************************/
class Scene4 : public ScriptScene
{
private:
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	//	������ ��� ������ � ��������
	ScriptSceneAPI::Timer* m_Timer;
	// ������ ��� ������ � �������
	ScriptSceneAPI::Human* m_Hacker;
	//	���������� ������ Loris
	point3 m_LorisDeath;
public:
	Scene4();
	virtual ~Scene4();
public:
	//	������ ���������� �����
	bool OnStart(void);
	//	��������� ���������� �����
	bool OnFinish(void) {return true;}
	//	���������� ���������� �����
	bool OnSkip(void) {return true;}
	//	��������� �������� ������� ���������� �����
	bool OnObjectNoActive(Object* object);
	//	�������� ���������� ���������� �����
	bool OnTick(void) {return true;}
};

/***********************************************************************************
> Scene5

����� �������� ������� �������� �� ������� china_town  � ���� 2 ��� 3,
�� ������������ ����� - ��-�� ���� ������ ������� ������ ����������,
����� ��� ��������� cyber_spider. ������ ��������� ������� �����,
����� ������� ������ back ����� ������������ ������ ����� ���������� �� ������.
������ �������� � �����.
������ - �������� �������� �� �����,
��� ��� ����� ����� ��� ���������� � ����� �������� ������������� � ����������� ����� ��������.

************************************************************************************/
class Scene5 : public ScriptScene
{
private:
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	// ������� ��� ������ � �����������
	ScriptSceneAPI::Human* m_Stormtroopers[3];
public:
	Scene5();
	virtual ~Scene5();
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
	bool OnTick(void) {return true;}
};

/***********************************************************************************
> Scene6

����� �������� ������� � troy (�� cityhall) �� ����������� �������� �����
(������� �� ����� ���� �������, � ������ �������� chill, ������ ��� ���� ���� �� ���,
chill ������ ��������� �� ���������� ��������� ���� � ���� � ����������� � ����������
 ��������� ���������. ����� ����� troy ������� �����. ���� � ������� ���� job �� ���
 ���� ������� �����.
������ ����������� ����������.
��������� - ��������� ��� �������� ��������� � ����� � �������������� ������� �����
�������� ��. ����� ����, �������� �������� �������� ����� � ������� ��� ��� chill,
chill ���������� �� ��������� ��������� ����� � ����.

************************************************************************************/
class Scene6 : public ScriptScene
{
private:
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	// ������ ��� ������ � ��������
	ScriptSceneAPI::Object* m_Door1;
	ScriptSceneAPI::Object* m_Door2;
	ScriptSceneAPI::Object* m_Door3;
	//	������ ��� ������ � ��������
	ScriptSceneAPI::Timer* m_Timer;
	// ������� ��� ������ � �����������
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
	void ChillToTroy(void);
};


/***********************************************************************************
> Scene7
************************************************************************************/
class Scene7 : public ScriptScene
{
private:
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	// ������ ��� ������ � ��������
	ScriptSceneAPI::Object* m_Door1;
	ScriptSceneAPI::Object* m_Door2;
	// ������� ��� ������ � �����������
	ScriptSceneAPI::Human* m_Moru;
	ScriptSceneAPI::Human* m_Tomas;
	ScriptSceneAPI::Human* m_Hacker;
	//	������ ��� ������ � ��������
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
ScriptScene* FinalEpisodeVideo(void);
/////////////////////////////////////////////////////////////////////////

}

#endif	//_SS_EPISODE_1_H_