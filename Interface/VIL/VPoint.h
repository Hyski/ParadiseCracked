/***********************************************************************

                         Virtual Interface Layer

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
	    Виртуальная точка.
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   18.07.2000

************************************************************************/
#ifndef _VPOINT_H_
#define _VPOINT_H_

struct VPoint
{
	float x,y;
	VPoint() {x = y = 0;}
	VPoint(float _x,float _y) {x = _x; y = _y;}
	VPoint& operator=(VPoint& p) {x = p.x; y = p.y; return *this;}
};

#endif	//_VPOINT_H_