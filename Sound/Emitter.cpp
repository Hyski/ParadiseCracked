#include "precomp.h"
#include "Emitter.h"
#include "Script.h"
#include "DX8Sound.h"
#include "Segment.h"
#include "PathMgr.h"
#include "SegmentState.h"

#if defined(_UNDER_CARCASS_)
//#include <3d/cc_vector3.h>
#include <mll/algebra/vector3.h>
#else
#include "../Common/3D/Geometry.h"
#endif

#include <stdio.h>
#include <assert.h>

#include <sstream>
#include <iomanip>
#include "DM8Core.h"

const char *cc_SndEmitter::m_LogFile = "gvz_sound_emitter.log";
static int g_emitCtr = 0;

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::cc_SndEmitter() /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

cc_SndEmitter::cc_SndEmitter(cc_SndScript *script, const char *wave, cc_EmitMgr *mgr)
:	m_emitMgr(mgr),
	m_Script(script),
	m_RefCount(0),
	m_AbsolutePos(0.0f,0.0f,0.0f),
	m_Path(0)
{
	std::ostringstream buf;
	buf<<"emitter"<<g_emitCtr++;
	m_name = buf.str();

	m_Segment = m_emitMgr->segmentMgr()->getSegment(wave);
	EMIT_LOG(m_LogFile,"Created emitter ["+m_name+"]\n");
}

