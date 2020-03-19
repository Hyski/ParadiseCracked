#if !defined(__PLAYING_SOUND_H_INCLUDED_5992692519605117__)
#define __PLAYING_SOUND_H_INCLUDED_5992692519605117__

#include "Emitter.h"
#include "NotifyReceiver.h"

class DecodeStream;

//=====================================================================================//
//                                 class PlayingSound                                  //
//=====================================================================================//
class PlayingSound : private noncopyable
{
	friend class Emitter;

	Module *m_module;
	const Script &m_script;
	Emitter *m_emitter;
	std::auto_ptr<DecodeStream> m_stream;

	CriticalSection m_bufferGuard;

	typedef NotifyAdaptor<PlayingSound> Adaptor_t;
	Adaptor_t m_onStop;
	void onStop();

	void setDistances(float,float);

public:
	PlayingSound(Module *, const Script &, std::auto_ptr<Decoder>, Emitter *);
	~PlayingSound();

	void setPosition(const snd_vector &);
	void setVelocity(const snd_vector &);
	void setVolume(float vol);
};

#endif // !defined(__PLAYING_SOUND_H_INCLUDED_5992692519605117__)