/***********************************************************************

                                Image Test

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.05.2001

************************************************************************/
#include "Precomp.h"
#include "Bitmap.h"

Bitmap::Bitmap() : Image()
{
}

Bitmap::Bitmap(const Bitmap& image) : Image(image)
{
}

Bitmap::Bitmap(const Image& image) : Image(image)
{
}

Bitmap::~Bitmap()
{
}
//	оператор копирования
const Bitmap& Bitmap::operator=(const Image& image)
{
	(*this).Image::operator=(image);

	return (*this);
}

bool Bitmap::Load(const char* file_name)
{
	FILE* hFile;
	int length;
	unsigned char* buff;
	bool ok = false;

	if(file_name && (*file_name))
	{
		if(hFile = fopen(file_name,"rb"))
		{
			fseek(hFile,0,SEEK_END);
			length = ftell(hFile);
			fseek(hFile,0,SEEK_SET);
			buff = new unsigned char[length];
			fread(buff,length,1,hFile);
			fclose(hFile);
			ok = Load(buff,length);
			delete[] buff;
		}
	}

	return ok;
}

bool Bitmap::Load(const unsigned char* data,int /*size*/)
{
	BitmapFileHeader bmfh;
	const BitmapInfoHeader* bmih;

	if(m_BitImage) Release();

	if(data)
	{
		memcpy(&bmfh,data,sizeof(BitmapFileHeader));
		if((bmfh.m_Type == 0x4d42) && bmfh.m_Size)
		{//	да это действительно похоже на BITMAP файл
			data += sizeof(BitmapFileHeader);
			//	считываем заголовок и таблицу цветов
			bmih = reinterpret_cast<const BitmapInfoHeader*>(data);
			data -= sizeof(BitmapFileHeader);
			if(bmih->m_Planes == 1)
			{
				//	ширина и высота битового образа
				m_Width  = bmih->m_Width;
				m_Height = bmih->m_Height;
				//	тип битмэпки
				switch(bmih->m_Compression)
				{
				case 0:
					return ReadUncompressBitmap(bmih,data,bmfh.m_OffBits);
				case 1:
					return ReadRLE8Bitmap(bmih,data,bmfh.m_OffBits);
				case 2:
					return ReadRLE4Bitmap(bmih,data,bmfh.m_OffBits);
				case 3:
					return BitFieldBitmap(bmih,data,bmfh.m_OffBits);
				}
			}
		}
	}

	return false;
}

bool Bitmap::ReadUncompressBitmap(const BitmapInfoHeader* bih,const unsigned char* data,int offbits)
{
	switch(bih->m_BitCount)
	{
	case 1:
	case 4:
	case 8:
		m_PixelFormat->Set(bih->m_BitCount,ic_PixelFormat::T_NONE);
		//	составляем палитру
		m_ColorMap = new ic_ColorMap(ic_PixelFormat(32,ic_PixelFormat::T_RGB888),
									 data+sizeof(BitmapFileHeader)+bih->m_Size,offbits-(sizeof(BitmapFileHeader)+bih->m_Size));
		break;
	case 16:
		m_PixelFormat->Set(bih->m_BitCount,ic_PixelFormat::T_RGB555);
		break;
	case 24:
	case 32:
		m_PixelFormat->Set(bih->m_BitCount,ic_PixelFormat::T_RGB888);
		break;
	default:
		return false;
	}
	//	рассчитываем метрики и размеры
	EstimateImageSize();
	//	копируем данные
	m_BitImage = new unsigned char[m_Size];
	memcpy(m_BitImage,data+offbits,m_Size);

	return true;
}

bool Bitmap::BitFieldBitmap(const BitmapInfoHeader* /*bih*/,const unsigned char* /*data*/,int /*offbits*/)
{
	return false;
}

bool Bitmap::ReadRLE8Bitmap(const BitmapInfoHeader* /*bih*/,const unsigned char* /*data*/,int /*offbits*/)
{
	return false;
}

bool Bitmap::ReadRLE4Bitmap(const BitmapInfoHeader* /*bih*/,const unsigned char* /*data*/,int /*offbits*/)
{
	return false;
}

bool Bitmap::Save(const char* file_name)
{
	if(file_name && m_BitImage)
	{
		FILE* hFile;

		if((hFile = fopen(file_name,"wb"))!=0)
		{
			BITMAPFILEHEADER bmfh = {0x4d42,sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+Size()+(ColorMap()?ColorMap()->Size():0),
									 0,0,sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)};
			BITMAPINFOHEADER bmih = {sizeof(BITMAPINFOHEADER),Width(),Height(),1,PixelFormat()->m_BitPerPixel,
									 0,0,0,0,0,0};
			//	непротестированный вариант!!!
			fwrite(&bmfh,sizeof(BITMAPFILEHEADER),1,hFile);
			fwrite(&bmih,sizeof(BITMAPINFOHEADER),1,hFile);
			if(ColorMap()) fwrite(ColorMap()->Map(),ColorMap()->Size(),1,hFile);
			fwrite(BitImage(),Size(),1,hFile);
			
			fclose(hFile);

			return true;
		}
	}

	return false;
}
