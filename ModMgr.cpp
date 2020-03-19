//=====================================================================================//
//                                                                                     //
//                                  Paradise Cracked                                   //
//                                                                                     //
//                             Copyright by MiST land 2002                             //
//                                                                                     //
//   -------------------------------------------------------------------------------   //
//   Description:                                                                      //
//                                                                                     //
//   -------------------------------------------------------------------------------   //
//   Author: Pavel A.Duvanov                                                           //
//   Date:   29.11.2002                                                                //
//                                                                                     //
//=====================================================================================//
#include "Precomp.h"

#if 0

#include "ModMgr.h"

//#include <mll/utils/functional.h>
//#include <mll/io/manip.h>
#include <algorithm>
#include <fstream>

#include "Common/Utils/Dir.h"

//=====================================================================================//
//                                      class Mod                                      //
//=====================================================================================//
Mod::Mod(const std::string& id) : m_id(id),m_isValid(false)
{
	if(id.size())
	{
		const std::string info_file_name = ModMgr::m_modPath+id+std::string("/")+ModMgr::m_modInfo;
		std::ifstream file(info_file_name.c_str(),std::ios_base::binary);

		if(file.good())
		{
			try
			{
				file >> mll::io::text(" name   = ") >> mll::io::exact(m_name  );
				file >> mll::io::text(" author = ") >> mll::io::exact(m_author);
				file >> mll::io::text(" desc   = ") >> mll::io::exact(m_desc  );

				m_isValid = true;
			}
			catch(std::exception&)
			{
			}
		}
	}
}

Mod::~Mod()
{
}

//=====================================================================================//
//                                    class ModMgr                                     //
//=====================================================================================//
const std::string ModMgr::m_modPath = "mods/";			//	путь до каталога с модами
const std::string ModMgr::m_modInfo = "info.txt";		//	наименование файлика с описанием мода

ModMgr::ModMgr()
{
	typedef std::vector<FileInfo> files_t;
	files_t files;
	Dir scaner;

	scaner.Scan((m_modPath+"*").c_str());
	files = scaner.Files();

	for(files_t::iterator it=files.begin();it!=files.end();++it)
	{
		if(it->attr&FileInfo::SUBDIR)
		{
			if(it->name != std::string(".") && it->name != std::string(""))
			{
				std::auto_ptr<Mod> mod = std::auto_ptr<Mod>(new Mod(it->name));
				if(mod->isValid())
				{
					m_mods.push_back(mod.release());
				}
			}
		}
	}
}

ModMgr::~ModMgr()
{
	std::for_each(m_mods.begin(),m_mods.end(),mll::utils::deleter<Mod>());
}

#endif