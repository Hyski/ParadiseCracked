#if !defined(__VOLUME_H_INCLUDED_4167671829007341__)
#define __VOLUME_H_INCLUDED_4167671829007341__

#include "volumemgr.h"

class SoundBuffer;

//=====================================================================================//
//                                    class Volume                                     //
//=====================================================================================//
class Volume : private noncopyable
{
	friend class VolumeMgr;

	VolumeMgr			*m_mgr;
	VolumeMgr::iterator	m_me;
	float				m_vol;
	float				m_fullVol;
	bool				m_muted;
	SoundBuffer			*m_buffer;

	Volume(VolumeMgr *mgr, const VolumeMgr::iterator &it);

	void adjust();
	void adjustDependents();
	inline float parentFull() const { return (*m_me.get_parent())->getFull(); }
	void setBufferVolume();

public:
	~Volume();

	void attach(SoundBuffer *buf);
	void detach();

	void change(float);
	void mute(bool);

	float get() const { if(!m_muted) return m_vol; return 0.0f; }
	float getFull() const { if(!m_muted) return m_fullVol; return 0.0f; }
	float getVolume() const { return m_vol; }

	long decibel() const;
};

#endif // !defined(__VOLUME_H_INCLUDED_4167671829007341__)