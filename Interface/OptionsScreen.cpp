/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.03.2001

************************************************************************/
#include "Precomp.h"
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../Common/UI/Button.h"
#include "../Common/UI/Slide.h"
#include "../Common/UI/Static.h"
#include "Text.h"
#include "Edit.h"
#include "../Common/Shell/Shell.h"
#include "../Common/DataMgr/DataMgr.h"
#include "../Common/DataMgr/TxtFile.h"
#include "../Common/D3DApp/GammaControl/GammaControl.h"
#include "../Common/3dEffects/EffectManager.h"
#include "../Sound/ISound.h"
#include "../Common/Utils/Dir.h"
#include "../Game.h"
#include "../IWorld.h"
#include "../Options/Options.h"
#include "WidgetFactory.h"
#include "Interface.h"
#include "MenuMgr.h"
#include "OptionsScreen.h"
#include <ctime>

extern float EffectsVol;

//***********************************************************************//
//******************   class OptionsWrapper     *************************//
class OptionsWrapper : OptionsUpdater
{
private:
	enum PARAM
	{
		MARKS_ON_WALLS,
		DYNAMIC_LIGHTS,
		ENVIRONMENT,
		SHOW_PATH,
		SHOW_HEXGRID,
		SHADOW_Q,
		EFFECTS_Q,
		GAMMA,
		VOICES_VOLUME,
		AMBIENT_VOLUME,
		EFFECTS_VOLUME,
		MUSIC_VOLUME,
		ANIMA_SPEED,
		CAMERA_SPEED,
		MOUSE_SENSITIVITY,
		//	новые опции для экрана HiddenMovement
		ENEMY_MOVEMENT,
		NEUTRAL_MOVEMENT,
		ALLY_MOVEMENT,
		CIVILIAN_MOVEMENT,
		VEHICLE_MOVEMENT,
		ENEMY_FIRE,
		NEUTRAL_FIRE,
		ALLY_FIRE,
		CIVILIAN_FIRE,
		VEHICLE_FIRE,
		//	-------------------------------------
		MAX_PARAM
	};
	struct Traits
	{
		float m_Value;
		float m_OldValue;
		bool m_Changed;
		bool m_Apllied;
	} m_Params[MAX_PARAM];
private:
	OptionsUpdater* m_Updater;
public:
	OptionsWrapper(OptionsUpdater* updater) : m_Updater(updater) {Reset();}
	virtual ~OptionsWrapper(){}
public:
	void SetMarksOnWalls(bool value)
	{
		m_Params[MARKS_ON_WALLS].m_Value = value;
		m_Params[MARKS_ON_WALLS].m_Changed = true;
	}
	void SetDynamicLights(bool value)
	{
		m_Params[DYNAMIC_LIGHTS].m_Value = value;
		m_Params[DYNAMIC_LIGHTS].m_Changed = true;
	}
	void SetEnvironment(bool value)
	{
		m_Params[ENVIRONMENT].m_Value = value;
		m_Params[ENVIRONMENT].m_Changed = true;
	}
	void SetPath(bool value)
	{
		m_Params[SHOW_PATH].m_Value = value;
		m_Params[SHOW_PATH].m_Changed = true;
	}
	void SetHexGrid(bool value)
	{
		m_Params[SHOW_HEXGRID].m_Value = value;
		m_Params[SHOW_HEXGRID].m_Changed = true;
	}
	void SetShadowsQ(float value)
	{
		m_Params[SHADOW_Q].m_Value = value;
		m_Params[SHADOW_Q].m_Changed = true;
	}
	void SetEffectsQ(float value)
	{
		m_Params[EFFECTS_Q].m_Value = value;
		m_Params[EFFECTS_Q].m_Changed = true;
	}
	void SetGamma(float value)
	{
		const float offset = 0;
		const float length = 3;

		GammaControl::SetGamma(offset+value*length);
		m_Params[GAMMA].m_Value = value;
		m_Params[GAMMA].m_OldValue = Options::GetFloat("system.video.gamma")/3;
		m_Params[GAMMA].m_Changed = true;
		m_Params[GAMMA].m_Apllied = true;
	}
	void SetVoicesVolume(float value)
	{
		m_Params[VOICES_VOLUME].m_Value = value;
		m_Params[VOICES_VOLUME].m_Changed = true;
	}
	void SetAmbientVolume(float value)
	{
		m_Params[AMBIENT_VOLUME].m_Value = value;
		m_Params[AMBIENT_VOLUME].m_OldValue = static_cast<float>(Options::GetInt("game.sound.ambientvol"))/100;
		//	--------------------------------------
		ISound::instance()->setVolume(ISound::cAmbient,value);
		//	--------------------------------------
		m_Params[AMBIENT_VOLUME].m_Changed = true;
		m_Params[AMBIENT_VOLUME].m_Apllied = true;
	}
	void SetEffectsVolume(float value)
	{
		m_Params[EFFECTS_VOLUME].m_Value = value;
		m_Params[EFFECTS_VOLUME].m_OldValue = static_cast<float>(Options::GetInt("game.sound.effectsvol"))/100;
		//	--------------------------------------
		ISound::instance()->setVolume(ISound::cEffects,value);
		//	--------------------------------------
		m_Params[EFFECTS_VOLUME].m_Changed = true;
		m_Params[EFFECTS_VOLUME].m_Apllied = true;
	}
	void SetMusicVolume(float value)
	{
		m_Params[MUSIC_VOLUME].m_Value = value;
//		int test = Options::GetInt("game.sound.musicvol");
		m_Params[MUSIC_VOLUME].m_OldValue = static_cast<float>(Options::GetInt("game.sound.musicvol"))/100;
		//	--------------------------------------
		ISound::instance()->setVolume(ISound::cThemes,value);
		//	--------------------------------------
		m_Params[MUSIC_VOLUME].m_Changed = true;
		m_Params[MUSIC_VOLUME].m_Apllied = true;
	}
	void SetAnimaSpeed(float value)
	{
		m_Params[ANIMA_SPEED].m_Value = value;
		m_Params[ANIMA_SPEED].m_Changed = true;
	}
	void SetCameraSpeed(float value)
	{
		m_Params[CAMERA_SPEED].m_Value = value;
		m_Params[CAMERA_SPEED].m_Changed = true;
	}
	void SetMouseSensitivity(float value)
	{
		const float offset = 0.5;
		const float length = 6;

		Input::Mouse()->SetSensitivity(offset+value*length);
		m_Params[MOUSE_SENSITIVITY].m_Value = value;
		m_Params[MOUSE_SENSITIVITY].m_OldValue = (Options::GetFloat("system.input.sensitivity")-offset)/length;
		m_Params[MOUSE_SENSITIVITY].m_Changed = true;
		m_Params[MOUSE_SENSITIVITY].m_Apllied = true;
	}
	void SetEnemyMovement(bool value)
	{
		m_Params[ENEMY_MOVEMENT].m_Value = value;
		m_Params[ENEMY_MOVEMENT].m_Changed = true;
	}
	void SetNeutralMovement(bool value)
	{
		m_Params[NEUTRAL_MOVEMENT].m_Value = value;
		m_Params[NEUTRAL_MOVEMENT].m_Changed = true;
	}
	void SetAllyMovement(bool value)
	{
		m_Params[ALLY_MOVEMENT].m_Value = value;
		m_Params[ALLY_MOVEMENT].m_Changed = true;
	}
	void SetCivilianMovement(bool value)
	{
		m_Params[CIVILIAN_MOVEMENT].m_Value = value;
		m_Params[CIVILIAN_MOVEMENT].m_Changed = true;
	}
	void SetVehicleMovement(bool value)
	{
		m_Params[VEHICLE_MOVEMENT].m_Value = value;
		m_Params[VEHICLE_MOVEMENT].m_Changed = true;
	}
	void SetEnemyFire(bool value)
	{
		m_Params[ENEMY_FIRE].m_Value = value;
		m_Params[ENEMY_FIRE].m_Changed = true;
	}
	void SetNeutralFire(bool value)
	{
		m_Params[NEUTRAL_FIRE].m_Value = value;
		m_Params[NEUTRAL_FIRE].m_Changed = true;
	}
	void SetAllyFire(bool value)
	{
		m_Params[ALLY_FIRE].m_Value = value;
		m_Params[ALLY_FIRE].m_Changed = true;
	}
	void SetCivilianFire(bool value)
	{
		m_Params[CIVILIAN_FIRE].m_Value = value;
		m_Params[CIVILIAN_FIRE].m_Changed = true;
	}
	void SetVehicleFire(bool value)
	{
		m_Params[VEHICLE_FIRE].m_Value = value;
		m_Params[VEHICLE_FIRE].m_Changed = true;
	}
public:
	bool GetPath(void){return m_Updater->GetPath();}
	bool GetHexGrid(void){return m_Updater->GetHexGrid();}
public:
	void Reset(void)
	{
		for(int i=0;i<MAX_PARAM;i++)
		{
			m_Params[i].m_Changed = false;
			m_Params[i].m_Apllied = false;
		}
	}
	void Update(void)
	{
		if(m_Params[MARKS_ON_WALLS].m_Changed)    m_Updater->SetMarksOnWalls(static_cast<bool>(m_Params[MARKS_ON_WALLS].m_Value));
		if(m_Params[DYNAMIC_LIGHTS].m_Changed)    m_Updater->SetDynamicLights(static_cast<bool>(m_Params[DYNAMIC_LIGHTS].m_Value));
		if(m_Params[ENVIRONMENT].m_Changed)       m_Updater->SetEnvironment(static_cast<bool>(m_Params[ENVIRONMENT].m_Value));
		if(m_Params[SHOW_PATH].m_Changed)         m_Updater->SetPath(static_cast<bool>(m_Params[SHOW_PATH].m_Value));
		if(m_Params[SHOW_HEXGRID].m_Changed)      m_Updater->SetHexGrid(static_cast<bool>(m_Params[SHOW_HEXGRID].m_Value));
		if(m_Params[SHADOW_Q].m_Changed)          m_Updater->SetShadowsQ(m_Params[SHADOW_Q].m_Value);
		if(m_Params[EFFECTS_Q].m_Changed)         m_Updater->SetEffectsQ(m_Params[EFFECTS_Q].m_Value);
		if(m_Params[GAMMA].m_Changed)             m_Updater->SetGamma(m_Params[GAMMA].m_Value);
		if(m_Params[VOICES_VOLUME].m_Changed)     m_Updater->SetVoicesVolume(m_Params[VOICES_VOLUME].m_Value);
		if(m_Params[AMBIENT_VOLUME].m_Changed)    m_Updater->SetAmbientVolume(m_Params[AMBIENT_VOLUME].m_Value);
		if(m_Params[EFFECTS_VOLUME].m_Changed)    m_Updater->SetEffectsVolume(m_Params[EFFECTS_VOLUME].m_Value);
		if(m_Params[MUSIC_VOLUME].m_Changed)      m_Updater->SetMusicVolume(m_Params[MUSIC_VOLUME].m_Value);
		if(m_Params[ANIMA_SPEED].m_Changed)       m_Updater->SetAnimaSpeed(m_Params[ANIMA_SPEED].m_Value);
		if(m_Params[CAMERA_SPEED].m_Changed)      m_Updater->SetCameraSpeed(m_Params[CAMERA_SPEED].m_Value);
		if(m_Params[MOUSE_SENSITIVITY].m_Changed) m_Updater->SetMouseSensitivity(m_Params[MOUSE_SENSITIVITY].m_Value);
		if(m_Params[ENEMY_MOVEMENT].m_Changed)		m_Updater->SetEnemyMovement(m_Params[ENEMY_MOVEMENT].m_Value);
		if(m_Params[ENEMY_FIRE].m_Changed)			m_Updater->SetEnemyFire(m_Params[ENEMY_FIRE].m_Value);
		if(m_Params[NEUTRAL_MOVEMENT].m_Changed)	m_Updater->SetNeutralMovement(m_Params[NEUTRAL_MOVEMENT].m_Value);
		if(m_Params[NEUTRAL_FIRE].m_Changed)		m_Updater->SetNeutralFire(m_Params[NEUTRAL_FIRE].m_Value);
		if(m_Params[ALLY_MOVEMENT].m_Changed)		m_Updater->SetAllyMovement(m_Params[ALLY_MOVEMENT].m_Value);
		if(m_Params[ALLY_FIRE].m_Changed)			m_Updater->SetAllyFire(m_Params[ALLY_FIRE].m_Value);
		if(m_Params[CIVILIAN_MOVEMENT].m_Changed)	m_Updater->SetCivilianMovement(m_Params[CIVILIAN_MOVEMENT].m_Value);
		if(m_Params[CIVILIAN_FIRE].m_Changed)		m_Updater->SetCivilianFire(m_Params[CIVILIAN_FIRE].m_Value);
		if(m_Params[VEHICLE_MOVEMENT].m_Changed)	m_Updater->SetVehicleMovement(m_Params[VEHICLE_MOVEMENT].m_Value);
		if(m_Params[VEHICLE_FIRE].m_Changed)		m_Updater->SetVehicleFire(m_Params[VEHICLE_FIRE].m_Value);

		Reset();
		Options::Flush();
	}
	void Restore(void)
	{
		if(m_Params[MARKS_ON_WALLS].m_Apllied)    m_Updater->SetMarksOnWalls(static_cast<bool>(m_Params[MARKS_ON_WALLS].m_OldValue));
		if(m_Params[DYNAMIC_LIGHTS].m_Apllied)    m_Updater->SetDynamicLights(static_cast<bool>(m_Params[DYNAMIC_LIGHTS].m_OldValue));
		if(m_Params[ENVIRONMENT].m_Apllied)       m_Updater->SetEnvironment(static_cast<bool>(m_Params[ENVIRONMENT].m_OldValue));
		if(m_Params[SHOW_PATH].m_Apllied)         m_Updater->SetPath(static_cast<bool>(m_Params[SHOW_PATH].m_OldValue));
		if(m_Params[SHOW_HEXGRID].m_Apllied)      m_Updater->SetHexGrid(static_cast<bool>(m_Params[SHOW_HEXGRID].m_OldValue));
		if(m_Params[SHADOW_Q].m_Apllied)          m_Updater->SetShadowsQ(m_Params[SHADOW_Q].m_OldValue);
		if(m_Params[EFFECTS_Q].m_Apllied)         m_Updater->SetEffectsQ(m_Params[EFFECTS_Q].m_OldValue);
		if(m_Params[GAMMA].m_Apllied)             m_Updater->SetGamma(m_Params[GAMMA].m_OldValue);
		if(m_Params[VOICES_VOLUME].m_Apllied)     m_Updater->SetVoicesVolume(m_Params[VOICES_VOLUME].m_OldValue);
		if(m_Params[AMBIENT_VOLUME].m_Apllied)    m_Updater->SetAmbientVolume(m_Params[AMBIENT_VOLUME].m_OldValue);
		if(m_Params[EFFECTS_VOLUME].m_Apllied)    m_Updater->SetEffectsVolume(m_Params[EFFECTS_VOLUME].m_OldValue);
		if(m_Params[MUSIC_VOLUME].m_Apllied)      m_Updater->SetMusicVolume(m_Params[MUSIC_VOLUME].m_OldValue);
		if(m_Params[ANIMA_SPEED].m_Apllied)       m_Updater->SetAnimaSpeed(m_Params[ANIMA_SPEED].m_OldValue);
		if(m_Params[CAMERA_SPEED].m_Apllied)      m_Updater->SetCameraSpeed(m_Params[CAMERA_SPEED].m_OldValue);
		if(m_Params[MOUSE_SENSITIVITY].m_Apllied) m_Updater->SetMouseSensitivity(m_Params[MOUSE_SENSITIVITY].m_OldValue);
		if(m_Params[ENEMY_MOVEMENT].m_Apllied)		m_Updater->SetEnemyMovement(m_Params[ENEMY_MOVEMENT].m_OldValue);
		if(m_Params[ENEMY_FIRE].m_Apllied)			m_Updater->SetEnemyFire(m_Params[ENEMY_FIRE].m_OldValue);
		if(m_Params[NEUTRAL_MOVEMENT].m_Apllied)	m_Updater->SetNeutralMovement(m_Params[NEUTRAL_MOVEMENT].m_OldValue);
		if(m_Params[NEUTRAL_FIRE].m_Apllied)		m_Updater->SetNeutralFire(m_Params[NEUTRAL_FIRE].m_OldValue);
		if(m_Params[ALLY_MOVEMENT].m_Apllied)		m_Updater->SetAllyMovement(m_Params[ALLY_MOVEMENT].m_OldValue);
		if(m_Params[ALLY_FIRE].m_Apllied)			m_Updater->SetAllyFire(m_Params[ALLY_FIRE].m_OldValue);
		if(m_Params[CIVILIAN_MOVEMENT].m_Apllied)	m_Updater->SetCivilianMovement(m_Params[CIVILIAN_MOVEMENT].m_OldValue);
		if(m_Params[CIVILIAN_FIRE].m_Apllied)		m_Updater->SetCivilianFire(m_Params[CIVILIAN_FIRE].m_OldValue);
		if(m_Params[VEHICLE_MOVEMENT].m_Apllied)	m_Updater->SetVehicleMovement(m_Params[VEHICLE_MOVEMENT].m_OldValue);
		if(m_Params[VEHICLE_FIRE].m_Apllied)		m_Updater->SetVehicleFire(m_Params[VEHICLE_FIRE].m_OldValue);

		Reset();
	}
};

