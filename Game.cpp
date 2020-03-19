/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   12.04.2000

************************************************************************/
#include "Precomp.h"
#include "Options/Options.h"
#include "Common/GraphPipe/GraphPipe.h"
#include "GameLevel/LongShot.h"
#include "Interface/Interface.h"
//#include "common/slib/sound.h"
#include "gamelevel/grid.h"
#include "gamelevel/gamelevel.h"
#include "Common/3dEffects/EffectManager.h"
#include "Interface/ScreenShot.h"
#include "Interface/graphmgr.h"
#include "logic2/GameLogic.h"
#include "common/saveload/saveload.h"
#include "Skin/Shadow.h"
#include "Game.h"
#include "IWorld.h"
#include "sound/ISound.h"
#include "common/utils/profiler.h"
#include "interface/screens.h"
#include "interface/gamescreen.h"
#include "common/piracyControl/piracycontrol.h"
//---------- Лог файл ------------
//#define _DEBUG_SHELL
#ifdef _DEBUG_SHELL
CLog game_log;
#define game	game_log["game.log"]
#else
#define game	/##/
#endif

//	внешние переменные
extern bool _OldRender;				//	что-то связанное с T&L оптимизацией

//	------------- Внутрение объекты ---------------


namespace Game	
{
	//	необходимо переинициализировать при смене Video Mode
	GraphPipe *m_pGraphPipe = 0;
	//	управление поведением уровня
    bool m_GameLevelRender = false;			//	флаг отрисовки уровня
    bool m_GameLevelUpdate = false;			//	флаг обновления уровня
}

namespace Game	
{
	std::string m_CurGameLevel;
	//	игровое время
	float m_fGameTime = 0;
}

//	--------------------- Функции -------------------------------------
namespace Game
{
	void InitScene(void);
	void UnloadGameLevel(void);
}
//	--------------------- Функции -------------------------------------
///////////////////////////////////////////////////////////////////////
//	переинициализация объектов при смене видео режима
void Game::Tune(void)
{
STACK_GUARD("Game::Tune");
	if(Interface::IsInitialized()) Interface::Instance()->OnChangeVideoMode();
	//	инициализация сцены
	InitScene();
	if(IWorld::Get()->GetLevel())
	   IWorld::Get()->GetLevel()->LinkShaders(m_pGraphPipe);
	game("--- GAME TUNE ---\n");
	//	инициализация теней
	Shadows::UpdateOnModeChange();
}

  extern float EffectsVol;
  extern float AmbientVol;
  extern float MusicVol;
  extern float VoicesVol;
//	инициализация игры - один раз
//#include "logic2/questengine.h"

void Game::Init(GraphPipe *pGraphPipe)
{
STACK_GUARD("Game::Init");

///Grom::testing
//   QuestEngine::ChangeEpisode(0);
///Grom::testing
	//--------------------------------------
	_OldRender = !Options::GetInt("system.video.tl_optimization");
	//--------------------------------------
	m_pGraphPipe = pGraphPipe;
	//--------------------------------------
	//Инициализация игрового мира
	IWorld::Get()->Init(m_pGraphPipe);
	
	point3 pos(m_pGraphPipe->GetCam()->GetPos());
    point3 front(m_pGraphPipe->GetCam()->GetFront());
    point3 up(m_pGraphPipe->GetCam()->GetUp());
	
	// --------------------------------------------------
	// инициализация теней
	Shadows::UpdateOnModeChange();
//	//	инициализируем интерфейс
//	Interface::Instance();
	// инициализация сцены
	InitScene();
  //   
		float vol=Options::GetInt("game.sound.effectsvol");
    EffectsVol=50*20*log(vol/100.f);
	ISound::instance()->setVolume(ISound::cEffects,vol*0.01f);
    vol=Options::GetInt("game.sound.ambientvol");
    AmbientVol=50*20*log(vol/100.f);
	ISound::instance()->setVolume(ISound::cAmbient,vol*0.01f);
    vol=Options::GetInt("game.sound.musicvol");
    MusicVol=50*20*log(vol/100.f);
 	ISound::instance()->setVolume(ISound::cThemes,vol*0.01f);
    vol=Options::GetInt("game.sound.voicesvol");
    VoicesVol=50*20*log(vol/100.f);
 	ISound::instance()->setVolume(ISound::cSpeech,vol*0.01f);



}

