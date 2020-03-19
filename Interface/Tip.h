/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.09.2000

************************************************************************/
#ifndef _TIP_H_
#define _TIP_H_

namespace Tip
{
	void MoveTo(float x,float y);				//	экранные координаты
	void SetText(const char *pText);
	void SetColor(unsigned int uiColor);
	//	-------------------------------
	void Activate(void);
	void Deactivate(void);
}

#endif	//_TIP_H_