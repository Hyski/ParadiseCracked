#include "Precomp.h"
#include "../Common/Shell/Shell.h"
#include "../Game.h"
#include "../Common/D3DApp/D3DInfo/D3DInfo.h"
#include "../Common/TextureMgr/TextureMgr.h"
#include "../Common/SurfaceMgr/SurfaceMgr.h"
#include "../Common/FontMgr/FontMgr.h"
#include "../Options/Options.h"
#include "../Options/XRegistry.h"
#include "../Common/CmdLine/CmdLine.h"
#include "../Common/BinkPlayer/BinkPlayer.h"
#include "../Common/BinkMgr/BinkMgr.h"
#include "../Sound/ISound.h"
#include "LoadingScreen.h"
#include "MouseCursor.h"
#include "BannerMgr.h"
#include "GraphMgr.h"
#include "MenuMgr.h"
#include "Console.h"
#include "Tip.h"
#include "Interface.h"

//	для строковых ресурсов
#include "../Logic2/logicdefs.h"
#include "../Logic2/DirtyLinks.h"

//---------- Лог файл ------------
#ifdef _HOME_VERSION
CLog iface_log;
#define iface	iface_log["iface.log"]
#else
#define iface	/##/
#endif

//=====================================================================================//
//                               class SafetyBinkPlayer                                //
//=====================================================================================//
class SafetyBinkPlayer : public DDFrame::Controller
{
public:

	typedef std::vector<std::string> files_t;

private:

	typedef void (SafetyBinkPlayer::*loop_t)(void);
	bool m_fatalInterrupt;
	loop_t m_loop;

public:
	SafetyBinkPlayer();
	virtual ~SafetyBinkPlayer();
public:
	//	начать проигрывание 
	void play(const files_t& files);

private:
	//	обработать закрытие окна
	void OnClose(void) {}
	//	обработать активацию окна
	void OnActivate(void) 
	{
		m_loop = &SafetyBinkPlayer::performanceLoop;
	}
	//	обработать деактивацию окна
	void OnDeactivate(void) 
	{
		m_fatalInterrupt = true;
		m_loop = &SafetyBinkPlayer::idleLoop;
	}
	//	остальное, нас не интересующее
	void OnStartChangeVideoMode(void) {}
	void OnFinishChangeVideoMode(void) {}

private:
	//	цикл исполнения
	void performanceLoop(void);
	//	цикл ожидания
	void idleLoop(void);

};

SafetyBinkPlayer::SafetyBinkPlayer() : m_loop(&SafetyBinkPlayer::performanceLoop),
									   m_fatalInterrupt(false)
{
}

SafetyBinkPlayer::~SafetyBinkPlayer()
{
}

