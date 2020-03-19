#if !defined(__EMITTER_MANAGER_INCLUDED__)
#define __EMITTER_MANAGER_INCLUDED__

#include <list>
#include <map>
#include <windows.h>
class cc_SndEmitter;
class cc_SndScript;

class cc_SegmentState;

class cc_SndVolume
{
public:
	virtual void updateVolume(float) = 0;
};

class cc_CS;
class cc_SegmentMgr;
class cc_PathMgr;
class cc_SegmentStateMgr;

class cc_EmitMgr
{
	cc_SegmentMgr *m_segmentMgr;
	cc_PathMgr *m_pathMgr;
	cc_SegmentStateMgr *m_sstateMgr;

	static const char *m_LogFile;
	static int m_Counter;

	typedef std::list<cc_SndEmitter *> emitters_t;
	emitters_t m_Referenced;
	emitters_t m_NotReferenced;

	typedef std::list<cc_SndVolume *> snd_sources_t;
	typedef std::map< ci_Sound::Channel, snd_sources_t > volume_data_t;
	volume_data_t m_Volume;

	typedef std::list<cc_SegmentState *> snd_states_t;
	snd_states_t m_toUpdate;

	cc_CS *m_QueueLock;

public:
	cc_EmitMgr(cc_SegmentMgr *, cc_PathMgr *, cc_SegmentStateMgr *);
	~cc_EmitMgr();

	// Инициализация/деинициализация
//	void init();
	void manage();
//	void shut();

	enum Params {pDestroyAfterStop = 1<<0, pPlay = 1<<1};

	cc_SndEmitter *emitSound (cc_SndScript *, const char *wave, unsigned params);
	cc_SndEmitter *emitSound (cc_SndScript *, unsigned params);

	void reference(cc_SndEmitter *);
	void dereference(cc_SndEmitter *);

	void updateOnNextFrame(cc_SegmentState *);

	inline cc_SegmentMgr *segmentMgr() { return m_segmentMgr; }
	inline cc_PathMgr *pathMgr() { return m_pathMgr; }
	inline cc_SegmentStateMgr *sstateMgr() { return m_sstateMgr; }
};

#endif