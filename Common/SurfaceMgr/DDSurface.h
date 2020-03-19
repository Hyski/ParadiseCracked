/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   28.02.2001

************************************************************************/
#ifndef _DD_SURFACE_H_
#define _DD_SURFACE_H_

class Image;

class DDSurface
{
private:
	LPDIRECTDRAWSURFACE7 m_pSurface;
public:
	DDSurface();
	DDSurface(LPDIRECTDRAW7 pDD,const Image* pImage);
	DDSurface(LPDIRECTDRAW7 pDD,LPDIRECTDRAWSURFACE7 surface);
	virtual ~DDSurface();
public:
	//	создать поверхность из образа
	bool Create(LPDIRECTDRAW7 pDD,const Image* pImage);
	//	создать копию другой поверхности
	bool Create(LPDIRECTDRAW7 pDD,LPDIRECTDRAWSURFACE7 surface);
	//	освободить связанные с поверхностью ресурсы
	void Release(void);
public:
	LPDIRECTDRAWSURFACE7 Surface(void);
};

inline LPDIRECTDRAWSURFACE7 DDSurface::Surface(void)
{
	return m_pSurface;
}

#endif	//_DD_SURFACE_H_