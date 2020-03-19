/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   13.04.2000

************************************************************************/
#ifndef _MOUSE_CURSOR_H_
#define _MOUSE_CURSOR_H_

namespace MouseCursor
{
	enum CURSOR_TYPE {CT_NORMAL,
					  CT_CANMOVE,
					  CT_CANNOTMOVE,
					  CT_CANATTACK,
					  CT_CANNOTATTACK,
					  CT_CANUSE,
					  CT_SELECT,
					  CT_UNLOAD,
					  CT_SHIPMENT,
					  CT_SPECIAL,
					  CT_PICKUP,
					  CT_MEDIC = CT_PICKUP,
					  MAX_CURSORS = CT_MEDIC+1};
	struct TextField
	{
		const char* m_Text;
		unsigned int m_Color;
		TextField() : m_Text(0),m_Color(0) {}
		TextField(const char* text,unsigned int color) : m_Text(text),m_Color(color) {}
	};
	//	-------------------------------------------------------------------------------
	void SetCursor(CURSOR_TYPE ctype,
				   const TextField* t1 = 0,
				   const TextField* t2 = 0,
				   const TextField* t3 = 0);
	void SetVisible(bool visible);
};

#endif
