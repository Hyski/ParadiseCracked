/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: 
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   18.05.2000

************************************************************************/
#include "Precomp.h"
#include "XRegistry.h"
#include "Options.h"

//---------- Лог файл ------------
#ifdef _DEBUG_SHELL
CLog options_log;
#define options	options_log["options.log"]
#else
#define options	/##/
#endif

namespace Options
{
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
		int GetInt(void) const {if(m_type == T_INT) return m_int; throw 0;}
		float GetFloat(void) const {if(m_type == T_FLOAT) return m_float; throw 0;}
		const std::string& GetString(void) const {if(m_type == T_STRING) return m_string; throw 0;}
		XVar::TYPE Type(void) const {return m_type;}
	};
}

namespace Options
{
	//	имя входного/выходного файла
	const char *m_sOptionsFile = "system.cfg";
	//	карты хранения переменных
	std::map<std::string,XVar> m_VarMap;
	//	карта считанной информации из входного файла
	std::map<std::string,std::string> m_VarFileMap;
	//	флаг изменений
	bool m_bChanged = false;
	//	работа с реестром
	XRegistry* m_Registry = 0;
}

namespace Options
{
	bool FindVarName(VFile *pVFile,unsigned int *pPos,std::string& sBuff);
	bool FindVarValue(VFile *pVFile,unsigned int *pPos,std::string& sBuff);
	bool IsSeparator(char ch);
}

//	работа с реестром
XRegistry* Options::Registry(void)
{
	if(!m_Registry) m_Registry = new XRegistry();
	return m_Registry;
}

//	считывание файла опций
void Options::Init(void)
{
	VFile vFile;
	unsigned int charRead = 0;
	std::string sVarName;
	std::string sVarValue;

	if(vFile.Load(m_sOptionsFile))
	{
		while(FindVarName(&vFile,&charRead,sVarName))
		{
			if(FindVarValue(&vFile,&charRead,sVarValue))
			{
				m_VarFileMap[sVarName] = sVarValue;
			}
		}
	}
}

//	очищение переменных
void Options::Close(void)
{
	m_VarMap.clear();
	m_VarFileMap.clear();
	//	завершаем работу с реестром
	if(m_Registry)
	{
		m_Registry->Save();
		delete m_Registry;
	}
}

//	сброс опций на диск в файл опций
void Options::Flush(bool bUseFlag)
{
	FILE* hFile;

	if((bUseFlag && m_bChanged) || (!bUseFlag))
	{
		if((hFile = fopen(m_sOptionsFile,"wt"))!=0)
		{
			std::map<std::string,XVar>::iterator i;

			for(i=m_VarMap.begin();i!=m_VarMap.end();i++)
			{
				switch(i->second.Type())
				{
				case XVar::T_INT:
					fprintf(hFile,"%s \"%d\"\n",i->first.c_str(),i->second.GetInt());
					break;
				case XVar::T_FLOAT:
					fprintf(hFile,"%s \"%f\"\n",i->first.c_str(),i->second.GetFloat());
					break;
				case XVar::T_STRING:
					fprintf(hFile,"%s \"%s\"\n",i->first.c_str(),i->second.GetString().c_str());
					break;
				}
			}
			
			fclose(hFile);
		}
	}
}

//	регистрируем тип INT
void Options::Set(const char *sName,int iValue)
{
	std::map<std::string,XVar>::iterator i;
	std::map<std::string,std::string>::iterator ii;

	options("try Set(\"%s\",%d);\n",sName,iValue);
	
	i = m_VarMap.find(sName);
	if(i != m_VarMap.end())
	{
		m_VarMap[sName] = iValue;
		m_bChanged = true;
	}
	else
	{
		ii = m_VarFileMap.find(sName);
		if(ii != m_VarFileMap.end())
		{
			m_VarMap[sName] = atoi(m_VarFileMap[sName].c_str());
			m_VarFileMap.erase(ii);
			options("read value == %d;\n",m_VarMap[sName].GetInt());
		}
		else
		{
			m_VarMap[sName] = iValue;
			m_bChanged = true;
		}
	}
}

