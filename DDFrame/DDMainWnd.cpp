/***********************************************************************

                                   Alfa

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   14.11.2001

************************************************************************/
#include "Precomp.h"
#include "DDMainWnd.h"

#ifdef _ENABLE_LOG
CLog mainwnd_log;
#define mainwnd	mainwnd_log["mainwnd.log"]
#else
#define mainwnd	/##/
#endif

LRESULT CALLBACK MainWndProc(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam);

DDMainWnd* DDMainWnd::m_Instance = 0;
const int DDMainWnd::m_MaxWidth = GetSystemMetrics(SM_CXFULLSCREEN);
const int DDMainWnd::m_MaxHeight = GetSystemMetrics(SM_CYFULLSCREEN);
DDMainWnd::DDMainWnd(const char* caption,unsigned int icon) : m_Caption(caption),m_Icon(icon),
					 m_Handle(0),m_Mode(M_NONE),m_Controller(0),m_Minimized(false),m_Activated(false)
{
	m_Instance = this;
	m_Client.cx = m_Client.cy = 0;
	m_Screen.cx = m_Screen.cy = 0;
}

DDMainWnd::~DDMainWnd()
{
	Destroy();
	m_Instance = 0;
}
//	установить контроллер
void DDMainWnd::SetController(Controller* controller)
{
	m_Controller = controller;
}
//	создание окна
bool DDMainWnd::Create(int width,int height,MODE mode)
{
	RECT rect;

	if(m_Handle) Destroy();
	RegisterMainWndClass();

	rect.right	= m_Screen.cx = width+(2*GetSystemMetrics(SM_CXSIZEFRAME));
	rect.bottom	= m_Screen.cy = height+(2*GetSystemMetrics(SM_CYSIZEFRAME))+GetSystemMetrics(SM_CYCAPTION);
	rect.left	= (m_MaxWidth-rect.right)/2;
	rect.top    = (m_MaxHeight-rect.bottom)/2;

	m_Handle = CreateWindowEx(0,m_Caption.c_str(),m_Caption.c_str(),
				(mode==M_WINDOWED)?WS_OVERLAPPEDWINDOW:WS_POPUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
				rect.left,rect.top,rect.right,rect.bottom,
				0,0,GetModuleHandle(0),0);
	
	m_Client.cx = width;
	m_Client.cy = height;
	m_Mode = mode;

	return m_Handle?true:false;
}
//	уничтожение окна
void DDMainWnd::Destroy(void)
{
	if(m_Handle)
	{
		//	снимаем контроллер
		m_Controller = 0;
		//	уничтожаем окно
		DestroyWindow(m_Handle);
		UnregisterClass(m_Caption.c_str(),GetModuleHandle(0));
		m_Handle = 0;
	}
}
//	показать окно
void DDMainWnd::Show(void)
{
	ShowWindow(m_Handle,SW_SHOW);
}
//	изменить размеры окна
void DDMainWnd::Resize(int width,int height)
{
	RECT rect;

	GetWindowRect(m_Handle,&rect);
	rect.right	= m_Screen.cx = width+(2*GetSystemMetrics(SM_CXSIZEFRAME));
	rect.bottom	= m_Screen.cy = height+(2*GetSystemMetrics(SM_CYSIZEFRAME))+GetSystemMetrics(SM_CYCAPTION);

	m_Client.cx = width;
	m_Client.cy = height;

	MoveWindow(m_Handle,rect.left,rect.top,rect.right,rect.bottom,true);
}
//	переместить окно в точку
void DDMainWnd::MoveTo(int x,int y)
{
	MoveWindow(m_Handle,x,y,m_Screen.cx,m_Screen.cy,true);
}
//	обработка оконных сообщений без ожидания
void DDMainWnd::Tick(void)
{
	MSG msg; 

	while(PeekMessage(&msg,0U,0U,0U,PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
//	обработка оконных сообщений с ожиданием
void DDMainWnd::Wait(void)
{
	MSG msg; 

	if(GetMessage(&msg,m_Handle,0U,0U))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
//	регистрирование класса окна
void DDMainWnd::RegisterMainWndClass(void) const
{
	WNDCLASSEX wndclass;

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_VREDRAW|CS_HREDRAW|CS_OWNDC;
	wndclass.lpfnWndProc = MainWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance  = GetModuleHandle(0);
	wndclass.hIcon = LoadIcon(wndclass.hInstance,MAKEINTRESOURCE(m_Icon));
	wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = m_Caption.c_str();
	wndclass.hIconSm = LoadIcon(wndclass.hInstance,MAKEINTRESOURCE(m_Icon));

	RegisterClassEx(&wndclass);
}
//	процедура обработки сообщений
LRESULT DDMainWnd::ProcessMessage(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam)
{
	switch(iMsg)
	{
	case WM_SETCURSOR:
		return OnSetCursor(wParam,lParam);
	case WM_ERASEBKGND:
		return 0;
	case WM_PAINT:
		return OnPaint(wParam,lParam);
	case WM_MOVE:
		return OnMove(wParam,lParam);
	case WM_SIZE:
		mainwnd("WM_SIZE:\n");
		return OnSize(wParam,lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam,lParam);
	case WM_ACTIVATE:
		mainwnd("WM_ACTIVATE:\n");
		return OnActivate(wParam,lParam);
	case WM_SYSCOMMAND:
		return OnSysCommand(wParam,lParam);
	} 

	return DefWindowProc(hWnd,iMsg,wParam,lParam);
}

LRESULT	DDMainWnd::OnPaint(WPARAM wParam,LPARAM lParam)
{
	if(m_Controller) m_Controller->OnPaint();

	return DefWindowProc(m_Handle,WM_PAINT,wParam,lParam);
}

LRESULT	DDMainWnd::OnMove(WPARAM ,LPARAM lParam)
{
	if(m_Controller) m_Controller->OnMove(LOWORD(lParam),HIWORD(lParam));

	return 0;
}

LRESULT DDMainWnd::OnSysCommand(WPARAM wParam,LPARAM lParam)
{
	switch(wParam)
	{
	case SC_MONITORPOWER:		//	недопущение отключения монитора в режим StandBy
		break;
	case SC_SCREENSAVE:			//	недопущение включения ScreenSaver'а
		break;
	case SC_CLOSE:				//	закрытие приложения
		PostQuitMessage(0);
		if(m_Controller) m_Controller->OnClose();
		break;
	default:
		DefWindowProc(m_Handle,WM_SYSCOMMAND,wParam,lParam);
	}

	return 0;
}

LRESULT DDMainWnd::OnSetCursor(WPARAM wParam,LPARAM lParam)
{
	if(LOWORD(lParam) == HTCLIENT && m_Activated)
	{
		SetCursor(0);
		return 0;
	}

	return DefWindowProc(m_Handle,WM_SETCURSOR,wParam,lParam);
}

LRESULT DDMainWnd::OnSize(WPARAM wParam,LPARAM lParam)
{
	switch(wParam)
	{
	case SIZE_MINIMIZED:
		mainwnd("   ...minimized;\n");
		m_Minimized = true;
		//	в полноэкранном режиме не приходит сообщение WM_ACTIVATE, если не отпуская Alt при Alt-Tab'е выбрать снова текущее приложение
		if(m_Mode == M_FULLSCREEN) OnActivate(MAKEWPARAM(WA_INACTIVE,1),0);
		break;
	case SIZE_MAXIMIZED:
		mainwnd("   ...maximazied;\n");
		if(m_Controller) m_Controller->OnResize(LOWORD(lParam),HIWORD(lParam));
		break;
	case SIZE_RESTORED:
		mainwnd("   ...restored;\n");
		if(m_Minimized)	m_Minimized = false;
		else
		{
			if(m_Controller) m_Controller->OnResize(LOWORD(lParam),HIWORD(lParam));
		}
		break;
	}

	return 0;
}

LRESULT DDMainWnd::OnMouseMove(WPARAM wParam,LPARAM lParam)
{
	if(m_Controller) m_Controller->OnMouseMove();

	return 0;
}

LRESULT DDMainWnd::OnActivate(WPARAM wParam,LPARAM lParam)
{
	if(LOWORD(wParam)!=WA_INACTIVE)
	{
		mainwnd("   ...activated;\n");
		if(!m_Activated)
		{
			SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
			if(m_Controller) m_Controller->OnActivate();
			m_Activated = true;
		}
	}
	else
	{
		mainwnd("   ...deactivated;\n");
		if(m_Activated)
		{
			if(m_Controller) m_Controller->OnDeactivate();
			SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
			m_Activated = false;
		}
	}

	return 0;
}

//	оконная процедура обработки сообщений
LRESULT CALLBACK MainWndProc(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam)
{
	return DDMainWnd::m_Instance->ProcessMessage(hWnd,iMsg,wParam,lParam);
}
