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

  ����� ����, ��� �� ������ reactor ����� roby �������� ��������� ���������
  (� ������ ����������) � ������ ���������� �� ���, �� ������� ����,
  ����� ���� �������.

************************************************************************************/
class Scene1 : public ScriptScene
{
private:
	// ������ ��� ������ � �������
	ScriptSceneAPI::Camera* m_Camera;
	//	������ ��� ������ � ��������
	ScriptSceneAPI::Timer* m_Timer;
	// ������� ��� ������ � ������
	ScriptSceneAPI::Object* m_Door;
	//	�����
	point3 m_Point;
	enum STAGE {ST_ONE,ST_TWO,ST_THREE};
	STAGE m_Stage;
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


}

#endif	//_S_S_EPISODE_4_H_