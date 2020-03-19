/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: класс для работы с определенным шрифтом.
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   14.06.2000

************************************************************************/
#include "Precomp.h"
#include "..\TextureMgr\NhtTarga.h"
#include "..\TextureMgr\D3DTexture.h"
#include "FormattedText.h"
#include "VirtualTexture.h"
#include "GFont.h"

//---------- Лог файл ------------
#ifdef _DEBUG_FONT
CLog font_log;
#define font	font_log["font.log"]
#else
#define font	/##/
#endif

#define max(a,b)	(a>b?a:b)
#define min(a,b)	(a<b?a:b)
#define TAB_SIZE	4
/*
#define max(a,b)	(a>b?a:b)

bool NhtBitBlt(unsigned char *pDestImage,
			   unsigned int uiDestPitch,
			   unsigned int uiDestBitCount,
			   unsigned int uiDestAlphaBitMask,
			   unsigned int uiDestRedBitMask,
			   unsigned int uiDestGreenBitMask,
			   unsigned int uiDestBlueBitMask,
			   unsigned char *pSrcImage,
			   unsigned int uiSrcPitch,
			   unsigned int uiSrcBitCount,
			   unsigned int uiSrcAlphaBitMask,
			   unsigned int uiSrcRedBitMask,
			   unsigned int uiSrcGreenBitMask,
			   unsigned int uiSrcBlueBitMask,
			   unsigned int uiImageWidth,
			   unsigned int uiImageHeight);
*/
GFont::GFont()
{
	m_bValid = false;
	m_pSurface = NULL;
}

GFont::~GFont()
{
	RELEASE(m_pSurface);
}

bool GFont::Load(LPDIRECT3DDEVICE7 lpd3dDevice,VFile *pVFile)
{
	MLF_FILEHEADER fh;
	MLF_DATAHEADER dh;

	if(pVFile->Size()!=0)
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
				if((pVFile->Size()-pVFile->Tell()) == dh.lWidth,dh.lHeight)
				{
					font("Try to create surface for font image.\n");
					//	создаем поверхность и копируем туда битмапку
					return CreateSurface(lpd3dDevice,dh.lWidth,dh.lHeight,pVFile);
				}
			}
		}
	}

	return false;
}

bool GFont::CreateSurface(LPDIRECT3DDEVICE7 lpd3dDevice,int width,int height,VFile *pVFile)
{
	NhtTarga tgaImage;
	D3DTexture d3dTexture;
	DDSURFACEDESC2 ddsd;

	//	делаем данные недействительными
	RELEASE(m_pSurface);
	m_bValid = false;
	if(lpd3dDevice)
	{
		//	создаем TGA-шку с заданными свойствами
		if(tgaImage.Create(1,1,32))
		{
			//	создаем текстуру из TGA-шки
			if(d3dTexture.Create(lpd3dDevice,&tgaImage,0,0))
			{
				font("Property surface was created.\n");
				//	получение объекта DirectDraw
				LPDIRECTDRAWSURFACE7 lpddsRender;
				LPDIRECTDRAW7        lpDD;
				lpd3dDevice->GetRenderTarget(&lpddsRender);
				lpddsRender->GetDDInterface((VOID**)&lpDD);
				lpddsRender->Release();
				//	создание поверхности
				ddsd.dwSize = sizeof(DDSURFACEDESC2);
				d3dTexture.Surface()->GetSurfaceDesc(&ddsd);
				ddsd.dwFlags	     = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
				ddsd.ddsCaps.dwCaps  = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
				ddsd.ddsCaps.dwCaps2 = 0L;
				ddsd.dwWidth         = width;
				ddsd.dwHeight        = height;
				font("Try create finished surface.\n");
				if(FAILED(lpDD->CreateSurface(&ddsd,&m_pSurface,NULL)))
				{
					lpDD->Release();	//	done by DirectDraw
					return false;
				}
				font("Finished surface was created.\n");
				//	создаем TGA-шку с заданными свойствами и размерами
				if(tgaImage.Create(width,height,32))
				{
					//	заполняем массив данными из битового образа шрифта
					int index;
					DWORD *pImage = (DWORD *)tgaImage.Image();
					for(int i=(height-1);i>=0;i--)
					{
						for(int j=0;j<width;j++)
						{
							index = (i*width)+j;
							pImage[index] = pVFile->ReadChar();
							pImage[index] += (pImage[index]<<8)+(pImage[index]<<16)+(pImage[index]<<24)/*+(pImage[index]<<24)*/;
						}
					}
					//	копируем данные в текстуру
					if(CopyRGBAToSurface(lpDD,&ddsd,&tgaImage,m_pSurface))
					{
						lpDD->Release();	//	done by DirectDraw
						return m_bValid = true;
					}
					
				}
				lpDD->Release();	//	done by DirectDraw
			}
		}

	}
	
	return false;
}

