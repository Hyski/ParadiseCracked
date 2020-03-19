/***********************************************************************

                                 In Bloom

                        Copyright by Naughty 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.09.2000

************************************************************************/
#ifndef _G_LAYER_H_
#define _G_LAYER_H_

class GLayer
{
public:
	HDC	m_hDC;					//	контекст устройства
private:
	HBITMAP m_hBitmap;			//	картинка, выбранная в данный контекст устройства
	HGDIOBJ m_hLastGDIObject;	//	предыдущий объект в данном контексте
public:
	GLayer();
	GLayer(HDC hdc,int x,int y);
	virtual ~GLayer();
public:
	void Init(HDC hdc,int x,int y);
	void Flip(HDC hdc,int DestX,int DestY,int DestWidth,int DestHeight,int SrcX,int SrcY,DWORD dwRop = SRCCOPY);
	void Flip(HDC hdc,PAINTSTRUCT *lpPS,DWORD dwRop = SRCCOPY);
	void Release(void);
};

#endif	//_G_LAYER_H_