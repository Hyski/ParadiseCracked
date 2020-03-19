/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   15.04.2000

************************************************************************/
#ifndef _DATA_MGR_H_
#define _DATA_MGR_H_

#include "..\Utils\VFile.h" 
#include <io.h>

namespace DataMgr
{
	enum METHOD {INPACK,ONDISK,ONDISK_INPACK,INPACK_ONDISK};
	//	-------------------------------
	struct FileInfo
	{
		enum 
		{
			ARCHIVE		=	_A_ARCH,
			HIDDEN		=	_A_HIDDEN,
			NORMAL		=	_A_NORMAL,
			READONLY	=	_A_RDONLY,
			SUBDIR		=	_A_SUBDIR,
			SYSTEM		=	_A_SYSTEM
		};
		//--------------------
		enum LOCATION {ON_DISK,IN_PACK};
		LOCATION m_Location;
		//--------------------
		std::string m_FullName;
		std::string m_Name;
		std::string m_Extension;
		//--------------------
		unsigned int m_Attr;
	};
	//	-------------------------------
	void ReadPacks(const char* path,bool is_mod = false);
	//	-------------------------------
	VFile *Load(const char *pDataName,int iParam = ONDISK_INPACK);
	void Release(const char *pDataName);
	//	-------------------------------
	void Initialize(void);
	void Uninitialize(void);
	//	-------------------------------
	//	сканирование каталога в паке и на диске
	//	пример: szPackDirName == "path1/path2/path3/";
	//	символ '/' в конце пути обязателен.
	int Scan(const char* dir_name,std::vector<DataMgr::FileInfo>& files,METHOD method = ONDISK_INPACK);
	//	получить кол-во данных хранимых в памяти
	int GetFileNum(void);	//	кол-во открытых файлов
	int GetDataSize(void);	//	кол-во байт занимаемых ими в памяти
}

#endif	//_DATA_MGR_H_