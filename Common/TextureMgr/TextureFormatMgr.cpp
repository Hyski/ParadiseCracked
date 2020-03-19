/***********************************************************************

                             Texture Manager

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.05.2001

************************************************************************/
#include "Precomp.h"
#include "TextureFormatMgr.h"

#include <algorithm>

//---------- Лог файл ------------
#ifdef _HOME_VERSION
CLog tfm_log;
#define tfm	tfm_log["tfm.log"]
#else
#define tfm	/##/
#endif
//--------------------------------

HRESULT CALLBACK D3DEnumPixelFormatsCallback(LPDDPIXELFORMAT lpDDPixFmt,  
											 LPVOID lpContext);

bool operator < (const PixelFormat& x,const PixelFormat& y);
bool operator == (const PixelFormat& x,const PixelFormat& y);

TextureFormatMgr::Deleter TextureFormatMgr::m_Deleter;
TextureFormatMgr::TextureFormatMgr()
{
}

TextureFormatMgr::~TextureFormatMgr()
{
	tfm("\ndestroy this instance ---\n\n");
}

void TextureFormatMgr::Init(LPDIRECT3DDEVICE7 lpd3dDevice)
{
	tfm("--- Init: Searching Existed Pixel's Formats ---\n\n");
	m_vPFs.clear();
	lpd3dDevice->EnumTextureFormats(D3DEnumPixelFormatsCallback,&m_vPFs);
	//	сортируем текстуры по качеству
	std::sort(m_vPFs.begin(),m_vPFs.end());
#ifdef _DEBUG_GRAPHICS
	tfm("\n--- Sorted Textures ---\n");
	for(int i=0;i<m_vPFs.size();i++)
	{
		tfm("[%d] ARGB:%d%d%d%d %s [%d]\n",m_vPFs[i].m_ddpf.dwRGBBitCount,m_vPFs[i].m_AlphaBitCount,m_vPFs[i].m_RedBitCount,m_vPFs[i].m_GreenBitCount,m_vPFs[i].m_BlueBitCount,m_vPFs[i].m_cf?"compressed":"",m_vPFs[i].m_cf);
	}
#endif
}

void TextureFormatMgr::Release(void)
{
	m_vPFs.clear();
	tfm("- released instance;\n");
}

