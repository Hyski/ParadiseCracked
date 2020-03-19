/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   07.03.2001

************************************************************************/
#include "Precomp.h"
#include "../Common/UI/Static.h"
#include "../Common/FontMgr/Font.h"
#include "Text.h"
#include "TextBox.h"
#include "WidgetFactory.h"
#include "DXImageMgr.h"
#include "InventoryScreen.h"
#include "TradeScreen.h"
#include "JournalScreen.h"
#include "DialogScreen.h"
#include "LoadingScreen.h"
#include "OptionsScreen.h"
#include "DescriptionScreen.h"
#include "SlotsEngine.h"
#include "WidgetSoundMgr.h"
#include "MenuMgr.h"
#include "GameScreen.h"

//	Для использования логики
#include "../logic2/logicdefs.h"
#include "../logic2/entity.h"

//**********************************************************************
//	class WeaponIcon
class WeaponIcon : public Widget
{
private:
	Text* m_Text;
	bool m_IsNotVisible;			//	дополнительный флаг
public:
	WeaponIcon() : Widget("weapon_icon_item")
	{
		Insert(m_Text = new Text("text"));
		m_Text->Align(LogicalText::T_BOTTOM|LogicalText::T_RIGHT);
		m_Text->SetFont("oskar",0x00dd00);
		m_Text->Enable(false);
	}
	virtual ~WeaponIcon(){};
public:
	void SetText(const char* text)
	{
		m_Text->SetText(text);
		m_Text->SetRegion(0,
						  GetLocation()->Region()->Height()/2,
						  GetLocation()->Region()->Width(),
						  GetLocation()->Region()->Height());
	}
	void SetNotVisible(bool value) {m_IsNotVisible = value;};
	bool IsNotVisible(void) const {return m_IsNotVisible;};
};


//**********************************************************************
//	class GameScreen
//	суффиксы для иконки персонажа
const char* g_pIconNone = "_gray";
const char* g_pIconDied = "_dead";

const char* GameScreen::m_pDialogName = "GameScreen";
const char* GameScreen::m_WeaponIconName = "WeaponIcon";
GameScreen::GameScreen() : Dialog(m_pDialogName,m_pDialogName),m_pController(0),
						   m_bUnloadOptionsScreen(false),m_EnemyIconMgr(new EnemyIconMgr(this)),
						   m_HackVehicleMgr(new HackVehicleMgr(this)),
						   m_PersonIconMgr(new PersonIconMgr(this)),
						   m_IsCameraMoveable(true),m_CurrentActiveDilog(0)
{
	char pBuff[50];
	int i;

	//	добавляем иконку на оружие
	Insert(m_WeaponIcon = new WeaponIcon(),L_LEFT_BOTTOM);
	//	добавляем окно сообщений
	Insert(m_pMsgWindow = new MessageWindow(),L_RIGHT_BOTTOM);
	BringToBottom(m_pMsgWindow->Name());
	//	добавляем мониторы на технику
	for(i=0;i<6;i++)
	{
		sprintf(pBuff,"vm%d",i);
		Insert(m_pVehicleMonitors[i].m_pMonitor = new VehicleMonitor(pBuff),L_RIGHT_TOP);
		m_pVehicleMonitors[i].m_pMonitor->Move(0,(m_pVehicleMonitors[i].m_pMonitor->GetLocation()->Region()->Height()+2)*i);
		m_pVehicleMonitors[i].m_bVisible = false;
	}
	//	вставляем дополнительные игровые экраны
	Insert(new InventoryScreen(),L_CENTER);
	Insert(new TradeScreen(),L_CENTER);
	Insert(new JournalScreen(),L_CENTER);
	Insert(new DialogScreen(),L_CENTER);
	Insert(new HeroInfoScreen(),L_CENTER);
	Insert(new HelperScreen(),L_CENTER);
	Insert(new FlagDescScreen(),L_CENTER);
	Insert(new HiddenMovementScreen(),L_CENTER);
	Insert(new PanicScreen(),L_CENTER);
	Insert(new DescriptionScreen(),L_CENTER);
	//	инициализируем массив чек-боксов
	m_CheckBoxes[CB_RUN].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"run");
	m_CheckBoxes[CB_RUN].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"run_hot");
	m_CheckBoxes[CB_SIT].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"sit");
	m_CheckBoxes[CB_SIT].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"sit_hot");
	m_CheckBoxes[CB_SPECIAL].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"special");
	m_CheckBoxes[CB_SPECIAL].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"special_hot");
	m_CheckBoxes[CB_USE].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"use");
	m_CheckBoxes[CB_USE].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"use_hot");
	m_CheckBoxes[CB_ROOF].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"roof");
	m_CheckBoxes[CB_ROOF].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"roof");
	m_CheckBoxes[CB_JOURNAL].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"journal");
	m_CheckBoxes[CB_JOURNAL].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"journal");
	m_CheckBoxes[CB_ENDTURN].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"endturn");
	m_CheckBoxes[CB_ENDTURN].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"endturn_hot");
	m_CheckBoxes[CB_MENU].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"menu");
	m_CheckBoxes[CB_MENU].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"menu_hot");
	m_CheckBoxes[CB_FOS].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"visibility");
	m_CheckBoxes[CB_FOS].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"visibility");
	m_CheckBoxes[CB_AUTOSHOT].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"autoshot");
	m_CheckBoxes[CB_AUTOSHOT].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"autoshot");
	m_CheckBoxes[CB_SNAPSHOT].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"snapshot");
	m_CheckBoxes[CB_SNAPSHOT].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"snapshot");
	m_CheckBoxes[CB_AIMSHOT].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"aimshot");
	m_CheckBoxes[CB_AIMSHOT].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"aimshot");
	m_CheckBoxes[CB_BACKPACK].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"backpack");
	m_CheckBoxes[CB_BACKPACK].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"backpack");
	m_CheckBoxes[CB_RUN].m_HotArea->Enable(false);
	m_CheckBoxes[CB_SIT].m_HotArea->Enable(false);
	m_CheckBoxes[CB_SPECIAL].m_HotArea->Enable(false);
	m_CheckBoxes[CB_USE].m_HotArea->Enable(false);
	//	инициализируем массив кнопок (кнопки имеют три состояния: normal, pressed, grayed)
	m_Buttons[B_RELOAD] = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"reload"));
	m_Buttons[B_UP_ARROW] = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"up_arrow"));
	m_Buttons[B_DOWN_ARROW] = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"down_arrow"));
	//	----------------------------------------
	m_MPTextFields[MTF_AIM] = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"aimshot_mp"));
	m_MPTextFields[MTF_SNAP] = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"snapshot_mp"));
	m_MPTextFields[MTF_AUTO] = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"autoshot_mp"));
	//	----------------------------------------
	//	проверка на существование
	WidgetFactory::Instance()->Assert(this,"head");
	WidgetFactory::Instance()->Assert(this,"body");
	WidgetFactory::Instance()->Assert(this,"hands");	
	WidgetFactory::Instance()->Assert(this,"legs");	
	WidgetFactory::Instance()->Assert(this,"electric");
	WidgetFactory::Instance()->Assert(this,"shock");
	WidgetFactory::Instance()->Assert(this,"flame");	
	//	проверяем текущее разрешение
	for(Widget::Iterator wi=Begin();wi!=End();++wi)
		AlignWidget(&(*wi));
	CheckConformMode(); 
	//	обновляем иконки врага
	m_EnemyIconMgr->UpdateLocation();
	m_HackVehicleMgr->UpdateLocation();
}

GameScreen::~GameScreen()
{
	//	уничтожаем менеджер иконок врага
	delete m_EnemyIconMgr;
	//	уничтожаем менеджер иконок хакнутой техники
	delete m_HackVehicleMgr;
	//	уничтожаем менеджер иконок на персонажи
	delete m_PersonIconMgr;
}

