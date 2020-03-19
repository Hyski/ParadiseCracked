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
#include "Precomp.h"
#include "../FontMgr/FontMgr.h"
#include "../FontMgr/FastFont.h"
#include "../CmdLine/CmdLine.h"
#include "DebugInfo.h"


namespace DebugInfo
{
	struct DebugData
	{
		int x;
		int y;
		std::string sData;
		DebugData(int ix,int iy,const char *pData)
		{
			x = ix; y = iy; sData = pData;
		};
	};
}

namespace DebugInfo
{
	std::vector<DebugData> m_DebugData;
}

void DebugInfo::Add(int x,int y,char *pStr,...)
{
	va_list list;
	static char Buff[500];

	if(CmdLine::IsKey("-di"))
	{
		va_start(list,pStr);
		vsprintf(Buff,pStr,list);
		va_end(list);
	
		m_DebugData.push_back(DebugData(x,y,Buff));
	}
}

void DebugInfo::Render(void)
{
	if(CmdLine::IsKey("-di"))
	{
		for(int i=0;i<m_DebugData.size();i++)
		{
			FontMgr::Instance()->GetFastFont("debug")->RenderText(m_DebugData[i].sData.c_str(),m_DebugData[i].x,m_DebugData[i].y,0xaaddff);
		}
		
		FontMgr::Instance()->GetFastFont("debug")->SwapBuffer();
		
		m_DebugData.clear();
	}
}

void DebugInfo::Clear(void)
{
	m_DebugData.clear();
}