//	получить близкий формат текстур к заданному
bool TextureFormatMgr::ChoosePixelFormat(DDPIXELFORMAT* pDDPF)
{
	PixelFormat pf(pDDPF);
	int suitable = -1;	
	int i;

	tfm("\n--- ChoosePixelFormat ---\n");
	tfm("in  [%d] ARGB:%d%d%d%d %s [%d]\n",pf.m_ddpf.dwRGBBitCount,pf.m_AlphaBitCount,pf.m_RedBitCount,pf.m_GreenBitCount,pf.m_BlueBitCount,pf.m_cf?"compressed":"",pf.m_cf);

	if(pf.m_cf != CF_NONE)
	{//	ищем среди компрессированных форматов
		for(i=0;i<m_vPFs.size();i++)
		{
			if(pf.m_cf == m_vPFs[i].m_cf)
			{//	нашли подходящий формат
				suitable = i;
				break;
			}
		}
		if(suitable == -1)
		{//	нужный формат не найден, преобразуем к явному формату
/*			switch(pf.m_cf)
			{
			case CF_DXT1:
				pf.m_AlphaBitCount = 1;
				pf.m_RedBitCount   = 5;
				pf.m_GreenBitCount = 5;
				pf.m_BlueBitCount  = 5;
				break;
			case CF_DXT2:
			case CF_DXT3:
				pf.m_AlphaBitCount = 4;
				pf.m_RedBitCount   = 4;
				pf.m_GreenBitCount = 4;
				pf.m_BlueBitCount  = 4;
				break;
			case CF_DXT4:
			case CF_DXT5:
				pf.m_AlphaBitCount = 8;
				pf.m_RedBitCount   = 8;
				pf.m_GreenBitCount = 8;
				pf.m_BlueBitCount  = 8;
				break;
			}*/
			pf.m_cf = CF_NONE;			//	используем несжатые текстуры
//			pf.m_pf = PF_NONE;			//	используем непалитрованные текстуры
		}
	}
	if(suitable == -1)
	{
		//	проверяем на наличие палитры
		if(pf.m_pf != PF_NONE)
		{
			for(i=0;i<m_vPFs.size();i++)
			{
				if(pf.m_pf == m_vPFs[i].m_pf)
				{//	нашли подходящий формат
					suitable = i;
					break;
				}
			}
			if(suitable == -1)
			{//	нужный формат не найден, преобразуем к явному формату
				pf.m_AlphaBitCount = 0;
				pf.m_RedBitCount   = 8;
				pf.m_GreenBitCount = 8;
				pf.m_BlueBitCount  = 8;
				pf.m_pf = PF_NONE;			//	используем непалитрованные текстуры
			}
		}
		if(suitable == -1)
		{
			//	ищем среди явных форматов
			for(i=(m_vPFs.size()-1);i>=0;i--)
			{
				/* by Flif int y = m_vPFs[i].m_cf;*/
				if(!m_vPFs[i].m_cf && !m_vPFs[i].m_pf)
				{//	не рассматриваем компрессированные и палитрованные текстуры
					if(!(pf<m_vPFs[i]))
					{
						if(pf == m_vPFs[i])
						{
							suitable = i;
						}
						else
						{
							i++;
							if((i != m_vPFs.size()) && !m_vPFs[i].m_cf)
							{
								//	если альфа в обоих случаях не равна нулю или в обоих случаях равна нулю
								if((m_vPFs[i].m_AlphaBitCount && pf.m_AlphaBitCount) || (!m_vPFs[i].m_AlphaBitCount && !pf.m_AlphaBitCount))
									suitable = i;
								else
									suitable = i-1;
							}
							else
							{
								suitable = i-1;
							}
						}
						break;
					}
				}
			}
		}
	}
	if(suitable != -1)
	{
		tfm("out [%d] ARGB:%d%d%d%d %s [%d] %s [%d]\n",m_vPFs[suitable].m_ddpf.dwRGBBitCount,m_vPFs[suitable].m_AlphaBitCount,m_vPFs[suitable].m_RedBitCount,m_vPFs[suitable].m_GreenBitCount,m_vPFs[suitable].m_BlueBitCount,m_vPFs[suitable].m_cf?"compressed":"",m_vPFs[suitable].m_cf,m_vPFs[suitable].m_pf?"with palette":"",m_vPFs[suitable].m_pf);
		memcpy(pDDPF,&m_vPFs[suitable].m_ddpf,sizeof(DDPIXELFORMAT));
		return true;
	}

	return false;
}

const char* TextureFormatMgr::GetFourCCName(DWORD dwFourCC)
{
	switch(dwFourCC)
	{
	case MAKEFOURCC('A','U','R','2'):
		return "AuraVision Aura 2: YUV 422";
	case MAKEFOURCC('A','U','R','A'):
		return "AuraVision Aura 1: YUV 411"; 
	case MAKEFOURCC('C','H','A','M'):
		return "Inc. MM_WINNOV_CAVIARA_CHAMPAGNE"; 
	case MAKEFOURCC('C','V','I','D'):
		return "Cinepak by Supermac"; 
	case MAKEFOURCC('C','Y','U','V'):
		return "Creative Labs YUV"; 
	case MAKEFOURCC('D','X','T','1'):
		return "DirectX Texture Compression Format 1"; 
	case MAKEFOURCC('D','X','T','2'):
		return "DirectX Texture Compression Format 2"; 
	case MAKEFOURCC('D','X','T','3'):
		return "DirectX Texture Compression Format 3"; 
	case MAKEFOURCC('D','X','T','4'):
		return "DirectX Texture Compression Format 4"; 
	case MAKEFOURCC('D','X','T','5'):
		return "DirectX Texture Compression Format 5"; 
	case MAKEFOURCC('F','V','F','1'):
		return "Fractal Video Frame"; 
	case MAKEFOURCC('I','F','0','9'):
		return "Intel Intermediate YUV9"; 
	case MAKEFOURCC('I','V','3','1'):
		return "Indeo 3.1"; 
	case MAKEFOURCC('J','P','E','G'):
		return "Still Image JPEG DIB"; 
	case MAKEFOURCC('M','J','P','G'):
		return "Motion JPEG DIB Format"; 
	case MAKEFOURCC('M','R','L','E'):
		return "Run Length Encoding"; 
	case MAKEFOURCC('M','S','V','C'):
		return "Video 1"; 
	case MAKEFOURCC('P','H','M','O'):
		return "Photomotion"; 
	case MAKEFOURCC('R','T','2','1'):
		return "Indeo 2.1"; 
	case MAKEFOURCC('U','L','T','I'):
		return "Ultimotion"; 
	case MAKEFOURCC('V','4','2','2'):
		return "24-bit YUV 4:2:2"; 
	case MAKEFOURCC('V','6','5','5'):
		return "16-bit YUV 4:2:2"; 
	case MAKEFOURCC('V','D','C','T'):
		return "Video Maker Pro DIB"; 
	case MAKEFOURCC('V','I','D','S'):
		return "YUV 4:2:2 CCIR 601 for V422"; 
	case MAKEFOURCC('Y','U','9','2'):
		return "YUV"; 
	case MAKEFOURCC('Y','U','V','8'):
		return "MM_WINNOV_CAVIAR_YUV8"; 
	case MAKEFOURCC('Y','U','V','9'):
		return "YUV9"; 
	case MAKEFOURCC('Y','U','Y','V'):
		return "BI_YUYV, Canopus"; 
	case MAKEFOURCC('Z','P','E','G'):
		return "Video Zipper"; 
	}

	return "Unknown technology";
}

