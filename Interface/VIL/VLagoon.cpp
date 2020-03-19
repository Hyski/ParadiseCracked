/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.08.2000

************************************************************************/
#include "precomp.h"
#include "../../globals.h"
#include "../../Common/GraphPipe/GraphPipe.h"
#include "VLagoon.h"

#ifdef _DEBUG_INTERFACE
#include "../Log/Log.h"
CLog vlagoon_log;
#define vl vlagoon_log["vlagoon.log"]
#else
#define vl /##/
#endif	//,_DEBUG_INTERFACE

/***********************************************************************
	Class:			VLagoon
	Constructor:	VLagoon(const char *pName);
	Description:	Конструктора по умолчанию не существует, потому как
					все объекты класса VLagoon должны иметь различные имена.
	Помимо этого в деструкторе уничтожаются все дочерние объекты класса 
	VLagoon. Параметр pName есть имя данной лагуны.

************************************************************************/
VLagoon::VLagoon(const char *pName)
{
	vl("--- VLagoon(\"%s\") ---\n",pName);
	m_pName = new char[strlen(pName)+1];
	if(m_pName)
		strcpy(m_pName,pName);
	m_sClass = "lagoon";
	//	виртуальные размеры экрана
	m_vResX = 1;			
	m_vResY = 1;
	//	экранные размеры экрана
	m_sResX = 1;				
	m_sResY = 1;
	//	прямоугольник вывода (виртуальные координаты)
	m_x = m_y = 0;
	m_width = m_height = 0;
	//	касаемо z координаты
	m_zBegin = 0.1f;
	m_zEnd = 0.9f;
	//	родительская лагуна
	m_pParent = 0;
	//	установки для фокуса ввода
	m_bFocus = false;
	m_bFocusRequired = false;
	//	установки для Drag'n'Drop'а
	m_bDrag = true;
	m_bDragged = false;
	m_xOffsetInDrag = 0;
	m_yOffsetInDrag = 0;
	m_bVisible = true;
	//	инициализация для графического отображения
	m_TexCoord[0] = texcoord(0,0);
	m_TexCoord[1] = texcoord(1,0);
	m_TexCoord[2] = texcoord(1,1);
	m_TexCoord[3] = texcoord(0,1);

	memset(&m_Object,0,sizeof(m_Object));
	m_Object.Pos = m_Vertex;
	m_Object.UVs[0] = m_TexCoord;
	m_Object.IdxNum = 0;
	m_Object.Prim = Primi::TRIANGLEFAN;
	m_Object.Contents = 0;
	m_Object.VertNum = 4;

	m_sShader = "noshader";
	SetVertex();
}

/***********************************************************************
	Class:			VLagoon
	Destructor:		~VLagoon();
	Description:	Уничтожаем все дочерние лагуны. Очищаем память под
					имя данной лагуны.

************************************************************************/
VLagoon::~VLagoon()
{
	std::map<std::string,VLagoon*>::iterator iChild;

	//	уничтожам все дочерние окна
	for(iChild=m_mChildren.begin();iChild!=m_mChildren.end();iChild++)
		delete iChild->second;
	m_mChildren.clear();
	vl("--- destroy VLagoon(\"%s\") ---\n",m_pName);
	//	прежде чем удалить окончательно вызываем у родительского окна уведомляющий метод
	if(m_pParent)
		m_pParent->OnDeleteChild(this/*VThis()*/);
	//	удаляем память выделенную под имя лагуны
	if(m_pName)
		delete[] m_pName;
}

//**********************************************************************//
//******************** ИЗМЕНЕНИЕ РАСПОЛОЖЕНИЯ ЭЛЕМЕНТА *****************//

/***********************************************************************
	Class:			VLagoon
	Method:			ScreenChanged();
	Description:	Устанавливаем новый экранные и виртуальный координаты,
					и всвязи с этим пересчитываем расположение данного
					элемента и всех его дочерних элементов.

************************************************************************/
void VLagoon::ScreenChanged(float vResX,float vResY,float sResX,float sResY)
{
	std::map<std::string,VLagoon*>::iterator iChild;

	m_vResX = vResX;
	m_vResY = vResY;
	m_sResX = sResX;
	m_sResY = sResY;

	m_ScreenRegion = m_VirtualRegion;
	m_ScreenRegion.Transform(sResX/vResX,0,0,
						      0,sResY/vResY,0);
	//	передаем информацию дочерним областям
	for(iChild=m_mChildren.begin();iChild!=m_mChildren.end();iChild++)
		iChild->second->ScreenChanged(vResX,vResY,sResX,sResY);
	//	------------------------------------------------------
	SetVertex();
}