//**************************************************************************//
const char* OptionsScreen::m_pDialogName = "OptionsScreen";
OptionsScreen::OptionsScreen() : Dialog(m_pDialogName,m_pDialogName),m_Controller(0),m_Mode(M_MAINMENU),m_pCurrentlyVisible(0),m_ParentNotify(PN_NONE)
{
	m_Updater = new OptionsWrapper(IWorld::Get()->GetOptionsUpdater());
	//	-----------------------------------------
	Insert(m_Video = new Video(m_Updater),L_CENTER);
	Insert(m_Audio = new Audio(m_Updater),L_CENTER);
	Insert(m_System = new System(m_Updater),L_CENTER);
	Insert(m_Camera = new Camera(m_Updater),L_CENTER);
	Insert(m_TipOfTheDay = new TipOfTheDay(),L_CENTER);
	Insert(m_SaveLoad = new SaveLoad(),L_CENTER);
	//	-----------------------------------------
	//	кнопочки
	m_Buttons[B_EXIT]	= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"exit"));
	m_Buttons[B_BACK]	= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"back"));
	m_Buttons[B_SAVE]	= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"save"));
	m_Buttons[B_LOAD]	= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"load"));
	m_Buttons[B_AUDIO]	= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"audio"));
	m_Buttons[B_VIDEO]	= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"video"));
	m_Buttons[B_SYSTEM] = static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"system"));
	m_Buttons[B_CAMERA] = static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"camera"));
	m_Buttons[B_OK]		= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"ok"));
	m_Buttons[B_CANCEL] = static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"cancel"));
	for(int i=0;i<MAX_BUTTON;i++)
		m_Buttons[i]->Focus()->Enable(WidgetFocus::FF_KEYBOARD,false);
	//	--------------------------------------------------------------------------------------
	SetMode(m_Mode);
}

