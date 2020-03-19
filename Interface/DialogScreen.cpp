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
#include "../Common/Shell/Shell.h"
#include "../Common/UI/Button.h"
#include "../Common/UI/Static.h"
#include "WidgetSoundMgr.h"
#include "WidgetFactory.h"
#include "MouseCursor.h"
#include "DXImageMgr.h"
#include "TextBox.h"
#include "MenuMgr.h"
#include "DialogScreen.h"

const char* DialogScreen::m_pDialogName = "DialogScreen";
const char* DialogScreen::m_PortraitPath = "Pictures/Interface/Portraits/";
const char* DialogScreen::m_PortraitExt  = "_talk.tga";
DialogScreen::DialogScreen() : Dialog(m_pDialogName,m_pDialogName),m_pController(0),
							   m_SpeechSound(0)
{
	SetPlayer(WidgetSoundMgr::Instance()->GetPlayer());
}

DialogScreen::~DialogScreen()
{
}
//	уведомление об изменении состояния системы
void DialogScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	сообщение от кнопок
void DialogScreen::OnButton(Button *pButton)
{
	if(!strcmp(pButton->Name(),"back"))
	{
		if(m_pController)
		{
			m_pController->OnClose();
		}
		else
		{
			Shell::Instance()->Exit();
		}
	}
}
//	сообщение от статиков
void DialogScreen::OnStatic(Static* pStatic)
{
	if(pStatic->LastEvent() == Static::LE_LBUTTONDOWN ||
	   pStatic->LastEvent() == Static::LE_LBUTTONPUSHED)
	{
		TextBox* content = static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"content"));
		if(!strcmp(pStatic->Name(),"up"))
		{
			content->Scroll(TextBox::SD_UP,true);
		}
		if(!strcmp(pStatic->Name(),"down"))
		{
			content->Scroll(TextBox::SD_DOWN,true);
		}
	}
}
//	производим какие-то действия
bool DialogScreen::Tick(void)
{
	if(IsVisible())
	{
		DebugInfo::Add(600,0,"DialogScreen");

		if(Input::KeyBack(DIK_ESCAPE|Input::RESET) ||
		   Input::KeyBack(DIK_RETURN|Input::RESET) ||
		   Input::KeyBack(DIK_SPACE|Input::RESET)  ||
		   Input::KeyBack(DIK_NUMPADENTER|Input::RESET))
		{
			if(m_pController)
			{
				m_pController->OnClose();
			}
			else
			{
				Shell::Instance()->Exit();
			}
		}
		if(Input::MouseState().LButtonFront)
		{
			Widget* child = FirstChildFromPoint(Input::MouseState().x,Input::MouseState().y);

			if((child == 0) || (child == WidgetFactory::Instance()->Assert(this,"content")))
			{
				static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"content"))->ToEnd();
			}
		}
	}

	return true;
}
//	становить параметры диалогового окна 
void DialogScreen::SetTraits(const DialogTraits& dt)
{
	TextBox* content = static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"content"));
	content->Scroll(TextBox::SD_BEGIN);
	content->SetText(dt.m_Text.c_str());
	Widget* portrait = WidgetFactory::Instance()->Assert(this,"portrait");
	std::string image_name = m_PortraitPath;

	image_name += dt.m_PortraitName;
	image_name += m_PortraitExt;
	if(portrait->GetImage()) DXImageMgr::Instance()->Release(static_cast<DXImage*>(portrait->GetImage()));
	portrait->SetImage(DXImageMgr::Instance()->CreateImage(image_name.c_str()));

	//	установка звука
	if(m_SpeechSound) m_SpeechSound->Release();
	m_SpeechSound = WidgetSoundMgr::Instance()->CreateSample(dt.m_Speech.c_str(),"COMMON");
}
//	установить контроллер
void DialogScreen::SetController(DialogScreenController* pController)
{
	m_pController = pController;
}
//	вызвать окошко как модальное
void DialogScreen::DoModal(const DialogTraits& dt)
{
	if(Parent())
	{
		std::vector<Widget*> widgets;
		std::vector<bool> states;
		//	составляем список элементов 
		for(Widget::Iterator it=Parent()->Begin();it!=Parent()->End();it++)
		{
			if(strcmp(it->Name(),Name()))
			{
				widgets.push_back(&(*it));
				states.push_back(it->IsEnable());
				it->Enable(false);
			}
		}
		//	обновляем курсор
		MouseCursor::SetCursor(MouseCursor::CT_NORMAL);
		Input::Mouse()->UnlockPosition();
		//	сбрасываем всплывающую подсказку
		MenuMgr::Instance()->Tip()->Activate(false);
		//	блокируем ввод для логики
		MenuMgr::Instance()->LockInput(true);
		//	отображаем диалог
		SetVisible(true);
		//	-------------------------------------------------------------------------------------
		SetTraits(dt);
		//	-------------------------------------------------------------------------------------
		m_Player->Play(m_SpeechSound);
		//	-------------------------------------------------------------------------------------
		Shell::Instance()->DoModal();
		//	-------------------------------------------------------------------------------------
		m_Player->Stop(m_SpeechSound);
		m_SpeechSound->Release();
		m_SpeechSound = 0;
		//	-------------------------------------------------------------------------------------
		SetVisible(false);
		//	разблокируем ввод для логики
		MenuMgr::Instance()->LockInput(false);
		//	восстанавливаем состояния
		for(unsigned int i=0;i<widgets.size();i++)
			widgets[i]->Enable(states[i]);
	}
}

