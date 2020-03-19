#include "precomp.h"
#include "Script.h"
#include "DX8Sound.h"
#include "Segment.h"

#include "DM8Core.h"

cc_SndParams::cc_SndParams()
{
}

cc_SndParams::cc_SndParams(const cc_SndParams &params)
:	m_Type(params.m_Type),
	m_Disable3D(params.m_Disable3D),
	m_Pan(params.m_Pan),
	m_ConeIn(params.m_ConeIn),
	m_ConeOut(params.m_ConeOut),
	m_ConeOutVol(params.m_ConeOutVol),
	m_MinDist(params.m_MinDist),
	m_MaxDist(params.m_MaxDist),
	m_Repeat(params.m_Repeat),
	m_Volume(params.m_Volume),
	m_Wave(params.m_Wave),
	m_Permanent(params.m_Permanent)
{
}

cc_SndScript::cc_SndScript(const cc_SndParams *params, const char *name, cc_SegmentMgr *mgr)
:	cc_SndParams(*params),
	m_mgr(mgr),
	m_Name(name)
{
	m_Segment = m_mgr->getSegmentRef(m_Wave.c_str());
}

const cc_SndParams *cc_SndScript::getParams() const
{
	return this;
}

const std::string &cc_SndScript::getName() const
{
	return m_Name;
}

cc_Segment *cc_SndScript::getSegment()
{
	return m_Segment;
}

void cc_SndScript::Release()
{
}