//**************** ФУНКЦИИ ЗАДАНИЯ КООРДИНАТ ********************//
void VLagoon::SetRegion(float x,float y,float right,float bottom)
{
	m_x = x;
	m_y = y;
	m_width = right-x;
	m_height = bottom-y;
	//	-----------------------------------------------
	m_VirtualRegion.Create(x,y,right,bottom);
	//	-----------------------------------------------
	m_ScreenRegion = m_VirtualRegion;
	m_ScreenRegion.Transform(m_sResX/m_vResX,0,0,
						      0,m_sResY/m_vResY,0);
	//	------------------------------------------------------
	SetVertex();
}

void VLagoon::SetRegion(VPoint *points,unsigned int num)
{
	m_VirtualRegion.Create(points,num);
	//	-----------------------------------------------
	m_VirtualRegion.GetRect(&m_x,&m_y,&m_width,&m_height);
	m_width -= m_x;
	m_height -= m_y;
	//	-----------------------------------------------
	m_ScreenRegion = m_VirtualRegion;
	m_ScreenRegion.Transform(m_sResX/m_vResX,0,0,
						      0,m_sResY/m_vResY,0);
	//	------------------------------------------------------
	SetVertex();
}

void VLagoon::MoveTo(float x,float y)
{
	std::map<std::string,VLagoon*>::iterator iChild;

	m_VirtualRegion.Transform(1,0,x-m_x,0,1,y-m_y);
	m_x = x;
	m_y = y;
	//	-----------------------------------------------
	m_ScreenRegion = m_VirtualRegion;
	m_ScreenRegion.Transform(m_sResX/m_vResX,0,0,
						      0,m_sResY/m_vResY,0);
	//	-----------------------------------------------
	//	перемещаем дочерние окна
	for(iChild=m_mChildren.begin();iChild!=m_mChildren.end();iChild++)
		iChild->second->MoveTo(x,y);
	//	------------------------------------------------------
	SetVertex();
}

//**********************************************************************//
//******************** РАБОТА С ДОЧЕРНИМИ ОБЛАСТЯМИ ********************//
bool VLagoon::AddChild(VLagoon *pLagoon)
{
	std::map<std::string,VLagoon*>::iterator iChild;

	if(pLagoon)
	{
		iChild = m_mChildren.find(pLagoon->Name());
		if(iChild == m_mChildren.end())
		{
			//	устанавливаем родительскую лагуну
			pLagoon->SetParent(this/*VThis()*/);
			//	установить родительские форматы экрана
			pLagoon->ScreenChanged(m_vResX,m_vResY,m_sResX,m_sResY);
			//	заносим лагуну в карту дочерних областей
			m_mChildren.insert(std::map<std::string,VLagoon*>::value_type(pLagoon->Name(),pLagoon));
			//	добавляем в конец zBuffer'а
			m_zBuffer.push_back(pLagoon);
			//	меняем расположение дочерних областей по z координате
			SetZOrder(m_zBegin,m_zEnd);

			return true;
		}
	}

	return false;
}

void VLagoon::DeleteChild(const char *pName)
{
	std::map<std::string,VLagoon*>::iterator iChild;
	std::vector<VLagoon*>::iterator iZ;

	iChild = m_mChildren.find(pName);
	if(iChild != m_mChildren.end())
	{
		for(iZ = m_zBuffer.begin();iZ!=m_zBuffer.end();iZ++)
		{
			if(*iZ == iChild->second)
			{
				m_zBuffer.erase(iZ);
				//	меняем расположение дочерних областей по z координате
				SetZOrder(m_zBegin,m_zEnd);
				break;
			}
		}
		delete iChild->second;
		m_mChildren.erase(iChild);
	}
}

void VLagoon::OnDeleteChild(VLagoon *pLagoon)
{
	//	передаем информацию выше
	if(m_pParent)
		m_pParent->OnDeleteChild(pLagoon);
}

void VLagoon::SetParent(VLagoon *pLagoon)
{
	m_pParent = pLagoon;
}

