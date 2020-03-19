/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
	   Static - это элемент который имеет несколько состояний.
	Состояния характеризуются картинками. Обработка ввода идет
	следующим образом: генерируются события от мыши
	   - LBUTTONDOWN	- кнопка мыши нажалась
	   - LBUTTONUP		- кнопка мыши отжалась
	   - RBUTTONDOWN	- кнопка мыши нажалась и отжалась
	   - RBUTTONUP		- кнопка мыши нажалась и отжалась
	   - LDBLCLICK		- двойной щелчек
	   - RDBLCLICK		- двойной щелчек
	   - MOUSELOST		- мышь убежала
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   11.03.2001

************************************************************************/
#ifndef _STATIC_H_
#define _STATIC_H_

#include "Widget.h"

class Static : public Widget
{
public:
	enum LAST_EVENT
	{
		LE_NONE,
		LE_MOUSEMOVE,		//	- мышка двигается
		LE_LBUTTONDOWN,		//	- кнопка мыши нажалась
		LE_LBUTTONUP,		//	- кнопка мыши отжалась
		LE_LBUTTONPUSHED,	//	- кнопка мыши нажата
		LE_RBUTTONDOWN,		//	- кнопка мыши нажалась и отжалась
		LE_RBUTTONUP,		//	- кнопка мыши нажалась и отжалась
		LE_RBUTTONPUSHED,	//	- кнопка мыши нажата
		LE_LDBLCLICK,		//	- двойной щелчек
		LE_RDBLCLICK,		//	- двойной щелчек
		LE_MOUSELOST		//	- мышь убежала
  	};
public:
	Static(const char *pName);
	virtual ~Static();
private:
	std::vector<WidgetImage*> m_pImages;		//	список картинок		
	int m_State;								//	индекс текущей картинки (номер состояния)
	LAST_EVENT m_LastEvent;
	Widget::Player::Sample* m_ClickSample;
public:
	//	добавить картинку (и соответственно состояние)
	void AddImage(WidgetImage *image);
	//	установить виджет в определенное состояние
	void SetState(unsigned int state);
	//	получить текущее состояние
	int GetState(void) const;
	//	получить кол-во состояний
	unsigned int GetStateSize(void) const;
	//	получить картинку по индексу
	WidgetImage* GetImage(unsigned int state) const;
	//	получить последнее изменение состояния
	Static::LAST_EVENT LastEvent(void) const;
	//	установить звук на клик
	void SetClickSample(Player::Sample* sample);
protected:
	//	обработка мышинного ввода
	virtual bool ProcessMouseInput(VMouseState* pMS);
	//	обработка потери мышиного ввода
	virtual bool ProcessMouseLost(void);
public:
	//	обработка системного сообщения
	virtual void OnSystemChange(void);
public:
	virtual const char* Type(void) const {return "static";};
};

inline int Static::GetState(void) const
{
	return m_State;
}

inline Static::LAST_EVENT Static::LastEvent(void) const
{
	return m_LastEvent;
}
//	получить кол-во состояний
inline unsigned int Static::GetStateSize(void) const
{
	return m_pImages.size();
}

#endif	//_STATIC_H_