#include "precomp.h"
#include "../../sound/ISound.h"
#include "../Shell/Shell.h"
#include "FileSystem.h"

//=====================================================================================//
//                          void PCSndServices::dbg_printf()                           //
//=====================================================================================//
void SndServices::dbg_printf(short x, short y, const char *str)
{
	std::string tmp_str(str);

//	for(unsigned pos = 0; pos < tmp_str.length(); ++pos)
//	{
//		pos = tmp_str.find("%",pos);
//		tmp_str.replace(pos,1,"%%");
//	};

	DebugInfo::Add(x,y,const_cast<char *>(tmp_str.c_str()));
}
