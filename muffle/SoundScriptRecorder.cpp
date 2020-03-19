#include "precomp.h"
#include "SoundScriptRecorder.h"
#include "SoundScriptLexer.hpp"
#include "scriptmgr.h"

//=====================================================================================//
//                     SoundScriptRecorder::SoundScriptRecorder()                      //
//=====================================================================================//
SoundScriptRecorder::SoundScriptRecorder(SoundScriptLexer &lexer, ScriptMgr *mgr)
:	SoundScriptParser(lexer),
	m_mgr(mgr)
{
}

//=====================================================================================//
//                          void SoundScriptRecorder::parse()                          //
//=====================================================================================//
void SoundScriptRecorder::parse()
{
	program();
}

//=====================================================================================//
//                     Script *SoundScriptRecorder::createScript()                     //
//=====================================================================================//
Script *SoundScriptRecorder::createScript(const std::string &name,
										  ISound::Channel channel)
{
	return m_mgr->insert(name,channel);
}