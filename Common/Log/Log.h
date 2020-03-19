/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: класс, помогающий вести лог-файлы.

   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#ifndef _LOG_H_
#define _LOG_H_

#include "LConsole.h"

class CLog
{
private:
	std::map<std::string,CLConsole> m_logMap;
public:
	CLog();
	virtual ~CLog();
public:
	CLConsole& operator[](const char *lpLogName);
};

#endif