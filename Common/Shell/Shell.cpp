/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#include "Precomp.h"
#include "..\D3DApp\GammaControl\GammaControl.h"
#include "..\GraphPipe\GraphPipe.h"
#include "..\TextureMgr\TextureMgr.h"
#include "..\DataMgr\DataMgr.h"
#include "..\FontMgr\FontMgr.h"
#include "..\..\Game.h"
#include "..\..\Options\Options.h"
#include "..\..\Options\PCOptions.h"
#include "..\..\resource.h"
#include "..\..\logic2\GameLogic.h"
#include "..\CmdLine\CmdLine.h"
#include "../../Options/XRegistry.h"
#include "..\SurfaceMgr\SurfaceMgr.h"
#include "..\BinkMgr\BinkMgr.h"
#include "../utils/profiler.h"
#include "../../ModMgr.h"
#include "Shell.h"

#include "../../sound/ISound.h"
#include "../gsound/filesystem.h"

//---------- ��� ���� ------------
//#define _DEBUG_SHELL
#ifdef _DEBUG_SHELL
CLog shell_log;
#define shell	shell_log["shell.log"]
#else
#define shell	/##/
#endif

#include "../utils/dir.h"
#include "../../skin/animalibrary.h"
namespace
{
	//	������������ ��������������� ��������
class DS:public DirectoryScanner
	{
	bool OnItem(const Item &it)
		{
		if(it.Ext==".skel")
			{
      AnimaLibrary::GetInst()->GetSkAnimation(it.FullName);
			}
		else if(it.Ext==".skin")
			{
      SkSkin *s=AnimaLibrary::GetInst()->GetSkSkin(it.FullName);
			delete s;
			}
		AnimaLibrary::GetInst()->Clear();
		return true;
		};
	};
	void ConvertAnimations(void)
	{
		if(CmdLine::IsKey("-convert"))
		{
			DS d;
			d.SetPath("animations","*.skin;*.skel");
			throw CasusImprovisus("��������� ��������������� ��������.");
		}
	}

	SndServices svc;
}

//*********************************************************************//
// class Shell::FPSometer
class Shell::FPSometer
{
private:
	enum {NUMOFFRAME=25};
	float m_LastTime;
	float m_TimeQueue[NUMOFFRAME];
	int m_EndQueue;
public:
	FPSometer() : m_LastTime(0),m_EndQueue(0)
	{
		memset(m_TimeQueue,NUMOFFRAME*sizeof(float),0);
	}
	virtual ~FPSometer() {}
public:
	//	���������� ������� �����
	void SetTime(float seconds)
	{
		for(int i=0;i<NUMOFFRAME;i++) m_TimeQueue[i] = seconds-m_LastTime;
		m_LastTime = seconds;
	}
	//	�������� ����� ��� ��������
	void AddTime(float seconds)
	{
	    m_TimeQueue[m_EndQueue++] = seconds-m_LastTime;
		if(m_EndQueue == NUMOFFRAME) m_EndQueue = 0;
		m_LastTime = seconds;    
	}
	//	�������� ������� ���-�� ������ � �������
	float FPS(void) const
	{
		float sum = 0;
		for(int i=0;i<NUMOFFRAME;i++) sum += m_TimeQueue[i];

		return static_cast<float>(NUMOFFRAME)/sum;
	}
};

//*********************************************************************//
//	struct Shell::Container
struct Shell::Container
{
	//	�������� FPS
	FPSometer m_FPSometer;
	//	������ ������ �������
	GraphPipe* m_GraphPipe;
	//	������ ����������
	DDFrame* m_Frame;
	//	�����������
	Container() : m_GraphPipe(0),m_Frame(0) {}
	//	����������
	~Container()  
	{
		if(m_Frame) delete m_Frame;
#if !defined(USE_SECUROM_TRIGGERS)
		if(m_GraphPipe) delete m_GraphPipe;
#endif
	}
	template<class T>
	void Delete(T& object) {delete object; object = 0;}
};

