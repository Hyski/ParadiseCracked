/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: систеиа вывода отладочной информации на экран.
				 Допускает вывод всего 500 байт.
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   23.05.2000

************************************************************************/
#ifndef _DEBUG_INFO_H_
#define _DEBUG_INFO_H_

namespace DebugInfo
{
	void Add(int x,int y,char *pStr,...);
	void Render(void);
	void Clear(void);
}

#endif	//_DEBUG_INFO_H_