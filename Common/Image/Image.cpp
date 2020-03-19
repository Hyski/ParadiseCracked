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
#include "Image.h"

Image::Image() : m_PixelFormat(new ic_PixelFormat()),m_BitImage(0),m_ColorMap(0),m_Size(0),m_Stride(0),m_Width(0),m_Height(0)
{
}

Image::Image(const Image& image) : m_PixelFormat(new ic_PixelFormat(*image.m_PixelFormat)),m_BitImage(0),m_ColorMap(0)
{
	m_Size   = image.m_Size;
	m_Stride = image.m_Stride;
	m_Width  = image.m_Width;
	m_Height = image.m_Height;

	if(image.m_BitImage)
	{
		m_BitImage = new byte[m_Size];
		memcpy(m_BitImage,image.m_BitImage,m_Size);
	}
	if(image.m_ColorMap)
	{
		m_ColorMap = new ic_ColorMap(*image.m_ColorMap);
	}
}

Image::~Image()
{
	Release();
	delete m_PixelFormat;
}
//	сбросить все данные
void Image::Release(void)
{
	if(m_BitImage)
	{
		delete[] m_BitImage;
		m_BitImage = 0;
	}
	if(m_ColorMap)
	{
		delete m_ColorMap;
		m_ColorMap = 0;
	}
	m_Size = m_Stride = 0;
	m_Width = m_Height = 0;
}
//	оператор копирования
const Image& Image::operator=(const Image& image)
{
	//	освобождаем предыдущие ресурсы
	if(m_BitImage) Release();
	if(image.m_BitImage)
	{
		//	копируем формат пикселя
		*m_PixelFormat = *image.m_PixelFormat;
		//	копируем палитру
		if(image.m_ColorMap)
		{ 
			m_ColorMap = new ic_ColorMap(*image.m_ColorMap);
		}
		//	копируем параметры
		m_Size   = image.m_Size;								
		m_Stride = image.m_Stride;								
		m_Width  = image.m_Width;								
		m_Height = image.m_Height;								
		//	копируем битовый образ
		m_BitImage = new unsigned char[m_Size];
		memcpy(m_BitImage,image.m_BitImage,m_Size);
	}

	return (*this);
}
//	создать пустое изображение заданного размера с определенным пиксель форматом и палитрой
bool Image::Create(int width,int height,const ic_PixelFormat* pf,const ic_ColorMap* color_map,const unsigned char* bit_image)
{
	if(m_BitImage) Release();
	//	поехали
	if(pf && width && height)
	{
		//	копируем пиксель формат
		*m_PixelFormat = *pf;
		//	копируем параметры
		m_Width  = width;								
		m_Height = height;								
		//	рассчитываем метрики
		EstimateImageSize();
		//	создаем битовый образ
		m_BitImage = new unsigned char[m_Size];
		if(bit_image)
		{
			memcpy(m_BitImage,bit_image,m_Size);
		}
		//	создаем палитру
		if(color_map)
		{
			m_ColorMap = new ic_ColorMap(*color_map);
		}
		
		return true;
	}

	return false;
}
//	уменьшить картинку вчетверо
bool Image::Quad(void)
{
	if(m_BitImage && m_Width && m_Height && (m_Width>1 || m_Height>1) && 
	   ((m_PixelFormat->m_BitPerPixel==8)  || 
		(m_PixelFormat->m_BitPerPixel==16) || 
		(m_PixelFormat->m_BitPerPixel==24) || 
		(m_PixelFormat->m_BitPerPixel==32)))
	{
		Image image_copy(*this);
		int out_width  = ((m_Width&0x1)?(m_Width+1):m_Width)>>1;
		int out_height = ((m_Height&0x1)?(m_Height+1):m_Height)>>1;
		
		//	создаем новое изображение
		Create(out_width,out_height,image_copy.m_PixelFormat,image_copy.m_ColorMap);
		if(m_PixelFormat->m_BitPerPixel == 8)
		{
			QuadPalette(image_copy);
		}
		else
		{
			QuadTrueColor(image_copy);
		}

		return true;
	}

	return false;
}
//	уменьшить картинку вчетверо (для палитры) 
void Image::QuadPalette(const Image& src_image)
{
	int out_width  = Width();
	int out_height = Height();

	unsigned char* src = src_image.m_BitImage;
	unsigned char* dst = m_BitImage;

	for(int i=0;i<out_height;i++)
	{
		unsigned char* in_src = src;
		unsigned char* in_dst = dst;
		
		for(int j=0;j<out_width;j++)
		{
			(*in_dst) = (*in_src);
			in_src++;
			in_src++;
			in_dst++; 
		}
		src += src_image.m_Stride<<1;
		dst += m_Stride;
	}
}
//	уменьшить картинку вчетверо (для ARGB) 
void Image::QuadTrueColor(const Image& src_image)
{
	int odd_width  = src_image.Width()&0x1;
	int odd_height = src_image.Height()&0x1;

	int out_height = Height()-odd_height;

	unsigned char* src1 = src_image.m_BitImage;
	unsigned char* src2 = src_image.m_BitImage+src_image.m_Stride;
	unsigned char* dest = m_BitImage;

	//	рассчитываем вспомогательные параметры
	unsigned int a_shift_r = m_PixelFormat->GetShiftR(m_PixelFormat->m_ABitMask);
	unsigned int r_shift_r = m_PixelFormat->GetShiftR(m_PixelFormat->m_RBitMask);
	unsigned int g_shift_r = m_PixelFormat->GetShiftR(m_PixelFormat->m_GBitMask);
	unsigned int b_shift_r = m_PixelFormat->GetShiftR(m_PixelFormat->m_BBitMask);
	unsigned int a,r,g,b;
	unsigned int pixel = 0;
	
	for(;odd_height>=0;odd_height--)
	{
		for(int i=0;i<out_height;i++)
		{
			unsigned char* in_src1 = src1;
			unsigned char* in_src2 = src2;
			unsigned char* in_dest = dest;

			int len = m_PixelFormat->m_BitPerPixel>>3;

			int out_width = Width()-odd_width;

			for(int l=odd_width;l>=0;l--)
			{
				for(int j=0;j<out_width;j++)
				{
					a = r = g = b = 0;
					
					for(int k=0;k<2;k++)
					{
						switch(len)
						{
						case 2:
							pixel = *((unsigned short*)in_src1);
							break;
						case 3:
							pixel = (*((unsigned short*)in_src1))+(*(in_src1+2)<<16);
							break;
						case 4:
							pixel = *((unsigned int*)in_src1);
							break;
						}
						
						a += (pixel&m_PixelFormat->m_ABitMask)>>a_shift_r;
						r += (pixel&m_PixelFormat->m_RBitMask)>>r_shift_r;
						g += (pixel&m_PixelFormat->m_GBitMask)>>g_shift_r;
						b += (pixel&m_PixelFormat->m_BBitMask)>>b_shift_r;
						in_src1 += len;
						
						switch(len)
						{
						case 2:
							pixel = *((unsigned short*)in_src2);
							break;
						case 3:
							pixel = (*((unsigned short*)in_src2))+(*(in_src2+2)<<16);
							break;
						case 4:
							pixel = *((unsigned int*)in_src2);
							break;
						}
						
						a += (pixel&m_PixelFormat->m_ABitMask)>>a_shift_r;
						r += (pixel&m_PixelFormat->m_RBitMask)>>r_shift_r;
						g += (pixel&m_PixelFormat->m_GBitMask)>>g_shift_r;
						b += (pixel&m_PixelFormat->m_BBitMask)>>b_shift_r;
						in_src2 += len;
					}
					
					a >>= 2;
					r >>= 2;
					g >>= 2;
					b >>= 2;
					
					pixel = (a<<a_shift_r)+(r<<r_shift_r)+(g<<g_shift_r)+(b<<b_shift_r);
					
					switch(m_PixelFormat->m_BitPerPixel)
					{
					case 16:
						*((unsigned short*)in_dest) = (unsigned short)pixel;
						in_dest += 2; 
						break;
					case 24:
						*in_dest = pixel;
						*(in_dest+1) = pixel>>8;
						*(in_dest+2) = pixel>>16;
						in_dest += 3; 
						break;
					case 32:
						*((unsigned int*)in_dest) = pixel;
						in_dest += 4; 
						break;
					}
				}
				len = 0;
				out_width = 1;
			}
			src1 += src_image.m_Stride<<1;
			src2 += src_image.m_Stride<<1;
			dest += m_Stride;
		}
		src2 = src1;
		out_height = 1;
	}
}
//	рассчитываем размеры массивов данных
void Image::EstimateImageSize(void)
{
	// Подсчет количества двойных слов в необходимом битовом поле
	unsigned int bytes = static_cast<unsigned int>(m_Width * m_PixelFormat->m_BitPerPixel)/32;
	// Выравниваем край по 4 двойному слову
	if(static_cast<unsigned int>(m_Width * m_PixelFormat->m_BitPerPixel)%32)
	{
		bytes++;
	}
	bytes *= 4;
	m_Stride = bytes;
	m_Size = bytes * m_Height; 
}

