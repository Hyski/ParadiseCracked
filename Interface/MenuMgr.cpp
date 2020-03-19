/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#include "Precomp.h"
#include "../sound/ISound.h"
#include "../Common/Shell/Shell.h"
#include "../IWorld.h"
#include "../Options/Options.h"
#include "../Common/CmdLine/CmdLine.h"
#include "../Common/SurfaceMgr/SurfaceMgr.h"
#include "../Common/DataMgr/DataMgr.h"
#include "../Common/DataMgr/TxtFile.h"
//	для строковых ресурсов
#include "../Logic2/logicdefs.h"
#include "../Logic2/DirtyLinks.h"
//	----------------------
#include "../Game.h"
#include "DXImageMgr.h"
#include "WidgetSoundMgr.h"
#include "WidgetFactory.h"
#include "MainMenu.h"
#include "GameScreen.h"
#include "LoadingScreen.h"
#include "OptionsScreen.h"
#include "Interface.h"
#include "MouseCursor.h"
#include "Screens.h"
#include "Console.h"
#include "GraphMgr.h"
#include "Text.h"
#include "MenuMgr.h"

#include <sstream>

//---------- Лог файл ------------
#ifdef _HOME_VERSION
CLog menu_mgr_log;
#define menu_mgr	menu_mgr_log["menu_mgr.log"]
#else
#define menu_mgr	/##/
#endif

namespace MenuMgrGameController
{
	void LoadLevel(const char* level_name);
	void UnloadLevel(void);
	void SaveLoadGame(const char* file_name,bool save);
}

//***************************************************************//
//	class DXView 
class DXView : public WidgetView
{
public:
	DXView();
	virtual ~DXView();
public:
	void Render(Widget *pWidget);
	void BltFast(int x,int y,LPDIRECTDRAWSURFACE7 pSurface);
	void BltFast(int x,int y,LPDIRECTDRAWSURFACE7 pSurface,int left,int top,int right,int bottom);
	void SaveBltFast(int x,int y,LPDIRECTDRAWSURFACE7 pSurface,int src_x,int src_y,int width,int height);
};

//***************************************************************//
//	class External
class External : public MenuMgr::ic_State
{
private:
	class SoundOscillator
	{
	private:
		const char* m_ScriptPath; 
		std::vector<Widget::Player::Sample*> m_Samples;
		float m_SleepTime;
		float m_SecondsMark;
		int m_MaxDelay;
	public:
		SoundOscillator(const char* script_path) : m_ScriptPath(script_path),m_SleepTime(10),m_SecondsMark(0),m_MaxDelay(40)
		{
			// читаем xls'ку
			TxtFile script(DataMgr::Load(m_ScriptPath));
			
			DataMgr::Release(m_ScriptPath);

			srand((unsigned int)time(0));

			for(int i=1;i<script.SizeY();i++)
			{
				if(script.SizeX(i))
				{
					if(!script(i,0).size()) break;
					m_Samples.push_back(WidgetSoundMgr::Instance()->CreateSample(script(i,0).c_str(),"COMMON"));
				}
			}
			//	считываем время задержки
			if(script.SizeY() && script.SizeX(0)>1) m_MaxDelay = atoi(script(0,1).c_str());
		}
		~SoundOscillator()
		{
			for(int i=m_Samples.size()-1;i>=0;i--)
				m_Samples[i]->Release();
		}
	public:
		void Tick(void)
		{
			if(m_Samples.size())
			{
				if(!m_SecondsMark)
				{
					m_SecondsMark = Timer::GetSeconds();
					return;
				}
				else
				{
					if((Timer::GetSeconds()-m_SecondsMark)>m_SleepTime)
					{
						WidgetSoundMgr::Instance()->GetPlayer()->Play(m_Samples[rand()%m_Samples.size()]);
						m_SecondsMark = Timer::GetSeconds();
						m_SleepTime = 10+rand()%m_MaxDelay;
					}
				}
			}
		}
	};
private:
	SoundOscillator m_NoiseOscillator;
	SoundOscillator m_ThunderOscillator;
public:
	External(MenuMgr* mm) : ic_State(mm),m_NoiseOscillator("scripts/interface/noise.txt"),m_ThunderOscillator("scripts/interface/thunder.txt")
	{
		menu_mgr("\n...enter to External constructor;\n");
		//	заглушаем звуковой канал cSpeech
		ISound::instance()->muteChannel(ISound::cSpeech,true);
		//	запускаем звук
		m_MenuMgr->PlayAmbientSound();
		menu_mgr("   ...play ambient sound;\n");
		WidgetSoundMgr::Instance()->GetPlayer()->Play(m_MenuMgr->RainSound());
		menu_mgr("   ...play rain sound;\n");
		//	необходимо переключится в 800x600
		m_MenuMgr->CheckVideoMode(MenuMgr::CVM_FOR_MAINMENU);
		menu_mgr("   ...check video mode;\n");
		//	создаем необходимые диалоги
		m_MenuMgr->Insert(new MainMenu());
		m_MenuMgr->Insert(new VideoMenu());
		m_MenuMgr->Insert(new CreditsMenu());
		m_MenuMgr->Insert(new GameMenu());
		m_MenuMgr->Insert(new OptionsMenu());
		m_MenuMgr->Insert(new LoadingMenu());
		menu_mgr("   ...insert & create all menu;\n");
		//	показываем первый из диалогов
		m_MenuMgr->ShowDialog(MainMenu::m_pDialogName);
		//	сбрасываем все модификаторы
		m_MenuMgr->ResetModifiers();
		//	показываем версию приложения
		m_MenuMgr->Version()->SetVisible(true);
		menu_mgr("...exit from External constructor;\n\n");
	}
	virtual ~External()
	{
		//	уничтожаем ранее созданные диалоги
		m_MenuMgr->Delete(MainMenu::m_pDialogName);
		m_MenuMgr->Delete(VideoMenu::m_pDialogName);
		m_MenuMgr->Delete(GameMenu::m_pDialogName);
		m_MenuMgr->Delete(OptionsMenu::m_pDialogName);
		m_MenuMgr->Delete(LoadingMenu::m_pDialogName);
	}
private:
	//	передача управления от дочерних окон
	bool OnChange(Widget* pChild)
	{
		if(!strcmp(pChild->Name(),MainMenu::m_pDialogName))
		{
			OnMainMenu(static_cast<MainMenu*>(pChild));
			return true;
		}
		if(!strcmp(pChild->Name(),GameMenu::m_pDialogName))
		{
			OnGameMenu(static_cast<GameMenu*>(pChild));
			return true;
		}
		if(!strcmp(pChild->Name(),OptionsMenu::m_pDialogName))
		{
			OnOptionsMenu(static_cast<OptionsMenu*>(pChild));
			return true;
		}
		if(!strcmp(pChild->Name(),VideoMenu::m_pDialogName))
		{
			OnVideoMenu(static_cast<VideoMenu*>(pChild));
			return true;
		}

		if(!strcmp(pChild->Name(),CreditsMenu::m_pDialogName))
		{
			OnCreditsMenu(static_cast<CreditsMenu*>(pChild));
			return true;
		}

		if(!strcmp(pChild->Name(),LoadingMenu::m_pDialogName))
		{
			OnLoadingMenu(static_cast<LoadingMenu*>(pChild));
			return true;
		}
		
		return false;
	}
	//	передача управления 
	void OnTick(void)
	{
		m_NoiseOscillator.Tick();
		m_ThunderOscillator.Tick();
		//	обработка клавиатуры
		if(Input::KeyFront(DIK_F10))
		{//	быстрый выход  из игры
			if(m_MenuMgr->IsQBFree())
			{
				if(m_MenuMgr->QuestionBox(m_MenuMgr->GetStrResourse("mm_quit").c_str()))
				{
					m_MenuMgr->ExitFromProgram();
				}
			}
		}
	}
	//	обработка событий от MainMenu
	void OnMainMenu(MainMenu *pMenu)
	{
		switch(pMenu->GetEvent())
		{
		case MainMenu::AE_GAME:
			m_MenuMgr->ShowDialog(GameMenu::m_pDialogName);
			break;
		case MainMenu::AE_OPTIONS:
			m_MenuMgr->ShowDialog(OptionsMenu::m_pDialogName);
			break;
		case MainMenu::AE_CINEMATICS:
			m_MenuMgr->ShowDialog(VideoMenu::m_pDialogName);
			break;
		case MainMenu::AE_CREDITS:
			m_MenuMgr->ShowDialog(CreditsMenu::m_pDialogName);
			break;
		case MainMenu::AE_EXIT:
			m_MenuMgr->ExitFromGame();
			break;
		}
	}
	//	обработка событий от MainMenu
	void OnVideoMenu(VideoMenu* pMenu)
	{
		m_MenuMgr->ShowDialog(MainMenu::m_pDialogName);
	}
	//	обработка событий от CreditsMenu
	void OnCreditsMenu(CreditsMenu* pMenu)
	{
		m_MenuMgr->ShowDialog(MainMenu::m_pDialogName);
	}
	//	обновление прогресс бара
	void UpdateLoadingProgressBar(float value) {}
	//	установить заголовок прогресc бара ЗАГРУЗКИ УРОВНЯ
	void SetProgressBarTitle(const char* title) {}
	//	обработка событий от GameMenu 
	void OnGameMenu(GameMenu *pMenu)
	{
		switch(pMenu->GetEvent())
		{
		case GameMenu::AE_NEW:
			//	насильно включаем хелпер в кфгешнике
			Options::Set("game.helper",1);
			//	устанавливаем модификатор на загрузку уровня
			if(CmdLine::IsKey("-test2"))
				m_MenuMgr->LoadLevelModifier()->SetLevelName("test2");
			else
				m_MenuMgr->LoadLevelModifier()->SetLevelName(m_MenuMgr->m_StartLevel);
			m_MenuMgr->LoadLevelModifier()->SetAvailable(true);
			m_Destroy = true;
			break;
		case GameMenu::AE_LOAD:
			m_MenuMgr->ShowDialog(LoadingMenu::m_pDialogName);
			pMenu->SetVisible(true);
			break;
		case GameMenu::AE_MULTIPLAYER:
			break;
		case GameMenu::AE_TRAINING:
			break;
		case GameMenu::AE_BACK:
			m_MenuMgr->ShowDialog(MainMenu::m_pDialogName);
			break;
		}
	}
	//	показать вновь MainMenu
	void OnOptionsMenu(OptionsMenu* pMenu)
	{	
		m_MenuMgr->ShowDialog(MainMenu::m_pDialogName);
	}
	//	обработка событий от LoadingMenu 
	void OnLoadingMenu(LoadingMenu * /*pMenu*/)
	{
		//	проверяем модификатор на загрузку/запись игры
		switch(m_MenuMgr->LoadSaveGameModifier()->Available())
		{
		case MenuMgr::ic_LoadSaveGameModifier::A_LOAD:
			//	производим загрузку игры, для этого переключаем режим игры
			m_Destroy = true;
			break;
		default:
			//	закрываем менюшку
			m_MenuMgr->ShowDialog(GameMenu::m_pDialogName);
		}
	}
	//	загружаем уровень
	void LoadLevel(const char* name)
	{
		//	устанавливаем модификитор на загрузку уровня
		m_MenuMgr->LoadLevelModifier()->SetLevelName(name);
		m_MenuMgr->LoadLevelModifier()->SetAvailable(true);
		m_Destroy = true;
	}
	//	возвращаем режим работы меню
	MenuMgr::ic_State::MODE Mode(void){return MenuMgr::ic_State::M_EXTERNAL;}
};

