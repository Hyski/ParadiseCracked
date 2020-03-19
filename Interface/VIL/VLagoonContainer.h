/***********************************************************************

                         Virtual Interface Layer

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   22.07.2000

************************************************************************/
#ifndef _VLAGOON_CONTAINER_H_
#define _VLAGOON_CONTAINER_H_

#include "VLagoon.h"

class VLagoonContainer : public VLagoon
{
public:
	enum VLC_COMMAND {VLC_SETFOCUS	=	0,
					  VLC_NEXTFOCUS	=	1,
					  VLC_PREVFOCUS	=	2};
protected:
	VLagoon *m_pUnderMouse;
	bool m_bMouseCapture;			//	���� ���� ��� ��������� �� ���� �������� ������ � ������������ ������
protected:
	//	������ � �������
	VLagoon *m_pUnderFocus;			//	������ �������� �� ������� ���������� �����
protected:
	VLagoon *m_pDragged;			//	������ ���������������

public:
	VLagoonContainer(const char *pName);
	virtual ~VLagoonContainer();
public:
	//	�������� ���������� ������� ����������
	virtual void Tick(VMouseState *pMS);
	//	��� ����������� � ������������� ����������
	virtual void Deactivate(VMouseState *pMS);
	virtual void Activate(void);
	//	���������� � ������� �����
	virtual void NextFocus(void);
	virtual void PrevFocus(void);
	virtual void SetFocus(VLagoon* pLagoon);
	//	��������� ����������
	VLagoon* GetFocus(void);
public:
	//	��������� �������
	virtual void OnDeleteChild(VLagoon *pLagoon);
	virtual void OnMouseCapture(bool bCapture);
	virtual void OnNotify(VLagoon *pLagoon,int iMsg);
public:
	virtual void OnMoveDragges(VLagoon *pLagoon);
	virtual bool OnDrag(VLagoon *pLagoon,VMouseState *pMS);	//	������ �� ��������������
	virtual bool OnDrop(VLagoon *pLagoon,VMouseState *pMS);	//	������ �� �����������



};

inline VLagoon* VLagoonContainer::GetFocus(void)
{
	return m_pUnderFocus;
}


#endif	//_VLAGOON_CONTAINER_H_