/***********************************************************************

                         Virtual Interface Layer

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   28.08.2000

************************************************************************/
#ifndef _V_BUTTON_H_
#define _V_BUTTON_H_

#include "VLagoon.h"

class VButton : public VLagoon
{
public:
	enum VB_NOTIFY {VBN_DOWN=100,VBN_UP=101};
protected:
	enum VB_STATE {VBS_NORMAL,VBS_SELECTED,VBS_PUSHED};
protected:
	VB_STATE m_vbState;
	bool m_bCaptureMouse;
public:
	VButton(const char *pName);
	virtual ~VButton();
public:
	void SetMouseCapture(bool bCapture);
public:
	//	сообщения, передаваемые контейнером от мыши
	virtual void OnMouseIn(VMouseState *pMS);
	virtual void OnMousePresent(VMouseState *pMS);
	virtual void OnMouseOut(VMouseState *pMS);
	//	получение сообщений от фокуса ввода
	virtual void OnFocusIn(void);
	virtual void OnFocusOut(void);
public:
	//	события кнопки
	virtual void OnPushed(void);
	virtual void OnSelected(void);
	virtual void OnNormal(void);
};

#endif	//_V_BUTTON_H_