cc_SndEmitter::cc_SndEmitter(cc_SndScript *script, cc_EmitMgr *mgr)
:	m_emitMgr(mgr),
	m_Script(script),
	m_RefCount(0),
	m_Path(0)
{
	std::ostringstream buf;
	buf<<"emitter"<<g_emitCtr++;
	m_name = buf.str();

	if (m_Script)
	{
		m_Segment = m_emitMgr->segmentMgr()->getSegment(m_Script->getParams()->m_Wave.c_str());
	}
	else
	{
		m_Segment = 0;
	}
	EMIT_LOG(m_LogFile,"Created emitter ["+m_name+"]\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::commonInit() ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void cc_SndEmitter::commonInit()
{
	TRACK_FUNC(cc_SndEmitter::commonInit());
	cc_PathMgr::PathParameters pp;

	EMIT_LOG(m_LogFile,"<"+STR_TIME_STAMP+"> "+STR_FULL_STAMP+"The script type is "+cc_DM8Error::unsigned2String(m_Script->getParams()->m_Type)+"\n");

	if (!m_Script->getParams()->m_Disable3D)
	{
		pp.m_Params = cc_PathMgr::PathParameters::use3D;
	}
	else if (m_Script->getParams()->m_Type == ci_Sound::cAmbient)
	{
		pp.m_Params = cc_PathMgr::PathParameters::ambient;
	}
	else if (m_Script->getParams()->m_Type == ci_Sound::cEffects)
	{
		pp.m_Params = cc_PathMgr::PathParameters::effects;
	}
	else
	{
		pp.m_Params = cc_PathMgr::PathParameters::other2D;
	}

	m_Path = m_emitMgr->pathMgr()->getAudioPath(pp,m_Script->getParams()->m_Type);
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::~cc_SndEmitter() ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
cc_SndEmitter::~cc_SndEmitter()
{
	TRACK_FUNC(cc_SndEmitter::~cc_SndEmitter());
	stop();
	if (isPlaying())
	{
		EMIT_WARN_LOG(m_LogFile,STR_FULL_STAMP+"Destroyed playing emitter ["+m_name+"]\n");
	}
	else
	{
		EMIT_LOG(m_LogFile,STR_FULL_STAMP+"Destroyed emitter ["+m_name+"]\n");
	}

	if (m_Path)
	{
		delete m_Path;
		m_Path = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::clone() /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
ci_SndEmitter *cc_SndEmitter::clone()
{
	m_RefCount++;
	m_emitMgr->reference(this);
	return this;
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::play() //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void cc_SndEmitter::play()
{
	TRACK_FUNC(cc_SndEmitter::play());

	if (!m_Script) return;
	if (m_Script->getParams()->m_Type == ci_Sound::cThemes) return;

	if (!m_Segment)
	{
		EMIT_WARN_LOG(m_LogFile,"<"+STR_TIME_STAMP+"> "+STR_FULL_STAMP+"Cannot play ["+m_name+"] due to segment absence\n");
		return;
	}

	if (!m_Script)
	{
		EMIT_WARN_LOG(m_LogFile,"<"+STR_TIME_STAMP+"> "+STR_FULL_STAMP+"Cannot play ["+m_name+"] due to script absence\n");
		return;
	}

	EMIT_LOG(m_LogFile,"<"+STR_TIME_STAMP+"> "+STR_FULL_STAMP+"Trying to play ["+m_name+"]\n");
	if(!m_Path) commonInit();
	cc_SegmentState *sst = new cc_SegmentState(m_Segment,m_emitMgr->sstateMgr());
	if(!sst->play(m_Path->getPath(),m_Script))
	{
		sst->release();
	}
	else
	{
		m_playing.push_back(sst);
		adjustParameters();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::adjustParameters() //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void cc_SndEmitter::adjustParameters()
{
	setDistances(m_Script->getParams()->m_MinDist,m_Script->getParams()->m_MaxDist);
	setPosition(m_AbsolutePos);
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::stop() //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void cc_SndEmitter::stop()
{
	while (!m_playing.empty())
	{
		(*m_playing.begin())->release();
		m_playing.erase(m_playing.begin());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::setPosition() ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void cc_SndEmitter::setPosition(const snd_vector &pos)
{
	m_AbsolutePos = pos;

	if (!m_Script) return;
	if (m_Script->getParams()->m_Disable3D) return;

	if (m_Path)
	{
		snd_vector tmp = m_AbsolutePos;
		cc_DX8Sound::m_Sound->translatePosition(tmp);
		m_Path->setPosition(tmp);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::update() ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void cc_SndEmitter::update()
{
	if(!isPlaying())
	{
		if(m_Path)
		{
			delete m_Path;
			m_Path = 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::setDistances() //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void cc_SndEmitter::setDistances(float minDist, float maxDist)
{
	if (!m_Script) return;
	if (m_Script->getParams()->m_Disable3D) return;

	if (m_Path)
	{
		if (m_Path->is2D()) return;

		IDirectSound3DBuffer8 *buffer;
		DWORD dwBuffer = 0;

		while (S_OK == m_Path->getPath()->GetObjectInPath(DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, dwBuffer++, GUID_NULL, 0, IID_IDirectSound3DBuffer8, reinterpret_cast<void * *>(&buffer)))
		{
			buffer->SetMaxDistance(maxDist, DS3D_IMMEDIATE);
			buffer->SetMinDistance(minDist, DS3D_IMMEDIATE);
			buffer->Release();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::setVelocity() ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void cc_SndEmitter::setVelocity(const snd_vector &vel)
{
	if(!m_Script) return;
	if(m_Script->getParams()->m_Disable3D) return;

	m_velocity = vel;

	if(m_Path)
	{
		if(m_Path->is2D()) return;

		IDirectSound3DBuffer8 *buffer;
		DWORD dwBuffer = 0;

		while(S_OK == m_Path->getPath()->GetObjectInPath(DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, dwBuffer++, GUID_NULL, 0, IID_IDirectSound3DBuffer8, reinterpret_cast<void * *>(&buffer)))
		{
			buffer->SetVelocity(m_velocity.x, m_velocity.y, m_velocity.z, DS3D_IMMEDIATE);
			buffer->Release();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::isPlaying() /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
bool cc_SndEmitter::isPlaying()
{
	segStates_t::iterator itor = m_playing.begin();
	for (; itor != m_playing.end() ;)
	{
		if ((*itor)->isFinished())
		{
			(*itor)->release();
			itor = m_playing.erase(itor);
		}
		else ++itor;
	}

	if (m_playing.empty()) return false;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndEmitter::Release() ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void cc_SndEmitter::Release()
{
	TRACK_FUNC(cc_SndEmitter::Release());
	EMIT_LOG(m_LogFile,"<"+STR_TIME_STAMP+"> "+STR_FULL_STAMP+"Released emitter ["+m_name+"]\n");
	m_RefCount--;
	m_emitMgr->dereference(this);
}