//***************************************************************//
//	class Internal
class Internal : public MenuMgr::ic_State
{
private: 
	enum ACTION {A_NONE,A_DESTROY_LOADING_SCREEN};
	ACTION m_Action;
public:
	Internal(MenuMgr* mm) : ic_State(mm),m_Action(A_NONE)
	{
		menu_mgr("Enter to Internal constructor;\n");
		//	включаем звуковой канал cSpeech
		ISound::instance()->muteChannel(ISound::cSpeech,false);
		//	останавливаем звук
		m_MenuMgr->StopAmbientSound();
		WidgetSoundMgr::Instance()->GetPlayer()->Stop(m_MenuMgr->RainSound());
		//	проигрываем вступительный мультик
		if(!m_MenuMgr->IsLevelKeyPresent())
		{
			if(m_MenuMgr->LoadLevelModifier()->IsAvailable() && !stricmp(MenuMgr::m_StartLevel,m_MenuMgr->LoadLevelModifier()->LevelName().c_str()))
				Interface::Instance()->PlayBink("e1_1.bik");
		}
		//	необходимо переключится в разрешение из cfg файла
		m_MenuMgr->CheckVideoMode(MenuMgr::CVM_FOR_CFG);
		//	проверка на размеры MenuMgr
		if((D3DKernel::ResX() != m_MenuMgr->GetLocation()->Region()->Width()) ||
		   (D3DKernel::ResY() != m_MenuMgr->GetLocation()->Region()->Height()))
		{
			m_MenuMgr->OnChangeVideoMode();
		}
		//	создаем необходимые диалоги
		m_MenuMgr->Insert(new GameScreen());
		//	проверяем модификаторы
		if(m_MenuMgr->LoadLevelModifier()->IsAvailable() || (m_MenuMgr->LoadSaveGameModifier()->Available()==MenuMgr::ic_LoadSaveGameModifier::A_LOAD))
		{
			menu_mgr("Insert LoadingScreen;");
			m_MenuMgr->Insert(new LoadingScreen());
			m_MenuMgr->ShowDialog(LoadingScreen::m_pDialogName);
		}
		else
		{
			//	показываем первый из диалогов
			m_MenuMgr->ShowDialog(GameScreen::m_pDialogName);
		}
		//	прячем версию приложения
		m_MenuMgr->Version()->SetVisible(false);
		menu_mgr("Exit from Internal constructor;\n");
	}
	virtual ~Internal()
	{
		//	блокируем организацию ввода для всего что не является интерфейсом
		m_MenuMgr->LockInput(true);
		//	выгружаем уровень
		MenuMgrGameController::UnloadLevel();
		//	уничтожаем ранее созданные диалоги
		m_MenuMgr->Delete(GameScreen::m_pDialogName);
	}
private:
	//	передача управления от дочерних окон
	bool OnChange(Widget* pChild)
	{
		if(!strcmp(pChild->Name(),LoadingScreen::m_pDialogName))
		{
			OnLoadingScreen(static_cast<LoadingScreen*>(pChild));
			return true;
		}
		if(!strcmp(pChild->Name(),GameScreen::m_pDialogName))
		{
			OnGameScreen(static_cast<GameScreen*>(pChild));
			return true;
		}

		return false;
	}
	//	передача управления 
	void OnTick(void)
	{
		switch(m_Action)
		{
		case A_DESTROY_LOADING_SCREEN:
			m_MenuMgr->Delete(LoadingScreen::m_pDialogName);
			m_MenuMgr->ShowDialog(GameScreen::m_pDialogName);
			m_Action = A_NONE;
			break;
		}
	}
	//	обновление прогресс бара
	void UpdateLoadingProgressBar(float value)
	{
		static_cast<LoadingScreen*>(WidgetFactory::Instance()->Assert(m_MenuMgr,LoadingScreen::m_pDialogName))->Update(value);
	}
	//	установить заголовок прогресc бара ЗАГРУЗКИ УРОВНЯ
	void SetProgressBarTitle(const char* title)
	{
		static_cast<LoadingScreen*>(WidgetFactory::Instance()->Assert(m_MenuMgr,LoadingScreen::m_pDialogName))->SetTitle(title);
	}
	//	обработать информацию от LoadingScreen  
	void OnLoadingScreen(LoadingScreen *pMenu)
	{
		switch(pMenu->Step())
		{
		case LoadingScreen::S_READY_FOR_LOADING:
			if(m_MenuMgr->LoadLevelModifier()->IsAvailable())
			{//	загружаем уровень
				MenuMgrGameController::LoadLevel(m_MenuMgr->LoadLevelModifier()->LevelName().c_str());
				//	разблокируем организацию ввода для всего что не является интерфейсом
				m_MenuMgr->LockInput(false);
				break;
			}
			if(m_MenuMgr->LoadSaveGameModifier()->Available()==MenuMgr::ic_LoadSaveGameModifier::A_LOAD)
			{//	загружаем сохраненную игру
				MenuMgrGameController::SaveLoadGame(m_MenuMgr->LoadSaveGameModifier()->SaveGameName().c_str(),false);
				//	разблокируем организацию ввода для всего что не является интерфейсом
				m_MenuMgr->LockInput(false);
				break;
			}
			break;
		case LoadingScreen::S_LOADING_DONE:
			m_Action = A_DESTROY_LOADING_SCREEN;
			//	сбрасываем все модификаторы
			m_MenuMgr->ResetModifiers();
			//	сбрасываем содержимое MessageWindow в GameScreen
			//static_cast<GameScreen*>(WidgetFactory::Instance()->Assert(m_MenuMgr,GameScreen::m_pDialogName))->MsgWindow()->Clear();
			break;
		}
	}
	//	обработать информацию от GameScreen  
	void OnGameScreen(GameScreen* pScreen)
	{//	необходимо выйти из игры в главное меню
		//	проверяем модификатор на загрузку/запись игры
		switch(m_MenuMgr->LoadSaveGameModifier()->Available())
		{
		case MenuMgr::ic_LoadSaveGameModifier::A_LOAD:
			//	необходимо прочитать записанную игру
			m_MenuMgr->Insert(new LoadingScreen());
			m_MenuMgr->ShowDialog(LoadingScreen::m_pDialogName);
			break;
		case MenuMgr::ic_LoadSaveGameModifier::A_SAVE:
			//	записываем игру
			MenuMgrGameController::SaveLoadGame(m_MenuMgr->LoadSaveGameModifier()->SaveGameName().c_str(),true);
			//	устанавливаем в окне сообщение
			if(!pScreen->MsgWindow()->IsPulled()) pScreen->MsgWindow()->IncMsgWindow();
			pScreen->MsgWindow()->AddText(m_MenuMgr->GetStrResourse("mm_gamesaved").c_str());
			//	сбрасываем модификаторы
			m_MenuMgr->ResetModifiers();
			break;
		case MenuMgr::ic_LoadSaveGameModifier::A_NONE:
			//	выходим в главное меню
			m_Destroy = true;
			break;
		}
	}
	//	загружаем уровень
	void LoadLevel(const char* name)
	{
		if(name && strlen(name))
		{
			//	устанавливаем модификитор на загрузку уровня
			m_MenuMgr->LoadLevelModifier()->SetLevelName(name);
			m_MenuMgr->LoadLevelModifier()->SetAvailable(true);
			//	создаем экран загрузки
			m_MenuMgr->Insert(new LoadingScreen());
			m_MenuMgr->ShowDialog(LoadingScreen::m_pDialogName);
		}
	}
	//	возвращаем режим работы меню
	MenuMgr::ic_State::MODE Mode(void){return MenuMgr::ic_State::M_INTERNAL;}
};

