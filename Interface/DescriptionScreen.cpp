/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   26.04.2001

************************************************************************/
#include "Precomp.h"
#include "../Common/FontMgr/Font.h"
#include "Text.h"
#include "MouseCursor.h"
#include "WidgetFactory.h"
#include "DXImageMgr.h"
#include "SlotsEngine.h"
#include "JournalScreen.h"
#include "DescriptionScreen.h"

const char* DescriptionScreen::m_pDialogName = "DescriptionScreen";
DescriptionScreen::DescriptionScreen() : Dialog(m_pDialogName,m_pDialogName),m_pController(0)
{
	//	добавляем виджет для отображения картинки
	Widget::Insert(m_Image = new Widget("image"));
	//	инициализируем рабочие инструменты
	m_ImagePlace = *WidgetFactory::Instance()->Assert(this,"image_place")->GetLocation()->Region();
	m_TextPlace  = *WidgetFactory::Instance()->Assert(this,"text_place")->GetLocation()->Region();
	m_Caption		= static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"name"));
	m_LeftColumn	= static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"left"));
	m_RightColumn	= static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"right"));
	m_Info			= static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"text"));
}

DescriptionScreen::~DescriptionScreen()
{
}
//	уведомление об изменении состояния системы
void DescriptionScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}

bool DescriptionScreen::Tick(void)
{
	if(m_pController && IsVisible())
	{
		if(!Input::MouseState().RButtonState)
		{
			m_pController->OnClose();
		}
	}

	return false;
}

void DescriptionScreen::SetVisible(bool bVisible)
{
	Dialog::SetVisible(bVisible);
	MouseCursor::SetVisible(!bVisible);
}

//	получить каталог с картинками для данной закладки
const char* DescriptionScreen::GetPathForResource(ITEM_TYPE it) const
{
	switch(it)
	{
	case IT_OUTFIT:
		return g_InventoryPictures;
	case IT_TECH:
		return g_TechPictures;
	case IT_FORCES:
		return g_ForcesPictures;
	case IT_PEOPLE:
		return g_PeoplePictures;
/*	case IT_LOCATION:
		return g_LocationPictures;*/
	default:
		return "";
	}
}

//****************    ИНТЕРФЕЙС С ЛОГИКОЙ       ***********************//
void DescriptionScreen::SetController(DescriptionScreenController* controller)
{
	m_pController = controller;
}
//	установить параметры
void DescriptionScreen::SetItemInfo(const ItemInfo& ii)
{
	static const int left_column_width = m_LeftColumn->GetLocation()->Region()->Width();
	static const int right_column_indention = 5;
	static const int info_indention = 5;
	static const int indention = 2;
	int top_border = m_ImagePlace.Y()+m_ImagePlace.Height();

	//	устанавливаем картинку, если она присутствует
	if(ii.m_Image)
	{
		int width,height;
		if(m_Image->GetImage()) m_Image->GetImage()->Release();
		m_Image->SetImage(DXImageMgr::Instance()->CreateImage(std::string(std::string(GetPathForResource(ii.m_Type))+std::string(ii.m_Image)+".tga").c_str()));
		DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_Image->GetImage()),&width,&height);
		int left = m_ImagePlace.X()+(m_ImagePlace.Width()-width)/2;
		int top	 = m_ImagePlace.Y()+(m_ImagePlace.Height()-height)/2;
		m_Image->GetLocation()->Create(left,top,left+width,top+height);
		m_Image->SetVisible(true);
	} else m_Image->SetVisible(false);
	//	устанавливаем наименование предмета описания
	if(ii.m_Name)
	{
		m_Caption->SetRegion(m_TextPlace.X()+indention,m_TextPlace.Y()+indention,
							 m_TextPlace.X()+m_TextPlace.Width()-indention,
							 m_TextPlace.Y()+m_Caption->GetFont()->Height()*2);
		m_Caption->SetText(ii.m_Name);
		m_Caption->SetVisible(true);
		top_border = m_Caption->GetLocation()->Region()->Y()+m_Caption->GetLocation()->Region()->Height();
	} else m_Caption->SetVisible(false);
	//	устанавливаем характеристики и их значения
	if(ii.m_LeftColumn && ii.m_RightColumn)
	{
		m_LeftColumn->SetRegion(m_TextPlace.X()+indention,top_border+indention,
								m_TextPlace.X()+left_column_width,
								m_TextPlace.Y()+m_TextPlace.Height()-indention*2);
		m_LeftColumn->SetText(ii.m_LeftColumn);
		m_LeftColumn->SetRegion(m_TextPlace.X()+indention,top_border+indention,
								m_TextPlace.X()+left_column_width,
								top_border+m_LeftColumn->Height()+indention);
		int right_left = m_LeftColumn->GetLocation()->Region()->X()+m_LeftColumn->GetLocation()->Region()->Width()+indention+right_column_indention;
		m_RightColumn->SetRegion(right_left,top_border+indention,
								right_left+(m_TextPlace.Width()-m_LeftColumn->GetLocation()->Region()->Width()-(indention*3)-right_column_indention),
								top_border+indention+m_LeftColumn->GetLocation()->Region()->Height());
		m_RightColumn->SetText(ii.m_RightColumn);
		m_LeftColumn->SetVisible(true);
		m_RightColumn->SetVisible(true);
		top_border = m_LeftColumn->GetLocation()->Region()->Y()+m_LeftColumn->GetLocation()->Region()->Height();
	} else {m_LeftColumn->SetVisible(false);m_RightColumn->SetVisible(false);}
	//	поле информации
	if(ii.m_Text)
	{
		m_Info->SetRegion(m_TextPlace.X()+indention,top_border+indention+info_indention,
						  m_TextPlace.X()+m_TextPlace.Width()-indention,
						  m_TextPlace.Y()+m_TextPlace.Height()-indention*2);
		m_Info->SetText(ii.m_Text);
		m_Info->SetVisible(true);
	} else m_Info->SetVisible(false);
}