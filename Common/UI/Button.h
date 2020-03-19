/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
        Кнопка.
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   28.02.2001

************************************************************************/
#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "Widget.h"

class Button : public Widget
{
public:
	enum BUTTON_STATE
	{
		BS_NORMAL,				//	нормальное, обычное состояние
		BS_PUSHED,				//	нажатое состояние
		BS_SELECTED,			//	кнопка под фокусом
		MAX_STATE
	};
private:
	BUTTON_STATE m_BS;
	WidgetImage* m_pImages[MAX_STATE];
	Player::Sample* m_Samples[MAX_STATE];
	bool m_FirstSelect;
	bool m_NoFocus;
public:
	Button(const char *pName);
	virtual ~Button();
public:
	void SetImage(BUTTON_STATE bs,WidgetImage *pImage);
	void SetSample(BUTTON_STATE bs,Player::Sample* sample);
	void SetState(BUTTON_STATE bs);
	BUTTON_STATE GetState(void);
	//	отключение клавиатурного фокуса
	void NoFocus(bool no_focus);
protected:
	//	обработка мышинного ввода
	virtual bool ProcessMouseInput(VMouseState* pMS);
	//	обработка клавиатурного ввода
	virtual bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	обработка потери мышиного ввода
	virtual bool ProcessMouseLost(void);
	//	обработка потери клавиатурного ввода
	virtual bool ProcessKeyboardLost(void);
	//	обработка системного сообщения
	virtual void OnSystemChange(void);
private:
	//	проигрывание звука
	void PlayState(BUTTON_STATE bs);
public:
	virtual const char* Type(void) const {return "button";};
};

inline Button::BUTTON_STATE Button::GetState(void)
{
	return m_BS;
}


#endif	//_BUTTON_H_