/*------------------------------------------------------*/
//     Class Name: NhtTarga
//     Coder: Naughty
//     Date: 06.03.2000 - ..
//     Company: Naughty
//	   Comments: Поддержка форматов 
//		- Uncompressed RGB (24,32 (alpha-channel))
/*------------------------------------------------------*/
#ifndef _NHT_TARGA_H_
#define _NHT_TARGA_H_

#include "DIBData.h"

typedef struct 
{
	BYTE IDLength;		//	длина идентификатора
	BYTE ColorMapType;	//	RGB or Palletize
	BYTE ImageType;		//	тип картинки
	BYTE ColorMapSpecification[5];	
	WORD XOrigin;
	WORD YOrigin;
	WORD ImageWidth;
	WORD ImageHeight;
	BYTE PixelDepth;
	BYTE ImageDescriptor;
} TARGA_HEADER;

class VFile;

class NhtTarga : public DIBData
{
protected:
	LPBYTE m_lpIDField;				//	поле индификации
	BYTE   m_IDLength;				//	длина поля индификации 
public:
	NhtTarga();
	NhtTarga(const char *pFileName);
	NhtTarga(FILE *hFile);
	NhtTarga(LPBYTE pData,DWORD dwSize);
	NhtTarga(VFile *pVFile);
	virtual ~NhtTarga();
public:	//	load
	BOOL Load(const char *pFileName);
	BOOL Load(FILE *hFile);
	BOOL Load(LPBYTE pData,DWORD dwSize);
	BOOL Load(VFile *pVFile);
protected:
	virtual void InZero(void);
	virtual void Release(void);
protected:
	void ComputeMetrics(void);
	BOOL ReadUncompressedRGBData(VFile *pVFile,bool LeftUpperCorner);
	BOOL ReadUncompressedRGB16Data(VFile *pVFile,bool LeftUpperCorner);
};

#endif