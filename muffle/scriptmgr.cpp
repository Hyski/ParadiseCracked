#include "precomp.h"
#include "scriptmgr.h"
#include "SoundScriptLexer.hpp"
#include "SoundScriptRecorder.h"

// TEST!!!
#include <fstream>
// TEST

//=====================================================================================//
//                               ScriptMgr::ScriptMgr()                                //
//=====================================================================================//
ScriptMgr::ScriptMgr(ci_VFileSystem *vfs)
{
	ci_VDirIt *it = vfs->GetDir("scripts\\sound\\*.seng");

	for(; !it->isDone(); it->next())
	{
		const std::string &name = it->getFullName();
		ci_VFile *file = vfs->CreatFile(name.c_str(),ci_VFileSystem::READ);
		std::istrstream in((char *)file->getData(),file->Size());
		SoundScriptLexer lexer(in);
		SoundScriptRecorder parser(lexer,this);

		try
		{
			parser.parse();
		}
		catch(const std::exception &e)
		{
			MessageBox(NULL,e.what(),"ScriptMgr",MB_OK);
		}
		file->Release();
	}

	it->Release();

	dump();
}

//=====================================================================================//
//                             Script *ScriptMgr::insert()                             //
//=====================================================================================//
Script *ScriptMgr::insert(const std::string &name, ISound::Channel channel)
{
	KERNEL_LOG("Зарегистрирован скрипт [" << name << "]\n");
	return &m_scripts.insert(Scripts_t::value_type(name,Script(name,channel))).first->second;
//	return /* &(m_scripts[name] = */new Script(name,channel);//);
}

//=====================================================================================//
//                              Script *ScriptMgr::get()                               //
//=====================================================================================//
const Script &ScriptMgr::get(const std::string &name) const
{
	assert( m_scripts.find(name) != m_scripts.end() );
	return m_scripts.find(name)->second;
}

//=====================================================================================//
//                               void ScriptMgr::dump()                                //
//=====================================================================================//
void ScriptMgr::dump() const
{
	for(Scripts_t::const_iterator i = m_scripts.begin(); i != m_scripts.end(); ++i)
	{
		KERNEL_LOG(i->second);
	}
}