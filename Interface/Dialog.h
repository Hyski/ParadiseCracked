/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#ifndef _DIALOG_H_
#define _DIALOG_H_

#include "../Common/UI/Widget.h"

class Button;
class Static;

class Dialog : public Widget
{
public:
	enum LINK {L_NONE,L_LEFT_TOP,L_LEFT_BOTTOM,L_RIGHT_TOP,L_RIGHT_BOTTOM,L_CENTER};
	static const int m_MinResX;
	static const int m_MinResY;
private:
	std::map<Widget*,LINK> m_LinkedWidgets;
protected:
	int m_ResX;
	int m_ResY;
public:
	Dialog(const char *pName,const char *pScriptName);
	virtual ~Dialog();
public:
	//	добавляем виджет с указанием его привязки 
	virtual Widget* Insert(Widget *widget,LINK link);
public:
	virtual bool ProcessMouseInput(VMouseState* pMS);
	virtual void OnKeyboardLost(void);
	virtual bool ProcessKeyboardInput(VKeyboardState* pKS);
	virtual void OnChange(Widget* pChild);
	virtual void OnSystemChange(void);
	virtual bool Delete(const char *pName);
	//	произвести настройку с текущим разрешением экрана
	virtual void CheckConformMode(void);
	void AlignWidget(Widget* widget);
protected:
	virtual void OnButton(Button * /*pButton*/){};
	virtual void OnStatic(Static * /*pStatic*/){};
public:
	virtual const char* Type(void) const {return "dialog";};
};

#endif	//_DIALOG_H_