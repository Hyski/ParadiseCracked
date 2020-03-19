#include "precomp.h"
#include "volume.h"
#include "SoundBuffer.h"
#include <math.h>

//=====================================================================================//
//                                  Volume::Volume()                                   //
//=====================================================================================//
Volume::Volume(VolumeMgr *mgr, const VolumeMgr::iterator &it)
:	m_mgr(mgr),
	m_me(it),
	m_vol(1.0f),
	m_fullVol(m_vol * (m_me.has_parent()?(*m_me.get_parent())->getFull():1.0f)),
	m_muted(false),
	m_buffer(0)
{
}

//=====================================================================================//
//                                  Volume::~Volume()                                  //
//=====================================================================================//
Volume::~Volume()
{
	m_mgr->removeVolume(this);
	assert( m_buffer == 0 );
}

//=====================================================================================//
//                           void Volume::adjustDependents()                           //
//=====================================================================================//
void Volume::adjustDependents()
{
	Entry guard(m_mgr->volumeGuard());
	VolumeMgr::iterator itor = m_me.get_son();
	for(; itor != m_me.son_end(); itor.brother())
	{
		(*itor)->adjust();
	}
}

//=====================================================================================//
//                                void Volume::adjust()                                //
//=====================================================================================//
void Volume::adjust()
{
	float newFull;
	{
		Entry guard(m_mgr->volumeGuard());
		newFull = m_vol * (m_me.has_parent()?parentFull():1.0f);
	}

	if(newFull != m_fullVol)
	{
		m_fullVol = newFull;
		setBufferVolume();
		if(!m_muted)
		{
			adjustDependents();
		}
	}
}

//=====================================================================================//
//                                void Volume::change()                                //
//=====================================================================================//
void Volume::change(float vol)
{
	if(vol != m_vol)
	{
		m_vol = vol;
		adjust();
	}
}

//=====================================================================================//
//                                 void Volume::mute()                                 //
//=====================================================================================//
void Volume::mute(bool bMute)
{
	if(m_muted != bMute)
	{
		m_muted = bMute;
		if(m_fullVol != 0.0f)
		{
			setBufferVolume();
			adjustDependents();
		}
	}
}

//=====================================================================================//
//                                void Volume::attach()                                //
//=====================================================================================//
void Volume::attach(SoundBuffer *buf)
{
	assert( m_buffer == 0 );
	m_buffer = buf;
	setBufferVolume();
}

//=====================================================================================//
//                                void Volume::detach()                                //
//=====================================================================================//
void Volume::detach()
{
	assert( m_buffer != 0 );
	m_buffer = 0;
}

//=====================================================================================//
//                           void Volume::setBufferVolume()                            //
//=====================================================================================//
void Volume::setBufferVolume()
{
	if(m_buffer)
	{
		m_buffer->getBuffer()->SetVolume(decibel());
	}
}

//=====================================================================================//
//                               long Volume::decibel()                                //
//=====================================================================================//
long Volume::decibel() const
{
	const float minimal = 0.05f;//pow(exp(1.0f),-9.6f);
	const float volume = getFull();
	if (volume < minimal) return -10000;
	if (volume > 1.0f) return 0;
	return static_cast<long>(1000.0f*logf(volume));
}