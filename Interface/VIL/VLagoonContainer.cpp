/***********************************************************************

                         Virtual Interface Layer

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   22.07.2000

************************************************************************/
#include "precomp.h"
#include "VLagoonContainer.h"

VLagoonContainer::VLagoonContainer(const char *pName) : VLagoon(pName)
{
	m_pUnderMouse = 0;
	m_bMouseCapture = false;
	//	��������� ����� ����� 
	m_pUnderFocus = 0;
	//	��������� Drag'n'Drop
	m_pDragged = 0;
}

VLagoonContainer::~VLagoonContainer()
{
}

/***********************************************************************
	Class:			VLagoonContainer
	Method:			Tick(VMouseState *pMS);
	Description:	���������� ���� �������� ������� �� ����. �������� ��
					��������� ������.

************************************************************************/
void VLagoonContainer::Tick(VMouseState *pMS)
{
	VLagoon *pLagoon;

	//	����������
	if(m_pDragged)
	{
		OnMoveDragges(m_pDragged);
		m_pDragged->OnMousePresent(pMS);
	}
	else
	{
		//	-------------------------------------
		if((pLagoon = GetLagoon(pMS->x,pMS->y)))
		{//	�������� ������ ��� �������� ����
			//	����� ���������� ��� ������� ��� ����:
			//	MouseIn, MousePresent, MouseOut
			if((m_pUnderMouse != pLagoon))
			{//	����� ������ ��� ������ �� ������
				if(!m_bMouseCapture)
				{//	���� ��������
					if(m_pUnderMouse)
						m_pUnderMouse->OnMouseOut(pMS);
					pLagoon->OnMouseIn(pMS);
					m_pUnderMouse = pLagoon;
				}
				else
				{//	���� ���������
					m_pUnderMouse->OnMousePresent(pMS);
					if(!m_bMouseCapture)
					{
						m_pUnderMouse->OnMouseOut(pMS);
						pLagoon->OnMouseIn(pMS);
						m_pUnderMouse = pLagoon;
					}
				}
			}
			else
			{//	����� ������������ �� ������ ��� �����
				m_pUnderMouse->OnMousePresent(pMS);
			}
			//	�������� �� Drag'n'Drop
			/*		if(pLagoon->IsDragged())
			{
			if(m_pDragged)
			pLagoon->OnDragEnd(pMS);
			else
			m_pDragged = pLagoon;
		}*/
		}
		else
		{//	������ ��� �������� ���� ���
			if(m_pUnderMouse)
			{//	�������� ��������� ������, ���������� ����
				if(!m_bMouseCapture)
				{//	���� ��������
					m_pUnderMouse->OnMouseOut(pMS);
					m_pUnderMouse = 0;
				}
				else
				{
					m_pUnderMouse->OnMousePresent(pMS);
					if(!m_bMouseCapture)
					{
						m_pUnderMouse->OnMouseOut(pMS);
						m_pUnderMouse = 0;
					}
				}
			}
		}
		//	�������� ���������� ������ ��� �������
		if(m_pUnderFocus)
			m_pUnderFocus->OnFocusPresent();
	}
}


//************************************************************************//
void VLagoonContainer::OnDeleteChild(VLagoon *pLagoon)
{
	//	�������� �� ����������� ������ ��� �����
	if(m_pUnderMouse && m_pUnderMouse == pLagoon)
	{//	�������� ������ ���������
		m_pUnderMouse = 0;
	}
	//	�������� �� ����������� ������ ��� �������
	if(m_pUnderFocus && m_pUnderFocus == pLagoon)
	{//	�������� ������ ���������
		m_pUnderFocus = 0;
	}

}

void VLagoonContainer::OnMouseCapture(bool bCapture)
{
	m_bMouseCapture = bCapture;
}

//************************************************************************//
void VLagoonContainer::Deactivate(VMouseState *pMS)
{
	if(m_pUnderMouse)
		m_pUnderMouse->OnMouseOut(pMS);
	if(m_pUnderFocus)
		m_pUnderFocus->OnFocusOut();
	m_bMouseCapture = false;
	m_pUnderMouse = 0;
}

