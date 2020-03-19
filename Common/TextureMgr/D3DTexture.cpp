/***********************************************************************

                           Texture Manager Test

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.05.2001

************************************************************************/
#include "Precomp.h"
#include "../Image/Image.h"
#include "../Image/ImageFactory.h"
#include "DIBData.h"
#include "TextureFormatMgr.h"
#include "D3DTexture.h"

//---------- Лог файл ------------
#ifdef _HOME_VERSION
CLog textures_log;
#define textures	textures_log["textures.log"]
#else
#define textures	/##/
#endif
//--------------------------------
D3DTexture::Copy D3DTexture::m_Copy = D3DTexture::CopyImageToSurface;
D3DTexture::D3DTexture() : m_Surface(0),m_Width(0),m_Height(0)
{
}

D3DTexture::~D3DTexture()
{
	if(m_Surface) Release();
}

D3DTexture::D3DTexture(LPDIRECT3DDEVICE7 lpd3dDevice,
					   DIBData* pImage,			
					   int iMipMapLevel,				
					   unsigned int iQuality,			
					   bool bCompressed,			
					   bool bAlphaPremultiplied) : m_Surface(0),m_Width(0),m_Height(0)
{
	Create(lpd3dDevice,pImage,iMipMapLevel,iQuality,bCompressed,bAlphaPremultiplied);
}

D3DTexture::D3DTexture(LPDIRECT3DDEVICE7 lpd3dDevice,
					   Image* pImage,			
					   int iMipMapLevel,				
					   unsigned int iQuality,			
					   bool bCompressed,			
					   bool bAlphaPremultiplied) : m_Surface(0),m_Width(0),m_Height(0)
{
	Create(lpd3dDevice,pImage,iMipMapLevel,iQuality,bCompressed,bAlphaPremultiplied);
}

bool D3DTexture::Create(LPDIRECT3DDEVICE7 lpd3dDevice,
						DIBData* pImage,			
						int iMipMapLevel,
						unsigned int iQuality,
						bool bCompressed,
						bool bAlphaPremultiplied)
{
	if(lpd3dDevice && pImage && pImage->IsValid())
	{
		Image image;

		image.Create(pImage->Width(),pImage->Height(),&Image::ic_PixelFormat(pImage->Bpp()),0,pImage->Image());
		return Create(lpd3dDevice,&image,iMipMapLevel,iQuality,bCompressed,bAlphaPremultiplied);
	}

	return false;
}

bool D3DTexture::Create(LPDIRECT3DDEVICE7 lpd3dDevice,
						Image* pImage,			
						int iMipMapLevel,
						unsigned int iQuality,
						bool bCompressed,
						bool bAlphaPremultiplied)
{
	DDSURFACEDESC2 ddsd;	//	свойства текстуры

	textures("creating texture [%x]:>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>......\n",this);
	//	если поверхность уже существует - освобождаем связанные с ней ресурсы
	if(m_Surface) Release();
	//	подготавливаем структуру к заполнению
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	//	проверяем на действительность данные
	if(lpd3dDevice && pImage && pImage->BitImage())
	{
		//	подбираем пиксель формат
		if(ChoosePixelFormat(pImage,&ddsd.ddpfPixelFormat,bCompressed,bAlphaPremultiplied))
		{
			bCompressed = static_cast<bool>(ddsd.ddpfPixelFormat.dwFourCC);
			//	если требуется, уменьшаем качество картинки
			CheckQuality(pImage,iQuality);
			//	определяем базовые свойства текстуры
			if(PrepareSurfaceProperties(lpd3dDevice,&ddsd))
			{
				//	если требуется сжатие выравниваем картинку
				if(bCompressed) AlignSizeForCompressing();
				//	подсчитываем получаемое в итоге кол-во мипмэпов
				CalculateMipMapLevel(pImage,&ddsd,iMipMapLevel);
				//	создаем поверхность 
				if(MakeSurface(lpd3dDevice,&ddsd))
				{
					//	копируем изображение в поверхность и создаем мипмэпы
					if(FlushImageToSurface(lpd3dDevice,&ddsd,pImage))
					{					
						textures("     SUCCESS;\n");

						return true;
					}
				}
			}
		}
	}

	if(m_Surface) Release();

	textures("     FAILED;\n");

	return false;
}
//**********************************************************************************//
//	подбираем пиксель формат исходя из параметров
bool D3DTexture::ChoosePixelFormat(Image* pImage,DDPIXELFORMAT* pDDPF,bool bCompressed,bool bAlphaPremultiplied)
{
	pDDPF->dwFourCC = 0;
	pDDPF->dwRGBBitCount = pImage->PixelFormat()->m_BitPerPixel;
	if(pImage->ColorMap())
	{
		switch(pImage->PixelFormat()->m_BitPerPixel)
		{
		case 1:
			pDDPF->dwFlags |= DDPF_PALETTEINDEXED1;
			break;
		case 2:
			pDDPF->dwFlags |= DDPF_PALETTEINDEXED2;
			break;
		case 4:
			pDDPF->dwFlags |= DDPF_PALETTEINDEXED4;
			break;
		case 8:
			pDDPF->dwFlags |= DDPF_PALETTEINDEXED8;
			break;
		}
	}
	else
	{
		pDDPF->dwRGBAlphaBitMask = pImage->PixelFormat()->m_ABitMask;
		pDDPF->dwRBitMask        = pImage->PixelFormat()->m_RBitMask;
		pDDPF->dwGBitMask        = pImage->PixelFormat()->m_GBitMask;
		pDDPF->dwBBitMask        = pImage->PixelFormat()->m_BBitMask;
	}
	if(bCompressed)
	{
		switch(pImage->PixelFormat()->m_BitPerPixel)
		{
		case 16:
			pDDPF->dwFourCC = MAKEFOURCC('D','X','T','1');
			break;
		default:
			pDDPF->dwFourCC = bAlphaPremultiplied?MAKEFOURCC('D','X','T','4'):MAKEFOURCC('D','X','T','5');
			break;
		}
	}

	return TextureFormatMgr::Instance()->ChoosePixelFormat(pDDPF);
}

