/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   18.01.2001

************************************************************************/
#include "Precomp.h"
#include "../Common/Shell/Shell.h"
#include "../Common/GraphPipe/GraphPipe.h"
#include "ELs/elText.h"
#include "Banner.h"
#include "BannerMgr.h"

namespace BannerMgr
{
	//	массив точек дл€ отображаемых текстов
	enum {BANNERS_NUM = 50};
	point3 m_pPoints[BANNERS_NUM];
	elText* m_ppText[BANNERS_NUM];
	int m_iPointsSize = 0;	//	текущий размер массива дл€ m_pPoints и m_ppText
	std::list<Banner*> m_lBanners;
}

void BannerMgr::DrawBanners(void)
{
	STACK_GUARD("BannerMgr::DrawBanners");
	//	трансформируем 3D точки в экранные координаты
	Shell::Instance()->GetGraphPipe()->TransformPoints(m_iPointsSize,m_pPoints,m_pPoints);
	//	отрисовываем надписи
	for(int i=0;i<m_iPointsSize;i++)
	{
	STACK_GUARD("BannerMgr::DrawBanners::for");
		if(m_pPoints[i].z != -1)
		{
			m_ppText[i]->MoveTo(ToVirtualX(m_pPoints[i].x)-(m_ppText[i]->Width()>>1),ToVirtualY(m_pPoints[i].y));
			m_ppText[i]->Render(Shell::Instance()->GetGraphPipe());
		}
	}
	m_iPointsSize = 0;
}

void BannerMgr::Tune(void)
{
	for(std::list<Banner*>::iterator i=m_lBanners.begin();i!=m_lBanners.end();i++)
		(*i)->Tune();
}

void BannerMgr::RegisterBanner(Banner *b)
{
	m_lBanners.push_back(b);
}

void BannerMgr::UnregisterBanner(Banner *b)
{
	std::list<Banner*>::iterator i = std::find(m_lBanners.begin(),m_lBanners.end(),b);
	if(i != m_lBanners.end())
		m_lBanners.erase(i);
}

void BannerMgr::AddPoint(const point3& pt,elText* pText)
{
	if(m_iPointsSize == BANNERS_NUM)
		throw CASUS("¬ыход за пределы массива.");
	m_pPoints[m_iPointsSize] = pt;
	m_ppText[m_iPointsSize] = pText;
	m_iPointsSize++;
}