bool GFont::CopyRGBAToSurface(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,DIBData *pDIBData,LPDIRECTDRAWSURFACE7 lpSurface)
{
	DDCOLORKEY ddck = {0,0};
	DDSURFACEDESC2 ddsd;
	LPDIRECTDRAWSURFACE7 lpTempSurface;

	if((pDIBData->Bpp()==32) && 
	   (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS))
	{
//		tex("- Create Temp Surface and use Blt();\n");
		// Setup the new surface desc
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		lpSurface->GetSurfaceDesc(&ddsd);
		ddsd.dwFlags	     = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps  = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
		ddsd.ddsCaps.dwCaps2 = 0L;
		ddsd.dwWidth         = pDIBData->Width();
		ddsd.dwHeight        = pDIBData->Height();
		// Create a new surface for the D3DTexture
		if(FAILED(lpDD->CreateSurface(&ddsd,&lpTempSurface,NULL)))
			return FALSE;
//		tex("- New Temp Surface create - ok;\n");
	    while(lpTempSurface->Lock(NULL,&ddsd,0,0) == DDERR_WASSTILLDRAWING);

		//DWORD lPitch = ddsd.lPitch;
		BYTE* pBytes = (BYTE*)ddsd.lpSurface;
		DWORD* lpImage = (DWORD *)pDIBData->Image();
		
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
	    lpSurface->Blt(NULL,lpTempSurface,NULL,DDBLT_WAIT,NULL);
		//	Set Color Key
		lpSurface->SetColorKey(DDCKEY_SRCBLT,&ddck);
//		tex("- lpSurface->Blt( lpTempSurface ) - ok;\n");
	    lpTempSurface->Release();
//		tex("- lpTempSurface->Release() - ok;\n");

		return TRUE;
	}

	return FALSE;
}
/*********************************************************************************
	- pSurface - поверхность куда блитить
	- x,y - в какой координате
	- Sym - какой символ
	- xOut - сюда возвращается следующая позиция по x
	- width,height - ширина, высота области вывода
**********************************************************************************/
void GFont::CharOut(LPDIRECTDRAWSURFACE7 pSurface,int x,int y,unsigned char Sym,int *xOut,int width,int height)
{
	font("    ...x:y - [%d:%d]\n",x,y);
	x += m_abcChar[Sym].abcA;
	RECT rcDst = {max(0,x),
			 	  max(0,y),
				  min(x+m_rcChar[Sym].right,x+width),
				  min(y+m_rcChar[Sym].bottom,y+height)};
	RECT rcSrc = {m_rcChar[Sym].left+min(m_rcChar[Sym].right,max(0,-x)),	// if x>0 that 0 else x
				  m_rcChar[Sym].top+min(m_rcChar[Sym].bottom,max(0,-y)),	// if y>0 that 0 else y,
				  m_rcChar[Sym].left+min(m_rcChar[Sym].right,rcDst.right-rcDst.left+max(0,-x)),
				  m_rcChar[Sym].top+min(m_rcChar[Sym].bottom,rcDst.bottom-rcDst.top+max(0,-y))};

	font("    ...m_rcChar[Sym].bottom == %d m_rcChar[Sym].top == %d\n",m_rcChar[Sym].bottom,m_rcChar[Sym].top);
	if(m_bValid && pSurface)
	{
		font("    ...char out - [%d,%d,%d,%d]:[%d,%d,%d,%d] - [%d,%d]:[%d,%d];\n",rcSrc.left,rcSrc.top,rcSrc.right,rcSrc.bottom,rcDst.left,rcDst.top,rcDst.right,rcDst.bottom,rcSrc.right-rcSrc.left,rcSrc.bottom-rcSrc.top,rcDst.right-rcDst.left,rcDst.bottom-rcDst.top);
		if(FAILED(pSurface->Blt(&rcDst,m_pSurface,&rcSrc,DDBLT_WAIT|DDBLT_KEYSRC,NULL)))
		{
			font("     - blit operation FAILED!\n");
		}
		*xOut = x+m_abcChar[Sym].abcC+m_abcChar[Sym].abcB;
	}
}

