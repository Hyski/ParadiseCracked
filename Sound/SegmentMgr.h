#if !defined(__SEGMENT_MANAGER_INCLUDED__)
#define __SEGMENT_MANAGER_INCLUDED__

#include <map>
#include <dmusicc.h>

class cc_SndDelayedCache;

#include COMPTR_H

class cc_Segment;
class cc_SegFromFile;

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// class cc_SegmentMgr //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class cc_SegmentMgr
{
//	static const char *m_LogFile;

	typedef cc_SegFromFile ListedSegment;

	typedef std::map<std::string,ListedSegment *> segments_t;
	segments_t m_Segments;

	cc_COMPtr<IDirectMusicLoader8> m_Loader;

	cc_SndDelayedCache *m_Delayed;

public:
	cc_SegmentMgr();
	~cc_SegmentMgr();
	// Инициализация/деинициализация
//	void init();
//	void shut();

	// Возвращает указатель на сегмент
	// ВНИМАНИЕ: НЕ УВЕЛИЧИВАЕТ СЧЕТЧИК ССЫЛОК
	ListedSegment *getSegment(const char *);
	ListedSegment *getSegmentRef(const char *);
	IDirectMusicLoader8 *getLoader();

	cc_SndDelayedCache *getDefaultCache();
};

#endif