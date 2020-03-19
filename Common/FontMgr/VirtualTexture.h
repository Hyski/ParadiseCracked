/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   25.09.2000

************************************************************************/
#ifndef _VIRTUAL_TEXTURE_H_
#define _VIRTUAL_TEXTURE_H_

#include "../TextureMgr/D3DTexture.h"

class VirtualTexture
{
private:
	class OnePart
	{
	public:
		D3DTexture *m_pTexture;			//	текстура
		D3DTLVERTEX m_d3dTLVertex[4];		//	данные о текстуре
	public:
		OnePart(){m_pTexture = 0;};
		~OnePart(){};
	};
private:
	std::vector<std::vector<OnePart> > m_TexMatrix;
public:
	DWORD m_dwVertexColor;
public:
	VirtualTexture();
	virtual ~VirtualTexture();
public:
	void Create(int vWidth,int vHeight);	//	виртуальные координаты
	void Release(void);
public:
	void Clear(void);
	void Render(void);
	void Blt(LPRECT pDestRect,LPDIRECTDRAWSURFACE7 pDDSrcSurface,  
			 LPRECT pSrcRect,DWORD dwFlags,LPDDBLTFX pDDBltFx);
private:
	void CalcRenderData(OnePart *pOP,
						float width,float height,
						float x,float y,
						float texWidth,float texHeight);
};

#endif	//_VIRTUAL_TEXTURE_H_