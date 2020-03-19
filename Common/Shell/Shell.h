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
	//	единственный экземпляр класса
	static Shell* m_Instance;
	//	контейнер для данных
	std::auto_ptr<Container> m_Container;
	//	мьютех для отслеживание повторного запуска
	HANDLE m_Mutex;
	//	основной цикл приложения
	typedef void (Shell::*Loop)(void);
	Loop m_Loop;
	//	флаг выхода из приложения
	bool m_Exit;
	//	флаг вхождния в модальный режим
	bool m_IsModalMode;
private:
	Shell();
	virtual ~Shell();
public:
	//	получить каркас приложения
	DDFrame* Frame(void);
	//	получить указатель на GraphPipe
	GraphPipe* GetGraphPipe(void) const;
	//	получить кол-во кадров в секунду
	float FPS(void) const;
	//	войти в модальный цикл
	void DoModal(void);
	//	проверить: свободен ли модальный режим
	bool IsModal(void) const;
	//	завершить работу приложения
	void Exit(void);
private:
	//	вперед!
	void Go(void);
	//	отслеживание повторного запуска
	bool CheckExistInstance(void);
	//	цикл исполнения
	void PerformanceLoop(void);
	//	цикл ожидания
	void IdleLoop(void);
private:
	//	обработка событий
	void OnClose(void);
	void OnActivate(void);
	void OnDeactivate(void);
	//	инициализация менеджера ресурсов
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
//	завершить работу приложения
inline void Shell::Exit(void)
{
	m_Exit = true;
}

//	вернуть указатель на каркас приложения
inline DDFrame* Frame(void) {return Shell::Instance()->Frame();}

#endif	//_SHELL_H_	