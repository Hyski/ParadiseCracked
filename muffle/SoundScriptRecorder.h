#if !defined(__SOUND_SCRIPT_RECORDER_H_INCLUDED_1104954923325159__)
#define __SOUND_SCRIPT_RECORDER_H_INCLUDED_1104954923325159__

#include "SoundScriptParser.hpp"

class ScriptMgr;

//=====================================================================================//
//                class SoundScriptRecorder : public SoundScriptParser                 //
//=====================================================================================//
class SoundScriptRecorder : private SoundScriptParser
{
	ScriptMgr *m_mgr;

private:
	Script *createScript(const std::string &name, ISound::Channel);

public:
	SoundScriptRecorder(class SoundScriptLexer &, ScriptMgr *);
	void parse();
};

#endif // !defined(__SOUND_SCRIPT_RECORDER_H_INCLUDED_1104954923325159__)