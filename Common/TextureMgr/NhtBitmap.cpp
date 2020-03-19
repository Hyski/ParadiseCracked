#include "Precomp.h"
#include "../Utils/VFile.h"
#include "NhtBitmap.h"

NhtBitmap::NhtBitmap() : DIBData()
{
	InZero();
}

NhtBitmap::NhtBitmap(const char *pFileName) : DIBData()
{
	InZero();
	Load(pFileName);
}

NhtBitmap::NhtBitmap(FILE* hFile) : DIBData()
{
	InZero();
	Load(hFile);
}

NhtBitmap::NhtBitmap(LPBYTE pData,DWORD dwSize) : DIBData()
{
	InZero();
	Load(pData,dwSize);
}

NhtBitmap::NhtBitmap(VFile *pVFile) : DIBData()
{
	InZero();
	Load(pVFile);
}

NhtBitmap::~NhtBitmap()
{
	Release();
}

BOOL NhtBitmap::Load(const char *pFileName)
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

BOOL NhtBitmap::Load(FILE *hFile)
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

BOOL NhtBitmap::Load(LPBYTE pData,DWORD dwSize)
{
	VFile vFile(pData,dwSize);

	return Load(&vFile);
}

BOOL NhtBitmap::Load(VFile *pVFile)
{
	BITMAPFILEHEADER bmfh;

	Release();
	//	перемещаем указатель файла на начало
	pVFile->Rewind();
	// считываем заголовок BITMAPFILEHEADER
	if(!pVFile->Read(&bmfh,sizeof(BITMAPFILEHEADER)))
		return FALSE;
	if(bmfh.bfType != 0x4d42)  // буковки BM
		return FALSE;
	// размер структуры BITMAPINFOHEADER и таблицы цветов
	m_dwBitmapInfoSize = bmfh.bfOffBits - sizeof(BITMAPFILEHEADER);
	// выделяем память для структуры BITMAPINFOHEADER и таблицы цветов
	m_lpBMIH = (LPBITMAPINFOHEADER) new char[m_dwBitmapInfoSize];
	if(!m_lpBMIH)
		return FALSE;
	// считываем из файла структуру BITMAPINFOHEADER и таблицу цветов
	if(!pVFile->Read(m_lpBMIH,m_dwBitmapInfoSize))
		return FALSE;
	// проверка на компрессию
	if(m_lpBMIH->biSize>16)
		if(m_lpBMIH->biCompression!=0)
			return FALSE;
	// расчитываем метрики
	ComputeMetrics();
	// выделяем память под битовый образ
	m_lpImage = (LPBYTE) new char[m_dwSizeImage];
	if(!m_lpImage)
		return FALSE;
	// считываем битовый образ из файла
	if(!pVFile->Read(m_lpImage,m_dwSizeImage))
		return FALSE;

	return m_bValid = TRUE;
}
/*
void NhtBitmap::ComputeMetrics(void)
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
	// Инициализация указателя на таблицу цветов палитры
	m_lpColorTable = (LPBYTE) m_lpBMIH + m_lpBMIH->biSize;
}
*/