//	регистрируем тип FLOAT
void Options::Set(const char *sName,float fValue)
{
	std::map<std::string,XVar>::iterator i;
	std::map<std::string,std::string>::iterator ii;

	options("try Set(\"%s\",%f);\n",sName,fValue);

	i = m_VarMap.find(sName);
	if(i != m_VarMap.end())
	{
		m_VarMap[sName] = fValue;
		m_bChanged = true;
	}
	else
	{
		ii = m_VarFileMap.find(sName);
		if(ii != m_VarFileMap.end())
		{
			m_VarMap[sName] = static_cast<float>(atof(m_VarFileMap[sName].c_str()));
			m_VarFileMap.erase(ii);
			options("read value == %f;\n",m_VarMap[sName].GetFloat());
		}
		else
		{
			m_VarMap[sName] = fValue;
			m_bChanged = true;
		}
	}
}

//	регистрируем тип STD::STRING
void Options::Set(const char *sName,const char *sValue)
{
	std::map<std::string,XVar>::iterator i;
	std::map<std::string,std::string>::iterator ii;

	options("try Set(\"%s\",\"%s\");\n",sName,sValue);

	i = m_VarMap.find(sName);
	if(i != m_VarMap.end())
	{
		m_VarMap[sName] = sValue;
		m_bChanged = true;
	}
	else
	{
		ii = m_VarFileMap.find(sName);
		if(ii != m_VarFileMap.end())
		{
			m_VarMap[sName] = m_VarFileMap[sName].c_str();
			m_VarFileMap.erase(ii);
			options("read value == %s;\n",m_VarMap[sName].GetString().c_str());
		}
		else
		{
			m_VarMap[sName] = sValue;
			m_bChanged = true;
		}
	}
}

int Options::GetInt(const char *sName)
{
	std::map<std::string,XVar>::iterator i;

	i = m_VarMap.find(sName);
	if((i == m_VarMap.end()) || (i->second.Type()!=XVar::T_INT))
		throw CASUS(std::string("Неверное имя переменной <")+std::string(sName)+std::string(">.\n"));

	return i->second.GetInt();
}

float Options::GetFloat(const char *sName)
{
	std::map<std::string,XVar>::iterator i;

	i = m_VarMap.find(sName);
	if((i == m_VarMap.end()) || (i->second.Type()!=XVar::T_FLOAT))
		throw CASUS(std::string("Неверное имя переменной <")+std::string(sName)+std::string(">.\n"));

	return i->second.GetFloat();
}

const std::string& Options::GetString(const char *sName)
{
	std::map<std::string,XVar>::iterator i;

	i = m_VarMap.find(sName);
	if((i == m_VarMap.end()) || (i->second.Type()!=XVar::T_STRING))
		throw CASUS(std::string("Неверное имя переменной <")+std::string(sName)+std::string(">.\n"));

	return i->second.GetString();
}

//**********************************************************************************//
bool Options::FindVarName(VFile *pVFile,unsigned int *pPos,std::string& sBuff)
{
	unsigned int iBegin;

	if((*pPos) == pVFile->Size())
		return false;
	//	ищем начало
	while(IsSeparator(*(pVFile->Data()+(*pPos))))	
	{
		(*pPos)++;
		if((*pPos) == pVFile->Size())
			return false;
	}
	iBegin = (*pPos);
	//	ищем конец
	while(!IsSeparator(*(pVFile->Data()+(*pPos))))	
	{
		(*pPos)++;
		if((*pPos) == pVFile->Size())
			return false;
	}
	//	выдираем данные
	sBuff.clear();
	for(;iBegin<(*pPos);iBegin++)
	{
		sBuff += *(pVFile->Data()+iBegin);
	}
	options("FindVarName: \"%s\";\n",sBuff.c_str());

	return true;
}

bool Options::FindVarValue(VFile *pVFile,unsigned int *pPos,std::string& sBuff)
{
	unsigned int iBegin;

	if((*pPos) == pVFile->Size())
		return false;
	//	ищем начало
	while(*(pVFile->Data()+(*pPos)) != '"')	
	{
		(*pPos)++;
		if((*pPos) == (pVFile->Size()-1))
			return false;
	}
	iBegin = ++(*pPos);
	//	ищем конец
	while(*(pVFile->Data()+(*pPos)) != '"')	
	{
		(*pPos)++;
		if((*pPos) == (pVFile->Size()-1))
			return false;
	}
	//	выдираем данные
	sBuff.clear();
	for(;iBegin<(*pPos);iBegin++)
	{
		sBuff += *(pVFile->Data()+iBegin);
	}
	(*pPos)++;
	options("FindVarValue: \"%s\";\n",sBuff.c_str());

	return true;
}

bool Options::IsSeparator(char ch)
{
	if((ch == ' ')		||
	   (ch == '\t')		||
	   (ch == '\r')		||
	   (ch == '\n'))
	{
		return true;
	}

	return false;
}
