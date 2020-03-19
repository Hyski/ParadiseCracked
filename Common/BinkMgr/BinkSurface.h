/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#ifndef _BINK_SURFACE_H_
#define _BINK_SURFACE_H_

#include "../Bink/Bink.h"

class BinkSurface
{
private:
	LPDIRECTDRAWSURFACE7 m_pSurface;
	HBINK m_hBink;
public:
	BinkSurface();
	virtual ~BinkSurface();
public:
	bool Create(LPDIRECTDRAW7 pDD,const char *pFileName);
	void Release(void);
public:	//	управление выводом бинка
	void Update(void);
public:
	LPDIRECTDRAWSURFACE7 Surface(void);
private:
	void Unpack(void);
};

inline LPDIRECTDRAWSURFACE7 BinkSurface::Surface(void)
{
	return m_pSurface;
}

#endif	//_BINK_SURFACE_H_