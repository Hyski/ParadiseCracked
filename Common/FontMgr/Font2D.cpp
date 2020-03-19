/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   06.03.2001

************************************************************************/
#include "Precomp.h"
#include "../DataMgr/DataMgr.h"
#include "LogicalText.h"
#include "mlf.h"
#include "Font2D.h"

//---------- Лог файл ------------
#ifdef _ENABLE_LOG
CLog font2d_log;
#define font2d	font2d_log["font2d.log"]
#else
#define font2d	/##/
#endif
//--------------------------------

Font2D::Font2D() : Font()
{
	m_pSurface = 0;
}

Font2D::Font2D(LPDIRECTDRAWSURFACE7 pPrimarySurface,const char *pFileName,int color)
{
	m_pSurface = 0;
	Create(pPrimarySurface,pFileName,color);
}

Font2D::~Font2D()
{
	Release();
}

bool Font2D::Create(LPDIRECTDRAWSURFACE7 pPrimarySurface,const char *pFileName,int color)
{
	VFile *pVFile = DataMgr::Load(pFileName);
	MLF_FILEHEADER fh;
	MLF_DATAHEADER dh;

	if(m_pSurface) Release();

	font2d(" --- TRY CREATE FONT 2D --- \n");
	font2d("    ...from file: [%s];\n",pFileName);
	font2d("    ...color: [0x%0.8x];\n",color);

	if(pVFile->Size())
	{
		//	на начало файла
		pVFile->Rewind();
		//	считываем заголовок файла
		if(pVFile->Read(&fh,sizeof(MLF_FILEHEADER)))
		{
			//	перемещаем указатель на начало заголовка данных
			pVFile->Seek(fh.lSize,VFile::VFILE_SET);
			//	считываем заголовок данных
			if(pVFile->Read(&dh,sizeof(MLF_DATAHEADER)))
			{
				//	перемещаем указатель на начало битового образа
				pVFile->Seek(dh.lSize+fh.lSize,VFile::VFILE_SET);
				//	заполняем общие данные
				m_iFontHeight = dh.lFontHeight;
				memcpy(m_rcChar,dh.rcChar,sizeof(m_rcChar));
				memcpy(m_abcChar,dh.abcChar,sizeof(m_abcChar));
				//	проверяем правильность файла
				if((pVFile->Size()-pVFile->Tell()) == dh.lWidth*dh.lHeight)
				{
					font2d("    ...headers reading complete;\n");
					//	создаем поверхность и копируем туда битмапку
					CreateSurface(pPrimarySurface,dh.lWidth,dh.lHeight,pVFile,color);
				}
			}
		}
	}

	//	освобождаем данные
	DataMgr::Release(pFileName);

	font2d("> %s\n",m_pSurface?"SUCCESS":"FAILED");
	return (bool)m_pSurface;
}

void Font2D::Release(void)
{
	font2d(" --- RELEASE FONT 2D> --- \n");
	if(m_pSurface)
	{
		m_pSurface->Release();
		m_pSurface = 0;
	}
}

void Font2D::CreateSurface(LPDIRECTDRAWSURFACE7 pPrimarySurface,int width,int height,VFile *pVFile,int color)
{
	DDSURFACEDESC2 ddsd;
	LPDIRECTDRAW7 pDD;

	if(pPrimarySurface && !FAILED(pPrimarySurface->GetDDInterface((void **)&pDD)))
	{
		//	создание поверхности
		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags	     = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps  = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
		ddsd.dwWidth         = width;
		ddsd.dwHeight        = height;
		ddsd.ddpfPixelFormat.dwSize  = sizeof(DDPIXELFORMAT);
		//	возьмем формат пикселов из первичной поверхности
		pPrimarySurface->GetPixelFormat(&ddsd.ddpfPixelFormat);

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
		
		font2d("    ...try create offscreen surface;\n");
		if(!FAILED(pDD->CreateSurface(&ddsd,&m_pSurface,0)))
		{
			font2d("    ...try lock the surface;\n");
			font2d("    ...color: [0x%0.8x];\n",color);
			//	заполняем массив данными из битового образа шрифта
		    while(m_pSurface->Lock(0,&ddsd,0,0) == DDERR_WASSTILLDRAWING);
			if(ddsd.lpSurface)
			{
				for(int i=0;i<height;i++)
				{
					BYTE *pImage = (BYTE *)ddsd.lpSurface+(i*ddsd.lPitch);
					for(int j=0;j<width;j++)
					{
						DWORD r,g,b;

						r = g = b = pVFile->ReadChar();

						r = (r*((color>>16)&0xff))/0xff;
						g = (g*((color>>8)&0xff))/0xff;
						b = (b*((color&0xff)))/0xff;
						
						switch(ddsd.ddpfPixelFormat.dwRGBBitCount)
						{
						case 32:
							((DWORD *)pImage)[j] = b+(g<<8)+(r<<16);
							break;
						case 24:
							pImage[(j*3)+2] = r;
							pImage[(j*3)+1] = g;
							pImage[(j*3)+0] = b;
							break;
						case 16:
							((WORD *)pImage)[j] = 
							(((r>>dwRShiftL)<<dwRShiftR)&dwRMask) +
							(((g>>dwGShiftL)<<dwGShiftR)&dwGMask) +
							(((b>>dwBShiftL)<<dwBShiftR)&dwBMask);
							break;
						}
					}
				}
			}

			m_pSurface->Unlock(0);

			DDCOLORKEY ddcolorkey;
			ddcolorkey.dwColorSpaceLowValue = 0;
			ddcolorkey.dwColorSpaceHighValue = ddcolorkey.dwColorSpaceLowValue;
			m_pSurface->SetColorKey(DDCKEY_SRCBLT,&ddcolorkey);
		}
		//	сделано с помощью DirectDraw
		pDD->Release();
		font2d("    ...DirectDraw object released;\n");
	}
}