void D3DTexture::CheckQuality(Image* pImage,unsigned int iQuality)
{
	textures("     check quality:\n");
	textures("	   quality level - %d;\n",iQuality);
	m_Width = pImage->Width();
	m_Height = pImage->Height();
	textures("	   inner size - [%d:%d];\n",m_Width,m_Height);
	for(int i=0;i<iQuality;i++)
	{
		if((pImage->Width()>1) && (pImage->Height()>1))
		{
			pImage->Quad();
		}
	}
	m_Width = pImage->Width();
	m_Height = pImage->Height();
	textures("	   outer size - [%d:%d];\n",m_Width,m_Height);
}

void D3DTexture::AlignSizeForCompressing(void)
{
	textures("align size for compressing:\n    inner size [%d,%d]\n",m_Width,m_Height);
	//	надо размеры привести к кратным четырем
	if(m_Width%4) m_Width += 4-(m_Width%4);
	if(m_Height%4) m_Height += 4-(m_Height%4);
	textures("    ...multipled for 4 pixels: [%d]:[%d]\n",m_Width,m_Height);
}

bool D3DTexture::PrepareSurfaceProperties(LPDIRECT3DDEVICE7 lpd3dDevice,LPDDSURFACEDESC2 lpDDSD)
{
    D3DDEVICEDESC7 ddDesc;

	textures("prepare surface properties:\n");
    if(FAILED(lpd3dDevice->GetCaps(&ddDesc)))
        return false;
	//	обрезаем размер текстуры до максимально возможного
	m_Width = std::min(static_cast<DWORD>(m_Width),ddDesc.dwMaxTextureWidth);
	m_Height = std::min(static_cast<DWORD>(m_Height),ddDesc.dwMaxTextureHeight);
	textures("    ...device max texture size: [%d]:[%d]\n",ddDesc.dwMaxTextureWidth,ddDesc.dwMaxTextureHeight);
	textures("    ...align texture size for max available: [%d]:[%d]\n",m_Width,m_Height);
	//	заполняем структурку свойствами
    lpDDSD->dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|
							  DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;
    lpDDSD->ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
    lpDDSD->dwWidth         = m_Width;
    lpDDSD->dwHeight        = m_Height;
    // Turn on D3DTexture management for hardware devices
    if(ddDesc.deviceGUID == IID_IDirect3DHALDevice )
	{
        lpDDSD->ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		textures("    ...set DDSCAPS2_TEXTUREMANAGE flag;\n");
	}
    else
	{
		if(ddDesc.deviceGUID == IID_IDirect3DTnLHalDevice )
		{
			lpDDSD->ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
			textures("    ...set DDSCAPS2_TEXTUREMANAGE flag;\n");
		}
		else
		{
			lpDDSD->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
			textures("    ...set DDSCAPS_SYSTEMMEMORY flag;\n");
		}
	}
	//	если требуется использование текстур размеры которых кратны 2
    if(ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2)
    {
		textures("    ...D3DPTEXTURECAPS_POW2 availble;\n");
        for(lpDDSD->dwWidth=1;m_Width>lpDDSD->dwWidth;lpDDSD->dwWidth<<=1);
        for(lpDDSD->dwHeight=1;m_Height>lpDDSD->dwHeight;lpDDSD->dwHeight<<=1);
		textures("    ...after align: [%d]:[%d]\n",lpDDSD->dwWidth,lpDDSD->dwHeight);
		//	обрезаем размер текстуры до максимально возможного
		lpDDSD->dwWidth = std::min(lpDDSD->dwWidth,ddDesc.dwMaxTextureWidth);
		lpDDSD->dwHeight = std::min(lpDDSD->dwHeight,ddDesc.dwMaxTextureHeight);
		textures("    ...align texture size for max available: [%d]:[%d]\n",lpDDSD->dwWidth,lpDDSD->dwHeight);
    }
	//	если требуется использование текстур ширина равна высоте
    if(ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
    {
		textures("    ...D3DPTEXTURECAPS_SQUAREONLY availble;\n");
        if(lpDDSD->dwWidth > lpDDSD->dwHeight)
		{
			lpDDSD->dwHeight = lpDDSD->dwWidth;
		}
        else
		{
			lpDDSD->dwWidth  = lpDDSD->dwHeight;
		}
		textures("    ...after align: [%d]:[%d]\n",lpDDSD->dwWidth,lpDDSD->dwHeight);
    }
	//	запоминаем новый размер текстуры
	m_Width = lpDDSD->dwWidth;
	m_Height = lpDDSD->dwHeight;
	
	return true;
}

void D3DTexture::CalculateMipMapLevel(Image* /*pImage*/,LPDDSURFACEDESC2 lpDDSD,int iMipMapLevel)
{
	int media;
	int level;

	lpDDSD->dwWidth = m_Width;
	lpDDSD->dwHeight = m_Height;
	textures("CALCULATE MIPMAP LEVEL:\n     innner mipmap level - [%d]\n",iMipMapLevel);
	//	что меньше высота или ширина
	media = std::min(m_Width,m_Height);
	for(level = 0;(1<<level)<media;level++);
	textures("     find mipmap level: [%d];\n",level);
	if(iMipMapLevel != -1) level = std::max(0,std::min(level,iMipMapLevel));
	textures("     FINAL mipmap level: [%d];\n",level);
	if(level)
	{
		lpDDSD->dwFlags |= DDSD_MIPMAPCOUNT;
	    lpDDSD->ddsCaps.dwCaps  |= DDSCAPS_MIPMAP|DDSCAPS_COMPLEX;
	    lpDDSD->dwMipMapCount   = level;
	}
}

bool D3DTexture::MakeSurface(LPDIRECT3DDEVICE7 lpd3dDevice,LPDDSURFACEDESC2 lpDDSD)
{
    LPDIRECTDRAWSURFACE7 lpddsRender;
    LPDIRECTDRAW7        lpDD;

	textures("    <-- create new surface -->\n");
	//	создание поверхности
    lpd3dDevice->GetRenderTarget(&lpddsRender);
    lpddsRender->GetDDInterface((VOID**)&lpDD);
    lpddsRender->Release();
	//	Создаем новую поверхность для текстуры
    if(FAILED(lpDD->CreateSurface(lpDDSD,&m_Surface,NULL)))
    {
        lpDD->Release();
		textures("    ...can't create new surface\n");
        return false;
    }
	textures("    ...new surface created;\n");
    // Done with DDraw
    lpDD->Release();

	return true;
}

bool D3DTexture::FlushImageToSurface(LPDIRECT3DDEVICE7 lpd3dDevice,LPDDSURFACEDESC2 lpDDSD,Image *pImage)
{
    LPDIRECTDRAWSURFACE7 lpddsRender;
    LPDIRECTDRAW7        lpDD;
	LPDIRECTDRAWSURFACE7 lpddsDest = m_Surface;
	DDSURFACEDESC2		 ddsd;					//	используется для сжатых поверхностей

	//	получаем указатель на объект DirectDraw
    lpd3dDevice->GetRenderTarget(&lpddsRender);
    lpddsRender->GetDDInterface((VOID**)&lpDD);
    lpddsRender->Release();
	
	//	опредеяем тип используемой функции Copy
	m_Copy = (pImage->PixelFormat()->m_BitPerPixel == 32)?UseBlitting:UseStretching;

	//	если требуется создать сжатые текстуры
	if(lpDDSD->ddpfPixelFormat.dwFourCC)
	{
		//	подбираем подходящий пиксель формат для не сжатой текстуры
		memcpy(&ddsd,lpDDSD,sizeof(DDSURFACEDESC2));
		textures("    ...using compressed textures;\n");
		if(!ChoosePixelFormat(pImage,&ddsd.ddpfPixelFormat,false,false))
		{
			textures("    ...can find suitable surface for using compressed textures;\n");
			lpDD->Release();
			return false;
		}
		//	убираем лишние флаги
		ddsd.dwFlags        &= ~DDSD_MIPMAPCOUNT;
	    ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_MIPMAP|DDSCAPS_COMPLEX);
	    ddsd.dwMipMapCount   = 0;
	}

	//	создаем мипмэпы
	int level = lpDDSD->dwMipMapCount?lpDDSD->dwMipMapCount:1;
	for(int i=0;i<level;i++)
	{
		textures("IMAGE SIZE: %dx%d\n",lpDDSD->dwWidth,lpDDSD->dwHeight);

		if(lpDDSD->ddpfPixelFormat.dwFourCC)
		{
			ddsd.dwWidth = lpDDSD->dwWidth;
			ddsd.dwHeight = lpDDSD->dwHeight;
			if(!UseCompressing(lpDD,&ddsd,pImage,lpddsDest,lpDDSD->dwWidth,lpDDSD->dwHeight))
			{
				RELEASE(m_Surface);
				lpDD->Release();
				return false;
			}
		}
		else
		{
			if(!m_Copy(lpDD,lpDDSD,pImage,lpddsDest))
			{
				RELEASE(m_Surface);
				lpDD->Release();
				return false;
			}

		}
		
		if(i == level-1) break;

		//	!!! а вдруг размеры нечетные???

		lpDDSD->dwWidth >>= 1;
		lpDDSD->dwHeight >>= 1;

		pImage->Quad();
		
        DDSCAPS2 ddsCaps;
        ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
        ddsCaps.dwCaps2 = 0;
        ddsCaps.dwCaps3 = 0;
        ddsCaps.dwCaps4 = 0;
        if(SUCCEEDED(lpddsDest->GetAttachedSurface(&ddsCaps,&lpddsDest)))
            lpddsDest->Release();
	}
	
    // Done with DDraw
    lpDD->Release();
	
	return true;
}

