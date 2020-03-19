/***********************************************************************

                                 In Bloom

                        Copyright by Naughty 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.09.2000

************************************************************************/
#include "Precomp.h"
#include "GLayer.h"

GLayer::GLayer()
{
	m_hDC = 0;				
	m_hBitmap = 0;			
	m_hLastGDIObject = 0;	

}

GLayer::GLayer(HDC hdc,int x,int y)
{
	m_hDC = 0;				
	m_hBitmap = 0;			
	m_hLastGDIObject = 0;	
	Init(hdc,x,y);
}

GLayer::~GLayer()
{
	Release();
}

void GLayer::Release(void)
{
	if(m_hDC)
	{
		SelectObject(m_hDC,m_hLastGDIObject);
		DeleteObject(m_hBitmap);
		DeleteDC(m_hDC);
		m_hDC = 0;				
		m_hBitmap = 0;			
		m_hLastGDIObject = 0;	
	}
}

void GLayer::Init(HDC hdc,int x,int y)
{
	Release();
	m_hDC = CreateCompatibleDC(hdc);
	m_hBitmap = CreateCompatibleBitmap(hdc,x,y);
	m_hLastGDIObject = SelectObject(m_hDC,m_hBitmap);
	SelectObject(m_hDC,GetStockObject(WHITE_BRUSH));
	SelectObject(m_hDC,GetStockObject(WHITE_PEN));
	Rectangle(m_hDC,0,0,x,y);
}

void GLayer::Flip(HDC hdc,int DestX,int DestY,int DestWidth,int DestHeight,int SrcX,int SrcY,DWORD dwRop)
{
	BitBlt(hdc,DestX,DestY,DestWidth,DestHeight,m_hDC,SrcX,SrcY,dwRop);
}

void GLayer::Flip(HDC hdc,PAINTSTRUCT *lpPS,DWORD dwRop)
{
	BitBlt(hdc,lpPS->rcPaint.left,lpPS->rcPaint.top,
			   lpPS->rcPaint.right,lpPS->rcPaint.bottom,
			   m_hDC,lpPS->rcPaint.left,lpPS->rcPaint.top,dwRop);
}
