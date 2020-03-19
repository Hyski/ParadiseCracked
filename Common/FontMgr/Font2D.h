/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   06.03.2001

************************************************************************/
#ifndef _FONT_2D_H_
#define _FONT_2D_H_

#include "Font.h" 

class VFile;
class LogicalText;

class Font2D : public Font
{
private:
	LPDIRECTDRAWSURFACE7 m_pSurface;
public:
	Font2D();
	Font2D(LPDIRECTDRAWSURFACE7 pPrimarySurface,const char *pFileName,int color);
	virtual ~Font2D();
public:
	bool Create(LPDIRECTDRAWSURFACE7 pPrimarySurface,const char *pFileName,int color);
	void Release(void);
public:
	void TextOut(LPDIRECTDRAWSURFACE7 pSurface,int y,int width_limit,int height_limit,LogicalText* pLT);

	void TextOut(LPDIRECTDRAWSURFACE7 pSurface,int y,int width_limit,int height_limit,
				 LogicalText* pLT,int begin,int end,int* x_out,int* y_out);

	void CharOut(LPDIRECTDRAWSURFACE7 pSurface,int x,int y,unsigned char ch,int width_limit,int height_limit,int *xOut);
private:
	void CreateSurface(LPDIRECTDRAWSURFACE7 pPrimarySurface,int width,int height,VFile *pVFile,int color);
};

#endif	//_FONT_2D_H_