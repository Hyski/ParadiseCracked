#include "Precomp.h"
#include "DIBData.h"

DIBData::DIBData()
{
	InZero();
}

DIBData::DIBData(const DIBData& image)
{
	InZero();
	Copy((DIBData *)&image);
}

DIBData::~DIBData()
{
	Release();
}

void DIBData::InZero(void)
{
	m_lpBMIH = NULL;
	m_lpImage = NULL;
	m_lpColorTable = NULL;
	m_dwSizeImage = 0;
	m_dwBitmapInfoSize = 0;
	m_dwStride = 0;
	m_bValid = FALSE;
}

void DIBData::Release(void)
{
	if(m_lpBMIH!=NULL)
		delete[] m_lpBMIH;
	if(m_lpImage!=NULL)
		delete[] m_lpImage;
	InZero();
}

BOOL DIBData::Draw(HDC hdc,int x,int y,DWORD dwRop)
{
	if(m_bValid)
	{
		return DIBData::StretchDIBits(hdc,x,y,
						m_lpBMIH->biWidth,m_lpBMIH->biHeight,
						0,0,m_lpBMIH->biWidth,m_lpBMIH->biHeight,
						dwRop);
	}

	return FALSE;
}


BOOL DIBData::StretchDIBits(HDC hdc,int XDest,int YDest,
									   int nDestWidth,int nDestHeight,
									   int XSrc,int YSrc,
									   int nSrcWidth,int nSrcHeight,
									   DWORD dwRop)
{
    if(!m_bValid || m_lpBMIH == NULL) 
		return FALSE;
    ::StretchDIBits(hdc,XDest,YDest,nDestWidth,nDestHeight,
        XSrc,YSrc,nSrcWidth,nSrcHeight,
        m_lpImage,(LPBITMAPINFO)m_lpBMIH,DIB_RGB_COLORS,dwRop);

    return TRUE;
}

void DIBData::ComputeMetrics(void)
{
	// ѕодсчет количества двойных слов в необходимом битовом поле
	DWORD dwBytes = ((DWORD) m_lpBMIH->biWidth * m_lpBMIH->biBitCount) / 32;
	// ¬ыравниваем край по 4 двойному слову
	if(((DWORD) m_lpBMIH->biWidth * m_lpBMIH->biBitCount) % 32)
	{
		dwBytes++;
	}
	dwBytes *= 4;
	m_dwStride = dwBytes;
	m_dwSizeImage = dwBytes * m_lpBMIH->biHeight; 
	// »нициализаци€ указател€ на таблицу цветов палитры
	m_lpColorTable = (LPBYTE) m_lpBMIH + m_lpBMIH->biSize;
}

BOOL DIBData::Copy(DIBData* pImage)
{
	if(pImage)
	{
		Release();
		if(pImage->IsValid())
		{
			m_lpBMIH = (LPBITMAPINFOHEADER) new char[pImage->m_dwBitmapInfoSize];
			m_lpImage = new BYTE[pImage->ImageSize()];
			if(m_lpBMIH && m_lpImage)
			{
				memcpy(m_lpBMIH,pImage->m_lpBMIH,pImage->m_dwBitmapInfoSize);
				memcpy(m_lpImage,pImage->Image(),pImage->ImageSize());
				if(pImage->m_lpColorTable)
					m_lpColorTable = (LPBYTE) m_lpBMIH + m_lpBMIH->biSize;
				m_dwBitmapInfoSize = pImage->m_dwBitmapInfoSize;
				m_dwSizeImage = pImage->m_dwSizeImage;
				m_dwStride = pImage->m_dwStride;
				m_bValid = TRUE;
			}
		}

		return TRUE;
	}

	return FALSE;
}

