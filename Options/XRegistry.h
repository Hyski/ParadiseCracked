/***********************************************************************

                                 In Bloom

                        Copyright by Naughty 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   13.10.2000

************************************************************************/
#ifndef _X_REGISTRY_H_
#define _X_REGISTRY_H_

class XVar
{
public:
	enum TYPE {T_INT,T_FLOAT,T_STRING};
private:
	TYPE		m_type;
	int			m_int;
	float		m_float;
	std::string m_string;
public:
	XVar(){m_int = 0; m_type = T_INT;}
	XVar(int v){m_int = v; m_type = T_INT;}
	XVar(float v){m_float = v; m_type = T_FLOAT;}
	XVar(const char *v){m_string = v; m_type = T_STRING;}
	XVar(std::string& v){m_string = v; m_type = T_STRING;}
	XVar(const XVar& v){m_type = v.m_type; m_int = v.m_int; m_float = v.m_float; m_string = v.m_string;}
	virtual ~XVar(){};
public:
	int operator << (int value)
	{
		if(m_type == T_INT)
		{ 
			m_int = value;
			return m_int;
		}
		throw 0;
	}
	float operator << (float value)
	{
		if(m_type == T_FLOAT)
		{ 
			m_float = value;
			return m_float;
		}
		throw 0;
	}
	const std::string& operator << (const char* value)
	{
		if(m_type == T_STRING)
		{ 
			m_string = value;
			return m_string;
		}
		throw 0;
	}
	const std::string& operator << (std::string& value)
	{
		if(m_type == T_STRING)
		{ 
			m_string = value;
			return m_string;
		}
		throw 0;
	}
public:
	int GetInt(void) const {if(m_type == T_INT) return m_int; throw 0;}
	float GetFloat(void) const {if(m_type == T_FLOAT) return m_float; throw 0;}
	const std::string& GetString(void) const {if(m_type == T_STRING) return m_string; throw 0;}
public:
	void Save(HKEY hKey,const char *pName)
	{
		switch(m_type)
		{
		case T_INT:
			RegSetValueEx(hKey,pName,0,REG_DWORD,(unsigned char *)&m_int,sizeof(int));
			break;
		case T_FLOAT:
			RegSetValueEx(hKey,pName,0,REG_BINARY,(unsigned char *)&m_float,sizeof(float));
			break;
		case T_STRING:
			RegSetValueEx(hKey,pName,0,REG_SZ,(unsigned char *)m_string.c_str(),m_string.size());
			break;
		}
	}
	void Read(HKEY hKey,const char *pName)
	{
		unsigned char *pBuff = 0;
		DWORD dwSize;
		DWORD dwType;

		switch(m_type)
		{
		case T_INT:
			dwType = REG_DWORD;
			dwSize = sizeof(int);
			RegQueryValueEx(hKey,pName,0,&dwType,(unsigned char *)&m_int,&dwSize);
			break;
		case T_FLOAT:
			dwType = REG_BINARY;
			dwSize = sizeof(float);
			RegQueryValueEx(hKey,pName,0,&dwType,(unsigned char *)&m_float,&dwSize);
			break;
		case T_STRING:
			dwType = REG_SZ;
			RegQueryInfoKey(hKey,0,0,0,0,0,0,0,0,&dwSize,0,0);
			if(!(pBuff = new unsigned char[dwSize]))
				throw 0;
			if((RegQueryValueEx(hKey,pName,0,&dwType,pBuff,&dwSize) == ERROR_SUCCESS) && dwSize)
				m_string = (char *)pBuff;
			delete[] pBuff;
			break;
		}
	}
};

class XRegistry
{
private:
	std::map<std::string,XVar> m_Vars;
	std::string m_sSubName;
	HKEY m_hKey;
public:
	XRegistry();
	virtual ~XRegistry();
public:
	void Init(HKEY hKey,const char *pSubName);
public:
	void Register(const char *pName,int iValue);
	void Register(const char *pName,float fValue);
	void Register(const char *pName,const char *pValue);
	void Register(const char *pName,std::string& sValue);
	void UnRegister(const char *pName);
public:
	XVar& Var(const char *pName);
	XVar& operator() (const char *pName);
public:
	void Save(void);
	void Save(const char *pName);
	void Read(void);
	void Read(const char *pName);
};

#endif	//_X_REGISTRY_H_