//***************************************************************//
//	class CutInRelay
class CutInRelay : public MenuMgr::ic_State
{
private:
	MenuMgr::ic_State* m_State;
	enum TYPE {T_EXTERNAL,T_INTERNAL};
	TYPE m_Type;
public:
	CutInRelay(MenuMgr* mm) : ic_State(mm)
	{
		if(m_MenuMgr->IsLevelKeyPresent())
		{
			m_Type = T_INTERNAL;
			m_MenuMgr->LoadLevelModifier()->SetLevelName(CmdLine::Query("-level").c_str());
			m_MenuMgr->LoadLevelModifier()->SetAvailable(true);
			menu_mgr("m_State = new Internal(m_MenuMgr);\n");
			m_State = new Internal(m_MenuMgr);
		}
		else
		{
			m_Type = T_EXTERNAL;
			m_State = new External(m_MenuMgr);
		}
		menu_mgr("Exit from CutInRelay constructor;\n");
	}
	virtual ~CutInRelay()
	{ 
		menu_mgr("Enter to CutInRelay desctructor;\n");
		delete m_State;
		menu_mgr("Exit from CutInRelay desctructor;\n");
	}
private:
	//	передача управления от дочерних окон
	bool OnChange(Widget* pChild) {return m_State->OnChange(pChild);}
	//	передача управления 
	void OnTick(void)
	{
		if(m_State->IsDestroy())
		{//	предыдущее состояние необходимо уничтожить и перейти в новое сотояние
			delete m_State;
			if(m_Type == T_EXTERNAL)
			{
				m_State = new Internal(m_MenuMgr);
				m_Type = T_INTERNAL;
			}
			else
			{
				m_State = new External(m_MenuMgr);
				m_Type = T_EXTERNAL;
			}
		}
		else
		{
			m_State->OnTick();
		}
	}
	//	обновление прогресс бара
	void UpdateLoadingProgressBar(float value) {m_State->UpdateLoadingProgressBar(value);}
	//	установить заголовок прогресc бара ЗАГРУЗКИ УРОВНЯ
	void SetProgressBarTitle(const char* title) {m_State->SetProgressBarTitle(title);}
	//	возвращаем режим работы меню
	MenuMgr::ic_State::MODE Mode(void) {return m_State->Mode();}
	//	загружаем уровень
	void LoadLevel(const char* name) {m_State->LoadLevel(name);}
};