void VLagoon::BringToTop(const char *pName)
{
	VLagoon *pLagoon;
	std::map<std::string,VLagoon*>::iterator iChild;

	iChild = m_mChildren.find(pName);
	if(iChild != m_mChildren.end())
	{
		for(int i=0;i<(m_zBuffer.size()-1);i++)
		{
			if(m_zBuffer[i] == iChild->second)
			{//	нашли лагуну, которую необходимо поднять наверх
				pLagoon = m_zBuffer[i];
				for(;i<(m_zBuffer.size()-1);i++)
				{
					m_zBuffer[i] = m_zBuffer[i+1];
				}
				m_zBuffer[m_zBuffer.size()-1] = pLagoon;
				//	меняем расположение дочерних областей по z координате
				SetZOrder(m_zBegin,m_zEnd);
				break;
			}
		}
	}
}

void VLagoon::BringToBottom(const char *pName)
{
	VLagoon *pLagoon;
	std::map<std::string,VLagoon*>::iterator iChild;

	iChild = m_mChildren.find(pName);
	if(iChild != m_mChildren.end())
	{
		for(int i=(m_zBuffer.size()-1);i>0;i--)
		{
			if(m_zBuffer[i] == iChild->second)
			{//	нашли лагуну, которую необходимо опустить в самый низ
				pLagoon = m_zBuffer[i];
				for(;i>0;i--)
				{
					m_zBuffer[i] = m_zBuffer[i-1];
				}
				m_zBuffer[0] = pLagoon;
				//	меняем расположение дочерних областей по z координате
				SetZOrder(m_zBegin,m_zEnd);
				break;
			}
		}
	}
}

//**********************************************************************//

/***********************************************************************
	Class:			VLagoon
	Method:			GetLagoon();
	Description:	Возвращает указатель на наследуемый класс от VLagoon,
					или же на на сам VLagoon, если от него никто не наследовался.

************************************************************************/
VLagoon* VLagoon::GetLagoon(float x,float y)
{
	VLagoon *pLagoon = 0;

	for(int i=(m_zBuffer.size()-1);i>=0;i--)
		if((pLagoon = m_zBuffer[i]->GetLagoon(x,y)))
			return pLagoon;
	if(PtInLagoon(x,y))
		return this;
	
	return 0;
}

/***********************************************************************
	Class:			VLagoon
	Method:			GetVLagoon();
	Description:	Возвращает указатель на базовый класс.

************************************************************************/
/*
VLagoon* VLagoon::GetVLagoon(float x,float y)
{
	VLagoon *pLagoon = 0;

	for(int i=(m_zBuffer.size()-1);i>=0;i--)
		if((pLagoon = m_zBuffer[i]->GetVLagoon(x,y)))
			return pLagoon;
	if(PtInLagoon(x,y))
		return this;
	
	return 0;
}
*/
//**********************************************************************//
void VLagoon::SetZOrder(float zBegin,float zEnd)
{
	float fStep;

	m_zBegin = zBegin;
	m_zEnd = zEnd;
	//	обновить информацию в дочерних окнах
	fStep = (m_zEnd-m_zBegin)/(m_zBuffer.size()+1);
	for(int i=0;i<m_zBuffer.size();i++)
		m_zBuffer[i]->SetZOrder(m_zBegin+(fStep*(i+1)),m_zBegin+(fStep*(i+1))+fStep);
	//	------------------------------------------------------
	SetVertex();
}

//**********************************************************************//
void VLagoon::OnMouseIn(VMouseState *pMS)
{
}

void VLagoon::OnMousePresent(VMouseState *pMS)
{
	if(pMS->LButtonFront)
	{
		if(!m_bDragged)
			OnDrag(this/**/,pMS);
		else
			OnDrop(this/*VThis()*/,pMS);
/*		m_bWantDragOrDrop = true;
		m_xOffsetInDrag = (int)ToVirtualX((Input::MouseState().x-ToScreenX(m_x)));
		m_yOffsetInDrag = (int)ToVirtualY((Input::MouseState().y-ToScreenY(m_y)));*/
	}
/*	else
	{
		if(m_bDragged)
			OnDrop(VThis(),pMS);
	}*/

}

void VLagoon::OnMouseOut(VMouseState *pMS)
{
}

