#if !defined(__SOUND_BUFFER_NOTIFY_H_INCLUDED_5560120145570403__)
#define __SOUND_BUFFER_NOTIFY_H_INCLUDED_5560120145570403__

class NotifyReceiver;
class SoundBuffer;

//=====================================================================================//
//                               class SoundBufferNotify                               //
//=====================================================================================//
class SoundBufferNotify
{
	CriticalSection m_vectorsGuard;
	Module * const m_module;

	// ВНИМАНИЕ: эта структура по содержанию должна
	// копировать структуру, которая передается в SetNotificationPositions
	struct Entry
	{
		unsigned		offset;
		HANDLE			handle;
	};

	MLL_STATIC_ASSERT( sizeof(Entry) == sizeof(DSBPOSITIONNOTIFY) );

	SoundBuffer *m_buffer;
	ComPtr<IDirectSoundNotify8> m_notify;

	typedef std::vector<Entry> Handles_t;
	typedef std::vector<NotifyReceiver *> Receivers_t;

	Handles_t		m_handles;
	Receivers_t		m_receivers;

	WndHandle		m_hstop;
	WndHandle		m_hdec;
	WndHandle		m_hpos;

	void dumpHandles(std::ostream &);

public:
	SoundBufferNotify(Module *, SoundBuffer *buffer);
	~SoundBufferNotify();

	void addDecodeReceiver(unsigned step, unsigned count, NotifyReceiver *);
	void addPosReceiver(unsigned pos, NotifyReceiver *);
	void addStopReceiver(NotifyReceiver *);
	void removeReceiver(NotifyReceiver *);

	void prepareForPlaying();
};

#endif // !defined(__SOUND_BUFFER_NOTIFY_H_INCLUDED_5560120145570403__)