bool D3DTexture::UseCompressing(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface,DWORD /*dwDstWidth*/,DWORD /*dwDstHeight*/)
{
	LPDIRECTDRAWSURFACE7 lpTempSurface;

	// Create a new surface for the D3DTexture
	if(!FAILED(lpDD->CreateSurface(lpDDSD,&lpTempSurface,NULL)))
	{
		if(!m_Copy(lpDD,lpDDSD,pImage,lpTempSurface))
		{
			lpTempSurface->Release();
			return false;
		}
		//	используем Flip
		pSurface->BltFast(0,0,lpTempSurface,0,DDBLTFAST_WAIT);
		lpTempSurface->Release();

		return true;
	}

	return false;
}

bool D3DTexture::UseStretching(LPDIRECTDRAW7 /*lpDD*/,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface)
{
	HDC hSurfaceDC;

    // Get a DC for the surface
    if(FAILED(pSurface->GetDC(&hSurfaceDC)))
    {
		textures("- FAILED: Can't get DC from lpSurface;\n");
		return FALSE;
	}
	//	копируем картинку
	textures("Attached D3DTexture: %dx%d\n",lpDDSD->dwWidth,lpDDSD->dwHeight);
	ImageFactory::StretchImage(hSurfaceDC,pImage,
							   0,0,lpDDSD->dwWidth,lpDDSD->dwHeight,
							   0,0,pImage->Width(),pImage->Height());
	pSurface->ReleaseDC(hSurfaceDC);

	return true;
}

