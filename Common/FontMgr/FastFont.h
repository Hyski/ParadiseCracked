/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   09.09.2000

************************************************************************/
#ifndef _FAST_FONT_H_
#define _FAST_FONT_H_

#include "../TextureMgr/D3DTexture.h"
#include "../TextureMgr/DIBData.h"

class FastFont
{
private:
	D3DTexture m_d3dTexture;		//	текстура под фонт
	DIBData m_SrcImage;
	int m_iTexWidth;
	int m_iTexHeight;
private:
	int m_iFontHeight;
	RECT m_rcChar[96];
	ABC m_abcChar[96];
private:
	//	буфер треугольников
	enum {MAX_CHAR_NUM = 64};
	D3DTLVERTEX m_d3dTLVertex[MAX_CHAR_NUM][6];		//	треугольники
	unsigned int m_uiChars;							//	число символов
public:
	FastFont();
	virtual ~FastFont();
public:
	bool Load(LPDIRECT3DDEVICE7 lpd3dDevice,VFile *pVFile);
	void Tune(LPDIRECT3DDEVICE7 lpd3dDevice);
public:
	//	вывод текста (виртуальные координаты)
	void RenderText(const char *pText,int vX,int vY,DWORD dwColor);
	void SwapBuffer(void);
private:
	bool CreateTexture(LPDIRECT3DDEVICE7 lpd3dDevice,int width,int height,VFile *pVFile);
};

#endif	//_FAST_FONT_H_