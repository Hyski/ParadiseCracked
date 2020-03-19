/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   28.02.2001

************************************************************************/
#include "Precomp.h"
#include "../Image/Image.h"
#include "../Image/ImageFactory.h"
#include "DDSurface.h"

//---------- Лог файл ------------
#ifdef _DEBUG_GRAPHICS
CLog srf_log;
#define srf	srf_log["srf.log"]
#else
#define srf	/##/
#endif
//--------------------------------

DDSurface::DDSurface() : m_pSurface(0)
{
}

DDSurface::DDSurface(LPDIRECTDRAW7 pDD,const Image* pImage) : m_pSurface(0)
{
	Create(pDD,pImage);
}

DDSurface::DDSurface(LPDIRECTDRAW7 pDD,LPDIRECTDRAWSURFACE7 surface) : m_pSurface(0)
{
	Create(pDD,surface);
}

DDSurface::~DDSurface()
{
	Release();
}

void DDSurface::Release(void)
{
	if(m_pSurface)
	{
		srf("Surface [%0.8x] released;\n",m_pSurface);
		m_pSurface->Release();
		m_pSurface = 0;
	}
}

bool DDSurface::Create(LPDIRECTDRAW7 pDD,const Image* pImage)
{
	DDSURFACEDESC2 ddsd;
	HDC hdc;

	//	если уже что-то есть - уничтожаем
	if(m_pSurface) Release();
	//	-------------------------------------
	if(pDD && pImage && pImage->BitImage())
	{
		memset(&ddsd,0,sizeof(ddsd));
		ddsd.dwSize  = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH; 
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY; 
		ddsd.dwWidth  = pImage->Width();
		ddsd.dwHeight = pImage->Height();
		if(FAILED(pDD->CreateSurface(&ddsd,&m_pSurface,0)))
		{
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY; 
			pDD->CreateSurface(&ddsd,&m_pSurface,0);
		}
		if(m_pSurface)
		{
			srf("Surface [%0.8x] created in %s memory;\n",m_pSurface,(ddsd.ddsCaps.dwCaps&DDSCAPS_SYSTEMMEMORY)?"SYSTEM":"VIDEO");
			if(!FAILED(m_pSurface->GetDC(&hdc)))
			{
				ImageFactory::DrawImage(hdc,pImage);
				m_pSurface->ReleaseDC(hdc);
				return true;
			}
		}
	}

	return false;
}

//	создать копию другой поверхности
bool DDSurface::Create(LPDIRECTDRAW7 pDD,LPDIRECTDRAWSURFACE7 surface)
{
	//	если уже что-то есть - уничтожаем
	if(m_pSurface) Release();
	//	-------------------------------------
	if(pDD && surface)
	{
		DDSURFACEDESC2 ddsd;

		memset(&ddsd,0,sizeof(ddsd));
		ddsd.dwSize  = sizeof(ddsd);

		surface->GetSurfaceDesc(&ddsd);

		ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH; 
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY; 

		if(FAILED(pDD->CreateSurface(&ddsd,&m_pSurface,0)))
		{
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY; 
			pDD->CreateSurface(&ddsd,&m_pSurface,0);
		}

		if(m_pSurface)
		{
			srf("Surface [%0.8x] created in %s memory;\n",m_pSurface,(ddsd.ddsCaps.dwCaps&DDSCAPS_SYSTEMMEMORY)?"SYSTEM":"VIDEO");

			if(!FAILED(m_pSurface->BltFast(0,0,surface,0,DDBLTFAST_WAIT)))
			{
				return true;
			}
		}
	}

	return false;
}