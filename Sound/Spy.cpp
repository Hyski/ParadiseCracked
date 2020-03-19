#include "precomp.h"
#include "Spy.h"

#include <dmusici.h>
#include <process.h>
#include "DX8Sound.h"

#include <assert.h>

#include "SegmentState.h"
#include "CriticalSection.h"

#include "coreDM8.h"

#if defined(_USE_CARCASS_)
#include <common/streambuf.h>
#endif

HANDLE cc_SndSpy::m_hEvents[3] = {NULL,NULL,NULL};
cc_SndSpy::segments_t cc_SndSpy::m_Segments;
cc_CS *cc_SndSpy::m_DataLock = 0;
cc_SndSpy *cc_SndSpy::m_instance = 0;

LOG_MEMBER_IMPL(cc_SndSpy,m_log,"sound/spy.log");
#define SPY_LOG		C_HOMELOG(m_log)

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// cc_SndSpy::cc_SndSpy() /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
cc_SndSpy::cc_SndSpy(cc_DirectMusic *music)
:	m_music(music)
{
	TRACK_FUNC(cc_SndSpy::init());

	m_instance = this;

	m_hEvents[0] = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hEvents[1] = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hEvents[2] = CreateEvent(NULL,FALSE,FALSE,NULL);

	m_DataLock = cc_CSEntry::initializeCriticalSection();

	SAFE_CALL(m_music->performance()->AddNotificationType(GUID_NOTIFICATION_SEGMENT));
	SAFE_CALL(m_music->performance()->SetNotificationHandle(m_hEvents[0],0));

	m_Entry = 0;
	m_EntryCount = 0;

	m_hSpy = reinterpret_cast<HANDLE>(_beginthread(segmentSpy,0,NULL));
	SPY_LOG << STR_FULL_STAMP << "Segment spy initialized and began spying\n";
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// cc_SndSpy::addSegment() ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_SndSpy::addSegment(cc_SegmentState *seg)
{
	TRACK_FUNC(cc_SndSpy::addSegment());
	segments_t::iterator itor = m_Segments.find(seg->getState());
	if (itor != m_Segments.end())
	{
		SPY_LOG << "Duplicate IDirectMusicSegmentState8 pointer. Segment state was not added\n";
	}

	suspend();
	m_Segments[seg->getState()] = seg;
	SPY_LOG << "Added segment state " << ml_printf("0x%08x",seg) << "\n";
	resume();
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// cc_SndSpy::suspend() //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_SndSpy::suspend()
{
	if (!m_EntryCount++)
	{
		m_Entry = new cc_CSEntry(m_DataLock);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// cc_SndSpy::resume() //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_SndSpy::resume()
{
	if (!--m_EntryCount)
	{
		delete m_Entry;
		m_Entry = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// cc_SndSpy::removeSegment() ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void cc_SndSpy::removeSegment(cc_SegmentState *seg)
{
	suspend();
	SetEvent(m_hEvents[2]);

	segments_t::iterator itor = m_Segments.find(seg->getState());
	if (itor != m_Segments.end())
	{
		m_Segments.erase(itor);
	}
	resume();
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// cc_SndSpy::segmentSpy () ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void __cdecl cc_SndSpy::segmentSpy (void *)
{
	for (;;)
	{
		DWORD result;
		SPY_LOG << "Waiting for objects...\n";
		result = WaitForMultipleObjects(3, m_hEvents, FALSE, INFINITE);

		SPY_LOG << "Got event: ";
		if (result == WAIT_OBJECT_0+2)
		{
			SPY_LOG << "lookup table changed notification\n";
			cc_CSEntry guard(m_DataLock);
		}
		// Событие, сигнализирующее о завершении работы шпиона
		else if (result == WAIT_OBJECT_0+1)
		{
			SPY_LOG << "thread termination\n";
			break;
		}
		else if (result == WAIT_OBJECT_0)
		{
			SPY_LOG << "segment state notification\n[begin]\n";

			// Код из примера playAudio в DirectMusic (на корню переделанный)
			DMUS_NOTIFICATION_PMSG* pPMsg;
        
			// Get waiting notification message from the performance
			while(S_FALSE != m_instance->m_music->performance()->GetNotificationPMsg( &pPMsg ) )
			{
				SPY_LOG << "\tgot a message\n";
				do
				{
					if (pPMsg->guidNotificationType!=GUID_NOTIFICATION_SEGMENT) break;

					IDirectMusicSegmentState8 *pSegmentState = NULL;
					if(FAILED(pPMsg->punkUser->QueryInterface(IID_IDirectMusicSegmentState8,
												reinterpret_cast<LPVOID*>(&pSegmentState))))
					{
						break;
					}

					segments_t::iterator itor;// = m_Segments.find(pSegmentState);
					cc_SegmentState *sst;
					{
						cc_CSEntry guard(m_DataLock);

						itor = m_Segments.find(pSegmentState);
						if (itor == m_Segments.end())
						{
							pSegmentState->Release();
							break;
						}

						sst = itor->second;
					}

					bool erase = false;

					switch( pPMsg->dwNotificationOption )
					{
						case DMUS_NOTIFICATION_SEGEND:
							erase = sst->onStop();
							break;
						case DMUS_NOTIFICATION_SEGSTART:
							erase = sst->onStart();
							break;
					}

					if (erase)
					{
						cc_CSEntry guard(m_DataLock);
						m_Segments.erase(itor);
					}

					pSegmentState->Release();
				} while(0);

				m_instance->m_music->performance()->FreePMsg( (DMUS_PMSG*)pPMsg ); 
			}

			SPY_LOG << "[end]\n";
		}
	}
}