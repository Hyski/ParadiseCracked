#include "precomp.h"
#include "SndScriptWrapper.h"

#include "pcctscfg.h"
#include "pccts_stdio.h"
#include "tokens.h"

#include "AParser.h"
#include "DLGLexer.h"
#include "cc_SndScriptParser.h"

#include <interfaces/IFile.h>

//#include "ScriptHandles.h"
//#include "SoundScript.h"

#include "DX8Sound.h"

namespace
{
	LOG_IMPL(g_log,"gvz_sound_parser.log");
}

#define SW_LOG	C_HOMELOG(g_log)

class cc_VFSInputStream : public DLGInputStream
{
	ci_File *file;

public:
	cc_VFSInputStream (const char *fileName)
	{
		TRACK_FUNC(cc_VFSInputStream::cc_VFSInputStream());
		ci_FileSystem *fs = CARCASS->GetFileSystem();
		file = fs->CreatFile(fileName, ci_FileSystem::READ);
		if (!file)
		{
			SW_LOG << STR_FULL_STAMP << "Cannot open file " << fileName << "\n";
		}
	}

	~cc_VFSInputStream ()
	{
		if (file) file->Release();
	}

	int nextChar ()
	{
		char ch;
		if (file->Read(&ch,1,1))
		{
			return ch;
		}
		return EOF;
	}
};

void cc_SndScriptWpr::parse(const char *fileName, cc_ScriptMgr *scriptMgr, cc_SegmentMgr *segmentMgr)
{
	cc_VFSInputStream input(fileName);
	DLGLexer scan(&input);
	ANTLRTokenBuffer pipe(&scan);
	ANTLRCommonToken *aToken = new ANTLRCommonToken;
	scan.setToken(aToken);
	cc_SndScriptParser parser(&pipe);
	parser.setManagers(scriptMgr,segmentMgr);
	parser.init();
	parser.start();
	delete aToken;
}