//	начать проигрывание 
void SafetyBinkPlayer::play(const files_t& files)
{
	DDFrame::VideoMode vm(Frame()->CurrentVideoMode());
	DDBLTFX ddfx;
	bool no_prev_video_mode = false;
	unsigned int ff = Options::GetInt("system.video.triple")?DDFrame::F_TRIPLEBUFFER:DDFrame::F_NONE;

	if(CmdLine::IsKey("-nofilms")) return;

#if defined(DEMO_VERSION)
	return;
#endif

	if(!vm.IsValid())
	{
		no_prev_video_mode = true;
		if(Options::GetInt("system.video.windowed") && !CmdLine::IsKey("-fs"))
		{
			vm.m_Driver		= Options::GetInt("system.video.driver");
			vm.m_Device		= Options::GetInt("system.video.device");
			vm.m_Width		= 800;
			vm.m_Height		= 600;
			vm.m_Depth		= Options::GetInt("system.video.bpp");
			vm.m_Freq		= 0;
			vm.m_Mode		= DDFrame::VideoMode::M_WINDOWED;
		}
		else
		{
			vm.m_Driver		= Options::GetInt("system.video.driver");
			vm.m_Device		= Options::GetInt("system.video.device");
			vm.m_Width		= 800;
			vm.m_Height		= 600;
			vm.m_Depth		= Options::GetInt("system.video.bpp");
			vm.m_Freq		= Options::GetInt("system.video.freq");
			vm.m_Mode		= DDFrame::VideoMode::M_FULLSCREEN;
		}
	}

	//	выключаем игровой звук
	if(ISound::isInitialized()) ISound::instance()->muteChannel(ISound::cMaster,true);
	//	отключаем контроллер каркаса
	DDFrame::Controller* pc = Frame()->SetController(0);
	//	выгружаем все данные связанные с текущим видеорежимом
	if(pc) Shell::Instance()->OnStartChangeVideoMode();
	//	для корректной работы звука в бинках необходимо какое-нибудь окно
	if(!Frame()->Handle()) Frame()->CreateHandle(vm.m_Mode);
	//	проигрываем мультики
	for(unsigned int i=0;i<files.size();i++)
	{
		if(m_fatalInterrupt) break;

		std::string file_name = Options::Registry()->Var("Video Path").GetString()+std::string("\\")+files[i];
		if(BinkPlayer::Instance()->Open(file_name.c_str()))
		{
			//	инициализируем видео режим для воспроизведения бика
			if(vm.m_Mode == DDFrame::VideoMode::M_WINDOWED)
			{
				Frame()->SetVideoMode(DDFrame::VideoMode(vm.m_Driver,
														 vm.m_Device,
														 BinkPlayer::Instance()->Width(),
														 BinkPlayer::Instance()->Height(),
														 vm.m_Depth,
														 vm.m_Freq,
														 vm.m_Mode),ff);
			}
			else
			{
				Frame()->SetVideoMode(DDFrame::VideoMode(vm.m_Driver,
														 vm.m_Device,
														 800,
														 600,
														 vm.m_Depth,
														 vm.m_Freq,
														 vm.m_Mode),ff);
			}
			memset(&ddfx,0,sizeof(DDBLTFX));
			ddfx.dwSize = sizeof(DDBLTFX);
			//	очищаем поверхность
			D3DKernel::GetBB()->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&ddfx);
			//	показываем окно
			Frame()->ShowWindow();
			//	подключаем новый контроллер каркаса
			Frame()->SetController(this);
			//	играем мультик
			while(!BinkPlayer::Instance()->IsFinish())
			{
				//	игровой цикл 
				(this->*m_loop)();
				//	проверка на выход
				if(Input::KeyState(DIK_SPACE|Input::RESET))
					break;
				if(Input::KeyState(DIK_ESCAPE|Input::RESET))
				{
					i = files.size()-1;
					break;
				}

				if(m_fatalInterrupt) break;
			}

			BinkPlayer::Instance()->Close();

			if(i == (files.size()-1))
			{//	проиграли последний файл
				if(!no_prev_video_mode)
				{//	устанавливаем предыдущий видео режим
					Frame()->SetVideoMode(vm,ff);
					//	показываем окошко
					Frame()->ShowWindow();
				}
			}
		}
		else
		{
			//throw CASUS(std::string("Не могу проиграть файл \"")+file_names[i]+std::string("\"."));
			//throw CASUS(std::string("Вставьте диск Б и перезапустите игру заново."));
			throw CASUS(DirtyLinks::GetStrRes("mainmenu_no_film"));
		}
	}
	//	подключаем контроллер каркаса
	Frame()->SetController(pc);
	//	подгружаем все данные связанные с видеорежимом
	if(pc) Shell::Instance()->OnFinishChangeVideoMode();
	//	востанавливаем игровой звук
	if(ISound::isInitialized()) ISound::instance()->muteChannel(ISound::cMaster,false);
}

//	цикл исполнения
void SafetyBinkPlayer::performanceLoop(void)
{
	//	следующий фрэйм
	BinkPlayer::Instance()->NextFrame();
	//	отображаем
	BinkPlayer::Instance()->Render(D3DKernel::GetBB());
	//	блитинг
	D3DKernel::Blt();
	//	ожидаем
	while(BinkPlayer::Instance()->Wait()) Sleep(1);
	//	обновление таймера
	Timer::Update();
	//	обработка сообщений от окна
	Frame()->Tick();
	//	выход
	Input::Update();
}
//	цикл ожидания
void SafetyBinkPlayer::idleLoop(void)
{
	Frame()->Wait();
}

//**********************************************************************************//
//**************************** class ConsoleInfo ***********************************//
class ConsoleInfo : public Console::Controller
{
public:
	ConsoleInfo(){}
	virtual ~ConsoleInfo(){}
public:
	void Activate(void)
	{
		std::vector<std::string> argv;

		if(Console::GetCommandLine(argv))
		{
			//	выход из игры
			if(argv[0] == "quit")
			{
				Shell::Instance()->Exit();
				return;
			}
			//	отключение экрана HiddenMovement
			if(argv[0] == "hm")
			{
				HiddenMovementScreen::m_DebugMode = (argv[1] == "0");
				return;
			}
			if((argv[0] == "level") && (argv.size()>1))
			{
				Interface::Instance()->LoadGameLevel(argv[1].c_str());
				return;
			}
		}
	}
};