//***************************************************************//
//	class MenuMgr::ic_InterfaceTip
class MenuMgr::ic_InterfaceTip
{
private:
	static const float m_SleepTime;
	static const int m_Color;
	bool m_Activated;
	bool m_TipActivated;
public:
	ic_InterfaceTip();
	virtual ~ic_InterfaceTip();
public:
	void Update(Widget* pWidget);
};

//***************************************************************//
//	class MenuMgr
MenuMgr::Deleter MenuMgr::m_Deleter;
const int MenuMgr::m_MainMenuResX = 800;
const int MenuMgr::m_MainMenuResY = 600;


#if defined(DEMO_VERSION)
		const char* MenuMgr::m_StartLevel = "energy";
#else
		const char* MenuMgr::m_StartLevel = "h_house";
#endif

MenuMgr::ic_MouseCursor* MenuMgr::m_MouseCursor = 0;
MenuMgr::MenuMgr() : Widget("MenuMgr"),m_bLockInput(true),m_AmbientSound(0),
					 m_RainSound(WidgetSoundMgr::Instance()->CreateSample("RAIN"))
#if FIX
					 , m_currentMod(Options::GetString("game.mod"))
#endif					 
{
	menu_mgr("Enter to MenuMgr contstructor;\n");
	//	создаем мышинный курсор
	m_MouseCursor = new ic_MouseCursor();
	//	устанавливаем себя во весь экран
	GetLocation()->Create(0,0,m_MainMenuResX,m_MainMenuResY);
	//	создание модуля реализации графики
	m_pDXView = new DXView();
	//	проверка и установка присутствия консоли в игре
	if(CmdLine::IsKey("-console")) Insert(Console::SetAvailable());
	//	проверка и установка присутствия модуля по поддержки графиков в игре
	if(CmdLine::IsKey("-graph")) GraphMgr::SetAvailable(this);
	//	добавляем типсину
	m_Tip = new ic_Tip(this);
	m_InterfaceTip = new ic_InterfaceTip();
	//	создаем модификаторы
	m_LoadLevelModifier = new ic_LoadLevelModifier();
	m_LoadSaveGameModifier = new ic_LoadSaveGameModifier();
	//	создаем текстовое поле версии приложения
	Insert(m_Version = new Text("version"));
	//	устанавливаем версию приложения
	extern unsigned int major_version;
	extern unsigned int minor_version;
	extern unsigned int build_number;

	std::ostringstream buffer;

	//	собираем описание версии проекта
	buffer << "Ver. " << std::dec << major_version << ".";
	buffer << std::setw(2) << std::setfill('0') << minor_version << ".";
	buffer << std::setw(3) << std::setfill('0') << build_number;

#if FIX
	if(m_currentMod.isValid())
	{//	добаляем описание мода
		buffer << " mod: " << m_currentMod.name();
	}
#endif

	m_Version->SetRegion(5,585,300,599);
	m_Version->SetFont("tip",0x00ff00);
	m_Version->SetText(buffer.str().c_str());
	menu_mgr("...version widget was created;\n");
	//	созадем каталог для хранения сейвов
	OptionsScreen::MakeSavesDir();
	//	создаем режим работы интерфейса
	m_State = new CutInRelay(this);
	menu_mgr("Exit from MenuMgr contstructor;\n");
}

MenuMgr::~MenuMgr()
{
	menu_mgr("Enter to MenuMgr desctructor;\n");
	//	удаляем мышинный курсор
	delete m_MouseCursor;
	//	удаляем состояние работы интерфейса
	delete m_State;
	//	уничтожаем модификаторы
	delete m_LoadLevelModifier;
	delete m_LoadSaveGameModifier; 
	//	удаления поддержки всплывающих подсказок в игре
	delete m_InterfaceTip;
	delete m_Tip;
	//	удаление модуля по поддержки графиков в игре
	if(GraphMgr::IsAvailable()) GraphMgr::Release();
	//	освобождаем ресурсы менеджера звука
	WidgetSoundMgr::Instance()->Release();
	//	удаление модуля реализации графики
	delete m_pDXView;
	menu_mgr("Exit from MenuMgr desctructor;\n");
}

void MenuMgr::Render(void)
{
	Widget::Render(m_pDXView);
	if(m_Version->IsVisible())
		m_Version->Render(m_pDXView);
//	TEST	
	DebugInfo::Add(300,540,"Surfaces: %d",SurfaceMgr::Instance()->Quantity());
//	TEST	
}

void MenuMgr::OnMouse(void)
{
	static VMouseState vms;

	memcpy(&vms,&Input::MouseState(),sizeof(vms));
	Widget::OnMouse(&vms);
}

void MenuMgr::OnKeyboard(void)
{
	static VKeyboardState vks;

	memcpy(&vks,Input::KeyboardState(),sizeof(vks));
	Widget::OnKeyboard(&vks);
}

//	обработка событий от дочерних меню
void MenuMgr::OnChange(Widget* pChild)
{
	if(!m_State->OnChange(pChild))
	{
		if(!strcmp(pChild->Name(),Console::m_pDialogName))
		{
			OnConsole(pChild);
		}
	}
}

//	реагирование на смену видео режима
void MenuMgr::OnChangeVideoMode(void)
{
	//	устанавливаем себя во весь экран
	GetLocation()->Create(0,0,D3DKernel::ResX(),D3DKernel::ResY());
	//	все остальное
	Widget::OnSystemChange();
}

