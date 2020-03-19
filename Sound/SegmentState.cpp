#include "precomp.h"
#include "SegmentState.h"
#include "Segment.h"
#include "Script.h"
#include "Spy.h"
#include "DX8Sound.h"
#include <assert.h>

#include <sstream>
#include <iomanip>

#include "CriticalSection.h"

#include "SegmentStateMgr.h"
#include "coreDM8.h"

LOG_MEMBER_IMPL(cc_SegmentState,m_log,"sound/segments.log");
#define SEG_LOG		C_HOMELOG(m_log)

unsigned cc_SegmentState::m_Counter = 0;
int cc_SegmentState::m_instanceCount = 0;
cc_SegmentState::StateList_t cc_SegmentState::m_states;

const char *cc_SegmentState::m_strStates[] = {"preStart","downloading","prePlay","playing","postPlay","unloading","finished"};
const char *cc_SegmentState::m_strOrders[] = {"nothing","halt","destroy"};

#define MACRO_SS_STAMP	"<" << STR_TIME_STAMP << "> " << cc_DM8Error::hexstr(this) << STR_FULL_STAMP

class CriticalSection
{
	cc_CS *m_sect;

public:
	CriticalSection()
	{
		m_sect = cc_CSEntry::initializeCriticalSection();
	}

	~CriticalSection()
	{
		cc_CSEntry::deleteCriticalSection(m_sect);
	}

	cc_CS *sect() {return m_sect;}
};

static CriticalSection g_segStateLock, g_segUpdateLock;

//////////////////////// cc_SegmentState::cc_SegmentState() ////////////////////////////
cc_SegmentState::cc_SegmentState(cc_Segment *segment, cc_SegmentStateMgr *mgr)
:	m_mgr(mgr),
	m_segment(segment),
	m_state(preStart),
	m_order(nothing),
	m_updateCount(0)
{
	TRACK_FUNC(cc_SegmentState::cc_SegmentState());

	m_states.insert(this);

	m_instanceCount++;

	char buffer[32];
	sprintf(buffer,"state%u",m_Counter++);
	m_name=buffer;

	SEG_LOG << MACRO_SS_STAMP << "Created segment state [" << m_name << "], ";
	SEG_LOG << "segment is ";
	if (m_segment)
	{
		SEG_LOG << m_segment->getName();
	}
	else
	{
		SEG_LOG << "NULL";
	}
	SEG_LOG << "\n";
}

//////////////////////// cc_SegmentState::~cc_SegmentState() ///////////////////////////
cc_SegmentState::~cc_SegmentState()
{
	TRACK_FUNC(cc_SegmentState::~cc_SegmentState());
	SEG_LOG << MACRO_SS_STAMP << "Destroyed segment state [" << m_name << "]\n";
	m_instanceCount--;
	m_states.erase(this);
}

//////////////////////// cc_SegmentState::onStart() ////////////////////////////////////
bool cc_SegmentState::onStart()
{
//	TRACK_FUNC(cc_SegmentState::onStart());
	cc_CSEntry guard(g_segStateLock.sect());
	if(m_state == prePlay)
	{
//		CORE_LOG(m_LogFile,MACRO_SS_STAMP+"Occured\n");
		m_state = playing;
		updateNextFrame();
	}
	return false;
}

//////////////////////// cc_SegmentState::onStop() /////////////////////////////////////
bool cc_SegmentState::onStop()
{
//	TRACK_FUNC(cc_SegmentState::onStop());
	cc_CSEntry guard(g_segStateLock.sect());
//	CORE_LOG(m_LogFile,MACRO_SS_STAMP+"Occured\n");
	if((m_state == postPlay) || (m_state == playing))
	{
		m_state = unloading;
		updateNextFrame();
	}
	return true;
}

