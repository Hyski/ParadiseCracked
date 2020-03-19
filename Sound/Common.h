#if !defined(__COMMON_CACHE_FUNCTIONALITY__)
#define __COMMON_CACHE_FUNCTIONALITY__

#include <list>

class cc_Segment;
class cc_SegmentMgr;

class cc_SndCache : public ci_SndCache
{
	cc_SegmentMgr *m_mgr;
	int m_Active;

	void addFile(const char *);
	void removeFile(const char *);

protected:
	typedef std::list<cc_Segment *> segments_t;
	segments_t m_Segments;

	virtual void onActivateSegment(cc_Segment *) = 0;
	virtual void onDeactivateSegment(cc_Segment *) = 0;

private:
	segments_t::iterator element(const char *);

public:
	cc_SndCache(cc_SegmentMgr *);
	virtual ~cc_SndCache();

	void activate();
	void deactivate();

	bool isActive() const;

	// Для загрузчика скриптов
	void files(const char *);
	void except(const char *);
};

#endif