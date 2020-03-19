#include "Precomp.h"
#include "../Utils/VFile.h"
#include "NhtTarga.h"

NhtTarga::NhtTarga() : DIBData()
{
	InZero();
}

NhtTarga::NhtTarga(const char *pFileName) : DIBData()
{
	InZero();
	Load(pFileName);
}

NhtTarga::NhtTarga(FILE* hFile) : DIBData()
{
	InZero();
	Load(hFile);
}

NhtTarga::NhtTarga(LPBYTE pData,DWORD dwSize) : DIBData()
{
	InZero();
	Load(pData,dwSize);
}

NhtTarga::NhtTarga(VFile *pVFile) : DIBData()
{
	InZero();
	Load(pVFile);
}

NhtTarga::~NhtTarga()
{
	Release();
}

void NhtTarga::InZero(void)
{
	m_lpBMIH = NULL;
	m_lpImage = NULL;
	m_dwSizeImage = 0;
	m_dwStride = 0;
	m_lpIDField = NULL;
	m_IDLength = 0;
	m_bValid = FALSE;
}

void NhtTarga::Release(void)
{
	if(m_lpBMIH)
		delete[] m_lpBMIH;
	if(m_lpImage)
		delete[] m_lpImage;
	if(m_lpIDField)
		delete[] m_lpIDField;
	InZero();
}

BOOL NhtTarga::Load(const char *pFileName)
{
	FILE* hFile;

	if((hFile = fopen(pFileName,"rb"))!=NULL)
	{
		if(Load(hFile))
		{
			fclose(hFile);
			return TRUE;
		}
		fclose(hFile);
	}

	return FALSE;
}

BOOL NhtTarga::Load(FILE *hFile)
{
	LPBYTE pData = NULL;
	DWORD dwFileSize;

	//	получаем длину файла
	fseek(hFile,0L,SEEK_END);
	dwFileSize = ftell(hFile);
	rewind(hFile);
	//	обрабатываем данные
	pData = new BYTE[dwFileSize];
	if(pData)
	{
		//	считываем данные
		if(fread(pData,dwFileSize,1,hFile))
		{
			if(Load(pData,dwFileSize))		
			{
				delete[] pData;
				return TRUE;
			}
		}
		delete[] pData;
	}

	return FALSE;
}

BOOL NhtTarga::Load(LPBYTE pData,DWORD dwSize)
{
	VFile vFile(pData,dwSize);

	return Load(&vFile);
}

BOOL NhtTarga::Load(VFile *pVFile)
{
    TARGA_HEADER tga;

	Release();
	//	перемещаем указатель файла на начало
	pVFile->Rewind();
	// считываем заголовок TARGA_HEADER
	if(!pVFile->Read(&tga,sizeof(TARGA_HEADER)))
		return FALSE;
	//	Поддерживаемые форматы: 2
	if((0!= tga.ColorMapType) || 
       (tga.ImageType != 2))
		 return FALSE;
	//	Поддерживаемая глубина 16,24,32
	if((tga.PixelDepth != 16) &&
	   (tga.PixelDepth != 24) &&	
	   (tga.PixelDepth != 32))
		return FALSE;
    // Нам ведь нужно поле идентификации?
    if((m_IDLength = tga.IDLength))
	{
		if(!(m_lpIDField = new BYTE[m_IDLength]))
			return FALSE;
		if(!pVFile->Read(m_lpIDField,m_IDLength))
			return FALSE;
	}
	//	Выделяем память для структуры BITMAPINFOHEADER 
	m_lpBMIH = (LPBITMAPINFOHEADER) new char[sizeof(BITMAPINFOHEADER)];
	m_dwBitmapInfoSize = sizeof(BITMAPINFOHEADER);
	if(!m_lpBMIH)
		return FALSE;
	//	Обнуляем содержимое
	memset(m_lpBMIH,0,sizeof(BITMAPINFOHEADER));
	//	Заполняем структурку
	m_lpBMIH->biSize = sizeof(BITMAPINFOHEADER);
    m_lpBMIH->biWidth = tga.ImageWidth;
    m_lpBMIH->biHeight = tga.ImageHeight;
    m_lpBMIH->biBitCount = tga.PixelDepth==16?24:tga.PixelDepth;
    m_lpBMIH->biPlanes = 1;
	// расчитываем метрики
	ComputeMetrics();
	// выделяем память под битовый образ
	m_lpImage = (LPBYTE) new char[m_dwSizeImage];
	if(!m_lpImage)
		return FALSE;
	//	считываем данные
	switch(tga.ImageType)
	{
	case 2:
		switch(tga.PixelDepth)
		{
		case 16:
			if(!ReadUncompressedRGB16Data(pVFile,tga.ImageDescriptor&32))
				return FALSE;
			break;
		case 24:
		case 32:
			if(!ReadUncompressedRGBData(pVFile,tga.ImageDescriptor&32))
				return FALSE;
			break;
		}
		break;
	}

	return m_bValid = TRUE;
}

void NhtTarga::ComputeMetrics(void)
{
	// Подсчет количества двойных слов в необходимом битовом поле
	DWORD dwBytes = ((DWORD) m_lpBMIH->biWidth * m_lpBMIH->biBitCount) / 32;
	// Выравниваем край по 4 двойному слову
	if(((DWORD) m_lpBMIH->biWidth * m_lpBMIH->biBitCount) % 32)
	{
		dwBytes++;
	}
	dwBytes *= 4;
	m_dwStride = dwBytes;
	m_dwSizeImage = dwBytes * m_lpBMIH->biHeight; 
}

BOOL NhtTarga::ReadUncompressedRGBData(VFile *pVFile,bool LeftUpperCorner)
{
	LPBYTE lpByte = m_lpImage;
	DWORD dwLen = m_lpBMIH->biWidth*(m_lpBMIH->biBitCount==24?3:4);
	int dwStride = LeftUpperCorner?(-m_dwStride):m_dwStride;

	if(LeftUpperCorner)
		lpByte += m_dwStride*(m_lpBMIH->biHeight-1);
	for(int j=0;j<m_lpBMIH->biHeight;j++)
	{
		if(!pVFile->Read(lpByte,dwLen))
			return FALSE;
		lpByte  += dwStride;
	}

	return TRUE;
}

BOOL NhtTarga::ReadUncompressedRGB16Data(VFile *pVFile,bool LeftUpperCorner)
{
	LPBYTE lpByte = m_lpImage;
	LPBYTE lpRGB;
	WORD wPixel;
	int dwStride = LeftUpperCorner?(-m_dwStride):m_dwStride;

	if(LeftUpperCorner)
		lpByte += m_dwStride*(m_lpBMIH->biHeight-1);
	for(int j=0;j<m_lpBMIH->biHeight;j++)
	{
		lpRGB = lpByte;
		for(int i=0;i<m_lpBMIH->biWidth;i++)
		{
			if(!pVFile->Read(&wPixel,2))
				return FALSE;
			*lpRGB = (wPixel&0x1f)<<3;
			lpRGB++; 
			*lpRGB = ((wPixel>>5)&0x1f)<<3;
			lpRGB++; 
			*lpRGB = ((wPixel>>10)&0x1f)<<3;
			lpRGB++; 
		}
		lpByte  += dwStride;
	}

	return TRUE;
}

