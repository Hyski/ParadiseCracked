#if !defined(__FADE_H_INCLUDED_2023399421164473__)
#define __FADE_H_INCLUDED_2023399421164473__

#include "NotifyReceiver.h"

class Emitter;

//=====================================================================================//
//                                     class Fade                                      //
//=====================================================================================//
class Fade
{
	HANDLE m_htimer;

	float m_volume;
	float m_increment;
	Module *m_module;
	Emitter *m_emitter;

	long *m_fadeCount;

	typedef NotifyAdaptor<Fade> Adaptor_t;
	Adaptor_t m_decVolAdaptor;

	void onDecVol();
	void setTimer();

#if !defined(MUFFLE_WAIT_FOR_MUSIC_FADE_OUT)
	static CriticalSection m_listLock;
	static CriticalSection m_destructLock;

	typedef std::set<Fade*> FadeList_t;
	static FadeList_t m_allFades;
	public:	static void destruct();
	private:
#endif

	void addToList();
	void removeFromList();

	bool m_selfDestruct;

public:
	Fade(Module *, Emitter *, float inc, long *fadeCount = 0);
	~Fade();
};


#endif // !defined(__FADE_H_INCLUDED_2023399421164473__)