COMPRESSED_FORMAT TextureFormatMgr::GetFourCCValue(DWORD dwFourCC)
{
	switch(dwFourCC)
	{
	case MAKEFOURCC('D','X','T','1'):
		return CF_DXT1; 
	case MAKEFOURCC('D','X','T','2'):
		return CF_DXT2; 
	case MAKEFOURCC('D','X','T','3'):
		return CF_DXT3; 
	case MAKEFOURCC('D','X','T','4'):
		return CF_DXT4; 
	case MAKEFOURCC('D','X','T','5'):
		return CF_DXT5; 
	}

	return CF_NONE;
}

PALETTE_FORMAT TextureFormatMgr::GetPaletteIndexed(DWORD dwFlags)
{
	if(dwFlags&DDPF_PALETTEINDEXED1) return PF_1BIT;
	if(dwFlags&DDPF_PALETTEINDEXED2) return PF_2BIT;
	if(dwFlags&DDPF_PALETTEINDEXED4) return PF_4BIT;
	if(dwFlags&DDPF_PALETTEINDEXED8) return PF_8BIT;

	return PF_NONE;
}

//******************************************************************************//
// struct PixelFormat
PixelFormat::PixelFormat()
{
	memset(this,0,sizeof(PixelFormat));
}

PixelFormat::PixelFormat(LPDDPIXELFORMAT pDDPF)
{
	DWORD mask;
	
	memset(this,0,sizeof(PixelFormat));

	m_cf = CF_NONE;
	m_pf = PF_NONE;

	//	прверяем на наличие палитры
	if(pDDPF->dwFlags & (DDPF_PALETTEINDEXED1|DDPF_PALETTEINDEXED2|DDPF_PALETTEINDEXED4|DDPF_PALETTEINDEXED8))
	{//	палитра присутствует
		m_pf = TextureFormatMgr::GetPaletteIndexed(pDDPF->dwFlags);
	}
	else
	{//	палитра отсутствует
		//	подсчет битов на каждый цвет в текстуре
		for(mask = pDDPF->dwRGBAlphaBitMask;mask;mask>>=1)
			m_AlphaBitCount += (mask & 0x1);
		for(mask = pDDPF->dwRBitMask;mask;mask>>=1)
			m_RedBitCount += (mask & 0x1);
		for(mask = pDDPF->dwGBitMask;mask;mask>>=1)
			m_GreenBitCount += (mask & 0x1);
		for(mask = pDDPF->dwBBitMask;mask;mask>>=1)
			m_BlueBitCount += (mask & 0x1);
	}
	if(pDDPF->dwFourCC != 0)
	{//	формат поддерживающий сжатые текстуры
		m_cf = TextureFormatMgr::GetFourCCValue(pDDPF->dwFourCC);
	}
	//	копируем пиксел формат
	memcpy(&m_ddpf,pDDPF,sizeof(DDPIXELFORMAT));
}