//	производим какие-то действия
bool GameScreen::Tick(void)
{
	if(m_bUnloadOptionsScreen)
	{
		Delete(OptionsScreen::m_pDialogName);
		m_bUnloadOptionsScreen = false;
		return false;
	}
	//	обработка клавиатуры
	if(Input::KeyFront(DIK_F10))
	{//	быстрый выход  из игры
		if(MenuMgr::Instance()->IsQBFree())
		{
			if(MenuMgr::Instance()->QuestionBox(MenuMgr::Instance()->GetStrResourse("mm_quit").c_str()))
			{
				//	Napalm сказал, что теперь это не нужно
				//MakeSpecialSaveLoad(SST_AUTOSAVE,SSM_SAVE);
				MenuMgr::Instance()->ExitFromProgram();
			}
		}
	}
	//	передача управления менеджеру иконок персонажей
	m_PersonIconMgr->Tick();

	return true;
}
//	загрузить меню опций
void GameScreen::LoadOptionsScreen(void)
{
	//	создаем экран опций
	Insert(new OptionsScreen(),Dialog::L_CENTER);
	//	выравниваем под текущее разрешение
	AlignWidget(Child(OptionsScreen::m_pDialogName));
	//	устанавливаем режим работы опций
	static_cast<OptionsScreen*>(Child(OptionsScreen::m_pDialogName))->SetMode(OptionsScreen::M_GAMEMENU);
}
//	выгрузить меню опций
void GameScreen::UnloadOptionsScreen(void)
{
	m_bUnloadOptionsScreen = true;
}
//	сменить дочернее диалоговое окно (0 - убрать все окна)
void GameScreen::ShowDialog(const char *pName)
{
	if(!pName)
	{
		if(Focus()->Get(WidgetFocus::FF_KEYBOARD))
			Focus()->Get(WidgetFocus::FF_KEYBOARD)->SetVisible(false);
		for(Widget::Iterator it=Begin();it!=End();it++)
			it->Focus()->Enable(WidgetFocus::FF_MOUSE,true);
		//	дополнительная проверка
		if(m_CurrentActiveDilog) { m_CurrentActiveDilog->SetVisible(false); m_CurrentActiveDilog = 0; }
		//	изменяем флаг перемещаемости камеры
		m_IsCameraMoveable = true;
	}
	else
	{
		Widget* prev_dialog = Focus()->Get(WidgetFocus::FF_KEYBOARD);

		if(Focus()->Get(WidgetFocus::FF_KEYBOARD) != WidgetFactory::Instance()->Assert(this,pName))
		{
			for(Widget::Iterator it=Begin();it!=End();it++)
			{
				//	проверка на экран описания героя
				if(strcmp(it->Name(),HeroInfoScreen::m_pDialogName))
					it->Focus()->Enable(WidgetFocus::FF_MOUSE,false);
			}
			Child(pName)->SetVisible(true);
			Child(pName)->Focus()->Enable(WidgetFocus::FF_MOUSE,true);
			Focus()->Set(WidgetFocus::FF_KEYBOARD,Child(pName));
			//	дополнительная проверка
			m_CurrentActiveDilog = Child(pName);
		}
		//	проверка на экран описания 
		if(!strcmp(pName,DescriptionScreen::m_pDialogName))
		{
			if(prev_dialog) prev_dialog->SetVisible(true);
			BringToTop(pName);
		}
		//	изменяем флаг перемещаемости камеры
		m_IsCameraMoveable = false;
	}
}
//	получить утверждение, что камеру можно перемещать
bool GameScreen::IsCameraMoveable(void)
{
	Widget* child = FirstChildFromPoint(Input::MouseState().x,Input::MouseState().y);

	if(child)
	{
		if((child == MsgWindow()) ||
		   (!strcmp(child->Name(),"weapon_icon")))
		{
			return false;
		}
	}

	return m_IsCameraMoveable;
}
//	спрятать/показать элемнеты интерфейса
void GameScreen::ShowControls(bool bShow)
{
	for(Widget::Iterator it=Begin();it!=End();it++)
	{
		if(strcmp(it->Type(),"dialog")!=0)
			it->SetVisible(bShow);
	}
	MsgWindow()->SetVisible(bShow);
	if(bShow)
	{
		m_WeaponIcon->SetVisible(!m_WeaponIcon->IsNotVisible());
		for(int i=0;i<6;i++)
			m_pVehicleMonitors[i].m_pMonitor->SetVisible(m_pVehicleMonitors[i].m_bVisible);
	}
}
//	выйти в главное меню
void GameScreen::ExitToMainMenu(void)
{
	Parent()->OnChange(this);
}

void GameScreen::OnChange(Widget* pChild)
{
	if(pChild == Child(OptionsScreen::m_pDialogName))
	{//	OptionsScreen была нажата кнопка Exit или производится запись/чтение
		switch(static_cast<OptionsScreen*>(pChild)->GetParentNotify())
		{
		case OptionsScreen::PN_EXIT:
			//	сохраняем игру в AUTOSAVE
			//	Napalm сказал, что теперь это не нужно
			//MakeSpecialSaveLoad(SST_AUTOSAVE,SSM_SAVE);
		case OptionsScreen::PN_SAVELOAD:
			//	выходим из меню
			ExitToMainMenu();
			break;
		}

		return;
	}
	if(m_pController)
	{
		if(!strcmp(pChild->Type(),"personmonitor"))
		{
			for(int i=0;i<6;i++)
			{
				if(m_PersonIconMgr->IconAt((int)i))
				{
					if(reinterpret_cast<Widget*>(m_PersonIconMgr->IconAt((int)i)->GetMonitor()) == pChild)
					{
						GameScreen::BN_CLICK clk = GameScreen::BNC_NONE;
						
						if(Input::MouseState().LDblClick)
							clk = GameScreen::BNC_LDBLCLICK;
						else
						{
							if(Input::MouseState().LButtonBack)
								clk = GameScreen::BNC_LCLICK;
						}
						if(clk != GameScreen::BNC_NONE)
							m_pController->OnIconClick(i,clk);
						else
						{
							if(Input::MouseState().RDblClick)
								clk = GameScreen::BNC_RDBLCLICK;
							else
							{
								if(Input::MouseState().RButtonBack)
									clk = GameScreen::BNC_RCLICK;
							}
							m_pController->OnIconClick(i,clk);
						}
						return;
					}
				}
			}
		}
		if(!strcmp(pChild->Type(),"vehiclemonitor"))
		{
			for(int i=0;i<6;i++)
			{
				if(m_pVehicleMonitors[i].m_pMonitor == pChild)
				{
					GameScreen::BN_CLICK clk = GameScreen::BNC_NONE;

					if(Input::MouseState().LDblClick)
						clk = GameScreen::BNC_LDBLCLICK;
					else
					{
						if(Input::MouseState().LButtonBack)
							clk = GameScreen::BNC_LCLICK;
					}
					if(clk != GameScreen::BNC_NONE)
						m_pController->OnVehicleIconClick(i,clk);
					else
					{
						if(Input::MouseState().RDblClick)
							clk = GameScreen::BNC_RDBLCLICK;
						else
						{
							if(Input::MouseState().RButtonBack)
								clk = GameScreen::BNC_RCLICK;
						}
						m_pController->OnVehicleIconClick(i,clk);
					}
					return;
				}
			}
		}
		else
		{//	далее
			Dialog::OnChange(pChild);
		}
	}
}

void GameScreen::OnStatic(Static *pStatic)
{
	int index;

	for(index=0;index<MAX_CHECKBOX;index++)
	{
		if(m_CheckBoxes[index].m_HotArea == pStatic)
			break;
	}
	if(index != MAX_CHECKBOX)
	{
		if(pStatic->LastEvent() == Static::LE_LBUTTONDOWN)
			m_pController->OnCheckBoxClick(static_cast<CHECKBOX>(index));
		return;
	}
	for(index=0;index<MAX_BUTTON;index++)
	{
		if(m_Buttons[index] == pStatic)
		{
			switch(pStatic->LastEvent())
			{
			case Static::LE_LBUTTONPUSHED:
				if(pStatic->GetState()==0) pStatic->SetState(1);
				break;
			case Static::LE_MOUSELOST:
				pStatic->SetState(0);
				break;
			case Static::LE_LBUTTONDOWN:
				pStatic->SetState(1);
				break;
			case Static::LE_LBUTTONUP:
				pStatic->SetState(0);
				if(m_pController) m_pController->OnButtonClick((BUTTON)index);
				break;
			}

			return;
		}
	}
	if(!strcmp(pStatic->Name(),"message_up"))
	{
		switch(pStatic->LastEvent())
		{
		case Static::LE_LBUTTONDOWN:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_control"))->SetState(2);
			m_pMsgWindow->IncMsgWindow();
			break;
		case Static::LE_LBUTTONUP:
		case Static::LE_MOUSELOST:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_control"))->SetState(0);
			break;
		}
		return;
	}
	if(!strcmp(pStatic->Name(),"message_down"))
	{
		switch(pStatic->LastEvent())
		{
		case Static::LE_LBUTTONDOWN:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_control"))->SetState(1);
			m_pMsgWindow->DecMsgWindow();
			break;
		case Static::LE_LBUTTONUP:
		case Static::LE_MOUSELOST:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_control"))->SetState(0);
			break;
		}
		return;
	}
	if(!strcmp(pStatic->Name(),"weapon_icon"))
	{
		if(!m_WeaponIcon->IsNotVisible() && 
		  (pStatic->LastEvent() == Static::LE_LBUTTONDOWN || 
		   pStatic->LastEvent() == Static::LE_RBUTTONDOWN))
		{
			if(m_pController) m_pController->OnWeaponIconClick();
		}

		return;
	}
}