void GFont::TextOut(LPDIRECTDRAWSURFACE7 pSurface,int x,int y,
					const char* pText,int width,int height)
{
	int len = strlen(pText);

	for(int i=0;i<len;i++)
	{
		CharOut(pSurface,x,y,pText[i],&x,width,height);
	}
}

void GFont::TextOutEx(LPDIRECTDRAWSURFACE7 pSurface,int y,std::vector<GFONT_STRING_INFO>& sStrings,int width,int height)
{
	int local_x;
	int local_y = y;

	for(int i=0;((i<sStrings.size()) && (local_y<height));i++)
	{
		local_x = 0;
		for(int j=0;j<sStrings[i].vWords.size();j++)
		{
			if((local_y+m_iFontHeight)>0)
			{
				
				for(int k=0;k<sStrings[i].vWords[j].size();k++)
				{
					CharOut(pSurface,local_x,local_y,sStrings[i].vWords[j][k],&local_x,width,height);
				}
				CharOut(pSurface,local_x,local_y,' ',&local_x,width,height);
				local_x += sStrings[i].iAddSpace;
			}
		}
		local_y += m_iFontHeight;
	}
}

void GFont::TextOutEx(LPDIRECTDRAWSURFACE7 pSurface,int y,std::vector<GFONT_STRING_INFO>& sStrings,
					  int width,int height,unsigned int uiPos,int *rx,int *ry)
{
	int local_x = 0;
	int local_y = y;
	int max_i,max_j,max_k;
	int i = 0, j = 0, k = 0;
	int max_words;
	int len_word;

	for(i=0;(i<sStrings.size() && uiPos);i++)
		for(j=0;(j<sStrings[i].vWords.size() && uiPos);j++)
			for(k=0;(k<sStrings[i].vWords[j].size() && uiPos);k++)
				uiPos--;

	max_i = i;
	max_j = j;
	max_k = k;

	*rx = 0;
	*ry = 0;

	for(i=0;(i<max_i && (local_y<height));i++)
	{
		local_x = 0;
		*rx = 0;
		max_words = sStrings[i].vWords.size();
		if(i == (max_i-1))
		{//	последняя строчка
			max_words = max_j;
			*ry -= m_iFontHeight;
		}
		for(j=0;j<max_words;j++)
		{
			len_word = sStrings[i].vWords[j].size();
			if((i == (max_i-1)) && (j == (max_j-1)))
			{//	последнее слово
				len_word = max_k;
			}
			for(k=0;k<len_word;k++)
			{
				CharOut(pSurface,local_x,local_y,sStrings[i].vWords[j][k],&local_x,width,height);
			}
			*rx = local_x;
			CharOut(pSurface,local_x,local_y,' ',&local_x,width,height);
			local_x += sStrings[i].iAddSpace;
		}
		local_y += m_iFontHeight;
		*ry += m_iFontHeight;
	}
/*	*rx = local_x-sStrings[i].iAddSpace;
	*ry = local_y;*/
}

