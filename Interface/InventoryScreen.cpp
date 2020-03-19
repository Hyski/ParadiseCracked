/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   12.03.2001

************************************************************************/
#include "Precomp.h"
#include "InventoryScreen.h"

#include "../Common/DataMgr/DataMgr.h"
#include "../Common/DataMgr/TxtFile.h"
#include "../Common/UI/Button.h"
#include "../Common/UI/Static.h"
#include "DescriptionScreen.h"
#include "WidgetFactory.h"
#include "SlotsEngine.h"
#include "DXImageMgr.h"
#include "Screens.h"
#include "Text.h"

const char* g_InventorySlotsScript = "scripts/interface/InventorySlots.txt";

/////////////////////////////////////////////////////////////////////////
////////////////////////    class SlotScroller    ///////////////////////
/////////////////////////////////////////////////////////////////////////
const float SlotScroller::m_Pause = 0.6f;
const float SlotScroller::m_Speed = 0.1f;
SlotScroller::SlotScroller(SESlot* slot) : m_Slot(slot),m_Seconds(0)
{
}

SlotScroller::~SlotScroller()
{
}

void SlotScroller::Start(DIRECTION dir)
{
	int dx,dy;
	
	Translate(dir,&dx,&dy);
	
	m_Start = m_Seconds = Timer::GetSeconds();
	m_Slot->Scroll(dx,dy);
	DebugInfo::Add(300,60,"Scroll");
}

void SlotScroller::Scroll(DIRECTION dir)
{
	int dx,dy;
	
	Translate(dir,&dx,&dy);
	if((Timer::GetSeconds()-m_Start)>m_Pause)
	{
		if((Timer::GetSeconds()-m_Seconds)>m_Speed)
		{
			m_Slot->Scroll(dx,dy);
			DebugInfo::Add(300,60,"Scroll");
		}
	}
}

void SlotScroller::Translate(DIRECTION dir,int* dx,int* dy)
{
	*dx = *dy = 0;
	switch(dir)
	{
	case D_LEFT:  *dx = 1; break;
	case D_RIGHT: *dx = -1; break;
	case D_UP:    *dy = 1; break;
	case D_DOWN:  *dy = -1; break;
	}
}

