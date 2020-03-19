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
#include "WidgetFactory.h"
#include "Dialog.h"

//---------- Лог файл ------------
#ifdef _DEBUG_GRAPHICS
CLog dialog_log;
#define dialog	dialog_log["dialog.log"]
#else
#define dialog	/##/
#endif
//--------------------------------



const int Dialog::m_MinResX = 640;
const int Dialog::m_MinResY = 480;
Dialog::Dialog(const char *pName,const char *pScriptName) : Widget(pName)
{
	WidgetFactory::Instance()->LoadScript(this,pScriptName);
	SetVisible(false);
	//	----------------------------------------
	m_ResX = m_MinResX;
	m_ResY = m_MinResY;
}

Dialog::~Dialog()
{
}

Widget* Dialog::Insert(Widget *widget,LINK link)
{
	std::map<Widget*,LINK>::iterator i;

	if(widget && (link != L_NONE))
	{
		i = m_LinkedWidgets.find(widget);
		if(i != m_LinkedWidgets.end())
			throw CASUS("Виджет с таким адресом уже имеется в списке.\n");
		else
		{
			m_LinkedWidgets.insert(std::map<Widget*,LINK>::value_type(widget,link));
		}
	}

	return Widget::Insert(widget);
}

//******************************************************************************//
//	обработка мышинного ввода
bool Dialog::ProcessMouseInput(VMouseState* /*pMS*/)
{
	if(Parent() && (Parent()->Focus()->Get(WidgetFocus::FF_KEYBOARD) != this)/* && (pMS->dx || pMS->dy)*/)
	{
		Parent()->Focus()->Set(WidgetFocus::FF_KEYBOARD,this);
	}

	return true;
}
//	уведомление о потере клавиатурного фокуса
void Dialog::OnKeyboardLost(void)
{
	SetVisible(false);
}
//	обработка клавиатурного ввода
bool Dialog::ProcessKeyboardInput(VKeyboardState* pKS)
{
	if(Focus()->Get(WidgetFocus::FF_KEYBOARD))
	{
		switch(WidgetFactory::Instance()->WidgetType(Focus()->Get(WidgetFocus::FF_KEYBOARD)->Type()))
		{
		case WidgetFactory::WT_BUTTON:
			if(pKS->Front[DIK_TAB] || pKS->Front[DIK_DOWN] || pKS->Front[DIK_RIGHT])
			{
				Focus()->Prev(WidgetFocus::FF_KEYBOARD);
				return false;
			}
			if(pKS->Front[DIK_UP] || pKS->Front[DIK_LEFT])
			{
				Focus()->Next(WidgetFocus::FF_KEYBOARD);
				return false;
			}
			break;
		case WidgetFactory::WT_TEXTBOX:
			if(pKS->Front[DIK_TAB] || pKS->Front[DIK_RIGHT])
			{
				Focus()->Prev(WidgetFocus::FF_KEYBOARD);
				return false;
			}
			if(pKS->Front[DIK_LEFT])
			{
				Focus()->Next(WidgetFocus::FF_KEYBOARD);
				return false;
			}
			break;
		case WidgetFactory::WT_EDIT:
			if(pKS->Front[DIK_TAB] || pKS->Front[DIK_DOWN])
			{
				Focus()->Prev(WidgetFocus::FF_KEYBOARD);
				return false;
			}
			if(pKS->Front[DIK_UP])
			{
				Focus()->Next(WidgetFocus::FF_KEYBOARD);
				return false;
			}
			break;
		}
	}

	return true;
}


//	уведомление от дочернего окна об изменении состояния
void Dialog::OnChange(Widget* pChild)
{
	switch(WidgetFactory::Instance()->WidgetType(pChild->Type()))
	{
	case WidgetFactory::WT_BUTTON:
		OnButton((Button *)pChild);
		break;
	case WidgetFactory::WT_STATIC:
		OnStatic((Static *)pChild);
		break;
	}
}

//	уведомление об изменении состояния системы
void Dialog::OnSystemChange(void)
{
	dialog("OnSystemChange(\"%s\")\n",Name());
	CheckConformMode();
	Widget::OnSystemChange();
}
//	уничтожить экземпляр другого виджета по имени, при неудаче
//	(если данного виджета не имеется) возвращается false
bool Dialog::Delete(const char *pName)
{
	std::map<Widget*,LINK>::iterator i;

	i = m_LinkedWidgets.find(Child(pName));
	if(i != m_LinkedWidgets.end())
		m_LinkedWidgets.erase(i);

	return Widget::Delete(pName);
}

void Dialog::CheckConformMode(void)
{
	int dx = D3DKernel::ResX() - m_ResX;
	int dy = D3DKernel::ResY() - m_ResY;

	dialog("\nCheckConformMode(\"%s\")\n",Name());
	dialog("D3DKernel: %dx%d\n",D3DKernel::ResX(),D3DKernel::ResY());
	dialog("Res: %dx%d\n",m_ResX,m_ResY);

	m_ResX = D3DKernel::ResX();
	m_ResY = D3DKernel::ResY();

	if(dx || dy)
	{
		//	необходимо изменить положение данного виджета
		GetLocation()->Create(0,0,m_ResX,m_ResY);
		//	необходимо раскинуть все элементы по местам
		for(std::map<Widget*,LINK>::iterator i = m_LinkedWidgets.begin();i!=m_LinkedWidgets.end();i++)
		{
			switch(i->second)
			{
			case L_LEFT_TOP:
				break;
			case L_LEFT_BOTTOM:
				i->first->Move(0,dy);
				break;
			case L_RIGHT_TOP:
				i->first->Move(dx,0);
				break;
			case L_RIGHT_BOTTOM:
				i->first->Move(dx,dy);
				break;
			case L_CENTER:
				i->first->MoveTo((D3DKernel::ResX()-i->first->GetLocation()->Region()->Width())/2,(D3DKernel::ResY()-i->first->GetLocation()->Region()->Height())/2);
				break;
			}
		}
	}
}

void Dialog::AlignWidget(Widget* widget)
{
	std::map<Widget*,LINK>::iterator i = m_LinkedWidgets.find(widget);

	int dx = m_ResX - m_MinResX;
	int dy = m_ResY - m_MinResY;

	if((i != m_LinkedWidgets.end()) && (dx||dy||(i->second == L_CENTER)))
	{
		switch(i->second)
		{
		case L_LEFT_TOP:
			break;
		case L_LEFT_BOTTOM:
			i->first->Move(0,dy);
			break;
		case L_RIGHT_TOP:
			i->first->Move(dx,0);
			break;
		case L_RIGHT_BOTTOM:
			i->first->Move(dx,dy);
			break;
		case L_CENTER:
			i->first->MoveTo((m_ResX-i->first->GetLocation()->Region()->Width())/2,(m_ResY-i->first->GetLocation()->Region()->Height())/2);
			break;
		}
	}
}