void Game::Close(void)
{
STACK_GUARD("Game::Close");
	IWorld::Get()->Close();
	//	освобождаем все ресурсы, используемые интерфейсом
	Interface::Instance()->Release();	
}


bool Game::RenderEnabled()
	{
STACK_GUARD("Game::RenderEnabled");
	return m_GameLevelRender;
	}

//	передача управления игре
void Game::Tick(void)
{
STACK_GUARD("Game::Tick");
	static float tGLevel=Timer::GetSeconds();
	float tSub;

	//	-----------------------------------------------
	tSub = Timer::GetSeconds()-tGLevel;
	tGLevel = Timer::GetSeconds();
	//	-----------------------------------------------
	//	управление на интерфейс
		{
     CodeProfiler CP("gui.tick");
	   Interface::Instance()->Tick();
		}
	if(m_GameLevelUpdate)
    {
     STACK_GUARD("GamelevelUpdate");
		//	-----------------------------------------------
			 IWorld::Get()->Tick();
				 {
				 CodeProfiler CP("level.tick");
				 IWorld::Get()->GetLevel()->UpdateMarks(Timer::GetSeconds());
				 IWorld::Get()->GetLevel()->UpdateObjects(Timer::GetSeconds());
				 IWorld::Get()->GetCamera()->Update(Timer::GetSeconds());
				 
				 IWorld::Get()->GetEffects()->NextTick(Timer::GetSeconds());
				 IWorld::Get()->GetEffects()->Update();
				 //	-----------------------------------------------
				 m_fGameTime += tSub;
				 }
    }
}