//**********************************************************************//
//	class HeroInfoScreen
const char* HeroInfoScreen::m_pDialogName = "HeroInfoScreen";
HeroInfoScreen::HeroInfoScreen() : Dialog(m_pDialogName,m_pDialogName),m_Controller(0)
{
}

HeroInfoScreen::~HeroInfoScreen()
{
}
//	уведомление об изменении состояния системы
void HeroInfoScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	сообщение от кнопок
void HeroInfoScreen::OnButton(Button *pButton)
{
	if(!strcmp(pButton->Name(),"ok"))
	{
		if(m_Controller)
		{
			m_Controller->OnClose();
		}
		else
		{
			Shell::Instance()->Exit();
		}
	}
}
//	производим какие-то действия
bool HeroInfoScreen::Tick(void)
{
	if(IsVisible())
	{
		DebugInfo::Add(600,0,"HeroInfoScreen");

		if(Input::KeyBack(DIK_ESCAPE|Input::RESET) ||
		   Input::KeyBack(DIK_RETURN|Input::RESET) ||
		   Input::KeyBack(DIK_SPACE|Input::RESET)  ||
		   Input::KeyBack(DIK_NUMPADENTER|Input::RESET))
		{
			if(m_Controller)
			{
				m_Controller->OnClose();
			}
			else
			{
				Shell::Instance()->Exit();
			}
		}
		if(Input::MouseState().LButtonFront)
		{
			Widget* child = FirstChildFromPoint(Input::MouseState().x,Input::MouseState().y);

			if((child == 0) || (child == WidgetFactory::Instance()->Assert(this,"content")))
			{
				static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"content"))->ToEnd();
			}
		}
	}

	return true;
}
//	становить параметры диалогового окна 
void HeroInfoScreen::SetTraits(const HeroTraits& ht)
{
	TextBox* content = static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"content"));
	content->Scroll(TextBox::SD_BEGIN);
	content->SetText(ht.m_Text.c_str());
	Widget* portrait = WidgetFactory::Instance()->Assert(this,"portrait");
	std::string image_name = DialogScreen::m_PortraitPath;

	image_name += ht.m_PortraitName;
	image_name += DialogScreen::m_PortraitExt;
	if(portrait->GetImage()) DXImageMgr::Instance()->Release(static_cast<DXImage*>(portrait->GetImage()));
	portrait->SetImage(DXImageMgr::Instance()->CreateImage(image_name.c_str()));
}
//	установить контроллер
void HeroInfoScreen::SetController(HeroInfoScreenController* controller)
{
	m_Controller = controller;
}
//	вызвать окошко как модальное
void HeroInfoScreen::DoModal(const HeroTraits& ht)
{
	if(Parent())
	{
		std::vector<Widget*> widgets;
		std::vector<bool> states;
		//	составляем список элементов 
		for(Widget::Iterator it=Parent()->Begin();it!=Parent()->End();it++)
		{
			if(strcmp(it->Name(),Name()))
			{
				widgets.push_back(&(*it));
				states.push_back(it->IsEnable());
				it->Enable(false);
			}
		}
		//	обновляем курсор
		MouseCursor::SetCursor(MouseCursor::CT_NORMAL);
		Input::Mouse()->UnlockPosition();
		//	сбрасываем всплывающую подсказку
		MenuMgr::Instance()->Tip()->Activate(false);
		//	блокируем ввод для логики
		MenuMgr::Instance()->LockInput(true);
		//	отображаем диалог
		SetVisible(true);
		//	-------------------------------------------------------------------------------------
		SetTraits(ht);
		//	-------------------------------------------------------------------------------------
		Shell::Instance()->DoModal();
		//	-------------------------------------------------------------------------------------
		SetVisible(false);
		//	разблокируем ввод для логики
		MenuMgr::Instance()->LockInput(false);
		//	восстанавливаем состояния
		for(unsigned int i=0;i<widgets.size();i++)
			widgets[i]->Enable(states[i]);
	}
}

