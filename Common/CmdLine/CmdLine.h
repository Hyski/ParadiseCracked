/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.10.2000

************************************************************************/
#ifndef _CMD_LINE_H_
#define _CMD_LINE_H_

namespace CmdLine
{
	//	инициализируем список ключей и их значений
	void Init(void);
	//	проверка на присутсвие ключа
	bool IsKey(const char* pKey);
	bool IsKey(const char* pKey,std::string& sValue);
	//	запросить значение по ключу, если значения нет вернется пустая строка
	const std::string Query(const char* pKey);
}

#endif	//_CMD_LINE_H_