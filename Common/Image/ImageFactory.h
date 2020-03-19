/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   25.05.2001

************************************************************************/
#ifndef _IMAGE_FACTORY_H_
#define _IMAGE_FACTORY_H_

class Image;

class ImageFactory
{
private:
	ImageFactory();
	virtual ~ImageFactory();
public:
	static void StretchImage(HDC hdc,const Image* image,
							 int x_dst,int y_dst,
							 int dst_width,int dst_height,
							 int x_src,int y_src,
							 int src_width,int src_height,
							 unsigned int rop = SRCCOPY);
	static void DrawImage(HDC hdc,const Image* image,int x = 0,int y = 0,unsigned int rop = SRCCOPY);
};

#endif	//_IMAGE_FACTORY_H_