#if !defined(__AUDIO_PATH_PROVIDER_INCLUDED__)
#define __AUDIO_PATH_PROVIDER_INCLUDED__

#include COMPTR_H
#include <dmusici.h>
#include <map>

class cc_PathMgr;
class cc_DirectMusic;

class cc_AudioPath
{
	friend cc_PathMgr;
	cc_PathMgr *m_pathMgr;

#if defined(_DEBUG)
	static const char *m_LogFile;
	static unsigned m_Counter;
	static int m_PathCount;
	std::string m_name;
#endif

	cc_COMPtr<IDirectMusicAudioPath8> m_Path;

	bool m_2d;

public:
	cc_AudioPath(cc_PathMgr *, IDirectMusicAudioPath8 *path, bool b2d);
	~cc_AudioPath();
	IDirectMusicAudioPath8 *getPath() {return m_Path;}
	bool is2D() {return m_2d;}

	void setPosition(const snd_vector &);
};

class cc_PathMgr
{
	static const char *m_LogFile;

	cc_COMPtr<IDirectMusicAudioPath8> m_2DAmbientPath;
	cc_COMPtr<IDirectMusicAudioPath8> m_2DEffectsPath;
	cc_COMPtr<IDirectMusicAudioPath8> m_2DMusicPath;

	typedef std::map<IDirectMusicAudioPath8 *, ci_Sound::Channel> pathes_t;
	pathes_t m_Pathes;

	cc_DirectMusic *m_music;

public:
	cc_PathMgr(cc_DirectMusic *);
	~cc_PathMgr();

	struct PathParameters
	{
		enum {use3D,ambient,effects,themes,other2D} m_Params;
	};

	cc_AudioPath *getAudioPath(const PathParameters &pp, ci_Sound::Channel channel);
	void releasePath(IDirectMusicAudioPath8 *);

	void adjustVolume(ci_Sound::Channel);
};

#endif