/////////////////////////////////////////////////////////////////////////
///////////////////////    class MMgrController    //////////////////////
/////////////////////////////////////////////////////////////////////////
class MMgrController : public SEMigrationMgrController
{
private:
	InventoryScreen* m_FeedBack;
public:
	MMgrController(InventoryScreen* feed_back) : m_FeedBack(feed_back) {}
	virtual ~MMgrController() {}
private:
	void OnDrag(void)
	{
		m_FeedBack->BlockButtons(true);
	}
	void OnDrop(void)
	{
		m_FeedBack->BlockButtons(false);
	}
};
/*
/////////////////////////////////////////////////////////////////////////
///////////////    class InventoryScreen::DSController    ///////////////
/////////////////////////////////////////////////////////////////////////
class InventoryScreen::DSController : public DescriptionScreenController
{
public:
	void OnClose(void)
	{
		//	снимаем контроллер
		Screens::Instance()->Description()->SetController(0);
		//	показываем меню Inventory
		Screens::Instance()->Activate(Screens::SID_INVENTORY);
	}
};
*/
//**********************************************************************//
//	class InventoryScreen
const char* InventoryScreen::m_pDialogName = "InventoryScreen";
const char* InventoryScreen::m_PortraitPath = "Pictures/Interface/Portraits/";
const char* InventoryScreen::m_PortraitExt  = "_big.tga";
InventoryScreen::InventoryScreen() : Dialog(m_pDialogName,m_pDialogName),
									 m_SlotsEngine(0),m_Controller(0),
									 m_MMgrController(0),m_GroundScroller(0)/*,
									 m_DSController(new DSController())*/
{
	//	инициализируем массив кнопок
	m_Buttons[B_DISMISS]		= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"dismiss"));
	m_Buttons[B_UNLOAD]			= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"unload"));
	m_Buttons[B_BACK]			= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"back"));
	m_Buttons[B_INFO]			= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"info"));
	m_Buttons[B_PREV_HERO]		= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"prev_hero"));
	m_Buttons[B_NEXT_HERO]		= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"next_hero"));
	//	инициализируем массив кнопок для левелапов
	m_LevelUps[LU_STRENGTH].m_Minus		= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"strength_flash_minus"));
	m_LevelUps[LU_DEXTERITY].m_Minus	= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"dexterity_flash_minus"));
	m_LevelUps[LU_REACTION].m_Minus		= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"reaction_flash_minus"));
	m_LevelUps[LU_ACCURACY].m_Minus		= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"accuracy_flash_minus"));
	m_LevelUps[LU_WISDOM].m_Minus		= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"wisdom_flash_minus"));
	m_LevelUps[LU_MECHANICS].m_Minus	= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"mechanics_flash_minus"));
	m_LevelUps[LU_SIGHT].m_Minus		= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"sight_flash_minus"));
	m_LevelUps[LU_HEALTH].m_Minus		= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"health_flash_minus"));
	m_LevelUps[LU_STRENGTH].m_Plus	= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"strength_flash_plus"));
	m_LevelUps[LU_DEXTERITY].m_Plus	= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"dexterity_flash_plus"));
	m_LevelUps[LU_REACTION].m_Plus	= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"reaction_flash_plus"));
	m_LevelUps[LU_ACCURACY].m_Plus	= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"accuracy_flash_plus"));
	m_LevelUps[LU_WISDOM].m_Plus	= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"wisdom_flash_plus"));
	m_LevelUps[LU_MECHANICS].m_Plus	= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"mechanics_flash_plus"));
	m_LevelUps[LU_SIGHT].m_Plus		= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"sight_flash_plus"));
	m_LevelUps[LU_HEALTH].m_Plus	= static_cast<Button *>(WidgetFactory::Instance()->Assert(this,"health_flash_plus"));
	m_LevelUps[LU_STRENGTH].m_Place		= static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"strength_place"));
	m_LevelUps[LU_DEXTERITY].m_Place	= static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"dexterity_place"));
	m_LevelUps[LU_REACTION].m_Place		= static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"reaction_place"));
	m_LevelUps[LU_ACCURACY].m_Place		= static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"accuracy_place"));
	m_LevelUps[LU_WISDOM].m_Place		= static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"wisdom_place"));
	m_LevelUps[LU_MECHANICS].m_Place	= static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"mechanics_place"));
	m_LevelUps[LU_SIGHT].m_Place		= static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"sight_place"));
	m_LevelUps[LU_HEALTH].m_Place		= static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"health_place"));

	//	инициализируем сроллеры
	m_Scrollers[S_GND_TO_LEFT]	= static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"ground_to_left"));
	m_Scrollers[S_GND_TO_RIGHT]	= static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"ground_to_right"));
	//	создаем менеджер слотов
	m_SlotsEngine = new SlotsEngine(this);
	//	создаем контроллер для MigrationMgr
	m_MMgrController = new MMgrController(this);
	m_SlotsEngine->MigrationMgr()->SetController(m_MMgrController);
	//	создаем слоты для раскладки итемов
	CreateSlots();
	//	обрабатываем кнопки
	for(int i=0;i<MAX_BUTTON;i++)
	{
		m_Buttons[i]->NoFocus(true);
		BringToTop(m_Buttons[i]->Name());
	}
	for(int i=0;i<MAX_LEVEL_UP;i++)
	{
		m_LevelUps[i].m_Plus->NoFocus(true);
		m_LevelUps[i].m_Minus->NoFocus(true);
	}
	//	устанавливаем скроллер для земли
	m_GroundScroller = new SlotScroller(m_SlotsEngine->SlotAt(S_GROUND));
}

