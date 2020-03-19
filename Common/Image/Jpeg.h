/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   25.05.2001

************************************************************************/
#ifndef _JPEG_H_
#define _JPEG_H_

#include "Image.h"

#pragma pack(1)

class Jpeg : public Image
{
public:
	Jpeg();
	Jpeg(const Jpeg& image);
	Jpeg(const Image& image);
	virtual ~Jpeg();
public:
	//	оператор копирования
	const Jpeg& operator=(const Image& image);
	//	загрузить картинку из файла
	bool Load(const char* file_name);
	//	загрузить картинку из памяти
	bool Load(const unsigned char* data,int size);
};

#endif	//_JPEG_H_