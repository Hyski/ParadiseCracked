/***********************************************************************

                                   Alfa

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   14.11.2001

************************************************************************/
#ifndef _DD_MAIN_WND_H_
#define _DD_MAIN_WND_H_

//*********************************************************************//
//	class DDMainWnd
class DDMainWnd
{
public:
	class Controller;
	enum MODE {M_NONE,M_WINDOWED,M_FULLSCREEN};
public:
	static const int m_MaxWidth;
	static const int m_MaxHeight;
private:
	Controller* m_Controller;
	std::string m_Caption;
	unsigned int m_Icon;
	bool m_Minimized;
	bool m_Activated;
	HWND m_Handle;
	SIZE m_Client;
	SIZE m_Screen;
	MODE m_Mode;
public:
	DDMainWnd(const char* caption,unsigned int icon);
	virtual ~DDMainWnd();
public:
	//	создание окна
	bool Create(int width,int height,MODE mode);
	//	уничтожение окна
	void Destroy(void);
	//	показать окно
	void Show(void);
	//	изменить размеры окна
	void Resize(int width,int height);
	//	переместить окно в точку
	void MoveTo(int x,int y);
	//	обработка оконных сообщений без ожидания
	void Tick(void);
	//	обработка оконных сообщений с ожиданием
	void Wait(void);
	//	установить контроллер
	void SetController(Controller* controller);
	//	получить идентификатор окна
	HWND Handle(void) const;
	//	режим отображения окна
	MODE Mode(void) const;
	//	получить размера окна
	const SIZE& ClientSize(void) const; 
	const SIZE& ScreenSize(void) const; 
private:
	//	регистрирование класса окна
	void RegisterMainWndClass(void) const;
private:
	//	единственный экземпляр класса
	static DDMainWnd* m_Instance;
	//	обработка оконных сообщений
	friend LRESULT CALLBACK MainWndProc(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam);
	LRESULT ProcessMessage(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam);
	LRESULT	OnPaint(WPARAM wParam,LPARAM lParam);
	LRESULT	OnMove(WPARAM ,LPARAM lParam);
	LRESULT OnSysCommand(WPARAM wParam,LPARAM lParam);
	LRESULT OnSetCursor(WPARAM wParam,LPARAM lParam);
	LRESULT OnSize(WPARAM wParam,LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam,LPARAM lParam);
	LRESULT OnActivate(WPARAM wParam,LPARAM lParam);
};

//	получить идентификатор окна
inline HWND DDMainWnd::Handle(void) const
{
	return m_Handle;
}
//	получить размер клиентской области окна
inline const SIZE& DDMainWnd::ClientSize(void) const
{
	return m_Client;
}
//	получить полные размеры окна
inline const SIZE& DDMainWnd::ScreenSize(void) const
{
	return m_Screen;
}
//	режим отображения окна
inline DDMainWnd::MODE DDMainWnd::Mode(void) const
{
	return m_Mode;
}

//*********************************************************************//
//	class DDMainWnd::Controller
class DDMainWnd::Controller
{
public:
	virtual ~Controller() {}
	virtual void OnClose(void) = 0;
	virtual void OnPaint(void) = 0;
	virtual void OnMove(short x,short y) = 0;
	virtual void OnMouseMove(void) = 0;
	virtual void OnActivate(void) = 0;
	virtual void OnDeactivate(void) = 0;
	virtual void OnResize(int width,int height) = 0;
};

#endif	//_DD_MAIN_WND_H_