/****************** ВНЕШНИЙ ИНТЕРФЕЙС С ЛОГИКОЙ *********************/
//	установить состояние для кнопки входа в журнал
void GameScreen::SetJournalButtonState(JOURNAL_BUTTON_STATE jbs)
{
	m_CheckBoxes[CB_JOURNAL].m_VisualArea->SetState(jbs);
}
// переключение кнопки в заданное состояние
void GameScreen::SetCBState(CHECKBOX id,CHECKBOX_STATE state)
{
	if(m_CheckBoxes[id].m_HotArea->IsEnable())
		m_CheckBoxes[id].m_VisualArea->SetState(state);
}
// получить состояние кнопки	
GameScreen::CHECKBOX_STATE GameScreen::GetCBState(CHECKBOX id)
{
	return (CHECKBOX_STATE)m_CheckBoxes[id].m_VisualArea->GetState();
}
// переключение кнопки в состояние Enable/Disable
void GameScreen::EnableCBState(CHECKBOX id,bool bEnable)
{
	if(!bEnable) m_CheckBoxes[id].m_VisualArea->SetState(2);
	else m_CheckBoxes[id].m_VisualArea->SetState(0);
	m_CheckBoxes[id].m_HotArea->Enable(bEnable);
}
// переключение кнопки в состояние Enable/Disable
void GameScreen::EnableButtonState(BUTTON id,bool bEnable)
{
	if(!bEnable) m_Buttons[id]->SetState(2);
	else m_Buttons[id]->SetState(0);
	m_Buttons[id]->Enable(bEnable);
}
//	установить параметры для иконки отображения состояния персонажа
void GameScreen::SetBodyIcon(const BodyIconTraits& bit)
{
	((Static *)Child("head"))->SetState(bit.m_Head);	
	((Static *)Child("body"))->SetState(bit.m_Body);	
	((Static *)Child("hands"))->SetState(bit.m_Hands);	
	((Static *)Child("legs"))->SetState(bit.m_Legs);	
	//	иконки поражения
	static_cast<Static *>(Child("electric"))->SetState((bit.m_Damages&BodyIconTraits::DT_ELECTRIC)?1:0);	
	static_cast<Static *>(Child("shock"))->SetState((bit.m_Damages&BodyIconTraits::DT_SHOCK)?1:0);	
	static_cast<Static *>(Child("flame"))->SetState((bit.m_Damages&BodyIconTraits::DT_FLAME)?1:0);	
}
//	установить параметры для иконки техники
void GameScreen::SetVehicleIcon(unsigned int icon,const VehicleIconTraits& vit)
{
	//	видим/не видим
	m_pVehicleMonitors[icon].m_pMonitor->SetVisible(vit.m_Visible);
	m_pVehicleMonitors[icon].m_bVisible = vit.m_Visible;
	//	тип параметра
	m_pVehicleMonitors[icon].m_pMonitor->SetMaxCrew(vit.m_MaxCrew);
	m_pVehicleMonitors[icon].m_pMonitor->SetNumCrew(vit.m_NumCrew);
	m_pVehicleMonitors[icon].m_pMonitor->SetMovePoints(vit.m_MovePoints);
	m_pVehicleMonitors[icon].m_pMonitor->SetHealth(vit.m_HitPoints);
	//	состояние иконки (флаги) VEHICLE_ICON_FLAG
	m_pVehicleMonitors[icon].m_pMonitor->EnableInMovement(vit.m_Flags&VehicleIconTraits::VIF_INMOVEMENT);
	//	тип выделения
	((Static *)WidgetFactory::Instance()->Assert(m_pVehicleMonitors[icon].m_pMonitor,"icon"))->SetState(vit.m_Selection);
	//	обновляем иконки врага
	m_EnemyIconMgr->UpdateLocation();

}
//	установить сотояние для иконки (0 - не отображать иконку)
void GameScreen::SetWeaponIcon(const WeaponIconTraits* traits)
{
	Text* ammo_desc = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"ammo_desc"));

	if(traits)
	{
		if(traits->m_ImageName && strlen(traits->m_ImageName) && WidgetFactory::Instance()->Assert(this,"metal")->IsVisible())
		{
			std::string image_name = std::string(g_InventoryPictures)+std::string(traits->m_ImageName)+".tga";
			
			m_WeaponIcon->SetVisible(true);
			m_WeaponIcon->SetNotVisible(false);
			if(!m_WeaponIcon->GetImage() || !(image_name == static_cast<DXImage*>(m_WeaponIcon->GetImage())->m_Resource))
			{
				int left,top,right,bottom;
				int width,height;
				int x,y;
				
				//	устанавливаем картинку
				if(m_WeaponIcon->GetImage()) m_WeaponIcon->GetImage()->Release();
				m_WeaponIcon->SetImage(DXImageMgr::Instance()->CreateImage(image_name.c_str()));
				DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_WeaponIcon->GetImage()),&width,&height);
				WidgetFactory::Instance()->Assert(this,"weapon_icon")->GetLocation()->Region()->GetRect(&left,&top,&right,&bottom);
				x = left + ((right-left)-width)/2;
				y = top + ((bottom-top)-height)/2;
				m_WeaponIcon->GetLocation()->Create(x,y,x+width,y+height);
			}
			BringToBottom(m_WeaponIcon->Name());
			BringToBottom(WidgetFactory::Instance()->Assert(this,"metal")->Name());
			BringToBottom(m_pMsgWindow->Name());
		}
		m_WeaponIcon->SetText(traits->m_AmmoQuantity);
		ammo_desc->SetText(traits->m_AmmoDesc);
	}
	else
	{
		m_WeaponIcon->SetVisible(false);
		m_WeaponIcon->SetNotVisible(true);
		ammo_desc->SetText("");
	}
}
//	проверить, находитсяли курсор на иконке оружия
bool GameScreen::IsCursorOnWeaponIcon(void)
{
	Widget* child = FirstChildFromPoint(Input::MouseState().x, Input::MouseState().y);
	
	return (child && !strcmp(child->Name(),"weapon_icon"));
}
//	установить значение текстовых полей для отображения мувпоинтов
void GameScreen::SetShootMovePoints(const ShootMovePoints& smp)
{
	m_MPTextFields[MTF_AIM]->SetFont(m_MPTextFields[MTF_AIM]->GetFontName(),smp.m_AimColor);
	m_MPTextFields[MTF_SNAP]->SetFont(m_MPTextFields[MTF_SNAP]->GetFontName(),smp.m_SnapColor);
	m_MPTextFields[MTF_AUTO]->SetFont(m_MPTextFields[MTF_AUTO]->GetFontName(),smp.m_AutoColor);
	m_MPTextFields[MTF_AIM]->SetText(smp.m_AimMP);
	m_MPTextFields[MTF_SNAP]->SetText(smp.m_SnapMP);
	m_MPTextFields[MTF_AUTO]->SetText(smp.m_AutoMP);
}
//	установить контроллер логики для интерфейса
void GameScreen::SetController(GameScreenController *pController)
{
	m_pController = pController;
}
//	для работы со скриптовыми сценами
void GameScreen::OpenScriptSceneScreen(void)
{
	if(!Child(ScriptSceneScreen::m_pDialogName))
	{
		Insert(new ScriptSceneScreen(),L_CENTER);
	}
}

void GameScreen::CloseScriptSceneScreen(void)
{
	Delete(ScriptSceneScreen::m_pDialogName);
}

int GameScreen::GetEnemyIconBottom(void)
{
	return GetLocation()->Region()->Height()-170;
}

int GameScreen::GetEnemyIconTop(void)
{
	int i;
	for(i=0;i<5;i++)
	{
		if(!m_pVehicleMonitors[i].m_bVisible)
			break;
	}

	return std::min(350,m_pVehicleMonitors[i].m_pMonitor->GetLocation()->Region()->Y());
}
//	уведомление об изменении состояния системы
void GameScreen::OnSystemChange(void)
{
	Dialog::OnSystemChange();
	//	обновляем иконки врага
	m_EnemyIconMgr->UpdateLocation();
	m_HackVehicleMgr->UpdateLocation();
}

void GameScreen::OnEnemyIconClick(Marker* marker)
{
	if(m_pController)
	{
		m_pController->OnEnemyIconClick(marker);
	}
}

int GameScreen::GetHackVehicleIconRight(void)
{
	return GetLocation()->Region()->Width()-m_pVehicleMonitors[0].m_pMonitor->GetLocation()->Region()->Width()-50;
}

void GameScreen::OnHackVehicleIconClick(VehicleEntity* ve,GameScreen::BN_CLICK click_type)
{
	if(m_pController)
	{
		m_pController->OnHackVehicleClick(ve,click_type);
	}
}
	//	записать/прочитать файл "автосохранение" и "быстрое сохранение" (если файла нет return false)
bool GameScreen::MakeSpecialSaveLoad(SPECIAL_SAVELOAD_TYPE type,SPECIAL_SAVELOAD_MODE mode)
{
	std::string file_name;
	std::string file_desc;

	switch(type)
	{
	case SST_AUTOSAVE:
		file_name = OptionsScreen::GetFullFileNameOfSave("save0");
		file_desc = MenuMgr::Instance()->GetStrResourse("os_autosave");
		break;
	case SST_QUICKSAVE:
		file_name = OptionsScreen::GetFullFileNameOfSave("save1");
		file_desc = MenuMgr::Instance()->GetStrResourse("os_quicksave");
		break;
	}

	switch(mode)
	{
	case SSM_SAVE:
		{
			//	изменить модификатор
			MenuMgr::Instance()->LoadSaveGameModifier()->SetSaveGameName(file_name.c_str());
			MenuMgr::Instance()->LoadSaveGameModifier()->SetSaveGameDesc(file_desc.c_str());
			MenuMgr::Instance()->LoadSaveGameModifier()->SetAvailable(MenuMgr::ic_LoadSaveGameModifier::A_SAVE);
			Parent()->OnChange(this);
			
			return true;
		}
	case SSM_LOAD:
		{
			//	проверка на существование файла
			VFile* test_file = DataMgr::Load(file_name.c_str());
			bool result = test_file->Size();
			
			DataMgr::Release(file_name.c_str());
			if(result)
			{
				//	изменить модификатор
				MenuMgr::Instance()->LoadSaveGameModifier()->SetSaveGameName(file_name.c_str());
				MenuMgr::Instance()->LoadSaveGameModifier()->SetSaveGameDesc(file_desc.c_str());
				MenuMgr::Instance()->LoadSaveGameModifier()->SetAvailable(MenuMgr::ic_LoadSaveGameModifier::A_LOAD);
				Parent()->OnChange(this);
			}
			
			return result;
		}
	}

	return false;
}

//=====================================================================================//
//                    class GameScreen::MessageWindow::StringBuffer                    //
//=====================================================================================//
class GameScreen::MessageWindow::StringBuffer
{
private:

	typedef std::list<unsigned int> lengthes_t;
	lengthes_t m_lengthes;							//	длины строк

	unsigned int m_capacity;						//	максимальное кол-во строк, хранимых в буфере
	std::string m_content;
public:
	StringBuffer(unsigned int capacity) : m_capacity(capacity) {}
	~StringBuffer() {}
public:
	//	добавить строчку 
	void addString(const char* str);
	//	получить готовый текст
	const char* getContent(void) const;
	//	очистить буфер
	void clear(void);

};

//	добавить строчку 
void GameScreen::MessageWindow::StringBuffer::addString(const char* str)
{
	//	проверяем на переполнение
	if(m_lengthes.size() == m_capacity)
	{
		const lengthes_t::iterator it = m_lengthes.begin();
		m_content.erase(0,*it);
		m_lengthes.erase(it);
	}
	//	добавляем строку
	m_content += std::string(str) + std::string("\n");
	m_lengthes.push_back(strlen(str)+1);
}
//	получить готовый текст
const char* GameScreen::MessageWindow::StringBuffer::getContent(void) const
{
	return m_content.c_str();
}
//	очистить буфер
void GameScreen::MessageWindow::StringBuffer::clear(void)
{
	m_content.clear();
	m_lengthes.clear();
}