InventoryScreen::~InventoryScreen()
{
	//	уничтожаем скроллер для земли
	delete m_GroundScroller;
	//	уничтожаем механизм работы слотов
	delete m_SlotsEngine;
	//	уничтожаем контролер для MigrationMgr
	delete m_MMgrController;
/*	//	уничтожаем контролер для DescriptionScreen
	delete m_DSController;*/
}

//	уведомление об изменении состояния системы
void InventoryScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	производим какие-то действия
bool InventoryScreen::Tick(void)
{
	return true;
}
//	сообщения от статиков
void InventoryScreen::OnStatic(Static* pStatic)
{
	for(int i=0;i<MAX_SCROLLER;i++)
	{
		if(pStatic == m_Scrollers[i])
		{
			switch(pStatic->LastEvent())
			{
			case Static::LE_MOUSEMOVE:
				pStatic->SetState(1);
				break;
			case Static::LE_MOUSELOST:
				pStatic->SetState(0);
				break;
			case Static::LE_LBUTTONDOWN:
			case Static::LE_LDBLCLICK:
				pStatic->SetState(2);
				if(i == S_GND_TO_LEFT)
				{
					m_GroundScroller->Start(SlotScroller::D_LEFT);
					return;
				}
				if(i == S_GND_TO_RIGHT)
				{
					m_GroundScroller->Start(SlotScroller::D_RIGHT);
					return;
				}
				break;
			case Static::LE_LBUTTONUP:
				pStatic->SetState(1);
				break;
			}
			if((pStatic->GetState()==2) && (pStatic->LastEvent() == Static::LE_LBUTTONPUSHED))
			{
				if(i == S_GND_TO_LEFT)
				{
					m_GroundScroller->Scroll(SlotScroller::D_LEFT);
					return;
				}
				if(i == S_GND_TO_RIGHT)
				{
					m_GroundScroller->Scroll(SlotScroller::D_RIGHT);
					return;
				}
			}
			return;
		}
	}
	//	проверяем LEVEL UP
	for(int i=0;i<MAX_LEVEL_UP;i++)
	{
		if(m_LevelUps[i].m_Place == pStatic)
		{
			if(pStatic->LastEvent() == Static::LE_RBUTTONDOWN)
			{
				if(m_Controller) m_Controller->OnLevelUpDescription(static_cast<LEVEL_UP>(i));
/*				DescriptionScreen::ItemInfo ii;

				ii.m_Text = "Здесь должен выводиться текст описания";
				Screens::Instance()->Description()->SetItemInfo(ii);
				Screens::Instance()->Description()->SetController(m_DSController);
				Screens::Instance()->Activate(Screens::SID_DESCRIPTION);*/
			}
			return;
		}
	}
}
//	уведомление об изменении кнопок
void InventoryScreen::OnButton(Button *pButton)
{
	if(m_Controller)
	{
		int i;

		for(i=0;i<MAX_BUTTON;i++)
		{
			if(m_Buttons[i] == pButton)
			{
				m_Controller->OnButtonClick(static_cast<BUTTON>(i));
				return;
			}
		}
		for(i=0;i<MAX_LEVEL_UP;i++)
		{
			if(m_LevelUps[i].m_Minus == pButton)
			{
				m_Controller->OnLevelUpClick(static_cast<LEVEL_UP>(i),LUS_MINUS);
				return;
			}
			if(m_LevelUps[i].m_Plus == pButton)
			{
				m_Controller->OnLevelUpClick(static_cast<LEVEL_UP>(i),LUS_PLUS);
				return;
			}
		}
	}
} 

//****************** ВНЕШНИЙ ИНТЕРФЕЙС С ЛОГИКОЙ *********************//
//	установить контроллер логики для интерфейса
void InventoryScreen::SetController(InventoryScreenController* controller)
{
	m_Controller = controller;
}

