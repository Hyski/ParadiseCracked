/***********************************************************************

                         Virtual Interface Layer

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   28.08.2000

************************************************************************/
#include "precomp.h"
#include "VLagoonContainer.h"
#include "VButton.h"

VButton::VButton(const char *pName) : VLagoon(pName)
{
	m_sClass = "button";
	m_vbState = VBS_NORMAL;
	m_bCaptureMouse = true;
	m_bFocusRequired = true;
}

VButton::~VButton()
{
}

void VButton::OnMouseIn(VMouseState *pMS)
{
	if(pMS->LButtonState)
	{
		m_vbState = VBS_PUSHED;
		if(m_bCaptureMouse)
		{
			if(m_pParent)
				m_pParent->OnMouseCapture(true);
		}
		OnPushed();
		//	информируем родительское окно
		OnNotify(this,VBN_DOWN);
	}
	else
	{
		m_vbState = VBS_SELECTED;
		OnSelected();
	}
	//	установка фокуса ------------------------------------------
	if(m_pParent)
		m_pParent->OnNotify(this,VLagoonContainer::VLC_SETFOCUS);
}

void VButton::OnMousePresent(VMouseState *pMS)
{
	if(pMS->LButtonState)
	{
		if(m_vbState != VBS_PUSHED)
		{
			m_vbState = VBS_PUSHED;
			OnPushed();
			//	установка фокуса ------------------------------------------
			if(!m_bFocus && m_pParent)
				m_pParent->OnNotify(this,VLagoonContainer::VLC_SETFOCUS);
			if(m_bCaptureMouse)
			{
				if(m_pParent)
					m_pParent->OnMouseCapture(true);
			}
			//	информируем родительское окно
			OnNotify(this,VBN_DOWN);
		}
	}
	else
	{
		if(m_vbState!=VBS_SELECTED)
		{
			if(pMS->dx || pMS->dy || m_bFocus)
			{
				if(m_bCaptureMouse)
				{
					if(m_pParent)
						m_pParent->OnMouseCapture(false);
				}
				m_vbState = VBS_SELECTED;
				OnSelected();
				//	установка фокуса ------------------------------------------
				if(!m_bFocus && m_pParent)
					m_pParent->OnNotify(this,VLagoonContainer::VLC_SETFOCUS);
				if(m_pParent)
				{
					if(m_pParent->GetLagoon(pMS->x,pMS->y) == this)
					{
						//	информируем родительское окно
						OnNotify(this,VBN_UP);
					}
				}
			}
		}
	}
}

void VButton::OnMouseOut(VMouseState *pMS)
{
	if(m_bCaptureMouse && m_vbState != VBS_PUSHED)
	{
		if(m_pParent)
			m_pParent->OnMouseCapture(false);
	}
}

void VButton::SetMouseCapture(bool bCapture)
{
	m_bCaptureMouse = bCapture;
}

void VButton::OnFocusIn(void)
{
	VLagoon::OnFocusIn();
	//	--------------------
	m_vbState = VBS_SELECTED;
	OnSelected();
}

void VButton::OnFocusOut(void)
{
	VLagoon::OnFocusOut();
	//	--------------------
	m_vbState = VBS_NORMAL;
	OnNormal();
}

//*************************************************************************//
void VButton::OnPushed(void)
{
}

void VButton::OnSelected(void)
{
}

void VButton::OnNormal(void)
{
}