//**********************************************************************//
//*********	              class MessageWindow              *************//	
GameScreen::MessageWindow::MessageWindow() : Dialog("MessageWindow","MessageWindow"),
											 m_MovingStep(150),
											 m_StringBuffer(new StringBuffer(300))
{
	SetVisible(true);
	Focus()->Enable(WidgetFocus::FF_KEYBOARD,false);
	//	настраиваем параметры свертки/развертки
	m_iMaxHeight = GetLocation()->Region()->Y();
	m_iMinHeight = m_iMaxHeight+GetLocation()->Region()->Height();
	m_iCurHeight = m_iMaxHeight;
	m_iAimHeight = m_iMinHeight;
	//	----------------------------------------
	CheckConformMode(); 
	//	----------------------------------------
	m_MessageBox = (TextBox *)WidgetFactory::Instance()->Assert(this,"message_box");
	m_iMessageBoxHeight = m_MessageBox->GetLocation()->Region()->Height();
}

GameScreen::MessageWindow::~MessageWindow()
{
}
//	двигаем окно, если требуется
bool GameScreen::MessageWindow::Tick(void)
{
	static float seconds = Timer::GetSeconds();
	float step;

	if(m_iCurHeight != m_iAimHeight)
	{
		step = (Timer::GetSeconds()-seconds)*m_MovingStep;
		if(m_iCurHeight > m_iAimHeight)
		{//	поднимаем окно
			step = -std::min(m_iCurHeight-m_iAimHeight,step);
		}
		else
		{//	опускаем окно
			step = std::min(m_iAimHeight-m_iCurHeight,step);
		}
		m_iCurHeight += step;
		MoveTo(GetLocation()->Region()->X(),m_iCurHeight);
		//	изменяем размеры message_box'а
		int old_height = m_MessageBox->GetLocation()->Region()->Height();
		m_MessageBox->SetRegion(m_MessageBox->GetLocation()->Region()->X(),
								m_MessageBox->GetLocation()->Region()->Y(),
								m_MessageBox->GetLocation()->Region()->X()+m_MessageBox->GetLocation()->Region()->Width(),
								m_MessageBox->GetLocation()->Region()->Y()+std::max(static_cast<int>(m_MessageBox->GetFont()->Height()),m_iMessageBoxHeight-static_cast<int>(m_iCurHeight-m_iMaxHeight)));
		if(m_MessageBox->IsScrollEnd())
			m_MessageBox->Scroll(TextBox::SD_END);
		else
			m_MessageBox->Scroll((int)m_MessageBox->GetLocation()->Region()->Height()-old_height);
	}
	seconds = Timer::GetSeconds();

	return true;
}

void GameScreen::MessageWindow::OnStatic(Static *pStatic)
{
	if(!strcmp(pStatic->Name(),"message_scroll_up"))
	{
		switch(pStatic->LastEvent())
		{
		case Static::LE_LBUTTONDOWN:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_scroll"))->SetState(2);
			m_MessageBox->Scroll(TextBox::SD_UP,true);
			break;
		case Static::LE_LBUTTONPUSHED:
			m_MessageBox->Scroll(TextBox::SD_UP,true);
			break;
		case Static::LE_LBUTTONUP:
		case Static::LE_MOUSELOST:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_scroll"))->SetState(0);
			break;
		}
		return;
	}
	if(!strcmp(pStatic->Name(),"message_scroll_down"))
	{
		switch(pStatic->LastEvent())
		{
		case Static::LE_LBUTTONDOWN:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_scroll"))->SetState(1);
			m_MessageBox->Scroll(TextBox::SD_DOWN,true);
			break;
		case Static::LE_LBUTTONPUSHED:
			m_MessageBox->Scroll(TextBox::SD_DOWN,true);
			break;
		case Static::LE_LBUTTONUP:
		case Static::LE_MOUSELOST:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_scroll"))->SetState(0);
			break;
		}
		return;
	}
}

void GameScreen::MessageWindow::CheckConformMode(void)
{
	int dy = D3DKernel::ResY() - m_ResY;

	m_ResY = D3DKernel::ResY();
	//	настраиваем параметры свертки/развертки
	m_iMaxHeight += dy;
	m_iMinHeight += dy;
	m_iCurHeight += dy;
	m_iAimHeight += dy;
}

//	увеличить видимость окна
void GameScreen::MessageWindow::IncMsgWindow(void)
{
	m_iAimHeight = std::max(m_iMaxHeight,m_iAimHeight-(m_iMinHeight-m_iMaxHeight)/2);
}
//	уменьшить видимость окна
void GameScreen::MessageWindow::DecMsgWindow(void)
{
	m_iAimHeight = std::min(m_iMinHeight,m_iAimHeight+(m_iMinHeight-m_iMaxHeight)/2);
}
//	добавить текст в окно
void GameScreen::MessageWindow::AddText(const char* pText)
{
	m_StringBuffer->addString(pText);
	m_MessageBox->SetText(m_StringBuffer->getContent());
	m_MessageBox->Scroll(TextBox::SD_END);
}
//	очистить окошко
void GameScreen::MessageWindow::Clear(void)
{
	m_StringBuffer->clear();
	m_MessageBox->SetText("");
}

//**********************************************************************//
//*********	              class VehicleMonitor             *************//
const char* GameScreen::VehicleMonitor::m_pScriptName = "VehicleMonitor";
const int GameScreen::VehicleMonitor::m_PlacesX = 5;
const int GameScreen::VehicleMonitor::m_PlacesY = 18;
const int GameScreen::VehicleMonitor::m_PlacesWidth = 7;
const int GameScreen::VehicleMonitor::m_PlacesHeight = 11;
GameScreen::VehicleMonitor::VehicleMonitor(const char *pName) : Dialog(pName,m_pScriptName)
{
	Focus()->Enable(WidgetFocus::FF_KEYBOARD,false);
	//	--------------------------------------------
	//	создаем иконки размещения
	char pBuff[50];
	for(int i=0;i<MAX_PLACES;i++)
	{
		sprintf(pBuff,"place%d",i);
		Widget::Insert(m_Places[i] = new Static(pBuff));
		int left = m_PlacesX+(m_PlacesWidth+1)*(i&1);
		int top  = m_PlacesY+(m_PlacesHeight+1)*(i/2);
		m_Places[i]->GetLocation()->Create(left,top,left+m_PlacesWidth,top+m_PlacesHeight);
		m_Places[i]->AddImage(DXImageMgr::Instance()->CreateImage("pictures/interface/GameScreen/place_empty.tga"));
		m_Places[i]->AddImage(DXImageMgr::Instance()->CreateImage("pictures/interface/GameScreen/place.tga"));
		m_Places[i]->SetVisible(false);
	}
}

GameScreen::VehicleMonitor::~VehicleMonitor()
{
}

void GameScreen::VehicleMonitor::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	сообщение от статика
void GameScreen::VehicleMonitor::OnStatic(Static *pStatic)
{
	if(Parent() && !strcmp("icon",pStatic->Name()) && (pStatic->LastEvent() != Static::LE_NONE))
		Parent()->OnChange(this);
}

void GameScreen::VehicleMonitor::SetMaxCrew(int value)
{
	value = std::min(value,(int)MAX_PLACES);
	for(int i=0;i<MAX_PLACES;i++)
	{
		if(i<value)
		{
			m_Places[i]->SetState(0);
			m_Places[i]->SetVisible(true);
		}
		else
		{
			m_Places[i]->SetVisible(false);
		}
	}
}

void GameScreen::VehicleMonitor::SetNumCrew(int value)
{
	value = std::min(value,(int)MAX_PLACES);
	for(int i=0;i<value;i++)
		m_Places[i]->SetState(1);
}

void GameScreen::VehicleMonitor::SetMovePoints(int value)
{
	static char pBuff[20];

	itoa(value,pBuff,10);
	((Text* )WidgetFactory::Instance()->Assert(this,"movepoints"))->SetText(pBuff);
}

void GameScreen::VehicleMonitor::SetHealth(int value)
{
	static char pBuff[20];

	itoa(value,pBuff,10);
	((Text* )WidgetFactory::Instance()->Assert(this,"health"))->SetText(pBuff);
}

void GameScreen::VehicleMonitor::EnableInMovement(bool /*bEnable*/)
{
//	WidgetFactory::Instance()->Assert(this,"inmovement")->SetVisible(bEnable);
}

//**********************************************************************//
//	class GameScreen::EnemyIconMgr
int GameScreen::EnemyIconMgr::m_Bottom = 0;							
int GameScreen::EnemyIconMgr::m_Top = 0;							
GameScreen::EnemyIconMgr::EnemyIconMgr(GameScreen* owner) : m_Owner(owner)
{
}

GameScreen::EnemyIconMgr::~EnemyIconMgr()
{
	if(m_Icons.size()) Clear();
}

void GameScreen::EnemyIconMgr::Insert(EnemyIcon& ei)
{
	EnemyIconExtended* eie = new EnemyIconExtended(m_Owner,ei);

	eie->MoveTo(GetNextLocation(m_Icons.size()));
	eie->SetNumber(m_Icons.size()+1);

	m_Icons.push_back(eie);
}

void GameScreen::EnemyIconMgr::Remove(Marker* marker)
{
	for(std::list<EnemyIconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if((*i)->GetMarker() == marker)
		{
			//	удаляем итем
			delete (*i);
			m_Icons.erase(i);
			//	обновляем позиции других итемов
			UpdateLocation();
			break;
		}
	}
}

