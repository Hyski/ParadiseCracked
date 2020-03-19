/***********************************************************************

                                   Alfa

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   14.11.2001

************************************************************************/
#ifndef _DD_FRAME_H_
#define _DD_FRAME_H_

class DDMainWnd;

//*********************************************************************//
//	class DDFrame
class DDFrame
{
public:
	class Controller;
	//*********************************************************************//
	//	class DDFrame::VideoMode
	class VideoMode
	{
	public:
		enum MODE {M_UNINITIALIZED,M_WINDOWED,M_FULLSCREEN};
		unsigned int m_Driver;
		unsigned int m_Device;
		unsigned int m_Width;
		unsigned int m_Height;
		unsigned int m_Depth;
		unsigned int m_Freq;
		MODE m_Mode;
	public:
		VideoMode() : m_Driver(0),m_Device(0),m_Width(0),m_Height(0),m_Depth(0),m_Freq(0),m_Mode(M_UNINITIALIZED) {}
		VideoMode(unsigned int driver,unsigned int device,
			unsigned int width,unsigned int height,
			unsigned int depth,unsigned int freq,
			MODE mode) : m_Driver(driver),m_Device(device),
			m_Width(width),m_Height(height),
			m_Depth(depth),m_Freq(freq),
			m_Mode(mode) {}
		VideoMode(const VideoMode& vm) : m_Driver(vm.m_Driver),m_Device(vm.m_Device),
			m_Width(vm.m_Width),m_Height(vm.m_Height),
			m_Depth(vm.m_Depth),m_Freq(vm.m_Freq),
			m_Mode(vm.m_Mode) {}
		virtual ~VideoMode() {}
	public:
		const VideoMode& operator=(const VideoMode& vm)
		{
			m_Driver	= vm.m_Driver;
			m_Device	= vm.m_Device;
			m_Width		= vm.m_Width;
			m_Height	= vm.m_Height;
			m_Depth		= vm.m_Depth;
			m_Freq		= vm.m_Freq;
			m_Mode		= vm.m_Mode;
			
			return *this;
		}
		bool IsValid(void) const {return (m_Mode!=M_UNINITIALIZED)?true:false;}
	};
private:
	class MWController;
private:
	MWController* m_MWController;
	Controller* m_Controller;
	VideoMode m_VideoMode;
	DDMainWnd* m_MainWnd;
public:
	DDFrame(const char* app_name,unsigned int app_icon);
	virtual ~DDFrame();
public:
	//	установить контроллер
	Controller* SetController(Controller* controller);
	//	установить видео режим
	enum FLAG {F_NONE,F_TRIPLEBUFFER};
	void SetVideoMode(const VideoMode& vm,unsigned int flags = F_NONE);
	//	отображаем окно
	void ShowWindow(void);
	//	обработка оконных сообщений без ожидания
	void Tick(void);
	//	обработка оконных сообщений с ожиданием
	void Wait(void);
	//	проверка положения мыши
	void CheckMouse(void);
	//	мышь в пределах окна?
	bool IsMouseWithin(void) const;
	//	текущий видео режим
	const VideoMode& CurrentVideoMode(void) const;
	//	создать окно (написано для применения к бинкам: что-то там со звуком не так)
	void CreateHandle(VideoMode::MODE mode);
	//	получить идентификатор окна
	HWND Handle(void) const;
private:
	friend MWController;
	void OnClose(void);
	void OnPaint(void);
	void OnMove(short x,short y);
	void OnMouseMove(void);
	void OnActivate(void);
	void OnDeactivate(void);
	void OnResize(int width,int height);
};

//	текущий видео режим
inline const DDFrame::VideoMode& DDFrame::CurrentVideoMode(void) const
{
	return m_VideoMode; 
}

//*********************************************************************//
//	class DDFrame::VideoMode
inline bool operator==(const DDFrame::VideoMode& a,const DDFrame::VideoMode& b)
{
	return (a.m_Driver == b.m_Driver) &&
		   (a.m_Device == b.m_Device) &&
		   (a.m_Width  == b.m_Width)  &&
		   (a.m_Height == b.m_Height) &&
		   (a.m_Depth  == b.m_Depth)  &&
		   (a.m_Freq   == b.m_Freq)   &&
		   (a.m_Mode   == b.m_Mode);
}

//*********************************************************************//
//	class DDFrame::Controller
class DDFrame::Controller
{
public:
	virtual ~Controller() {}
	virtual void OnClose(void) = 0;
	virtual void OnActivate(void) = 0;
	virtual void OnDeactivate(void) = 0;
	virtual void OnStartChangeVideoMode(void) = 0;
	virtual void OnFinishChangeVideoMode(void) = 0;
};

#endif	//_DD_FRAME_H_
