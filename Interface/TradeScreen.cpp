/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   13.03.2001

************************************************************************/
#include "Precomp.h"
#include "../Common/DataMgr/DataMgr.h"
#include "../Common/DataMgr/TxtFile.h"
#include "../Common/UI/Button.h"
#include "../Common/UI/Static.h"
#include "InventoryScreen.h"
#include "WidgetFactory.h"
#include "SlotsEngine.h"
#include "DXImageMgr.h"
#include "TextBox.h"
#include "Text.h"
#include "TradeScreen.h"

const char* g_TradeSlotsScript = "scripts/interface/TradeSlots.txt";

//**********************************************************************
//	class CommodityItem
class CommodityItem : public Widget
{
public:
	enum IMAGE
	{
		I_NORMAL,
		I_HIGHLIGHTED,
		I_GRAYED,
		MAX_IMAGE
	};
	enum EVENT
	{
		E_NONE,
		E_MOUSE_CLICK,
		E_MOUSE_COME,
		E_MOUSE_LEAVE
	};
private:
	Text* m_Text;
	DXImage* m_Images[MAX_IMAGE];
	IMAGE m_CurImage;
	EVENT m_PrevEvent;
	const float m_DetonateTime;
	bool m_FirstDetonate;
	bool m_FirstMouseCome;
	float m_Seconds;
public:
	CommodityItem() : Widget("commodity_item"),m_CurImage(I_NORMAL),m_PrevEvent(E_NONE),
					  m_DetonateTime(SESlot::m_DetonateTime),m_Seconds(0),
					  m_FirstDetonate(true),m_FirstMouseCome(true)
	{
		Insert(m_Text = new Text("text"));
		m_Text->Align(LogicalText::T_BOTTOM|LogicalText::T_RIGHT);
		m_Text->SetFont("oskar",0x00dd00);
		m_Text->Enable(false);
		//	обнуляем значения списка картинок
		m_Images[I_NORMAL]		= 0;
		m_Images[I_HIGHLIGHTED] = 0;
		m_Images[I_GRAYED]		= 0;
	}
	virtual ~CommodityItem()
	{
		//	удаляем картинки
		SetImage(0);
		for(int i=0;i<MAX_IMAGE;i++) if(m_Images[i]) m_Images[i]->Release();
	};
public:
	void SetText(const char* text)
	{
		m_Text->SetText(text);
		m_Text->SetRegion(0,
						  GetLocation()->Region()->Height()/2,
						  GetLocation()->Region()->Width(),
						  GetLocation()->Region()->Height());
	}
	bool IsIdentical(const char* image_name)
	{
		return (GetImage() && (static_cast<DXImage*>(GetImage())->m_Resource == image_name));
	}
	void SetNewImage(int left,int top,int right,int bottom,const char* image_name)
	{
		int width,height;
		int x,y;

		for(int i=0;i<MAX_IMAGE;i++)
		{
			if(m_Images[i]) m_Images[i]->Release();
			m_Images[i] = 0;
		}
		m_Images[I_NORMAL]		= DXImageMgr::Instance()->CreateImage(image_name);
		m_Images[I_HIGHLIGHTED] = SEFactory::DoHighlightedImage(m_Images[I_NORMAL]);
		m_Images[I_GRAYED]		= SEFactory::DoGrayedImage(m_Images[I_NORMAL]);
		//	устанавливаем начальное состояние
		SetImage(m_Images[m_CurImage = I_NORMAL]);
		//	рассчитываем координаты
		DXImageMgr::Instance()->GetImageSize(m_Images[I_NORMAL],&width,&height);
		x = left + ((right-left)-width)/2;
		y = top + ((bottom-top)-height)/2;
		GetLocation()->Create(x,y,x+width,y+height);
	}
	void SetState(unsigned int state)
	{
											m_CurImage = I_NORMAL;
		if(state&SEItem::ST_HIGHLIGHTED)	m_CurImage = I_HIGHLIGHTED;
		if(state&SEItem::ST_GRAYED)			m_CurImage = I_GRAYED;
		SetImage(m_Images[m_CurImage]);
	}
	EVENT Event(void) {return m_PrevEvent;}
private:
	//	обработка мышинного ввода
	bool ProcessMouseInput(VMouseState* pMS)
	{
		if(pMS->RButtonFront)
		{
			EVENT old_value = m_PrevEvent;
			m_PrevEvent = E_MOUSE_CLICK;
			if(Parent()) Parent()->OnChange(this);
			m_PrevEvent = old_value;
		}
		else
		{
			if(m_PrevEvent == E_NONE)
			{
				if(m_FirstMouseCome)
				{
					m_Seconds = Timer::GetSeconds();
					m_FirstMouseCome = false;
				}
				if(m_FirstDetonate && (Timer::GetSeconds()-m_Seconds)>m_DetonateTime)
				{
					m_PrevEvent = E_MOUSE_COME;
					if(Parent()) Parent()->OnChange(this);
					m_FirstDetonate = false;
				}
			}
		}

		return false;
	}
	//	производим какие-то действия
	bool Tick(void)
	{
		if(m_PrevEvent != E_NONE)
		{
			const int x = Input::MouseState().x;
			const int y = Input::MouseState().y;
			
			VRegion region = *(GetLocation()->Region());
			
			region.Transform(1,0,GetLocation()->OriginX(),0,1,GetLocation()->OriginY());
			
			if(!region.PtInRegion(x,y))
			{
				if((Timer::GetSeconds()-m_Seconds)>m_DetonateTime)
				{
					m_PrevEvent = E_MOUSE_LEAVE;
					if(Parent()) Parent()->OnChange(this);
					m_PrevEvent = E_NONE;
				}
				m_FirstDetonate = true;
				m_FirstMouseCome = true;
			}
		}

		return false;
	}
	//	обработка системного сообщения
	void OnSystemChange(void)
	{
		if(m_Images[I_NORMAL]) m_Images[I_NORMAL]->Reload();
		m_Images[I_HIGHLIGHTED] = SEFactory::DoHighlightedImage(m_Images[I_NORMAL]);
		m_Images[I_GRAYED]		= SEFactory::DoGrayedImage(m_Images[I_NORMAL]);
		SetImage(m_Images[m_CurImage]);
		//	уведомляем другие виджеты
		m_Text->OnSystemChange();
	}
};


