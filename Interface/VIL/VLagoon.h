/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.08.2000

************************************************************************/
#ifndef _VLAGOON_H_
#define _VLAGOON_H_

#include <map>
#include <string>
#include <vector>
#include "VRegion.h"
#include "VMouseState.h"
#include "../../Common/GraphPipe/GraphPipe.h"

class VLagoon
{
protected:
	char *m_pName;
	std::string m_sClass;	
protected:			//	прямоугольник вывода
	//	виртуальные координаты
	float m_x;					//	коордтнаты левого-верхнего угла прямоугольника
	float m_y;
	float m_width;				//	ширина и высота прямоугольника
	float m_height;
protected:			// регион ввода	
	VRegion m_VirtualRegion;	//	виртуальная региональная область ввода
	VRegion m_ScreenRegion;		//	экранная региональная область ввода
protected:
	float m_vResX;				//	виртуальные размеры экрана
	float m_vResY;
	float m_sResX;				//	экранные размеры экрана
	float m_sResY;
protected:
	std::map<std::string,VLagoon*> m_mChildren; 		//	карта дочерних областей
	VLagoon *m_pParent;			//	родительская лагуна
protected:
	//	все что связанно с z координатой
	float m_zBegin;		//	начало отпущенного промежутка - текущая координа Z
	float m_zEnd;		//	конец отпущенного промежутка
	//	расположение объектов по z
	std::vector<VLagoon*> m_zBuffer;	
protected:
	//	все что связано с получением фокуса ввода с клавиатуры
	bool m_bFocus;				//	имеется ли фокус ввода на данной лагуне
	bool m_bFocusRequired;		//	требуется передавать фокус ввода данному элементу
protected:
	//	все что связанно с Drag & Drop'ом
	bool m_bDrag;
	bool m_bDragged;			//	лагуна в данный момент перетаскивается
public:
	//	виртуальные координаты
	float m_xOffsetInDrag;		//	смещение места взятия
	float m_yOffsetInDrag;		//	смещение места взятия
protected:
	//	данные для отрисовки
	point3 m_Vertex[4];
	texcoord m_TexCoord[4];
	Primi m_Object;
public:
	std::string m_sShader;
	bool m_bVisible;			//	отображать данную лагуну или нет
//	bool m_bEnable;				//	замечать ли эту лагуну или нет*/
public:
	VLagoon(const char *pName);
	virtual ~VLagoon();
public:
	//	функция пересчета под новые экранные и виртуальные координаты
	virtual void ScreenChanged(float vResX,float vResY,float sResX,float sResY);
	//	функции задания виртуальных координат
	virtual void SetRegion(float x,float y,float right,float bottom);
	virtual void SetRegion(VPoint *points,unsigned int num);
	//	функции изменения местоположение (виртуальные координаты)
	virtual void MoveTo(float x,float y);
	//	функции для работы с дочерними окнами
	virtual bool AddChild(VLagoon *pLagoon);
	virtual void DeleteChild(const char *pName);
	virtual void SetParent(VLagoon *pLagoon);
	virtual void BringToTop(const char *pName);
	virtual void BringToBottom(const char *pName);
	unsigned int ChildrenNum(void);		//	кол-во дочерних объектов
	//	---------------------------------------------------------------
	virtual void PaintRegion(HDC hdc);
	virtual void Render(GraphPipe *lpGraphPipe);
	virtual void SetVertex(void);
	//	сообщения, передаваемые контейнером при удалении дочернего окна
	virtual void OnDeleteChild(VLagoon *pLagoon);
	//	функция установки z кординаты
	void SetZOrder(float zBegin,float zEnd);
	//	сообщения, передаваемые контейнером от мыши
	virtual void OnMouseIn(VMouseState *pMS);
	virtual void OnMousePresent(VMouseState *pMS);
	virtual void OnMouseOut(VMouseState *pMS);
	//	сообщение для захвата и освобождеия мыши
	virtual void OnMouseCapture(bool bCapture);
	//	сообщения передаваемые в связи с событиями фокуса ввода
	virtual void OnFocusIn(void);
	virtual void OnFocusPresent(void);
	virtual void OnFocusOut(void);
	//	нотификационные сообщения
	virtual void OnNotify(VLagoon *pLagoon,int iMsg);
	//	сообщения связанные с Drag'n'Drop'ом
	virtual bool OnDrag(VLagoon *pLagoon,VMouseState *pMS);	//	запрос на перетаскивание
	virtual void OnDragEnd(VMouseState *pMS);	//	уведомление о том, что объект должен быть скинут
	virtual bool OnDrop(VLagoon *pLagoon,VMouseState *pMS);	//	запрос на сбрасывание
public:
	//	информационные функции
	const char* Name(void);
	float X(void);
	float Y(void);
	float Width(void);
	float Height(void);
	float Right(void);
	float Bottom(void);
	float ZBegin(void);
	float ZEnd(void);
	//	информация о фокусе ввода
	bool IsFocusRequired(void);
	// тест на принадлежность точки региону (экранные координаты)
	bool PtInLagoon(float x,float y);
	//	информация об окне в точке  (экранные координаты)
	VLagoon* GetLagoon(float x,float y);
//	VLagoon* GetVLagoon(float x,float y);
	//	касаемо drag'n'drop
	bool IsDragged(void);
	bool IsCanDragged(void);
	void SetDragged(bool bDragged);
public:
	//	возвращает указатель на данную реализацию класса
//	virtual void* This(void) {return this;};
	//	возвращает указатель на базовый класс VLagoon
//	VLagoon* VThis(void) {return (VLagoon *)VLagoon::This();};
	bool IsClass(const char *pClass);
};

//*********************************************************************//
inline const char* VLagoon::Name(void)
{
	return m_pName;
}

inline float VLagoon::X(void)
{
	return m_x;
}

inline float VLagoon::Y(void)
{
	return m_y;
}

inline float VLagoon::Width(void)
{
	return m_width;
}

inline float VLagoon::Height(void)
{
	return m_height;
}

inline float VLagoon::Right(void)
{
	return m_x+m_width;
}

inline float VLagoon::Bottom(void)
{
	return m_y+m_height;
}
//	возвращаем промежуток по z
inline float VLagoon::ZBegin(void)
{
	return m_zBegin;
}
//	возвращаем промежуток по z
inline float VLagoon::ZEnd(void)
{
	return m_zEnd;
}
// тест на принадлежность точки региону (экранные координаты)
inline bool VLagoon::PtInLagoon(float x,float y)
{
	return m_ScreenRegion.PtInRegion(x,y);
}
//	кол-во дочерних объектов
inline unsigned int VLagoon::ChildrenNum(void)
{
	return m_mChildren.size();
}
//	возможно ли получение фокуса ввода
inline bool VLagoon::IsFocusRequired(void)
{
	return m_bFocusRequired;
}

inline bool VLagoon::IsDragged(void)
{
	return m_bDragged;
}

inline bool VLagoon::IsCanDragged(void)
{
	return m_bDrag;
}

inline void VLagoon::SetDragged(bool bDragged)
{
	m_bDragged = bDragged;
}


#endif	//_VLAGOON_H_