//**********************************************************************
//	class Image::ic_PixelFormat
Image::ic_PixelFormat::ic_PixelFormat()
{
	m_BitPerPixel = 0;
	m_ABitMask = m_RBitMask = m_GBitMask = m_BBitMask = 0;
}
//	конструктор копирования
Image::ic_PixelFormat::ic_PixelFormat(const ic_PixelFormat& pf)
{
	m_BitPerPixel = pf.m_BitPerPixel;
	m_ABitMask = pf.m_ABitMask;
	m_RBitMask = pf.m_RBitMask;
	m_GBitMask = pf.m_GBitMask;
	m_BBitMask = pf.m_BBitMask;
}
//	перегруженный конструктор
Image::ic_PixelFormat::ic_PixelFormat(unsigned int bpp)
{
	m_BitPerPixel = bpp;
	switch(bpp)
	{
	case 16:
		Set(bpp,T_ARGB555);
		break;
	case 24:
		Set(bpp,T_RGB888);
		break;
	case 32:
		Set(bpp,T_ARGB888);
		break;
	}
}
//	перегруженный конструктор
Image::ic_PixelFormat::ic_PixelFormat(unsigned int bpp,TYPE type)
{
	Set(bpp,type);
}
//	перегруженный конструктор
Image::ic_PixelFormat::ic_PixelFormat(unsigned int bpp,
									  unsigned int abitmask,
									  unsigned int rbitmask,
									  unsigned int gbitmask,
									  unsigned int bbitmask)
{
	m_BitPerPixel = bpp;
	m_ABitMask = abitmask;
	m_RBitMask = rbitmask;
	m_GBitMask = gbitmask;
	m_BBitMask = bbitmask;
}

