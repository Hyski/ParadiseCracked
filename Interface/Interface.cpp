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

//	��� ��������� ��������
#include "../Logic2/logicdefs.h"
#include "../Logic2/DirtyLinks.h"

//---------- ��� ���� ------------
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
	//	������ ������������ 
	void play(const files_t& files);

private:
	//	���������� �������� ����
	void OnClose(void) {}
	//	���������� ��������� ����
	void OnActivate(void) 
	{
		m_loop = &SafetyBinkPlayer::performanceLoop;
	}
	//	���������� ����������� ����
	void OnDeactivate(void) 
	{
		m_fatalInterrupt = true;
		m_loop = &SafetyBinkPlayer::idleLoop;
	}
	//	���������, ��� �� ������������
	void OnStartChangeVideoMode(void) {}
	void OnFinishChangeVideoMode(void) {}

private:
	//	���� ����������
	void performanceLoop(void);
	//	���� ��������
	void idleLoop(void);

};

SafetyBinkPlayer::SafetyBinkPlayer() : m_loop(&SafetyBinkPlayer::performanceLoop),
									   m_fatalInterrupt(false)
{
}

SafetyBinkPlayer::~SafetyBinkPlayer()
{
}

//	������ ������������ 
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

	//	��������� ������� ����
	if(ISound::isInitialized()) ISound::instance()->muteChannel(ISound::cMaster,true);
	//	��������� ���������� �������
	DDFrame::Controller* pc = Frame()->SetController(0);
	//	��������� ��� ������ ��������� � ������� ������������
	if(pc) Shell::Instance()->OnStartChangeVideoMode();
	//	��� ���������� ������ ����� � ������ ���������� �����-������ ����
	if(!Frame()->Handle()) Frame()->CreateHandle(vm.m_Mode);
	//	����������� ��������
	for(unsigned int i=0;i<files.size();i++)
	{
		if(m_fatalInterrupt) break;

		std::string file_name = Options::Registry()->Var("Video Path").GetString()+std::string("\\")+files[i];
		if(BinkPlayer::Instance()->Open(file_name.c_str()))
		{
			//	�������������� ����� ����� ��� ��������������� ����
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
			//	������� �����������
			D3DKernel::GetBB()->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&ddfx);
			//	���������� ����
			Frame()->ShowWindow();
			//	���������� ����� ���������� �������
			Frame()->SetController(this);
			//	������ �������
			while(!BinkPlayer::Instance()->IsFinish())
			{
				//	������� ���� 
				(this->*m_loop)();
				//	�������� �� �����
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
			{//	��������� ��������� ����
				if(!no_prev_video_mode)
				{//	������������� ���������� ����� �����
					Frame()->SetVideoMode(vm,ff);
					//	���������� ������
					Frame()->ShowWindow();
				}
			}
		}
		else
		{
			//throw CASUS(std::string("�� ���� ��������� ���� \"")+file_names[i]+std::string("\"."));
			//throw CASUS(std::string("�������� ���� � � ������������� ���� ������."));
			throw CASUS(DirtyLinks::GetStrRes("mainmenu_no_film"));
		}
	}
	//	���������� ���������� �������
	Frame()->SetController(pc);
	//	���������� ��� ������ ��������� � ������������
	if(pc) Shell::Instance()->OnFinishChangeVideoMode();
	//	�������������� ������� ����
	if(ISound::isInitialized()) ISound::instance()->muteChannel(ISound::cMaster,false);
}

//	���� ����������
void SafetyBinkPlayer::performanceLoop(void)
{
	//	��������� �����
	BinkPlayer::Instance()->NextFrame();
	//	����������
	BinkPlayer::Instance()->Render(D3DKernel::GetBB());
	//	�������
	D3DKernel::Blt();
	//	�������
	while(BinkPlayer::Instance()->Wait()) Sleep(1);
	//	���������� �������
	Timer::Update();
	//	��������� ��������� �� ����
	Frame()->Tick();
	//	�����
	Input::Update();
}
//	���� ��������
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
			//	����� �� ����
			if(argv[0] == "quit")
			{
				Shell::Instance()->Exit();
				return;
			}
			//	���������� ������ HiddenMovement
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