void VLagoon::OnMouseCapture(bool bCapture)
{
	//	передаем информацию выше
	if(m_pParent)
		m_pParent->OnMouseCapture(bCapture);
}

//**********************************************************************//
void VLagoon::OnFocusIn(void)
{
	m_bFocus = true;
}

void VLagoon::OnFocusPresent(void)
{
}

void VLagoon::OnFocusOut(void)
{
	m_bFocus = false;
}

//**********************************************************************//
void VLagoon::OnNotify(VLagoon *pLagoon,int iMsg)
{
	//	передаем информацию выше
	if(m_pParent)
		m_pParent->OnNotify(pLagoon,iMsg);
}

//**********************************************************************//
bool VLagoon::OnDrag(VLagoon *pLagoon,VMouseState *pMS)
{
	m_xOffsetInDrag = (m_vResX*(pMS->x-((m_sResX*m_x)/m_vResX)))/m_sResX;
	m_yOffsetInDrag = (m_vResY*(pMS->y-((m_sResY*m_y)/m_vResY)))/m_sResY;
	if(m_pParent)
		return m_pParent->OnDrag(pLagoon,pMS);

	return false;
}

void VLagoon::OnDragEnd(VMouseState *pMS)
{
	m_bDragged = false;
}

bool VLagoon::OnDrop(VLagoon *pLagoon,VMouseState *pMS)
{
	if(m_pParent)
		return m_pParent->OnDrop(pLagoon,pMS);
	pLagoon->OnDragEnd(pMS);
	
	return true;
}

//**********************************************************************//
bool VLagoon::IsClass(const char *pClass)
{
	return !strcmp(pClass,m_sClass.c_str());
}

//**********************************************************************//
void VLagoon::PaintRegion(HDC hdc)
{
	RECT rect;
	float left,top,right,bottom;

	if(m_ScreenRegion.Num())
	{
		HBRUSH hBrush = CreateSolidBrush(0xaaaa00+((unsigned int)((0xff*m_zBegin)/0.8)));

		SelectObject(hdc,GetStockObject(BLACK_PEN));
		SelectObject(hdc,hBrush);
		POINT *pPoints = new POINT[m_ScreenRegion.Num()+1];
		int i;
		for(i=0;i<m_ScreenRegion.Num();i++)
		{
			pPoints[i].x = m_ScreenRegion.Points()[i].x;
			pPoints[i].y = m_ScreenRegion.Points()[i].y;
		}
		pPoints[i].x = m_ScreenRegion.Points()[0].x;
		pPoints[i].y = m_ScreenRegion.Points()[0].y;
		Polygon(hdc,pPoints,m_ScreenRegion.Num()+1);

		m_ScreenRegion.GetRect(&left,&top,&right,&bottom);
		rect.left = left;
		rect.top = top;
		rect.right = right;
		rect.bottom = bottom;
		DrawText(hdc,m_pName,-1,&rect,DT_SINGLELINE|DT_VCENTER|DT_CENTER);

		delete pPoints;
		SelectObject(hdc,GetStockObject(WHITE_BRUSH));
		DeleteObject(hBrush);
	}

	//	передаем информацию дочерним областям
	for(int i=0;i<m_zBuffer.size();i++)
		m_zBuffer[i]->PaintRegion(hdc);
}

void VLagoon::Render(GraphPipe *lpGraphPipe)
{
	if(m_bVisible)
	{
		lpGraphPipe->Chop(m_sShader,&m_Object);
		//	передаем информацию дочерним областям
		for(int i=0;i<m_zBuffer.size();i++)
			m_zBuffer[i]->Render(lpGraphPipe);
	}
}

//	--------- ДОПОЛНИТЕЛЬНЫЕ ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ----------------
void VLagoon::SetVertex(void)
{
	m_Vertex[0] = point3(ToScreenX(m_x),ToScreenY(m_y),m_zBegin);
	m_Vertex[1] = point3(m_Vertex[0].x+ToScreenX(m_width),m_Vertex[0].y,m_zBegin);
	m_Vertex[2] = point3(m_Vertex[0].x+ToScreenX(m_width),m_Vertex[0].y+ToScreenY(m_height),m_zBegin);
	m_Vertex[3] = point3(m_Vertex[0].x,m_Vertex[0].y+ToScreenY(m_height),m_zBegin);
}