void GameScreen::EnemyIconMgr::Clear(void)
{
	for(std::list<EnemyIconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		delete (*i);
	m_Icons.clear();
}

GameScreen::EnemyIconMgr::EnemyIcon* GameScreen::EnemyIconMgr::ItemAt(const Marker* marker)
{
	for(std::list<EnemyIconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if((*i)->GetMarker() == marker)
		{
			return (*i);
		}
	}

	return 0;
}

GameScreen::EnemyIconMgr::EnemyIcon* GameScreen::EnemyIconMgr::ItemAt(unsigned int number)
{
	std::list<EnemyIconExtended*>::iterator i = m_Icons.begin();
	
	while((i!=m_Icons.end()) && number)
	{
		i++;
		number--;
	}
	if(i!=m_Icons.end()) return (*i);

	return 0;
}

ipnt2_t GameScreen::EnemyIconMgr::GetNextLocation(unsigned int size)
{
	const int width = GameScreen::EnemyIconMgr::EnemyIconAsWidget::Width();
	const int height = GameScreen::EnemyIconMgr::EnemyIconAsWidget::Height();

	const int in_column = (m_Bottom-m_Top)/height;
	const int width_factor = size/in_column;
	const int height_factor = size-(width_factor*in_column);

	return ipnt2_t(m_Owner->GetLocation()->Region()->Width()-(width*(width_factor+1)),m_Bottom-height*(height_factor+1));
}

void GameScreen::EnemyIconMgr::UpdateLocation(void)
{
	int size = 0;

	m_Bottom = m_Owner->GetEnemyIconBottom();
	m_Top	 = m_Owner->GetEnemyIconTop();
	for(std::list<EnemyIconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++,size++)
	{
		(*i)->MoveTo(GetNextLocation(size));
		(*i)->SetNumber(size+1);
	}
}
//	установить флаги для всех иконок стразу
void GameScreen::EnemyIconMgr::SetFlags(unsigned int flags)
{
	for(std::list<EnemyIconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		(*i)->SetFlags(flags);
}

//**********************************************************************//
//	class GameScreen::EnemyIconMgr::EnemyIcon
GameScreen::EnemyIconMgr::EnemyIcon::EnemyIcon(const EnemyIcon& ei) : m_Tip(ei.m_Tip),m_Marker(ei.m_Marker),m_Flags(ei.m_Flags)
{
}

GameScreen::EnemyIconMgr::EnemyIcon::EnemyIcon(const std::string& tip,Marker* marker,unsigned int flags) : m_Tip(tip),m_Marker(marker),m_Flags(flags)
{
}

GameScreen::EnemyIconMgr::EnemyIcon::~EnemyIcon()
{
}

std::string GameScreen::EnemyIconMgr::EnemyIcon::GetTip(void) const
{
	return m_Tip;
}

unsigned int GameScreen::EnemyIconMgr::EnemyIcon::GetFlags(void) const
{
	return m_Flags;
}

void GameScreen::EnemyIconMgr::EnemyIcon::SetFlags(unsigned int flags)
{
	m_Flags = flags;
}

Marker* GameScreen::EnemyIconMgr::EnemyIcon::GetMarker(void) const
{
	return m_Marker;
}

//**********************************************************************
//	class GameScreen::EnemyIconMgr::EnemyIconExtended
GameScreen::EnemyIconMgr::EnemyIconExtended::EnemyIconExtended(GameScreen* owner,const GameScreen::EnemyIconMgr::EnemyIcon& ei) : GameScreen::EnemyIconMgr::EnemyIcon(ei),m_Owner(owner)
{
	m_Owner->Insert(m_Icon = new EnemyIconAsWidget(this),L_RIGHT_BOTTOM);
	m_Owner->BringToBottom(m_Icon->Name());
	SetFlags(m_Flags);
}

GameScreen::EnemyIconMgr::EnemyIconExtended::~EnemyIconExtended()
{
	m_Owner->Delete(m_Icon->Name());
}

void GameScreen::EnemyIconMgr::EnemyIconExtended::MoveTo(const ipnt2_t& pt)
{
	m_Icon->MoveTo(pt.x,pt.y);
}

void GameScreen::EnemyIconMgr::EnemyIconExtended::SetNumber(int number)
{
	static char buff[20];

	sprintf(buff,"%d",number);
	m_Icon->SetNumber(buff);
}

void GameScreen::EnemyIconMgr::EnemyIconExtended::SetFlags(unsigned int flags)
{
	m_Flags = flags;
	//	-----------------------------------------------------------------------
	(m_Flags&F_SELECTED)?m_Icon->SetSelection(true):m_Icon->SetSelection(false);
}
//	уведомление от иконки
void GameScreen::EnemyIconMgr::EnemyIconExtended::OnClick(void)
{
	m_Owner->OnEnemyIconClick(m_Marker);
}

//**********************************************************************
//	class GameScreen::EnemyIconMgr::EnemyIconAsWidget
const char* GameScreen::EnemyIconMgr::EnemyIconAsWidget::m_EnemyIconName = "pictures/interface/gamescreen/enemy_icon.tga";
const char* GameScreen::EnemyIconMgr::EnemyIconAsWidget::m_EnemyIconSelectedName = "pictures/interface/gamescreen/enemy_icon_selected.tga";
const char* GameScreen::EnemyIconMgr::EnemyIconAsWidget::m_NameDesc = "enemy_icon_";
int GameScreen::EnemyIconMgr::EnemyIconAsWidget::m_Width = 32;
int GameScreen::EnemyIconMgr::EnemyIconAsWidget::m_Height = 32;
GameScreen::EnemyIconMgr::EnemyIconAsWidget::EnemyIconAsWidget(EnemyIconExtended* link) : Widget(GenerateName().c_str()),m_Selected(false),m_Link(link)
{
	Insert(m_Icon = new Static("icon"));
	m_Icon->AddImage(DXImageMgr::Instance()->CreateImage(m_EnemyIconName));
	m_Icon->AddImage(DXImageMgr::Instance()->CreateImage(m_EnemyIconSelectedName));
	m_Icon->SetState(0);
	//	узнаем размеры картинки
	DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_Icon->GetImage(0)),&m_Width,&m_Height);
	//	создаем текстовое поле
	Insert(m_Number = new Text("number"));
	m_Number->SetRegion(0,0,m_Width,m_Height);
	m_Number->SetFont("oskar",0xffff00);
	m_Number->Align(LogicalText::T_HCENTER|LogicalText::T_VCENTER);
	m_Number->SetText("X");
	//	устанавливаем всплывающую подсказку
	m_Number->m_Tip.m_Text = m_Link->GetTip();
	m_Number->m_Tip.m_Activated = true;
	m_Number->Enable(false);
	//	устанавливаем размеры	
	m_Icon->GetLocation()->Create(0,0,m_Width,m_Height);
	GetLocation()->Create(0,0,m_Width,m_Height);
}

GameScreen::EnemyIconMgr::EnemyIconAsWidget::~EnemyIconAsWidget(void)
{
}

std::string GameScreen::EnemyIconMgr::EnemyIconAsWidget::GenerateName(void)
{
	static int counter = 0;
	static char buff[50];

	sprintf(buff,"%s%ld",m_NameDesc,counter++);

	return std::string(buff);
}
//	установить/снять выделение
void GameScreen::EnemyIconMgr::EnemyIconAsWidget::SetSelection(bool enable)
{
	m_Icon->SetState(enable?1:0);
}
//	установить номер иконки
void GameScreen::EnemyIconMgr::EnemyIconAsWidget::SetNumber(const char* number)
{
	m_Number->SetText(number);
}
//	сообщение от дочерних окон
void GameScreen::EnemyIconMgr::EnemyIconAsWidget::OnChange(Widget* pChild)
{
	if(static_cast<Static*>(pChild)->LastEvent() == Static::LE_LDBLCLICK)
	{
		m_Link->OnClick();
	}
}

//**********************************************************************//
//	class GameScreen::HackVehicleMgr
int GameScreen::HackVehicleMgr::m_Left = 0;							
int GameScreen::HackVehicleMgr::m_Right = 0;							
GameScreen::HackVehicleMgr::HackVehicleMgr(GameScreen* owner) : m_Owner(owner)
{
}

GameScreen::HackVehicleMgr::~HackVehicleMgr()
{
	if(m_Icons.size()) Clear();
}

void GameScreen::HackVehicleMgr::Insert(Icon& icon)
{
	IconExtended* ie = new IconExtended(m_Owner,icon);

	ie->MoveTo(GetNextLocation(m_Icons.size()));
	ie->SetNumber(m_Icons.size()+1);

	m_Icons.push_back(ie);
}

void GameScreen::HackVehicleMgr::Remove(VehicleEntity* ve)
{
	for(std::list<IconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if((*i)->GetVehicleEntity() == ve)
		{
			//	удаляем итем
			delete (*i);
			m_Icons.erase(i);
			//	обновляем позиции других итемов
			UpdateLocation();
			break;
		}
	}
}

