/***********************************************************************

                            Paradise Cracked

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   02.03.2001

************************************************************************/
#ifndef _VDATA_
#define _VDATA_

//*********************************************************************//
//******************         struct VPoint         ********************//
struct VPoint
{
	int x,y;
	VPoint() {x = y = 0;}
	VPoint(int _x,int _y) {x = _x; y = _y;}
	VPoint& operator=(VPoint& p) {x = p.x; y = p.y; return *this;}
};

//*********************************************************************//
//******************         class VRegion         ********************//
class VRegion
{
private:
	VPoint *m_vPoints;			//	массив точек
	int m_left,m_top;			//	координаты прямоугольника
	int m_right,m_bottom;		//	координаты прямоугольника
	int m_iNum;					//	число точек в массиве
public:
	VRegion(){m_vPoints = 0; m_iNum = 0; m_left = m_top = m_right = m_bottom = 0;}
	VRegion(const VRegion& r)
	{
		m_vPoints = 0;
		m_iNum = 0;
		m_left = m_top = m_right = m_bottom = 0;
		*this = r;
	}
	VRegion(VPoint *pPoints,unsigned int num)
	{
		m_vPoints = 0;
		m_iNum = 0; 
		m_left = m_top = m_right = m_bottom = 0;
		Create(pPoints,num);
	}
	VRegion(int left,int top,int right,int bottom)
	{
		m_vPoints = 0; 
		m_iNum = 0; 
		m_left = m_top = m_right = m_bottom = 0;
		Create(left,top,right,bottom);
	}
	virtual ~VRegion(){	if(m_vPoints) delete[] m_vPoints;}
public:
	bool Create(VPoint *pPoints,unsigned int num);
	bool Create(int left,int top,int right,int bottom);
	void Release(void);
	const VRegion& operator=(const VRegion& r);
	//	-----------------------------------------------
	int X(void) const {return m_left;} 	
	int Y(void) const {return m_top;}	
	int Width(void) const {return (m_right-m_left);}	
	int Height(void) const {return (m_bottom-m_top);}	
	bool GetRect(int *left,int *top,int *right,int *bottom) const;
	bool PtInRegion(int x,int y) const;
	//	-----------------------------------------------
	unsigned int Num(void) const {return m_iNum;}
	const VPoint* Points(void) const {return m_vPoints;}
	bool IsValid(void) const {return (m_vPoints && m_iNum);}
	//	------- матрица трансформирования -------------
	void Transform(float em11,float em12,float em13,
				   float em21,float em22,float em23);
private:
	bool RayIntersection(VPoint& a,VPoint& b,VPoint& c,VPoint& d) const;
	void CalcRect(void);
};

inline void VRegion::Release(void)
{
	if(m_vPoints)
	{
		delete[] m_vPoints;
		m_vPoints = 0;
	}
	m_left = m_top = 0;
	m_right = m_bottom = 0;
	m_iNum = 0;
}

inline bool VRegion::Create(VPoint *pPoints,unsigned int num)
{
	if(pPoints && num)
	{
		if(m_vPoints) Release();
		m_vPoints = new VPoint[num];
		if(m_vPoints)
		{
			memcpy(m_vPoints,pPoints,sizeof(VPoint)*num);
			m_iNum = num;
			CalcRect();

			return true;
		}
	}

	return false;
}

inline bool VRegion::Create(int left,int top,int right,int bottom)
{
	if(m_vPoints) Release();
	m_vPoints = new VPoint[4];
	if(m_vPoints)
	{
		m_vPoints[0] = VPoint(left,top);
		m_vPoints[1] = VPoint(right,top);
		m_vPoints[2] = VPoint(right,bottom);
		m_vPoints[3] = VPoint(left,bottom);
		m_iNum = 4;
		//	calc rect
		m_left = left;
		m_top = top;
		m_right = right;
		m_bottom = bottom;

		return true;
	}

	return false;
}

inline bool VRegion::GetRect(int *left,int *top,int *right,int *bottom) const
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
		for(int i=1;i<m_iNum;i++)
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

inline bool VRegion::PtInRegion(int x,int y) const
{
	int counter = 0;

	if(m_vPoints)
	{
		if(m_left<=x && x<=m_right && m_top<=y && y<=m_bottom)
		{
			int i=0;
			for(;i<(m_iNum-1);i++)
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
				if(y != m_vPoints[0].y)
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

inline bool VRegion::RayIntersection(VPoint& a,VPoint& b,VPoint& c,VPoint& d) const
{
	float r,s;

	if(c.y != d.y)
	{
		r = (float)((a.y-c.y)*(d.x-c.x)-(a.x-c.x)*(d.y-c.y))/((b.x-a.x)*(d.y-c.y)-(b.y-a.y)*(d.x-c.x));
		s = (float)((a.y-c.y)*(b.x-a.x)-(a.x-c.x)*(b.y-a.y))/((b.x-a.x)*(d.y-c.y)-(b.y-a.y)*(d.x-c.x));
		
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
		for(int i=0;i<m_iNum;i++)
		{
			x = em11*m_vPoints[i].x+em12*m_vPoints[i].y+em13;
			y = em21*m_vPoints[i].x+em22*m_vPoints[i].y+em23;
			m_vPoints[i].x = x;
			m_vPoints[i].y = y;
		}
		CalcRect();
	}
}

inline const VRegion& VRegion::operator=(const VRegion& r)
{
	if(m_vPoints) Release();
	m_iNum = 0;
	if(r.m_vPoints)
	{
		m_vPoints = new VPoint[r.m_iNum];
		if(m_vPoints)
		{
			memcpy(m_vPoints,r.m_vPoints,sizeof(VPoint)*r.m_iNum);
			m_iNum = r.m_iNum;
			m_left = r.m_left;
			m_top = r.m_top;
			m_right = r.m_right;
			m_bottom = r.m_bottom;
		}
	}

	return *this;
}

//*********************************************************************//
//******************      struct VMouseState       ********************//
struct VMouseState
{
	float x,y;
	float dx,dy,dz;
	int LButtonFront;
	int LButtonBack;
	int LButtonState;
	int RButtonFront;
	int RButtonBack;
	int RButtonState;
	int MButtonFront;
	int MButtonBack;
	int MButtonState;
	int LDblClick;
	int RDblClick;
	int MDblClick;
};

//*********************************************************************//
//******************     struct VKeyboardState     ********************//
struct VKeyboardState
{
	unsigned char Front[256];
	unsigned char Back[256];
	unsigned char State[256];
};

#endif //_VDATA_