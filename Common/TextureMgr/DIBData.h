/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
    Date:   16.05.2000

************************************************************************/
#ifndef _DIB_DATA_H_
#define _DIB_DATA_H_

class DIBData
{
protected:
	BOOL m_bValid;					//	действительна ли картинка
protected:	
	LPBITMAPINFOHEADER m_lpBMIH;	//	Заголовок битового образа
	LPBYTE m_lpImage;				//	Указатель на начало битового образа в памяти
	LPVOID m_lpColorTable;			//	Указатель на таблицу цветов
	DWORD m_dwSizeImage;			//	Размер битового образа в байтах
	DWORD m_dwBitmapInfoSize;		//	Размер BITMAPINFOHEADER + ColorTable
	DWORD m_dwStride;				//	кол-во байт в строке
public:
	DIBData();
	DIBData(const DIBData& image);
	virtual ~DIBData();
public:	//	params
	inline BOOL IsValid(void);
	inline DWORD Width(void);
	inline DWORD Height(void);
	inline LPBYTE Image(void);
	inline DWORD Stride(void);
	inline DWORD ImageSize(void);
	inline DWORD Bpp(void);
public:	//	draw
	BOOL StretchDIBits(HDC hdc,int XDest,int YDest,
							   int nDestWidth,int nDestHeight,
							   int XSrc,int YSrc,
							   int nSrcWidth,int nSrcHeight,
							   DWORD dwRop = SRCCOPY);
	BOOL Draw(HDC hdc,int x=0,int y=0,DWORD dwRop=SRCCOPY);
public:	//	другие возможности
	//	копируем содиржимое другого класса в данный
	BOOL Copy(DIBData* pImage);
	//	уменьшаем оригинал в два раза по высоте и ширине
	BOOL Halve(void);
	//	создание пустого образа
	BOOL Create(int width,int height,int bpp);
	//	создание образа из внешнего битового образа
	BOOL Create(int width,int height,int bpp,LPBYTE pImage);
	//	создание образа с параметрами взятыми из другого образа
	BOOL Create(DIBData* pImage,int width,int height);
	//	получение значения пиксела в точке (x,y) только для 24,32 битных форматов
	DWORD GetPixel(unsigned int x,unsigned int y);	//	ARGB формат
	//	занесение значения пиксела в точку (x,y) только для 24,32 битных форматов
	void SetPixel(unsigned int x,unsigned int y,DWORD dwPixel);	//	ARGB формат
	//	запись на диск в формате BMP
	virtual BOOL Save(const char *pFileName);
	//	запись в открытый файл
	virtual BOOL Save(FILE *f);
  //чтение из открытого файла
	virtual BOOL Load(FILE *f);
protected:
	virtual void InZero(void);
	virtual void ComputeMetrics(void);
private:
	void StringHalve(LPBYTE pSrc1,LPBYTE pSrc2,LPBYTE pDest,DWORD dwSrcLen,DWORD dwStep,BOOL bWidthNoExactly);
public:
	virtual void Release(void);

};

BOOL DIBData::IsValid(void)
{
	return m_bValid;
}

DWORD DIBData::Width(void)
{
	if(m_lpBMIH)
		return m_lpBMIH->biWidth;
	return 0;
}

DWORD DIBData::Height(void)
{
	if(m_lpBMIH)
		return m_lpBMIH->biHeight;
	return 0;
}

LPBYTE DIBData::Image(void)
{
	return m_lpImage;
}

DWORD DIBData::Stride(void)
{
	return m_dwStride;
}

DWORD DIBData::ImageSize(void)
{
	return m_dwSizeImage;
}

DWORD DIBData::Bpp(void)
{
	if(m_lpBMIH)
		return m_lpBMIH->biBitCount;
	return 0;
}

#endif