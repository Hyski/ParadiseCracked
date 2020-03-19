/***********************************************************************

                          Directory Explorer

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   01.08.2000

************************************************************************/
#include "Precomp.h"
#include "Dir.h"

Dir::Dir()
{
}

Dir::Dir(const char *szDir)
{
	Scan(szDir);
}

Dir::~Dir()
{
}

bool Dir::Scan(const char *szDir)
{
	FileInfo fi;
	struct _finddata_t fd;
	long hFile;
	
	m_vFile.clear();
	if((hFile =_findfirst(szDir,&fd)) != -1L ) 
    {
		SetFileInfo(fi,&fd);
		m_vFile.push_back(fi);

		while(_findnext(hFile,&fd)==0)
		{
			SetFileInfo(fi,&fd);
			m_vFile.push_back(fi);
		}

		_findclose(hFile); // GvozdodeR
 
		return true;
	}

	return false;
}

void Dir::SetFileInfo(FileInfo& fi,struct _finddata_t *fd)
{
	static char drive[_MAX_DRIVE];
	static char dir[_MAX_DIR];
	static char fname[_MAX_FNAME];
	static char ext[_MAX_EXT];

	_strlwr(fd->name);
	_splitpath(fd->name,drive,dir,fname,ext);
	fi.full_name = fd->name;
	fi.name = fname;
	fi.extension = ext;
    fi.attr = FileInfo::NORMAL;
    if(fd->attrib&_A_RDONLY) fi.attr|=FileInfo::READONLY;
    if(fd->attrib&_A_SYSTEM) fi.attr|=FileInfo::SYSTEM;
    if(fd->attrib&_A_HIDDEN) fi.attr|=FileInfo::HIDDEN;
    if(fd->attrib&_A_ARCH)   fi.attr|=FileInfo::ARCHIVE;
    if(fd->attrib&_A_SUBDIR) fi.attr|=FileInfo::SUBDIR;
}
void DirectoryScanner::SetPath(const std::string &BasePath, const std::string &masks)
{

	m_Files.clear();
   std::string::size_type t;
	 std::string Masks=masks;
	 std::string Mask;
	 while(((t=Masks.find_first_of(",;\t"))!=Masks.npos)||Masks.size()) 
    {
	//1. составить список масок
    if(t==Masks.npos)   t=Masks.size();
    Mask=std::string(&Masks[0],t);
    Masks.erase(0,t);
    while(Masks.find_first_of(",;\t")==0) Masks.erase(0,1);
	//2. вызвать для каждой Traverse
		Traverse(BasePath,Mask);
		}
	//3. для собранных файлов вызвать callback
	 FilesSet ::iterator it=m_Files.begin(),ite=m_Files.end();
	 for(;it!=ite;it++)
		 {
		 OnItem(*it);
	 }
}
void DirectoryScanner::Traverse(const std::string &BasePath,const std::string &Mask)
{
	int i;
	std::string t(BasePath+std::string("/")+Mask);
		Dir pack(t.c_str());
		for(i=0;i<pack.Files().size();i++)
		{
			Item NewItem;
			const FileInfo &fi=pack.Files()[i];
			if(!fi.name.size()) continue;
			NewItem.IsDirectory=fi.attr&FileInfo::SUBDIR;
			if(NewItem.IsDirectory) continue;
			if(fi.name=="."||fi.name=="..") continue;
			NewItem.Ext=fi.extension;
			NewItem.FullName=BasePath+"/"+fi.name+fi.extension;
			NewItem.Name=fi.name+fi.extension;
			m_Files.push_back(NewItem);
		}
		Dir pack1((BasePath+std::string("/*.*")).c_str());
		for(i=0;i<pack1.Files().size();i++)
		{
			Item NewItem;
			const FileInfo &fi=pack1.Files()[i];
			if(!fi.name.size()) continue;
			NewItem.IsDirectory=fi.attr&FileInfo::SUBDIR;
			if(!NewItem.IsDirectory)	continue;
			if(fi.name=="."||fi.name=="..") continue;
			NewItem.Ext=fi.extension;
			NewItem.FullName=BasePath+"/"+fi.name;
			NewItem.Name=fi.name;
			if(NewItem.IsDirectory)
			{
				if(OnItem(NewItem))
					Traverse(BasePath+"/"+fi.name,Mask);
			}
		}
}

