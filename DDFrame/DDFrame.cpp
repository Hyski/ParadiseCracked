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
#include "../Common/D3DApp/D3DInfo/D3DInfo.h"
#include "DDFrame.h"

#ifdef _ENABLE_LOG
CLog ddframe_log;
#define ddframe	ddframe_log["ddframe.log"]
#else
#define ddframe	/##/
#endif

namespace DDFrameUtils
{
	DDMainWnd::MODE ToMainWndMode(DDFrame::VideoMode::MODE mode)
	{
		switch(mode)
		{
		case DDFrame::VideoMode::M_WINDOWED:
			return DDMainWnd::M_WINDOWED;
		case DDFrame::VideoMode::M_FULLSCREEN:
			return DDMainWnd::M_FULLSCREEN;
		default:
			return DDMainWnd::M_NONE;
		}
	}
	bool ToD3DKernelMode(DDFrame::VideoMode::MODE mode)
	{
		switch(mode)
		{
		case DDFrame::VideoMode::M_WINDOWED:
			return true;
		case DDFrame::VideoMode::M_FULLSCREEN:
			return false;
		default:
			return true;
		}
	}
}

using namespace DDFrameUtils;

//*********************************************************************//
//	class DDFrame::MWController
class DDFrame::MWController : public DDMainWnd::Controller
{
private:
	DDFrame* m_Frame;
public:
	MWController(DDFrame* frame) : m_Frame(frame) {}
	void OnClose(void) {m_Frame->OnClose();}
private:
	void OnPaint(void) {m_Frame->OnPaint();}
	void OnMove(short x,short y) {m_Frame->OnMove(x,y);}
	void OnMouseMove(void) {m_Frame->OnMouseMove();}
	void OnActivate(void) {m_Frame->OnActivate();}
	void OnDeactivate(void) {m_Frame->OnDeactivate();}
	void OnResize(int width,int height) {m_Frame->OnResize(width,height);}
};

//*********************************************************************//
//	class DDFrame
DDFrame::DDFrame(const char* app_name,unsigned int app_icon) : m_MainWnd(new DDMainWnd(app_name,app_icon)),
				 m_Controller(0)
{
	//	создание конироллера главного окна
	m_MWController = new MWController(this);
	//	составление списка доступных видеорежимов
	D3DInfo::Refresh();
	ddframe("DDFrame:\n...created;\n");
}