//////////////////////// cc_SegmentState::play() ///////////////////////////////////////
bool cc_SegmentState::play(IDirectMusicAudioPath8 *path, cc_SndScript *script)
{
	TRACK_FUNC(cc_SegmentState::play());

	SEG_LOG << MACRO_SS_STAMP << "Entered routine; state name is [" << m_name << "]...\n";

	if (m_state>preStart)
	{
		SEG_LOG << "\t\t...FAILURE: segment state is already activated\n";
		SEG_LOG << MACRO_SS_STAMP << "Leaved routine.\n";
		return false;
	}

	if (script->getParams()->m_Type == ci_Sound::cThemes)
	{
		SEG_LOG << "\t\t...cannot play theme as not theme\n";
		SEG_LOG << MACRO_SS_STAMP << "Leaved routine.\n";
		return false;
	}

	// Проинициализируем параметры воспроизведения
	m_repeats = (script->getParams()->m_Repeat == 0xFFFFFFFF)?
					DMUS_SEG_REPEAT_INFINITE : script->getParams()->m_Repeat;
	m_startTime = 0;
	m_flags = DMUS_SEGF_SECONDARY;

	m_segment->addRef();
	if (!m_segment->getSegment())
	{
		SEG_LOG << "\t\t...failed to obtain IDirectMusicSegment8\n";
		m_state = finished;
		m_segment->release();
		SEG_LOG << MACRO_SS_STAMP << "Leaved routine.\n";
		return false;
	}
	SEG_LOG << "\t\t...obtained IDirectMusicSegment8\n";

	SEG_LOG << "\t\t...switched to <downloading> state\n";
	m_state = downloading;
	m_Path = path;
	m_Path->AddRef();

	cc_SndSpy *spy = m_mgr->spy();
	spy->suspend();
	updateNextFrame();
//	while(performStateUpdate());
	spy->resume();

	SEG_LOG << MACRO_SS_STAMP << "Leaved routine.\n";
	return true;
}

bool cc_SegmentState::play(IDirectMusicAudioPath8 *path)
{
	TRACK_FUNC(cc_SegmentState::play());

	SEG_LOG << MACRO_SS_STAMP << "Entered routine; state name is [" << m_name << "]...\n";

	if (m_state>preStart)
	{
		SEG_LOG << "\t\t...FAILURE: segment state is already activated\n";
		SEG_LOG << MACRO_SS_STAMP << "Leaved routine.\n";
		return false;
	}

	// Проинициализируем параметры воспроизведения
	m_repeats = 0;
	m_startTime = 0;
	m_flags = DMUS_SEGF_SECONDARY;

	m_segment->addRef();
	if (!m_segment->getSegment())
	{
		SEG_LOG << "\t\t...failed to obtain IDirectMusicSegment8\n";
		m_state = finished;
		m_segment->release();
		SEG_LOG << MACRO_SS_STAMP << "Leaved routine.\n";
		return false;
	}
	SEG_LOG << "\t\t...obtained IDirectMusicSegment8\n";

	SEG_LOG << "\t\t...switched to <downloading> state\n";
	m_state = downloading;
	m_Path = path;
	m_Path->AddRef();

	cc_SndSpy *spy = m_mgr->spy();
	spy->suspend();
	updateNextFrame();
//	while(performStateUpdate());
	spy->resume();

	SEG_LOG << MACRO_SS_STAMP << "Leaved routine.\n";
	return true;
}

void cc_SegmentState::stop()
{
	TRACK_FUNC(cc_SegmentState::stop());

	{
		cc_CSEntry guard(g_segStateLock.sect());

		if (m_order>=halt)
		{
			SEG_LOG << MACRO_SS_STAMP << "[" << m_name << "] is already stopping\n";
			return;
		}

		m_order = halt;
		SEG_LOG << MACRO_SS_STAMP << "Initiated <halt> order for [" << m_name << "]\n";
	}
	updateNextFrame();
}

