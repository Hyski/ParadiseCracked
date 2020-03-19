/*------------------------------------------------------*/
//     Class Name: NhtBitmap
//     Coder: Naughty
//     Date: 06.03.2000 - ..
//     Company: Naughty
/*------------------------------------------------------*/
#ifndef _NHT_BITMAP_H_
#define _NHT_BITMAP_H_

#include "DIBData.h"

class VFile;

class NhtBitmap : public DIBData
{
public:
	NhtBitmap();
	NhtBitmap(const char *pFileName);
	NhtBitmap(FILE *hFile);
	NhtBitmap(LPBYTE pData,DWORD dwSize);
	NhtBitmap(VFile *pVFile);
	virtual ~NhtBitmap();
public:	//	load
	BOOL Load(const char *pFileName);
	BOOL Load(FILE *hFile);
	BOOL Load(LPBYTE pData,DWORD dwSize);
	BOOL Load(VFile *pVFile);
protected:	// in help
//	void ComputeMetrics(void);
};

#endif