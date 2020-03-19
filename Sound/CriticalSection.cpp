#include "precomp.h"
#include "CriticalSection.h"

#include <windows.h>

class cc_CS
{
	CRITICAL_SECTION m_section;

public:
	cc_CS()
	{
		InitializeCriticalSection(&m_section);
	}

	~cc_CS()
	{
		DeleteCriticalSection(&m_section);
	}

	void enter()
	{
		EnterCriticalSection(&m_section);
	}

	void leave()
	{
		LeaveCriticalSection(&m_section);
	}
};

cc_CSEntry::cc_CSEntry(cc_CS *cs)
:	m_CriticalSection(cs)
{
	m_CriticalSection->enter();
}

cc_CSEntry::~cc_CSEntry()
{
	m_CriticalSection->leave();
}

cc_CS *cc_CSEntry::initializeCriticalSection()
{
	return new cc_CS;
}

void cc_CSEntry::deleteCriticalSection(cc_CS *cs)
{
	delete cs;
}