BOOL DIBData::Halve(void)
{
	BOOL bWidthNoExactly = FALSE;
	BOOL bHeightNoExactly = FALSE;
	DIBData image;

	if(m_bValid && (Width()>1 || Height()>1))
	{
		//	работаем только с True Color
		if((m_lpBMIH->biBitCount == 24) || (m_lpBMIH->biBitCount == 32))
		{
			image.Copy(this);
			Release();

			//	новые ширина и высота
			m_lpBMIH = (LPBITMAPINFOHEADER) new char[sizeof(BITMAPINFOHEADER)];
			if(m_lpBMIH)
			{
				m_dwBitmapInfoSize = sizeof(BITMAPINFOHEADER);
				//	ќбнул€ем содержимое
				memset(m_lpBMIH,0,sizeof(BITMAPINFOHEADER));
				//	«аполн€ем структурку
				m_lpBMIH->biSize = sizeof(BITMAPINFOHEADER);
				if((image.Width()%2)==0)
				{
					m_lpBMIH->biWidth = image.Width()/2;
				}
				else
				{
					m_lpBMIH->biWidth = (image.Width()/2)+1;
					bWidthNoExactly = TRUE;
				}
				if((image.Height()%2)==0)
				{
					m_lpBMIH->biHeight = image.Height()/2;
				}
				else
				{
					m_lpBMIH->biHeight = (image.Height()/2)+1;
					bHeightNoExactly = TRUE;
				}
				m_lpBMIH->biBitCount = image.Bpp();
				m_lpBMIH->biPlanes = 1;
				// расчитываем метрики
				ComputeMetrics();
				// выдел€ем пам€ть под битовый образ
				m_lpImage = (LPBYTE) new char[m_dwSizeImage];
				if(m_lpImage)
				{
					LPBYTE pSrc1 = image.Image();
					LPBYTE pSrc2 = image.Image()+image.Stride();
					LPBYTE pDest = m_lpImage;
					BYTE dwStep = (m_lpBMIH->biBitCount/8);

					for(int j=0;j<(((int)m_lpBMIH->biHeight)-1);j++)
					{
						StringHalve(pSrc1,pSrc2,pDest,image.Width(),dwStep,bWidthNoExactly);
						pSrc1 += (image.Stride()*2);
						pSrc2 += (image.Stride()*2);
						pDest += m_dwStride;
					}
					//	дорисовываем последнюю линию
					if(bHeightNoExactly)
						StringHalve(pSrc1,pSrc1,pDest,image.Width(),dwStep,bWidthNoExactly);
					else
						StringHalve(pSrc1,pSrc2,pDest,image.Width(),dwStep,bWidthNoExactly);

					return m_bValid = TRUE;
				}
			}
		}
	}

	return FALSE;
}

void DIBData::StringHalve(LPBYTE pSrc1,LPBYTE pSrc2,LPBYTE pDest,DWORD dwSrcLen,DWORD dwStep,BOOL bWidthNoExactly)
{
	int i,j;

	for(i=0;i<(((int)dwSrcLen)-2);i+=2)
	{
		for(j=0;j<dwStep;j++)
		{
			pDest[j] = (float)(pSrc1[j]+pSrc1[j+dwStep]+pSrc2[j]+pSrc2[j+dwStep])/4.0f;
		}
		pSrc1 += (dwStep*2);
		pSrc2 += (dwStep*2);
		pDest += dwStep;
	}
	//	дорисовываем последнюю точку
	if(bWidthNoExactly)
	{
		for(j=0;j<dwStep;j++)
		{
			pDest[j] = (float)(pSrc1[j]+pSrc1[j]+pSrc2[j]+pSrc2[j])/4.0f;
		}
	}
	else
	{
		for(j=0;j<dwStep;j++)
		{
			pDest[j] = (float)(pSrc1[j]+pSrc1[j+dwStep]+pSrc2[j]+pSrc2[j+dwStep])/4.0f;
		}
	}
}