void GameScreen::HackVehicleMgr::Clear(void)
{
	for(std::list<IconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		delete (*i);
	m_Icons.clear();
}

GameScreen::HackVehicleMgr::Icon* GameScreen::HackVehicleMgr::ItemAt(const VehicleEntity* ve)
{
	for(std::list<IconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if((*i)->GetVehicleEntity() == ve)
		{
			return (*i);
		}
	}

	return 0;
}

GameScreen::HackVehicleMgr::Icon* GameScreen::HackVehicleMgr::ItemAt(unsigned int number)
{
	std::list<IconExtended*>::iterator i = m_Icons.begin();
	
	while((i!=m_Icons.end()) && number)
	{
		i++;
		number--;
	}
	if(i!=m_Icons.end()) return (*i);

	return 0;
}

ipnt2_t GameScreen::HackVehicleMgr::GetNextLocation(unsigned int size)
{
	const int width = GameScreen::HackVehicleMgr::IconAsWidget::Width();
	const int height = GameScreen::HackVehicleMgr::IconAsWidget::Height();

	const int in_row = (m_Right-m_Left)/width;
	const int height_factor = size/in_row; 
	const int width_factor = size-(height_factor*in_row);

	return ipnt2_t(m_Left+width*width_factor,height*height_factor);
}

void GameScreen::HackVehicleMgr::UpdateLocation(void)
{
	int size = 0;

	m_Right = m_Owner->GetHackVehicleIconRight();
	m_Left	 = m_Owner->GetHackVehicleIconLeft();
	for(std::list<IconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++,size++)
	{
		(*i)->MoveTo(GetNextLocation(size));
		(*i)->SetNumber(size+1);
	}
}

//	установить флаги для всех иконок стразу
void GameScreen::HackVehicleMgr::SetFlags(unsigned int flags)
{
	for(std::list<IconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		(*i)->SetFlags(flags);
}

//**********************************************************************
//	class GameScreen::HackVehicleMgr::IconExtended
GameScreen::HackVehicleMgr::IconExtended::IconExtended(GameScreen* owner,const GameScreen::HackVehicleMgr::Icon& icon) : GameScreen::HackVehicleMgr::Icon(icon),m_Owner(owner)
{
	m_Owner->Insert(m_Icon = new IconAsWidget(this),L_RIGHT_BOTTOM);
	m_Owner->BringToBottom(m_Icon->Name());
	SetFlags(m_Flags);
}

GameScreen::HackVehicleMgr::IconExtended::~IconExtended()
{
	m_Owner->Delete(m_Icon->Name());
}

void GameScreen::HackVehicleMgr::IconExtended::MoveTo(const ipnt2_t& pt)
{
	m_Icon->MoveTo(pt.x,pt.y);
}

void GameScreen::HackVehicleMgr::IconExtended::SetNumber(int number)
{
	static char buff[20];

	sprintf(buff,"%d",number);
	m_Icon->SetNumber(buff);
}

void GameScreen::HackVehicleMgr::IconExtended::SetFlags(unsigned int flags)
{
	m_Flags = flags;
	//	-----------------------------------------------------------------------
	(m_Flags&F_SELECTED)?m_Icon->SetSelection(true):m_Icon->SetSelection(false);
}
//	уведомление от иконки
void GameScreen::HackVehicleMgr::IconExtended::OnClick(GameScreen::BN_CLICK btn_clk)
{
	m_Owner->OnHackVehicleIconClick(m_VehicleEntity,btn_clk);
}


//**********************************************************************
//	class GameScreen::HackVehicleMgr::IconAsWidget
const char* GameScreen::HackVehicleMgr::IconAsWidget::m_IconName = "pictures/interface/gamescreen/hv_icon.tga";
const char* GameScreen::HackVehicleMgr::IconAsWidget::m_IconSelectedName = "pictures/interface/gamescreen/hv_icon_selected.tga";
const char* GameScreen::HackVehicleMgr::IconAsWidget::m_NameDesc = "hv_icon_";
int GameScreen::HackVehicleMgr::IconAsWidget::m_Width = 32;
int GameScreen::HackVehicleMgr::IconAsWidget::m_Height = 32;
GameScreen::HackVehicleMgr::IconAsWidget::IconAsWidget(IconExtended* link) : Widget(GenerateName().c_str()),m_Selected(false),m_Link(link),m_lastHealth(0),m_lastMovepnts(0)
{
	Insert(m_Icon = new Static("icon"));
	m_Icon->AddImage(DXImageMgr::Instance()->CreateImage(m_IconName));
	m_Icon->AddImage(DXImageMgr::Instance()->CreateImage(m_IconSelectedName));
	m_Icon->SetState(0);
	//	узнаем размеры картинки
	DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_Icon->GetImage(0)),&m_Width,&m_Height);
	//	создаем текстовое поле
	Insert(m_Number = new Text("number"));
	m_Number->SetRegion(0,0,m_Width,m_Height);
	m_Number->SetFont("oskar",0xffff00);
	m_Number->Align(LogicalText::T_HCENTER|LogicalText::T_VCENTER);
	m_Number->SetText("X");
	//	устанавливаем всплывающую подсказку
	m_Number->m_Tip.m_Text = AssemblyTip(m_Link->GetVehicleEntity()->GetEntityContext()->GetHealth(),
								         m_Link->GetVehicleEntity()->GetEntityContext()->GetMovepnts());
	m_Number->m_Tip.m_Activated = true;
	m_Number->Enable(false);
	//	устанавливаем размеры	
	m_Icon->GetLocation()->Create(0,0,m_Width,m_Height);
	GetLocation()->Create(0,0,m_Width,m_Height);
}

GameScreen::HackVehicleMgr::IconAsWidget::~IconAsWidget(void)
{
}

std::string GameScreen::HackVehicleMgr::IconAsWidget::GenerateName(void)
{
	static int counter = 0;
	static char buff[50];

	sprintf(buff,"%s%ld",m_NameDesc,counter++);

	return std::string(buff);
}
//	установить/снять выделение
void GameScreen::HackVehicleMgr::IconAsWidget::SetSelection(bool enable)
{
	m_Icon->SetState(enable?1:0);
}
//	установить номер иконки
void GameScreen::HackVehicleMgr::IconAsWidget::SetNumber(const char* number)
{
	m_Number->SetText(number);
}
//	сообщение от дочерних окон
void GameScreen::HackVehicleMgr::IconAsWidget::OnChange(Widget* pChild)
{
	GameScreen::BN_CLICK btn_clk = BNC_NONE;

	switch(static_cast<Static*>(pChild)->LastEvent())
	{
	case Static::LE_LBUTTONDOWN:
		btn_clk = GameScreen::BNC_LCLICK;
		break;
	case Static::LE_RBUTTONDOWN:
		btn_clk = GameScreen::BNC_RCLICK;
		break;
	case Static::LE_LDBLCLICK:
		btn_clk = GameScreen::BNC_LDBLCLICK;
		break;
	case Static::LE_RDBLCLICK:
		btn_clk = GameScreen::BNC_RDBLCLICK;
		break;
	}
	m_Link->OnClick(btn_clk);
}
//	производим проверку на изменение состояния
bool GameScreen::HackVehicleMgr::IconAsWidget::Tick(void)
{
	if(m_lastHealth != m_Link->GetVehicleEntity()->GetEntityContext()->GetHealth() ||
	   m_lastMovepnts != m_Link->GetVehicleEntity()->GetEntityContext()->GetMovepnts())
	{
		m_Number->m_Tip.m_Text = AssemblyTip(m_Link->GetVehicleEntity()->GetEntityContext()->GetHealth(),
										     m_Link->GetVehicleEntity()->GetEntityContext()->GetMovepnts());
	}

	return false;
}
//	собрать всплывающую подсказку
std::string GameScreen::HackVehicleMgr::IconAsWidget::AssemblyTip(int h,int mp)
{
	m_lastHealth = h;				//	здоровье
	m_lastMovepnts = mp;			//	очки хода
	return mlprintf("%s (%d/%d)",m_Link->GetTip().c_str(),m_lastHealth,m_lastMovepnts);
}


//**********************************************************************//
//	class GameScreen::PersonIconMgr::Monitor
class GameScreen::PersonIconMgr::Monitor : public Dialog
{
public:
	enum COMMAND {C_OPEN,C_QUICK_CLOSE,C_SLOW_CLOSE,C_CHANGE_STATE};
	//**********************************************************************//
	//	class Strategy
	class Strategy
	{
	protected:
		GameScreen::PersonIconMgr::Monitor* m_Controller;
	public:
		Strategy(GameScreen::PersonIconMgr::Monitor* controller) : m_Controller(controller) {}
		virtual ~Strategy() {}
		virtual void Tick(void) = 0;
		virtual COMMAND Type(void) = 0;
	};
	class OpenStrategy;
	class QuickCloseStrategy;
	class SlowCloseStrategy;
private:
	static const char* m_ScriptName;				//	наименование скрипта
public:
	static const float m_MaxWidth;					//	максимальная ширина монитора
	static const float m_MinWidth;					//	минимальная ширина монитора
	static const float m_MovingStep;				//	шаг смещения
	static const float m_LevelUpFlipTime;			//	частота мигания LevelUp'а
private:
	bool m_IsLevelUp;								//	флаг установки левел ап'а
	float m_LevelUpSeconds;
private:
	Text* m_MovePoints;
	Text* m_Health;
	std::map<std::string,WidgetImage*> m_IconImages;
private:
	bool m_Busy;									//	монитор используется
	bool m_Closed;
private:
	struct Order
	{
		COMMAND m_Command;							//	комманда
		MonitorTraits m_Traits;						//	свойства
		Order(COMMAND command,const MonitorTraits& traits):m_Command(command),m_Traits(traits){}
	};
	std::deque<Order> m_Commands;					//	очередь выполняемых задач 
	Strategy* m_Strategy;							//	текущая комманда
	PersonIconMgr* m_Controller;
public:
	Monitor(const char* name,PersonIconMgr* controller);
	virtual ~Monitor();
public:
	//	скомандовать
	void ToOrder(COMMAND command);
	void ToOrder(COMMAND command,const MonitorTraits& traits);
	//	установить занятость монитора
	void OnBusy(bool busy);
	//	монитор занят?
	bool IsBusy(void) {return m_Busy;}
	//	монитор закрыт?
	bool IsClosed(void) {return m_Closed;}
	//	уведомление о том,что команада выполнена
	void OnStrategyFinish(void);
	//	очистить монитор 
	void Clear(void);
private:
	//	двигаем окно если требуется
	bool Tick(void);
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
	//	траля-таполя
	void OnStatic(Static *pStatic);
	//	обновим состояние иконки
	void UpdateState(const MonitorTraits& traits);
public:
	virtual const char* Type(void) const {return "personmonitor";};
};