Image::ic_PixelFormat::~ic_PixelFormat()
{
}
//	оператор копирования
const Image::ic_PixelFormat& Image::ic_PixelFormat::operator=(const ic_PixelFormat& pf)
{
	m_BitPerPixel = pf.m_BitPerPixel;
	m_ABitMask = pf.m_ABitMask;
	m_RBitMask = pf.m_RBitMask;
	m_GBitMask = pf.m_GBitMask;
	m_BBitMask = pf.m_BBitMask;
	
	return (*this);
}
//	установить пиксель формат
void Image::ic_PixelFormat::Set(unsigned int bpp,TYPE type)
{
	m_BitPerPixel = bpp;
	switch(type)
	{
	case T_ARGB888:
		m_ABitMask = 0xff000000;
		m_RBitMask = 0x00ff0000;
		m_GBitMask = 0x0000ff00;
		m_BBitMask = 0x000000ff;
		break;
	case T_ARGB444:
		m_ABitMask = 0x0000f000;
		m_RBitMask = 0x00000f00;
		m_GBitMask = 0x000000f0;
		m_BBitMask = 0x0000000f;
		break;
	case T_ARGB555:
		m_ABitMask = 0x00008000;
		m_RBitMask = 0x00007c00;
		m_GBitMask = 0x000003e0;
		m_BBitMask = 0x0000001f;
		break;
	case T_RGB888:
		m_ABitMask = 0x00000000;
		m_RBitMask = 0x00ff0000;
		m_GBitMask = 0x0000ff00;
		m_BBitMask = 0x000000ff;
		break;
	case T_RGB565:
		m_ABitMask = 0x00000000;
		m_RBitMask = 0x0000f800;
		m_GBitMask = 0x000007e0;
		m_BBitMask = 0x0000001f;
		break;
	case T_RGB555:
		m_ABitMask = 0x00000000;
		m_RBitMask = 0x00007c00;
		m_GBitMask = 0x000003e0;
		m_BBitMask = 0x0000001f;
		break;
	default:
		m_ABitMask = m_RBitMask = m_GBitMask = m_BBitMask = 0;
	}
}
//	получить пиксель формат
Image::ic_PixelFormat::TYPE Image::ic_PixelFormat::Get(void) const
{
	switch(m_BitPerPixel)
	{
	case 16:
		if(m_ABitMask == 0x00000000 && m_RBitMask == 0x00007c00 && m_GBitMask == 0x000003e0 && m_BBitMask == 0x0000001f) return T_RGB555;
		if(m_ABitMask == 0x00000000 && m_RBitMask == 0x0000f800 && m_GBitMask == 0x000007e0 && m_BBitMask == 0x0000001f) return T_RGB565;
		if(m_ABitMask == 0x00008000 && m_RBitMask == 0x00007c00 && m_GBitMask == 0x000003e0 && m_BBitMask == 0x0000001f) return T_ARGB555;
		if(m_ABitMask == 0x0000f000 && m_RBitMask == 0x00000f00 && m_GBitMask == 0x000000f0 && m_BBitMask == 0x0000000f) return T_ARGB444;
		break;
	case 24:
		if(m_ABitMask == 0x00000000 && m_RBitMask == 0x00ff0000 && m_GBitMask == 0x0000ff00 && m_BBitMask == 0x000000ff) return T_RGB888;
		break;
	case 32:
		if(m_ABitMask == 0xff000000 && m_RBitMask == 0x00ff0000 && m_GBitMask == 0x0000ff00 && m_BBitMask == 0x000000ff) return T_ARGB888;
		break;
	}

	return T_NONE;
}

