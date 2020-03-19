#if !defined(__SCRIPT_MANAGER_INCLUDED__)
#define __SCRIPT_MANAGER_INCLUDED__

#include <map>

class cc_DirectMusic;
class cc_SndScript;
class cc_SegmentMgr;

class cc_ScriptMgr
{
	static const char *m_LogFile;

	typedef std::map<std::string,cc_SndScript *> scripts_t;
	scripts_t m_Scripts;

	DECLARE_LOG_MEMBER(m_log);

	cc_SegmentMgr *m_segmentMgr;

public:
	cc_ScriptMgr(cc_SegmentMgr *);
	~cc_ScriptMgr();
	// Инициализация/деинициализация
//	void init();
//	void shut();

	cc_SndScript *getScript(const char *scriptName);

	// Для загрузчика
	void addScript(cc_SndScript *);
};

#endif