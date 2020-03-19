#if !defined(__SOUND_EMITTER_INCLUDED__)
#define __SOUND_EMITTER_INCLUDED__

#include COMPTR_H
#include <dmusici.h>
#include "Spy.h"
#include "EmitMgr.h"
#include "DX8Sound.h"
#include "PathMgr.h"

class cc_SndScript;
class cc_Segment;
class cc_EmitMgr;

class cc_SndEmitter : public ci_SndEmitter
{
	static const char *m_LogFile;
	static int m_emitCount;

	std::string m_name;

	cc_EmitMgr *m_emitMgr;

	cc_SndScript *m_Script;
	cc_AudioPath *m_Path;
	cc_Segment *m_Segment;
	mutable int m_RefCount;

	typedef std::list<cc_SegmentState *> segStates_t;
	segStates_t m_playing;

	void commonInit();

	snd_vector m_AbsolutePos;
	snd_vector m_velocity;

/*	void initializeSegment();
	void prepareForPlaying();*/
	void adjustParameters();

	void setNoMute();

public:
	cc_SndEmitter(cc_SndScript *, const char *wave, cc_EmitMgr *);
	cc_SndEmitter(cc_SndScript *, cc_EmitMgr *);
	~cc_SndEmitter();

	ci_SndEmitter *clone();

	void play ();
	void stop ();

	void update();

	// В процессе...
//	virtual void tieToEntity (?) = 0

	// Установить позицию звука
	void setPosition (const snd_vector &);
	void setVelocity (const snd_vector &);
	// etc.

	void setDistances (float minDist, float maxDist);

	// Возвращает состояние источника
	bool isPlaying ();

	void Release ();

	int getRefCount() const {return m_RefCount;}
	const std::string &getName() const {return m_name;}
};

#endif