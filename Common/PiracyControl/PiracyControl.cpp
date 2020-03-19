/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2002

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   11.07.2002

************************************************************************/
#include "Precomp.h"
#include "PiracyControl.h"

#include "../Utils/Dir.h"
#include <sys/stat.h>

//	получение списка мультов и их размеров
const PiracyControl::MultInfo& PiracyControl::getMultInfo(const std::string& path)
{
	static MultInfo mi;
	Dir dir((path+"*.bik").c_str());
	const unsigned int file_num = dir.Files().size();
	const unsigned int max_possible_files = 20;
	static struct _stat st;

	mi.m_quantity = 0;
	mi.m_size = 0;

	if(file_num < max_possible_files)
	{
		mi.m_quantity = file_num;
		for(int i=0;i<file_num;++i)
		{
			_stat((path+dir.Files()[i].full_name).c_str(),&st);
			mi.m_size += st.st_size;
		}
	}

	return mi;
}