OptionsScreen::~OptionsScreen()
{
	delete m_Updater;
}
//	сбросить опции
void OptionsScreen::Reset(void)
{
	m_Video->Reset();
	m_Audio->Reset();
	m_System->Reset();
	m_Camera->Reset();
}

//	установить контролллер логики
void OptionsScreen::SetController(OptionsScreenController* controller)
{
	m_Controller = controller;
}
//	уведомление об изменении состояния системы
void OptionsScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}

//	уведомление об изменении состояния дочернего виджета
void OptionsScreen::OnChange(Widget* pChild)
{
	if((pChild == m_SaveLoad) && m_Controller)
	{
		OrginizeSaveLoad();
		return;
	}
	//	--------------------
	Dialog::OnChange(pChild);
}
//	обработка клавиатурного ввода
bool OptionsScreen::ProcessKeyboardInput(VKeyboardState* /*pKS*/)
{
	return true;
}
//	уведомление об изменении состояния кнопок
void OptionsScreen::OnButton(Button *pButton)
{
	if((pButton != m_Buttons[B_OK]) && (m_pCurrentlyVisible == m_SaveLoad))
		m_SaveLoad->OnCanceled();
	//	--------------------------------------
	if(pButton == m_Buttons[B_VIDEO])
	{
		if(m_pCurrentlyVisible) m_pCurrentlyVisible->SetVisible(false);
		m_Video->SetVisible(true);
		m_pCurrentlyVisible = m_Video;
		return;
	}
	if(pButton == m_Buttons[B_AUDIO])
	{
		if(m_pCurrentlyVisible) m_pCurrentlyVisible->SetVisible(false);
		m_Audio->SetVisible(true);
		m_pCurrentlyVisible = m_Audio;
		return;
	}
	if(pButton == m_Buttons[B_SYSTEM])
	{
		if(m_pCurrentlyVisible) m_pCurrentlyVisible->SetVisible(false);
		m_System->SetVisible(true);
		m_pCurrentlyVisible = m_System;
		return;
	}
	if(pButton == m_Buttons[B_CAMERA])
	{
		if(m_pCurrentlyVisible) m_pCurrentlyVisible->SetVisible(false);
		m_Camera->SetVisible(true);
		m_pCurrentlyVisible = m_Camera;
		return;
	}
	if(m_Controller)
	{
		if(pButton == m_Buttons[B_EXIT])
		{
			if(MenuMgr::Instance()->QuestionBox(MenuMgr::Instance()->GetStrResourse("mm_exitfromgame").c_str()))
			{
				m_ParentNotify = PN_EXIT;
				if(Parent()) Parent()->OnChange(this);
			}
			return;
		}
		if(pButton == m_Buttons[B_BACK])
		{
			m_Controller->OnBack();
			return;
		}
		if(pButton == m_Buttons[B_CANCEL])
		{
			m_Updater->Restore();
			switch(m_Mode)
			{
			case M_MAINMENU:
				//	обновляем закладки опций
				m_Video->Reset();
				m_Audio->Reset();
				m_System->Reset();
				m_Camera->Reset();
				//	выходим
				m_Controller->OnBack();
				break;
			case M_GAMEMENU:
				//	обновляем закладки опций
				m_Video->Reset();
				m_Audio->Reset();
				m_System->Reset();
				m_Camera->Reset();
				//	oops!
				if(m_pCurrentlyVisible) m_pCurrentlyVisible->SetVisible(false);
				m_pCurrentlyVisible = m_TipOfTheDay;
				m_pCurrentlyVisible->SetVisible(true);
				break;
			case M_LOADINGMENU:
				m_Controller->OnBack();
				break;
			}
			return;
		}
		if(pButton == m_Buttons[B_OK])
		{
			if(m_pCurrentlyVisible == m_SaveLoad)
			{
				OrginizeSaveLoad();
			}
			else
			{
				m_Updater->Update();
				switch(m_Mode)
				{
				case M_MAINMENU:
					m_Controller->OnBack();
					break;
				case M_GAMEMENU:
					if(m_pCurrentlyVisible) m_pCurrentlyVisible->SetVisible(false);
					m_pCurrentlyVisible = m_TipOfTheDay;
					m_pCurrentlyVisible->SetVisible(true);
					break;
				}
			}
			
			return;
		}
	}
	if(pButton == m_Buttons[B_SAVE])
	{
		switch(m_Mode)
		{
		case M_MAINMENU:
		case M_GAMEMENU:
			m_Updater->Restore();
			//	обновляем закладки опций
			m_Video->Reset();
			m_Audio->Reset();
			m_System->Reset();
			m_Camera->Reset();
		}
		m_SaveLoad->SetMode(SaveLoad::M_SAVE);
	}
	if(pButton == m_Buttons[B_LOAD])
	{
		switch(m_Mode)
		{
		case M_MAINMENU:
		case M_GAMEMENU:
			m_Updater->Restore();
			//	обновляем закладки опций
			m_Video->Reset();
			m_Audio->Reset();
			m_System->Reset();
			m_Camera->Reset();
		}
		m_SaveLoad->SetMode(SaveLoad::M_LOAD);
	}
	if(m_SaveLoad->GetMode() != SaveLoad::M_NONE)
	{
		if(m_pCurrentlyVisible) m_pCurrentlyVisible->SetVisible(false);
		m_SaveLoad->SetVisible(true);
		Focus()->Set(WidgetFocus::FF_KEYBOARD,m_SaveLoad);
		m_pCurrentlyVisible = m_SaveLoad;
		return;
	}
}
//	установить режим работы меню
void OptionsScreen::SetMode(MODE mode)
{
	//	установки взависимости от режима
	switch(m_Mode = mode)
	{
	case M_MAINMENU:
		m_Buttons[B_EXIT]->SetVisible(false);
		m_Buttons[B_BACK]->SetVisible(false);
		m_Buttons[B_SAVE]->SetVisible(false);
		m_Buttons[B_LOAD]->SetVisible(false);
		m_Buttons[B_OK]->SetVisible(true);
		m_Buttons[B_CANCEL]->SetVisible(true);
		if(m_pCurrentlyVisible) m_pCurrentlyVisible->SetVisible(false);
		m_pCurrentlyVisible = m_Video;
		m_pCurrentlyVisible->SetVisible(true);
		break;
	case M_GAMEMENU:
		m_Buttons[B_EXIT]->SetVisible(true);
		m_Buttons[B_BACK]->SetVisible(true);
		m_Buttons[B_SAVE]->SetVisible(true);
		m_Buttons[B_LOAD]->SetVisible(true);
		m_Buttons[B_OK]->SetVisible(false);
		m_Buttons[B_CANCEL]->SetVisible(false);
		if(m_pCurrentlyVisible) m_pCurrentlyVisible->SetVisible(false);
		m_pCurrentlyVisible = m_TipOfTheDay;
		m_pCurrentlyVisible->SetVisible(true);
		m_SaveLoad->SetMode(SaveLoad::M_LOAD);
		break;
	case M_LOADINGMENU:
		m_Buttons[B_AUDIO]->SetVisible(false);
		m_Buttons[B_VIDEO]->SetVisible(false);
		m_Buttons[B_SYSTEM]->SetVisible(false);
		m_Buttons[B_CAMERA]->SetVisible(false);
		m_Buttons[B_EXIT]->SetVisible(false);
		m_Buttons[B_BACK]->SetVisible(false);
		m_Buttons[B_SAVE]->SetVisible(false);
		m_Buttons[B_LOAD]->SetVisible(false);
		m_Buttons[B_OK]->SetVisible(true);
		m_Buttons[B_CANCEL]->SetVisible(true);
		if(m_pCurrentlyVisible) m_pCurrentlyVisible->SetVisible(false);
		m_pCurrentlyVisible = m_SaveLoad;
		m_SaveLoad->SetVisible(true);
		Focus()->Set(WidgetFocus::FF_KEYBOARD,m_SaveLoad);
		m_SaveLoad->SetMode(SaveLoad::M_LOAD);
		break;
	}
}
//	организовать загрузку и запись игры
void OptionsScreen::OrginizeSaveLoad(void)
{
	std::string name,desc;
	
	if(m_SaveLoad->GetSelection(name,desc))
	{
		//	изменить модификатор
		MenuMgr::Instance()->LoadSaveGameModifier()->SetSaveGameName(name.c_str());
		MenuMgr::Instance()->LoadSaveGameModifier()->SetSaveGameDesc(desc.c_str());
		switch(m_SaveLoad->GetMode())
		{
		case SaveLoad::M_SAVE:
			//	проверка на перезапись игры
			if(!CheckResaving())
			{
				m_SaveLoad->FillSlots();
				return;
			}
			MenuMgr::Instance()->LoadSaveGameModifier()->SetAvailable(MenuMgr::ic_LoadSaveGameModifier::A_SAVE);
			break;
		case SaveLoad::M_LOAD:
			MenuMgr::Instance()->LoadSaveGameModifier()->SetAvailable(MenuMgr::ic_LoadSaveGameModifier::A_LOAD);
			break;
		default:
			MenuMgr::Instance()->LoadSaveGameModifier()->SetAvailable(MenuMgr::ic_LoadSaveGameModifier::A_NONE);
		}
		//	уведомить об этом родителя
		m_ParentNotify = PN_SAVELOAD;
		Parent()->OnChange(this);
		//	выйти
		m_Controller->OnBack();
	}
}
//	проверка на перезапись игры
bool OptionsScreen::CheckResaving(void)
{
	if(m_SaveLoad->IsResavingSelection())
	{
		return MenuMgr::Instance()->QuestionBox(MenuMgr::Instance()->GetStrResourse("mm_resaving").c_str());
	}

	return true;
}
//	вернуть полное имя файла для сейва
std::string OptionsScreen::GetFullFileNameOfSave(const char* save_file_name)
{
	return SaveLoad::GetFullFileNameOfSave(save_file_name);
}
//	создать каталог для хранения файлов сохранения
void OptionsScreen::MakeSavesDir(void)
{
	mkdir(SaveLoad::m_SavesPath);
}