void cc_SegmentState::updateNextFrame()
{
	TRACK_FUNC(cc_SegmentState::updateNextFrame());
	{
		cc_CSEntry guard(g_segUpdateLock.sect());

		if(!m_updateCount)
		{
			m_mgr->emitMgr()->updateOnNextFrame(this);
			m_updateCount++;
		}
	}
	SEG_LOG << MACRO_SS_STAMP << "Appended [" << m_name << "] to update queue\n";
}

void cc_SegmentState::update()
{
	TRACK_FUNC(cc_SegmentState::update());

	{
		cc_CSEntry guard(g_segUpdateLock.sect());
		m_updateCount--;
	}

//	m_willBeUpdated = false;

	SEG_LOG << MACRO_SS_STAMP << "Performing [" << m_name << "] update...\n";
	SEG_LOG << "\t\t...current order is <" << m_strOrders[m_order] << ">\n";
	SEG_LOG << "\t\t...current state is <" << m_strStates[m_state] << ">\n";

	{
		cc_CSEntry guard(g_segStateLock.sect());
		while(performStateUpdate());

		if ((m_state == finished) && (m_order == destroy))
		{
			if (!m_updateCount)
			{
				SEG_LOG << "\t\t...destroyed [" << m_name << "]\n";
				SEG_LOG << MACRO_SS_STAMP << "Finished [" << m_name << "] update...\n";
				delete this;
				return;
			}
			else
			{
				SEG_LOG << "\t\t...deferred [" << m_name << "] destruction for a while\n";
			}
		}
	}

	SEG_LOG << MACRO_SS_STAMP << "Finished [" << m_name << "] update...\n";
}

bool cc_SegmentState::isFinished()
{
//	update();
	if (m_state == finished) return true;
	return false;
}

bool cc_SegmentState::isStarted()
{
//	update();
	if (m_state >= playing) return true;
	return false;
}

bool cc_SegmentState::performStateUpdate()
{
	TRACK_FUNC(cc_SegmentState::performStateUpdate());
	switch(m_state)
	{
		case preStart: return statePreStart();
		case downloading: return stateDownloading();
		case prePlay: return statePrePlay();
		case playing: return statePlaying();
		case postPlay: return statePostPlay();
		case unloading: return stateUnloading();
		case finished: return false;
		default: assert(false); return false; // Не должно выполняться никогда
	}
}

bool cc_SegmentState::statePreStart()
{
	TRACK_FUNC(cc_SegmentState::statePreStart());
	assert(m_state==preStart);
	if (m_order >= halt)
	{
		m_state = finished;
		SEG_LOG << "\t\t...switched to <finished> state\n";
		return true;
	}
	return false;
}

bool cc_SegmentState::stateDownloading()
{
	TRACK_FUNC(cc_SegmentState::stateDownloading());
	assert(m_state==downloading);

	if (m_order >= halt)
	{
		m_state = finished;
		SEG_LOG << "\t\t...switched to <finished> state\n";
		return true;
	}

//	SAFE_CALL(m_segment->getSegment()->Download(m_Path));
	HRESULT hr = m_segment->getSegment()->Download(m_Path);
	if (SUCCEEDED(hr))
	{
		m_state = prePlay;
		SEG_LOG << "\t\t...download succeeded, switching to <prePlay> state\n";
		return true;
	}
	else
	{
		SEG_LOG << "\t\t...download failed, I'll try to download during next update\n";
		SEG_LOG << "\t\t......cause: ";
		SEG_LOG << cc_DM8Error::getDescription(hr);
		SEG_LOG << "\n";
		updateNextFrame();
		return false;
	}
}