//	-------	�������� ���������� ��������� ���������� -------
	Timer::Update();
	ftime = Timer::GetSeconds();
	//	�������� �������� ���� 
	MenuMgr::Instance()->OnMouse();
	//	�������� ������������ ���� 
	MenuMgr::Instance()->OnKeyboard();
	//	�������� ����������
	MenuMgr::Instance()->OnTick();

	//	�������� ���������� BinkMgr
	BinkMgr::Instance()->Update();

	// �������� ���������� ���� 
	if(!MenuMgr::Instance()->IsLockInput())
	{
		Game::ProcessInput();
	}
	//	---------- ��������� ���������� ------------------------
	//	������� ��������
	if(Input::KeyFront(DIK_SYSRQ)) Game::MakeScreenShot();
	//	������������ ��������� ����������
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
		{//	������������ � ������� �����
			Frame()->SetVideoMode(DDFrame::VideoMode(0,0,800,600,16,0,DDFrame::VideoMode::M_WINDOWED));
			Frame()->ShowWindow();
		}
#endif //_HOME_VERSION

	//	����������� ���������
	Tip::MoveTo(Input::MouseState().x,Input::MouseState().y);
}

void Interface::OnChangeVideoMode(void)
{
	//	��������� ����
	Input::Mouse()->SetBorder(-1,-1,D3DKernel::ResX(),D3DKernel::ResY());
	Input::Mouse()->SetPos(D3DKernel::ResX()>>1,D3DKernel::ResY()>>1);
	MenuMgr::MouseCursor()->SetBorder(0,0,D3DKernel::ResX()-1,D3DKernel::ResY()-1);
	MenuMgr::MouseCursor()->OnChangeVideoMode();
	//	������������� 2D ����
	MenuMgr::Instance()->OnChangeVideoMode();
	//	�������� ����������
	BannerMgr::Tune();
	//	����������� ������������ �������� �����
//	BinkPlayer::Instance()->Close();
	//	������ � �������
	if(Console::IsAvailable())
	{
		char pBuff[200];
		Console::AddString(">> video mode changed");
		sprintf(pBuff,"now using %s on %s",D3DInfo::m_vDriverInfo[Frame()->CurrentVideoMode().m_Driver].vDevice[Frame()->CurrentVideoMode().m_Device].sName.c_str(),D3DInfo::m_vDriverInfo[Frame()->CurrentVideoMode().m_Driver].sDescription.c_str());
		Console::AddString(pBuff);
	}
}

//	����������� 2D ����������
void Interface::Render2D(void)
{
	MenuMgr::Instance()->Render();

	//	������������ ����
	MenuMgr::MouseCursor()->SetPos(Input::MouseState().x,Input::MouseState().y);
	MenuMgr::MouseCursor()->Render();
}

