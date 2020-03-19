/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.09.2000

************************************************************************/
#include "Precomp.h"
#include "MenuMgr.h"
#include "Tip.h"

void Tip::MoveTo(float x,float y)				//	экранные координаты
{
	MenuMgr::Instance()->Tip()->MoveTo(x,y);
}

void Tip::SetText(const char *pText)
{
	MenuMgr::Instance()->Tip()->SetText(pText);
}

void Tip::Activate(void)
{
	MenuMgr::Instance()->Tip()->Activate(true);
}

void Tip::Deactivate(void)
{
	MenuMgr::Instance()->Tip()->Activate(false);
}

void Tip::SetColor(unsigned int uiColor)
{
	MenuMgr::Instance()->Tip()->SetColor(uiColor);
}

