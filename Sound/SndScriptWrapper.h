#if !defined(__SOUND_SCRIPT_PARSER_WRAPPER_INCLUDED__)
#define __SOUND_SCRIPT_PARSER_WRAPPER_INCLUDED__

class cc_ScriptHdl;
class cc_SMParserFacet;
class cc_ScriptMgr;
class cc_SegmentMgr;

class cc_SndScriptWpr
{
public:
	static void parse (const char *fileName, cc_ScriptMgr *, cc_SegmentMgr *);
};

#endif