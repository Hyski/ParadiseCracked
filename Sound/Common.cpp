#include "precomp.h"
#include "Common.h"
#include "DX8Sound.h"
#include "Segment.h"

cc_SndCache::cc_SndCache(cc_SegmentMgr *mgr)
:	m_mgr(mgr),
	m_Active(0)
{
}

cc_SndCache::~cc_SndCache()
{
	while (isActive()) deactivate();
}

void cc_SndCache::activate()
{
	if (m_Active) return;

	for (segments_t::iterator itor = m_Segments.begin(); itor != m_Segments.end(); itor++)
	{
		onActivateSegment(*itor);
	}

	m_Active++;
}

void cc_SndCache::deactivate()
{
	if (!m_Active) return;

	for (segments_t::iterator itor = m_Segments.begin(); itor != m_Segments.end(); itor++)
	{
		onDeactivateSegment(*itor);
	}

	m_Active--;
}

bool cc_SndCache::isActive () const
{
	return m_Active>0;
}

void cc_SndCache::files (const char *mask)
{
	ci_VDirIt *itor = cc_DX8Sound::m_Sound->getFileSystem()->GetDir(mask);
	for (; !itor->isDone(); itor->next()) addFile(itor->getFullName().c_str());
	itor->Release();
}

void cc_SndCache::except(const char *mask)
{
	ci_VFileSystem *fs = cc_DX8Sound::m_Sound->getFileSystem();
	ci_VDirIt *itor = fs->GetDir(mask);

	for (; !itor->isDone(); itor->next()) removeFile(itor->getFullName().c_str());

	itor->Release();
}

void cc_SndCache::addFile(const char *name)
{
	segments_t::iterator itor = element(name);
	if (itor == m_Segments.end())
	{
		m_Segments.push_back(m_mgr->getSegment(name));
	}
}

void cc_SndCache::removeFile(const char *name)
{
	segments_t::iterator itor = element(name);
	if (itor != m_Segments.end())
	{
		m_Segments.erase(itor);
	}
}

cc_SndCache::segments_t::iterator cc_SndCache::element(const char *name)
{
	for (segments_t::iterator itor = m_Segments.begin(); itor != m_Segments.end(); itor++)
	{
		if ((*itor)->getName() == name) break;
	}

	return itor;
}
