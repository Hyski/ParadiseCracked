#include "precomp.h"
#include "EmitMgr.h"
#include "Emitter.h"
#include "DX8Sound.h"

#include "SegmentState.h"
#include "CriticalSection.h"

#include <assert.h>
#include <stdio.h>

const char *cc_EmitMgr::m_LogFile = "gvz_sound_emit.log";

cc_EmitMgr::cc_EmitMgr(cc_SegmentMgr *segmentMgr, cc_PathMgr *pathMgr, cc_SegmentStateMgr *sstateMgr)
:	m_segmentMgr(segmentMgr),
	m_pathMgr(pathMgr),
	m_sstateMgr(sstateMgr)
{
	TRACK_FUNC(cc_EmitMgr::cc_EmitMgr());
	m_QueueLock = cc_CSEntry::initializeCriticalSection();
	EMIT_LOG(m_LogFile, STR_FULL_STAMP+"Emit manager initialized\n");
}

cc_EmitMgr::~cc_EmitMgr()
{
	TRACK_FUNC(cc_EmitMgr::~cc_EmitMgr());

	EMIT_LOG(m_LogFile, STR_FULL_STAMP+"Shutting down emit manager\n");

	manage();

	while (!m_NotReferenced.empty())
	{
		if ((*m_NotReferenced.begin())->isPlaying()) (*m_NotReferenced.begin())->stop();
		delete *m_NotReferenced.begin();
		m_NotReferenced.erase(m_NotReferenced.begin());
	}

	while (!m_Referenced.empty())
	{
		EMIT_LOG(m_LogFile, STR_FULL_STAMP+"Destroying active emitter ["+(*m_Referenced.begin())->getName()+"]\n");
		if ((*m_Referenced.begin())->isPlaying()) (*m_Referenced.begin())->stop();
		delete *m_Referenced.begin();
		m_Referenced.erase(m_Referenced.begin());
	}

	while(cc_SegmentState::m_instanceCount>0) manage();

	EMIT_LOG(m_LogFile, STR_FULL_STAMP+"Emit manager shut down\n");

	cc_CSEntry::deleteCriticalSection(m_QueueLock);
}

cc_SndEmitter *cc_EmitMgr::emitSound (cc_SndScript *script, const char *wave, unsigned flags)
{
	TRACK_FUNC(cc_EmitMgr::emitSound());
	cc_SndEmitter *emitter = new cc_SndEmitter(script,wave,this);
	if (flags&pPlay) emitter->play();
	m_NotReferenced.push_back(emitter);
	return emitter;
}

cc_SndEmitter *cc_EmitMgr::emitSound (cc_SndScript *script, unsigned flags)
{
	TRACK_FUNC(cc_EmitMgr::emitSound());
	cc_SndEmitter *emitter = new cc_SndEmitter(script,this);
	if (flags&pPlay) emitter->play();
	m_NotReferenced.push_back(emitter);
	return emitter;
}

void cc_EmitMgr::reference(cc_SndEmitter *emitter)
{
	TRACK_FUNC(cc_EmitMgr::reference());
	if(emitter->getRefCount() == 1)
	{
		emitters_t::iterator itor = m_NotReferenced.begin();
		for (; itor != m_NotReferenced.end(); itor++)
		{
			if (emitter == *itor)
			{
				m_NotReferenced.erase(itor);
				break;
			}
		}
		m_Referenced.push_front(emitter);
	}
}

void cc_EmitMgr::dereference(cc_SndEmitter *emitter)
{
	TRACK_FUNC(cc_EmitMgr::dereference());
	if (emitter->getRefCount() == 0)
	{
		emitters_t::iterator itor = m_Referenced.begin();
		for (; itor != m_Referenced.end(); itor++)
		{
			if (*itor == emitter)
			{
				m_Referenced.erase(itor);
				break;
			}
		}
		if (!emitter->isPlaying())
		{
			delete emitter;
		}
		else
		{
			m_NotReferenced.push_front(emitter);
		}
	}
}

void cc_EmitMgr::manage()
{
	TRACK_FUNC(cc_EmitMgr::manage());

	EMIT_LOG(m_LogFile,"<"+STR_TIME_STAMP+"> "+STR_FULL_STAMP+"Started emit management\n");
	{
		emitters_t::iterator itor = m_NotReferenced.begin();
		for (; itor != m_NotReferenced.end(); itor++)
		{
			(*itor)->update();
			if (!(*itor)->isPlaying())
			{
				delete *itor;
				itor = m_NotReferenced.erase(itor);
			}
		}
	}

	{
		emitters_t::iterator itor = m_Referenced.begin();
		for (; itor != m_Referenced.end(); itor++)
		{
			(*itor)->update();
		}
	}

	EMIT_LOG(m_LogFile,"\t\t...segment state count is "+cc_DM8Error::unsigned2String(cc_SegmentState::m_instanceCount)+"\n");
#if defined(TURN_ON_ALL_DEBUG_STUFF)
	cc_SegmentState::flushDebugInfo();
#endif

	{
//		CORE_LOG(cc_SegmentState::m_LogFile, "<"+STR_TIME_STAMP+"> "+STR_FULL_STAMP+"========================================== entered state update\n");
		snd_states_t::iterator itor = m_toUpdate.begin();
		for (; itor != m_toUpdate.end() ;)
		{
			(*itor)->update();
			itor = m_toUpdate.erase(itor);
		}
//		CORE_LOG(cc_SegmentState::m_LogFile, "<"+STR_TIME_STAMP+"> "+STR_FULL_STAMP+"========================================== leaved state update\n");
	}
	EMIT_LOG(m_LogFile,"<"+STR_TIME_STAMP+"> "+STR_FULL_STAMP+"Finished emit management\n");
}

void cc_EmitMgr::updateOnNextFrame(cc_SegmentState *sst)
{
	assert(0 != sst);

	cc_CSEntry guard(m_QueueLock);
	m_toUpdate.push_front(sst);
}