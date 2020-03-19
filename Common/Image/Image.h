/***********************************************************************

                             Texture Manager

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.05.2001

************************************************************************/
#ifndef _IMAGE_H_
#define _IMAGE_H_

//**********************************************************************//
//	class Image
class Image
{
public:
	class ic_PixelFormat;
	class ic_ColorMap;
protected:
	ic_PixelFormat* m_PixelFormat;
	ic_ColorMap* m_ColorMap;				//	палитра
	unsigned char* m_BitImage;				//	битовый образ
	int m_Size;								//	размер битового образа в байтах
	int m_Stride;							//	кол-во байт в строке
	int m_Width;							//	ширина в пикселях
	int m_Height;							//	высота в пикселях
public:
	Image();
	Image(const Image& image);
	virtual ~Image();
public:
	//	оператор копирования
	const Image& operator=(const Image& image);
	//	создать пустое изображение заданного размера с определенным пиксель форматом и палитрой
	bool Create(int width,int height,const ic_PixelFormat* pf,const ic_ColorMap* color_map = 0,const unsigned char* bit_image = 0);
	//	уменьшить картинку вчетверо
	bool Quad(void);
	//	сбросить все данные
	void Release(void);
protected:
	//	рассчитываем размеры массивов данных
	void EstimateImageSize(void);
	//	уменьшить картинку вчетверо (для палитры) 
	void QuadPalette(const Image& src_image);
	//	уменьшить картинку вчетверо (для ARGB) 
	void QuadTrueColor(const Image& src_image);
public:
	//	получить информацию о формате пикселов
	const ic_PixelFormat* PixelFormat(void) const;
	//	получить информацию о палитре
	const ic_ColorMap* ColorMap(void) const;
	//	получить доступ к битовому образу
	unsigned char* BitImage(void) const;
	//	получить размер битового образа
	int Size(void) const;
	//	получить длину строку в байтах
	int Stride(void) const;
	//	получить ширину изображения в пикселах
	int Width(void) const;
	//	получить высоту изображения в пикселах
	int Height(void) const;
};

//	получить информацию о формате пикселов
inline const Image::ic_PixelFormat* Image::PixelFormat(void) const
{
	return m_PixelFormat;
}
//	получить информацию о палитре
inline const Image::ic_ColorMap* Image::ColorMap(void) const
{
	return m_ColorMap;
}
//	получить доступ к битовому образу
inline unsigned char* Image::BitImage(void) const
{
	return m_BitImage;
}
//	получить размер битового образа
inline int Image::Size(void) const
{
	return m_Size;
}
//	получить длину строку в байтах
inline int Image::Stride(void) const
{
	return m_Stride;
}
//	получить ширину изображения в пикселах
inline int Image::Width(void) const
{
	return m_Width;
}
//	получить высоту изображения в пикселах
inline int Image::Height(void) const
{
	return m_Height;
}

//**********************************************************************
//	class Image::ic_PixelFormat
class Image::ic_PixelFormat
{
public:
	enum TYPE
	{
		T_ARGB888,
		T_ARGB444,
		T_ARGB555,
		T_RGB888,
		T_RGB565,
		T_RGB555,
		T_NONE
	};
public:
	unsigned int m_BitPerPixel;
	unsigned int m_ABitMask;
	unsigned int m_RBitMask;
	unsigned int m_GBitMask;
	unsigned int m_BBitMask;
public:
	ic_PixelFormat();
	ic_PixelFormat(const ic_PixelFormat& pf);
	ic_PixelFormat(unsigned int bpp);
	ic_PixelFormat(unsigned int bpp,TYPE type);
	ic_PixelFormat(unsigned int bpp,
				   unsigned int abitmask,
				   unsigned int rbitmask,
				   unsigned int gbitmask,
				   unsigned int bbitmask);
	virtual ~ic_PixelFormat();
public:
	//	оператор копирования
	const ic_PixelFormat& operator=(const ic_PixelFormat& pf);
	//	установить пиксель формат
	void Set(unsigned int bpp,TYPE type);
	//	получить пиксель формат
	TYPE Get(void) const;
	//	получить кол-во бит в компоненте
	int BitCount(unsigned int mask) const;
	//	получить смещение вправо до нуля
	int GetShiftR(unsigned int mask) const;
};

//	получить кол-во бит в компоненте
inline int Image::ic_PixelFormat::BitCount(unsigned int mask) const
{
	int counter = 0;
	for(;mask && !(mask&0x1);mask>>=1);
	for(;mask;mask>>=1) counter++; 
	return counter;
}
//	получить смещение вправо до нуля
inline int Image::ic_PixelFormat::GetShiftR(unsigned int mask) const
{
	int counter = 0;
	for(;mask && !(mask&0x1);mask>>=1) counter++;
	return counter;
}

inline bool operator == (const Image::ic_PixelFormat& a,const Image::ic_PixelFormat& b)
{
	return (a.m_BitPerPixel == b.m_BitPerPixel) &&
	       (a.m_ABitMask == b.m_ABitMask) &&
	       (a.m_RBitMask == b.m_RBitMask) &&
	       (a.m_GBitMask == b.m_GBitMask) &&
	       (a.m_BBitMask == b.m_BBitMask);
}
//**********************************************************************
//	class Image::ic_ColorMap
class Image::ic_ColorMap
{
public:
	Image::ic_PixelFormat* m_PixelFormat;	//	формат палитры
	unsigned char* m_Map;					//	палитра для палитрованных картинок
	int m_Size;								//	размер буфера палитры
public:
	ic_ColorMap();
	ic_ColorMap(const ic_ColorMap& cm);
	ic_ColorMap(const Image::ic_PixelFormat& pf,const unsigned char* data,int size);
	virtual ~ic_ColorMap();
public:
	//	оператор копирования
	const ic_ColorMap& operator=(const ic_ColorMap& cm);
	//	установить палитру
	void Set(const Image::ic_PixelFormat& pf,const unsigned char* data,int size);
public:
	//	получить пиксель формат
	const ic_PixelFormat* PixelFormat(void) const;
	//	получить карту цветов
	const unsigned char* Map(void) const;
	//	получить размер карты цветов
	int Size(void) const;
};

//	получить пиксель формат
inline const Image::ic_PixelFormat* Image::ic_ColorMap::PixelFormat(void) const
{
	return m_PixelFormat;
}
//	получить карту цветов
inline const unsigned char* Image::ic_ColorMap::Map(void) const
{
	return m_Map;
}
//	получить размер карты цветов
inline int Image::ic_ColorMap::Size(void) const
{
	return m_Size;
}

#endif	//_IMAGE_H_