//	показать то или иное меню
void MenuMgr::ShowDialog(const char *pName)
{
	if(Focus()->Get(WidgetFocus::FF_KEYBOARD) != Child(pName))
	{
		WidgetFactory::Instance()->Assert(this,pName)->SetVisible(true);
		Focus()->Set(WidgetFocus::FF_KEYBOARD,WidgetFactory::Instance()->Assert(this,pName));
	}
}
//	обработка сообщений от консоли
void MenuMgr::OnConsole(Widget* pChild)
{
	Widget *pPrevFocus;

	switch(Console::LastEvent())
	{
	case Console::LE_ACTIVATE:
		pPrevFocus = Focus()->Get(WidgetFocus::FF_KEYBOARD);
		Focus()->Set(WidgetFocus::FF_KEYBOARD,pChild);
		if(pPrevFocus) pPrevFocus->SetVisible(true);
		break;
	}
}
//	обработка всего всего
bool MenuMgr::Tick(void)
{
	//	передаем управление состоянию
	m_State->OnTick();
	//	выводим подсказку
	if(m_State->Mode() == ic_State::M_INTERNAL)
		m_InterfaceTip->Update(Child(GameScreen::m_pDialogName)->FromPoint(Input::MouseState().x,Input::MouseState().y));
	else
		m_InterfaceTip->Update(FromPoint(Input::MouseState().x,Input::MouseState().y));

//	TEST
/*	static bool run_flag = false;

	if(Input::KeyFront(DIK_R)) run_flag = !run_flag;

	if(run_flag)
	{
		GameScreen* gs = static_cast<GameScreen*>(Child(GameScreen::m_pDialogName));
		static int counter = 0;

		Timer::Update();
		const float seconds = Timer::GetSeconds();

		gs->MsgWindow()->AddText(mlprintf("Test message #%0.6d",++counter).c_str());

		Timer::Update();
		DebugInfo::Add(600,300,"add message time: %0.6f",Timer::GetSeconds()-seconds);
	}
*/	
//	TEST

	return true;
}
//	освобождение всех ресурсов
void MenuMgr::Release(void)
{
	//	удаляем сэмплы
	if(m_AmbientSound)
	{
		m_AmbientSound->Release();
		m_AmbientSound = 0;
	}
	m_RainSound->Release();
	//	----------------------
	m_Deleter.m_pInstance = 0;
	delete this;
}
//	загрузка уровня
void MenuMgr::LoadLevel(const char* name)
{
	m_State->LoadLevel(name);
}
//	быстрый выход из программы
void MenuMgr::ExitFromProgram(void)
{
	Shell::Instance()->Exit();
}
//	обновление прогресc бара ЗАГРУЗКИ УРОВНЯ
void MenuMgr::UpdateLoadingProgressBar(float value)
{
	m_State->UpdateLoadingProgressBar(value);
}
//	установить заголовок прогресc бара ЗАГРУЗКИ УРОВНЯ
void MenuMgr::SetProgressBarTitle(const char* title)
{
	m_State->SetProgressBarTitle(title);
}
//	проверка соответсвия видео режима текущему состоянию игры
void MenuMgr::CheckVideoMode(CHECK_VIDEO_MODE cvm)
{
	DDFrame::VideoMode current_mode = Frame()->CurrentVideoMode();

	menu_mgr("Entire to CheckVideoMode;\n");

	if(!current_mode.IsValid())
	{
		if(Options::GetInt("system.video.windowed") && !CmdLine::IsKey("-fs"))
		{
			current_mode.m_Driver		= Options::GetInt("system.video.driver");
			current_mode.m_Device		= Options::GetInt("system.video.device");
			current_mode.m_Depth		= Options::GetInt("system.video.bpp");
			current_mode.m_Freq			= 0;
			current_mode.m_Mode			= DDFrame::VideoMode::M_WINDOWED;
		}
		else
		{
			current_mode.m_Driver		= Options::GetInt("system.video.driver");
			current_mode.m_Device		= Options::GetInt("system.video.device");
			current_mode.m_Depth		= Options::GetInt("system.video.bpp");
			current_mode.m_Freq			= Options::GetInt("system.video.freq");
			current_mode.m_Mode			= DDFrame::VideoMode::M_FULLSCREEN;
		}
		switch(cvm)
		{
		case CVM_FOR_MAINMENU:
			current_mode.m_Width = m_MainMenuResX;
			current_mode.m_Height = m_MainMenuResY;
			break;
		case CVM_FOR_CFG:
			current_mode.m_Width = Options::GetInt("system.video.width");
			current_mode.m_Height = Options::GetInt("system.video.height");
			break;
		}
		if(current_mode.m_Mode == DDFrame::VideoMode::M_WINDOWED)
		{
			current_mode.m_Width = m_MainMenuResX;
			current_mode.m_Height = m_MainMenuResY;
		}
		menu_mgr("...call D3DApp::SetVideoMode()\n");
		Frame()->SetVideoMode(current_mode);
	}
	else
	{
		if(current_mode.m_Mode == DDFrame::VideoMode::M_FULLSCREEN)
		{
			switch(cvm)
			{
			case CVM_FOR_MAINMENU:
				if((current_mode.m_Width != m_MainMenuResX) ||
					(current_mode.m_Height != m_MainMenuResY))
				{//	необходимо переключиться 
					menu_mgr("...call D3DApp::SetVideoMode()\n");
					current_mode.m_Width = m_MainMenuResX;
					current_mode.m_Height = m_MainMenuResY;
					Frame()->SetVideoMode(current_mode);
				}
				break;
			case CVM_FOR_CFG:
				if((current_mode.m_Width != Options::GetInt("system.video.width")) ||
					(current_mode.m_Height != Options::GetInt("system.video.height")))
				{//	необходимо переключиться 
					menu_mgr("...call D3DApp::SetVideoMode()\n");
					current_mode.m_Width = Options::GetInt("system.video.width");
					current_mode.m_Height = Options::GetInt("system.video.height");
					Frame()->SetVideoMode(current_mode);
				}
				break;
			}
		}
		else
		{
			if((current_mode.m_Width < m_MainMenuResX) ||
			   (current_mode.m_Height < m_MainMenuResY))
			{
				menu_mgr("...call D3DApp::SetVideoMode()\n");
				current_mode.m_Width = m_MainMenuResX;
				current_mode.m_Height = m_MainMenuResY;
				Frame()->SetVideoMode(current_mode);
			}
		}
	}
	Frame()->ShowWindow();
}
//	записать/считать данные необходимые интерфейсу
void MenuMgr::MakeSaveLoad(Storage& store)
{
	SavSlot name(&store,"description");
	std::string desc = m_LoadSaveGameModifier->SaveGameDesc();

	if(name.IsSaving())
	{
		name << desc;
	}
	else
	{
		name >> desc;
	}
	m_LoadSaveGameModifier->SetSaveGameDesc(desc.c_str());
}
//	вывести на экран MessageBox
int MenuMgr::QuestionBox(const char* text)
{
	bool lock_input = IsLockInput();
	class QuestionBox* qb;
	int result;

/*	LockInput(true);
	//	обновляем курсор
	MouseCursor::SetCursor(MouseCursor::CT_NORMAL);
	MenuMgr::Instance()->Tip()->Activate(false);*/
	//	создаем элемент интерфейса
	Insert(qb = new class QuestionBox());
	result = qb->DoModal(text);
	Delete(qb->Name());
/*	if(m_State->Mode() == ic_State::M_INTERNAL) LockInput(false);*/
	LockInput(lock_input);

	return result;
}

bool MenuMgr::IsQBFree(void)
{
	return !Child(QuestionBox::m_pDialogName) && !Shell::Instance()->IsModal();
}

