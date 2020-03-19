/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#ifndef _LCONSOLE_H_
#define _LCONSOLE_H_

class CLConsole
{
private:
	FILE *m_lpFile;					//	ץ‎םהכונ םא פאיכ
public:
	CLConsole();
	virtual ~CLConsole();
public:
	void Open(const char *lpFileName);
	void operator()(char *lpStr,...);
};

#endif