/***********************************************************************

                                 In Bloom

                        Copyright by Naughty 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   13.10.2000

************************************************************************/
#include "Precomp.h"
#include "XRegistry.h"

XRegistry::XRegistry()
{
	m_hKey = 0;
}

XRegistry::~XRegistry()
{
}

void XRegistry::Init(HKEY hKey,const char *pSubName)
{
	m_hKey = hKey;
	m_sSubName = pSubName;
	m_Vars.clear();
}

void XRegistry::Register(const char *pName,int iValue)
{
	if(m_Vars.find(pName) == m_Vars.end())
		m_Vars.insert(std::map<std::string,XVar>::value_type(pName,XVar(iValue)));
	else
		throw 0;
}

void XRegistry::Register(const char *pName,float fValue)
{
	if(m_Vars.find(pName) == m_Vars.end())
		m_Vars.insert(std::map<std::string,XVar>::value_type(pName,XVar(fValue)));
	else
		throw 0;
}

void XRegistry::Register(const char *pName,const char *pValue)
{
	if(m_Vars.find(pName) == m_Vars.end())
		m_Vars.insert(std::map<std::string,XVar>::value_type(pName,XVar(pValue)));
	else
		throw 0;
}

void XRegistry::Register(const char *pName,std::string& sValue)
{
	if(m_Vars.find(pName) == m_Vars.end())
		m_Vars.insert(std::map<std::string,XVar>::value_type(pName,XVar(sValue)));
	else
		throw 0;
}

void XRegistry::UnRegister(const char *pName)
{
	std::map<std::string,XVar>::iterator i;

	if((i = m_Vars.find(pName)) != m_Vars.end())
		m_Vars.erase(i);
	else
		throw 0;
}

XVar& XRegistry::Var(const char *pName)
{
	return m_Vars[pName];
}

XVar& XRegistry::operator () (const char *pName)
{
	return m_Vars[pName];
}

void XRegistry::Save(void)
{
	HKEY hSubKey;
	std::map<std::string,XVar>::iterator i;

	if(m_hKey && m_sSubName.size())
	{
		RegCreateKey(m_hKey,m_sSubName.c_str(),&hSubKey);
		for(i=m_Vars.begin();i!=m_Vars.end();i++)
			i->second.Save(hSubKey,i->first.c_str());
		RegCloseKey(hSubKey);
	}
}

void XRegistry::Read(void)
{
	HKEY hSubKey;
	std::map<std::string,XVar>::iterator i;

	if(m_hKey && m_sSubName.size())
	{
		if(RegOpenKey(m_hKey,m_sSubName.c_str(),&hSubKey) == ERROR_SUCCESS)
		{
			for(i=m_Vars.begin();i!=m_Vars.end();i++)
				i->second.Read(hSubKey,i->first.c_str());
			RegCloseKey(hSubKey);
		}
	}
}

void XRegistry::Save(const char *pName)
{
	HKEY hSubKey;
	std::map<std::string,XVar>::iterator i;

	if((i = m_Vars.find(pName))!=m_Vars.end())
	{
		if(m_hKey && m_sSubName.size())
		{
			RegCreateKey(m_hKey,m_sSubName.c_str(),&hSubKey);
			i->second.Save(hSubKey,i->first.c_str());
			RegCloseKey(hSubKey);
		}
	}
	else
		throw 0;
}

void XRegistry::Read(const char *pName)
{
	HKEY hSubKey;
	std::map<std::string,XVar>::iterator i;

	if((i = m_Vars.find(pName))!=m_Vars.end())
	{
		if(m_hKey && m_sSubName.size())
		{
			if(RegOpenKey(m_hKey,m_sSubName.c_str(),&hSubKey) == ERROR_SUCCESS)
			{
				i->second.Read(hSubKey,i->first.c_str());
				RegCloseKey(hSubKey);
			}
		}
	}
	else
		throw 0;
}