//***********************************************************************//
//****************** class OptionsScreen::Video *************************//
const char* OptionsScreen::Video::m_pDialogName = "Video";
OptionsScreen::Video::Video(OptionsWrapper* updater) : Dialog(m_pDialogName,m_pDialogName),m_Updater(updater)
{
	m_Sliders[SHADOWS] = static_cast<Slide*>(WidgetFactory::Instance()->Assert(this,"shadows"));
	m_Sliders[EFFECTS] = static_cast<Slide*>(WidgetFactory::Instance()->Assert(this,"effects"));
	m_Sliders[GAMMA] = static_cast<Slide*>(WidgetFactory::Instance()->Assert(this,"gamma"));
	//	---------------------------------------------------------------------
	m_Switches[MARKS] = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"marks"));
	m_Switches[LIGHTS] = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"lights"));
	m_Switches[ENVIRONMENT] = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"environment"));
	//	---------------------------------------------------------------------
	Reset();
}

OptionsScreen::Video::~Video()
{
}

void OptionsScreen::Video::Reset(void)
{
	m_Sliders[GAMMA]->SetStep(0.025);
	m_Sliders[GAMMA]->SetValue(Options::GetFloat("system.video.gamma")/3);
	m_Sliders[EFFECTS]->SetStep(0.025);
	m_Sliders[EFFECTS]->SetValue(Options::GetFloat("system.video.effectsquality"));
	m_Sliders[SHADOWS]->SetStep(1.0f/6);
	m_Sliders[SHADOWS]->SetValue(static_cast<float>(Options::GetInt("system.video.shadowquality"))/6);
	//	---------------------------------------------------------------------
	m_Switches[MARKS]->SetState(Options::GetInt("game.marks_on_walls"));
	m_Switches[LIGHTS]->SetState(Options::GetInt("game.dynamic_lights"));
	m_Switches[ENVIRONMENT]->SetState(Options::GetInt("game.environment"));
}

