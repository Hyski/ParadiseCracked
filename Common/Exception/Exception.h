/***********************************************************************

                               Alfa Project

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   10.12.2001

************************************************************************/
#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

class NoBreak {};

/////////////////////////////////////////////////////////////////////////
//////////////////////    class CasusImprovisus    //////////////////////
/////////////////////////////////////////////////////////////////////////
class CasusImprovisus
{
public:
	CasusImprovisus();
	CasusImprovisus(const std::string msg);
	CasusImprovisus(const std::string file,unsigned int line,const std::string msg);
	CasusImprovisus(const NoBreak&);
	CasusImprovisus(const std::string msg,const NoBreak&);
	CasusImprovisus(const std::string file,unsigned int line,const std::string msg,const NoBreak&);
	~CasusImprovisus();
public:
	//	получить строковое содержимое исключения
	virtual const char* Content(void);
	//	поставить точку останова
	static void DbgBreak(void);
};

/////////////////////////////////////////////////////////////////////////
////////////////////////    class CommonCasus    ////////////////////////
/////////////////////////////////////////////////////////////////////////
class CommonCasus : public CasusImprovisus
{
private:
	std::string m_Content;
public:
	CommonCasus(const std::string file,unsigned int line,const std::string msg);
	const char* Content(void);
};

#define CASUS(msg) CommonCasus(__FILE__,__LINE__,msg)

#endif	//_EXCEPTION_H_