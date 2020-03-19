#if !defined(__SND_SEGMENT_STATE_INCLUDED__)
#define __SND_SEGMENT_STATE_INCLUDED__

#include <dmusici.h>
#include COMPTR_H

class cc_Segment;
class cc_SndScript;
class cc_SegmentStateMgr;

#include <set>

///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// cc_SegmentState ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
class cc_SegmentState
{
	typedef std::set<cc_SegmentState *> StateList_t;
	static StateList_t m_states;

	// Строковые названия состояний и приказов (ОТЛАДКА)
	static const char *m_strStates[];
	static const char *m_strOrders[];

	// Счетчик создания экземпляров класса (генерация уникальных имен)
	static unsigned m_Counter;
	
	// Имя экземпляра (ОТЛАДКА)
	std::string m_name;

	// Флаг, сигнализирующий о том, что экземпляр 
	// уже добавлен в очередь обновления состояний
	//bool m_willBeUpdated;
	int m_updateCount;

	// Параметры проигрывания звука
	unsigned m_repeats;
	__int64 m_startTime;
	unsigned m_flags;

	cc_Segment *m_segment;
	cc_COMPtr<IDirectMusicSegmentState8> m_segState;
	cc_COMPtr<IDirectMusicAudioPath8> m_Path;

	cc_SegmentStateMgr *m_mgr;

	// Состояние экземпляра
	enum 
	{
		preStart		=	0,
		downloading		=	1,
		prePlay			=	2,
		playing			=	3,
		postPlay		=	4,
		unloading		=	5,
		finished		=	6
	} m_state;

	// Приказ экземпляра
	enum 
	{
		nothing			=	0,
		halt			=	1,
		destroy			=	2
	} m_order;

	// Обработка состояний экземпляра
	bool performStateUpdate();
	bool statePreStart();
	bool stateDownloading();
	bool statePrePlay();
	bool statePlaying();
	bool statePostPlay();
	bool stateUnloading();

	~cc_SegmentState();

	// Поместить экземпляр в очередь обновления состояний
	void updateNextFrame();

	DECLARE_LOG_MEMBER(m_log);

public:
	// Нужен для того, чтобы по выходу дождаться завершения проигрывания всех состояний
	static int m_instanceCount;

	cc_SegmentState(cc_Segment *, cc_SegmentStateMgr *);
	void release();

	bool onStart();
	bool onStop();

	bool play(IDirectMusicAudioPath8 *, cc_SndScript *);
	bool play(IDirectMusicAudioPath8 *); // для тем
	void stop();

	void update();

	bool isFinished();
	bool isStarted();

	IDirectMusicSegmentState8 *getState() {return m_segState;}

	static void flushDebugInfo();
};

#endif