//	уведомление об изменении состояния системы
void OptionsScreen::Video::OnSystemChange(void)
{
	Widget::OnSystemChange();
}

void OptionsScreen::Video::OnStatic(Static *pStatic)
{
	if(pStatic->LastEvent() == Static::LE_LBUTTONUP)
	{
		pStatic->SetState(!pStatic->GetState());
		if(pStatic == m_Switches[MARKS]) m_Updater->SetMarksOnWalls(m_Switches[MARKS]->GetState());
		if(pStatic == m_Switches[LIGHTS]) m_Updater->SetDynamicLights(m_Switches[LIGHTS]->GetState());
		if(pStatic == m_Switches[ENVIRONMENT]) m_Updater->SetEnvironment(m_Switches[ENVIRONMENT]->GetState());
	}
}

void OptionsScreen::Video::OnChange(Widget* pChild)
{
	if(pChild == m_Sliders[SHADOWS]) m_Updater->SetShadowsQ(m_Sliders[SHADOWS]->Value());
	if(pChild == m_Sliders[EFFECTS]) m_Updater->SetEffectsQ(m_Sliders[EFFECTS]->Value());
	if(pChild == m_Sliders[GAMMA])   m_Updater->SetGamma(m_Sliders[GAMMA]->Value());

	Dialog::OnChange(pChild);
}

//***********************************************************************//
//****************** class OptionsScreen::Audio *************************//
const char* OptionsScreen::Audio::m_pDialogName = "Audio";
OptionsScreen::Audio::Audio(OptionsWrapper* updater) : Dialog(m_pDialogName,m_pDialogName),m_Updater(updater)
{
	m_Sliders[VOICES]  = static_cast<Slide*>(WidgetFactory::Instance()->Assert(this,"voices"));
	m_Sliders[AMBIENT] = static_cast<Slide*>(WidgetFactory::Instance()->Assert(this,"ambient"));
	m_Sliders[EFFECTS] = static_cast<Slide*>(WidgetFactory::Instance()->Assert(this,"effects"));
	m_Sliders[MUSIC]   = static_cast<Slide*>(WidgetFactory::Instance()->Assert(this,"music"));
	//	---------------------------------------------------------------------
	Reset();
}

OptionsScreen::Audio::~Audio()
{
}
//	уведомление об изменении состояния системы
void OptionsScreen::Audio::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	сброс всех параметров
void OptionsScreen::Audio::Reset(void)
{
	m_Sliders[VOICES]->SetStep(0.01);
	m_Sliders[VOICES]->SetValue((float)Options::GetInt("game.sound.voicesvol")/100);
	m_Sliders[AMBIENT]->SetStep(0.01);
	m_Sliders[AMBIENT]->SetValue((float)Options::GetInt("game.sound.ambientvol")/100);
	m_Sliders[EFFECTS]->SetStep(0.01);
	m_Sliders[EFFECTS]->SetValue((float)Options::GetInt("game.sound.effectsvol")/100);
	m_Sliders[MUSIC]->SetStep(0.01);
	m_Sliders[MUSIC]->SetValue((float)Options::GetInt("game.sound.musicvol")/100);
}

void OptionsScreen::Audio::OnChange(Widget* pChild)
{
	if(pChild == m_Sliders[VOICES])  m_Updater->SetVoicesVolume(m_Sliders[VOICES]->Value());
	if(pChild == m_Sliders[AMBIENT]) m_Updater->SetAmbientVolume(m_Sliders[AMBIENT]->Value());
	if(pChild == m_Sliders[EFFECTS]) m_Updater->SetEffectsVolume(m_Sliders[EFFECTS]->Value());
	if(pChild == m_Sliders[MUSIC])   m_Updater->SetMusicVolume(m_Sliders[MUSIC]->Value());

	Dialog::OnChange(pChild);
}

//***********************************************************************//
//****************** class OptionsScreen::System ************************//
const char* OptionsScreen::System::m_pDialogName = "System";
OptionsScreen::System::System(OptionsWrapper* updater) : Dialog(m_pDialogName,m_pDialogName),m_Updater(updater)
{
	m_Sliders[ANIMA_SPEED] = static_cast<Slide*>(WidgetFactory::Instance()->Assert(this,"anima"));
	m_Sliders[CAMERA_SPEED] = static_cast<Slide*>(WidgetFactory::Instance()->Assert(this,"camera"));
	m_Sliders[MOUSE_SENSITIVITY] = static_cast<Slide*>(WidgetFactory::Instance()->Assert(this,"mouse"));
	//	---------------------------------------------------------------------
	m_Switches[PATH] = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"path"));
	m_Switches[HEXGRID] = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"hexgrid"));
	//	---------------------------------------------------------------------
	Reset();
}

