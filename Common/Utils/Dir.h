/***********************************************************************

                          Directory Explorer

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   01.08.2000

************************************************************************/
#ifndef _DIR_H_
#define _DIR_H_

#include <io.h>

struct FileInfo
{
  enum {ARCHIVE		=	_A_ARCH,
		HIDDEN		=	_A_HIDDEN,
		NORMAL		=	_A_NORMAL,
		READONLY	=	_A_RDONLY,
		SUBDIR		=	_A_SUBDIR,
		SYSTEM		=	_A_SYSTEM
	   };
  //--------------------
  std::string full_name;
  std::string name;
  std::string extension;
  //--------------------
  unsigned int attr;
};

class Dir
{
private:
	std::vector<FileInfo> m_vFile;
public:
	Dir();
	Dir(const char *szDir);
	~Dir();
public:
	bool Scan(const char *szDir);
	const std::vector<FileInfo>& Files(void);
private:
	void SetFileInfo(FileInfo& fi,struct _finddata_t *fd);
};

inline const std::vector<FileInfo>& Dir::Files(void)
{
	return m_vFile;
}

class DirectoryScanner
{
public:
	struct Item
		{
		bool IsDirectory;
		std::string Name;
		std::string FullName;
		std::string Ext;
	};
	void SetPath(const std::string &BasePath, const std::string &Masks);//маски в виде "*.exe;?.bak";
	virtual bool OnItem(const Item &it)=0;
private:
	void Traverse(const std::string &BasePath,const std::string &Mask);
	std::string m_BasePath;
	typedef std::vector<Item> FilesSet;
	FilesSet m_Files;
};
#endif	//_DIR_H_