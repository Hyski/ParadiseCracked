#if !defined(__SCRIPTMGR_H_INCLUDED_0442986472349163__)
#define __SCRIPTMGR_H_INCLUDED_0442986472349163__

#include "script.h"
#include <hash_map>

//=====================================================================================//
//                                   class ScriptMgr                                   //
//=====================================================================================//
class ScriptMgr : private noncopyable
{
	typedef std::hash_map<std::string,Script> Scripts_t;
	Scripts_t m_scripts;

	friend class SoundScriptRecorder;

	Script *insert(const std::string &, ISound::Channel);
	void dump() const;

public:
	ScriptMgr(ci_VFileSystem *);

	const Script &get(const std::string &) const;
};

#endif // !defined(__SCRIPTMGR_H_INCLUDED_0442986472349163__)