OptionsScreen::System::~System()
{
}

//	уведомление об изменении состояния системы
void OptionsScreen::System::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	сброс всех параметров
void OptionsScreen::System::Reset(void)
{
	m_Sliders[MOUSE_SENSITIVITY]->SetStep(0.001);
	m_Sliders[MOUSE_SENSITIVITY]->SetValue((Options::GetFloat("system.input.sensitivity")-0.5)/6);
	m_Sliders[ANIMA_SPEED]->SetStep(0.001);
	m_Sliders[ANIMA_SPEED]->SetValue((Options::GetFloat("game.anispeed")-0.5f)/4.5f);
	m_Sliders[CAMERA_SPEED]->SetStep(0.001);
	m_Sliders[CAMERA_SPEED]->SetValue((Options::GetFloat("game.camera_speed")-0.5f)/4.5f);
	//	---------------------------------------------------------------------
	m_Switches[PATH]->SetState(m_Updater->GetPath());
	m_Switches[HEXGRID]->SetState(m_Updater->GetHexGrid());
}

void OptionsScreen::System::OnStatic(Static *pStatic)
{
	if(pStatic->LastEvent() == Static::LE_LBUTTONUP)
	{
		pStatic->SetState(!pStatic->GetState());
		if(pStatic == m_Switches[PATH]) m_Updater->SetPath(m_Switches[PATH]->GetState());
		if(pStatic == m_Switches[HEXGRID]) m_Updater->SetHexGrid(m_Switches[HEXGRID]->GetState());
	}
}

void OptionsScreen::System::OnChange(Widget* pChild)
{
	if(pChild == m_Sliders[ANIMA_SPEED]) m_Updater->SetAnimaSpeed(m_Sliders[ANIMA_SPEED]->Value());
	if(pChild == m_Sliders[CAMERA_SPEED]) m_Updater->SetCameraSpeed(m_Sliders[CAMERA_SPEED]->Value());
	if(pChild == m_Sliders[MOUSE_SENSITIVITY])   m_Updater->SetMouseSensitivity(m_Sliders[MOUSE_SENSITIVITY]->Value());

	Dialog::OnChange(pChild);
}

//***********************************************************************//
//****************** class OptionsScreen::Camera ************************//
const char* OptionsScreen::Camera::m_pDialogName = "Camera";
OptionsScreen::Camera::Camera(OptionsWrapper* updater) : Dialog(m_pDialogName,m_pDialogName),m_Updater(updater)
{
	//	---------------------------------------------------------------------
	m_Switches[ENEMY_MOVEMENT]		= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"enemy_movement"));
	m_Switches[NEUTRAL_MOVEMENT]	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"neutral_movement"));
	m_Switches[ALLY_MOVEMENT]		= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"ally_movement"));
	m_Switches[CIVILIAN_MOVEMENT]	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"civilian_movement"));
	m_Switches[VEHICLE_MOVEMENT]	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"vehicle_movement"));
	m_Switches[ENEMY_FIRE]		= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"enemy_fire"));
	m_Switches[NEUTRAL_FIRE]	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"neutral_fire"));
	m_Switches[ALLY_FIRE]		= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"ally_fire"));
	m_Switches[CIVILIAN_FIRE]	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"civilian_fire"));
	m_Switches[VEHICLE_FIRE]	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"vehicle_fire"));
	//	---------------------------------------------------------------------

	Reset();
}

OptionsScreen::Camera::~Camera()
{
}

//	уведомление об изменении состояния системы
void OptionsScreen::Camera::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	сброс всех параметров
void OptionsScreen::Camera::Reset(void)
{
	m_Switches[ENEMY_MOVEMENT]->SetState(Options::GetInt("game.hidden_movement.enemy_related.move"));
	m_Switches[ENEMY_FIRE]->SetState(Options::GetInt("game.hidden_movement.enemy_related.shoot"));

	m_Switches[NEUTRAL_MOVEMENT]->SetState(Options::GetInt("game.hidden_movement.neutral_related.move"));
	m_Switches[NEUTRAL_FIRE]->SetState(Options::GetInt("game.hidden_movement.neutral_related.shoot"));

	m_Switches[ALLY_MOVEMENT]->SetState(Options::GetInt("game.hidden_movement.friend_related.move"));
	m_Switches[ALLY_FIRE]->SetState(Options::GetInt("game.hidden_movement.friend_related.shoot"));

	m_Switches[CIVILIAN_MOVEMENT]->SetState(Options::GetInt("game.hidden_movement.civilian_humans_traders.move"));
	m_Switches[CIVILIAN_FIRE]->SetState(Options::GetInt("game.hidden_movement.civilian_humans_traders.shoot"));

	m_Switches[VEHICLE_MOVEMENT]->SetState(Options::GetInt("game.hidden_movement.civilian_vehicles.move"));
	m_Switches[VEHICLE_FIRE]->SetState(Options::GetInt("game.hidden_movement.civilian_vehicles.shoot"));
}

void OptionsScreen::Camera::OnStatic(Static *pStatic)
{
	if(pStatic->LastEvent() == Static::LE_LBUTTONUP)
	{
		pStatic->SetState(!pStatic->GetState());
		//	---------------------------------------------------------------------
		switch(GetStaticId(pStatic))
		{
		case ENEMY_MOVEMENT:
			m_Updater->SetEnemyMovement(pStatic->GetState());
			break;
		case ENEMY_FIRE:
			m_Updater->SetEnemyFire(pStatic->GetState());
			break;
		case NEUTRAL_MOVEMENT:
			m_Updater->SetNeutralMovement(pStatic->GetState());
			break;
		case NEUTRAL_FIRE:
			m_Updater->SetNeutralFire(pStatic->GetState());
			break;
		case ALLY_MOVEMENT:
			m_Updater->SetAllyMovement(pStatic->GetState());
			break;
		case ALLY_FIRE:
			m_Updater->SetAllyFire(pStatic->GetState());
			break;
		case CIVILIAN_MOVEMENT:
			m_Updater->SetCivilianMovement(pStatic->GetState());
			break;
		case CIVILIAN_FIRE:
			m_Updater->SetCivilianFire(pStatic->GetState());
			break;
		case VEHICLE_MOVEMENT:
			m_Updater->SetVehicleMovement(pStatic->GetState());
			break;
		case VEHICLE_FIRE:
			m_Updater->SetVehicleFire(pStatic->GetState());
			break;
		}
	}
}

OptionsScreen::Camera::SWITCH OptionsScreen::Camera::GetStaticId(Static* s)
{
	for(int i=0;i<MAX_SWITCH;i++)
	{
		if(s == m_Switches[i])
		{
			return (SWITCH)i;
		}
	}

	return MAX_SWITCH;
}

void OptionsScreen::Camera::OnChange(Widget* pChild)
{
	Dialog::OnChange(pChild);
}