//**********************************************************************//
//	class HelperScreen
const char* HelperScreen::m_pDialogName = "HelperScreen";
HelperScreen::HelperScreen() : Dialog(m_pDialogName,m_pDialogName)
{
	m_ShowThisMessage = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"show_this_message"));
	m_ShowHelper = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"show_helper"));;
	//	картинка
	m_ImagePlace = *WidgetFactory::Instance()->Assert(this,"image_place")->GetLocation()->Region();
	Widget::Insert(m_Image = new Widget("image"));
	m_Image->GetLocation()->Create(m_ImagePlace);
}

HelperScreen::~HelperScreen()
{
}
//	уведомление об изменении состояния системы
void HelperScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	сообщение от статиков
void HelperScreen::OnStatic(Static* pStatic)
{
	if(!strcmp("ok",pStatic->Name()))
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
			pStatic->SetState(2);
			break;
		case Static::LE_LBUTTONUP:
			pStatic->SetState(1);
			Shell::Instance()->Exit();
			break;
		}
		return;

	}
	else
	{
		if(pStatic->LastEvent() == Static::LE_LBUTTONUP)
			pStatic->SetState(!pStatic->GetState());
	}
}
//	производим какие-то действия
bool HelperScreen::Tick(void)
{
	if(IsVisible())
	{
		DebugInfo::Add(600,0,"HelperScreen");

		if(Input::KeyBack(DIK_ESCAPE|Input::RESET) ||
		   Input::KeyBack(DIK_RETURN|Input::RESET) ||
		   Input::KeyBack(DIK_SPACE|Input::RESET)  ||
		   Input::KeyBack(DIK_NUMPADENTER|Input::RESET))
		{
			Shell::Instance()->Exit();
		}
		if(Input::MouseState().LButtonFront)
		{
			Widget* child = FirstChildFromPoint(Input::MouseState().x,Input::MouseState().y);

			if((child == 0) || (child == WidgetFactory::Instance()->Assert(this,"content")))
			{
				static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"content"))->ToEnd();
			}
		}
	}

	return true;
}
//	становить параметры диалогового окна 
void HelperScreen::SetTraits(const Traits& t)
{
	TextBox* content = static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"content"));
	content->Scroll(TextBox::SD_BEGIN);
	content->SetText(t.m_Text.c_str());
	//	картинка
	int width  = 0;
	int height = 0;
	if(m_Image->GetImage())
		DXImageMgr::Instance()->Release(static_cast<DXImage*>(m_Image->GetImage()));
	m_Image->SetImage(DXImageMgr::Instance()->CreateImage(std::string(std::string("Pictures/Interface/HelperScreen/Images/")+std::string(t.m_Image)+".tga").c_str()));
	DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_Image->GetImage()),&width,&height);
	int left = m_ImagePlace.X()+(m_ImagePlace.Width()-width)/2;
	int top	 = m_ImagePlace.Y()+(m_ImagePlace.Height()-height)/2;
	m_Image->GetLocation()->Create(left,top,left+width,top+height);
	//	настройка чек-боксов
	m_ShowThisMessage->SetState(t.m_ShowThisMessage);
	m_ShowHelper->SetState(t.m_ShowHelper);
	//	надписи на чек-боксах
	static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"show_this_message_text"))->SetText(MenuMgr::Instance()->GetStrResourse("dsh_show_this_message").c_str());
	static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"show_helper_text"))->SetText(MenuMgr::Instance()->GetStrResourse("dsh_show_helper").c_str());
}
//	вызвать окошко как модальное
void HelperScreen::DoModal(Traits& t)
{
	if(Parent())
	{
		std::vector<Widget*> widgets;
		std::vector<bool> states;
		//	запоминаем элемент фокуса
		Widget* prev_focus = Parent()->Focus()->Get(WidgetFocus::FF_KEYBOARD);
		//	составляем список элементов 
		for(Widget::Iterator it=Parent()->Begin();it!=Parent()->End();it++)
		{
			if(strcmp(it->Name(),Name()))
			{
				widgets.push_back(&(*it));
				states.push_back(it->IsEnable());
				it->Enable(false);
			}
		}
		//	обновляем курсор
		MouseCursor::SetCursor(MouseCursor::CT_NORMAL);
		Input::Mouse()->UnlockPosition();
		//	сбрасываем всплывающую подсказку
		MenuMgr::Instance()->Tip()->Activate(false);
		//	блокируем ввод для логики
		MenuMgr::Instance()->LockInput(true);
		//	отображаем диалог
		SetVisible(true);
		//	-------------------------------------------------------------------------------------
		SetTraits(t);
		//	-------------------------------------------------------------------------------------
		Focus()->Enable(WidgetFocus::FF_MOUSE,true);
		//	-------------------------------------------------------------------------------------
		Shell::Instance()->DoModal();
		//	-------------------------------------------------------------------------------------
		Focus()->Enable(WidgetFocus::FF_MOUSE,false);
		//	-------------------------------------------------------------------------------------
		t.m_ShowThisMessage = m_ShowThisMessage->GetState()?true:false;
		t.m_ShowHelper = m_ShowHelper->GetState()?true:false;
		//	-------------------------------------------------------------------------------------
		SetVisible(false);
		//	разблокируем ввод для логики
		MenuMgr::Instance()->LockInput(false);
		//	восстанавливаем состояния
		for(unsigned int i=0;i<widgets.size();i++)
			widgets[i]->Enable(states[i]);
		//	востанавливаем фокус
		Parent()->Focus()->Set(WidgetFocus::FF_KEYBOARD,prev_focus);
	}
}