//*********************************************************************//
// class Shell
Shell* Shell::m_Instance = 0;
const char* Shell::m_AppName = "Paradise Cracked";
Shell::Shell() : m_Container(new Container()),m_Mutex(0),m_Loop(&Shell::PerformanceLoop),
				 m_Exit(false),m_IsModalMode(false)
{
STACK_GUARD("Shell::Shell()");
	shell("Shell:\n...start initiation;\n");
	CodeProfiler::Init();
	//	��������� �� ��������� ������� ��� ������������
	m_Instance = this;
	//	������ ���������� ������
	CmdLine::Init();
	//	������������ ���������� �������
	if(!CheckExistInstance()) throw CASUS("��������� ���������� ��� ������.");
	//	������������� �������
	Timer::Init();
	//	������ ����� ����� � �������
	Options::Init();
	//	��������������� ������������ ����������
	PCOptions::Register();
	shell("   options initialized;\n");
	//	������������� DataMgr
	InitDataMgr();
	//	������������ ��������������� ��������
	ConvertAnimations();
	//	�������� ������� ����������
	m_Container->m_Frame = new DDFrame(m_AppName,IDI_MAIN_ICON);
	shell("   create DDFrame;\n");
	//	��������� ��������������� ��������� ���������� � ������������ ���������
	Game::PriorInterfaceInitialize();
	//	������������� ������ ���������� �������
	m_Container->m_GraphPipe = new GraphPipe();
	shell("   create GraphPipe;\n");
	//	��������� ����� ������
	D3DKernel::SetTriple(Options::GetInt("system.video.triple"));
	//	������������� GammaControl'�
	GammaControl::Init(D3DKernel::GetPS());
	GammaControl::SetGamma(Options::GetFloat("system.video.gamma"));
	shell("    ....GammaControl::Init();\n");
	//	---------------------------------------
	//	������������� ������ �����
	svc.m_Wnd = (long)Frame()->Handle();
	cc_VFileSystem::initInstance();
	svc.m_FileSystem = cc_VFileSystem::instance();
	ISound::initInstance(&svc);
	shell("    ...sound init;\n");
	//	---------------------------------------
	//	������������� ����
	StatesManager::Clear();
	Game::Init(m_Container->m_GraphPipe);
	shell("    ...init Game;\n");
	//	���������� ����
	m_Container->m_Frame->ShowWindow();
	shell("    ...show window;\n");
	//	��������� ���������������� ����
	Input::Mouse()->SetSensitivity(Options::GetFloat("system.input.sensitivity"));
	//	������������� �������
	TextureMgr::Instance()->Init(D3DKernel::GetD3DDevice(),
					 Options::GetInt("system.video.mipmap"),
					 Options::GetInt("system.video.texquality"),
					 Options::GetInt("system.video.usecompressing"),
					 false);
	shell("    ....TextureMgr::Instance()->Init(); \n");
	//	������������ GraphPipe
	m_Container->m_GraphPipe->Init(D3DKernel::GetD3D(),
								   D3DKernel::GetD3DDevice(),
								   D3DKernel::ResX(),
								   D3DKernel::ResY());

	shell("    ....m_pGraphPipe->Init();\n");
	//	--------------------------------------------
	m_Container->m_GraphPipe->RegisterShaderFile("shaders.sys/shaders.shader");
	//	--------------------------------------------
    //������������� AI ����
    GameLogic::GetInst()->Init();
    shell("    ... GameLogic init;\n");
	//	�������� ����������
	Game::CreateInterface();
    shell("    ... Game Interface init;\n");
	//	������������� ����� FPS'������
	m_Container->m_FPSometer.SetTime(Timer::GetSeconds());
	//	������������� ����������
	m_Container->m_Frame->SetController(this);

	shell("...finished;\n\n");
}