//	заблокировать ввод на интерфейсе
void MenuMgr::LockInput(bool lock)
{
	m_bLockInput = lock;
}
//	выйти из игры
void MenuMgr::ExitFromGame(void)
{
	if(QuestionBox(GetStrResourse("mm_quit").c_str()))
	{
		Shell::Instance()->Exit();
	}
}
//	сбросить все модификаторы
void MenuMgr::ResetModifiers(void)
{
	m_LoadLevelModifier->SetAvailable(false);
	m_LoadSaveGameModifier->SetAvailable(MenuMgr::ic_LoadSaveGameModifier::A_NONE);
}
//	получить строковый ресурс
std::string MenuMgr::GetStrResourse(const std::string& name)
{
	return DirtyLinks::GetStrRes(name);
}

void MenuMgr::PlayAmbientSound(void)
{
	if(!m_AmbientSound)
	{
		m_AmbientSound = ISound::instance()->beginSession();
		m_AmbientSound->changeTheme("MAINMENU",0);
	}
}

void MenuMgr::StopAmbientSound(void)
{
	if(m_AmbientSound)
	{
		m_AmbientSound->Release();
		m_AmbientSound = 0;
	}
}
//	получить утверждение, что камеру можно перемещать
bool MenuMgr::IsCameraMoveable(void)
{
	if(Child(GameScreen::m_pDialogName)) return static_cast<GameScreen*>(Child(GameScreen::m_pDialogName))->IsCameraMoveable();

	return false;
}
//	информация о ключах
bool MenuMgr::IsLevelKeyPresent(void) const
{
#ifdef _HOME_VERSION
	return CmdLine::IsKey("-level");
#endif	//_HOME_VERSION
	return false;
}

//***************************************************************//
//******************* class DXView ************************//
DXView::DXView()
{
}

DXView::~DXView()
{
}

void DXView::Render(Widget *pWidget)
{
	const VRegion* rgn = pWidget->GetClipper()->Region();

	if(pWidget->GetImage())
	{
		SaveBltFast(rgn->X()+pWidget->GetLocation()->OriginX(),
					rgn->Y()+pWidget->GetLocation()->OriginY(),
					static_cast<DXImage *>(pWidget->GetImage())->m_pSurface,
					pWidget->GetClipper()->SubX(),
					pWidget->GetClipper()->SubY(),
					rgn->Width(),
					rgn->Height());
	}
#ifdef _HOME_VERSION
	const VRegion *pRegion = pWidget->GetLocation()->Region();
	static bool bFlag = false;
	if(Input::KeyFront(DIK_D|Input::RESET))
		bFlag = !bFlag;
	if(bFlag && pRegion->IsValid())
	{
		HDC hdc;

		D3DKernel::GetBB()->GetDC(&hdc);

		SelectObject(hdc,GetStockObject(NULL_BRUSH));
		SelectObject(hdc,GetStockObject(WHITE_PEN));
		int i;
		for(i=0;i<pRegion->Num()-1;i++)
		{
			MoveToEx(hdc,pRegion->Points()[i].x+pWidget->GetLocation()->OriginX(),pRegion->Points()[i].y+pWidget->GetLocation()->OriginY(),0);
			LineTo(hdc,pRegion->Points()[i+1].x+pWidget->GetLocation()->OriginX(),pRegion->Points()[i+1].y+pWidget->GetLocation()->OriginY());
		}
		if(i > 0)
		{
			MoveToEx(hdc,pRegion->Points()[i].x+pWidget->GetLocation()->OriginX(),pRegion->Points()[i].y+pWidget->GetLocation()->OriginY(),0);
			LineTo(hdc,pRegion->Points()[0].x+pWidget->GetLocation()->OriginX(),pRegion->Points()[0].y+pWidget->GetLocation()->OriginY());
		}

		D3DKernel::GetBB()->ReleaseDC(hdc);
	}
#endif	//_HOME_VERSION
}

void DXView::BltFast(int x,int y,LPDIRECTDRAWSURFACE7 pSurface)
{
	if(pSurface)
		D3DKernel::GetBB()->BltFast(x,y,pSurface,0,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);
}

void DXView::BltFast(int x,int y,LPDIRECTDRAWSURFACE7 pSurface,int left,int top,int right,int bottom)
{
	RECT rect = {left,top,right,bottom};

	if(pSurface)
		D3DKernel::GetBB()->BltFast(x,y,pSurface,&rect,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);
}

void DXView::SaveBltFast(int x,int y,LPDIRECTDRAWSURFACE7 pSurface,int src_x,int src_y,int width,int height)
{
	static RECT rect;
	
	rect.left	= std::max(src_x,-x);
	rect.top	= std::max(src_y,-y);
	rect.right	= std::min(width+src_x,D3DKernel::ResX()-x);
	rect.bottom = std::min(height+src_y,D3DKernel::ResY()-y);

	if(pSurface)
	{
		if(FAILED(D3DKernel::GetBB()->BltFast(std::max(0,x),std::max(0,y),pSurface,&rect,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY)))
		{
//			throw CASUS("Неудачный блиттинг при использовании BltFast.");
		}
	}
}

//***************************************************************//
//	class MenuMgr::ic_Tip 
const int MenuMgr::ic_Tip::m_TieX = 32;
const int MenuMgr::ic_Tip::m_TieY = 16;
MenuMgr::ic_Tip::ic_Tip(Widget* pParent)
{
	pParent->Insert(m_Text = new Text("tip"));
	m_Text->SetFont("tip",0x00ff00);
	m_Text->Align(LogicalText::T_LEFT|LogicalText::T_TOP);
	m_Text->SetRegion(0,0,200,30);
	m_Text->SetVisible(false);
}

MenuMgr::ic_Tip::~ic_Tip()
{
}

void MenuMgr::ic_Tip::Activate(bool on)
{
	m_Text->SetVisible(on);
	m_Text->Parent()->BringToTop(m_Text->Name());
}

void MenuMgr::ic_Tip::SetColor(unsigned int color)
{
	m_Text->SetFont("tip",color);
	m_Text->Update();
}

void MenuMgr::ic_Tip::MoveTo(int x,int y)
{
	x = std::min(x+m_TieX,D3DKernel::ResX()-m_Text->Width());
	y = std::min(y+m_TieY,D3DKernel::ResY()-m_Text->Height());

	m_Text->MoveTo(x,y);
}

void MenuMgr::ic_Tip::SetText(const char* text)
{
	m_Text->SetText(text);
}

//***************************************************************//
//	class MenuMgr::ic_InterfaceTip
const float MenuMgr::ic_InterfaceTip::m_SleepTime = 1.5f;
const int MenuMgr::ic_InterfaceTip::m_Color = 0xffffff;
MenuMgr::ic_InterfaceTip::ic_InterfaceTip() : m_Activated(false),m_TipActivated(false)
{
}

MenuMgr::ic_InterfaceTip::~ic_InterfaceTip()
{
}

