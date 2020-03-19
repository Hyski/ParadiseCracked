/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#ifndef _SHELL_H_
#define _SHELL_H_

#include "../../DDFrame/DDFrame.h"

class GraphPipe;

/////////////////////////////////////////////////////////////////////////
///////////////////////////    class Shell    ///////////////////////////
/////////////////////////////////////////////////////////////////////////
class Shell : public DDFrame::Controller
{
	struct Container;
	class FPSometer;
public:
	static const char* m_AppName;
private:
	//	������������ ��������� ������
	static Shell* m_Instance;
	//	��������� ��� ������
	std::auto_ptr<Container> m_Container;
	//	������ ��� ������������ ���������� �������
	HANDLE m_Mutex;
	//	�������� ���� ����������
	typedef void (Shell::*Loop)(void);
	Loop m_Loop;
	//	���� ������ �� ����������
	bool m_Exit;
	//	���� �������� � ��������� �����
	bool m_IsModalMode;
private:
	Shell();
	virtual ~Shell();
public:
	//	�������� ������ ����������
	DDFrame* Frame(void);
	//	�������� ��������� �� GraphPipe
	GraphPipe* GetGraphPipe(void) const;
	//	�������� ���-�� ������ � �������
	float FPS(void) const;
	//	����� � ��������� ����
	void DoModal(void);
	//	���������: �������� �� ��������� �����
	bool IsModal(void) const;
	//	��������� ������ ����������
	void Exit(void);
private:
	//	������!
	void Go(void);
	//	������������ ���������� �������
	bool CheckExistInstance(void);
	//	���� ����������
	void PerformanceLoop(void);
	//	���� ��������
	void IdleLoop(void);
private:
	//	��������� �������
	void OnClose(void);
	void OnActivate(void);
	void OnDeactivate(void);
	//	������������� ��������� ��������
	void InitDataMgr(void);
public:
	void OnStartChangeVideoMode(void);
	void OnFinishChangeVideoMode(void);
public:
	static Shell* Instance(void);
	friend int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow);
};

inline Shell* Shell::Instance(void)
{
	return m_Instance;
}
//	��������� ������ ����������
inline void Shell::Exit(void)
{
	m_Exit = true;
}

//	������� ��������� �� ������ ����������
inline DDFrame* Frame(void) {return Shell::Instance()->Frame();}

#endif	//_SHELL_H_	