//**********************************************************************
//	class TradeScreen
const char* TradeScreen::m_pDialogName = "TradeScreen";
const char* TradeScreen::m_PortraitPath = "Pictures/Interface/Portraits/";
const char* TradeScreen::m_PortraitExt  = "_talk.tga";
TradeScreen::TradeScreen() : Dialog(m_pDialogName,m_pDialogName),m_Controller(0),
							 m_SlotsEngine(new SlotsEngine(this)),m_GroundScroller(0),
							 m_TradeScroller(0)
{
	//	создаем продаваемый итем 
	Widget::Insert(m_CommodityItem = new CommodityItem());
	BringToBottom(m_CommodityItem->Name());
	WidgetFactory::Instance()->Assert(this,"commodity")->Enable(false);
	//	создаем слоты для раскладки итемов
	CreateSlots();
	//	заполняем массив кнопок
	m_Buttons[B_UNLOAD]			= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"unload"));
	m_Buttons[B_BACK]			= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"back"));
	m_Buttons[B_PREV_HERO]		= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"prev_hero"));
	m_Buttons[B_NEXT_HERO]		= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"next_hero"));
	m_Buttons[B_PREV_THING]		= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"down_buy"));
	m_Buttons[B_NEXT_THING]		= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"up_buy"));
	m_Buttons[B_BUY]			= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"buy"));
	m_Buttons[B_SELL]			= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"sell"));
	//	заполняем массив скроллеров
	m_Scrollers[S_TRADE_TO_UP]		= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"up_sell"));
	m_Scrollers[S_TRADE_TO_DOWN]	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"down_sell"));
	m_Scrollers[S_GND_TO_LEFT]		= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"ground_to_left"));
	m_Scrollers[S_GND_TO_RIGHT]		= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"ground_to_right"));
	//	заполняем массив закладок
	m_Sheets[S_GUNS]	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"guns"));
	m_Sheets[S_AMMO]	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"ammo"));
	m_Sheets[S_EQUIP]	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"equip"));
	m_Sheets[S_GREN]	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"gren"));
	m_Sheets[S_ARMOR]	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"armor"));
	//	обрабатываем кнопки
	for(int i=0;i<MAX_BUTTON;i++)
	{
		m_Buttons[i]->NoFocus(true);
		BringToTop(m_Buttons[i]->Name());
	}
	//	создаем скроллеры на слоты
	m_GroundScroller = new SlotScroller(m_SlotsEngine->SlotAt(S_GROUND));
	m_TradeScroller = new SlotScroller(m_SlotsEngine->SlotAt(S_TRADE));
}

TradeScreen::~TradeScreen()
{
	//	уничтожаем скроллеры на слоты
	delete m_GroundScroller;
	delete m_TradeScroller;
	//	уничотжаем механизм слотов
	delete m_SlotsEngine;
}
//	уведомление об изменении состояния системы
void TradeScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}

