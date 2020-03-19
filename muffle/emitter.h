#if !defined(__EMITTER_H_INCLUDED_6491963142910359__)
#define __EMITTER_H_INCLUDED_6491963142910359__

#include "SoundBufferNotify.h"
#include <list>

class DecodeStream;
class Decoder;
class Script;
class DecodeMgr;

//=====================================================================================//
//                           class Emitter : public IEmitter                           //
//=====================================================================================//
class Emitter : public ISndEmitter, private noncopyable
{
	static long m_emittersCount;

	Module * const m_module;

	bool m_stopping;
	const Script * const m_script;
	const std::string m_wave;

	typedef std::list<class PlayingSound *> Playings_t;
	Playings_t m_sounds;

	CriticalSection m_soundsGuard;

	//Grom
	static CriticalSection m_staticListGuard;
	typedef std::set<Emitter *> EmitterList_t;
	static EmitterList_t m_allEmitters;

	static CriticalSection m_onStopGuard;

	snd_vector m_pos;
	snd_vector m_vel;

public:
	typedef Playings_t::iterator iterator;
	void onStop(PlayingSound *);

private:
	bool m_selfDestruct;

	void insertToList();
	void removeFromList();

public:
	Emitter(Module *, const Script &);
	Emitter(Module *, const Script &, const std::string &);
	virtual ~Emitter();

	void setVolume(float vol);
	void setSelfDestruct();

	virtual void play();
	virtual void stop();

	// Установить позицию звука
	virtual void setPosition(const snd_vector &);
	virtual void setVelocity(const snd_vector &);
	// etc.

	const snd_vector &position() const { return m_pos; }
	const snd_vector &velocity() const { return m_vel; }

	// Возвращает состояние источника
	virtual bool isPlaying();

	virtual void Release();

	static long getEmittersCount();

	//Grom
	static void ClearAll();
};

#endif // !defined(__EMITTER_H_INCLUDED_6491963142910359__)