//  Обработка Input'а
extern bool DoMulti,DoWire;
bool ProcessCamera=true;
float KEYSPEED=1;
float MOUSESPD=1;
void Game::ProcessInput(void)
  {
STACK_GUARD("Game::ProcessInput");
  GameLevel *GLevel=IWorld::Get()->GetLevel();

  static float f,n,fov; //информация о камере
  static float LastTime=0;
  float DifTime=Timer::GetSeconds()-LastTime;
  LastTime+=DifTime;
  //управление камерой 
  float dx,dy,dz,mx,my;
  dx=Input::MouseState().dx/D3DKernel::ResX();
  dy=Input::MouseState().dy/D3DKernel::ResY();
  dz=Input::MouseState().dz;
  mx=Input::MouseState().x;
  my=Input::MouseState().y;

	//проверка можно ли двигаться
  bool BlockKeys = !Interface::Instance()->IsCameraMoveable();
  bool _shift=Input::KeyState(DIK_RSHIFT)||Input::KeyState(DIK_LSHIFT);
  bool _rmb=Input::MouseState().RButtonState;
  bool _lmb=Input::MouseState().LButtonState;
  bool _mmb=Input::MouseState().MButtonState;
  bool _up=Input::KeyState(DIK_UPARROW);
  bool _down=Input::KeyState(DIK_DOWNARROW);
  bool _right=Input::KeyState(DIK_RIGHTARROW);
  bool _left=Input::KeyState(DIK_LEFTARROW);

		if(_rmb||_mmb)  Input::Mouse()->LockPosition();
		else            Input::Mouse()->UnlockPosition();

	if(ProcessCamera && !BlockKeys)
		{
		Camera *Cam=m_pGraphPipe->GetCam();
		//горизонтальное перемещение
		if(_mmb)            Cam->Move(MOUSESPD/30*point3(200*dx,-200*dy,0));
		if(_right&&!_shift) Cam->Move(DifTime*KEYSPEED*point3(15,0,0));
		if(_left&&!_shift)  Cam->Move(DifTime*KEYSPEED*point3(-15,0,0));
		if(_up&&!_shift)    Cam->Move(DifTime*KEYSPEED*point3(0,15,0));
		if(_down&&!_shift)  Cam->Move(DifTime*KEYSPEED*point3(0,-15,0));
		if(mx<2)            Cam->Move(DifTime*MOUSESPD*point3(-10,0,0));
		if(my<2)            Cam->Move(DifTime*MOUSESPD*point3(0,10,0));
		if(mx>D3DKernel::ResX()-3) Cam->Move(DifTime*MOUSESPD*point3(10,0,0));
		if(my>D3DKernel::ResY()-3) Cam->Move(DifTime*MOUSESPD*point3(0,-10,0));
		
		//изменения угла к горизонту
		unsigned cflag=_shift?Camera::SELF:Camera::AIMSPOT;
		if(Input::KeyState(DIK_PGUP))  Cam->Rotate(DifTime*KEYSPEED,cflag|Camera::RIGHT);
		if(Input::KeyState(DIK_PGDN))  Cam->Rotate(-DifTime*KEYSPEED,cflag|Camera::RIGHT);
		//вращение камеры
		if(_right&&_shift)   Cam->Rotate(DifTime*KEYSPEED*2,cflag|Camera::Z);
		if(_left&&_shift)    Cam->Rotate(-DifTime*KEYSPEED*2,cflag|Camera::Z);
		//два предыдущих вместе
		if(_rmb&&!_lmb&&!_mmb)
		{
			Cam->Rotate(dx*MOUSESPD,cflag|Camera::Z);
			Cam->Rotate(dy*MOUSESPD,cflag|Camera::RIGHT);
		}
		//приближение/удаление
		if(_rmb&&_lmb)     Cam->Move(MOUSESPD/30*point3(0,0,-100*(dx+dy)));
		if(_up&&_shift)    Cam->Move(DifTime*KEYSPEED*point3(0,0,15));
		if(_down&&_shift)  Cam->Move(DifTime*KEYSPEED*point3(0,0,-15));
		if(dz!=0.f) 			 Cam->Move(DifTime*MOUSESPD*point3(0,0,dz*0.2));
	}
	if(Input::KeyFront(DIK_SCROLL))
		if(!_shift)
			DoMulti=!DoMulti; else DoWire=!DoWire;
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//
		// EFFECTS
		//
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/*
		
		EffectManager *m_pEffectManager=IWorld::Get()->GetEffects();
		if(Input::KeyFront(DIK_1))
		{
			m_pEffectManager->CreateAmbientEffect("small_fountain", 
				point3(10.0, 10.0, 1.0), 
				point3(0.0, 0.0, .0));
		}
		
		if(Input::KeyFront(DIK_2))
		{
			m_pEffectManager->CreateHitEffect("hit_large_v_explosion", 
				point3(0.0, 0.0, 1.0), 
				point3(5.0, 5.0, 2.0),
				point3(5.0, 5.0, 0.0),
				1.0);
		}
		if(Input::KeyFront(DIK_3))
		{
			m_pEffectManager->CreateHitEffect("hit_huge_v_explosion", 
				point3(0.0, 0.0, 1.0), 
				point3(5.0, 5.0, 2.0),
				point3(5.0, 5.0, 0.0),
				1.0);
		}
		if(Input::KeyFront(DIK_4))
		{
			m_pEffectManager->CreateHitEffect("hit_discharge", 
				point3(0.0, 0.0, 1.0), 
				point3(5.0, 5.0, 1.0),
				point3(5.0, 5.0, 0.0),
				1.0);
		}
		if(Input::KeyFront(DIK_5))
		{
			m_pEffectManager->CreateHitEffect("hit_discharge", 
				point3(0.0, 0.0, 1.0), 
				point3(5.0, 5.0, 1.0),
				point3(5.0, 5.0, 0.0),
				1.0);
		}
		if(Input::KeyFront(DIK_6))
		{
			m_pEffectManager->CreateHitEffect("hit_small_explosion", 
				point3(5.0, 5.0, 1.0), 
				point3(10.0, 10.0, 2.0),
				point3(10.0, 10.0, 1.0),
				1.0);
		}
		if(Input::KeyFront(DIK_7))
		{
			m_pEffectManager->CreateHitEffect("hit_large_v_explosion", 
				point3(5.0, 5.0, 1.0), 
				point3(10.0, 10.0, 2.0),
				point3(10.0, 10.0, 1.0),
				1.0);
		}
		if(Input::KeyFront(DIK_8))
		{
			m_pEffectManager->CreateHitEffect("hit_shield", 
				point3(0.0, 0.0, 1.0), 
				point3(4.5, 4.5, 1.0),
				point3(4.5, 4.5, 0.0),
				1.0);
		}
		if(Input::KeyFront(DIK_9))
		{
			m_pEffectManager->CreateMovableEffect("new_shield", 
				point3(5.0, 5.0, 0.0));
		}
		if(Input::KeyFront(DIK_0))
		{
			m_pEffectManager->CreateMovableEffect("new_shield_big", 
				point3(10.0, 10.0, 0.0));
		}
		*/
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		GameLogic::GetInst()->Think();
}

  //	отрисовка всех графических объектов

