/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   01.08.2000

************************************************************************/
#ifndef _ZIP_INFO_H_
#define _ZIP_INFO_H_

namespace ZipInfo
{
	//	получить листинг zip файла - szZipName
	void ListZip(const char *szZipName,std::vector<std::string>& vStr);
	//	получить содержимое файла, заносится в pData, его потом необходимо освободить
	bool LoadFile(const char *szZipName,
				  const char *szFileName,
				  unsigned char **pData,
				  unsigned int *pSize);
	//	распаковать файл на диск
	bool UnzipFile(const char *szZipName,
				   const char *szFileName);
}


#endif	//_ZIP_INFO_H_