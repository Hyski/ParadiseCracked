/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   07.05.2001

************************************************************************/
#ifndef _SLIDE_H_
#define _SLIDE_H_

#include "Widget.h"

class Slide : public Widget
{
private:
	float m_Value;
	float m_Step;
	Widget* m_Image;
public:
	Slide(const char *pName);
	virtual ~Slide();
public:
	//	установить значение нормированное к единице
	void SetValue(float v);
	//	установить шаг
	void SetStep(float s);
	//	получить значение
	float Value(void) const;
public:
	//	установить картинку отображения
	virtual void SetImage(WidgetImage* pImage);
	//	обработка мышинного ввода
	virtual bool ProcessMouseInput(VMouseState* pMS);
	//	обработка клавиатурного ввода
	virtual bool ProcessKeyboardInput(VKeyboardState* pKS);
private:
	void Update(void);
public:
	virtual const char* Type(void) const {return "slider";}
};

//	получить значение
inline float Slide::Value(void) const
{
	return m_Value;
}

#endif	//_SLIDE_H_