void D3DTexture::Release(void)
{
	if(m_Surface)
	{
		m_Surface->Release();
		m_Surface = 0;
		m_Width = m_Height = 0;
		textures("[%x] released;\n",this);
	}
}

bool D3DTexture::CopyImageToSurface(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface)
{
	if(!UseBlitting(lpDD,lpDDSD,pImage,pSurface))
	{//	Using Stretch
		textures(" - UseBlitting FAILED\n");
		if(!UseStretching(lpDD,lpDDSD,pImage,pSurface))
		{
			textures("- FAILED: Can't Stretch Bitmap To Surface;\n");
			return false;
		}
	}

	return true;
}

bool D3DTexture::UseBlitting(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface)
{
	DDSURFACEDESC2 ddsd;
	LPDIRECTDRAWSURFACE7 lpTempSurface;

	if((pImage->PixelFormat()->m_BitPerPixel == 32) && 
	   (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS))
	{
		textures("-- UseBlitting;\n");
		// Setup the new surface desc
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		pSurface->GetSurfaceDesc(&ddsd);
		ddsd.dwFlags	     = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;
		ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE|DDSCAPS_SYSTEMMEMORY;
		ddsd.ddsCaps.dwCaps2 = 0L;
		ddsd.dwWidth         = pImage->Width();
		ddsd.dwHeight        = pImage->Height();
		// Create a new surface for the D3DTexture
		if(FAILED(lpDD->CreateSurface(&ddsd,&lpTempSurface,NULL)))
			return false;
		//	лочим временную текстуру
		HRESULT hResult;
	    while((hResult = lpTempSurface->Lock(NULL,&ddsd,0,0)) == DDERR_WASSTILLDRAWING);
		if(FAILED(hResult))
		{//	не удалось залочить - выходим
			textures("!!! LOCK FAILED !!!");
			lpTempSurface->Release();
			return false;
		}

		BYTE* pBytes = (BYTE*)ddsd.lpSurface;
		DWORD* lpImage = (DWORD *)pImage->BitImage();
		
		DWORD dwRMask = ddsd.ddpfPixelFormat.dwRBitMask;
		DWORD dwGMask = ddsd.ddpfPixelFormat.dwGBitMask;
		DWORD dwBMask = ddsd.ddpfPixelFormat.dwBBitMask;
		DWORD dwAMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
		
		DWORD dwRShiftL = 8, dwRShiftR = 0;
		DWORD dwGShiftL = 8, dwGShiftR = 0;
		DWORD dwBShiftL = 8, dwBShiftR = 0;
		DWORD dwAShiftL = 8, dwAShiftR = 0;
		
		DWORD dwMask;
		for(dwMask=dwRMask;dwMask && !(dwMask&0x1);dwMask>>=1) dwRShiftR++;
		for(;dwMask;dwMask>>=1) dwRShiftL--;
		
		for(dwMask=dwGMask;dwMask && !(dwMask&0x1);dwMask>>=1) dwGShiftR++;
		for(;dwMask;dwMask>>=1) dwGShiftL--;
		
		for(dwMask=dwBMask;dwMask && !(dwMask&0x1);dwMask>>=1) dwBShiftR++;
		for(;dwMask;dwMask>>=1) dwBShiftL--;
		
		for(dwMask=dwAMask;dwMask && !(dwMask&0x1);dwMask>>=1) dwAShiftR++;
		for(;dwMask;dwMask>>=1) dwAShiftL--;
		
		for(int y=(ddsd.dwHeight-1);y>=0;y--)
		{
			DWORD* pDstData32 = (DWORD*)pBytes;
			WORD*  pDstData16 = (WORD*)pBytes;

			for(DWORD x=0;x<ddsd.dwWidth;x++)
			{
				DWORD dwPixel = lpImage[y*ddsd.dwWidth+x];
				
				BYTE r = (BYTE)((dwPixel>>16)&0x000000ff);
				BYTE g = (BYTE)((dwPixel>> 8)&0x000000ff);
				BYTE b = (BYTE)((dwPixel>> 0)&0x000000ff);
				BYTE a = (BYTE)((dwPixel>>24)&0x000000ff);
				
				DWORD dr = ((r>>(dwRShiftL))<<dwRShiftR)&dwRMask;
				DWORD dg = ((g>>(dwGShiftL))<<dwGShiftR)&dwGMask;
				DWORD db = ((b>>(dwBShiftL))<<dwBShiftR)&dwBMask;
				DWORD da = ((a>>(dwAShiftL))<<dwAShiftR)&dwAMask;
				
				if( 32 == ddsd.ddpfPixelFormat.dwRGBBitCount )
					pDstData32[x] = (DWORD)(dr+dg+db+da);
				else
					pDstData16[x] = (WORD)(dr+dg+db+da);
			}
			
			pBytes += ddsd.lPitch;
		}

	    lpTempSurface->Unlock(0);

		// Copy the temp surface to the real Texture surface
	    pSurface->Blt(NULL,lpTempSurface,NULL,DDBLT_WAIT,NULL);
	    lpTempSurface->Release();

		return true;
	}

	return false;
}