//*************************************************************************************//
void GFont::TextOut(LPDIRECTDRAWSURFACE7 pSurface,
					FormattedText *m_pfText,
					int y,
					int width,
					int height)
{
	int local_x = 0;
	int local_y = y+m_pfText->TopOffset();

	font("\n--- TextOut ---\n");
	font("    ...height == %d\n",height);
	font("    ...width == %d\n",width);
	font("    ...m_iFontHeight == %d\n",m_iFontHeight);
	font("    ...m_pfText->TopOffset() == %d\n",m_pfText->TopOffset());

	for(int i=0;((i<m_pfText->m_vFS.size()) && (local_y<height));i++)
	{//	бежим построкам
		font("    ...local_y == %d\n",local_y);
		if((local_y+m_iFontHeight)>0)
		{
			local_x = m_pfText->m_vFS[i].uiOffset;
			font("     - local_x == %d\n",local_x);
			for(int j=0;j<m_pfText->m_vFS[i].vWords.size();j++)
			{//	бежим по словам
				font("[%s]\n",m_pfText->m_vFS[i].vWords[j].c_str());
				for(int k=0;k<m_pfText->m_vFS[i].vWords[j].size();k++)
				{//	бежим по символам
					CharOut(pSurface,local_x,local_y,m_pfText->m_vFS[i].vWords[j][k],&local_x,width,height);
					font("local_x:local_y == [%d:%d]\n",local_x,local_y);
				}
				CharOut(pSurface,local_x,local_y,' ',&local_x,width,height);
				local_x += m_pfText->m_vFS[i].uiAddSpace;
			}
			font("\n");
		}
		local_y += m_iFontHeight;
	}
}

void GFont::TextOut(VirtualTexture *pTexture,
					FormattedText *pfText,
					int y,
					int width,
					int height)
{
	int local_x = 0;
	int local_y = y+pfText->TopOffset();

	font("\n--- TextOut ---\n");
	font("    ...height == %d\n",height);
	font("    ...width == %d\n",width);
	font("    ...m_iFontHeight == %d\n",m_iFontHeight);

	for(int i=0;((i<pfText->m_vFS.size()) && (local_y<height));i++)
	{//	бежим построкам
		font("    ...local_y == %d\n",local_y);
		if((local_y+m_iFontHeight)>0)
		{
			local_x = pfText->m_vFS[i].uiOffset;
			font("     - local_x == %d\n",local_x);
			for(int j=0;j<pfText->m_vFS[i].vWords.size();j++)
			{//	бежим по словам
				font("[%s]\n",pfText->m_vFS[i].vWords[j].c_str());
				for(int k=0;k<pfText->m_vFS[i].vWords[j].size();k++)
				{//	бежим по символам
					CharOut(pTexture,local_x,local_y,pfText->m_vFS[i].vWords[j][k],&local_x,width,height);
					font("local_x:local_y == [%d:%d]\n",local_x,local_y);
				}
				CharOut(pTexture,local_x,local_y,' ',&local_x,width,height);
				local_x += pfText->m_vFS[i].uiAddSpace;
			}
			font("\n");
		}
		local_y += m_iFontHeight;
	}
}

void GFont::CharOut(VirtualTexture *pTexture,int x,int y,unsigned char Sym,int *xOut,int width,int height)
{
	x += m_abcChar[Sym].abcA;
	RECT rcDst = {max(0,x),
			 	  max(0,y),
				  min(x+m_rcChar[Sym].right,x+width),
				  min(y+m_rcChar[Sym].bottom,y+height)};
	RECT rcSrc = {m_rcChar[Sym].left+min(m_rcChar[Sym].right,max(0,-x)),	// if x>0 that 0 else x
				  m_rcChar[Sym].top+min(m_rcChar[Sym].bottom,max(0,-y)),	// if y>0 that 0 else y,
				  m_rcChar[Sym].left+m_rcChar[Sym].right,
				  m_rcChar[Sym].top+m_rcChar[Sym].bottom};

	if(m_bValid && pTexture)
	{
		font("    ...char out - [%d,%d,%d,%d]:[%d,%d,%d,%d] - [%d,%d]:[%d,%d];\n",rcSrc.left,rcSrc.top,rcSrc.right,rcSrc.bottom,rcDst.left,rcDst.top,rcDst.right,rcDst.bottom,rcSrc.right-rcSrc.left,rcSrc.bottom-rcSrc.top,rcDst.right-rcDst.left,rcDst.bottom-rcDst.top);
		pTexture->Blt(&rcDst,m_pSurface,&rcSrc,DDBLT_WAIT|DDBLT_KEYSRC,NULL);
		*xOut = x+m_abcChar[Sym].abcC+m_abcChar[Sym].abcB;
	}
}

