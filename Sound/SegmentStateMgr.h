#if !defined(__SEGMENT_STATE_MANAGER_INCLUDED__)
#define __SEGMENT_STATE_MANAGER_INCLUDED__

class cc_SndSpy;
class cc_EmitMgr;
class cc_DirectMusic;

class cc_SegmentStateMgr
{
	cc_SndSpy *m_spy;
	cc_EmitMgr *m_emitMgr;
	cc_DirectMusic *m_music;

public:
	cc_SegmentStateMgr(cc_SndSpy *, cc_EmitMgr *, cc_DirectMusic *);

	inline cc_SndSpy *spy() { return m_spy; }
	inline cc_EmitMgr *emitMgr() { return m_emitMgr; }
	inline cc_DirectMusic *music() { return m_music; }
};

#endif