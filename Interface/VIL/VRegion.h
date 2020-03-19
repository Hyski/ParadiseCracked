/***********************************************************************

                         Virtual Interface Layer

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   18.07.2000

************************************************************************/
#ifndef _VREGION_H_
#define _VREGION_H_

#include <memory.h> 
#include "VPoint.h"

class VRegion
{
private:
	VPoint *m_vPoints;			//	массив точек
	float m_left,m_top;			//	координаты прямоугольника
	float m_right,m_bottom;		//	координаты прямоугольника
	unsigned int m_uiNum;		//	число точек в массиве
public:
	VRegion(){m_vPoints = 0; m_uiNum = 0;}
	virtual ~VRegion(){	if(m_vPoints) delete[] m_vPoints;}
public:
	bool Create(VPoint *pPoints,unsigned int num);
	bool Create(float left,float top,float right,float bottom);
	void Release(void);
	VRegion& operator=(VRegion& r);
	//	-----------------------------------------------
	bool GetRect(float *left,float *top,float *right,float *bottom);
	bool PtInRegion(float x,float y);
	//	-----------------------------------------------
	unsigned int Num(void);
	const VPoint* Points(void);
	//	------- матрица трансформирования -------------
	void Transform(float em11,float em12,float em13,
				   float em21,float em22,float em23);
private:
	bool RayIntersection(VPoint& a,VPoint& b,VPoint& c,VPoint& d);
	void CalcRect(void);
};

inline void VRegion::Release(void)
{
	if(m_vPoints)
		delete[] m_vPoints;
	m_vPoints = 0;
}

inline bool VRegion::Create(VPoint *pPoints,unsigned int num)
{
	if(pPoints && num)
	{
		if(m_vPoints)
			delete[] m_vPoints;
		m_vPoints = new VPoint[num];
		if(m_vPoints)
		{
			memcpy(m_vPoints,pPoints,sizeof(VPoint)*num);
			m_uiNum = num;
			CalcRect();

			return true;
		}
	}

	return false;
}

inline bool VRegion::Create(float left,float top,float right,float bottom)
{
	if(m_vPoints)
		delete[] m_vPoints;
	m_vPoints = new VPoint[4];
	if(m_vPoints)
	{
		m_vPoints[0] = VPoint(left,top);
		m_vPoints[1] = VPoint(right,top);
		m_vPoints[2] = VPoint(right,bottom);
		m_vPoints[3] = VPoint(left,bottom);
		m_uiNum = 4;
		//	calc rect
		m_left = left;
		m_top = top;
		m_right = right;
		m_bottom = bottom;

		return true;
	}

	return false;
}

inline unsigned int VRegion::Num(void)
{
	return m_uiNum;
}

inline const VPoint* VRegion::Points(void)
{
	return m_vPoints;
}

inline bool VRegion::GetRect(float *left,float *top,float *right,float *bottom)
{
	if(m_vPoints)
	{
		*left = m_left;
		*top = m_top;
		*right = m_right;
		*bottom = m_bottom;

		return true;
	}

	return false;
}

inline void VRegion::CalcRect(void)
{
	if(m_vPoints)
	{
		m_left = m_right = m_vPoints[0].x;
		m_top = m_bottom = m_vPoints[0].y;
		for(unsigned int i=1;i<m_uiNum;i++)
		{
			if(m_left > m_vPoints[i].x)
				m_left = m_vPoints[i].x;
			if(m_top > m_vPoints[i].y)
				m_top = m_vPoints[i].y;
			if(m_right < m_vPoints[i].x)
				m_right = m_vPoints[i].x;
			if(m_bottom < m_vPoints[i].y)
				m_bottom = m_vPoints[i].y;
		}
	}
}

inline bool VRegion::PtInRegion(float x,float y)
{
	int counter = 0;

	if(m_vPoints)
	{
		if(m_left<=x && x<=m_right && m_top<=y && y<=m_bottom)
		{
			unsigned int i;
			for(i=0;i<(m_uiNum-1);i++)
			{
				if(RayIntersection(VPoint(x,y),VPoint(x>m_right?x:m_right+1,y),
					m_vPoints[i],m_vPoints[i+1]))
				{
					//	проверка на попадание в вершину
					if(y != m_vPoints[i+1].y)
					{
						counter++;
					}
				}
			}
			if(RayIntersection(VPoint(x,y),VPoint(x>m_right+1?x:m_right+1,y),
				m_vPoints[i],m_vPoints[0]))
			{
				//	проверка на попадание в вершину
				if(y != m_vPoints[i+1].y)
				{
					counter++;
				}
			}
			if(counter&1)
				return true;
		}
	}

	return false;
}

inline bool VRegion::RayIntersection(VPoint& a,VPoint& b,VPoint& c,VPoint& d)
{
	float r,s;

	if(c.y != d.y)
	{
		r = ((a.y-c.y)*(d.x-c.x)-(a.x-c.x)*(d.y-c.y))/((b.x-a.x)*(d.y-c.y)-(b.y-a.y)*(d.x-c.x));
		s = ((a.y-c.y)*(b.x-a.x)-(a.x-c.x)*(b.y-a.y))/((b.x-a.x)*(d.y-c.y)-(b.y-a.y)*(d.x-c.x));
		
		if((0<=r && r<=1) && (0<=s && s<=1))
			return true;
	}

	return false;
}

inline void VRegion::Transform(float em11,float em12,float em13,
							   float em21,float em22,float em23)
{
	float x,y;

	if(m_vPoints)
	{
		for(unsigned int i=0;i<m_uiNum;i++)
		{
			x = em11*m_vPoints[i].x+em12*m_vPoints[i].y+em13;
			y = em21*m_vPoints[i].x+em22*m_vPoints[i].y+em23;
			m_vPoints[i].x = x;
			m_vPoints[i].y = y;
		}
		CalcRect();
	}
}

inline VRegion& VRegion::operator=(VRegion& r)
{
	if(m_vPoints)
		delete[] m_vPoints;
	m_uiNum = 0;
	if(r.m_vPoints)
	{
		m_vPoints = new VPoint[r.m_uiNum];
		if(m_vPoints)
		{
			memcpy(m_vPoints,r.m_vPoints,sizeof(VPoint)*r.m_uiNum);
			m_uiNum = r.m_uiNum;
			m_left = r.m_left;
			m_top = r.m_top;
			m_right = r.m_right;
			m_bottom = r.m_bottom;
		}
	}

	return *this;
}

#endif	//_VREGION_H_