#include "precomp.h"
#include "SoundBufferNotify.h"
#include "NotifyThread.h"
#include "SoundBuffer.h"

//=====================================================================================//
//                       SoundBufferNotify::SoundBufferNotify()                        //
//=====================================================================================//
SoundBufferNotify::SoundBufferNotify(Module *mdl, SoundBuffer *buf)
:	m_module(mdl),
	m_buffer(buf)
{
//	m_handles.reserve(4);
//	m_receivers.reserve(4);
	IDirectSoundNotify8 *notify;
	SAFE_CALL(m_buffer->getBuffer()->QueryInterface(IID_IDirectSoundNotify8,(void **)&notify));
	m_notify.attach(notify);
}

//=====================================================================================//
//                       SoundBufferNotify::~SoundBufferNotify()                       //
//=====================================================================================//
SoundBufferNotify::~SoundBufferNotify()
{
	m_notify->SetNotificationPositions(0,0);
	assert( m_handles.empty() );
}

//=====================================================================================//
//                      void SoundBufferNotify::addPosReceiver()                       //
//=====================================================================================//
void SoundBufferNotify::addPosReceiver(unsigned offset, NotifyReceiver *recv)
{
	Entry entry;
	entry.offset = offset * m_buffer->getFormat().channels * sizeof(short);
	entry.handle = m_hpos.handle();

	{
		::Entry guard(m_vectorsGuard);

		m_handles.insert(m_handles.begin(),entry);
		m_receivers.insert(m_receivers.begin(),recv);

		m_module->getNotifyThread()->addNotify(m_hpos.handle(),recv);
	}
}

//=====================================================================================//
//                     void SoundBufferNotify::addDecodeReceiver()                     //
//=====================================================================================//
void SoundBufferNotify::addDecodeReceiver(unsigned step, unsigned count, NotifyReceiver *recv)
{
	Entry entry;
	entry.handle = m_hdec.handle();

	{
		::Entry guard(m_vectorsGuard);

		for(unsigned i = 0; i < count; ++i)
		{
			entry.offset = (step*i) * m_buffer->getFormat().channels * sizeof(short);
			m_handles.insert(m_handles.begin(),entry);
			m_receivers.insert(m_receivers.begin(),recv);
		}

		m_module->getNotifyThread()->addNotify(m_hdec.handle(),recv);
	}
}

//=====================================================================================//
//                      void SoundBufferNotify::removeReceiver()                       //
//=====================================================================================//
void SoundBufferNotify::removeReceiver(NotifyReceiver *recv)
{
	::Entry guard(m_vectorsGuard);
	Receivers_t::iterator it;

	HANDLE handle = 0;

	while((it = std::find(m_receivers.begin(),m_receivers.end(),recv)) != m_receivers.end())
	{
		Handles_t::iterator hit = m_handles.begin()+(it - m_receivers.begin());
		assert( handle == 0 || handle == hit->handle );
		handle = hit->handle;

		m_handles.erase(hit);
		m_receivers.erase(it);
	}

	m_module->getNotifyThread()->removeNotify(handle);
}

//=====================================================================================//
//                        void SoundBufferNotify::dumpHandles()                        //
//=====================================================================================//
void SoundBufferNotify::dumpHandles(std::ostream &sstr)
{
	for(Handles_t::iterator i = m_handles.begin(); i != m_handles.end(); ++i)
	{
		sstr << std::setw(8) << std::hex << i->offset << " " << i->handle << "\n";
	}

	sstr << "-=-=-=-=-=-=-=-=-=-\n";
}

//=====================================================================================//
//                      void SoundBufferNotify::addStopReceiver()                      //
//=====================================================================================//
void SoundBufferNotify::addStopReceiver(NotifyReceiver *recv)
{
	Entry entry;
	entry.offset = DSBPN_OFFSETSTOP;
	entry.handle = m_hstop.handle();

	{
		::Entry guard(m_vectorsGuard);

		m_handles.push_back(entry);
		m_receivers.push_back(recv);

		m_module->getNotifyThread()->addNotify(m_hstop.handle(),recv);
	}
}

//=====================================================================================//
//                     void SoundBufferNotify::prepareForPlaying()                     //
//=====================================================================================//
void SoundBufferNotify::prepareForPlaying()
{
	::Entry guard(m_vectorsGuard);
	SAFE_CALL(m_notify->SetNotificationPositions(m_handles.size(),
		reinterpret_cast<LPCDSBPOSITIONNOTIFY>(&m_handles[0])));
}