Shell::~Shell()
{
STACK_GUARD("Shell::~Shell");
	shell("Shell:\n...start completion;\n");

	//	������������� ����������
	m_Container->m_Frame->SetController(0);
    //���������� AI 
    shell("    ...closing GameLogic;\n");
    GameLogic::GetInst()->Shut();
    shell("    ...closed GameLogic;\n");
	//	���������� ����� �� ����
	Options::Flush();
	shell("    ...flush options to disk;\n");
	Game::Close();
	shell("    ...close Game;\n");
	//	-------------------------------------
	GammaControl::Release();
	shell("    ...destroy Gamma Control;\n");
	//	-------------------------------------
	ISound::shutInstance();
	cc_VFileSystem::shutInstance();
	shell("    ...Shutdown Sound;\n");
	//	-------------------------------------
#if !defined(USE_SECUROM_TRIGGERS)
	m_Container->Delete(m_Container->m_GraphPipe); 
#endif

	shell("   destroy GraphPipe;\n");
	//	������������ �������
	TextureMgr::Instance()->Release();
	shell("    ...release TextureMgr;\n");
	//	����������� ������� ����������
	m_Container->Delete(m_Container->m_Frame);
	shell("   destroy DDFrame;\n");
	//	-------------------------------------
	DataMgr::Uninitialize();
	shell("    ...release DataMgr;\n");
	//	��������� �����
	Options::Close();
	//	���������� �������
	if(m_Mutex) CloseHandle(m_Mutex);
	//	������� ������ �� ���������� - ��������� �� ������������
	m_Instance = 0;

	shell("...finished;\n");
}

void Shell::Go(void)
{
STACK_GUARD("Shell::Go");
	shell("Shell:\n...start the Process;\n");
	while(!m_Exit) (*this.*m_Loop)();
	shell("...finished;\n\n");
}

//	���� ����������
void Shell::PerformanceLoop(void)
	{
	STACK_GUARD("Shell::PerformanceLoop");
	
	//	���������� �������
	Timer::Update();
	//	���������� �����
		{
		STACK_GUARD("Sound::manage");
		ISound::instance()->manage();
		}
		//	���������� ��������� ��������� �����
	Input::Update();
		//	������� ���� ����
	Game::Tick();
	//	��������� ����
	Game::Render();
		{
		STACK_GUARD("Window Messages handling");
		//	������������ ������� ���������
		m_Container->m_Frame->Tick();
		//	�������� ��������� ����
		m_Container->m_Frame->CheckMouse();
		}
//	TEST
	//	����������� �� fps'��
	if(m_Container->m_FPSometer.FPS()>100)
	{
		int sleep = (0.015f-(1.0f/m_Container->m_FPSometer.FPS()))*1000;
		if(sleep)
		{
			DebugInfo::Add(235,5,"Sleep(%d)",sleep);
			Sleep(sleep);
		}
	}
//	TEST
	//	����� �����
	D3DKernel::UpdateFrame();
	//	�������� fps'�
	m_Container->m_FPSometer.AddTime(Timer::GetSeconds());
}
//	���� ��������
void Shell::IdleLoop(void)
{
STACK_GUARD("Shell::IdleLoop");
	//	������������ ������� ���������
	m_Container->m_Frame->Wait();
}
//	������� [����� �� ����������]
void Shell::OnClose(void)
{
STACK_GUARD("Shell::OnClose");
CodeProfiler::Close();
	Exit();
}

void Shell::OnActivate(void)
{
STACK_GUARD("Shell::OnActivate");
	DDFrame::VideoMode vm = m_Container->m_Frame->CurrentVideoMode();

	m_Loop = &Shell::PerformanceLoop;
	if(vm.m_Mode == DDFrame::VideoMode::M_FULLSCREEN)
	{
		m_Container->m_Frame->SetVideoMode(DDFrame::VideoMode());
		m_Container->m_Frame->SetVideoMode(vm);
//		m_Container->m_Frame->ShowWindow();
	}
	//	������ �������
	Timer::Resume();

	shell("   on activated;\n");
}