//	����������� 3D ����������
void Interface::Render3D(void)
{
	STACK_GUARD("Render3D");
	static bool bRenderDebug = true;

	//	������������ �������
	BannerMgr::DrawBanners();
	//	---------------------------------
#ifdef _HOME_VERSION
	DebugInfo::Add(0,60,"_HOME_VERSION");
#endif
	if(bRenderDebug)
	{//	������������ DebugInfo
		STACK_GUARD("DebugRender");
		DebugInfo::Add(135,5,"% 4d fps",(int)ceil(Shell::Instance()->FPS()));
		DebugInfo::Add(320,5,"Files open: %d",DataMgr::GetFileNum());

		//for(int t=0;t<100;t++) DataMgr::GetDataSize(); //�������� HACK ������ ���!

		DebugInfo::Add(450,5,"Bytes: %d",DataMgr::GetDataSize());

		//	��������� ����
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
	//	���������� ���������� �������
	if(g_pCIC)
	{
		Console::RemoveController(g_pCIC);
		delete g_pCIC;
		g_pCIC = 0;
	}
	//	���������� ���������� ������� �������
	if(g_pTI)
	{
		Console::RemoveController(g_pTI);
		delete g_pTI;
		g_pTI = 0;
	}
	//	����������� �������
	MenuMgr::Instance()->Release();
}

//**********************************************************************************//
//	�������� ������
void Interface::LoadGameLevel(const char* pLevelName)
{
	MenuMgr::Instance()->LoadLevel(pLevelName);
}
//	���������� �������c ���� �������� ������
void Interface::UpdateLoadingProgressBar(float value)
{
	MenuMgr::Instance()->UpdateLoadingProgressBar(value);
}
//	���������� ��������� �������� ���� �������� ������
void Interface::SetProgressBarTitle(const char* title)
{
	MenuMgr::Instance()->SetProgressBarTitle(title);
}
//	��������� �������
void Interface::PlayBink(const char* file_name)
{
	std::vector<std::string> biks;

	biks.push_back(file_name);
	PlayBink(biks);
}

//	��������� ��������� ���������
void Interface::PlayBink(std::vector<std::string>& file_names)
{
	SafetyBinkPlayer bp;
	bp.play(file_names);
}
//	�������� ����
void Interface::RunMenuFlow(void)
{
	if(m_IsInitialized) throw CASUS("������� ��������� ������������� ����������!");

	MenuMgr::Instance();
	iface("create MenuMgr instance;\n");

	m_IsInitialized = true;

	//	�������������� �������� ������
	MenuMgr::MouseCursor()->SetBorder(0,0,D3DKernel::ResX()-1,D3DKernel::ResY()-1);
	Input::Mouse()->SetBorder(-1,-1,D3DKernel::ResX(),D3DKernel::ResY());
	Input::Mouse()->SetPos(D3DKernel::ResX()>>1,D3DKernel::ResY()>>1);
	iface("initialize Mouse Cursor;\n");

	//	������� ���������� ��� ��������� ���������� �� �������
	if(!g_pCIC)
	{
		g_pCIC = new ConsoleInfo();
		Console::AddController(g_pCIC);
		iface("...add console controller for console info;\n");
	}
	//	������� ���������� ��� ��������� ���������� � ��������� �� �������
	if(!g_pTI)
	{
		g_pTI = new TexturesInfo();
		Console::AddController(g_pTI);
		iface("...add console controller for textures info;\n");
	}
	iface("exit from RunMenuFlow function;\n\n");
}
//	��������/������� ������ ����������� ����������
void Interface::MakeSaveLoad(Storage& store)
{
	MenuMgr::Instance()->MakeSaveLoad(store);
}
//	����� ����������� ���� yes/no
int Interface::QuestionBox(const char* text)
{
	return MenuMgr::Instance()->QuestionBox(text);
}
//	��������������� ��������� ����������
void Interface::PriorInitialize(void)
{
	//	������������ ������������� ���������
	if(!CmdLine::IsKey("-nointro"))
	{//	������������ ���������� ��������
		std::vector<std::string> biks;

		biks.push_back("buka.bik");
		biks.push_back("mistland.bik");
		biks.push_back("intro.bik");
		PlayBink(biks);
	}
	//	�������������� ��������� ������ � �������� �����
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
//	��������� ��������� ����������� �������� 0..1 �� �������� from..to
void Interface::ProgressBar::SetRange(float from, float to)
{
	m_From = from; m_To = to;
}
//	��������� �������� ���� � ������������� ���������
void Interface::ProgressBar::SetPercent(float value)
{
	Interface::Instance()->UpdateLoadingProgressBar(m_From+(m_To-m_From)*value);
}
//	��������� �������� �� ������� m/n
void Interface::ProgressBar::SetMofN(int m, int n)
{
	SetPercent(static_cast<float>(m)/static_cast<float>(n));
}
//	��������� ������, ����������� ������� ����������� ������
void Interface::ProgressBar::SetTitle(const char* title)
{
	Interface::Instance()->SetProgressBarTitle(title);
}
//	�������� �����������, ��� ������ ����� ����������
bool Interface::IsCameraMoveable(void)
{
	return MenuMgr::Instance()->IsCameraMoveable();
}
