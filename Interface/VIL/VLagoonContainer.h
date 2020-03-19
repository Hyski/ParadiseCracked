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
	bool m_bMouseCapture;			//	флаг того что сообщения от мыши попадают только в определенную лагуну
protected:
	//	работа с фокусом
	VLagoon *m_pUnderFocus;			//	индекс элемента на котором находиться фокус
protected:
	VLagoon *m_pDragged;			//	лагуна перетаскиваемая

public:
	VLagoonContainer(const char *pName);
	virtual ~VLagoonContainer();
public:
	//	передача управления данному контейнеру
	virtual void Tick(VMouseState *pMS);
	//	при активизации и деактивизации контейнера
	virtual void Deactivate(VMouseState *pMS);
	virtual void Activate(void);
	//	управление с фокусом ввода
	virtual void NextFocus(void);
	virtual void PrevFocus(void);
	virtual void SetFocus(VLagoon* pLagoon);
	//	получение информации
	VLagoon* GetFocus(void);
public:
	//	обработка событий
	virtual void OnDeleteChild(VLagoon *pLagoon);
	virtual void OnMouseCapture(bool bCapture);
	virtual void OnNotify(VLagoon *pLagoon,int iMsg);
public:
	virtual void OnMoveDragges(VLagoon *pLagoon);
	virtual bool OnDrag(VLagoon *pLagoon,VMouseState *pMS);	//	запрос на перетаскивание
	virtual bool OnDrop(VLagoon *pLagoon,VMouseState *pMS);	//	запрос на сбрасывание



};

inline VLagoon* VLagoonContainer::GetFocus(void)
{
	return m_pUnderFocus;
}


#endif	//_VLAGOON_CONTAINER_H_