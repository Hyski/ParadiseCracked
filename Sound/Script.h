#if !defined(__SOUND_SCRIPT_INCLUDED__)
#define __SOUND_SCRIPT_INCLUDED__

class cc_Segment;
class cc_SegmentMgr;

struct cc_SndParams
{
	cc_SndParams();
	cc_SndParams(const cc_SndParams &);

	ci_Sound::Channel m_Type;
	bool m_Disable3D;
	float m_Pan;
	float m_ConeIn, m_ConeOut, m_ConeOutVol;
	float m_MinDist, m_MaxDist;
	unsigned m_Repeat;
	float m_Volume;
	std::string m_Wave;

	bool m_Permanent;
};

class cc_SndScript : private cc_SndParams, public ci_SndScript
{
	cc_SegmentMgr *m_mgr;
	std::string m_Name;
	cc_Segment *m_Segment;

public:
	cc_SndScript(const cc_SndParams *, const char *, cc_SegmentMgr *);

	const std::string &getName() const;
	const cc_SndParams *getParams() const;

	cc_Segment *getSegment();

	void Release();
};

#endif