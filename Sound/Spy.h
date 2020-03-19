#if !defined(__SOUND_EVENTS_SPY_INCLUDED__)
#define __SOUND_EVENTS_SPY_INCLUDED__

#include <map>
#include <windows.h>
class cc_SndCallback;
struct IDirectMusicSegment8;

class cc_SegmentState;
class cc_CS;
class cc_CSEntry;
class cc_DirectMusic;

class cc_SndSpy
{
	DECLARE_LOG_MEMBER(m_log);

	// Поток, выполняющий слежение за состоянием сегментов
	static void __cdecl segmentSpy (void *);
	HANDLE m_hSpy;

	cc_DirectMusic *m_music;

	typedef std::map<IDirectMusicSegmentState8 *, cc_SegmentState *> segments_t;

	static HANDLE m_hEvents[3];
	static segments_t m_Segments;
	static cc_CS *m_DataLock;
	static cc_SndSpy *m_instance;
	cc_CSEntry *m_Entry;
	int m_EntryCount;

public:
	cc_SndSpy(cc_DirectMusic *);

	void addSegment(cc_SegmentState *);
	void removeSegment(cc_SegmentState *);

	void suspend();
	void resume();
};

#endif