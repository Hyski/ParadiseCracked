/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#include "Precomp.h"
#include "BinkSurface.h"

BinkSurface::BinkSurface() : m_pSurface(0),m_hBink(0)
{
}

BinkSurface::~BinkSurface()
{
	Release();
}

void BinkSurface::Release(void)
{
	if(m_pSurface)
	{
		m_pSurface->Release();
		m_pSurface = 0;
	}
	BinkClose(m_hBink);
	m_hBink = 0;
}

bool BinkSurface::Create(LPDIRECTDRAW7 pDD,const char *pFileName)
{
	DDSURFACEDESC2 ddsd;

	//	освобождаем ресурсы
	Release();
	//	открываем бинк
	if(pDD && (m_hBink = BinkOpen(pFileName,0)))
	{
		//	задаем параметры поверхности для бинка
		memset(&ddsd,0,sizeof(ddsd));
		ddsd.dwSize  = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH; 
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY; 
		ddsd.dwWidth  = m_hBink->Width;
		ddsd.dwHeight = m_hBink->Height;
		//	создаем поверхность
		if(FAILED(pDD->CreateSurface(&ddsd,&m_pSurface,0)))
		{
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY; 
			pDD->CreateSurface(&ddsd,&m_pSurface,0);
		}
		if(m_pSurface)
		{
			return true;
		}
	}

	return false;
}

void BinkSurface::Update(void)
{
	if(m_hBink)
	{
		if(!BinkWait(m_hBink))
		{
			BinkDoFrame(m_hBink);
			BinkNextFrame(m_hBink);
			Unpack();
		}
	}
}

void BinkSurface::Unpack(void)
{
	static DDSURFACEDESC2 ddsd;

	ddsd.dwSize = sizeof(DDSURFACEDESC2);


//Grom был тута
	//while(m_pSurface->Lock(0,&ddsd,0,0) == DDERR_WASSTILLDRAWING);
		HRESULT hResult;
	while( (hResult=m_pSurface->Lock(0,&ddsd,0,0)) == DDERR_WASSTILLDRAWING);
		if(FAILED(hResult))
		{//	не удалось залочить - выходим
			return ;
		}
//дальше меня не было



	BinkCopyToBuffer(m_hBink,
					 ddsd.lpSurface,
					 ddsd.lPitch,
					 ddsd.dwHeight,
					 0,0,
                     BinkDDSurfaceType(m_pSurface));

	m_pSurface->Unlock(0);
}
