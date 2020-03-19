/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   18.01.2001

************************************************************************/
#ifndef _BANNER_H_
#define _BANNER_H_

#include "BannerMgr.h"

class elText;

class Banner
{
private:
	elText *m_pText;		//	отображаемый текст 
public:
	Banner();
	virtual ~Banner();
public:
	void SetText(const char *pText);
	void SetColor(unsigned long ulTextColor = 0xffffff);
	void SetPoint(const point3& pt);
public:
	void Tune(void);
};

inline void Banner::SetPoint(const point3& pt)
{
	BannerMgr::AddPoint(pt,m_pText);
}


#endif	//_BANNER_H_