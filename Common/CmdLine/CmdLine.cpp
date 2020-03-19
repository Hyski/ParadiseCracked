/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.10.2000

************************************************************************/
#include "Precomp.h"
#include "CmdLine.h"

namespace CmdLine
{
	std::map<std::string,std::string> m_mKeys;
}

void CmdLine::Init(void)
{
	int prev = -1;

	for(int i=1;i<__argc;i++)
	{
		if(__argv[i][0] == '-')
		{
			if(m_mKeys.find(__argv[i]) == m_mKeys.end())
			{
				m_mKeys[__argv[i]] = "";
				prev = i;
			}
		}
		else
		{
			if(prev!=-1)
			{
				m_mKeys[__argv[prev]] = __argv[i];
				prev = -1;
			}
		}
	}
}

//	проверка на присутствие ключа
bool CmdLine::IsKey(const char* pKey)
{

#ifdef	_HOME_VERSION

	if(m_mKeys.find(pKey) != m_mKeys.end())
		return true;

#endif//_HOME_VERSION

	return false;
}

bool CmdLine::IsKey(const char* pKey,std::string& sValue)
{
#ifdef	_HOME_VERSION

	std::map<std::string,std::string>::iterator im;

	if((im = m_mKeys.find(pKey)) != m_mKeys.end())
	{
		sValue = im->second;
		return true;
	}

#endif//_HOME_VERSION

	return false;
}

//	запросить значение по ключу, если значения нет вернется пустая строка
const std::string CmdLine::Query(const char* pKey)
{
	static std::string empty;
	std::map<std::string,std::string>::iterator im;

	if((im = m_mKeys.find(pKey)) != m_mKeys.end())
	{
		return im->second;
	}

	return empty;
}