//**********************************************************************//
//	class GameScreen::PersonIconMgr::Monitor::OpenStrategy
class GameScreen::PersonIconMgr::Monitor::OpenStrategy : public Strategy
{
private:
	float m_Seconds;
	float m_Position;
	float m_Aim;
public:
	OpenStrategy(GameScreen::PersonIconMgr::Monitor* controller) : Strategy(controller),m_Seconds(0) {}
	virtual ~OpenStrategy(){}
private:
	void Tick(void)
	{
		float step;

		if(!m_Seconds)
		{
			m_Seconds = Timer::GetSeconds();
			m_Position = m_Controller->m_MinWidth;
			m_Aim = m_Controller->m_MaxWidth;
		}

		step = (Timer::GetSeconds()-m_Seconds)*m_Controller->m_MovingStep;
		if(m_Position < m_Aim)
		{//	выдвигаем окно
			step = std::min(m_Aim-m_Position,step);
			m_Position += step;
			m_Controller->MoveTo(m_Position-m_Controller->GetLocation()->Region()->Width(),m_Controller->GetLocation()->Region()->Y());
		}
		else
		{
			m_Controller->OnStrategyFinish();
		}
	}
	COMMAND Type(void) {return C_OPEN;}
};
//**********************************************************************//
//	class GameScreen::PersonIconMgr::Monitor::QuickCloseStrategy
class GameScreen::PersonIconMgr::Monitor::QuickCloseStrategy : public Strategy
{
private:
	float m_Seconds;
	float m_Position;
	float m_Aim;
public:
	QuickCloseStrategy(GameScreen::PersonIconMgr::Monitor* controller) : Strategy(controller),m_Seconds(0)
	{

	}
	virtual ~QuickCloseStrategy() {}
private:
	void Tick(void)
	{
		float step;

		if(!m_Seconds)
		{
			m_Seconds = Timer::GetSeconds();
			m_Position = m_Controller->m_MaxWidth;
			m_Aim = m_Controller->m_MinWidth;
		}

		step = (Timer::GetSeconds()-m_Seconds)*m_Controller->m_MovingStep;
		if(m_Position > m_Aim)
		{//	задвигаем окно
			step = -std::min(m_Position-m_Aim,step);
			m_Position += step;
			m_Controller->MoveTo(m_Position-m_Controller->GetLocation()->Region()->Width(),m_Controller->GetLocation()->Region()->Y());
		}
		else
		{
			m_Controller->OnStrategyFinish();
		}
	}
	COMMAND Type(void) {return C_QUICK_CLOSE;}
};
//**********************************************************************//
//	class GameScreen::PersonIconMgr::Monitor::SlowCloseStrategy
class GameScreen::PersonIconMgr::Monitor::SlowCloseStrategy : public Strategy
{
private:
	const float m_WaitTime;
	float m_Seconds;
	float m_Position;
	float m_Aim;
	bool m_Wait;
public:
	SlowCloseStrategy(GameScreen::PersonIconMgr::Monitor* controller) : Strategy(controller),m_Seconds(0),m_WaitTime(4.5f),m_Wait(true) {}
	virtual ~SlowCloseStrategy() {}
private:
	void Tick(void)
	{
		float step;

		if(!m_Seconds)
		{
			m_Seconds = Timer::GetSeconds();
			m_Position = m_Controller->m_MaxWidth;
			m_Aim = m_Controller->m_MinWidth;
		}
		if(m_Wait)
		{
			if((Timer::GetSeconds()-m_Seconds)>m_WaitTime)
			{
				m_Seconds = Timer::GetSeconds();
				m_Wait = false;
			}
		}
		else
		{
			step = (Timer::GetSeconds()-m_Seconds)*m_Controller->m_MovingStep;
			if(m_Position > m_Aim)
			{//	задвигаем окно
				step = -std::min(m_Position-m_Aim,step);
				m_Position += step;
				m_Controller->MoveTo(m_Position-m_Controller->GetLocation()->Region()->Width(),m_Controller->GetLocation()->Region()->Y());
			}
			else
			{
				m_Controller->OnStrategyFinish();
			}
		}
	}
	COMMAND Type(void) {return C_SLOW_CLOSE;}
};

//**********************************************************************
//	class GameScreen::PersonIconMgr
unsigned int GameScreen::PersonIconMgr::m_MaxMonitors = 6;
GameScreen::PersonIconMgr::PersonIconMgr(GameScreen* owner) : m_Owner(owner)
{
	char buff[20];

	//	добавляем иконки на персонажей
	for(unsigned int i=0;i<m_MaxMonitors;i++)
	{
		Monitor* monitor;

		sprintf(buff,"pm%d",i);
		m_Monitors.push_back(monitor = new Monitor(buff,this));
		m_MonitorViews.push_back(monitor);
		m_Owner->Insert(monitor,L_LEFT_TOP);
		monitor->Move(0,(monitor->GetLocation()->Region()->Height()+2)*i);
	}
}

GameScreen::PersonIconMgr::~PersonIconMgr()
{
	for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		delete *i;
}

void GameScreen::PersonIconMgr::InsertIcon(const Icon& icon)
{
	//	проверяем на существование свободного места
	if(m_Icons.size() == m_MaxMonitors) throw CASUS("Нет свободного места для новой иконки персонажа.");
	for(std::list<Monitor*>::iterator m=m_Monitors.begin();m!=m_Monitors.end();m++)
	{
		if(!(*m)->IsBusy())
		{
			m_Icons.push_back(new Icon(icon,(*m)));
			break;
		}
	}
}

GameScreen::PersonIconMgr::Icon* GameScreen::PersonIconMgr::IconAt(const HumanEntity* he)
{
	for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if((*i)->GetHumanEntity() == he) return *i;
	}

	return 0;
}

GameScreen::PersonIconMgr::Icon* GameScreen::PersonIconMgr::IconAt(unsigned int number)
{
	for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if(!number)	return *i;
		number--;
	}

	return 0;
}

void GameScreen::PersonIconMgr::RemoveIcon(const HumanEntity* he,REMOVE_TYPE rt)
{
	for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if((*i)->GetHumanEntity() == he) 
		{
			//	удаляем иконку
			(*i)->Remove(rt);
			m_Icons.erase(i);
			//	пересортировываем мониторы
			for(std::list<Monitor*>::iterator m=m_Monitors.begin();m!=m_Monitors.end();m++)
			{
				if(!(*m)->IsBusy())
				{//	нашли первый не занятый монитор
					//	помещаем его в конец списка
					Monitor* monitor = *m;
					m_Monitors.erase(m);
					m_Monitors.push_back(monitor);
					break;
				}
			}
			break;
		}
	}
}
//	удалить все иконки 
void GameScreen::PersonIconMgr::RemoveAll(void)
{
	int j = 0;

	//	удаляем все иконки
	for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		delete (*i);
	m_Icons.clear();
	m_Monitors.clear();
	//	приводим в соответствие вид мониторов их кординаты
	for(std::list<Monitor*>::iterator m=m_MonitorViews.begin();m!=m_MonitorViews.end();m++,j++)
	{
		m_Monitors.push_back(*m);
		(*m)->Clear();
	}
}
//	тик
void GameScreen::PersonIconMgr::Tick(void)
{
	for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		(*i)->Tick();
}
//	переопределить координаты мониторов
void GameScreen::PersonIconMgr::SwapMonitors(void)
{	
	std::list<Monitor*>::iterator m;
	int i = 0;

	//	пересортировываем мониторы
	for(m=m_MonitorViews.begin();m!=m_MonitorViews.end();m++)
	{
		if((*m)->IsClosed())
		{//	нашли первый не занятый монитор
			//	помещаем его в конец списка
			Monitor* monitor = *m;
			m_MonitorViews.erase(m);
			m_MonitorViews.push_back(monitor);
			break;
		}
	}
	//	устанавливаем новые координаты для мониторов
	for(m=m_MonitorViews.begin();m!=m_MonitorViews.end();m++,i++)
	{
		(*m)->MoveTo((*m)->GetLocation()->Region()->X(),((*m)->GetLocation()->Region()->Height()+2)*i);
	}
}

//**********************************************************************
//	class GameScreen::PersonIconMgr::Icon
GameScreen::PersonIconMgr::Icon::Icon(HumanEntity* he) : m_HumanEntity(he),m_Monitor(0)
{
}

GameScreen::PersonIconMgr::Icon::Icon(const Icon& icon,GameScreen::PersonIconMgr::Monitor* monitor) : m_Monitor(monitor),
								 m_HumanEntity(icon.m_HumanEntity),m_MonitorState(icon.m_MonitorState)
{
	//	занимаем монитор
	m_Monitor->OnBusy(true);
	//	задаем начальное состояние
	m_Monitor->ToOrder(Monitor::C_CHANGE_STATE,m_MonitorState);
	//	открываем монитор
	m_Monitor->ToOrder(Monitor::C_OPEN);
}

GameScreen::PersonIconMgr::Icon::~Icon()
{
}
//	установить здоровье
void GameScreen::PersonIconMgr::Icon::SetHealth(int health)
{
	m_MonitorState.m_Flags |= MonitorTraits::F_HEALTH;
	m_MonitorState.m_Health = health;
}
//	установить портрет и тип выделения персонажа
void GameScreen::PersonIconMgr::Icon::SetPortrait(const char* name,MonitorTraits::SEL_TYPE selection)
{
	m_MonitorState.m_Flags |= MonitorTraits::F_PORTRAIT;
	m_MonitorState.m_Flags |= MonitorTraits::F_SELECTION;
	m_MonitorState.m_Portrait = name;
	m_MonitorState.m_Selection = selection;
}

void GameScreen::PersonIconMgr::Icon::SetMovePoints(int mp)
{
	m_MonitorState.m_Flags |= MonitorTraits::F_MOVEPOINTS;
	m_MonitorState.m_MovePoints = mp;
}

