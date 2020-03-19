/***********************************************************************

                         Virtual Interface Layer

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   25.07.2000

************************************************************************/
#ifndef _VMOUSE_STATE_H_
#define _VMOUSE_STATE_H_

//	экранные координаты
struct VMouseState
{
	float x,y;
	float dx,dy;
	int LButtonFront;
	int LButtonBack;
	int LButtonState;
	int RButtonFront;
	int RButtonBack;
	int RButtonState;
	int MButtonFront;
	int MButtonBack;
	int MButtonState;
};

#endif	//_VMOUSE_STATE_H_