//*************************************************************************************//
void GFont::ResolveText(const char *pText,std::vector<GFONT_STRING_INFO>& sStrings,int width,int )
{
	int len = strlen(pText);
	int begin_pos;
	int fix_pos;
	GFONT_STRING_INFO si;

	font("****************** ResolveText *******************\n");
	font("Text: [%s]",pText);
	//	очищаем массив
	sStrings.clear();
	for(begin_pos=0;begin_pos<len;)
	{
		//	расчитываем строчку
		font("Resolve string: \n");
		fix_pos = begin_pos;
		begin_pos += ResolveString(pText+begin_pos,width,si.vWords,&si.iAddSpace,&si.bEndOfString);
		font("    ...iSpaceWidth == %d;\n",si.iAddSpace);
		font("    ...bEndOfString == %d;\n",si.bEndOfString);
		if(si.bEndOfString)
			begin_pos++;
		if(begin_pos == fix_pos && !si.bEndOfString)
				break;	//	невозможно вывести текст
		sStrings.push_back(si);
		font("--------------------------push_back sWords;\n");
	}
}

//	возвращает начало следующей строки
unsigned int GFont::ResolveString(const char *pText,int width,std::vector<std::string>& vWords,unsigned int *iAddSpace,bool *bEndOfString)
{
	std::string str;
	int len = strlen(pText);
	SIZE sz,szSpace;
	int size = 0;
	int word_len;

	vWords.clear();
	*iAddSpace = 0;
	*bEndOfString = false;
	GetSize(' ',&szSpace);
	//	ищем максимальное кол-во слов вмещаемых в строку
	int i;
	for(i=0;i<len;i++)
	{
		word_len = GetWord(pText+i,str);
		if(word_len)
		{
			GetSize(str.c_str(),str.size(),&sz);
			if((size + sz.cx) > width)
			{
				//	вдруг слово не вмещается
				if(i==0)
				{
					vWords.push_back(str);
					i = word_len;
				}
				else
				{
					//	рассчитывем пробелы
					font("width: %d size %d vWords.size() %d\n",width,size,vWords.size());
					if(vWords.size()>1)
						*iAddSpace = (width-(size-szSpace.cx))/(vWords.size()-1);
					else
						*iAddSpace = 0;
				}
				break;
			}
			else
			{
				vWords.push_back(str);
				font("word == \"%s\"\n",vWords[vWords.size()-1].c_str());
//				words++;
				size += sz.cx;
				size += szSpace.cx;
				i += word_len;
			}
		}
		if(pText[i] == '\n')
		{//	конец строки
			*bEndOfString = true;
			break;
		}
	}
	//	нашли
	return i;
}

unsigned int GFont::GetWord(const char *pText,std::string& str)
{
	int len = strlen(pText);
	bool bNoSpaceFind = false;

	str.clear();
	int i;
	for(i=0;i<len;i++)
	{
		if(pText[i] == '\t')
		{
			for(int j=0;j<TAB_SIZE;j++)
			{
				str += ' ';
			}
			bNoSpaceFind = true;
			continue;
		}
		if((bNoSpaceFind && pText[i] == ' ') || (pText[i] == '\n'))
			break;
		str += pText[i];
		if(pText[i] != ' ')
			bNoSpaceFind = true;
	}

	return i;
}

//**********************************************************************************//
void GFont::GetSize(const char *pText,unsigned int n,SIZE *pSZ)
{
	pSZ->cx = 0;
	pSZ->cy = 0;
	for(int i=0;i<n;i++)
	{
		pSZ->cx += m_abcChar[(unsigned char)pText[i]].abcA+
				   m_abcChar[(unsigned char)pText[i]].abcC+
				   m_abcChar[(unsigned char)pText[i]].abcB;
	}
	pSZ->cy = m_iFontHeight;
}

void GFont::GetSize(const char *pText,SIZE *pSZ)
{
	int len = strlen(pText);

	pSZ->cx = 0;
	pSZ->cy = 0;
	for(int i=0;i<len;i++)
	{
		pSZ->cx += m_abcChar[(unsigned char)pText[i]].abcA+
				   m_abcChar[(unsigned char)pText[i]].abcC+
				   m_abcChar[(unsigned char)pText[i]].abcB;
//		pSZ->cx += m_rcChar[pText[i]].right;
//		pSZ->cy += m_rcChar[pText[i]].bottom;
	}
	pSZ->cy = m_iFontHeight;
}

