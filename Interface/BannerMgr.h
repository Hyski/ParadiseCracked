/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   18.01.2001

************************************************************************/
#ifndef _BANNER_MGR_H_
#define _BANNER_MGR_H_

class Banner;
class elText;

namespace BannerMgr
{
	void DrawBanners(void);
	void Tune(void);
	//	---------------------------------------------------------
	void RegisterBanner(Banner *b);
	void UnregisterBanner(Banner *b);
	//	---------------------------------------------------------
	void AddPoint(const point3& pt,elText* pText);
}

#endif	//_BANNER_MGR_H_