void Shitit(int i);
		void DrawIt();
void Game::Render(void)
{
STACK_GUARD("Game::Render");
	//	game("--- Game Render ---\n");
  CodeProfiler CP("game.render");
	//	чистим экранные буффера
	D3DKernel::GetD3DDevice()->Clear(0,NULL,D3DCLEAR_TARGET,0,1.0f,0);
  	D3DKernel::GetD3DDevice()->Clear(0,NULL,D3DCLEAR_ZBUFFER,0,1.0f,0);
	//	отрисовываем 3D элементы
	D3DKernel::GetD3DDevice()->BeginScene();
	m_pGraphPipe->StartFrame();
	if(m_GameLevelRender)
	{

		{
		STACK_GUARD("Game::Render::LevelRender");
		IWorld::Get()->GetLevel()->Draw(m_pGraphPipe,false);
		m_pGraphPipe->Chop(&IWorld::Get()->GetLevel()->Env);
		}
		
		IWorld::Get()->Draw();
		IWorld::Get()->GetLevel()->LevelObjects.Draw(m_pGraphPipe,false);
		
		{
		STACK_GUARD("Game::Render::GameLogic");
		GameLogic::GetInst()->Draw();
		}
		
		IWorld::Get()->GetLevel()->DrawMarks();
		IWorld::Get()->GetLevel()->Draw(m_pGraphPipe,true);
		
		{
		STACK_GUARD("Game::Render::DrawShadows");
		// Отрисовка теней
		Shadows::DrawShadows (m_pGraphPipe);
		}
		//отрисовка эффектов
		{
		STACK_GUARD("Game::Render::DrawEffects");
		IWorld::Get()->GetEffects()->DrawEffects();
		}
		//Shitit(!Input::KeyState(DIK_SPACE));
	}
	//	game("     ....RenderGLevel\n");
	////	воспроизведение интерфейса
	StatesManager::SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
		{
		STACK_GUARD("Game::Render::Interface3D");
	Interface::Instance()->Render3D();
		}
	//	камера
	if(m_pGraphPipe->GetCam()->IsViewChanged())
    {
    point3 pos(m_pGraphPipe->GetCam()->GetPos());
    point3 front(m_pGraphPipe->GetCam()->GetFront());
    point3 up(m_pGraphPipe->GetCam()->GetUp());
	point3 right(m_pGraphPipe->GetCam()->GetRight());
	ISound::instance()->setCamera(front,up,right,pos);
   }
	//fixme:   testing. don't delete this!(Grom)
	{
		DrawIt();
	} 
  m_pGraphPipe->EndFrame();
	D3DKernel::GetD3DDevice()->EndScene();

	//	отрисовываем 2D элементы
		{
		STACK_GUARD("Game::Render::Interface2D");
	Interface::Instance()->Render2D();
		}
}
void SetupPixelFog(DWORD dwColor, DWORD dwMode)
  {
STACK_GUARD("SetupPixelFog");
  float fStart = 20.f,    // for linear mode     
    fEnd   = 30.f,
    fDensity = 0.12;  // for exponential modes    
  // Enable fog blending.
  StatesManager::SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE); 
  // Set the fog color.
  StatesManager::SetRenderState(D3DRENDERSTATE_FOGCOLOR, dwColor);    
  // Set fog parameters.
  if(D3DFOG_LINEAR == dwMode)
    {
    StatesManager::SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, dwMode);
    StatesManager::SetRenderState(D3DRENDERSTATE_FOGSTART, *(DWORD *)(&fStart));
    StatesManager::SetRenderState(D3DRENDERSTATE_FOGEND,   *(DWORD *)(&fEnd));
    }  
  else
    {
    StatesManager::SetRenderState(D3DRENDERSTATE_FOGSTART, *(DWORD *)(&fStart));
    StatesManager::SetRenderState(D3DRENDERSTATE_FOGEND,   *(DWORD *)(&fEnd));
    StatesManager::SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, dwMode);
    StatesManager::SetRenderState(D3DRENDERSTATE_FOGDENSITY, *(DWORD *)(&fDensity));
    } 
  }