static ConsoleInfo* g_pCIC = 0;

class TexturesInfo : public Console::Controller
{
public:
	TexturesInfo(){}
	virtual ~TexturesInfo(){}
public:
	void Activate(void)
	{
		std::vector<std::string> argv;

		if(Console::GetCommandLine(argv))
		{
			if(argv[0] == "textures")
			{
				std::vector<TextureMgr::Description> info;
				char buff[100];

				TextureMgr::Instance()->Report(info);
				for(int i=0;i<info.size();i++)
				{	
					Console::AddString(info[i].m_Name.c_str());
					sprintf(buff,"%dx%dx%d mipmap[%d]",info[i].m_Width,info[i].m_Height,info[i].m_Bpp,info[i].m_MipMapCount);
					Console::AddString(buff);
				}
				sprintf(buff,"quantity: %d",info.size());
				Console::AddString(buff);
				sprintf(buff,"memory size: %d",TextureMgr::Instance()->Memory());
				Console::AddString(buff);
				
				return;
			}
		}
	}
};

static TexturesInfo* g_pTI = 0;
//**********************************************************************************//
//**************************** class Interface ***********************************//

Interface::Deleter Interface::m_Deleter;
bool Interface::m_IsInitialized = false;

Interface::Interface() : m_ProgressBar(new ProgressBar())
{
}

Interface::~Interface()
{
	delete m_ProgressBar;
}

void Interface::Tick(void)
{
  STACK_GUARD("Interface::Tick");
	static float full_time;
	static float ftime;


	Timer::Update();
	full_time = Timer::GetSeconds();

//	-------	ПЕРЕДАЧА УПРАВЛЕНИЯ ЭЛЕМЕНТАМ ИНТЕРФЕЙСА -------
	Timer::Update();
	ftime = Timer::GetSeconds();
	//	передаем мышинный ввод 
	MenuMgr::Instance()->OnMouse();
	//	передаем клавиатурный ввод 
	MenuMgr::Instance()->OnKeyboard();
	//	передаем управление
	MenuMgr::Instance()->OnTick();

	//	передаем управление BinkMgr
	BinkMgr::Instance()->Update();

	// передача управления игре 
	if(!MenuMgr::Instance()->IsLockInput())
	{
		Game::ProcessInput();
	}
	//	---------- обработка КЛАВИАТУРЫ ------------------------
	//	сделаем скриншот
	if(Input::KeyFront(DIK_SYSRQ)) Game::MakeScreenShot();
	//	переключение раскладки клавиатуры
	if(Input::KeyFront(DIK_CAPITAL)) ActivateKeyboardLayout((HKL)HKL_NEXT,0);

#ifdef _HOME_VERSION
		if(Input::KeyFront(DIK_NUMPAD0|Input::RESET))
			Shell::Instance()->Exit();
		if(Input::KeyFront(DIK_GRAVE))
		{
			Console::Show(!Console::IsOnScreen());
		}
		if(GraphMgr::Window("fps"))
		{
			static bool exist = false;
			if(!exist) {GraphMgr::Window("fps")->Calibrate(10,0,100);exist=true;}
			GraphMgr::Window("fps")->AddValue(Timer::GetSeconds(),Shell::Instance()->FPS());
		}
		if(Input::KeyFront(DIK_NUMPAD1))
		{
			Frame()->SetVideoMode(DDFrame::VideoMode(0,0,640,480,16,0,DDFrame::VideoMode::M_FULLSCREEN));
			Frame()->ShowWindow();
		}
		if(Input::KeyFront(DIK_NUMPAD2))
		{
			Frame()->SetVideoMode(DDFrame::VideoMode(0,0,800,600,16,0,DDFrame::VideoMode::M_FULLSCREEN));
			Frame()->ShowWindow();
		}
		if(Input::KeyFront(DIK_NUMPAD3))
		{
			Frame()->SetVideoMode(DDFrame::VideoMode(0,0,1024,768,16,0,DDFrame::VideoMode::M_FULLSCREEN));
			Frame()->ShowWindow();
		}
		if(Input::KeyFront(DIK_NUMPAD4))
		{
			GraphMgr::BuildWindow("fps",200,100);
		}
		if(Input::KeyFront(DIK_NUMPAD5))
		{
			GraphMgr::BuildWindow("tick.interface",200,100);
		}
		if(Input::KeyState(DIK_F12))
		{//	переключение в оконный режим
			Frame()->SetVideoMode(DDFrame::VideoMode(0,0,800,600,16,0,DDFrame::VideoMode::M_WINDOWED));
			Frame()->ShowWindow();
		}
#endif //_HOME_VERSION

	//	передвигаем подсказку
	Tip::MoveTo(Input::MouseState().x,Input::MouseState().y);
}