void TradeScreen::OnChange(Widget* pChild)
{
	if((pChild == m_CommodityItem) && m_Controller && !m_SlotsEngine->MigrationMgr()->GetItem())
	{
		switch(m_CommodityItem->Event())
		{
		case CommodityItem::E_MOUSE_CLICK:
			m_Controller->OnCommodityClick();
			break;
		case CommodityItem::E_MOUSE_COME:
			m_Controller->OnCommodityHanging(TradeScreenController::HM_START);
			break;
		case CommodityItem::E_MOUSE_LEAVE:
			m_Controller->OnCommodityHanging(TradeScreenController::HM_FINISH);
			break;
		}

		return;
	}

	Dialog::OnChange(pChild);
}
//	уведомление об изменении кнопок
void TradeScreen::OnButton(Button *pButton)
{
/*	if(pButton == m_Buttons[B_GND_TO_LEFT])
	{
		m_SlotsEngine->SlotAt(S_GROUND)->Scroll(1,0);
		return;
	}
	if(pButton == m_Buttons[B_GND_TO_RIGHT])
	{
		m_SlotsEngine->SlotAt(S_GROUND)->Scroll(-1,0);
		return;
	}
	if(pButton == m_Buttons[B_TRADE_TO_UP])
	{
		m_SlotsEngine->SlotAt(S_TRADE)->Scroll(0,1);
		return;
	}
	if(pButton == m_Buttons[B_TRADE_TO_DOWN])
	{
		m_SlotsEngine->SlotAt(S_TRADE)->Scroll(0,-1);
		return;
	}*/
	if(m_Controller)
	{
		for(int i=0;i<MAX_BUTTON;i++)
		{
			if(m_Buttons[i] == pButton)
			{
				m_Controller->OnButtonClick(static_cast<BUTTON>(i));
				return;
			}
		}
	}
}