void GFont::GetSize(unsigned char ch,SIZE *pSZ)
{
	pSZ->cx = m_abcChar[ch].abcA+
			  m_abcChar[ch].abcC+
			  m_abcChar[ch].abcB;
	pSZ->cy = m_iFontHeight;
}

/*
//	поддерживаем форматы 16,24,32 бит на пиксел
bool NhtBitBlt(unsigned char *pDestImage,
			   unsigned int uiDestPitch,
			   unsigned int uiDestBitCount,
			   unsigned int uiDestAlphaBitMask,
			   unsigned int uiDestRedBitMask,
			   unsigned int uiDestGreenBitMask,
			   unsigned int uiDestBlueBitMask,
			   unsigned char *pSrcImage,
			   unsigned int uiSrcPitch,
			   unsigned int uiSrcBitCount,
			   unsigned int uiSrcAlphaBitMask,
			   unsigned int uiSrcRedBitMask,
			   unsigned int uiSrcGreenBitMask,
			   unsigned int uiSrcBlueBitMask,
			   unsigned int uiImageWidth,
			   unsigned int uiImageHeight)
{
	unsigned int mask;
	unsigned char cRSrcShiftRight = 0; 
	unsigned char cGSrcShiftRight = 0; 
	unsigned char cBSrcShiftRight = 0; 
	unsigned char cASrcShiftRight = 0; 
	//	кол-во бит в маске
	int cRSrcMaskBits = 0; 
	int cGSrcMaskBits = 0; 
	int cBSrcMaskBits = 0; 
	int cASrcMaskBits = 0; 
	//	------------------
	unsigned char cRDestMaskLeft = 0; 
	unsigned char cGDestMaskLeft = 0; 
	unsigned char cBDestMaskLeft = 0; 
	unsigned char cADestMaskLeft = 0; 
	//	смещение влево для выравнивания цвета
	unsigned char cRDestShiftLeft = 0; 
	unsigned char cGDestShiftLeft = 0; 
	unsigned char cBDestShiftLeft = 0; 
	unsigned char cADestShiftLeft = 0; 
	//	смещеная маска
	unsigned int cRDestMask = 0; 
	unsigned int cGDestMask = 0; 
	unsigned int cBDestMask = 0; 
	unsigned int cADestMask = 0; 
	//	---------------
	int cDestMaskBits = 0;

	if((pDestImage && pSrcImage) && 
	   (uiDestBitCount == 16 || uiDestBitCount == 24 || uiDestBitCount == 32) &&
	   (uiSrcBitCount == 16 || uiSrcBitCount == 24 || uiSrcBitCount == 32))
	{
		//	подсчет битов на каждый цвет в SrcImage
		for(mask = uiSrcRedBitMask;mask && !(mask&1);mask>>=1)
			cRSrcShiftRight++;
		for(;mask;mask>>=1)
			cRSrcMaskBits++;
		for(mask = uiSrcGreenBitMask;mask && !(mask&1);mask>>=1)
			cGSrcShiftRight++;
		for(;mask;mask>>=1)
			cGSrcMaskBits++;
		for(mask = uiSrcBlueBitMask;mask && !(mask&1);mask>>=1)
			cBSrcShiftRight++;
		for(;mask;mask>>=1)
			cBSrcMaskBits++;
		for(mask = uiSrcAlphaBitMask;mask && !(mask&1);mask>>=1)
			cASrcShiftRight++;
		for(;mask;mask>>=1)
			cASrcMaskBits++;
		//	для DestImage
		//	смещение маски на начало 
		//	подсчет битов на кол-во которых надо сместить маску
		for(mask = uiDestRedBitMask;mask && !(mask&1);mask>>=1)
			cRDestMaskLeft++;
		cRDestMask = mask;
		for(cDestMaskBits = 0;mask;mask>>=1)
			cDestMaskBits++;
		cRDestShiftLeft = max((cDestMaskBits-cRSrcMaskBits),0);
		for(mask = uiDestGreenBitMask;mask && !(mask&1);mask>>=1)
			cGDestMaskLeft++;
		cGDestMask = mask;
		for(cDestMaskBits = 0;mask;mask>>=1)
			cDestMaskBits++;
		cGDestShiftLeft = max((cDestMaskBits-cGSrcMaskBits),0);
		for(mask = uiDestBlueBitMask;mask && !(mask&1);mask>>=1)
			cBDestMaskLeft++;
		cBDestMask = mask;
		for(cDestMaskBits = 0;mask;mask>>=1)
			cDestMaskBits++;
		cBDestShiftLeft = max((cDestMaskBits-cBSrcMaskBits),0);
		for(mask = uiDestAlphaBitMask;mask && !(mask&1);mask>>=1)
			cADestMaskLeft++;
		cADestMask = mask;
		for(cDestMaskBits = 0;mask;mask>>=1)
			cDestMaskBits++;
		cADestShiftLeft = max((cDestMaskBits-cASrcMaskBits),0);
		//	копируем пикселы uiImageHeight
		for(unsigned int y=0;y<uiImageHeight;y++)
		{
/*			unsigned short *pWordDestImage = (unsigned short *)pDestImage;
			unsigned int *pDWordDestImage = (unsigned int *)pDestImage;*/