void MenuMgr::ic_InterfaceTip::Update(Widget* pWidget)
{
	static Widget* pLastWidget = 0;
	static int ftime = 0;

	if(pWidget)
	{//	курсор находится на интерфейсе
		if(m_Activated)
		{//	курсор уже активизирован
			if(pWidget->m_Tip.m_Activated)
			{//	активизировать типсу
				if(!m_TipActivated || (pLastWidget != pWidget))
				{//	активизировать таймер
					pLastWidget = pWidget;
					MenuMgr::Instance()->Tip()->SetColor(m_Color);
					MenuMgr::Instance()->Tip()->SetText(pWidget->m_Tip.m_Text.c_str());
//					MenuMgr::Instance()->Tip()->SetText(MenuMgr::Instance()->GetStrResourse(pWidget->m_Tip.m_Text.c_str()).c_str());
					ftime = Timer::GetSeconds();
					m_TipActivated = true;
				}
				else
				{//	активизировать
					if((Timer::GetSeconds()-ftime) > m_SleepTime)
					{
						MenuMgr::Instance()->Tip()->Activate(true);
					}
				}
			}
			else
			{
				if(m_TipActivated)
				{
					MenuMgr::Instance()->Tip()->Activate(m_TipActivated = false);
				}
			}
		}
		else
		{
//	заремарено всвязи с переносом смены курсора полностью на сторону Punch'а
//			MouseCursor::SetCursor(MouseCursor::CT_NORMAL);
////////////////////////////////////////////////////////////////////////////
			MenuMgr::Instance()->Tip()->Activate(false);
			m_Activated	= true;
		}
//	TEST
/*
		Widget* widget = pWidget;
		std::string fwn = pWidget->Name();
		while(widget && widget->Parent())
		{
			widget = widget->Parent();
			std::string tmp = fwn;
			fwn = widget->Name();
			fwn += "/";
			fwn += tmp;
		}
		DebugInfo::Add(200,200,"Widget: [%s]",fwn.c_str());
*/
//	TEST
	}
	else
	{
		if(m_Activated)
		{
			m_Activated = m_TipActivated = false;
			MenuMgr::Instance()->Tip()->Activate(false);
		}
	}
}

//***************************************************************//
//	namespace MenuMgrGameController
//	загрузить уровень
void MenuMgrGameController::LoadLevel(const char* level_name)
{
	Game::LoadGameLevel(level_name);
	Game::SetGameLevelRender(true);
	Game::SetGameLevelUpdate(true);
}
//	выгрузить уровень
void MenuMgrGameController::UnloadLevel(void)
{
	Game::SetGameLevelRender(false);
	Game::SetGameLevelUpdate(false);
	Game::UnloadGameLevel();
}
//	сохранить/прочитать игру
void MenuMgrGameController::SaveLoadGame(const char* file_name,bool save)
{
	if(file_name && strlen(file_name))
	{
		if(save)
		{
			Game::MakeSaveLoad(file_name,true);
		}
		else
		{
			IWorld::Get()->CreateLevel();
			Game::MakeSaveLoad(file_name,false);
			Game::SetGameLevelRender(true);
			Game::SetGameLevelUpdate(true);
		}
	}
}

/*

Модификатор загрузки уровня.
	Он в себе хранит имя загружаемого уровня
	и указывает следует ли его загружать.
Модификатор загрузки сохраненой игры.
	В себе содержит имя загружаемой игры,
	и указывает следует ли ее загружать.

*/

//////////////////////////////////////////////////////////////////////////
//////////////    class MenuMgr::ic_MouseCursor::Changer    //////////////
//////////////////////////////////////////////////////////////////////////
class MenuMgr::ic_MouseCursor::Params
{
	struct TField
	{
		bool m_Valid;
		std::string m_Text;
		unsigned int m_Color;
		TField() : m_Valid(false),m_Color(0) {}
		void Set(const TextField* tf)
		{
			m_Valid = (tf != 0);
			if(m_Valid)
			{
				m_Text = tf->m_Text;
				m_Color = tf->m_Color;
			}
		}
		const TField& operator=(const TField& tf)
		{
			m_Valid = tf.m_Valid;
			m_Text = tf.m_Text;
			m_Color = tf.m_Color;

			return *this;
		}
		bool operator==(const TField& tf)
		{
			return (m_Valid == tf.m_Valid) && 
				   (m_Text  == tf.m_Text ) &&
				   (m_Color == tf.m_Color);
		}
	};
public:
	CURSOR_TYPE m_Type;
	TField m_TextFields[3];
public:
	Params() : m_Type(CT_CANMOVE) {}
	virtual ~Params() {}
public:
	void Set(CURSOR_TYPE type,const TextField* t1,const TextField* t2,const TextField* t3)
	{
		m_Type = type;
		m_TextFields[0].Set(t1);
		m_TextFields[1].Set(t2);
		m_TextFields[2].Set(t3);
	}
	bool operator == (const Params& p)
	{
		return (m_Type == p.m_Type) && 
			   (m_TextFields[0] == p.m_TextFields[0]) &&
			   (m_TextFields[1] == p.m_TextFields[1]) &&
			   (m_TextFields[2] == p.m_TextFields[2]);
	}
	const Params& operator = (const Params& p)
	{
		m_Type = p.m_Type;
		m_TextFields[0] = p.m_TextFields[0];
		m_TextFields[1] = p.m_TextFields[1];
		m_TextFields[2] = p.m_TextFields[2];

		return *this;
	}
};

//***************************************************************//
//	class MenuMgr::ic_MouseCursor
const char* MenuMgr::ic_MouseCursor::m_CursorsPath = "pictures/interface/cursors/";
MenuMgr::ic_MouseCursor::ic_MouseCursor() : m_Surface(0),m_CursorX(0),m_CursorY(0),
											m_Visible(true)
{
	TxtFile script;
	int count;
	int i;

	//	создаем контейнеры для параметров
	for(i=0;i<P_MAX;i++) m_Params[i] = new Params();
	//	считываем скрипт
	if(!script.Load(DataMgr::Load("scripts/interface/cursors.txt")))
		throw CASUS("Отсутствует файл с описанием типов курсоров.\nPath: scripts/interface/cursors.txt\n");
	DataMgr::Release("scripts/interface/cursors.txt");
	//	разбираем скрипт
	for(i=1;i<script.SizeY();i++)
	{
		m_Cursors[i-1].m_LinkX		= atoi(script(i,1).c_str());
		m_Cursors[i-1].m_LinkY		= atoi(script(i,2).c_str());
		m_Cursors[i-1].m_Width		= atoi(script(i,3).c_str());
		m_Cursors[i-1].m_Height		= atoi(script(i,4).c_str());
		m_Cursors[i-1].m_ImageName	= std::string(m_CursorsPath)+script(i,5)+".tga";
		for(int j=0;j<MAX_TEXT_FIELDS;j++)
		{
			count = sscanf(script(i,6+j).c_str(),"%d,%d",&m_Cursors[i-1].m_TextXY[j].x,&m_Cursors[i-1].m_TextXY[j].y);
			if(count != 2) m_Cursors[i-1].m_TextXY[j] = ipnt2_t(0,0);
		}
	}
	m_Font = script(1,9);
	count = sscanf(script(1,10).c_str(),"%d,%d",&m_TextFieldSize.x,&m_TextFieldSize.y);
	if(count != 2) m_TextFieldSize = ipnt2_t(200,30);
	//	инициализируем текстовые поля
	for(i=0;i<MAX_TEXT_FIELDS;i++)
	{
		m_TextFields[i] = new Text("text");
		m_TextFields[i]->SetRegion(0,0,m_TextFieldSize.x,m_TextFieldSize.y);
	}
	//	------------------
	memset(&m_rcCursor,0,sizeof(RECT));
	SetBorder(0,0,0,0);
	SetCursor(CT_NORMAL);
}

