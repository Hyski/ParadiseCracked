/***********************************************************************

                           Texture Manager Test

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.05.2001

************************************************************************/
#ifndef _D3D_TEXTURE_H_
#define _D3D_TEXTURE_H_

class DIBData;
class Image;

class D3DTexture
{
private:
	LPDIRECTDRAWSURFACE7 m_Surface;
	int m_Width;
	int m_Height;
private:
	typedef bool (*Copy)(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface);
	static Copy m_Copy;
public:
	D3DTexture();
	D3DTexture(LPDIRECT3DDEVICE7 lpd3dDevice,
			   DIBData* pImage,			
			   int iMipMapLevel = -1,				//	-1 все mipmap'ы, 0 - ни одной
			   unsigned int iQuality = 0,			//	качество текстур, 0 - лучшее качество			
			   bool bCompressed = false,			//	использовать сжатые текстуры
			   bool bAlphaPremultiplied = false);	//	при использовании сжатых текстур,
	D3DTexture(LPDIRECT3DDEVICE7 lpd3dDevice,
			   Image* pImage,			
			   int iMipMapLevel = -1,				//	-1 все mipmap'ы, 0 - ни одной
			   unsigned int iQuality = 0,			//	качество текстур, 0 - лучшее качество			
			   bool bCompressed = false,			//	использовать сжатые текстуры
			   bool bAlphaPremultiplied = false);	//	при использовании сжатых текстур,
	virtual ~D3DTexture();
public:
	bool Create(LPDIRECT3DDEVICE7 lpd3dDevice,
				DIBData* pImage,			
				int iMipMapLevel = -1,				//	-1 все mipmap'ы, 0 - ни одной
			    unsigned int iQuality = 0,			//	качество текстур, 0 - лучшее качество			
				bool bCompressed = false,			//	использовать сжатые текстуры
				bool bAlphaPremultiplied = false);	//	при использовании сжатых текстур,
	bool Create(LPDIRECT3DDEVICE7 lpd3dDevice,
				Image* pImage,			
				int iMipMapLevel = -1,				//	-1 все mipmap'ы, 0 - ни одной
			    unsigned int iQuality = 0,			//	качество текстур, 0 - лучшее качество			
				bool bCompressed = false,			//	использовать сжатые текстуры
				bool bAlphaPremultiplied = false);	//	при использовании сжатых текстур,
	void Release(void);
public:
	LPDIRECTDRAWSURFACE7 Surface(void) const;
	int Width(void) const;
	int Height(void) const;
private:
	bool ChoosePixelFormat(Image* pImage,DDPIXELFORMAT* pDDPF,bool bCompressed,bool bAlphaPremultiplied);
	void CheckQuality(Image* pImage,unsigned int iQuality);
	void AlignSizeForCompressing(void);
	bool PrepareSurfaceProperties(LPDIRECT3DDEVICE7 lpd3dDevice,LPDDSURFACEDESC2 lpDDSD);
	void CalculateMipMapLevel(Image* pImage,LPDDSURFACEDESC2 lpDDSD,int iMipMapLevel);
	bool MakeSurface(LPDIRECT3DDEVICE7 lpd3dDevice,LPDDSURFACEDESC2 lpDDSD);
	bool FlushImageToSurface(LPDIRECT3DDEVICE7 lpd3dDevice,LPDDSURFACEDESC2 lpDDSD,Image *pImage);
	bool UseCompressing(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface,DWORD dwDstWidth,DWORD dwDstHeight);
public:
	//	функция для общего случая 
	static bool CopyImageToSurface(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface);
	//	использование временной текстуры и последующего блитинга (работает только для 32 бит)
	static bool UseBlitting(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface);
	//	использование HDC поверхностей (не работает с альфа каналом)
	static bool UseStretching(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface);
};

inline LPDIRECTDRAWSURFACE7 D3DTexture::Surface(void) const
{
	return m_Surface;
}

inline int D3DTexture::Width(void) const
{
	return m_Width;
}

inline int D3DTexture::Height(void) const
{
	return m_Height;
}

#endif	//_D3D_TEXTURE_H_