void Font2D::TextOut(LPDIRECTDRAWSURFACE7 pSurface,int y,int width_limit,int height_limit,LogicalText* pLT)
{
	int local_x = 0;
	int local_y = y+pLT->TopOffset();

/*	font2d("\n --- TextOut --- \n");
	font2d("    ...height == %d\n",height_limit);
	font2d("    ...width == %d\n",width_limit);
	font2d("    ...m_iFontHeight == %d\n",m_iFontHeight);
	font2d("    ...m_pfText->TopOffset() == %d\n",pLT->TopOffset());*/

	for(int i=0;((i<pLT->Text().size()) && (local_y<height_limit));i++)
	{//	бежим построкам
		if((local_y+m_iFontHeight)>0)
		{
			local_x = pLT->Text()[i].m_Offset;
			for(int j=0;j<pLT->Text()[i].m_vWords.size();j++)
			{//	бежим по словам
				for(int k=0;k<pLT->Text()[i].m_vWords[j].size();k++)
				{//	бежим по символам
					CharOut(pSurface,local_x,local_y,pLT->Text()[i].m_vWords[j][k],width_limit,height_limit,&local_x);
				}
				CharOut(pSurface,local_x,local_y,' ',width_limit,height_limit,&local_x);
				local_x += pLT->Text()[i].m_AddSpace;
			}
		}
		local_y += m_iFontHeight;
	}
}

void Font2D::TextOut(LPDIRECTDRAWSURFACE7 pSurface,int y,int width_limit,int height_limit,
				     LogicalText* pLT,int begin,int end,int* x_out,int* y_out)
{
	int local_x = 0;
	int local_y = y+pLT->TopOffset();
	int symbol_counter = 0;
	unsigned char ch;

	for(int i=0;((i<pLT->Text().size()) && (local_y<height_limit));i++)
	{//	бежим построкам
		if((local_y+m_iFontHeight)>0)
		{
			local_x = pLT->Text()[i].m_Offset;
			for(int j=0;j<pLT->Text()[i].m_vWords.size();j++)
			{//	бежим по словам
				for(int k=0;k<pLT->Text()[i].m_vWords[j].size();k++)
				{//	бежим по символам
					ch = pLT->Text()[i].m_vWords[j][k];
					if(symbol_counter>=begin)
						CharOut(pSurface,local_x,local_y,ch,width_limit,height_limit,&local_x);
					else
						local_x += m_abcChar[ch].abcA+m_abcChar[ch].abcC+m_abcChar[ch].abcB;
					symbol_counter++;
					if(symbol_counter >= end)
					{
						*x_out = local_x;
						*y_out = local_y;
//						font2d("[%0.4d,%0.4d]\n",*x_out,*y_out);
						return;
					}
				}
				if(j != (pLT->Text()[i].m_vWords.size()-1))
				{
					if(symbol_counter>=begin)
						CharOut(pSurface,local_x,local_y,' ',width_limit,height_limit,&local_x);
					else
						local_x += m_abcChar[' '].abcA+m_abcChar[' '].abcC+m_abcChar[' '].abcB;
					local_x += pLT->Text()[i].m_AddSpace;
					symbol_counter++;
					if(symbol_counter >= end)
					{
						*x_out = local_x;
						*y_out = local_y;
//						font2d("[%0.4d,%0.4d]\n",*x_out,*y_out);
						return;
					}
				}
			}
		}
		else
		{
			symbol_counter += pLT->Text()[i].m_Symbols;
		}
		local_y += m_iFontHeight;
	}
	*x_out = local_x;
	*y_out = local_y;
//	font2d("[%0.4d,%0.4d]\n",*x_out,*y_out);
}

void Font2D::CharOut(LPDIRECTDRAWSURFACE7 pSurface,int x,int y,unsigned char ch,int width_limit,int height_limit,int *xOut)
{
	x += m_abcChar[ch].abcA;
	RECT rcDst = {std::max(0,x),
			 	  std::max(0,y),
				  std::min(x+(int)m_rcChar[ch].right,/*x+*/width_limit),
				  std::min(y+(int)m_rcChar[ch].bottom,/*y+*/height_limit)};
	RECT rcSrc = {m_rcChar[ch].left+std::min((int)m_rcChar[ch].right,std::max(0,-x)),	// if x>0 that 0 else x
				  m_rcChar[ch].top+std::min((int)m_rcChar[ch].bottom,std::max(0,-y)),	// if y>0 that 0 else y,
				  m_rcChar[ch].left+std::min((int)m_rcChar[ch].right,(int)rcDst.right-(int)rcDst.left+std::max(0,-x)),
				  m_rcChar[ch].top+std::min((int)m_rcChar[ch].bottom,(int)rcDst.bottom-(int)rcDst.top+std::max(0,-y))};

	if(pSurface)
	{
		if(FAILED(pSurface->Blt(&rcDst,m_pSurface,&rcSrc,DDBLT_WAIT|DDBLT_KEYSRC,NULL)))
		{
//			font2d("     - blit operation FAILED!\n");
		}
		*xOut = x+m_abcChar[ch].abcC+m_abcChar[ch].abcB;
	}
}