void TradeScreen::OnStatic(Static *pStatic)
{
	for(int scroller=0;scroller<MAX_SCROLLER;scroller++)
	{
		if(m_Scrollers[scroller] == pStatic)
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
				if(pStatic == m_Scrollers[S_GND_TO_LEFT])
				{
					m_GroundScroller->Start(SlotScroller::D_LEFT);
					return;
				}
				if(pStatic == m_Scrollers[S_GND_TO_RIGHT])
				{
					m_GroundScroller->Start(SlotScroller::D_RIGHT);
					return;
				}
				if(pStatic == m_Scrollers[S_TRADE_TO_UP])
				{
					m_TradeScroller->Start(SlotScroller::D_UP);
					return;
				}
				if(pStatic == m_Scrollers[S_TRADE_TO_DOWN])
				{
					m_TradeScroller->Start(SlotScroller::D_DOWN);
					return;
				}
				break;
			case Static::LE_LBUTTONUP:
				pStatic->SetState(1);
				break;
			}
			if((pStatic->GetState()==2) && (pStatic->LastEvent() == Static::LE_LBUTTONPUSHED))
			{
				if(pStatic == m_Scrollers[S_GND_TO_LEFT])
				{
					m_GroundScroller->Scroll(SlotScroller::D_LEFT);
					return;
				}
				if(pStatic == m_Scrollers[S_GND_TO_RIGHT])
				{
					m_GroundScroller->Scroll(SlotScroller::D_RIGHT);
					return;
				}
				if(pStatic == m_Scrollers[S_TRADE_TO_UP])
				{
					m_TradeScroller->Scroll(SlotScroller::D_UP);
					return;
				}
				if(pStatic == m_Scrollers[S_TRADE_TO_DOWN])
				{
					m_TradeScroller->Scroll(SlotScroller::D_DOWN);
					return;
				}
			}
			
		}
	}
	for(int sheet=0;sheet<MAX_SHEET;sheet++)
	{
		if(m_Sheets[sheet] == pStatic)
		{
			switch(pStatic->LastEvent())
			{
			case Static::LE_MOUSEMOVE:
				if(pStatic->GetState()!=2) pStatic->SetState(1);
				break;
			case Static::LE_MOUSELOST:
				if(pStatic->GetState()!=2) pStatic->SetState(0);
				break;
			case Static::LE_LBUTTONDOWN:
				if(pStatic->GetState()!=2)
				{
					for(int i=0;i<MAX_SHEET;i++)
						m_Sheets[i]->SetState(0);
					pStatic->SetState(2);
					if(m_Controller) m_Controller->OnSheetClick(static_cast<SHEET>(sheet));
				}
				break;
			}
			break;
		}
	}
}
//	показать/скрыть кнопки
void TradeScreen::EnableButton(BUTTON id,bool enable)
{
	m_Buttons[id]->SetVisible(enable);
}
//	показать/скрыть закладки
void TradeScreen::EnableSheet(SHEET id,bool enable)
{
	m_Sheets[id]->SetVisible(enable);
}
//	создаем слоты для раскладки итемов
void TradeScreen::CreateSlots(void)
{
	TxtFile script = DataMgr::Load(g_TradeSlotsScript);
	SESlot* slot;

	DataMgr::Release(g_TradeSlotsScript);
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

#ifdef _DEBUG_SLOTS_ENGINE
//	отображение виджета и всех дочерних виджетов
void TradeScreen::Render(WidgetView* pView)
{
	Dialog::Render(pView);
	if(IsVisible())	m_SlotsEngine->Render();
}
#endif

//****************** ВНЕШНИЙ ИНТЕРФЕЙС С ЛОГИКОЙ *********************//
//	установить контроллер логики для интерфейса
void TradeScreen::SetController(TradeScreenController* controller)
{
	m_Controller = controller;
}

void TradeScreen::SetSheetState(SHEET sheet,SHEET_STATE ss)
{
	switch(ss)
	{
	case SS_NORMAL:
		m_Sheets[sheet]->SetState(0);
		break;
	case SS_PUSHED:
		m_Sheets[sheet]->SetState(2);
		break;
	}
}
//	установка свойств предмета торговца на продажу
void TradeScreen::SetCommodityTraits(const CommodityTraits& ct)
{
	static char buff[50];

	if(ct.m_SystemName && strlen(ct.m_SystemName))
	{
		std::string image_name = std::string(g_InventoryPictures)+std::string(ct.m_SystemName)+".tga";

		m_CommodityItem->SetVisible(true);
		if(!m_CommodityItem->IsIdentical(image_name.c_str()))
		{
			int left,top,right,bottom;

			WidgetFactory::Instance()->Assert(this,"commodity")->GetLocation()->Region()->GetRect(&left,&top,&right,&bottom);
			m_CommodityItem->SetNewImage(left,top,right,bottom,image_name.c_str());
		}
		m_CommodityItem->SetState(ct.m_State);
		m_CommodityItem->SetText(ct.m_Text);
		//	устанавливаем текстовые поля
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"cost"))->SetText(itoa(ct.m_Cost,buff,10));
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"quantity"))->SetText(itoa(ct.m_Quantity,buff,10));
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"commodity"))->SetText(ct.m_Name);
	}
	else
	{
		m_CommodityItem->SetVisible(false);
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"cost"))->SetText("");
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"quantity"))->SetText("");
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"commodity"))->SetText("");
	}
}
//	установить текстовое значение	
void TradeScreen::SetHeroTraits(const HeroTraits& t)
{
	static char buff[50];

	sprintf(buff,"%0.2d/%0.2d",t.m_WeightCurrent,t.m_WeightMax);
	static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"weight"))->SetText(buff);

	//	диаграмма веса
	int index = (static_cast<float>(t.m_WeightCurrent)/(static_cast<float>(t.m_WeightMax)/100))/10;
	static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"weight_diagram"))->SetState(std::min(10,index));
	static_cast<Static *>(WidgetFactory::Instance()->Assert(this,"max"))->SetState((index>10)?1:0);
}

void TradeScreen::SetTraderTraits(const TraderTraits& t)
{
	Widget* portrait = WidgetFactory::Instance()->Assert(this,"trader");
	std::string image_name = m_PortraitPath;

	image_name += t.m_Portrait;
	image_name += m_PortraitExt;
	if(portrait->GetImage()) DXImageMgr::Instance()->Release(static_cast<DXImage*>(portrait->GetImage()));
	portrait->SetImage(DXImageMgr::Instance()->CreateImage(image_name.c_str()));
	//	установка описания торговца
	TextBox* trader_desc = static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"description"));
	trader_desc->SetText(t.m_Description);
	trader_desc->Scroll(TextBox::SD_BEGIN);

}
//	стоимость продажи и покупки
void TradeScreen::SetText(TEXT id,const char* value)
{
	switch(id)
	{
	case T_HERO_NAME:
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"hero_name"))->SetText(value);
		break;
	case T_HERO_MONEY:
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"hero_money"))->SetText(value);
		break;
	case T_SALE_COST:
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"sale_cost"))->SetText(value);
		break;
	}
}