//**********************************************************************//
//	class FlagDescScreen
const char* FlagDescScreen::m_pDialogName = "FlagDescScreen";
FlagDescScreen::FlagDescScreen() : Dialog(m_pDialogName,m_pDialogName)
{
	//	картинка
	m_ImagePlace = *WidgetFactory::Instance()->Assert(this,"image_place")->GetLocation()->Region();
	Widget::Insert(m_Image = new Widget("image"));
	m_Image->GetLocation()->Create(m_ImagePlace);
}

FlagDescScreen::~FlagDescScreen()
{
}
//	уведомление об изменении состояния системы
void FlagDescScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	сообщение от статиков
void FlagDescScreen::OnStatic(Static* pStatic)
{
	if(!strcmp("ok",pStatic->Name()))
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
			pStatic->SetState(2);
			break;
		case Static::LE_LBUTTONUP:
			pStatic->SetState(1);
			Shell::Instance()->Exit();
			break;
		}
		return;

	}
}
//	производим какие-то действия
bool FlagDescScreen::Tick(void)
{
	if(IsVisible())
	{
		DebugInfo::Add(600,0,"FlagDescScreen");

		if(Input::KeyBack(DIK_ESCAPE|Input::RESET) ||
		   Input::KeyBack(DIK_RETURN|Input::RESET) ||
		   Input::KeyBack(DIK_SPACE|Input::RESET)  ||
		   Input::KeyBack(DIK_NUMPADENTER|Input::RESET))
		{
			Shell::Instance()->Exit();
		}
		if(Input::MouseState().LButtonFront)
		{
			Widget* child = FirstChildFromPoint(Input::MouseState().x,Input::MouseState().y);

			if((child == 0) || (child == WidgetFactory::Instance()->Assert(this,"content")))
			{
				static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"content"))->ToEnd();
			}
		}
	}

	return true;
}
//	становить параметры диалогового окна 
void FlagDescScreen::SetTraits(const Traits& t)
{
	TextBox* content = static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"content"));
	content->Scroll(TextBox::SD_BEGIN);
	content->SetText(t.m_Text.c_str());
	//	картинка
	int width  = 0;
	int height = 0;
	if(m_Image->GetImage())
		DXImageMgr::Instance()->Release(static_cast<DXImage*>(m_Image->GetImage()));
	m_Image->SetImage(DXImageMgr::Instance()->CreateImage(t.m_Image.c_str()));
	DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_Image->GetImage()),&width,&height);
	int left = m_ImagePlace.X()+(m_ImagePlace.Width()-width)/2;
	int top	 = m_ImagePlace.Y()+(m_ImagePlace.Height()-height)/2;
	m_Image->GetLocation()->Create(left,top,left+width,top+height);
}
//	вызвать окошко как модальное
void FlagDescScreen::DoModal(const Traits& t)
{
	if(Parent())
	{
		std::vector<Widget*> widgets;
		std::vector<bool> states;
		//	запоминаем элемент фокуса
		Widget* prev_focus = Parent()->Focus()->Get(WidgetFocus::FF_KEYBOARD);
		//	составляем список элементов 
		for(Widget::Iterator it=Parent()->Begin();it!=Parent()->End();it++)
		{
			if(strcmp(it->Name(),Name()))
			{
				widgets.push_back(&(*it));
				states.push_back(it->IsEnable());
				it->Enable(false);
			}
		}
		//	обновляем курсор
		MouseCursor::SetCursor(MouseCursor::CT_NORMAL);
		Input::Mouse()->UnlockPosition();
		//	сбрасываем всплывающую подсказку
		MenuMgr::Instance()->Tip()->Activate(false);
		//	блокируем ввод для логики
		MenuMgr::Instance()->LockInput(true);
		//	отображаем диалог
		SetVisible(true);
		//	-------------------------------------------------------------------------------------
		SetTraits(t);
		//	-------------------------------------------------------------------------------------
		Focus()->Enable(WidgetFocus::FF_MOUSE,true);
		//	-------------------------------------------------------------------------------------
		Shell::Instance()->DoModal();
		//	-------------------------------------------------------------------------------------
		Focus()->Enable(WidgetFocus::FF_MOUSE,false);
		//	-------------------------------------------------------------------------------------
		SetVisible(false);
		//	разблокируем ввод для логики
		MenuMgr::Instance()->LockInput(false);
		//	восстанавливаем состояния
		for(unsigned int i=0;i<widgets.size();i++)
			widgets[i]->Enable(states[i]);
		//	востанавливаем фокус
		Parent()->Focus()->Set(WidgetFocus::FF_KEYBOARD,prev_focus);
	}
}