MenuMgr::ic_MouseCursor::~ic_MouseCursor()
{
	for(int i=0;i<MAX_TEXT_FIELDS;i++) delete m_TextFields[i];
	//	уничтожаем контейнеры для параметров
	for(int i=0;i<P_MAX;i++) delete m_Params[i];
}

//	установить позицию курсора
void MenuMgr::ic_MouseCursor::SetPos(int x,int y)
{
	m_CursorX = x-m_Cursors[m_Params[P_OLD]->m_Type].m_LinkX;
	m_CursorY = y-m_Cursors[m_Params[P_OLD]->m_Type].m_LinkY;

	m_rcCursor.right  = std::min(m_Cursors[m_Params[P_OLD]->m_Type].m_Width,D3DKernel::ResX()-m_CursorX);
	m_rcCursor.bottom = std::min(m_Cursors[m_Params[P_OLD]->m_Type].m_Height,D3DKernel::ResY()-m_CursorY);
	for(int i=0;i<MAX_TEXT_FIELDS;i++)
	{
		if(m_TextFields[i]->IsVisible())
		{
			TextMoveTo(m_TextFields[i],m_CursorX+m_Cursors[m_Params[P_OLD]->m_Type].m_TextXY[i].x,m_CursorY+m_Cursors[m_Params[P_OLD]->m_Type].m_TextXY[i].y);
		}
	}
}
//	установить границы действия курсора
void MenuMgr::ic_MouseCursor::SetBorder(int left,int top,int right,int bottom)
{
	m_rcBorder.left = left;
	m_rcBorder.top = top;
	m_rcBorder.right = right;
	m_rcBorder.bottom = bottom;
}
//	установить тип курсора и текстовые поля
void MenuMgr::ic_MouseCursor::SetCursor(CURSOR_TYPE type,
										const TextField* t1,
										const TextField* t2,
										const TextField* t3)
{
	m_Params[P_NEW]->Set(type,t1,t2,t3);
/*	const TextField* belt[MAX_TEXT_FIELDS] = {t1,t2,t3};

	if(type != m_Type)
	{//	курсор сменил свой тип
		if(m_Surface) SurfaceMgr::Instance()->Release(m_Cursors[m_Type].m_ImageName.c_str());
		m_Surface = SurfaceMgr::Instance()->Surface(m_Cursors[type].m_ImageName.c_str());

		m_rcCursor.right  = std::min(m_Cursors[type].m_Width,D3DKernel::ResX()-m_CursorX);
		m_rcCursor.bottom = std::min(m_Cursors[type].m_Height,D3DKernel::ResY()-m_CursorY);

		m_Type = type;
	}
	for(int i=0;i<MAX_TEXT_FIELDS;i++)
	{
		if(belt[i] && belt[i]->m_Text && strlen(belt[i]->m_Text))
		{
			m_TextFields[i]->SetVisible(true);
			TextMoveTo(m_TextFields[i],m_CursorX+m_Cursors[m_Type].m_TextXY[i].x,m_CursorY+m_Cursors[m_Type].m_TextXY[i].y);
			m_TextFields[i]->SetFont(m_Font.c_str(),belt[i]->m_Color);
			m_TextFields[i]->SetText(belt[i]->m_Text);
		}
		else
		{
			m_TextFields[i]->SetVisible(false);
		}
	}*/
}
//	видимость курсора
void MenuMgr::ic_MouseCursor::SetVisible(bool visible)
{
	m_Visible = visible;
}
//	уведомление о смене видео режима
void MenuMgr::ic_MouseCursor::OnChangeVideoMode(void)
{
	m_Surface = SurfaceMgr::Instance()->Surface(m_Cursors[m_Params[P_OLD]->m_Type].m_ImageName.c_str());
	for(int i=0;i<MAX_TEXT_FIELDS;i++)
		m_TextFields[i]->OnSystemChange();
}
//	отрисовать курсор
void MenuMgr::ic_MouseCursor::Render(void)
{
	//	проверим изменения состояния курсора
	if(!((*m_Params[P_OLD]) == (*m_Params[P_NEW])))
	{
		if(m_Params[P_OLD]->m_Type != m_Params[P_NEW]->m_Type)
		{
			if(m_Surface) SurfaceMgr::Instance()->Release(m_Cursors[m_Params[P_OLD]->m_Type].m_ImageName.c_str());
			m_Surface = SurfaceMgr::Instance()->Surface(m_Cursors[m_Params[P_NEW]->m_Type].m_ImageName.c_str());
		
			m_rcCursor.right  = std::min(m_Cursors[m_Params[P_NEW]->m_Type].m_Width,D3DKernel::ResX()-m_CursorX);
			m_rcCursor.bottom = std::min(m_Cursors[m_Params[P_NEW]->m_Type].m_Height,D3DKernel::ResY()-m_CursorY);
		}
		for(int i=0;i<MAX_TEXT_FIELDS;i++)
		{
			if(m_Params[P_NEW]->m_TextFields[i].m_Valid && m_Params[P_NEW]->m_TextFields[i].m_Text.size())
			{
				m_TextFields[i]->SetVisible(true);
				TextMoveTo(m_TextFields[i],m_CursorX+m_Cursors[m_Params[P_NEW]->m_Type].m_TextXY[i].x,m_CursorY+m_Cursors[m_Params[P_NEW]->m_Type].m_TextXY[i].y);
				m_TextFields[i]->SetFont(m_Font.c_str(),m_Params[P_NEW]->m_TextFields[i].m_Color);
				m_TextFields[i]->SetText(m_Params[P_NEW]->m_TextFields[i].m_Text.c_str());
			}
			else
			{
				m_TextFields[i]->SetVisible(false);
			}
		}

		(*m_Params[P_OLD]) = (*m_Params[P_NEW]);
	}
	if(m_Surface && m_Visible)
	{
		D3DKernel::GetBB()->BltFast(std::max(0,m_CursorX),std::max(0,m_CursorY),m_Surface,&m_rcCursor,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);
		for(int i=0;i<MAX_TEXT_FIELDS;i++)
			m_TextFields[i]->Render(MenuMgr::Instance()->GetDXView());
	}
}

void MenuMgr::ic_MouseCursor::TextMoveTo(Widget* widget,int x,int y)
{
	static int left,top,right,bottom; 
		
	widget->MoveTo(x,y);
	widget->GetLocation()->Region()->GetRect(&left,&top,&right,&bottom);
	widget->GetClipper()->Clip(*widget->GetLocation()->Region(),
							   left,top,right,bottom);
}
