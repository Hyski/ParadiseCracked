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
#include "ijl.h"
#include "Jpeg.h"

Jpeg::Jpeg() : Image()
{
}

Jpeg::Jpeg(const Jpeg& image) : Image(image)
{
}

Jpeg::Jpeg(const Image& image) : Image(image)
{
}

Jpeg::~Jpeg()
{
}

//	оператор копирования
const Jpeg& Jpeg::operator=(const Image& image)
{
	(*this).Image::operator=(image);

	return (*this);
}

bool Jpeg::Load(const char* file_name)
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

bool Jpeg::Load(const unsigned char* data,int size)
{
	JPEG_CORE_PROPERTIES jpeg_core_prop;

	if(m_BitImage) Release();

	memset(&jpeg_core_prop,0,sizeof(JPEG_CORE_PROPERTIES));
	if(ijlInit(&jpeg_core_prop) == IJL_OK)
	{
		//	далее
		jpeg_core_prop.JPGBytes = const_cast<unsigned char*>(data);
		jpeg_core_prop.JPGSizeBytes = size;
		//	читаем заголовок
		if(ijlRead(&jpeg_core_prop, IJL_JBUFF_READPARAMS) == IJL_OK)
		{
			//	ширина и высота битового образа
			m_Width  = jpeg_core_prop.JPGWidth;
			m_Height = jpeg_core_prop.JPGHeight;
			//	устанавливаем формат пикселя
			m_PixelFormat->Set(24,ic_PixelFormat::T_RGB888);
			//	рассчитываем метрики
			EstimateImageSize();
			//	выделяем память под битовый образ
			m_BitImage = new unsigned char[m_Size];
			//	заполняем структурку
			jpeg_core_prop.DIBWidth    = jpeg_core_prop.JPGWidth;
			jpeg_core_prop.DIBHeight   = jpeg_core_prop.JPGHeight;
			jpeg_core_prop.DIBChannels = jpeg_core_prop.JPGChannels;
			jpeg_core_prop.DIBColor	   = IJL_BGR;
			jpeg_core_prop.DIBBytes    = m_BitImage;

			if(ijlRead(&jpeg_core_prop,IJL_JBUFF_READWHOLEIMAGE) == IJL_OK)
			{
				if(ijlFree(&jpeg_core_prop) == IJL_OK)
				{
					//	переворачиваем картинку
					unsigned char* pTurnOverImage = new unsigned char[m_Size];
					unsigned char* pSrcImage = m_BitImage;
					unsigned char* pDstImage = pTurnOverImage+(m_Stride*(m_Height-1));
					if(pTurnOverImage)
					{ 
						for(int i=0;i<m_Height;i++)
						{
							memcpy(pDstImage,pSrcImage,m_Stride);
							pSrcImage += m_Stride;
							pDstImage -= m_Stride;
						}
						
						delete[] m_BitImage;
						m_BitImage = pTurnOverImage;
					}

					return true;
				}
			}
		}
	}

	return false;
}