BOOL DIBData::Save(const char *pFileName)
{
	BITMAPFILEHEADER bmfh = {0x4d42,0,0,0,sizeof(BITMAPFILEHEADER)+m_dwBitmapInfoSize};
	FILE *hFile;

	if(pFileName && m_bValid)
	{
		if((hFile = fopen(pFileName,"wb"))!=NULL)
		{
			//	записываем информацию в файл
			fwrite(&bmfh,sizeof(BITMAPFILEHEADER),1,hFile);
			fwrite(m_lpBMIH,sizeof(BITMAPINFOHEADER),1,hFile);
			if(m_lpColorTable)
			{
				fwrite(m_lpColorTable,m_dwBitmapInfoSize-m_lpBMIH->biSize,1,hFile);
			}
			fwrite(m_lpImage,m_dwSizeImage,1,hFile);			
			fclose(hFile);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL DIBData::Create(int width,int height,int bpp)
{
	//	очищаем переменные
	Release();
	//	выдел€ем пам€ть под загловок
	m_lpBMIH = (LPBITMAPINFOHEADER) new char[sizeof(BITMAPINFOHEADER)];
	if(!m_lpBMIH)
		return FALSE;
	//	заносим размер пам€ти под m_lpBMIH
	m_dwBitmapInfoSize = sizeof(BITMAPINFOHEADER);
	//	oбнул€ем содержимое
	memset(m_lpBMIH,0,sizeof(BITMAPINFOHEADER));
	//	заполн€ем заголовок
	m_lpBMIH->biSize = sizeof(BITMAPINFOHEADER);
	m_lpBMIH->biWidth = width;
	m_lpBMIH->biHeight = height;
	m_lpBMIH->biBitCount = bpp;
	m_lpBMIH->biPlanes = 1;
	//	подсчитываем метрики
	ComputeMetrics();
	//	выдел€ем пам€ть под битовый образ
	m_lpImage = new BYTE[m_dwSizeImage];
	if(!m_lpImage)
		return FALSE;
	
	return m_bValid = TRUE;
}

BOOL DIBData::Create(int width,int height,int bpp,LPBYTE pImage)
{
	//	очищаем переменные
	Release();
	//	выдел€ем пам€ть под загловок
	m_lpBMIH = (LPBITMAPINFOHEADER) new char[sizeof(BITMAPINFOHEADER)];
	if(!m_lpBMIH)
		return FALSE;
	//	заносим размер пам€ти под m_lpBMIH
	m_dwBitmapInfoSize = sizeof(BITMAPINFOHEADER);
	//	oбнул€ем содержимое
	memset(m_lpBMIH,0,sizeof(BITMAPINFOHEADER));
	//	заполн€ем заголовок
	m_lpBMIH->biSize = sizeof(BITMAPINFOHEADER);
	m_lpBMIH->biWidth = width;
	m_lpBMIH->biHeight = height;
	m_lpBMIH->biBitCount = bpp;
	m_lpBMIH->biPlanes = 1;
	//	подсчитываем метрики
	ComputeMetrics();
	//	выдел€ем пам€ть под битовый образ
	m_lpImage = new BYTE[m_dwSizeImage];
	if(!m_lpImage)
		return FALSE;
	memcpy(m_lpImage,pImage,m_dwSizeImage);
	
	return m_bValid = TRUE;
}

BOOL DIBData::Create(DIBData* pImage,int width,int height)
{
	if(pImage)
	{
		Release();
		if(pImage->IsValid())
		{
			m_lpBMIH = (LPBITMAPINFOHEADER) new char[pImage->m_dwBitmapInfoSize];
			if(m_lpBMIH)
			{
				memcpy(m_lpBMIH,pImage->m_lpBMIH,pImage->m_dwBitmapInfoSize);
				//	измен€ем размеры
				m_lpBMIH->biWidth = width;
				m_lpBMIH->biHeight = height;
				//	рассчитываем метрики
				ComputeMetrics();
				//	выдел€ем пам€ть под битовый образ
				m_lpImage = new BYTE[m_dwSizeImage];
				if(m_lpImage)
				{
					m_dwBitmapInfoSize = pImage->m_dwBitmapInfoSize;
					return m_bValid = TRUE;
				}
			}
		}
	}

	return m_bValid = FALSE;
}

DWORD DIBData::GetPixel(unsigned int x,unsigned int y)
{
	DWORD dwPixel = 0;

	if(m_bValid && 
	   (m_lpBMIH->biBitCount==24 || m_lpBMIH->biBitCount==32) && 
	   (x<m_lpBMIH->biWidth && y<m_lpBMIH->biHeight))
	{
		y = (m_lpBMIH->biHeight-1-y)*m_dwStride;
		x = x*(m_lpBMIH->biBitCount>>3);

		dwPixel += (*(m_lpImage+y+x))<<0;		//	b
		dwPixel += (*(m_lpImage+y+x+1))<<8;		//	g
		dwPixel += (*(m_lpImage+y+x+2))<<16;	//	r
		if(m_lpBMIH->biBitCount==32)
			dwPixel += *(m_lpImage+y+x+3)<<24;	//	a
	}

	return dwPixel;
}

void DIBData::SetPixel(unsigned int x,unsigned int y,DWORD dwPixel)
{
	if(m_bValid && 
	   (m_lpBMIH->biBitCount==24 || m_lpBMIH->biBitCount==32) && 
	   (x<m_lpBMIH->biWidth && y<m_lpBMIH->biHeight))
	{
		y = (m_lpBMIH->biHeight-1-y)*m_dwStride;
		x = x*(m_lpBMIH->biBitCount>>3);

		*(m_lpImage+y+x) = dwPixel&0xff;			//	b
		*(m_lpImage+y+x+1) = (dwPixel>>8)&0xff;		//	g
		*(m_lpImage+y+x+2) = (dwPixel>>16)&0xff;	//	r
		if(m_lpBMIH->biBitCount==32)
			*(m_lpImage+y+x+3) = (dwPixel>>24);		//	a
	}
}
BOOL DIBData::Save(FILE *f)
  {
  int i;
  i=Width();
  fwrite(&i,sizeof(int),1,f);
  i=Height();
  fwrite(&i,sizeof(int),1,f);
  i=Bpp();
  fwrite(&i,sizeof(int),1,f);
  i=ImageSize();
  fwrite(&i,sizeof(int),1,f);
  i=ImageSize();
  fwrite(Image(),i,1,f);
  return 1;
  }
BOOL DIBData::Load(FILE *f)
  {
  int x,y,bpp,isize;
  fread(&x,sizeof(int),1,f);
  fread(&y,sizeof(int),1,f);
  fread(&bpp,sizeof(int),1,f);
  Create(x,y,bpp);
  fread(&isize,sizeof(int),1,f);
  fread(Image(),isize,1,f);
  return 1;
  }

