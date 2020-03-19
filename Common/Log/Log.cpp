/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: класс, помогающий вести лог-файлы.

   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#include "Precomp.h"
#include "Log.h"

using namespace std;

CLog::CLog()
{
}

CLog::~CLog()
{
}

CLConsole& CLog::operator[](const char *lpLogName)
{
	map<string,CLConsole>::iterator i;

	i = m_logMap.find(lpLogName);
	
	if(i==m_logMap.end())
	{
		m_logMap[lpLogName].Open(lpLogName);
		i = m_logMap.find(lpLogName);
	}

	return i->second;
}



		
