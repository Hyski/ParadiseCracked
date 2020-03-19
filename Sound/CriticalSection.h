#if !defined(__CRITICAL_SECTION_ENTRY_INCLUDED__)
#define __CRITICAL_SECTION_ENTRY_INCLUDED__

class cc_CS;

class cc_CSEntry
{
	cc_CS *m_CriticalSection;

public:
	cc_CSEntry(cc_CS *);
	~cc_CSEntry();

	static cc_CS *initializeCriticalSection();
	static void deleteCriticalSection(cc_CS *);
};

#endif