void VLagoonContainer::Activate(void)
{
	//	������������� ����� ����� �� �������
	if(m_pUnderFocus)
		m_pUnderFocus->OnFocusIn();
	else
	{//	���� ������ ������� �� ZOrder'�, ������� ����� �������� �����
		for(int i=0;i<m_zBuffer.size();i++)
		{
			if(m_zBuffer[i]->IsFocusRequired())
			{
				m_pUnderFocus = m_zBuffer[i];
				m_pUnderFocus->OnFocusIn();
				break;
			}
		}
	}
}
//************************************************************************//
void VLagoonContainer::NextFocus(void)
{
	int i,iFocus = 0;
	bool bFlag = false;

	if(m_pUnderFocus)
	{
		for(i=0;i<m_zBuffer.size();i++)
		{
			if(m_zBuffer[i] == m_pUnderFocus)
			{
				bFlag = true;
				iFocus = i;
				continue;
			}
			if(bFlag && m_zBuffer[i]->IsFocusRequired())
			{
				m_pUnderFocus->OnFocusOut();
				m_pUnderFocus = m_zBuffer[i];
				m_pUnderFocus->OnFocusIn();
				bFlag = false;
				break;
			}
		}
		if(bFlag)
		{
			for(i=0;i<iFocus;i++)
			{
				if(m_zBuffer[i]->IsFocusRequired())
				{
					m_pUnderFocus->OnFocusOut();
					m_pUnderFocus = m_zBuffer[i];
					m_pUnderFocus->OnFocusIn();
					break;
				}
			}
		}
	}
}

void VLagoonContainer::PrevFocus(void)
{
	int i,iFocus = 0;
	bool bFlag = false;

	if(m_pUnderFocus)
	{
		for(i=(m_zBuffer.size()-1);i>=0;i--)
		{
			if(m_zBuffer[i] == m_pUnderFocus)
			{
				bFlag = true;
				iFocus = i;
				continue;
			}
			if(bFlag && m_zBuffer[i]->IsFocusRequired())
			{
				m_pUnderFocus->OnFocusOut();
				m_pUnderFocus = m_zBuffer[i];
				m_pUnderFocus->OnFocusIn();
				bFlag = false;
				break;
			}
		}
		if(bFlag)
		{
			for(i=(m_zBuffer.size()-1);i>iFocus;i--)
			{
				if(m_zBuffer[i]->IsFocusRequired())
				{
					m_pUnderFocus->OnFocusOut();
					m_pUnderFocus = m_zBuffer[i];
					m_pUnderFocus->OnFocusIn();
					break;
				}
			}
		}
	}
}

void VLagoonContainer::SetFocus(VLagoon* pLagoon)
{
	std::map<std::string,VLagoon*>::iterator iChild;

	//	�������� �� �������������� ������ ������

	if(pLagoon && (pLagoon != m_pUnderFocus) && pLagoon->IsFocusRequired())
	{
		iChild = m_mChildren.find(pLagoon->Name());
		if((iChild != m_mChildren.end()) && (iChild->second == pLagoon))
		{
			if(m_pUnderFocus)
				m_pUnderFocus->OnFocusOut();
			m_pUnderFocus = pLagoon;
			m_pUnderFocus->OnFocusIn();
		}
	}
}

void VLagoonContainer::OnNotify(VLagoon *pLagoon,int iMsg)
{
	switch(iMsg)
	{
	case VLC_SETFOCUS:
		SetFocus(pLagoon);
		break;
	case VLC_NEXTFOCUS:
		NextFocus();
		break;
	case VLC_PREVFOCUS:
		PrevFocus();
		break;
	}
}

void VLagoonContainer::OnMoveDragges(VLagoon *pLagoon)
{
}

bool VLagoonContainer::OnDrag(VLagoon *pLagoon,VMouseState *pMS)
{
	if(pLagoon->IsCanDragged() && !m_pDragged)
	{
		BringToTop(pLagoon->Name());
		pLagoon->SetDragged(true);
		m_pDragged = pLagoon;
		return true;
	}

	return false;
}

bool VLagoonContainer::OnDrop(VLagoon *pLagoon,VMouseState *pMS)
{
	pLagoon->SetDragged(false);
	m_pDragged = 0;

	return true;
}