inline bool operator < (const PixelFormat& x,const PixelFormat& y)
{
	//	разделение на сжатые форматы и явные
	if(!x.m_cf && y.m_cf)
		return true;
	if(x.m_cf && !y.m_cf)
		return false;
	if(x.m_cf < y.m_cf)
		return true;
	//	отбрасываем альфа слева
	if(!x.m_AlphaBitCount && y.m_AlphaBitCount)
		return true;
	if(x.m_AlphaBitCount && !y.m_AlphaBitCount)
		return false;
	//	преимущество за кол-вом битов на RGB компоненты
	if((x.m_RedBitCount+x.m_GreenBitCount+x.m_BlueBitCount) < (y.m_RedBitCount+y.m_GreenBitCount+y.m_BlueBitCount))
		return true;

	return false;
}

bool operator == (const PixelFormat& x,const PixelFormat& y)
{
	if((x.m_cf == y.m_cf) &&
	   (x.m_pf == y.m_pf) &&
	   (x.m_AlphaBitCount == y.m_AlphaBitCount) &&
	   (x.m_RedBitCount == y.m_RedBitCount) &&
	   (x.m_GreenBitCount == y.m_GreenBitCount) &&
	   (x.m_BlueBitCount == y.m_BlueBitCount))
	{
		return true;
	}

	return false;
}

//****************************************************************************//
HRESULT CALLBACK D3DEnumPixelFormatsCallback(LPDDPIXELFORMAT lpDDPixFmt,  
											 LPVOID lpContext)
{
	std::vector<PixelFormat>* pf_vector = static_cast<std::vector<PixelFormat>*>(lpContext);

	//	проверяем действительность входных параметров
    if(!lpDDPixFmt || !lpContext) return DDENUMRET_OK;
	//	двинули!
    // в данной реализации пропускаем подобные пиксел форматы
    if(lpDDPixFmt->dwFlags & (DDPF_LUMINANCE|DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV))
	{
		tfm("     ...skip DDPF_LUMINANCE|DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV;\n");
        return DDENUMRET_OK;
	}
    //	проверяем на наличие палитры
    if(lpDDPixFmt->dwFlags & (DDPF_PALETTEINDEXED1|DDPF_PALETTEINDEXED2|DDPF_PALETTEINDEXED4|DDPF_PALETTEINDEXED8))
	{//	сейчас мы не поддерживаем палитру с альфа каналом
		if(!(lpDDPixFmt->dwFlags&DDPF_ALPHAPIXELS))
		{
			tfm("     ...find pallete format [%d indexed];\n",TextureFormatMgr::GetPaletteIndexed(lpDDPixFmt->dwFlags));
			tfm("        current module skip palliete formats :(((\n");
	        return DDENUMRET_OK;
		}
	}
	else
	{//	непалитрованные тексуры
		//	проверяем на наличие сжатых форматов текстур
		if(lpDDPixFmt->dwFourCC != 0)
		{//	сжатый формат
			tfm("     ...find compressed format [%s];\n",TextureFormatMgr::GetFourCCName(lpDDPixFmt->dwFourCC));
			if(TextureFormatMgr::GetFourCCValue(lpDDPixFmt->dwFourCC) == CF_NONE)
			{
				tfm("        current compressed format not support;\n");
				return DDENUMRET_OK;
			}
		}
		else
		{//	обычный некомпрессированный формат
			// в данной реализации пропускаем подобные пиксел форматы
			if(lpDDPixFmt->dwRGBBitCount < 16)
			{
				tfm("     ...skip lpDDPixFmt->dwRGBBitCount < 16;\n");
				return DDENUMRET_OK;
			}
			tfm("     ...find format;\n");
		}
	}
	//	заносим найденный формат в список
	pf_vector->push_back(PixelFormat(lpDDPixFmt));
	tfm("        ARGB:%d%d%d%d (%d)\n",(*pf_vector)[pf_vector->size()-1].m_AlphaBitCount,(*pf_vector)[pf_vector->size()-1].m_RedBitCount,(*pf_vector)[pf_vector->size()-1].m_GreenBitCount,(*pf_vector)[pf_vector->size()-1].m_BlueBitCount,lpDDPixFmt->dwRGBBitCount);
	tfm("        Masks: A(%0.8x) R(%0.8x) G(%0.8x) B(%0.8x) \n",lpDDPixFmt->dwRGBAlphaBitMask,lpDDPixFmt->dwRBitMask,lpDDPixFmt->dwGBitMask,lpDDPixFmt->dwBBitMask);

	return DDENUMRET_OK;
}