DDFrame::~DDFrame()
{
	D3DKernel::Release();
	ddframe("   D3DKernel released;\n");
	delete m_MainWnd;
	ddframe("   DDMainWnd destroyed;\n");
	delete m_MWController;
	ddframe("...destroyed;\n");
	if(Input::IsInit()) Input::Close();
}
//	установить контроллер
DDFrame::Controller* DDFrame::SetController(Controller* controller)
{
	Controller* pc = m_Controller;
	m_Controller = controller;
	return pc;
}
//	установить видео режим
void DDFrame::SetVideoMode(const VideoMode& vm,unsigned int flags)
{
	if(vm.IsValid())
	{
		if(!(vm == m_VideoMode))
		{
			//	отключение контроллера окна
			m_MainWnd->SetController(0);
			//	уведомляем контроллер о предстоящей смене видеорежима
			if(m_Controller) m_Controller->OnStartChangeVideoMode();
			//	отключение клавиатурного и мышинного ввода
			if(Input::IsInit()) Input::Close();
			//	проверка на существование окна
			if(!m_MainWnd->Handle() || (ToMainWndMode(vm.m_Mode) != m_MainWnd->Mode()))
				m_MainWnd->Create(vm.m_Width,vm.m_Height,(vm.m_Mode==VideoMode::M_WINDOWED)?DDMainWnd::M_WINDOWED:DDMainWnd::M_FULLSCREEN);
			//	проверка размеров окна
			if((m_MainWnd->ClientSize().cx!=vm.m_Width) || (m_MainWnd->ClientSize().cy!=vm.m_Height))
				m_MainWnd->Resize(vm.m_Width,vm.m_Height);
			//	выравниваем окно по центру экрана, если режим оконный
			if(vm.m_Mode == VideoMode::M_WINDOWED) 
			{
				int x = (DDMainWnd::m_MaxWidth-m_MainWnd->ScreenSize().cx)/2;
				int y = (DDMainWnd::m_MaxHeight-m_MainWnd->ScreenSize().cy)/2;

				if(!((x<0) || (y<0))) m_MainWnd->MoveTo(x,y);
			}
			//	сохраняем текущую конфигурацию
			m_VideoMode = vm;
			//	проверяем на границы диапозона
			D3DInfo::CheckIndex(&m_VideoMode.m_Driver,&m_VideoMode.m_Device);
			//	пробуем установить заданный видео режим
			if(D3DKernel::Init(m_MainWnd->Handle(),
							   D3DInfo::GetDriverGuidPtr(m_VideoMode.m_Driver),
							   D3DInfo::GetDeviceGuidPtr(m_VideoMode.m_Driver,m_VideoMode.m_Device),
							   m_VideoMode.m_Width,
							   m_VideoMode.m_Height,
							   m_VideoMode.m_Depth,
							   m_VideoMode.m_Freq,
							   ToD3DKernelMode(m_VideoMode.m_Mode)))
			{
				//	инициализация поддержки ввода
				Input::Init(m_MainWnd->Handle());
				Input::Mouse()->SetBorder(-1,-1,m_VideoMode.m_Width,m_VideoMode.m_Height);
				if(D3DKernel::IsFullScreen()) Input::Mouse()->SetPos(D3DKernel::ResX()>>1,D3DKernel::ResY()>>1);
				else
				{
					POINT pt;
					
					GetCursorPos(&pt);
					ScreenToClient(m_MainWnd->Handle(),&pt);
					Input::Mouse()->SetPos(pt.x,pt.y);
				}
				//	уведомляем контроллер о смене видеорежима
				if(m_Controller) m_Controller->OnFinishChangeVideoMode();
			}
			else throw CASUS("Не удалось проинициализировать видеорежим.");
		}
	}
	else
	{
		D3DKernel::Release();
		m_VideoMode = VideoMode();
	}
}
//	создать окно (написано для применения к бинкам: что-то там со звуком не так)
void DDFrame::CreateHandle(VideoMode::MODE mode)
{
	if(!m_MainWnd->Handle()) m_MainWnd->Create(0,0,(mode==VideoMode::M_WINDOWED)?DDMainWnd::M_WINDOWED:DDMainWnd::M_FULLSCREEN);
}
//	обработка оконных сообщений без ожидания
void DDFrame::Tick(void)
{
	m_MainWnd->Tick();
}
//	обработка оконных сообщений с ожиданием
void DDFrame::Wait(void)
{
	m_MainWnd->Wait();
}
//	отображаем окно
void DDFrame::ShowWindow(void)
{
	//	отключаем контроллер главного окна
	m_MainWnd->SetController(0);
	//	показываем окно
	m_MainWnd->Show();
	//	пропускаем накопившееся сообщения
	m_MainWnd->Tick();
	//	инициализируем устройства ввода
	if(D3DKernel::IsFullScreen()) Input::Mouse()->Lock();
	Input::Mouse()->Run();
	Input::Keyboard()->Lock();
	Input::Keyboard()->Run();
	//	устанавливаем контроллер главного окна
	m_MainWnd->SetController(m_MWController);
}
//	получить идентификатор окна
HWND DDFrame::Handle(void) const
{
	return m_MainWnd->Handle();
}
//	проверка положения мыши
void DDFrame::CheckMouse(void)
{
	if(!D3DKernel::IsFullScreen())
	{
		if(!IsMouseWithin() && Input::Mouse()->IsLocked())
		{
			POINT pt = {0,0};

			Input::Mouse()->Unlock();
			ClientToScreen(m_MainWnd->Handle(),&pt);
			SetCursorPos(pt.x+Input::MouseState().x,
				pt.y+Input::MouseState().y);
		}
	}
}
//	мышь в пределах окна?
bool DDFrame::IsMouseWithin(void) const
{
	return !((Input::MouseState().x == -1) ||
		     (Input::MouseState().y == -1) ||
			 (Input::MouseState().x == D3DKernel::ResX()) ||
			 (Input::MouseState().y == D3DKernel::ResY()));
}
//	сообщение от окна [закрытие окна]
void DDFrame::OnClose(void)
{
	if(m_Controller) m_Controller->OnClose();
}
//	сообщение от окна [отрисовка области окна]
void DDFrame::OnPaint(void)
{
	D3DKernel::UpdateFrame();
}
//	сообщение от окна [перемещение окна]
void DDFrame::OnMove(short x,short y)
{
	D3DKernel::MoveViewPort(x,y);
}

void DDFrame::OnMouseMove(void)
{
	POINT pt;

	ddframe("   mouse moved;\n");

	if(D3DKernel::IsFullScreen())
	{
		if(!Input::Mouse()->IsLocked())
		{
			Input::Mouse()->Lock();
			Input::Keyboard()->Lock();
			ddframe("   mouse locked;\n");
		}
	}
	else
	{
		if(!Input::Mouse()->IsLocked())
		{//	захват мыши и клавиатуры
			GetCursorPos(&pt);
			ScreenToClient(m_MainWnd->Handle(),&pt);
			Input::Mouse()->SetPos(pt.x,pt.y);
			Input::Mouse()->Lock();
			Input::Keyboard()->Lock();
			Input::Update();
		}
	}
}

void DDFrame::OnActivate(void)
{
	//	подключаем устройства ввода
	if(D3DKernel::IsFullScreen()) Input::Mouse()->Lock();
	Input::Keyboard()->Lock();
	//	передаем уведомление
	if(m_Controller) m_Controller->OnActivate();
}

void DDFrame::OnDeactivate(void)
{
	//	подправляем позицию виндового курсора
	if(!D3DKernel::IsFullScreen() && Input::Mouse()->IsLocked())
	{
		POINT pt = {0,0};
		
		ClientToScreen(m_MainWnd->Handle(),&pt);
		SetCursorPos(pt.x+Input::MouseState().x,
					 pt.y+Input::MouseState().y);
	}
	//	отключаем устройства ввода
	Input::Mouse()->Unlock();
	Input::Keyboard()->Unlock();
	//	передаем уведомление
	if(m_Controller) m_Controller->OnDeactivate();
}

void DDFrame::OnResize(int width,int height)
{
	if(m_VideoMode.m_Mode == VideoMode::M_WINDOWED)
	{
		SetVideoMode(VideoMode(m_VideoMode.m_Driver,m_VideoMode.m_Device,
					           width,height,m_VideoMode.m_Depth,
							   m_VideoMode.m_Freq,m_VideoMode.m_Mode));
		ShowWindow();
	}
}
