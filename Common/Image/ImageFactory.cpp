/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   25.05.2001

************************************************************************/
#include "Precomp.h"
#include "Image.h"
#include "ImageFactory.h"

ImageFactory::ImageFactory()
{
}

ImageFactory::~ImageFactory()
{
}

void ImageFactory::StretchImage(HDC hdc,const Image* image,
								int x_dst,int y_dst,
								int dst_width,int dst_height,
								int x_src,int y_src,
								int src_width,int src_height,
								unsigned int /*rop*/)
{
	BITMAPINFOHEADER* bmih;
	unsigned char* buffer;

	if(image->BitImage())
	{
		if(image->ColorMap())
		{
			int color_map_count = image->ColorMap()->Size()/(image->ColorMap()->PixelFormat()->m_BitPerPixel/8);
			buffer = new unsigned char[sizeof(BITMAPINFOHEADER)+(color_map_count*32)];
			if(image->ColorMap()->PixelFormat()->m_BitPerPixel == 32)
				memcpy(buffer+sizeof(BITMAPINFOHEADER),image->ColorMap()->Map(),image->ColorMap()->Size());
			if(image->ColorMap()->PixelFormat()->m_BitPerPixel == 24)
			{
				for(int i=0;i<color_map_count;i++)
				{
					*(buffer+sizeof(BITMAPINFOHEADER)+i*4) = *(image->ColorMap()->Map()+i*3);
					*(buffer+sizeof(BITMAPINFOHEADER)+i*4+1) = *(image->ColorMap()->Map()+i*3+1);
					*(buffer+sizeof(BITMAPINFOHEADER)+i*4+2) = *(image->ColorMap()->Map()+i*3+2);
				}
			}
		}
		else
		{
			buffer = new unsigned char[sizeof(BITMAPINFOHEADER)];
		}
		bmih	= reinterpret_cast<BITMAPINFOHEADER*>(buffer);

		memset(bmih,0,sizeof(BITMAPINFOHEADER));
		bmih->biSize = sizeof(BITMAPINFOHEADER);
		bmih->biWidth = image->Width();
		bmih->biHeight = image->Height();
		bmih->biPlanes = 1;
		bmih->biBitCount = image->PixelFormat()->m_BitPerPixel;
		bmih->biCompression = 0;
		bmih->biSizeImage = image->Size();

		StretchDIBits(hdc,x_dst,y_dst,dst_width,dst_height,
					  x_src,y_src,src_width,src_height,
					  image->BitImage(),reinterpret_cast<BITMAPINFO*>(buffer),DIB_RGB_COLORS,SRCCOPY);

		delete[] buffer;
	}
}

void ImageFactory::DrawImage(HDC hdc,const Image* image,int x,int y,unsigned int rop)
{
	StretchImage(hdc,image,x,y,image->Width(),image->Height(),0,0,image->Width(),image->Height(),rop);
}