void InventoryScreen::ShowLevelUp(bool show)
{
	for(int i =0;i<MAX_LEVEL_UP;i++)
	{
		m_LevelUps[i].m_Plus->SetVisible(show);
		m_LevelUps[i].m_Minus->SetVisible(show);
	}
}

void InventoryScreen::ShowLevelUp(LEVEL_UP index,LEVEL_UP_SIGN sign,bool show)
{
	if(index < MAX_LEVEL_UP)
	{
		switch(sign)
		{
		case LUS_PLUS:
			m_LevelUps[index].m_Plus->SetVisible(show);
			break;
		case LUS_MINUS:
			m_LevelUps[index].m_Minus->SetVisible(show);
			break;
		}
	}
}

// показать/скрыть кнопку
void InventoryScreen::EnableButton(BUTTON id,bool enable)
{
	m_Buttons[id]->SetVisible(enable);
}
//	установить текстовое значение	
void InventoryScreen::SetHeroTraits(const HeroTraits& t)
{
	static char pBuff[60];

	((Text *)WidgetFactory::Instance()->Assert(this,"experience"))->SetText(itoa(t.m_Experience,pBuff,10));
	((Text *)WidgetFactory::Instance()->Assert(this,"next_level"))->SetText(t.m_NextLevel.c_str()/*itoa(t.m_NextLevel,pBuff,10)*/);
	((Text *)WidgetFactory::Instance()->Assert(this,"level"))->SetText(itoa(t.m_Level,pBuff,10));
	((Text *)WidgetFactory::Instance()->Assert(this,"level_up_points"))->SetText(itoa(t.m_LevelUpPoints,pBuff,10));
	((Text *)WidgetFactory::Instance()->Assert(this,"strength"))->SetText(itoa(t.m_Strength,pBuff,10));
	((Text *)WidgetFactory::Instance()->Assert(this,"dexterity"))->SetText(itoa(t.m_Dexterity,pBuff,10));
	((Text *)WidgetFactory::Instance()->Assert(this,"reaction"))->SetText(itoa(t.m_Reaction,pBuff,10));
	((Text *)WidgetFactory::Instance()->Assert(this,"accuracy"))->SetText(itoa(t.m_Accuracy,pBuff,10));
	((Text *)WidgetFactory::Instance()->Assert(this,"wisdom"))->SetText(itoa(t.m_Wisdom,pBuff,10));
	((Text *)WidgetFactory::Instance()->Assert(this,"mechanics"))->SetText(itoa(t.m_Mechanics,pBuff,10));
	((Text *)WidgetFactory::Instance()->Assert(this,"sight"))->SetText(itoa(t.m_Sight,pBuff,10));
	((Text *)WidgetFactory::Instance()->Assert(this,"health"))->SetText(itoa(t.m_HealthMax,pBuff,10));
	((Text *)WidgetFactory::Instance()->Assert(this,"money"))->SetText(itoa(t.m_Money,pBuff,10));

	sprintf(pBuff,"%0.2d%%",t.m_ShockRes);
	((Text *)WidgetFactory::Instance()->Assert(this,"shock_res"))->SetText(pBuff);
	sprintf(pBuff,"%0.2d%%",t.m_FireRes);
	((Text *)WidgetFactory::Instance()->Assert(this,"fire_res"))->SetText(pBuff);
	sprintf(pBuff,"%0.2d%%",t.m_ElectricRes);
	((Text *)WidgetFactory::Instance()->Assert(this,"electric_res"))->SetText(pBuff);
	sprintf(pBuff,"%0.2d/%0.2d",t.m_WeightCurrent,t.m_WeightMax);
	((Text *)WidgetFactory::Instance()->Assert(this,"weight"))->SetText(pBuff);
	sprintf(pBuff,"%0.3d/%0.3d",t.m_MoveCurrent,t.m_MoveMax);
	((Text *)WidgetFactory::Instance()->Assert(this,"move"))->SetText(pBuff);
	sprintf(pBuff,"%0.3d/%0.3d",t.m_MoralCurrent,t.m_MoralMax);
	((Text *)WidgetFactory::Instance()->Assert(this,"moral"))->SetText(pBuff);

	//	диаграмма веса
	int index = (static_cast<float>(t.m_WeightCurrent)/(static_cast<float>(t.m_WeightMax)/100))/10;
	static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"weight_diagram"))->SetState(std::min(10,index));
	static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"max"))->SetState((index>10)?1:0);
}