void Shell::OnDeactivate(void)
{
STACK_GUARD("Shell::OnDeactivate");
	m_Loop = &Shell::IdleLoop;
	//	��������������� �������
	Timer::Suspend();
	shell("   on deactivated;\n");
}

void Shell::OnStartChangeVideoMode(void)
{
STACK_GUARD("Shell::OnStartChangeVideoMode");
	shell("   start change Video Mode;\n");

	//	��������������� �������
	Timer::Suspend();
	//	������������ �������� GraphPipe
	m_Container->m_GraphPipe->Release();
	//	���������� �������� 2D ������������
	SurfaceMgr::Instance()->Release();
	//	������������ ������
	BinkMgr::Instance()->Release();
	//	������������ �������
	FontMgr::Instance()->Release();
	//	������������ �������
	TextureMgr::Instance()->Release();
	//	������� �������
	FontMgr::Instance()->Release();
	//	������������ GammaControl'�
	GammaControl::Release();
}

void Shell::OnFinishChangeVideoMode(void)
{
STACK_GUARD("Shell::OnFinishChangeVideoMode");
	//	���������� ����
	m_Container->m_Frame->ShowWindow();
	//	������������� �������
	TextureMgr::Instance()->Init(D3DKernel::GetD3DDevice(),
								 Options::GetInt("system.video.mipmap"),
								 Options::GetInt("system.video.texquality"),
								 Options::GetInt("system.video.usecompressing"),
								 false);
	shell("    ....TextureMgr::Instance()->Init(); \n");
	//	������������ GraphPipe
	m_Container->m_GraphPipe->Init(D3DKernel::GetD3D(),
								   D3DKernel::GetD3DDevice(),
								   D3DKernel::ResX(),
								   D3DKernel::ResY());
	shell("    ....GraphPipe->Init();\n");
	//	������������� ����
	Game::Tune();
	shell("    ....Game::Tune();\n");
	//	������������� �����-��������
	GammaControl::Init(D3DKernel::GetPS());
	shell("    ....GammaControl::Init();\n");
	//	������ �������
	Timer::Resume();

	shell("   finish change Video Mode;\n");
}
//	�������� ���-�� ������ � �������
float Shell::FPS(void) const
{
	return m_Container->m_FPSometer.FPS();
}
//	������������ ���������� �������
bool Shell::CheckExistInstance(void)
{
STACK_GUARD("Shell::CheckExistInstance");
	m_Mutex = CreateMutex(0,false,"CheckExistInstanceForParadiseCracked");
	if(GetLastError() == ERROR_ALREADY_EXISTS) return false;

	return true;
}

/*
//	-------------- ����� ���������� ���������� -----------
void Shell::DebugInfo(void)
{
	static struct tm *pSysTime;

//	DebugInfo::Add(5,5,"Fps: %f",FPSometer::GetFPS());
	DebugInfo::Add(35,5,"% 4d fps",(int)ceil(FPSometer::GetFPS()));
//	shell("fps: %0.6f\n",FPSometer::GetFPS());
//	DebugInfo::Add(22,20,"Time: %f",Timer::GetSeconds());
	//	�������� � ������
	//	��������� ���� + ����������
//	DebugInfo::Add(D3DKernel::ResX()-135,D3DKernel::ResY()-20,"Mouse: (%0.0f,%0.0f)",Input::MouseState().x,Input::MouseState().y);
//	DebugInfo::Add(D3DKernel::ResX()-135,D3DKernel::ResY()-40,"B1:%d B2:%d B3:%d",Input::MouseState().LButtonState,Input::MouseState().RButtonState,Input::MouseState().MButtonState);
//	DebugInfo::Add(D3DKernel::ResX()-135,D3DKernel::ResY()-60,"M:%d K:%d",Input::Mouse()->IsLock(),Input::Keyboard()->IsLock());
	//	��������� �����
	pSysTime = Timer::GetSysTime();
	DebugInfo::Add(930,5,"%0.2d:%0.2d:%0.2d",pSysTime->tm_hour,pSysTime->tm_min,pSysTime->tm_sec);
	//	���������� �� ���������
/*	DebugInfo::Add(300,5,"TexNum: %ld",TextureMgr::Instance()->GetTexNum());
	DebugInfo::Add(300,25,"SysMem: %ld",TextureMgr::Instance()->GetTexSysMem());
	DebugInfo::Add(300,45,"VidMem: %ld",TextureMgr::Instance()->GetTexVidMem());*/
	//	��������� ����������
