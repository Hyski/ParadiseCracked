/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   23.04.2000

************************************************************************/
#include "Precomp.h"
#include "ScreenShot.h"

bool ScreenShot(LPDIRECTDRAWSURFACE7 lpSurface,const char *pFileName)
{
	BITMAPFILEHEADER bmfh = {0x4d42,0,0,0,sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)};
	BITMAPINFOHEADER bmih = {sizeof(BITMAPINFOHEADER),0,0,1,24,0,0,0,0,0,0};
	DDSURFACEDESC2 ddsd;
	DWORD dwImageSize;
	DWORD dwStride;
	BYTE *lpImage = 0;
	DWORD mask;
	BYTE cRShiftRight = 0; 
	BYTE cGShiftRight = 0; 
	BYTE cBShiftRight = 0; 
	BYTE cRShiftLeft = 8; 
	BYTE cGShiftLeft = 8; 
	BYTE cBShiftLeft = 8; 
	FILE *stream;

	if((stream = fopen(pFileName,"wb"))!=0)
	{
		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize		= sizeof(DDSURFACEDESC2);
		ddsd.dwFlags	= DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_LPSURFACE|DDSD_PITCH;
		//	блокируем поверхность
	    while(lpSurface->Lock(NULL,&ddsd,DDLOCK_WAIT|DDLOCK_READONLY,0) == DDERR_WASSTILLDRAWING);
		//	ширина и высота
		bmih.biWidth = ddsd.dwWidth;
		bmih.biHeight = ddsd.dwHeight;
		//	подсчет битов на каждый цвет в текстуре
		for(mask = ddsd.ddpfPixelFormat.dwRBitMask;mask && !(mask&1);mask>>=1)
			cRShiftRight ++;
		for(;mask;mask>>=1)
			cRShiftLeft--;
		for(mask = ddsd.ddpfPixelFormat.dwGBitMask;mask && !(mask&1);mask>>=1)
			cGShiftRight ++;
		for(;mask;mask>>=1)
			cGShiftLeft--;
		for(mask = ddsd.ddpfPixelFormat.dwBBitMask;mask && !(mask&1);mask>>=1)
			cBShiftRight ++;
		for(;mask;mask>>=1)
			cBShiftLeft--;


		//	подсчет длины битового образа
		dwStride = ((DWORD)bmih.biBitCount*bmih.biWidth)/32;
		if(((DWORD)bmih.biWidth * bmih.biBitCount) % 32)
		{
			dwStride++;
		}
		dwStride *= 4;
		dwImageSize = dwStride*bmih.biHeight;
		//	подсчет длины выходного файла
		bmfh.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwImageSize;
		//	выделение памяти под битовый образ
		if((lpImage = new BYTE[dwImageSize]))
		{
			BYTE* pBytes = (BYTE*)ddsd.lpSurface;
			BYTE* pDstImage = lpImage;
			//	копируем цвет из поверхности во временные переменные
			for(DWORD y=0;y<bmih.biHeight;y++)
			{
					for(DWORD x=0;x<bmih.biWidth;x++)
					{
						DWORD dwPixel = 0;

						switch(ddsd.ddpfPixelFormat.dwRGBBitCount)
						{
						case 16:
							dwPixel = *((WORD*)(pBytes+(y*ddsd.lPitch))+x);
							break;
						case 24:
							dwPixel = ((DWORD)*((BYTE*)(pBytes+(y*ddsd.lPitch))+x*3));
							dwPixel += ((DWORD)*((BYTE*)(pBytes+(y*ddsd.lPitch))+x*3+1))<<8;
							dwPixel += ((DWORD)*((BYTE*)(pBytes+(y*ddsd.lPitch))+x*3+2))<<16;
							break;
						case 32:
							dwPixel = *((DWORD*)(pBytes+(y*ddsd.lPitch))+x);
							break;
						}
						//	извлекаем текущий пиксел
						BYTE r = (BYTE)((((dwPixel&ddsd.ddpfPixelFormat.dwRBitMask)>>cRShiftRight)<<cRShiftLeft));
						BYTE g = (BYTE)((((dwPixel&ddsd.ddpfPixelFormat.dwGBitMask)>>cGShiftRight)<<cGShiftLeft));
						BYTE b = (BYTE)((((dwPixel&ddsd.ddpfPixelFormat.dwBBitMask)>>cBShiftRight)<<cBShiftLeft));

						//	заносим компоненты цвета в выводимый рисунок
						pDstImage[((bmih.biHeight-y-1)*dwStride)+(x*3)]   = b;
						pDstImage[((bmih.biHeight-y-1)*dwStride)+(x*3)+1] = g;
						pDstImage[((bmih.biHeight-y-1)*dwStride)+(x*3)+2] = r;
					}
				}
			//	записываем информацию в файл
			fwrite(&bmfh,sizeof(BITMAPFILEHEADER),1,stream);
			fwrite(&bmih,sizeof(BITMAPINFOHEADER),1,stream);
			fwrite(lpImage,dwImageSize,1,stream);

			delete[]  lpImage;
		}

		//	разблокируем поверхность
		lpSurface->Unlock(0);
		//  закрываем файл
		fclose(stream);
	}

	return false;
}

//*******************************************************************************************//
#include "../Common/D3DApp/GammaControl/GammaControl.h"