void Interface::OnChangeVideoMode(void)
{
	//	обработка мыши
	Input::Mouse()->SetBorder(-1,-1,D3DKernel::ResX(),D3DKernel::ResY());
	Input::Mouse()->SetPos(D3DKernel::ResX()>>1,D3DKernel::ResY()>>1);
	MenuMgr::MouseCursor()->SetBorder(0,0,D3DKernel::ResX()-1,D3DKernel::ResY()-1);
	MenuMgr::MouseCursor()->OnChangeVideoMode();
	//	инициализация 2D меню
	MenuMgr::Instance()->OnChangeVideoMode();
	//	элементы интерфейса
	BannerMgr::Tune();
	//	прекращение проигрывания текущего бинка
//	BinkPlayer::Instance()->Close();
	//	запись в консоль
	if(Console::IsAvailable())
	{
		char pBuff[200];
		Console::AddString(">> video mode changed");
		sprintf(pBuff,"now using %s on %s",D3DInfo::m_vDriverInfo[Frame()->CurrentVideoMode().m_Driver].vDevice[Frame()->CurrentVideoMode().m_Device].sName.c_str(),D3DInfo::m_vDriverInfo[Frame()->CurrentVideoMode().m_Driver].sDescription.c_str());
		Console::AddString(pBuff);
	}
}

//	отображение 2D интерфейса
void Interface::Render2D(void)
{
	MenuMgr::Instance()->Render();

	//	отрисовываем мышь
	MenuMgr::MouseCursor()->SetPos(Input::MouseState().x,Input::MouseState().y);
	MenuMgr::MouseCursor()->Render();
}

//	отображение 3D интерфейса
void Interface::Render3D(void)
{
	STACK_GUARD("Render3D");
	static bool bRenderDebug = true;

	//	отрисовываем баннеры
	BannerMgr::DrawBanners();
	//	---------------------------------
#ifdef _HOME_VERSION
	DebugInfo::Add(0,60,"_HOME_VERSION");
#endif
	if(bRenderDebug)
	{//	отрисовываем DebugInfo
		STACK_GUARD("DebugRender");
		DebugInfo::Add(135,5,"% 4d fps",(int)ceil(Shell::Instance()->FPS()));
		DebugInfo::Add(320,5,"Files open: %d",DataMgr::GetFileNum());

		//for(int t=0;t<100;t++) DataMgr::GetDataSize(); //ВНИМАНИЕ HACK убрать это!

		DebugInfo::Add(450,5,"Bytes: %d",DataMgr::GetDataSize());

		//	состояние мышь
		DebugInfo::Add(D3DKernel::ResX()-135,D3DKernel::ResY()-20,"Mouse dz: (%0.f)",Input::MouseState().dz);


		DebugInfo::Render();
		DebugInfo::Clear();
	}
	else
	{
		STACK_GUARD("FpsRender");
		DebugInfo::Clear();
		DebugInfo::Add(135,5,"% 4d fps",(int)ceil(Shell::Instance()->FPS()));

		DebugInfo::Render();
	}
	if(Input::KeyFront(DIK_NUMPADSTAR))	bRenderDebug = !bRenderDebug;
}

void Interface::Release(void)
{
	//	уничтожаем контроллер консоли
	if(g_pCIC)
	{
		Console::RemoveController(g_pCIC);
		delete g_pCIC;
		g_pCIC = 0;
	}
	//	уничтожаем контроллер консоли текстур
	if(g_pTI)
	{
		Console::RemoveController(g_pTI);
		delete g_pTI;
		g_pTI = 0;
	}
	//	освобождаем ресурсы
	MenuMgr::Instance()->Release();
}