/*			for(unsigned int x=0;x<uiImageWidth;x++)
			{
				unsigned int uiPixel = 0;	//	извлекаемый пиксел
				unsigned int uiValue;
				
				switch(uiSrcBitCount)
				{
				case 16:
					uiPixel = *((unsigned short*)(pSrcImage+(y*uiSrcPitch))+x);
					break;
				case 24:
					uiPixel = ((unsigned int)*((unsigned char*)(pSrcImage+(y*uiSrcPitch))+x*3));
					uiPixel += ((unsigned int)*((unsigned char*)(pSrcImage+(y*uiSrcPitch))+x*3+1))<<8;
					uiPixel += ((unsigned int)*((unsigned char*)(pSrcImage+(y*uiSrcPitch))+x*3+2))<<16;
					break;
				case 32:
					uiPixel = *((unsigned int*)(pSrcImage+(y*uiSrcPitch))+x);
					break;
				}
				//	извлекаем текущий пиксел
/*				unsigned int r = (unsigned char)((((uiPixel&uiSrcRedBitMask)>>cRSrcShiftRight)<<cRSrcShiftLeft));
				unsigned int g = (unsigned char)((((uiPixel&uiSrcGreenBitMask)>>cGSrcShiftRight)<<cGSrcShiftLeft));
				unsigned int b = (unsigned char)((((uiPixel&uiSrcBlueBitMask)>>cBSrcShiftRight)<<cBSrcShiftLeft));
				unsigned int a = (unsigned char)((((uiPixel&uiSrcAlphaBitMask)>>cASrcShiftRight)<<cASrcShiftLeft));*/
/*				unsigned int r = (unsigned char)((((uiPixel&uiSrcRedBitMask)>>cRSrcShiftRight)/*<<cRSrcShiftLeft*//*));*/
/*				unsigned int g = (unsigned char)((((uiPixel&uiSrcGreenBitMask)>>cGSrcShiftRight)/*<<cGSrcShiftLeft*//*));*/
/*				unsigned int b = (unsigned char)((((uiPixel&uiSrcBlueBitMask)>>cBSrcShiftRight)/*<<cBSrcShiftLeft*//*));*/
/*				unsigned int a = (unsigned char)((((uiPixel&uiSrcAlphaBitMask)>>cASrcShiftRight)/*<<cASrcShiftLeft*//*));*/

/*				a = ((a&cADestMask)<<cADestMaskLeft)<<cADestShiftLeft;
				r = ((r&cRDestMask)<<cRDestMaskLeft)<<cRDestShiftLeft;
				g = ((g&cGDestMask)<<cGDestMaskLeft)<<cGDestShiftLeft;
				b = ((b&cBDestMask)<<cBDestMaskLeft)<<cBDestShiftLeft;
				switch(uiDestBitCount)
				{
				case 16:
					((unsigned short *)pDestImage)[x] = a|r|g|b;
					break;
				case 24:
					uiValue = (a|r|g|b)<<8;
					((unsigned int *)pDestImage)[x] &= 0x000000ff;
					((unsigned int *)pDestImage)[x] |= uiValue;
					break;
				case 32:
					((unsigned int *)pDestImage)[x] = a|r|g|b;
					break;
				}
			}
			pDestImage += uiDestPitch;
		}

		return true;
	}

	return false;
}
*/