void Game::InitScene(void)
{
STACK_GUARD("Game::InitScene");
//SetupPixelFog(RGB_MAKE(92,93,103), D3DFOG_EXP);

	StatesManager::SetRenderState(D3DRENDERSTATE_COLORKEYENABLE,FALSE);
	StatesManager::SetRenderState(D3DRENDERSTATE_COLORKEYBLENDENABLE,FALSE);
	//StatesManager::SetRenderState(D3DRENDERSTATE_NORMALIZENORMALS    ,TRUE);
  StatesManager::SetRenderState(D3DRENDERSTATE_EXTENTS ,FALSE);
  StatesManager::SetRenderState(D3DRENDERSTATE_LIGHTING ,FALSE);
  
for(int i=0;i<2;i++)
  {
	StatesManager::SetTextureStageState(i, D3DTSS_MINFILTER, D3DTFN_LINEAR);
	StatesManager::SetTextureStageState(i, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
	StatesManager::SetTextureStageState(i, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
	
	StatesManager::SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT );
	StatesManager::SetTextureStageState(i, D3DTSS_COLORARG1,D3DTA_TEXTURE );
	StatesManager::SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	StatesManager::SetTextureStageState(i, D3DTSS_ALPHAARG1,D3DTA_TEXTURE );
	
	StatesManager::SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StatesManager::SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	//StatesManager::SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	//StatesManager::SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
  }
	StatesManager::SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StatesManager::SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	
	D3DMATERIAL7 d3dMaterial;
    ZeroMemory(&d3dMaterial, sizeof(d3dMaterial));
    d3dMaterial.dcvDiffuse.r = 0.f;
    d3dMaterial.dcvDiffuse.g = 0.f;
    d3dMaterial.dcvDiffuse.b = 0.f;
    d3dMaterial.dcvDiffuse.a = 0.f;
    d3dMaterial.dcvAmbient.r = 1;
    d3dMaterial.dcvAmbient.g = 1;
    d3dMaterial.dcvAmbient.b = 1;
    d3dMaterial.dcvSpecular.r = 0.f;
    d3dMaterial.dcvSpecular.g = 0.f;
    d3dMaterial.dcvSpecular.b = 0.f;
    d3dMaterial.dcvSpecular.b = 0.f;
    d3dMaterial.dvPower = 10.0f;     
    D3DKernel::GetD3DDevice()->SetMaterial(&d3dMaterial);    
    StatesManager::SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
    StatesManager::SetRenderState(D3DRENDERSTATE_LIGHTING,FALSE);
    StatesManager::SetRenderState( D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
	//StatesManager::SetRenderState( D3DRENDERSTATE_WRAP0 , D3DWRAPCOORD_0|D3DWRAPCOORD_1);
}

//*********************** Управление выводом уровня *******************************//
void Game::SetGameLevelRender(bool render)
{
	m_GameLevelRender = render;
}

void Game::SetGameLevelUpdate(bool update)
{
	m_GameLevelUpdate = update;
}


namespace DirtyLinks
	{
	std::string GetStrRes(const std::string& rid); 
	}
void Game::LoadGameLevel(const char *pLevelName)
{
STACK_GUARD("Game::LoadGameLevel");
  //Interface::Instance()->ProBar()->SetTitle("Смена уровня");

	IWorld::Get()->ChangeLevel(pLevelName);
	//начнем игру заново
  //Interface::Instance()->ProBar()->SetTitle("начало новой игры");
	GameLogic::GetInst()->BegNewGame();
  Interface::Instance()->ProBar()->SetTitle(DirtyLinks::GetStrRes("ld_levelstart").c_str());
	IWorld::Get()->GetLevel()->Start();
  //Interface::Instance()->ProBar()->SetTitle("все");
	//Shadows::OptionsChanged();
}

void Game::UnloadGameLevel(void)
{
STACK_GUARD("Game::UnloadGameLevel");
	SetGameLevelRender(false);
	SetGameLevelUpdate(false);
	//	выгрузим уровень
	IWorld::Get()->ChangeLevel("");
}

//******************** ПОЛУЧЕНИЕ СКРИНШОТА ********************************//
void Game::MakeScreenShot(void)
{
STACK_GUARD("Game::MakeScreenShot");
	static int counter = 0;
	char lpBuff[50];

	sprintf(lpBuff,"scr%0.2d.bmp",counter++);
	GammaScreenShot(D3DKernel::GetPS(),lpBuff);
}

//************************************************************************//
void Game::AdjustHW(void)
{
STACK_GUARD("Game::AdjustHW");
 	if(IWorld::Get()->GetLevel())
 IWorld::Get()->GetLevel()->LinkShaders(m_pGraphPipe);
}

void Game::SetEffectsQuality(float q)
{
STACK_GUARD("Game::SetEffectsQuality");
	IWorld::Get()->GetEffects()->SetEffectsQuality(q);
}

void Game::MakeSaveLoad(const char* pFileName,bool bSave)
{
STACK_GUARD("Game::MakeSaveLoad");
	StdFile  file(pFileName, bSave?"wb":"rb");

	if(file.IsOk())
	{
		Storage  store(&file, bSave?Storage::SM_SAVE:Storage::SM_LOAD);
		
		Interface::Instance()->MakeSaveLoad(store);
		IWorld::Get()->MakeSaveLoad(store);
		GameLogic::GetInst()->MakeSaveLoad(store);
		//Shadows::OptionsChanged();
		if(!bSave)
		{
			IWorld::Get()->GetLevel()->Start();
		}
	}
	else throw CASUS("MakeSaveLoad: указанный Файл невозможно использовать.");
}
//	предварительная настройка интерфейса
void Game::PriorInterfaceInitialize(void)
{
STACK_GUARD("Game::PriorInterfaceInitialize");
	Interface::PriorInitialize();
}
//	создание игрового интерфейса
void Game::CreateInterface(void)
{
STACK_GUARD("Game::CreateInterface");
	Interface::Instance()->RunMenuFlow();
}
//	проигрывание мультика
void Game::PlayBink(const char *pFileName)
{
STACK_GUARD("Game::PlayBink");
	Interface::Instance()->PlayBink(pFileName);
}