//***********************************************************************//
//**************** class OptionsScreen::SaveLoad ************************//
class SaveStrategy : public OptionsScreen::SaveLoad::Strategy
{
private:
	enum STATE {S_NONE,S_EDIT} m_State;
public:
	SaveStrategy(OptionsScreen::SaveLoad* owner) : Strategy(owner),m_State(S_NONE) {}
	void OnClick(void)
	{
		switch(m_State)
		{
		case S_NONE:
			break;
		case S_EDIT:
			m_Owner->EnableEdit(false);
			m_State = S_NONE;
			break;
		}
		m_Owner->SetSelectionToMouse();
	}
	void OnDblClk(void)
	{
		switch(m_State)
		{
		case S_NONE:
			m_Owner->SetSelectionToMouse();
			if(m_Owner->IsCurrentSlotAvailable())
			{
				m_Owner->EnableEdit(true);
				m_State = S_EDIT;
			}
			break;
		case S_EDIT:
			m_Owner->EnableEdit(false);
			m_Owner->EnableEdit(true);
			break;
		}
	}
	void OnReturn(void)
	{
		switch(m_State)
		{
		case S_NONE:
			if(m_Owner->IsCurrentSlotAvailable())
			{
				m_Owner->EnableEdit(true);
				m_State = S_EDIT;
			}
			else
			{//	уведомить о необходимости сохранить игру
				m_Owner->Report();
			}
			break;
		}
	}
	void OnUpDown(void)
	{
		switch(m_State)
		{
		case S_EDIT:
			m_Owner->EnableEdit(false);
			m_State = S_NONE;
			break;
		}
	}

};

class LoadStrategy : public OptionsScreen::SaveLoad::Strategy
{
public:
	LoadStrategy(OptionsScreen::SaveLoad* owner) : Strategy(owner) {}
	void OnClick(void){m_Owner->SetSelectionToMouse();}
	void OnDblClk(void){m_Owner->MakeLoad();}
	void OnReturn(void){m_Owner->MakeLoad();}
	void OnUpDown(void) {}
};

const char* OptionsScreen::SaveLoad::m_pDialogName = "SaveLoad";
const char* OptionsScreen::SaveLoad::m_SavesPath = "Saves";
const char* OptionsScreen::SaveLoad::m_SavesName = "save";
const char* OptionsScreen::SaveLoad::m_SavesExt = "sav";
const char* OptionsScreen::SaveLoad::m_DashName = "------------------";
const int OptionsScreen::SaveLoad::m_MaxLength = 18;
const int OptionsScreen::SaveLoad::m_MaxSlot = 8;
OptionsScreen::SaveLoad::SaveLoad() : Dialog(m_pDialogName,m_pDialogName),m_SelPos(0),m_Mode(M_NONE),m_Strategy(0),m_HidedSlot(0)
{
	char Buff[20];
	int left,top,right,bottom;

	VRegion slot_rgn = *WidgetFactory::Instance()->Assert(this,"slot")->GetLocation()->Region();
	for(int i=0;i<m_MaxSlot;i++)
	{
		Widget::Insert(m_Slots[i] = new Text(itoa(i,Buff,10)));
		Widget::Insert(m_Times[i] = new Text(itoa(i+100,Buff,10)));
		slot_rgn.GetRect(&left,&top,&right,&bottom);
		m_Slots[i]->SetFont("msg_b",0xffffff);
		m_Times[i]->SetFont("msg_b",0xffffff);
		m_Slots[i]->SetRegion(left,top,right,bottom);
		m_Times[i]->SetRegion(left,top,right,bottom);
		m_Slots[i]->Align(LogicalText::T_VCENTER);
		m_Times[i]->Align(LogicalText::T_VCENTER|LogicalText::T_RIGHT);
		slot_rgn.Transform(1,0,0,0,1,5+slot_rgn.Height());
	}
	//	выделение
	m_Selection = WidgetFactory::Instance()->Assert(this,"selection");
	BringToTop(m_Selection->Name());
	//	строка редактирования
	m_Edit = static_cast<Edit*>(WidgetFactory::Instance()->Assert(this,"edit"));
	m_Edit->Align(LogicalText::T_VCENTER|LogicalText::T_RIGHT);
	m_Edit->SetVisible(false);
	//	настраиваем меню
	FillSlots();
}

OptionsScreen::SaveLoad::~SaveLoad()
{
	if(m_Strategy)
	{
		delete m_Strategy;
		m_Strategy = 0;
	}
}

//	уведомление об изменении состояния системы
void OptionsScreen::SaveLoad::OnSystemChange(void)
{
	Widget::OnSystemChange();
}

void OptionsScreen::SaveLoad::FillSlots(void)
{
	char Buff[MAX_PATH];
	std::vector<FileInfo> fi;
	std::string desc;
	struct _stat file_info;
	struct tm* file_time;
	bool filled;
	Dir scaner;

	//	сканируем на наличие файлов
	sprintf(Buff,"%s/%s*.%s",m_SavesPath,m_SavesName,m_SavesExt);
	scaner.Scan(Buff);
	fi = scaner.Files();
	for(int i=0;i<m_MaxSlot;i++)
	{
		filled = false;
		sprintf(Buff,"%s%d",m_SavesName,i);
		for(std::vector<FileInfo>::iterator it = fi.begin();it!=fi.end();it++)
		{
			if(it->name == Buff)
			{
				sprintf(Buff,"%s/%s",m_SavesPath,it->full_name.c_str());				
				//	имя файла
				if(GetSaveDesc(Buff,desc))
				{
					m_Slots[i]->SetText(desc.c_str());
					//	дата создания
					_stat(Buff,&file_info);
					file_time = localtime(&file_info.st_mtime);
					sprintf(Buff,"%0.2d:%0.2d %0.2d/%0.2d/%0.2d",file_time->tm_hour,file_time->tm_min,file_time->tm_mday,file_time->tm_mon+1,file_time->tm_year%100);
					m_Times[i]->SetText(Buff);
					filled = true;
				}
				fi.erase(it);
				break;
			}
		}
		if(!filled)
		{
			if(i > 1) m_Slots[i]->SetText("");
			else m_Slots[i]->SetText(m_DashName);
			m_Times[i]->SetText("");
		}
	}
}
//	проверить существует ли сэйв
bool OptionsScreen::SaveLoad::IsResavingSelection(void)
{
	return m_Times[m_SelPos]->GetText().size();
}

bool OptionsScreen::SaveLoad::GetSaveDesc(const char* file_name,std::string& desc)
{
	try
	{
		StdFile file(file_name,"rb");
		Storage store(&file,Storage::SM_LOAD);	
		SavSlot name(&store,"description");
		
		name >> desc;
		
		return true;
	}
	catch(CasusImprovisus& /*ci*/)
	{
		return false;
	}
}

void OptionsScreen::SaveLoad::UpdateSelection(void)
{
	m_Selection->GetLocation()->Create(*m_Slots[m_SelPos]->GetLocation()->Region());
}

