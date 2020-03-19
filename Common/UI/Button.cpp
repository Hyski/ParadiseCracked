/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
     нопка.
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   28.02.2001

************************************************************************/
#include "Precomp.h"
#include "Button.h"

Button::Button(const char *pName) : Widget(pName),m_FirstSelect(false),
									m_NoFocus(false)
{
	m_BS = BS_NORMAL;
	memset(m_pImages,0,sizeof(m_pImages));
	for(int i=0;i<MAX_STATE;i++)
	{
		m_pImages[i] = 0;
		m_Samples[i] = 0;
	}
}

Button::~Button()
{
	Widget::SetImage(0);
	for(int i=0;i<MAX_STATE;i++)
	{
		if(m_pImages[i]) m_pImages[i]->Release();
		if(m_Samples[i]) m_Samples[i]->Release();
	}
}

void Button::SetImage(BUTTON_STATE bs,WidgetImage *pImage)
{
	m_pImages[bs] = pImage;
	if(m_BS == bs) 
		Widget::SetImage(pImage);
}

void Button::SetSample(BUTTON_STATE bs,Player::Sample* sample)
{
	m_Samples[bs] = sample;
}

void Button::SetState(BUTTON_STATE bs)
{
	m_BS = bs;
	Widget::SetImage(m_pImages[m_BS]);
}

	//	обработка мышинного ввода
bool Button::ProcessMouseInput(VMouseState* pMS)
{
	if(!m_FirstSelect)
	{
		if(!pMS->LButtonState) PlayState(BS_SELECTED);
		m_FirstSelect = true;
	}
	if((pMS->dx || pMS->dy) && (!pMS->LButtonState) && (!pMS->LButtonBack))
	{
		SetState(BS_SELECTED);
	}
	if(pMS->LButtonState)
	{
		if(m_BS != BS_PUSHED) PlayState(BS_PUSHED);
		SetState(BS_PUSHED);
	}
	if(pMS->LButtonBack)
	{
		PlayState(BS_NORMAL);
		SetState(BS_SELECTED);
		Parent()->OnChange(this);
	}
	if(Parent() && (Parent()->Focus()->Get(WidgetFocus::FF_KEYBOARD) != this) && (pMS->dx || pMS->dy))
			Parent()->Focus()->Set(WidgetFocus::FF_KEYBOARD,this);

	return false;
}

bool Button::ProcessMouseLost(void)
{
	if(m_BS == BS_PUSHED)  PlayState(BS_NORMAL);

	if(!m_NoFocus)
	{
		if(Parent() && (Parent()->Focus()->Get(WidgetFocus::FF_KEYBOARD) != this))
			SetState(BS_NORMAL);
		else
			SetState(BS_SELECTED);
	}
	else
	{
		SetState(BS_NORMAL);
	}

	m_FirstSelect = false;

	return false;
}

//	обработка клавиатурного ввода
bool Button::ProcessKeyboardInput(VKeyboardState* pKS)
{
	if(!m_NoFocus)
	{
		if(m_BS == BS_NORMAL)
		{
			PlayState(BS_SELECTED);
			SetState(BS_SELECTED);
		}
		if(pKS->Front[DIK_RETURN] || pKS->State[DIK_NUMPADENTER])
		{
			if(m_BS != BS_PUSHED) PlayState(BS_PUSHED);
			SetState(BS_PUSHED);
		}
		if(pKS->Back[DIK_RETURN] || pKS->Back[DIK_NUMPADENTER])
		{
			if(m_BS == BS_PUSHED) PlayState(BS_NORMAL);
			SetState(BS_SELECTED);
			if(Parent()) Parent()->OnChange(this);
		}
	}

	return false;
}

bool Button::ProcessKeyboardLost(void)
{
	if(!m_NoFocus)
	{
		SetState(BS_NORMAL);
	}

	return false;
}

void Button::OnSystemChange(void)
{
	WidgetImage* image = GetImage();

	Widget::SetImage(0);
	for(int i=0;i<MAX_STATE;i++)
		m_pImages[i]->Reload();
	Widget::OnSystemChange();
	Widget::SetImage(image);
}
//	проигрывание звука
void Button::PlayState(BUTTON_STATE bs)
{
	if(m_Player) m_Player->Play(m_Samples[bs]);
}
//	отключение клавиатурного фокуса
void Button::NoFocus(bool no_focus)
{
	m_NoFocus = no_focus;
}
