/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#include "precomp.h"
#include <stdio.h>
#include <stdarg.h>
#include "LConsole.h"

CLConsole::CLConsole()
{
	m_lpFile = 0;
}

void CLConsole::Open(const char *lpFileName)
{
	m_lpFile = fopen(lpFileName,"w");
}

CLConsole::~CLConsole()
{
	if(m_lpFile)
		fclose(m_lpFile);
}

void CLConsole::operator()(char *lpStr,...)
{
	va_list list;

	va_start(list,lpStr);
	if(m_lpFile && list)
	{
		vfprintf(m_lpFile,lpStr,list);
		fflush(m_lpFile);
	}
	va_end(list);
}