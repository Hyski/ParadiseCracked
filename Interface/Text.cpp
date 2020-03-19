/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   07.03.2001

************************************************************************/
#include "Precomp.h"
#include "../Common/FontMgr/FontMgr.h"
#include "../Common/FontMgr/Font2D.h"
#include "DXImageMgr.h"
#include "Text.h"

Text::Text(const char *pName) : Widget(pName)
{
	Focus()->Enable(WidgetFocus::FF_KEYBOARD,false);
	//	------------------------------------
	m_Aling = 0;
	m_FontName = "none";
	m_FontColor = 0;
	//	------------------------------------
	m_ImageWidth = 0;
	m_ImageHeight = 0;
}

Text::~Text()
{
}

//	задать прямоугольник вывода текста (new_image - создавать новую картинку в любом случае)
void Text::SetRegion(int left,int top,int right,int bottom,bool new_image)
{
	if(m_ImageWidth<(right-left) || m_ImageHeight<(bottom-top) || new_image)
	{
		m_ImageWidth = right-left;
		m_ImageHeight = bottom-top;
		if(GetImage()) DXImageMgr::Instance()->Release(static_cast<DXImage*>(GetImage()));
		SetImage(DXImageMgr::Instance()->CreateImage(m_ImageWidth,m_ImageHeight));
	}
	GetLocation()->Create(left,top,right,bottom);
	SetText(m_Text.SourceText().c_str());
}

//	установить шрифт
void Text::SetFont(const char* name,int color)
{
	m_FontName = name;
	m_FontColor = color;
}

//	установить текст
void Text::SetText(const char* text)
{
	LPDIRECTDRAWSURFACE7 surface;

	m_Text.Resolve(text,
				   FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor),
				   m_Aling,
				   GetLocation()->Region()->Width(),
				   GetLocation()->Region()->Height());
	if(GetImage() && (surface = ((DXImage *)GetImage())->m_pSurface))
	{
		//	очищаем поверхность
		ClearSurface(surface);
		//	блитим туда текст
		FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->TextOut(surface,0,GetLocation()->Region()->Width(),GetLocation()->Region()->Height(),&m_Text);
	}
}

//	выровнять текст
void Text::Align(int align)
{
	m_Aling = align;
}

void Text::OnSystemChange(void)
{
	LPDIRECTDRAWSURFACE7 surface;

	if(GetImage())
	{
		m_ImageWidth = GetLocation()->Region()->Width();
		m_ImageHeight = GetLocation()->Region()->Height();
		if(GetImage()) DXImageMgr::Instance()->Release(static_cast<DXImage*>(GetImage()));
		SetImage(DXImageMgr::Instance()->CreateImage(m_ImageWidth,m_ImageHeight));
		if(GetImage() && (surface = ((DXImage *)GetImage())->m_pSurface))
		{
			//	очищаем поверхность
			ClearSurface(surface);
			//	блитим туда текст
			FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->TextOut(surface,0,GetLocation()->Region()->Width(),GetLocation()->Region()->Height(),&m_Text);
		}
	}
}

//	обновить текст (например если поменялся шрифт)
void Text::Update(void)
{
	LPDIRECTDRAWSURFACE7 surface;

	if(GetImage() && (surface = ((DXImage *)GetImage())->m_pSurface))
	{
		//	очищаем поверхность
		ClearSurface(surface);
		//	блитим туда текст
		FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->TextOut(surface,0,GetLocation()->Region()->Width(),GetLocation()->Region()->Height(),&m_Text);
	}
}

//*********************************************************************************//
void Text::ClearSurface(LPDIRECTDRAWSURFACE7 surface)
{
	DDSURFACEDESC2 ddsd;
	memset(&ddsd,0,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	surface->GetSurfaceDesc(&ddsd);
	DDBLTFX ddfx;
	memset(&ddfx,0,sizeof(DDBLTFX));
	ddfx.dwSize = sizeof(DDBLTFX);
	ddfx.dwFillColor = ddsd.ddpfPixelFormat.dwGBitMask|
					   ddsd.ddpfPixelFormat.dwBBitMask;
	//	очищаем поверхность
	surface->Blt(0,0,0,DDBLT_COLORFILL,&ddfx);
}
//	получить фонт
const Font* Text::GetFont(void)
{
	return FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor);
}
