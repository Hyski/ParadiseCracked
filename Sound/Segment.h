#if !defined(__SOUND_SEGMENT_INCLUDED__)
#define __SOUND_SEGMENT_INCLUDED__

#include <dmusici.h>
#include COMPTR_H

class cc_SegmentMgr;

/*class cc_Segment// : public ci_SndSegment
{
	static const char *m_LogFile;

	mutable int m_RefCount;
	cc_COMPtr<IDirectMusicSegment8> m_Segment;

	std::string m_Name;
	int m_InCache;

	void cache();
	void discard();

public:
	cc_Segment(const char *);
	~cc_Segment();

	const std::string &getName() const;

	void addRef();
	void release();

	IDirectMusicSegment8 *getSegment();

	// Возвращает true, если сегмент находится в одном из кешей
	bool isInCache() const;

	void addedToCache();
	void kickedFromCache();
};*/

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// cc_Segment //////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
// Инкапсулирует сегмент
class cc_Segment
{
	static unsigned m_segCount;

protected:
	static const char *m_LogFile;
	cc_SegmentMgr *m_mgr;

private:
	int m_refCount;
	std::string m_name;

protected:
	cc_COMPtr<IDirectMusicSegment8> m_segment;

public:
	cc_Segment(cc_SegmentMgr *);
	virtual ~cc_Segment();

	const std::string &getName() const {return m_name;}
	void rename(const char *name);
	IDirectMusicSegment8 *getSegment() {return m_segment;}
	int getRefCount() const {return m_refCount;}

	virtual void addRef();
	virtual void release();

};

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// cc_SegFromFile //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
// Сегмент, загруженный из файла
class cc_SegFromFile : public cc_Segment
{
	bool m_bInCache;

	void load();
	void unload();

public:
	cc_SegFromFile(cc_SegmentMgr *mgr,const char *);

	void addRef();
	void release();

	bool isInCache() const {return m_bInCache;}

	void addedToCache();
	void kickedFromCache();
};

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// cc_SegGenerated /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
// Сегмент, сгенерированный в процессе выполнения
class cc_SegGenerated : public cc_Segment
{
public:
	cc_SegGenerated(cc_SegmentMgr *, IDirectMusicSegment8 *);
	virtual ~cc_SegGenerated();
};

#endif