void InventoryScreen::SetPortraitTraits(const PortraitTraits& t)
{
	static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"hero_name"))->SetText(t.m_hero_name);
	Widget* portrait = WidgetFactory::Instance()->Assert(this,"hero_portrait");
	std::string image_name = m_PortraitPath;

	image_name += t.m_portrait_name;
	image_name += m_PortraitExt;
	if(portrait->GetImage()) DXImageMgr::Instance()->Release(static_cast<DXImage*>(portrait->GetImage()));
	portrait->SetImage(DXImageMgr::Instance()->CreateImage(image_name.c_str()));
}
//	создаем слоты для раскладки итемов
void InventoryScreen::CreateSlots(void)
{
	TxtFile script = DataMgr::Load(g_InventorySlotsScript);
	SESlot* slot;

	DataMgr::Release(g_InventorySlotsScript);
	for(int i=0;i<MAX_SLOT;i++)
	{
		int index = i+1;

		if(slot = m_SlotsEngine->InsertSlot(i))
		{
			int x = atoi(script(index,1).c_str());
			int y = atoi(script(index,2).c_str());
			
			int width = atoi(script(index,3).c_str());
			int height = atoi(script(index,4).c_str());
			
			slot->SetLocation(x,y,width,height);

			if(atoi(script(index,5).c_str())) slot->SetPlacement(SESlot::P_ONE_ITEM_IN_SLOT);
			slot->SetAlign(atoi(script(index,6).c_str())?SESlot::A_LEFT_TO_RIGHT:SESlot::A_RIGHT_TO_LEFT);
			if(atoi(script(index,7).c_str())) slot->SetPlacement(SESlot::P_UNLIMITED);
			//	считываем информацию о картинках подсветки
			slot->SetSelectedImage(script(index,8).c_str(),atoi(script(index,9).c_str()),atoi(script(index,10).c_str()));
			//	информация о режиме расширения
			std::string em = script(index,11);
			if(em == "to_right") slot->SetExpMode(SESlot::EM_TO_RIGHT);
			if(em == "to_down") slot->SetExpMode(SESlot::EM_TO_DOWN);
			//	просматриваем наличие областей, которые всегда заняты
			int num_of_always_busy = atoi(script(index,12).c_str());
			int left,top,right,bottom;
			for(int j=0;j<num_of_always_busy;j++)
			{
				if(sscanf((char *)script(index,13+j).c_str(),"%d,%d,%d,%d",&left,&top,&right,&bottom) == 4)
				{
					slot->SetInaccessibleCells(left,top,right,bottom);
				}
			}
		}
	}
}
//	заблокировать/разблокировать кнопки
void InventoryScreen::BlockButtons(bool block)
{
	int i;

	for(i=0;i<MAX_BUTTON;i++)
		m_Buttons[i]->Enable(!block);
	for(i=0;i<MAX_LEVEL_UP;i++)
	{
		m_LevelUps[i].m_Plus->Enable(!block);
		m_LevelUps[i].m_Minus->Enable(!block);
		m_LevelUps[i].m_Place->Enable(!block);
	}
}

#ifdef _DEBUG_SLOTS_ENGINE
//	отображение виджета и всех дочерних виджетов
void InventoryScreen::Render(WidgetView* pView)
{
	Dialog::Render(pView);
	if(IsVisible())	m_SlotsEngine->Render();
}
#endif

