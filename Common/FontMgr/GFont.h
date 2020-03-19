/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: класс для работы с определенным шрифтом.
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   14.06.2000

************************************************************************/
#ifndef _GFONT_H_
#define _GFONT_H_

#include "mlf.h"
#include "..\TextureMgr\DIBData.h"

typedef struct
{
	std::vector<std::string> vWords;		//	вектор слов
	unsigned int iAddSpace;					//	дополнительное расстояние между словами
	bool bEndOfString;						//	конец строки (абзаца)
} GFONT_STRING_INFO;

class FormattedText;
class VirtualTexture;

class GFont
{
public:
	LPDIRECTDRAWSURFACE7 m_pSurface;
	bool m_bValid;
private:
	int m_iFontHeight;
	RECT m_rcChar[256];
	ABC m_abcChar[256];
public:
	GFont();
	virtual ~GFont();
private:
	bool CreateSurface(LPDIRECT3DDEVICE7 lpd3dDevice,int width,int height,VFile *pVFile);
	bool CopyRGBAToSurface(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,DIBData *pDIBData,LPDIRECTDRAWSURFACE7 lpSurface);
public:
	bool Load(LPDIRECT3DDEVICE7 lpd3dDevice,VFile *pVFile);
	//	вывод текста
	void CharOut(LPDIRECTDRAWSURFACE7 pSurface,int x,int y,unsigned char Sym,int *xOut,int width,int height);
	void TextOut(LPDIRECTDRAWSURFACE7 pSurface,int x,int y,const char* pText,int width,int height);
	void TextOutEx(LPDIRECTDRAWSURFACE7 pSurface,int y,std::vector<GFONT_STRING_INFO>& sStrings,int width,int height);
	void TextOutEx(LPDIRECTDRAWSURFACE7 pSurface,int y,std::vector<GFONT_STRING_INFO>& sStrings,
				   int width,int height,unsigned int uiPos,
				   int *rx,int *ry);
	void TextOut(LPDIRECTDRAWSURFACE7 pSurface,
				 FormattedText *m_pfText,
				 int y,
				 int width,
				 int height);
	void TextOut(VirtualTexture *pTexture,
				 FormattedText *pfText,
				 int y,
				 int width,
				 int height);
	void CharOut(VirtualTexture *pTexture,int x,int y,unsigned char Sym,int *xOut,int width,int height);
	//	просчет текста
	void ResolveText(const char *pText,std::vector<GFONT_STRING_INFO>& sStrings,int width,int height);
public:	//	информационные функции
	inline bool IsValid(void);
	void GetSize(const char *pText,unsigned int n,SIZE *pSZ);
	void GetSize(const char *pText,SIZE *pSZ);
	void GetSize(unsigned char ch,SIZE *pSZ);
	//	информация
	inline int Height(void);
private:
	unsigned int ResolveString(const char *pText,int width,std::vector<std::string>& vWords,unsigned int *iAddSpace,bool *bEndOfString);
	unsigned int GetWord(const char *pText,std::string& str);


};

bool GFont::IsValid(void)
{
	return m_bValid;
}

int GFont::Height(void)
{
	return m_iFontHeight;
}

#endif	//_GFONT_H_