bool OptionsScreen::SaveLoad::ProcessMouseInput(VMouseState* pMS)
{
	if(pMS->LButtonFront)
	{
		m_Strategy->OnClick();
	}
	if(pMS->LDblClick)
	{
		m_Strategy->OnDblClk();
	}

	return true;
}
//	обработка клавиатурного ввода
bool OptionsScreen::SaveLoad::ProcessKeyboardInput(VKeyboardState* pKS)
{
	if(pKS->Back[DIK_UP])
	{
		if(m_SelPos>0)
		{
			m_SelPos--;
			UpdateSelection();
			m_Strategy->OnUpDown();
		}
		return false;
	}
	if(pKS->Back[DIK_DOWN])
	{
		if(m_SelPos<(m_MaxSlot-1))
		{
			m_SelPos++;
			UpdateSelection();
			m_Strategy->OnUpDown();
		}
		return false;
	}
	if(pKS->Back[DIK_RETURN] || pKS->Back[DIK_NUMPADENTER])
	{
		if(!m_Edit->IsVisible())
		{
			m_Strategy->OnReturn();
			return false;
		}
	}

	return true;
}
//	получить имя файла под курсором
bool OptionsScreen::SaveLoad::GetSelection(std::string& file_name,std::string& file_desc)
{
	if(m_Edit->IsVisible())
	{
		EnableEdit(false);
		m_Slots[m_HidedSlot]->SetText(m_Edit->GetText().c_str());
	}
	if(m_Slots[m_SelPos]->GetText().size())
	{//	слот не пустой
		if((m_SelPos>1) || m_Slots[m_SelPos]->GetText() != m_DashName)
		{//	слот не является прочерком
			char Buff[MAX_PATH];
			sprintf(Buff,"%s/%s%d.%s",m_SavesPath,m_SavesName,m_SelPos,m_SavesExt);	
			file_name = Buff;
			file_desc = m_Slots[m_SelPos]->GetText().c_str();
			
			return true;
		}
	}

	return false;
}
//	установить режим работы меню
void OptionsScreen::SaveLoad::SetMode(MODE mode)
{
	if(m_Strategy)
	{
		delete m_Strategy;
		m_Strategy = 0;
	}

	switch(m_Mode = mode)
	{
	case M_SAVE:
		m_Strategy = new SaveStrategy(this);
		m_Slots[0]->SetText(MenuMgr::Instance()->GetStrResourse("os_autosave").c_str());
		m_Slots[1]->SetText(MenuMgr::Instance()->GetStrResourse("os_quicksave").c_str());
		break;
	case M_LOAD:
		m_Strategy = new LoadStrategy(this);
		if(!m_Times[0]->GetText().size()) m_Slots[0]->SetText(m_DashName);
		if(!m_Times[1]->GetText().size()) m_Slots[1]->SetText(m_DashName);
		break;
	}
}
//	выравнять селекшион под мышку
void OptionsScreen::SaveLoad::SetSelectionToMouse(void)
{
	int x = Input::MouseState().x-GetLocation()->OriginX()-GetLocation()->Region()->X();
	int y = Input::MouseState().y-GetLocation()->OriginY()-GetLocation()->Region()->Y();

	for(int i=0;i<m_MaxSlot;i++)
	{
		if(m_Slots[i]->GetLocation()->Region()->PtInRegion(x,y))
		{
			m_SelPos = i;
			UpdateSelection();
			break;
		}
	}
}
//	выровнять строку редактирования под мышку
void OptionsScreen::SaveLoad::EnableEdit(bool enable)
{
	if(enable)
	{
		m_HidedSlot = m_SelPos;
		m_Slots[m_HidedSlot]->SetVisible(false);
		m_Times[m_HidedSlot]->SetVisible(false);
		m_Edit->SetText(m_Slots[m_HidedSlot]->GetText().c_str());
		m_Edit->MoveTo(m_Slots[m_HidedSlot]->GetLocation()->Region()->X(),
			m_Slots[m_HidedSlot]->GetLocation()->Region()->Y());
		m_Edit->SetVisible(true);
		//	передаем фокус ввода				
		Focus()->Set(WidgetFocus::FF_KEYBOARD,m_Edit);
		Parent()->Focus()->Set(WidgetFocus::FF_KEYBOARD,this);
	}
	else
	{
		m_Slots[m_HidedSlot]->SetVisible(true);
		m_Times[m_HidedSlot]->SetVisible(true);
		m_Edit->SetVisible(false);
	}
}

//	обработка сообщений от дочерних виджетов
void OptionsScreen::SaveLoad::OnChange(Widget* pChild)
{
	if(pChild == m_Edit)
	{
		EnableEdit(false);
		m_Slots[m_HidedSlot]->SetText(m_Edit->GetText().c_str());
		//	теперь надо передать команду на запись игры
		Report();
	}
}
//	уведомить родителя о том что необходимо сделать сейв/лоад
void OptionsScreen::SaveLoad::Report(void)
{
	if(m_Slots[m_SelPos]->GetText().size())
	{
		Parent()->OnChange(this);
	}
}
//	вернуть полное имя файла для сейва
std::string OptionsScreen::SaveLoad::GetFullFileNameOfSave(const char* save_file_name)
{
	char Buff[MAX_PATH];
	sprintf(Buff,"%s/%s.%s",m_SavesPath,save_file_name,m_SavesExt);	
	return std::string(Buff);
}
//	прерывание работы
void OptionsScreen::SaveLoad::OnCanceled(void)
{
	EnableEdit(false);
}
//	сделать загрузку
void OptionsScreen::SaveLoad::MakeLoad(void)
{
	Parent()->OnChange(this);
}
//	проверить не является ли текущий слот autosave или quicksave
bool OptionsScreen::SaveLoad::IsCurrentSlotAvailable(void)
{
	return (m_SelPos>1);
}

//***********************************************************************//
//	class OptionsScreen::TipOfTheDay
const char* OptionsScreen::TipOfTheDay::m_pDialogName = "TipOfTheDay";
const char* OptionsScreen::TipOfTheDay::m_pTipsScript = "Scripts/Interface/Tips.txt";
OptionsScreen::TipOfTheDay::TipOfTheDay() : Dialog(m_pDialogName,m_pDialogName)
{
	TxtFile script;

	//	читаем скрипт с подсказками
	script.Load(DataMgr::Load(m_pTipsScript));
	DataMgr::Release(m_pTipsScript);
	//	выбираем подсказку
	if(script.SizeY()>1)
	{
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"tip"))->SetText(script(rand()%(script.SizeY()-1),0).c_str());
	}
}

OptionsScreen::TipOfTheDay::~TipOfTheDay()
{
}
//	уведомление об изменении состояния системы
void OptionsScreen::TipOfTheDay::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	установить видимость виджета
void OptionsScreen::TipOfTheDay::SetVisible(bool bVisible)
{
	Dialog::SetVisible(bVisible);
	//	------------------------
	if(bVisible)
	{
		WidgetFactory::Instance()->Assert(Parent(),"ok")->SetVisible(false);
		WidgetFactory::Instance()->Assert(Parent(),"cancel")->SetVisible(false);
		WidgetFactory::Instance()->Assert(Parent(),"exit")->SetVisible(true);
		WidgetFactory::Instance()->Assert(Parent(),"back")->SetVisible(true);
	}
	else
	{
		WidgetFactory::Instance()->Assert(Parent(),"ok")->SetVisible(true);
		WidgetFactory::Instance()->Assert(Parent(),"cancel")->SetVisible(true);
		WidgetFactory::Instance()->Assert(Parent(),"exit")->SetVisible(false);
		WidgetFactory::Instance()->Assert(Parent(),"back")->SetVisible(false);
	}
}

/*/////////////////////////////////////////////////////////////////////////
ShadowQ - [0;6]
Gamma - [0;3]
*//////////////////////////////////////////////////////////////////////////