//**********************************************************************
//	class Image::ic_ColorMap
Image::ic_ColorMap::ic_ColorMap() : m_PixelFormat(new ic_PixelFormat()),m_Map(0),m_Size(0)
{
}

Image::ic_ColorMap::ic_ColorMap(const ic_ColorMap& cm) : m_PixelFormat(new ic_PixelFormat((*cm.m_PixelFormat)))
{
	m_Map = new unsigned char[cm.m_Size];
	memcpy(m_Map,cm.m_Map,cm.m_Size);
	m_Size = cm.m_Size;
}

Image::ic_ColorMap::ic_ColorMap(const Image::ic_PixelFormat& pf,const unsigned char* data,int size) : m_PixelFormat(new ic_PixelFormat()),m_Map(0),m_Size(0)
{
	Set(pf,data,size);
}

Image::ic_ColorMap::~ic_ColorMap()
{
	if(m_Map) delete[] m_Map;
	delete m_PixelFormat;
}
//	оператор копирования
const Image::ic_ColorMap& Image::ic_ColorMap::operator=(const ic_ColorMap& cm)
{
	Set(*cm.m_PixelFormat,cm.m_Map,cm.m_Size);

	return (*this);
}

void Image::ic_ColorMap::Set(const Image::ic_PixelFormat& pf,const unsigned char* data,int size)
{
	(*m_PixelFormat) = pf;
	if(data && size)
	{
		if(m_Map) delete[] m_Map;
		m_Size = size;
		m_Map = new unsigned char[m_Size];
		memcpy(m_Map,data,m_Size);
	}
}