bool cc_SegmentState::statePrePlay()
{
	TRACK_FUNC(cc_SegmentState::statePrePlay());
	assert(m_state==prePlay);

	if (m_segState == 0) // Если сегмент не запущен
	{
		if (m_order == nothing)
		{
			SAFE_CALL(m_segment->getSegment()->SetRepeats(m_repeats));

			cc_SndSpy *spy = m_mgr->spy();

			IDirectMusicSegmentState *state = NULL;

			SAFE_CALL(m_mgr->music()->performance()->PlaySegmentEx
				(
					m_segment->getSegment(),
					NULL,
					NULL,
					m_flags,
					m_startTime,
					&state,
					NULL,
					m_Path
				));

			SAFE_CALL(state->QueryInterface(IID_IDirectMusicSegmentState8,m_segState.asVoidPtr()));
			spy->addSegment(this);
			state->Release();
			SEG_LOG << "\t\t...initiated playback\n";
		}
		else
		{
			m_state = unloading;
			SEG_LOG << "\t\t...switched to <unloading> state\n";
			return true;
		}
	}

	if (m_order>=halt) updateNextFrame();

	if (m_mgr->music()->performance()->IsPlaying(NULL,this->m_segState) == S_OK)
	{
		// Сегмент уже играет, а уведомления еще не было
		// Сообщения, возможно, пропадают через некоторое время
		m_state = playing;
		SEG_LOG << "\t\t...update was not occured, but playback started ==> switched to <playing> state\n";
		return true;
	}
	else
	{
		updateNextFrame();
	}

	return false;
}

bool cc_SegmentState::statePlaying()
{
	TRACK_FUNC(cc_SegmentState::statePlaying());
	assert(m_state==playing);

	if (m_order>=halt)
	{
		m_mgr->music()->performance()->StopEx(m_segState,0,0);
		SEG_LOG << "\t\t...stopping playback\n";
		cc_SndSpy *spy = m_mgr->spy();
		spy->removeSegment(this);
		m_state = unloading;
		SEG_LOG << "\t\t...switched to <unloading> state\n";
		return true;
	}
	else
	{
		if (m_mgr->music()->performance()->IsPlaying(NULL,this->m_segState) != S_OK)
		{
			m_state = unloading;
			return true;
		}
		return false;
	}
}

bool cc_SegmentState::statePostPlay()
{
	TRACK_FUNC(cc_SegmentState::statePostPlay());
	assert(m_state==postPlay);

	if(m_mgr->music()->performance()->IsPlaying(NULL,this->m_segState) != S_OK)
	{
		m_state = unloading;
		return true;
	}

	return false;
}

bool cc_SegmentState::stateUnloading()
{
	TRACK_FUNC(cc_SegmentState::stateUnloading());
	assert(m_state==unloading);

	SAFE_CALL(m_segment->getSegment()->Unload(m_Path));
	SEG_LOG << "\t\t...unloaded segment\n";
	m_segment->release();
	SEG_LOG << "\t\t...switched to <finished> state\n";
	m_state = finished;
	return true;
}

void cc_SegmentState::release()
{
	TRACK_FUNC(cc_SegmentState::release());
	{
		cc_CSEntry guard(g_segStateLock.sect());

		if (m_order>=destroy)
		{
			SEG_LOG << MACRO_SS_STAMP << "[" << m_name << "] is already under destruction\n";
			return;
		}

		m_order = destroy;
		SEG_LOG << MACRO_SS_STAMP << "Initiated <destroy> order for [" << m_name << "]\n";
	}
	updateNextFrame();
}


void cc_SegmentState::flushDebugInfo()
{
/*	static const char *log = "state_statistic.log";

	StateList_t::iterator itor = m_states.begin();

	std::ostringstream buffer;
	buffer << "==================================================\n";

	for(; itor != m_states.end(); itor++)
	{
		buffer << "Segment state {" << std::setw(8) << std::setfill('0') << std::hex << *itor << std::dec << "} ";
		buffer << "[" << (*itor)->m_name << "] ";
		if((*itor)->m_segment) buffer << "with segment [" << (*itor)->m_segment->getName() << "] ";
		buffer << "order: <" << m_strOrders[(*itor)->m_order] << "> state: <" << m_strStates[(*itor)->m_state] << ">\n";
	}

	buffer << std::ends;
	CORE_LOG(log,buffer.str());*/
}