//**********************************************************************************//
//	загрузка уровня
void Interface::LoadGameLevel(const char* pLevelName)
{
	MenuMgr::Instance()->LoadLevel(pLevelName);
}
//	обновление прогресc бара ЗАГРУЗКИ УРОВНЯ
void Interface::UpdateLoadingProgressBar(float value)
{
	MenuMgr::Instance()->UpdateLoadingProgressBar(value);
}
//	установить заголовок прогресс бара ЗАГРУЗКИ УРОВНЯ
void Interface::SetProgressBarTitle(const char* title)
{
	MenuMgr::Instance()->SetProgressBarTitle(title);
}
//	проиграть мультик
void Interface::PlayBink(const char* file_name)
{
	std::vector<std::string> biks;

	biks.push_back(file_name);
	PlayBink(biks);
}

//	проиграть несколько мультиков
void Interface::PlayBink(std::vector<std::string>& file_names)
{
	SafetyBinkPlayer bp;
	bp.play(file_names);
}
//	создание меню
void Interface::RunMenuFlow(void)
{
	if(m_IsInitialized) throw CASUS("Попытка повторной инициализации интерфейса!");

	MenuMgr::Instance();
	iface("create MenuMgr instance;\n");

	m_IsInitialized = true;

	//	инициализируем мышинный курсор
	MenuMgr::MouseCursor()->SetBorder(0,0,D3DKernel::ResX()-1,D3DKernel::ResY()-1);
	Input::Mouse()->SetBorder(-1,-1,D3DKernel::ResX(),D3DKernel::ResY());
	Input::Mouse()->SetPos(D3DKernel::ResX()>>1,D3DKernel::ResY()>>1);
	iface("initialize Mouse Cursor;\n");

	//	создаем контроллер для получения информации из консоли
	if(!g_pCIC)
	{
		g_pCIC = new ConsoleInfo();
		Console::AddController(g_pCIC);
		iface("...add console controller for console info;\n");
	}
	//	создаем контроллер для получения информации о текстурах из консоли
	if(!g_pTI)
	{
		g_pTI = new TexturesInfo();
		Console::AddController(g_pTI);
		iface("...add console controller for textures info;\n");
	}
	iface("exit from RunMenuFlow function;\n\n");
}
//	записать/считать данные необходимые интерфейсу
void Interface::MakeSaveLoad(Storage& store)
{
	MenuMgr::Instance()->MakeSaveLoad(store);
}
//	вызов диалогового окна yes/no
int Interface::QuestionBox(const char* text)
{
	return MenuMgr::Instance()->QuestionBox(text);
}
//	предварительная настройка интерфейса
void Interface::PriorInitialize(void)
{
	//	проигрывание вступительных мультиков
	if(!CmdLine::IsKey("-nointro"))
	{//	проигрывание интрошного мультика
		std::vector<std::string> biks;

		biks.push_back("buka.bik");
		biks.push_back("mistland.bik");
		biks.push_back("intro.bik");
		PlayBink(biks);
	}
	//	анализирование командной строки и настроек опций
	if(CmdLine::IsKey("-level"))
		MenuMgr::CheckVideoMode(MenuMgr::CVM_FOR_CFG);
	else
		MenuMgr::CheckVideoMode(MenuMgr::CVM_FOR_MAINMENU);
}

/////////////////////////////////////////////////////////////////////////
///////////////////    class Interface::ProgressBar    //////////////////
/////////////////////////////////////////////////////////////////////////
Interface::ProgressBar::ProgressBar() : m_From(0),m_To(1)
{
}

Interface::ProgressBar::~ProgressBar()
{
}
//	установка интервала отображения значений 0..1 на интервал from..to
void Interface::ProgressBar::SetRange(float from, float to)
{
	m_From = from; m_To = to;
}
//	установка процента бара в установленном интервале
void Interface::ProgressBar::SetPercent(float value)
{
	Interface::Instance()->UpdateLoadingProgressBar(m_From+(m_To-m_From)*value);
}
//	установка процента по формуле m/n
void Interface::ProgressBar::SetMofN(int m, int n)
{
	SetPercent(static_cast<float>(m)/static_cast<float>(n));
}
//	установка строки, описывающей текущие загружаемые данные
void Interface::ProgressBar::SetTitle(const char* title)
{
	Interface::Instance()->SetProgressBarTitle(title);
}
//	получить утверждение, что камеру можно перемещать
bool Interface::IsCameraMoveable(void)
{
	return MenuMgr::Instance()->IsCameraMoveable();
}