void GameScreen::PersonIconMgr::Icon::SetHeartbeat(MonitorTraits::HEARTBEAT hb)
{
	m_MonitorState.m_Flags |= MonitorTraits::F_HEARTBEAT;
	m_MonitorState.m_Heartbeat = hb;
}

void GameScreen::PersonIconMgr::Icon::SetStateFlags(unsigned int flags)
{
	m_MonitorState.m_Flags |= MonitorTraits::F_ICONFLAGS;
	m_MonitorState.m_StateFlags = flags;
}

//	передача управления в кадре
void GameScreen::PersonIconMgr::Icon::Tick(void)
{
	if(m_MonitorState.m_Flags && m_Monitor)
	{
		m_Monitor->ToOrder(Monitor::C_CHANGE_STATE,m_MonitorState);
		m_MonitorState.m_Flags = 0;
	}
}
//	удалить иконку и уничтожить объект this в памяти
void GameScreen::PersonIconMgr::Icon::Remove(REMOVE_TYPE rt)
{
	if(m_Monitor)
	{	
		if(m_MonitorState.m_Flags || (m_MonitorState.m_StateFlags&MonitorTraits::IF_LEVELUP))
		{
			m_MonitorState.m_StateFlags &= ~MonitorTraits::IF_LEVELUP;
			m_MonitorState.m_Flags |= MonitorTraits::F_ICONFLAGS;
			m_Monitor->ToOrder(Monitor::C_CHANGE_STATE,m_MonitorState);
		}
		switch(rt)
		{
		case RT_QUICK:
			m_Monitor->ToOrder(Monitor::C_QUICK_CLOSE);
			break;
		case RT_SLOW:
			m_Monitor->ToOrder(Monitor::C_SLOW_CLOSE);
			break;
		}

		m_Monitor->OnBusy(false);
	}

	delete this;
}
//**********************************************************************//
//	class GameScreen::PersonIconMgr::Monitor
const char* GameScreen::PersonIconMgr::Monitor::m_ScriptName = "PersonMonitor";
const float GameScreen::PersonIconMgr::Monitor::m_MaxWidth = 81;
const float GameScreen::PersonIconMgr::Monitor::m_MinWidth = 11;
const float GameScreen::PersonIconMgr::Monitor::m_MovingStep = 100;
const float GameScreen::PersonIconMgr::Monitor::m_LevelUpFlipTime = 0.5f;
GameScreen::PersonIconMgr::Monitor::Monitor(const char* name,PersonIconMgr* controller) : Dialog(name,m_ScriptName),m_Controller(controller),
									m_Strategy(0),m_Busy(false),m_Closed(true),m_IsLevelUp(false)
{
	//	запрещаем передачу фокуса
	Focus()->Enable(WidgetFocus::FF_KEYBOARD,false);
	//	элементы 
	m_MovePoints = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"movepoints"));
	m_Health = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"health"));
	//	закрываемся
	MoveTo(m_MinWidth-GetLocation()->Region()->Width(),GetLocation()->Region()->Y());
	//	установить невидимость левелапа
	WidgetFactory::Instance()->Assert(this,"levelup")->SetVisible(false);
}

GameScreen::PersonIconMgr::Monitor::~Monitor()
{
	//	удаляем остаточную стратегию
	if(m_Strategy) delete m_Strategy;
	//	удаляем изображения портрета
	WidgetFactory::Instance()->Assert(this,"icon")->SetImage(0);
	for(std::map<std::string,WidgetImage*>::iterator i=m_IconImages.begin();i!=m_IconImages.end();i++)
		i->second->Release();
}
//	скомандовать
void GameScreen::PersonIconMgr::Monitor::ToOrder(COMMAND command)
{
	m_Commands.push_back(Order(command,MonitorTraits()));
}
//	скомандовать
void GameScreen::PersonIconMgr::Monitor::ToOrder(COMMAND command,const MonitorTraits& traits)
{
	m_Commands.push_back(Order(command,traits));
}
//	двигаем окно если требуется
bool GameScreen::PersonIconMgr::Monitor::Tick(void)
{
	if(!m_Strategy && m_Commands.size())
	{
		switch(m_Commands[0].m_Command)
		{
		case C_OPEN:
			m_Closed = false;
			m_Strategy = new OpenStrategy(this);
			break;
		case C_QUICK_CLOSE:
			m_Strategy = new QuickCloseStrategy(this);
			break;
		case C_SLOW_CLOSE:
			m_Strategy = new SlowCloseStrategy(this);
			break;
		case C_CHANGE_STATE:
			UpdateState(m_Commands[0].m_Traits);
			break;
		}
		//	удаляем команду
		m_Commands.pop_front();
	}
	
	if(m_Strategy) m_Strategy->Tick();
	//	функционирование LevelUp'а
	if(m_IsLevelUp)
	{
		if((Timer::GetSeconds()-m_LevelUpSeconds)>m_LevelUpFlipTime)
		{
			Child("levelup")->SetVisible(!WidgetFactory::Instance()->Assert(this,"levelup")->IsVisible());
			m_LevelUpSeconds = Timer::GetSeconds();
		}
	}

	return false;
}
//	уведомление о том,что команада выполнена
void GameScreen::PersonIconMgr::Monitor::OnStrategyFinish(void)
{
	switch(m_Strategy->Type())
	{
	case C_QUICK_CLOSE:
	case C_SLOW_CLOSE:
		m_Closed = true;
		m_Controller->SwapMonitors();
		break;
	}
	//	удаляем стратегию
	delete m_Strategy;
	m_Strategy = 0;
}
//	обновить состояние
void GameScreen::PersonIconMgr::Monitor::UpdateState(const MonitorTraits& traits)
{
	static char pBuff[20];

	//	уровень здоровья
	if(traits.m_Flags&MonitorTraits::F_HEALTH)
	{
		itoa(traits.m_Health,pBuff,10);
		m_Health->SetText(pBuff);
	}
	//	кол-во мувпоинтов
	if(traits.m_Flags&MonitorTraits::F_MOVEPOINTS)
	{
		itoa(traits.m_MovePoints,pBuff,10);
		m_MovePoints->SetText(pBuff);
	}
	//	тип сердцебиения
	if(traits.m_Flags&MonitorTraits::F_HEARTBEAT)
	{
		static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"heartbeat"))->SetState(traits.m_Heartbeat);
	}
	//	установить иконку персонажа и выделение
	if(traits.m_Flags&MonitorTraits::F_PORTRAIT)
	{
		std::string portrait_ex = traits.m_Portrait;

		//	проверяем выделение персонажа
		switch(traits.m_Selection)
		{
		case MonitorTraits::ST_NONE:
			portrait_ex += g_pIconNone;
			break;
		case MonitorTraits::ST_DEAD:
			portrait_ex += g_pIconDied;
			break;
		case MonitorTraits::ST_NORMAL:
			break;
		}
		const std::string icon_name = std::string("pictures/interface/portraits/")+std::string(portrait_ex)+".tga";
		std::map<std::string,WidgetImage*>::iterator i = m_IconImages.find(icon_name);
		
		if(i != m_IconImages.end())
		{
			WidgetFactory::Instance()->Assert(this,"icon")->SetImage(i->second);
		}
		else
		{
			m_IconImages[icon_name] = DXImageMgr::Instance()->CreateImage(icon_name.c_str());
			i = m_IconImages.find(icon_name);
		}
		WidgetFactory::Instance()->Assert(this,"icon")->SetImage(i->second);
	}
	//	проверить флаги ссстояния
	if(traits.m_Flags&MonitorTraits::F_ICONFLAGS)
	{
		//	человек в технике
		WidgetFactory::Instance()->Assert(this,"invehicle")->SetVisible(traits.m_StateFlags&MonitorTraits::IF_INVEHICLE);
		//	человек получил левел
		if(m_IsLevelUp = (traits.m_StateFlags&MonitorTraits::IF_LEVELUP))
		{
			WidgetFactory::Instance()->Assert(this,"levelup")->SetVisible(true);
			m_LevelUpSeconds = Timer::GetSeconds();
		}
		else
			WidgetFactory::Instance()->Assert(this,"levelup")->SetVisible(false);
		//	человек в движении
		WidgetFactory::Instance()->Assert(this,"inmovement")->SetVisible(traits.m_StateFlags&MonitorTraits::IF_INMOVEMENT);
		//	человек в панике
		WidgetFactory::Instance()->Assert(this,"inpanic")->SetVisible(traits.m_StateFlags&MonitorTraits::IF_INPANIC);
	}
}
//	установить занятость монитора
void GameScreen::PersonIconMgr::Monitor::OnBusy(bool busy)
{
	m_Busy = busy;
}

void GameScreen::PersonIconMgr::Monitor::OnSystemChange(void)
{
	for(std::map<std::string,WidgetImage*>::iterator i = m_IconImages.begin();i!=m_IconImages.end();i++)
		i->second->Reload();
	//	-----------------------------------------
	Widget::OnSystemChange();
}
//	очистить монитор 
void GameScreen::PersonIconMgr::Monitor::Clear(void)
{
	//	уничтожаем стратегию поведения
	if(m_Strategy)
	{
		delete m_Strategy;
		m_Strategy = 0;
	}
	//	очищаем список команд
	m_Commands.clear();
	//	закрываем иконку
	MoveTo(m_MinWidth-GetLocation()->Region()->Width(),GetLocation()->Region()->Y());
	m_Closed = true;
	m_Busy = false;
}
//	сообщение от статика
void GameScreen::PersonIconMgr::Monitor::OnStatic(Static *pStatic)
{
	if(Parent() && !strcmp("icon",pStatic->Name()) && (pStatic->LastEvent() != Static::LE_NONE))
		Parent()->OnChange(this);
}

//*****************************************************************************************
//	class GameScreen
int GameScreen::GetHackVehicleIconLeft(void)
{
	return PersonIconMgr::Monitor::m_MaxWidth+50;
}
