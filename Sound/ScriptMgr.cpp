#include "precomp.h"
#include "ScriptMgr.h"
#include "Script.h"
#include "DX8Sound.h"
#include "SndScriptWrapper.h"
#include <interfaces/IFileSystem.h>
#include <interfaces/IFile.h>

LOG_MEMBER_IMPL(cc_ScriptMgr,m_log,"sound/scripts.log");
#define SCRIPT_LOG	C_HOMELOG(m_log)

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// cc_ScriptMgr::cc_ScriptMgr() //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_ScriptMgr::cc_ScriptMgr(cc_SegmentMgr *segmentMgr)
:	m_segmentMgr(segmentMgr)
{
	TRACK_FUNC(cc_ScriptMgr::cc_ScriptMgr());
	SCRIPT_LOG << STR_FULL_STAMP << "Script manager initializing...\n";

	const char *scriptDir = "scripts\\sound\\";
	const char *scriptFiles = "scripts\\sound\\*.seng";

	// Пропарсим все скрипты в каталоге "scripts/sound"
	ci_DirIt *itor = CARCASS->GetFileSystem()->GetDir(scriptFiles);
	for (; !itor->isDone(); itor->next())
	{
		SCRIPT_LOG << STR_FULL_STAMP << "Preparing [" << itor->getFullName() << "] for parsing...\n";
		cc_SndScriptWpr::parse((
//#if defined(_UNDER_CARCASS)
//#pragma SMART_WARNING("Исправить эту фишку для Каркаса")
//		scriptDir+
//#endif
			itor->getFullName()).c_str(),this,m_segmentMgr);
	}
	itor->Release();

	SCRIPT_LOG << STR_FULL_STAMP << "Script manager initialized\n";
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// cc_ScriptMgr::~cc_ScriptMgr() /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_ScriptMgr::~cc_ScriptMgr()
{
	TRACK_FUNC(cc_ScriptMgr::~cc_ScriptMgr());
	SCRIPT_LOG << STR_FULL_STAMP << "Script manager uninitializing...\n";
	scripts_t::iterator itor = m_Scripts.begin();

	for (; itor != m_Scripts.end(); itor++)
	{
		SCRIPT_LOG << "\t...deleting script " << itor->first << " - ";
		delete itor->second;
		SCRIPT_LOG << "done!\n";
	}

	m_Scripts.clear();

	SCRIPT_LOG << STR_FULL_STAMP << "Script manager uninitialized\n";
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_ScriptMgr::addScript () ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_ScriptMgr::addScript (cc_SndScript *script)
{
	TRACK_FUNC(cc_ScriptMgr::addScript());
	scripts_t::iterator i = m_Scripts.find(script->getName());
	if (i != m_Scripts.end())
	{
		SCRIPT_LOG << STR_FULL_STAMP << "Duplicate script name " << script->getName() << ". Script not added\n";
		return;
	}

	m_Scripts[script->getName()] = script;
	SCRIPT_LOG << STR_FULL_STAMP << "Added script " << script->getName() << "\n";
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// cc_ScriptMgr::getScript() ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_SndScript *cc_ScriptMgr::getScript(const char *scriptName)
{
	TRACK_FUNC(cc_ScriptMgr::getScript());
	scripts_t::iterator i = m_Scripts.find(scriptName);
	if (i == m_Scripts.end())
	{
		SCRIPT_LOG << "<" << STR_TIME_STAMP << "> " << STR_FULL_STAMP << "Script [" << scriptName << "] not found\n";
		return 0;
	}
	return i->second;
}