bool GammaScreenShot(LPDIRECTDRAWSURFACE7 lpSurface,const char *pFileName)
{
	BITMAPFILEHEADER bmfh = {0x4d42,0,0,0,sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)};
	BITMAPINFOHEADER bmih = {sizeof(BITMAPINFOHEADER),0,0,1,24,0,0,0,0,0,0};
	DDSURFACEDESC2 ddsd;
	DWORD dwImageSize;
	DWORD dwStride;
	BYTE *lpImage = 0;
	DWORD mask;
	BYTE cRShiftRight = 0; 
	BYTE cGShiftRight = 0; 
	BYTE cBShiftRight = 0; 
	BYTE cRShiftLeft = 8; 
	BYTE cGShiftLeft = 8; 
	BYTE cBShiftLeft = 8; 
	FILE *stream;

	if((stream = fopen(pFileName,"wb"))!=0)
	{
		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize		= sizeof(DDSURFACEDESC2);
		ddsd.dwFlags	= DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_LPSURFACE|DDSD_PITCH;
		//	блокируем поверхность
	    while(lpSurface->Lock(NULL,&ddsd,DDLOCK_WAIT|DDLOCK_READONLY,0) == DDERR_WASSTILLDRAWING);
		//	ширина и высота
		bmih.biWidth = ddsd.dwWidth;
		bmih.biHeight = ddsd.dwHeight;
		//	подсчет битов на каждый цвет в текстуре
		for(mask = ddsd.ddpfPixelFormat.dwRBitMask;mask && !(mask&1);mask>>=1)
			cRShiftRight ++;
		for(;mask;mask>>=1)
			cRShiftLeft--;
		for(mask = ddsd.ddpfPixelFormat.dwGBitMask;mask && !(mask&1);mask>>=1)
			cGShiftRight ++;
		for(;mask;mask>>=1)
			cGShiftLeft--;
		for(mask = ddsd.ddpfPixelFormat.dwBBitMask;mask && !(mask&1);mask>>=1)
			cBShiftRight ++;
		for(;mask;mask>>=1)
			cBShiftLeft--;
		
		//	извлекаем гамму
		DDGAMMARAMP m_GammaRamp = GammaControl::GammaRamp();
	
		//	подсчет длины битового образа
		dwStride = ((DWORD)bmih.biBitCount*bmih.biWidth)/32;
		if(((DWORD)bmih.biWidth * bmih.biBitCount) % 32)
		{
			dwStride++;
		}
		dwStride *= 4;
		dwImageSize = dwStride*bmih.biHeight;
		//	подсчет длины выходного файла
		bmfh.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwImageSize;
		//	выделение памяти под битовый образ
		if((lpImage = new BYTE[dwImageSize]))
		{
			BYTE* pBytes = (BYTE*)ddsd.lpSurface;
			BYTE* pDstImage = lpImage;
			//	копируем цвет из поверхности во временные переменные
			for(DWORD y=0;y<bmih.biHeight;y++)
			{
					for(DWORD x=0;x<bmih.biWidth;x++)
					{
						DWORD dwPixel = 0;

						switch(ddsd.ddpfPixelFormat.dwRGBBitCount)
						{
						case 16:
							dwPixel = *((WORD*)(pBytes+(y*ddsd.lPitch))+x);
							break;
						case 24:
							dwPixel = ((DWORD)*((BYTE*)(pBytes+(y*ddsd.lPitch))+x*3));
							dwPixel += ((DWORD)*((BYTE*)(pBytes+(y*ddsd.lPitch))+x*3+1))<<8;
							dwPixel += ((DWORD)*((BYTE*)(pBytes+(y*ddsd.lPitch))+x*3+2))<<16;
							break;
						case 32:
							dwPixel = *((DWORD*)(pBytes+(y*ddsd.lPitch))+x);
							break;
						}
						//	извлекаем текущий пиксел
						BYTE r = (BYTE)((((dwPixel&ddsd.ddpfPixelFormat.dwRBitMask)>>cRShiftRight)<<cRShiftLeft));
						BYTE g = (BYTE)((((dwPixel&ddsd.ddpfPixelFormat.dwGBitMask)>>cGShiftRight)<<cGShiftLeft));
						BYTE b = (BYTE)((((dwPixel&ddsd.ddpfPixelFormat.dwBBitMask)>>cBShiftRight)<<cBShiftLeft));
						
						//	используем преобразование гаммы
						r = m_GammaRamp.red[r]   >> 8;
						g = m_GammaRamp.green[g] >> 8;
						b = m_GammaRamp.blue[b]  >> 8;

						//	заносим компоненты цвета в выводимый рисунок
						pDstImage[((bmih.biHeight-y-1)*dwStride)+(x*3)]   = b;
						pDstImage[((bmih.biHeight-y-1)*dwStride)+(x*3)+1] = g;
						pDstImage[((bmih.biHeight-y-1)*dwStride)+(x*3)+2] = r;
					}
				}
			//	записываем информацию в файл
			fwrite(&bmfh,sizeof(BITMAPFILEHEADER),1,stream);
			fwrite(&bmih,sizeof(BITMAPINFOHEADER),1,stream);
			fwrite(lpImage,dwImageSize,1,stream);

			delete[]  lpImage;
		}

		//	разблокируем поверхность
		lpSurface->Unlock(0);
		//  закрываем файл
		fclose(stream);
	}

	return false;
}