/*	for(int i=0;i<16;i++)
	{
		for(int j=0;j<16;j++)
		{
			DebugInfo::Add(200+j*30,20+i*22,"%ld",Input::KeyState(i*16+j));
		}
	}*/
	//	��������� ����
//	DebugInfo::Add(200,500,"[%0.4f][%0.4f][%0.4f]",Input::MouseState().dx,Input::MouseState().dy,Input::MouseState().dz);
/*
}


void Shell::DebugGammaRender(void)
{
	static DDGAMMARAMP m_GammaRamp;
	HDC hdc;

	m_GammaRamp = GammaControl::GammaRamp();

	D3DKernel::GetBB()->GetDC(&hdc);

	SelectObject(hdc,GetStockObject(WHITE_PEN));

	MoveToEx(hdc,0,0+D3DKernel::ResY(),NULL);
	for(int i=1;i<256;i++)
	{
		float y = ToScreenY((float)(m_GammaRamp.red[i])*256/65535);
		LineTo(hdc,ToScreenX(((float)i*256/256)),-y+D3DKernel::ResY());
	}

	D3DKernel::GetBB()->ReleaseDC(hdc);
}

*/
//	������������� DataMgr
void Shell::InitDataMgr(void)
{
STACK_GUARD("Shell::InitDataMgr");

	//	�������������� DataMgr
	DataMgr::Initialize();
	//	��������� ���� � �������� �� ���������
	DataMgr::ReadPacks("Data");
	//	��������� ���� �� ���������� ����
	const std::string mod_name = Options::GetString("game.mod");
#if FIX
	if(mod_name.size())
	{
		//	��������� �� ������������ ���������� � ����
		if(Mod(mod_name).isValid())
		{
			DataMgr::ReadPacks((ModMgr::m_modPath+mod_name).c_str(),true);
		}
	}
#endif
	//	��������� ���� �� ��������� ��������� � �������
	const char* path = Options::Registry()->Var("Packs Path").GetString().c_str();
	const char* next;
	char buff[MAX_PATH];

	while(*path)
	{
		int len = strlen(path);
		if(next = strchr(path,';')) len -= strlen(next);
		strncpy(buff,path,len);
		path += len;
		if(len && (buff[len-1] == '/') || (buff[len-1] == '\\')) len--;
		buff[len] = 0;
		DataMgr::ReadPacks(buff);
		if(next) path++;
	}
}
//	����� � ��������� ����
void Shell::DoModal(void)
{
STACK_GUARD("Shell::DoModal");
	if(m_IsModalMode) throw CASUS("������� ����� � ��������� ���� ������ ���.");
	m_IsModalMode = true;
	while(!m_Exit) (*this.*m_Loop)();
	m_Exit = false;
	m_IsModalMode = false;
}
//	���������: �������� �� ��������� �����
bool Shell::IsModal(void) const
{
	return m_IsModalMode;
}
//	������� ��������� �� ������ ����������
DDFrame* Shell::Frame(void)
{
	return m_Container->m_Frame;
}
//	�������� ��������� �� GraphPipe
GraphPipe* Shell::GetGraphPipe(void) const
{
	return m_Container->m_GraphPipe;
}


