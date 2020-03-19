/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: EAX arranged sound library
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/

#include "..\precomp.h"
#include "sndcompiler.h"
#include "mmsystem.h"
#include "sound.h"
#include "eax.h"
#include "eaxdev.h"
#include "..\..\options\options.h"

using namespace sound;

#if defined(DSPL__DEB_VER_STORE_ERRORS)||defined(DSPL__DEB_VER_SHOW_COMMENT)||defined(DSPL__DEBUG_INFO)
#include <time.h>
#include "..\..\globals.h"
extern const char* log_name;
#endif

#ifdef DSPL__DEB_VER_STORE_ERRORS
#include "abnormal.h"
extern Abnormal err; 
#endif

/***************************************************************************
****************************************************************************

                            eaxdev realization

***************************************************************************
***************************************************************************/

eaxdev::eaxdev() : sddev(),device(NULL),camera(NULL),props_set(NULL),eax_supports(0),
                   GlobalVolume(0)
{         
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    struct tm *newtime;        
    time_t long_time;
    time( &long_time );                // Get time as long integer. 
    newtime = localtime( &long_time ); // Convert to local time. 

    logFile[log_name]("Sound log is opened now...\t%.19s \nEAX support is connected to sound engine.\n", asctime(newtime));
#endif
}

eaxdev::~eaxdev()
{
	// Disconnecting MP3-pipe
    iomp3::DisconnectMP3Pipe();
#ifdef DSPL__DEB_VER_STORE_ERRORS
    logFile[log_name]("MPEG-3 Pipe has been disconnected...\n");
#endif
	
	// Clean-Up the list of emitters and the list of sources
	this->Close();

    // release previously allocated objects
    if(this->camera){ this->camera->Release(); this->camera = NULL; };
    if(this->device){ this->device->Release(); this->device = NULL; };

#ifdef DSPL__DEB_VER_SHOW_COMMENT
    err.GiveStats();
    struct tm *newtime;        
    time_t long_time;
    time( &long_time );                // Get time as long integer. 
    newtime = localtime( &long_time ); // Convert to local time.

    logFile[log_name]("Closing sound log at [%.19s]... Done.\n", asctime(newtime));
#endif
}
 
//
// (Smart function)  Create
// Creating sound buffer for source with owner, so the map of emitters will be browsed
// for possible source duplication 
//
HRESULT eaxdev::CreateSound(void* dsbd,void* buf, const std::string& fname,const unsigned int& _owner_id,bool& duplicated) const
{
    REGISTERED_EMITTERS::const_iterator ci;
    sdsrc* src=NULL;

    for(ci=this->_emitmap.begin();ci!=this->_emitmap.end();++ci)
    {
        // trying to get duplication copy
        if((ci->first!=_owner_id)&&(src=ci->second->browse(fname)))
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
            logFile[log_name]("**COMMENT** Duplicating with id: found source for duplicating...\n");
#endif
            duplicated=true;
            return this->device->DuplicateSoundBuffer(reinterpret_cast<LPDIRECTSOUNDBUFFER>(src->GetBuf()),reinterpret_cast<LPDIRECTSOUNDBUFFER*>(buf));
        }
    }
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("**COMMENT** Creating original: no source's been found duplicating... creating original one.\n");
#endif
	// if user asks for hardware accleration
	DSBUFFERDESC* buf_desc=reinterpret_cast<DSBUFFERDESC*>(dsbd);
	if(this->_device_flags&PCI_CARD)
	{
		// clear STATIC flag 
		if(buf_desc->dwFlags&DSBCAPS_STATIC)buf_desc->dwFlags&=~DSBCAPS_STATIC;
		// set LOCDEFER
		buf_desc->dwFlags|=DSBCAPS_LOCDEFER;
	}
	else
	{
		// user hasnt asked for hardware acceleration,but if there's a free room - 
		//  - why shouldn't we use it?
		if(!(buf_desc->dwFlags&DSBCAPS_STATIC))
		{
			// Only for ISA cards:
			// check if there's a free hardware memory for this sound 
			// if there is, load sound into hardware memory
			DSCAPS caps;
			caps.dwSize=sizeof(DSCAPS);
			if(SUCCEEDED(this->device->GetCaps(&caps)))
			{
				if(caps.dwFreeHwMemBytes>buf_desc->dwBufferBytes)
					buf_desc->dwFlags|=DSBCAPS_STATIC;
#ifdef DSPL__DEB_VER_SHOW_COMMENT
				logFile[log_name]("Free HW memory:%d, size of buffer:%d\n",caps.dwFreeHwMemBytes,buf_desc->dwBufferBytes);
#endif
			}
#ifdef  DSPL__DEB_VER_STORE_ERRORS
			else
				err.PostError("IDirectSoundbuffer::::GetCaps()",-1,"failed trying to get free memory info for ISA card memory management",__FILE__,__LINE__);
#endif
		}
	}

    return this->device->CreateSoundBuffer(buf_desc,reinterpret_cast<LPDIRECTSOUNDBUFFER*>(buf),NULL);
}

//
// (Smart function) Create
// Creating sound buffer for source without owner, so the map of device sources will be browsed
// for possible source duplication
//
HRESULT eaxdev::CreateSound(void* dsbd,void* buf, const std::string& fname,bool& duplicated) const
{
    OWNEDBY_DEVICE_SOURCES::const_iterator ci=this->_devsrcmap.find(fname);

    if(ci!=this->_devsrcmap.end())
    {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        logFile[log_name]("**COMMENT** Duplicating: found source for duplicating...\n");
#endif
        duplicated=true;
        return this->device->DuplicateSoundBuffer(reinterpret_cast<LPDIRECTSOUNDBUFFER>(ci->second->GetBuf()),reinterpret_cast<LPDIRECTSOUNDBUFFER*>(buf));
    }
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("**COMMENT** Creating original: no source's been found duplicating... creating original one.\n");
#endif
	// if user asks for hardware accleration
	DSBUFFERDESC* buf_desc=reinterpret_cast<DSBUFFERDESC*>(dsbd);
	if(this->_device_flags&PCI_CARD)
	{
		// clear STATIC flag 
		if(buf_desc->dwFlags&DSBCAPS_STATIC)buf_desc->dwFlags&=~DSBCAPS_STATIC;
		// set LOCDEFER
		buf_desc->dwFlags|=DSBCAPS_LOCDEFER;
	}
	else
	{
		// user hasnt asked for hardware acceleration,but if there's a free room - 
		//  - why shouldn't we use it?
		if(!(buf_desc->dwFlags&DSBCAPS_STATIC))
		{
			// Only for ISA cards:
			// check if there's a free hardware memory for this sound 
			// if there is, load sound into hardware memory
			DSCAPS caps;
			caps.dwSize=sizeof(DSCAPS);
			if(SUCCEEDED(this->device->GetCaps(&caps)))
			{
				if(caps.dwFreeHwMemBytes>buf_desc->dwBufferBytes)
					buf_desc->dwFlags|=DSBCAPS_STATIC;
#ifdef DSPL__DEB_VER_SHOW_COMMENT
				logFile[log_name]("Free HW memory:%d, size of buffer:%d\n",caps.dwFreeHwMemBytes,buf_desc->dwBufferBytes);
#endif
			}
#ifdef  DSPL__DEB_VER_STORE_ERRORS
			else
				err.PostError("IDirectSoundbuffer::::GetCaps()",-1,"failed trying to get free memory info for ISA card memory management",__FILE__,__LINE__);
#endif
		}
	}

    return this->device->CreateSoundBuffer(buf_desc,reinterpret_cast<LPDIRECTSOUNDBUFFER*>(buf), NULL);
}

//
// (Smart function) Duplicate
// Creating sound buffer for source with owner, so the map of emitters will be browsed
// for possible source duplication 
//
HRESULT eaxdev::DuplicateSound(void* buf, const std::string& fname,const unsigned int& _owner_id) const
{
    REGISTERED_EMITTERS::const_iterator ci;
    sdsrc* src=NULL;

    for(ci=this->_emitmap.begin();ci!=this->_emitmap.end();++ci)
    {
        // trying to get duplication copy
        if((ci->first!=_owner_id)&&(src=ci->second->browse(fname)))
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
            logFile[log_name]("**COMMENT** Duplicating with id: found source for duplicating...\n");
#endif
            return this->device->DuplicateSoundBuffer(reinterpret_cast<LPDIRECTSOUNDBUFFER>(src->GetBuf()),reinterpret_cast<LPDIRECTSOUNDBUFFER*>(buf));
        }
    }
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("**COMMENT** Duplicating with id: no source's been found duplicating... try to create original.\n");
#endif

    return DSERR_INVALIDCALL;
}

//
// (Smart function) Duplicate
// Creating sound buffer for source without owner, so the map of device sources will be browsed
// for possible source duplication
//
HRESULT eaxdev::DuplicateSound(void* buf, const std::string& fname) const
{
    OWNEDBY_DEVICE_SOURCES::const_iterator ci=this->_devsrcmap.find(fname);

    if(ci!=this->_devsrcmap.end())
    {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        logFile[log_name]("**COMMENT** Duplicating: found source for duplicating...\n");
#endif
        return this->device->DuplicateSoundBuffer(reinterpret_cast<LPDIRECTSOUNDBUFFER>(ci->second->GetBuf()),reinterpret_cast<LPDIRECTSOUNDBUFFER*>(buf));
    }
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("**COMMENT** Duplicating : no source's been found duplicating... try to create original.\n");
#endif

    return DSERR_INVALIDCALL;
}

//
// Creating sound buffer
//
HRESULT eaxdev::CreateSound(void* dsbd,void* buf) const
{
	// if user asks for hardware accleration
	DSBUFFERDESC* buf_desc=reinterpret_cast<DSBUFFERDESC*>(dsbd);
	if(this->_device_flags&PCI_CARD)
	{
		// clear STATIC flag 
		if(buf_desc->dwFlags&DSBCAPS_STATIC)buf_desc->dwFlags&=~DSBCAPS_STATIC;
		// set LOCDEFER
		buf_desc->dwFlags|=DSBCAPS_LOCDEFER;
	}
	else
	{
		// user hasnt asked for hardware acceleration,but if there's a free room - 
		//  - why shouldn't we use it?
		if(!(buf_desc->dwFlags&DSBCAPS_STATIC))
		{
			// Only for ISA cards:
			// check if there's a free hardware memory for this sound 
			// if there is, load sound into hardware memory
			DSCAPS caps;
			caps.dwSize=sizeof(DSCAPS);
			if(SUCCEEDED(this->device->GetCaps(&caps)))
			{
				if(caps.dwFreeHwMemBytes>buf_desc->dwBufferBytes)
					buf_desc->dwFlags|=DSBCAPS_STATIC;
#ifdef DSPL__DEB_VER_SHOW_COMMENT
				logFile[log_name]("Free HW memory:%d, size of buffer:%d\n",caps.dwFreeHwMemBytes,buf_desc->dwBufferBytes);
#endif
			}
#ifdef  DSPL__DEB_VER_STORE_ERRORS
			else
				err.PostError("IDirectSoundbuffer::::GetCaps()",-1,"failed trying to get free memory info for ISA card memory management",__FILE__,__LINE__);
#endif
		}
	}

    return this->device->CreateSoundBuffer(buf_desc,reinterpret_cast<LPDIRECTSOUNDBUFFER*>(buf),NULL);
}

//
// Duplicating sound buffer
//
HRESULT eaxdev::DuplicateSound(void* srcbuf,void* destbuf) const
{
    return this->device->DuplicateSoundBuffer(reinterpret_cast<LPDIRECTSOUNDBUFFER>(srcbuf),reinterpret_cast<LPDIRECTSOUNDBUFFER*>(destbuf));
}

//
// Get eaxsrc exempliar
//
inline sound::sdsrc* eaxdev::PurchaseSource()const{ return new eaxsrc; };

//
// Get eaxsrc exempliar(sign.2)
//
inline void eaxdev::PurchaseSource(sound::sdsrc** source)const{ *source=new eaxsrc; };

//
// Get eaxsrc exempliar with id
//
inline void eaxdev::PurchaseSource(sound::sdsrc** source,const unsigned int& id)const{ *source=new eaxsrc(id); };

//
// Get eaxsrc exempliar with id(sign.2)
//
inline sound::sdsrc* eaxdev::PurchaseSource(const unsigned int& id) const 
{ 
    return new eaxsrc(id); 
};

/*

  This routine creates a dummy buffer from which to retrieve a property
  set interface. The property set interface retrieved here will be used
  to set the EAX Listener properties that effect all hardware 3D buffers.

*/
bool eaxdev::CreatePropertySet() 
{
    if(this->device==NULL) return false;
 
    LPDIRECTSOUNDBUFFER dummybuf=NULL;
    if(this->props_set==NULL)
    {
        WAVEFORMATEX wave;
        memset(&wave, 0, sizeof(WAVEFORMATEX)); 
        wave.wFormatTag = WAVE_FORMAT_PCM;
        wave.nChannels = 1; 
        wave.nSamplesPerSec = 22050; 
        wave.wBitsPerSample = 16; 
        wave.nBlockAlign = wave.wBitsPerSample / 8 * wave.nChannels;
        wave.nAvgBytesPerSec = wave.nSamplesPerSec * wave.nBlockAlign;

        DSBUFFERDESC dsbdesc;
        memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); 
        dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
        dsbdesc.dwFlags = DSBCAPS_STATIC|DSBCAPS_CTRL3D; 
        dsbdesc.dwBufferBytes = 64;  
        dsbdesc.lpwfxFormat = &wave; 

        if(this->device->CreateSoundBuffer(&dsbdesc, &dummybuf, NULL) != DS_OK)
        {
            return false; 
        }
    }

    if ((dummybuf->QueryInterface(IID_IKsPropertySet, (LPVOID *)&this->props_set)) != DS_OK)
    {
        dummybuf->Release();
        dummybuf = NULL;
        return false; 
    }

    return true;
}

/*

  This routine queries support for a given EAX listener property value and sets
  an associated flag in the eax_supports member if the property is supported. If
  the property is not supported, the return value is FALSE.

*/
bool eaxdev::QuerySupport(ULONG ulQuery)
{
    ULONG ulSupport = 0;
    HRESULT hr = this->props_set->QuerySupport(DSPROPSETID_EAX_ListenerProperties, ulQuery, &ulSupport);
    if(FAILED(hr)) return false;
 
    if ((ulSupport&(KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET)) == (KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET))
    {
        this->eax_supports |= (DWORD)(1 << ulQuery); 
        return true;
    }
 
    return false;
}

/*

  This routine handles the steps in creating a property set interface to be
  used for EAX and querying for EAX listener property support. This implmentation
  verifies that all EAX2.0 listener properties are available. If not the return
  value is FALSE.

*/
bool eaxdev::CreateEAX()
{
    if(!this->CreatePropertySet()) return false;
    eax_supports = 0;

    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_NONE))                return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_ALLPARAMETERS))       return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_ROOM))                return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_ROOMHF))              return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR))   return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_DECAYTIME))           return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_DECAYHFRATIO))        return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_REFLECTIONS))         return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY))    return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_REVERB))              return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_REVERBDELAY))         return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_ENVIRONMENT))         return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE))     return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION))return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF))     return false;
    if (!this->QuerySupport(DSPROPERTY_EAXLISTENER_FLAGS))               return false;

    return this->eax_supports ? true:false;
}

bool eaxdev::SetAll(LPEAXLISTENERPROPERTIES lpData)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_ALLPARAMETERS) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, NULL, 0, lpData, sizeof(EAXLISTENERPROPERTIES)));
}


bool eaxdev::SetRoom(LONG lValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_ROOM) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOM, NULL, 0, &lValue, sizeof(LONG)));
}

bool eaxdev::SetRoomHF(LONG lValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_ROOMHF) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOMHF, NULL, 0, &lValue, sizeof(LONG)));
}

bool eaxdev::SetRoomRolloff(float fValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR, NULL, 0, &fValue, sizeof(float)));
}

bool eaxdev::SetDecayTime(float fValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_DECAYTIME) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_DECAYTIME, NULL, 0, &fValue, sizeof(float)));
}

bool eaxdev::SetDecayHFRatio(float fValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_DECAYHFRATIO) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_DECAYHFRATIO, NULL, 0, &fValue, sizeof(float)));
}

bool eaxdev::SetReflections(LONG lValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_REFLECTIONS) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_REFLECTIONS, NULL, 0, &lValue, sizeof(LONG)));
}

bool eaxdev::SetReflectionsDelay(float fValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY, NULL, 0, &fValue, sizeof(float)));
}

bool eaxdev::SetReverb(LONG lValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_REVERB) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_REVERB, NULL, 0, &lValue, sizeof(LONG)));
}

bool eaxdev::SetReverbDelay(float fValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_REVERBDELAY) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_REVERBDELAY, NULL, 0, &fValue, sizeof(float)));
}

bool eaxdev::SetEAXEnvironment(DWORD dwValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_ENVIRONMENT) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENT, NULL, 0, &dwValue, sizeof(DWORD)));
}

bool eaxdev::SetEAXEnvironmentSize(float fValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE, NULL, 0, &fValue, sizeof(float)));
}

bool eaxdev::SetEnvironmentDiffusion(float fValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION, NULL, 0, &fValue, sizeof(float)));
}

bool eaxdev::SetAirAbsorption(float fValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF, NULL, 0, &fValue, sizeof(float)));
}

bool eaxdev::SetFlags(DWORD dwValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_FLAGS) ) return FALSE;
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwValue, sizeof(DWORD)));
}

bool eaxdev::SetScaleDecayTime(bool bValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_FLAGS) ) return FALSE;
	
	DWORD dwReceived, dwFlags;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD), &dwReceived)))
		return FALSE;
	
	dwFlags &= (0xFFFFFFFF ^ EAXLISTENERFLAGS_DECAYTIMESCALE);
	if ( bValue ) dwFlags |= EAXLISTENERFLAGS_DECAYTIMESCALE;
	
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD)));
}

bool eaxdev::SetScaleReflections(bool bValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_FLAGS) ) return FALSE;
	
	DWORD dwReceived, dwFlags;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD), &dwReceived)))
		return FALSE;
	
	dwFlags &= (0xFFFFFFFF ^ EAXLISTENERFLAGS_REFLECTIONSSCALE);
	if ( bValue ) dwFlags |= EAXLISTENERFLAGS_REFLECTIONSSCALE;
	
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD)));
}

bool eaxdev::SetScaleReflectionsDelay(bool bValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_FLAGS) ) return FALSE;
	
	DWORD dwReceived, dwFlags;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD), &dwReceived)))
		return FALSE;
	
	dwFlags &= (0xFFFFFFFF ^ EAXLISTENERFLAGS_REFLECTIONSDELAYSCALE);
	if ( bValue ) dwFlags |= EAXLISTENERFLAGS_REFLECTIONSDELAYSCALE;
	
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD)));
}

bool eaxdev::SetScaleReverb(bool bValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_FLAGS) ) return FALSE;
	
	DWORD dwReceived, dwFlags;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD), &dwReceived)))
		return FALSE;
	
	dwFlags &= (0xFFFFFFFF ^ EAXLISTENERFLAGS_REVERBSCALE);
	if ( bValue ) dwFlags |= EAXLISTENERFLAGS_REVERBSCALE;
	
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD)));
}

bool eaxdev::SetScaleReverbDelay(bool bValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_FLAGS) ) return FALSE;
	
	DWORD dwReceived, dwFlags;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD), &dwReceived)))
		return FALSE;
	
	dwFlags &= (0xFFFFFFFF ^ EAXLISTENERFLAGS_REVERBDELAYSCALE);
	if ( bValue ) dwFlags |= EAXLISTENERFLAGS_REVERBDELAYSCALE;
	
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD)));
}

bool eaxdev::SetClipDecayHF(bool bValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_FLAGS) ) return FALSE;
	
	DWORD dwReceived, dwFlags;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD), &dwReceived)))
		return FALSE;
	
	dwFlags &= (0xFFFFFFFF ^ EAXLISTENERFLAGS_DECAYHFLIMIT);
	if ( bValue ) dwFlags |= EAXLISTENERFLAGS_DECAYHFLIMIT;
	
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD)));
}

bool eaxdev::GetAll(LPEAXLISTENERPROPERTIES lpData)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_ALLPARAMETERS) ) return FALSE;
	ULONG ulReceived;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, NULL, 0, lpData, sizeof(EAXLISTENERPROPERTIES), &ulReceived)) ) return FALSE;
	if ( ulReceived != sizeof(EAXLISTENERPROPERTIES) ) return FALSE;
	return TRUE;
}

bool eaxdev::GetDecayTime(float* pfValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_DECAYTIME) ) return FALSE;
	ULONG ulReceived;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_DECAYTIME, NULL, 0, pfValue, sizeof(float), &ulReceived)) ) return FALSE;
	if ( ulReceived != sizeof(float) ) return FALSE;
	return TRUE;
}

bool eaxdev::GetReflections(long* plValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_REFLECTIONS) ) return FALSE;
	ULONG ulReceived;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_REFLECTIONS, NULL, 0, plValue, sizeof(long), &ulReceived)) ) return FALSE;
	if ( ulReceived != sizeof(long) ) return FALSE;
	return TRUE;
}

bool eaxdev::GetReflectionsDelay(float* pfValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY) ) return FALSE;
	ULONG ulReceived;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY, NULL, 0, pfValue, sizeof(float), &ulReceived)) ) return FALSE;
	if ( ulReceived != sizeof(float) ) return FALSE;
	return TRUE;
}

bool eaxdev::GetReverb(long* plValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_REVERB) ) return FALSE;
	ULONG ulReceived;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_REVERB, NULL, 0, plValue, sizeof(long), &ulReceived)) ) return FALSE;
	if ( ulReceived != sizeof(long) ) return FALSE;
	return TRUE;
}

bool eaxdev::GetReverbDelay(float* pfValue)
{
	if ( !(this->eax_supports & (DWORD)1 << DSPROPERTY_EAXLISTENER_REVERBDELAY) ) return FALSE;
	ULONG ulReceived;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_REVERBDELAY, NULL, 0, pfValue, sizeof(float), &ulReceived)) ) return FALSE;
	if ( ulReceived != sizeof(float) ) return FALSE;
	return TRUE;
}

bool eaxdev::SetListenerRolloff(float fValue)
{
	if ( this->camera == NULL ) return FALSE;
	return SUCCEEDED(this->camera->SetRolloffFactor(fValue, DS3D_DEFERRED ));
}

//
// Initialize sound device
//
bool eaxdev::Initialize(HWND hwnd)
{
    HRESULT             hr = 0;
    DSBUFFERDESC        dsbdesc;
    LPDIRECTSOUNDBUFFER primbuf;
	SndCompiler compiler;

    if(FAILED(hr=DirectSoundEnumerate((LPDSENUMCALLBACK)EnumerateSoundDevices,NULL)))
	{
#ifdef  DSPL__DEB_VER_STORE_ERRORS
        err.PostError("DirectSoundEnumerate()", hr, "failed in sddev::Initialize",__FILE__,__LINE__);
        logFile[log_name]("\n\nCRITICAL ERROR! ...DirectSound enumeration has failed. You will not be able to operate with sound...\n\n");
#endif
        return false; // smthg is wrong, you've got to handle
	}

    if(FAILED(hr = EAXDirectSoundCreate(NULL, &this->device, NULL)))
    {
#ifdef  DSPL__DEB_VER_STORE_ERRORS
        err.PostError("EAXDirectSoundCreate()", hr, "failed in eaxdev::Initialize",__FILE__,__LINE__);
        logFile[log_name]("\n\nCRITICAL ERROR! ...Initialization of EAXDirectSound object has failed. You will not be able to operate with sound...\n\n");
#endif
        return false; // smthg is wrong, you've got to handle
    }

    if(FAILED(hr = this->device->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("SetCooperativeLevel()", hr, "failed in eaxdev::Initialize",__FILE__,__LINE__);
#endif

        return false; // smthg is wrong, you've got to handle
    }
    // build the listener

    // Obtain primary buffer, asking it for 3D control
    ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
    if( FAILED( hr = this->device->CreateSoundBuffer(&dsbdesc,&primbuf,NULL)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("Createsoundbuffer()", hr, "failed in eaxdev::Initialize",__FILE__,__LINE__);
#endif
        return false;
    }

    if( FAILED( hr = primbuf->QueryInterface(IID_IDirectSound3DListener, 
                                              (VOID**)&this->camera)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundbuffer::QueryInterface()", hr, "failed in eaxdev::Initialize",__FILE__,__LINE__);
#endif

        return false;
    }

    if(primbuf)primbuf->Release();

    if(!this->CreateEAX())
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("eaxdev::Initialize()", hr, "failed creating EAX support",__FILE__,__LINE__);
#endif
        // EAX support wasn't detected...Not critical error
        // EAX Featurez ain't available...EAX continue in DirectSound layer
        //return 0;
    }

    compiler.Compile(script_filename);

//
// setting step by step
//
	if(ListenerDefs._doppler_factor.changed)this->camera->SetDopplerFactor(ListenerDefs._doppler_factor.value,DS3D_IMMEDIATE);
	if(ListenerDefs._rolloff.changed)      this->camera->SetRolloffFactor(ListenerDefs._rolloff.value,DS3D_IMMEDIATE);
    if(ListenerDefs._room.changed)	      this->SetRoom(ListenerDefs._room.value);
    if(ListenerDefs._roomHF.changed)		  this->SetRoomHF(ListenerDefs._roomHF.value);
    if(ListenerDefs._room_rolloff.changed)	  this->SetRoomRolloff(ListenerDefs._room_rolloff.value);
    if(ListenerDefs._decay_time.changed)	  this->SetDecayTime(ListenerDefs._decay_time.value);
    if(ListenerDefs._decay_HFRatio.changed) this->SetDecayHFRatio(ListenerDefs._decay_HFRatio.value);
    if(ListenerDefs._reflections.changed)  this->SetReflections(ListenerDefs._reflections.value);
    if(ListenerDefs._reflections_delay.changed)this->SetReflectionsDelay(ListenerDefs._reflections_delay.value);
    if(ListenerDefs._reverb.changed)		  this->SetReverb(ListenerDefs._reverb.value);
    if(ListenerDefs._reverb_delay.changed)  this->SetReverbDelay(ListenerDefs._reverb_delay.value);
    if(ListenerDefs._environment.changed)  this->SetEAXEnvironment(ListenerDefs._environment.value);
    if(ListenerDefs._environment_size.changed)       this->SetEAXEnvironmentSize(ListenerDefs._environment_size.value);
    if(ListenerDefs._environment_diffusion.changed)  this->SetEnvironmentDiffusion(ListenerDefs._environment_diffusion.value);
    if(ListenerDefs._air_absorption.changed)         this->SetAirAbsorption(ListenerDefs._air_absorption.value);
    if(ListenerDefs._lflags&SCALE_DECAY_TIME)       this->SetScaleDecayTime(true);
    if(ListenerDefs._lflags&CLIP_DECAY_HF)          this->SetClipDecayHF(true);
    if(ListenerDefs._lflags&SCALE_REFLECTIONS)      this->SetScaleReflections(true);
    if(ListenerDefs._lflags&SCALE_REFLECTIONS_DELAY)this->SetScaleReflectionsDelay(true);
    if(ListenerDefs._lflags&SCALE_REVERB)           this->SetScaleReverb(true);
    if(ListenerDefs._lflags&SCALE_REVERB_DELAY)     this->SetScaleReverbDelay(true);
    if(ListenerDefs._flags.changed)                 this->SetFlags(ListenerDefs._flags.value);

//
// setting all at once
//
/*
    EAXLISTENERPROPERTIES data;
    if(!GetAll(&data))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("eaxdev::Initialize()", -1, "failed reading data for listener",__FILE__,__LINE__);
#endif
        return 1;
    }

    if(ListenerDefs._room.changed)                
        data.lRoom=ListenerDefs._room.value;
    if(ListenerDefs._roomHF.changed)
        data.lRoomHF=ListenerDefs._roomHF.value;
    if(ListenerDefs.RoomRolloff.changed)         
        data.flRoomRolloffFactor=ListenerDefs.RoomRolloff.value;  
    if(ListenerDefs._decay_time.changed)           
        data.flDecayTime=ListenerDefs._decay_time.value;            
    if(ListenerDefs._decay_HFRatio.changed)        
        data.flDecayHFRatio=ListenerDefs._decay_HFRatio.value;        
    if(ListenerDefs._reflections.changed)         
        data.lReflections=ListenerDefs._reflections.value;           
    if(ListenerDefs._reflections_delay.changed)    
        data.flReflectionsDelay=ListenerDefs._reflections_delay.value;
    if(ListenerDefs._reverb.changed)              
        data.lReverb=ListenerDefs._reverb.value;               
    if(ListenerDefs._reverb_delay.changed)         
        data.flReverbDelay=ListenerDefs._reverb_delay.value;        
    if(ListenerDefs._environment.changed)         
        data.dwEnvironment=ListenerDefs._environment.value;        
    if(ListenerDefs._environment_size.changed)     
        data.flEnvironmentSize=ListenerDefs._environment_size.value;    
    if(ListenerDefs._environment_diffusion.changed)
        data.flEnvironmentDiffusion=ListenerDefs._environment_diffusion.value;
    if(ListenerDefs._air_absorption.changed)       
        data.flAirAbsorptionHF=ListenerDefs._air_absorption.value;
    if(ListenerDefs._lflags.changed)               
        data.dwFlags=ListenerDefs._lflags.value;              

    if(!SetAll(&data))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("eaxdev::Initialize()", -1, "failed setting data for listener",__FILE__,__LINE__);
#endif
        return 1;
    }
*/
// Initializing MP3
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(iomp3::InitMP3Pipe())
		this->_device_flags|=MP3_INITIALIZED_SUCCESSFULLY;
	else
		err.PostError("dsdev::Initialize()",-1,"Failed initializing MPEG-3 Decoder",__FILE__,__LINE__);		
#else
    if(iomp3::InitMP3Pipe())
		this->_device_flags|=MP3_INITIALIZED_SUCCESSFULLY;
#endif

    // return success
    return true;
}

//
// Set properties for device
//
void eaxdev::SetDeviceProperty(lprops prop,void* valaddr)
{
	if(valaddr)
	{
		switch(prop)
		{
		default:
		case PROP_ROLLOFF:
			this->camera->SetRolloffFactor(*reinterpret_cast<float*>(valaddr),DS3D_IMMEDIATE);
			break;
		case PROP_DOPPLER_FACTOR:
			this->camera->SetDopplerFactor(*reinterpret_cast<float*>(valaddr),DS3D_IMMEDIATE);
			break;
		case PROP_ROOM:
			this->SetRoom(*reinterpret_cast<LONG*>(valaddr));
			break;
		case PROP_ROOM_HF:
			this->SetRoomHF(*reinterpret_cast<LONG*>(valaddr));
			break;
		case PROP_REFLECTIONS:
			this->SetRoomRolloff(*reinterpret_cast<LONG*>(valaddr));
			break;
		case PROP_ENVIRONMENT:
			this->SetEAXEnvironment(*reinterpret_cast<DWORD*>(valaddr));
			break;
		case PROP_PROP_FLAGS:
			this->SetFlags(*reinterpret_cast<DWORD*>(valaddr));
			break;
		case PROP_REVERB_DELAY:
			this->SetReverbDelay(*reinterpret_cast<float*>(valaddr));
			break;
		case PROP_REFLECTIONS_DELAY:
			this->SetReflectionsDelay(*reinterpret_cast<float*>(valaddr));
			break;
		case PROP_ROOM_ROLLOFF:
			this->SetRoomRolloff(*reinterpret_cast<float*>(valaddr));
			break;
		case PROP_DECAY_TIME:
			this->SetDecayTime(*reinterpret_cast<float*>(valaddr));
			break;
		case PROP_DECAY_HF_RATIO:
			this->SetDecayHFRatio(*reinterpret_cast<float*>(valaddr));
			break;
		case PROP_ENVIRONMENT_SIZE:
			this->SetEAXEnvironmentSize(*reinterpret_cast<float*>(valaddr));
			break;
		case PROP_ENVIRONMENT_DIFFUSION:
			this->SetEnvironmentDiffusion(*reinterpret_cast<float*>(valaddr));
			break;
		case PROP_AIR_ABSORPTION:
			this->SetAirAbsorption(*reinterpret_cast<float*>(valaddr));
			break;
		case PROP__FLAGS:
			this->_device_flags|=*reinterpret_cast<DWORD*>(valaddr);
			break;
		}
	}
}

//
// Playing sound (effect of slow volume growing)
//
void eaxdev::SmoothPlay(const tray& tr, const point3* new_pos)
{
	if(tr._cell1)
	{
		SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(*(tr._cell1));
		if(p!=sound::dscmap.end())
		{
			const srcdsc& description   = p->second;
			const std::string& filename = tr._cell2 ? *(tr._cell2) : p->second._name;

			sdsrc* source=NULL;
			source=sound::dev->PurchaseSource();

			if(source->Initialize(description,filename.c_str()))
			{
				((LPDIRECTSOUNDBUFFER)(source->GetBuf()))->SetVolume(DSBVOLUME_MIN);
				SRCFUNC func=&sdsrc::SmoothVolumeExpand;
				source->SetSourceExec(func);
				if(new_pos)source->SetPos(*new_pos);
				source->Play();
				typedef OWNEDBY_DEVICE_SOURCES::value_type Pairs;
				Pairs new_pair(filename,source);
				this->_devsrcmap.insert(new_pair);
			}
			else
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("eaxsrc::Initialize()", -1, "failed in eaxdev::SmoothPlay()",__FILE__,__LINE__);
#endif
				this->ThrashSource(source);
			}
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("eaxdev::SmoothPlay()", -1, "description was not found:" + *tr._cell1,__FILE__,__LINE__);
#endif
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("eaxdev::SmoothPlay()", -1, "description was not found(NULL)",__FILE__,__LINE__);
#endif
}

//
// Update sound device (as listener changed its position)
//
void eaxdev::Update(const point3& pos,
					const point3& vel,
					const point3& front,
					const point3& up) 
{
	// first, be sure that not too frequent calls
	static float  _prev_game_time = 0.f;
	float	       _cur_game_time = Timer::GetSeconds();
	float _min_game_time_interval = 1.f / Options::GetFloat("system.sound.update_freq");

	if((_cur_game_time - _prev_game_time) < _min_game_time_interval) return;
	_prev_game_time = _cur_game_time;

    // updating camera coordinates and orientation
    D3DVALUE px=pos.x,py=pos.y,pz=pos.z,
             vx=vel.x,vy=vel.y,vz=vel.z,
             fx=front.x,fy=front.y,fz=front.z,
             tx=up.x  ,ty=up.y  ,tz=up.z;

    // setting new parameteres
    this->camera->SetPosition   (px, py, pz, DS3D_DEFERRED);
    this->camera->SetVelocity   (vx, vy, vz, DS3D_DEFERRED);
    this->camera->SetOrientation(fx, fy, fz, tx, ty, tz, DS3D_DEFERRED);
    //camera->CommitDeferredSettings();

    // updating sound emitters
    REGISTERED_EMITTERS::const_iterator ci;
    for(ci=this->_emitmap.begin();ci!=this->_emitmap.end();++ci)
    {
        if(ci->second) 
        {
            if(ci->second->IsChangedPos())
                ci->second->SetPos(ci->second->GetPos());
        }
#ifdef DSPL__DEB_VER_STORE_ERRORS
        else
            err.PostError("eaxdev::Update()",-1,"attempt to use missed emitter in map",__FILE__,__LINE__);
#endif
    }

    // applying changed settings
    this->camera->CommitDeferredSettings();

    OWNEDBY_DEVICE_SOURCES::iterator cis;
    for(cis=this->_devsrcmap.begin();cis!=this->_devsrcmap.end();++cis)
    {
		// if source is playing or it's _lifetype is PERMANENT
		// then let it go, else delete from the memory
		bool playing=cis->second->IsPlaying();
        if(playing||(cis->second->GetLifeType()==PERMANENT))
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
			if(playing)
				logFile[log_name]("File [%s]::[playing]\n",cis->first);
			else
				logFile[log_name]("File [%s]::[permanent resident]\n",cis->first);
#endif
            cis->second->Execute();
        }
        else
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
            logFile[log_name]("File [%s]::[to be destructed].\n",cis->first);
#endif
            this->ThrashSource(cis->second);
            this->_devsrcmap.erase(cis);
        }
    }
#ifdef DSPL__DEBUG_INFO
	static DSCAPS caps;
	caps.dwSize=sizeof(DSCAPS);
	if(SUCCEEDED(this->device->GetCaps(&caps)))
	{
		logFile[log_name]("Memory statistics:\n\tHW usage:busy HW buffers %d\n\t"
												"HW currently mixing %d buffers\n\t"
												"CPU SW percentage overhead:%d\n",caps.dwTotalHwMemBytes-caps.dwFreeHwMemBytes,
																				  caps.dwMaxHwMixingAllBuffers-caps.dwFreeHwMixingAllBuffers,
																				  caps.dwPlayCpuOverheadSwBuffers
																				  );
	}
#endif
}

//
// Update sound device (as listener is still there)
//
void eaxdev::Update() 
{
	// first, be sure that not too frequent calls
	static float  _prev_game_time = 0.f;
	float	       _cur_game_time = Timer::GetSeconds();
	float _min_game_time_interval = 1.f / Options::GetFloat("system.sound.update_freq");

	if((_cur_game_time - _prev_game_time) < _min_game_time_interval) return;
	_prev_game_time = _cur_game_time;

    // updating sound emitters
    REGISTERED_EMITTERS::const_iterator ci;
    //logFile[log_name]("Scanning map of emitters...");
    for(ci=this->_emitmap.begin();ci!=this->_emitmap.end();++ci)
    {
        if(ci->second) 
        {
            if(ci->second->IsChangedPos())
                ci->second->SetPos(ci->second->GetPos());

            //logFile[log_name]("\nUpdating sound:Found registered emitter in map [sid=%d].",ci->second->mysid());
        }
#ifdef DSPL__DEB_VER_STORE_ERRORS
        else
            err.PostError("eaxdev::Update()",-1,"attempt to use missed emitter in map",__FILE__,__LINE__);
#endif
    }
    //logFile[log_name]("Finished scanning.\n");

    // applying changed settings
    this->camera->CommitDeferredSettings();

    OWNEDBY_DEVICE_SOURCES::iterator cis;
    for(cis=this->_devsrcmap.begin();cis!=this->_devsrcmap.end();++cis)
    {
		// if source is playing or it's _lifetype is PERMANENT
		// then let it go, else delete from the memory
		bool playing=cis->second->IsPlaying();
        if(playing||(cis->second->GetLifeType()==PERMANENT))
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
			if(playing)
				logFile[log_name]("File [%s]::[playing]\n",cis->first);
			else
				logFile[log_name]("File [%s]::[permanent resident]\n",cis->first);
#endif
            cis->second->Execute();
        }
        else
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
            logFile[log_name]("File [%s]::[to be destructed].\n",cis->first);
#endif
            this->ThrashSource(cis->second);
            this->_devsrcmap.erase(cis);
        }
    }
#ifdef DSPL__DEBUG_INFO
	static DSCAPS caps;
	caps.dwSize=sizeof(DSCAPS);
	if(SUCCEEDED(this->device->GetCaps(&caps)))
	{
		logFile[log_name]("Memory statistics:\n\tHW usage:busy HW buffers %d\n\t"
												"HW currently mixing %d buffers\n\t"
												"CPU SW percentage overhead:%d\n",caps.dwTotalHwMemBytes-caps.dwFreeHwMemBytes,
																				  caps.dwMaxHwMixingAllBuffers-caps.dwFreeHwMixingAllBuffers,
																				  caps.dwPlayCpuOverheadSwBuffers
																				  );
	}
#endif
}

//
// Adjust output volume
//
void eaxdev::AdjustGlobalVolume(int value)
{
    // setting global volume for all sources except
    this->GlobalVolume+=value;

    if(this->GlobalVolume<DSBVOLUME_MIN)
		this->GlobalVolume=DSBVOLUME_MIN;

    if(this->GlobalVolume>DSBVOLUME_MAX)
		this->GlobalVolume=DSBVOLUME_MAX;

    REGISTERED_EMITTERS::const_iterator cie;
    for(cie=this->_emitmap.begin();cie!=this->_emitmap.end();++cie)
    {
        if(cie->second) 
            cie->second->AdjustVolume(value);
    }
    OWNEDBY_DEVICE_SOURCES::const_iterator cis;
    for(cis=this->_devsrcmap.begin();cis!=this->_devsrcmap.end();++cis)
    {
        if(cis->second) 
            cis->second->AdjustVolume(value);
    }
}

//
// Update only music volume
//
void eaxdev::AdjustMusicVolume(int value)
{
    iomp3::AdjustVolumeMusic(value);
}

//
// get camera position 
//
void eaxdev::GetPos(point3& pos)const
{
    D3DVECTOR campos;
    HRESULT hr=0;
    if(SUCCEEDED(hr=this->camera->GetPosition(&campos)))
    {
        pos.x=campos.x; pos.y=campos.y; pos.z=campos.z;
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("GetVelocity()",hr,"failed in eaxdev::GetVel",__FILE__,__LINE__);
#endif
}

//
// get camera velocity 
//
void eaxdev::GetVel(point3& vel)const
{
    D3DVECTOR camvel;
    HRESULT hr=0;
    if(SUCCEEDED(hr=this->camera->GetVelocity(&camvel)))
    {
        vel.x=camvel.x; vel.y=camvel.y; vel.z=camvel.z;
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("GetVelocity()",hr,"failed in eaxdev::GetVel",__FILE__,__LINE__);
#endif
}

//
// set camera position 
//
void eaxdev::SetPos(const point3& pos)
{
    D3DVALUE x=pos.x,
             y=pos.y,
             z=pos.z;

    this->camera->SetPosition(x, y, z, DS3D_DEFERRED);
}

//
// set camera velocity 
//
void eaxdev::SetVel(const point3& vel)
{
    D3DVALUE x=vel.x,
             y=vel.y,
             z=vel.z;

    this->camera->SetVelocity(x, y, z, DS3D_DEFERRED);
}

//
// set camera orientation
//
void eaxdev::BeOriented(const point3& front, const point3& top)
{
    D3DVALUE fx=front.x,
             fy=front.y,
             fz=front.z,
             tx=top.x,
             ty=top.y,
             tz=top.z;

    this->camera->SetOrientation(fx, fy, fz, tx, ty, tz, DS3D_DEFERRED);
}

//
// Set camera position
//
void eaxdev::SetXYZ(const point3& pos,const point3& vel,const point3& front,const point3& top)
{
    D3DVALUE px=pos.x,py=pos.y,pz=pos.z,
             vx=vel.x,vy=vel.y,vz=vel.z,
             fx=front.x,fy=front.y,fz=front.z,
             tx=top.x  ,ty=top.y  ,tz=top.z;

    // setting new parameteres
    this->camera->SetPosition   (px, py, pz, DS3D_DEFERRED);
    this->camera->SetVelocity   (vx, vy, vz, DS3D_DEFERRED);
    this->camera->SetOrientation(fx, fy, fz, tx, ty, tz, DS3D_DEFERRED);
    this->camera->CommitDeferredSettings();
}

/***************************************************************************
****************************************************************************

                            eaxsrc realization

***************************************************************************
***************************************************************************/

inline eaxsrc::eaxsrc() : sdsrc(),
						  buf(NULL),
						  buf3d(NULL),
						  props_set(NULL),
						  Volume(0)
{
}

inline eaxsrc::eaxsrc(unsigned int id) : sdsrc(id),
										 buf(NULL),
										 buf3d(NULL),
										 props_set(NULL),
										 Volume(0)
{
}

eaxsrc::eaxsrc(const eaxsrc& sndbuf) : buf(NULL),
									   buf3d(NULL),
									   props_set(NULL)
{
}

eaxsrc::~eaxsrc()
{
    if(this->props_set)
	{ 
		this->props_set->Release(); 
		this->props_set=NULL; 
	};

    if(this->buf3d)
	{ 
		this->buf3d->Release();
		this->buf3d=NULL; 
	};

    if(this->buf)
	{ 
		this->buf->Release();
		this->buf=NULL; 
	};
}

bool eaxsrc::SetSourceAll(LPEAXBUFFERPROPERTIES lpData)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ALLPARAMETERS, NULL, 0, lpData, sizeof(EAXBUFFERPROPERTIES)));
}

bool eaxsrc::SetSourceDirect(LONG lValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_DIRECT, NULL, 0, &lValue, sizeof(LONG)));
}

bool eaxsrc::SetSourceDirectHF(LONG lValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_DIRECTHF, NULL, 0, &lValue, sizeof(LONG)));
}

bool eaxsrc::SetSourceRoom(LONG lValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ROOM, NULL, 0, &lValue, sizeof(LONG)));
}

bool eaxsrc::SetSourceRoomHF(LONG lValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ROOMHF, NULL, 0, &lValue, sizeof(LONG)));
}

bool eaxsrc::SetSourceRolloff(float fValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ROOMROLLOFFFACTOR, NULL, 0, &fValue, sizeof(float)));
}

bool eaxsrc::SetSourceOutside(LONG lValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OUTSIDEVOLUMEHF, NULL, 0, &lValue, sizeof(LONG)));
}

bool eaxsrc::SetSourceAbsorption(float fValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_AIRABSORPTIONFACTOR, NULL, 0, &fValue, sizeof(float)));
}

bool eaxsrc::SetSourceFlags(DWORD dwValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_FLAGS, NULL, 0, &dwValue, sizeof(DWORD)));
}

bool eaxsrc::SetSourceObstruction(LONG lValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OBSTRUCTION, NULL, 0, &lValue, sizeof(LONG)));
}

bool eaxsrc::SetSourceObstructionLF(float fValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OBSTRUCTIONLFRATIO, NULL, 0, &fValue, sizeof(float)));
}

bool eaxsrc::SetSourceOcclusion(LONG lValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSION, NULL, 0, &lValue, sizeof(LONG)));
}

bool eaxsrc::SetSourceOcclusionLF(float fValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO, NULL, 0, &fValue, sizeof(float)));
}

bool eaxsrc::SetSourceOcclusionRoom(float fValue)
{
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO, NULL, 0, &fValue, sizeof(float)));
}

bool eaxsrc::SetPositionX(float fValue)
{
	D3DVECTOR vec;
	if ( this->buf3d->GetPosition(&vec) != DS_OK ) return FALSE;
	return SUCCEEDED(this->buf3d->SetPosition(fValue, vec.y, vec.z, DS3D_DEFERRED));
}

bool eaxsrc::SetPositionY(float fValue)
{
	D3DVECTOR vec;
	if ( this->buf3d->GetPosition(&vec) != DS_OK ) return FALSE;
	return SUCCEEDED(this->buf3d->SetPosition(vec.x, fValue, vec.z, DS3D_DEFERRED));
}

bool eaxsrc::SetPositionZ(float fValue)
{
	D3DVECTOR vec;
	if ( this->buf3d->GetPosition(&vec) != DS_OK ) return FALSE;
	return SUCCEEDED(this->buf3d->SetPosition(vec.x, vec.y, fValue, DS3D_DEFERRED));
}

bool eaxsrc::SetConeOrientationX(float fValue)
{
	D3DVECTOR vec;
	if ( this->buf3d->GetConeOrientation(&vec) != DS_OK ) return FALSE;
	return SUCCEEDED(this->buf3d->SetConeOrientation(fValue, vec.y, vec.z, DS3D_DEFERRED));
}

bool eaxsrc::SetConeOrientationY(float fValue)
{
	D3DVECTOR vec;
	if ( this->buf3d->GetConeOrientation(&vec) != DS_OK ) return FALSE;
	return SUCCEEDED(this->buf3d->SetConeOrientation(vec.x, fValue, vec.z, DS3D_DEFERRED));
}

bool eaxsrc::SetConeOrientationZ(float fValue)
{
	D3DVECTOR vec;
	if ( this->buf3d->GetConeOrientation(&vec) != DS_OK ) return FALSE;
	return SUCCEEDED(this->buf3d->SetConeOrientation(vec.x, vec.y, fValue, DS3D_DEFERRED));
}

bool eaxsrc::SetConeInsideAngle(DWORD dwValue)
{
	DWORD dwIn, dwOut;
	if ( this->buf3d->GetConeAngles(&dwIn, &dwOut) != DS_OK ) return FALSE;
	return SUCCEEDED(this->buf3d->SetConeAngles(dwValue, dwOut, DS3D_DEFERRED));
}

bool eaxsrc::SetConeOutsideAngle(DWORD dwValue)
{
	DWORD dwIn, dwOut;
	if ( this->buf3d->GetConeAngles(&dwIn, &dwOut) != DS_OK ) return FALSE;
	return SUCCEEDED(this->buf3d->SetConeAngles(dwIn, dwValue, DS3D_DEFERRED));
}

bool eaxsrc::SetConeOutsideVolume(DWORD dwValue)
{
	return SUCCEEDED(this->buf3d->SetConeOutsideVolume(dwValue, DS3D_DEFERRED));
}

bool eaxsrc::SetSourceAffectDirectHF(bool bValue)
{
	DWORD dwReceived, dwFlags;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD), &dwReceived)))
		return FALSE;
	
	dwFlags &= (0xFFFFFFFF ^ EAXBUFFERFLAGS_DIRECTHFAUTO);
	if ( bValue ) dwFlags |= EAXBUFFERFLAGS_DIRECTHFAUTO;
	
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD)));
}

bool eaxsrc::SetSourceAffectRoom(bool bValue)
{
	DWORD dwReceived, dwFlags;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD), &dwReceived)))
		return FALSE;
	
	dwFlags &= (0xFFFFFFFF ^ EAXBUFFERFLAGS_ROOMAUTO);
	if ( bValue ) dwFlags |= EAXBUFFERFLAGS_ROOMAUTO;
	
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD)));
}

bool eaxsrc::SetSourceAffectRoomHF(bool bValue)
{
	DWORD dwReceived, dwFlags;
	if ( FAILED(this->props_set->Get(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD), &dwReceived)))
		return FALSE;
	
	dwFlags &= (0xFFFFFFFF ^ EAXBUFFERFLAGS_ROOMHFAUTO);
	if ( bValue ) dwFlags |= EAXBUFFERFLAGS_ROOMHFAUTO;
	
	return SUCCEEDED(this->props_set->Set(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_FLAGS, NULL, 0, &dwFlags, sizeof(DWORD)));
}

bool eaxsrc::SetMinDistance(float fValue)
{
	return SUCCEEDED(this->buf3d->SetMinDistance(fValue, DS3D_DEFERRED));
}

bool eaxsrc::SetMaxDistance(float fValue)
{
	return SUCCEEDED(this->buf3d->SetMaxDistance(fValue, DS3D_DEFERRED));
}

//
// Initialize sound source
//
bool eaxsrc::Initialize(const srcdsc& sdc,const char* ovrfname)
{
    HRESULT hr = 0;

	// if it's MP3-type source, then initialization is quite different
	if(sdc._Flags&MPEG3FILE)
	{
		// test if MP3 is successfully initialized if it is not - fail
		return sound::dev->TestDeviceFlags(MP3_INITIALIZED_SUCCESSFULLY) ? this->Initialize2(sdc,ovrfname):false;
	}

    // remember description address
    this->_mydesc=&sdc;

    bool buffer3d=true;
    if(sdc._Flags&DISABLE_3D)buffer3d=false;

    // if source is owned by someone(_owner_id!=0), then give id of owner
    // to DuplicateSound(...) for browsing map of emitters for potential source duplicating.
    // if source haven't owner, then there's no reason to browse emitters
    // note: duplicating is economy of memory

	const char* file_to_load=NULL;
	if(ovrfname)
	{
		file_to_load=ovrfname;
		this->_filename=new std::string(ovrfname);
		this->_owner_id ? hr=sound::dev->DuplicateSound(&this->buf, *this->_filename, this->_owner_id): 
		           hr=sound::dev->DuplicateSound(&this->buf, *this->_filename); 

	}
	else
	{
		file_to_load=sdc._name.c_str();
		_owner_id ? hr=sound::dev->DuplicateSound(&this->buf, sdc._name, this->_owner_id): 
				   hr=sound::dev->DuplicateSound(&this->buf, sdc._name); 

	}

	try
	{
		// So, there's no source for get duplication copy,
		// creating original one...
		if(FAILED(hr))
		{
			//
			// not a critical error, continue initializing
			//
			DWORD len=0;
			
			BYTE* ptr1=NULL;
			BYTE* ptr2=NULL;
			DWORD size1=0;
			DWORD size2=0;
			const unsigned char* local_buffer=NULL;
			
			VFile *VFile = DataMgr::Load(file_to_load);
			
			len=VFile->Size();
			len ? local_buffer=VFile->Data() : throw "dssrc::error message::EMPTY_DATA_FILE:probably file does not exist";			
			// check, if it's really a wave-formatted file
			// and if it ain't so cancel initializing
			
			DWORD* dword_lpv=(DWORD*)local_buffer;
			if((*dword_lpv)!=RIFF_FORMATTED_FILE)
				throw "dssrc::error message::NOT_RIFF_FILE"; // it's not RIFF-formatted file, canceling...

			dword_lpv+=2;
			if((*dword_lpv)!=WAVE_FORMATTED_FILE)
				throw "dssrc::error message::NOT_WAVE_FILE"; // it's not WAVE-formatted file, canceling...
			
			int data_size=0;
			int header_size=0;

			BYTE* byte_lpv=(BYTE*)local_buffer;
			
			// Trying to find 'data' symbols in wave header to know size of wav-data
			// Note: there's no need to scan last 3 byte - they can't be part of 'data'
			//       because 'data' is double word
			for(header_size=0;header_size<len-3;header_size++)
			{
				dword_lpv=(DWORD*)byte_lpv;
				if(*dword_lpv==WAVE_DATA_KEYWORD)
				{
					data_size=*++dword_lpv;
					break;
				}
				byte_lpv++;
			}
			if(data_size==0 || data_size>=len)
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("eaxsrc::Initialize", hr, "Reading WAV-header error:Size of the data from WAV is greater than filesize",__FILE__,__LINE__);
#endif
				throw "dssrc::error message::WRONG_WAVE_DATA_SIZE:probably wave header is bad";
			}
			
			header_size+=8;//header_size+=sizeof('data')+sizeof(data_size)
			
			// buffer charasteristics
			WAVEFORMATEX wave;
			
			wave.cbSize=sizeof(WAVEFORMATEX);
			
			wave.nAvgBytesPerSec = *((DWORD *)local_buffer + AVG_BYTES_OFFSET);
			wave.nBlockAlign     = *(( WORD *)local_buffer + BLOCK_ALIGN_OFFSET);
			wave.nChannels       = *(( WORD *)local_buffer + CHANNELS_OFFSET);
			wave.nSamplesPerSec  = *((DWORD *)local_buffer + SAMPLE_PER_SEC_OFFSET);
			wave.wBitsPerSample  = *(( WORD *)local_buffer + BITS_PER_SAMPLE_OFFSET);
			wave.wFormatTag      = *(( WORD *)local_buffer + FORMAT_TAG_OFFSET);//WAVE_FORMAT_PCM;

			DSBUFFERDESC dsbd;
			ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
			dsbd.dwSize        = sizeof(DSBUFFERDESC);
			dsbd.dwFlags       = DSBCAPS_CTRLPAN 
				| DSBCAPS_CTRLVOLUME 
				| DSBCAPS_CTRLFREQUENCY ;
			
			// 3d buffer
			if(buffer3d)
			{
				dsbd.dwFlags|= DSBCAPS_CTRL3D|DSBCAPS_MUTE3DATMAXDISTANCE; 
				// set 3d algoritm
				switch(sdc._quality.value)
				{
				default:
				case HIGHQUALITY:
					dsbd.guid3DAlgorithm = DS3DALG_HRTF_FULL;
					break;
				case MEDQUALITY:
					dsbd.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;
					break;
				case LOWQUALITY:
					dsbd.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
					break;
				}
			}
			
			// set looping flag
			sdc._Flags&LOOPED ? dsbd.dwFlags|=DSBCAPS_STATIC: 
			dsbd.dwFlags|=DSBCAPS_LOCDEFER ;
			
			dsbd.dwBufferBytes = data_size;
			dsbd.lpwfxFormat   = &wave;
			
			if(FAILED(hr=sound::dev->CreateSound(&dsbd, &this->buf)))
				throw "dssrc::error message::FAILED_CREATING_SOUND_SOURCE";
			
			if(this->buf->Lock(0,data_size,(void**)&ptr1,&size1,(void**)&ptr2,&size2,0)!=DS_OK)
				throw "dssrc::error message::FAILED_LOCKING_SOURCE";

			memcpy(ptr1,(BYTE*)local_buffer+header_size,data_size);

			if(this->buf->Unlock(ptr1,data_size,NULL,0)!=DS_OK)
				throw "dssrc::error message::FAILED_UNLOCKING_SOURCE";

			DataMgr::Release(file_to_load);

			// calculaye average playing time
			this->_avg_playing_time = data_size / wave.nAvgBytesPerSec;
		}
	
		if(buffer3d)
		{
			// Get the 3D buffer from the secondary buffer
			if(FAILED(hr=this->buf->QueryInterface(IID_IDirectSound3DBuffer,(VOID**)&this->buf3d)))
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("IDirectSoundBuffer::QueryInterface()", hr, "failed in eaxsrc::Initialize",__FILE__,__LINE__);
#endif
				throw "dssrc::error message::IDirectSoundBuffer::QueryInterface()";
			}
			
			// Get the buffer properties from the secondary buffer
			if(FAILED(hr=this->buf3d->QueryInterface(IID_IKsPropertySet,(LPVOID*)&this->props_set)))
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("IDirectSoundBuffer3D::QueryInterface()", hr, "failed in eaxsrc::Initialize",__FILE__,__LINE__);
#endif
				throw "dssrc::error message::IDirectSoundBuffer3D::QueryInterface()";
			}
			
			if(sdc._min_dist.changed)this->SetMinDistance(sdc._min_dist.value);
			
			if(sdc._max_dist.changed)this->SetMaxDistance(sdc._max_dist.value);
			
			if(sdc._source_rolloff.changed)this->SetSourceRolloff(sdc._source_rolloff.value);
			
			if(sdc._source_outside.changed)this->SetSourceOutside(sdc._source_outside.value);
			
#ifdef DSPL__ENABLE_EXTENDED_SRC_SETTINGS
			if(sdc._Flags&S_AFFECT_DIRHF)       this->SetSourceDirectHF(true);  
			if(sdc._Flags&S_AFFECT_ROOM)        this->SetSourceRoom(true);
			if(sdc._Flags&S_AFFECT_ROOMHF)      this->SetSourceRoomHF(true);
			if(sdc.InsideConeAngle.changed)    this->SetConeInsideAngle(sdc.InsideConeAngle.value);
			if(sdc.OutsideConeAngle.changed)   this->SetConeOutsideAngle(sdc.OutsideConeAngle.value);
			if(sdc.ConeOutsideVolume.changed)  this->SetConeOutsideVolume(sdc.ConeOutsideVolume.value);
			if(sdc.SourceDirect.changed)       this->SetSourceDirect(sdc.SourceDirect.value);
			if(sdc.SourceObstruction.changed)  this->SetSourceObstruction(sdc.SourceObstruction.value);
			if(sdc.SourceObstructionLF.changed)this->SetSourceObstructionLF(sdc.SourceObstructionLF.value);
			if(sdc.SourceOcclusion.changed)    this->SetSourceOcclusion(sdc.SourceOcclusion.value);
			if(sdc.SourceOcclusionLF.changed)  this->SetSourceOcclusionLF(sdc.SourceOcclusionLF.value);
			if(sdc.SourceOcclusionRoom.changed)this->SetSourceOcclusionRoom(sdc.SourceOcclusionRoom.value);

			if(sdc.SourceAbsorption.changed)SetSourceAbsorption(sdc.SourceAbsorption.value);
#endif // DSPL__ENABLE_EXTENDED_SRC_SETTINGS
			if(sdc._source_flags.changed)this->SetSourceFlags(sdc._source_flags.value);
			
			if(sdc._posX.changed)this->SetPositionX(sdc._posX.value);
			
			if(sdc._posY.changed)this->SetPositionY(sdc._posY.value);
			
			if(sdc._posZ.changed)this->SetPositionZ(sdc._posZ.value);
		}
		
		//if(sdc.PanValue.changed)buf->SetPan(sdc.PanValue.value);
		
		int volume=0;
		sound::dev->GetGlobalVolume(&volume);
		if(sdc._vol_value.changed)
		{
			volume+=sdc._vol_value.value;

			if(volume<DSBVOLUME_MIN)
				volume=DSBVOLUME_MIN;

			if(volume>DSBVOLUME_MAX)
				volume=DSBVOLUME_MAX;

			this->buf->SetVolume(volume);
		}
		else
			this->buf->SetVolume(volume);
		
		if(sdc._freq_value.changed)
			this->buf->SetFrequency(sdc._freq_value.value);
		// set _lifetype for the source to deal with Sound Engine (see sddev::Update)
		if(sdc._Flags&PERMANENT)
			this->_lifetype=PERMANENT;
	}
	catch(const char* error)
	{
#ifdef DSPL__DEB_VER_STORE_ERRORS
		logFile[log_name]("Initializing error:: %s...\n",error);
#endif
		DataMgr::Release(file_to_load);
		return false;
	}
	catch(...)
	{
#ifdef DSPL__DEB_VER_STORE_ERRORS
		logFile[log_name]("Unknown exception has happened.\n");
#endif
		return false;
	}

    return true;
}

//
// Initialize sound source as MP3-source
//
int  eaxsrc::Initialize2(const srcdsc& sdc,const char* ovrfname)
{
    this->_mydesc=&sdc;
	HRESULT hr=0;

	WAVEFORMATEX wf;

	wf.wFormatTag=WAVE_FORMAT_PCM;
	wf.nChannels =2;
	wf.nSamplesPerSec =22050;
	wf.nAvgBytesPerSec=22050*2*16/8;
	wf.nBlockAlign    =2*16/8;
	wf.wBitsPerSample =16;
	
	DSBUFFERDESC    dsbd;
	
	memset(&dsbd,0,sizeof(DSBUFFERDESC));
	dsbd.dwSize =sizeof(DSBUFFERDESC);
	dsbd.dwFlags=DSBCAPS_STATIC|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY|DSBCAPS_GETCURRENTPOSITION2;
	bool enable3d=true;
	if(this->_mydesc->_Flags&DISABLE_3D)enable3d=false;
	if(enable3d)
	{
		dsbd.dwFlags|=DSBCAPS_CTRL3D|DSBCAPS_MUTE3DATMAXDISTANCE; 
		dsbd.guid3DAlgorithm=DS3DALG_HRTF_FULL;
	}
	
	dsbd.dwBufferBytes=20*2*2*32*18;
	dsbd.lpwfxFormat=&wf;

	if(FAILED(hr=sound::dev->CreateSound(&dsbd,&this->buf)))return 0;

	if(enable3d)
	{
        // Get the 3D buffer from the secondary buffer
        if(FAILED(hr=buf->QueryInterface(IID_IDirectSound3DBuffer,(VOID**)&this->buf3d)))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirectSoundBuffer::QueryInterface()", hr, "failed in eaxsrc::Initialize2",__FILE__,__LINE__);
#endif
            return 0;
        }
        // Get the 3D buffer parameters
        DS3DBUFFER  buf3dparms;
        buf3dparms.dwSize = sizeof(DS3DBUFFER);
        if(FAILED(hr = this->buf3d->GetAllParameters( &buf3dparms )))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirectSound3DBuffer::GetAllParameters()", hr, "failed in dssrc::Initialize2",__FILE__,__LINE__);
#endif
        }
        // Set new 3D buffer parameters
        //buf3dparms.dwMode        = sdc.Mode;
        buf3dparms.dwMode        = DS3DMODE_NORMAL;
        buf3dparms.flMinDistance = sdc._min_dist.value;
        buf3dparms.flMaxDistance = sdc._max_dist.value;
        if(FAILED(hr=buf3d->SetAllParameters(&buf3dparms, DS3D_IMMEDIATE)))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirectSound3DBuffer::SetAllParameters()", hr, "failed in dssrc::Initialize2",__FILE__,__LINE__);
#endif
        }
	}
    int volume=0;
	sound::dev->GetMusicVolume(&volume);
    if(sdc._vol_value.changed)
    {
        volume+=sdc._vol_value.value;
        if(volume<DSBVOLUME_MIN)volume=DSBVOLUME_MIN;
        if(volume>DSBVOLUME_MAX)volume=DSBVOLUME_MAX;
        this->buf->SetVolume(volume);
    }
    else
        this->buf->SetVolume(volume);
	if(ovrfname)this->_filename=new std::string(ovrfname);
	// set permanent _lifetype for the MP3-source to deal with Sound Engine
	this->_lifetype=PERMANENT;

	return 1;
};

//
// Play source
//
void eaxsrc::Play()
{
    HRESULT hr=0;
	// checking for MP3-type source
	if(this->_mydesc->_Flags&MPEG3FILE)
	{
		// it's MP3-type file
		unsigned result=0;

		const char* fname = NULL;

		if(this->_filename)
		{
			result=iomp3::PlayMusic(*this->_filename,this);
			fname = this->_filename->c_str();
		}
		else
		{
			result=iomp3::PlayMusic(this->_mydesc->_name,this);
			fname = this->_mydesc->_name.c_str();
		}

		if(result!=IOMP3_SUCCESS)	
		{
			// pipe is busy by someone,take place in queue and wait your turn
			if(result==IOMP3_BUSY) 
			{
				static std::string waiter;
				static unsigned waiter_id = 0; 

				if(sdsrc::in_queue >= 1)
					sound::dev->ThrashSource(waiter_id);
				else
					sdsrc::in_queue++;

				SRCFUNC func=NULL;
				func=&sdsrc::SmoothPlay;
				this->SetSourceExec(func);

				waiter_id = this->_sid;
			}
		}
	}
	else
	{
		if(FAILED(hr=RestoreBuffers()))
		{
#ifdef DSPL__DEB_VER_STORE_ERRORS
			err.PostError("RestoreBuffers()", hr, "failed in eaxsrc::Play()",__FILE__,__LINE__);
#endif
		}
		// Play buffer 
		DWORD flags=0;
		DWORD priority=0;

		if(this->_mydesc->_Flags&LOOPED)flags=DSBPLAY_LOOPING;

		// If PCI-card then use Voice Allocation and Voice Manager
		if(sound::dev->TestDeviceFlags(PCI_CARD))
		{
			if(!(this->_mydesc->_Flags&LOWPRIORITY))this->_mydesc->_Flags&HIGHPRIORITY ? priority=0xffffffff : priority=0xefffffff;
			if(this->_mydesc->_Flags&STATIC)flags|=DSBPLAY_TERMINATEBY_PRIORITY|DSBPLAY_LOCHARDWARE;
		}

		// reset to the beginning of buffer
		// reason: if buffer play cursor reached end of the buffer 
		//         setting cursor back to the beginning is necessary,
		//         first of all it's got done for PERMANENT buffer that were played and now got to be played again
		buf->SetCurrentPosition(0);

		if(FAILED(hr=this->buf->Play(0,priority,flags)))
		{
#ifdef DSPL__DEB_VER_STORE_ERRORS
			err.PostError("IDirectSoundBuffer::Play()", hr, "failed in dssrc::Play()",__FILE__,__LINE__);
#endif
		}
	}
}

//
// Stop source
//
void eaxsrc::Stop()const
{
    HRESULT hr = 0;
	if(this->_mydesc->_Flags&MPEG3FILE)
	{
		iomp3::RemoveMusic();
	}
	else
	{
		if(FAILED(hr=this->buf->Stop()))
		{
#ifdef DSPL__DEB_VER_STORE_ERRORS
			err.PostError("IDirectSoundBuffer::Stop()", hr, "failed in eaxsrc::Stop()",__FILE__,__LINE__);
#endif
			return;
		}
		if(FAILED(hr=this->buf->SetCurrentPosition(0L)))
		{
#ifdef DSPL__DEB_VER_STORE_ERRORS
			err.PostError("IDirectSoundBuffer::SetCurrentPosition()", hr, "failed in eaxsrc::Stop()",__FILE__,__LINE__);
#endif
		}
	}
}

//
// Pause source
//
void eaxsrc::Pause()
{
    HRESULT hr = 0;
	// to prevent sound device deleteing paused source from Common-List-of Sources,
	// (non-playing and BRIEF-type sources are to be deleted)
	// temporary(!) set _lifetype of source to PERMANENT
	this->_lifetype=PERMANENT;

	if(this->_mydesc->_Flags&MPEG3FILE)
	{
		iomp3::PauseMusic();
	}
	else
	{
		if(FAILED(hr=buf->Stop()))
		{
#ifdef DSPL__DEB_VER_STORE_ERRORS
			err.PostError("IDirectSoundBuffer::Stop()", hr, "failed in eaxsrc::Stop()",__FILE__,__LINE__);
#endif
			return;
		}
	}
}

//
// Resume source
//
void eaxsrc::Resume()
{
    HRESULT hr = 0;
	// if source was paused, it temporary(!) got PERMANENT-_lifetype,
	// so, if original description tells,that source is BRIEF-type, then set it back
	if(this->_mydesc->_Flags&BRIEF)this->_lifetype=BRIEF;

	if(this->_mydesc->_Flags&MPEG3FILE)
	{
		iomp3::ResumeMusic();
	}
	else
	{
		if(FAILED(hr=this->RestoreBuffers()))
		{
#ifdef DSPL__DEB_VER_STORE_ERRORS
			err.PostError("RestoreBuffers()", hr, "failed in eaxsrc::Play()",__FILE__,__LINE__);
#endif
		}
	    // Resume buffer playing
		this->Play();
	}
}

//
// Reset source
//
void eaxsrc::Reset()const
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    HRESULT hr=0;
    if(FAILED(hr = this->buf->SetCurrentPosition(0)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("eaxsrc::Reset()", hr, "failed setting position to 0",__FILE__,__LINE__);
#endif
    }
#else
    this->buf->SetCurrentPosition(0);
#endif
}

//
// Adjust source volume
//
void eaxsrc::AdjustVolume(int new_volume)const
{
    HRESULT hr = 0;

    if(new_volume < DSBVOLUME_MIN)
		new_volume = DSBVOLUME_MIN;

    if(new_volume > DSBVOLUME_MAX)
		new_volume = DSBVOLUME_MAX;

#ifdef DSPL__DEB_VER_STORE_ERRORS
        if(FAILED(hr = this->buf->SetVolume(new_volume)))
        {
            err.PostError("IDirectSoundBuffer::SetVolume()", hr, "failed in eaxsrc::Volume()",__FILE__,__LINE__);
            return;
        }
#else
        this->buf->SetVolume(new_volume);
#endif
}

//
// Set frequency for source
//
void eaxsrc::Freq(unsigned int value)const
{
    HRESULT hr = 0;

    if(FAILED(hr = this->buf->SetFrequency(value)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundBuffer::SetFrequency()", hr, "failed in eaxsrc::Freq()",__FILE__,__LINE__);
#endif
        return;
    }
}

//
// Set panning value
//
void eaxsrc::Pan(int value)const
{
    HRESULT hr = 0;

    if(FAILED(hr = this->buf->SetPan(value)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundBuffer::SetPan()", hr, "failed in eaxsrc::Pan()",__FILE__,__LINE__);
#endif
        return;
    }
}  
        
//
// Restore source if it's lost
//
HRESULT eaxsrc::RestoreBuffers()const
{
    HRESULT hr = 0;

    if(this->buf==NULL)return S_OK;

    DWORD dwStatus;
    if(FAILED(hr=this->buf->GetStatus(&dwStatus)))return hr;

    if(dwStatus&DSBSTATUS_BUFFERLOST)
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control.
        do 
        {
            hr=this->buf->Restore();
            if(hr==DSERR_BUFFERLOST)Sleep(10);
        }
        while(hr=this->buf->Restore());

        if(FAILED(hr=this->FillBuffer()))return hr;
    }

    return S_OK;
}

//
// Fill buffer with data
//
HRESULT eaxsrc::FillBuffer()const
{
	BYTE* ptr1=NULL;
	BYTE* ptr2=NULL;
	DWORD size1=0;
	DWORD size2=0;
	HRESULT hr=0;
	const unsigned char* local_buffer=NULL;
	const std::string* filename=NULL;
	this->_filename ? filename=this->_filename : filename=&this->_mydesc->_name;
	int len=0;
	
	VFile *VFile = DataMgr::Load(filename->c_str());
	
	try
	{
		len=VFile->Size();
		len ? local_buffer=VFile->Data() : throw "empty file or file does not exist";
		
		int data_size=0;
		int header_size=0;
		BYTE* byte_lpv=(BYTE*)local_buffer;
		DWORD* dword_lpv=NULL;
		
		for(header_size=0;header_size<len-3;header_size++)
		{
			dword_lpv=(DWORD*)byte_lpv;
			if(*dword_lpv==WAVE_DATA_KEYWORD)
			{
				data_size=*++dword_lpv;
				break;
			}
			byte_lpv++;
		}
		if(data_size==0 || data_size>=len)throw "WAVE_DATA_SIZE_ERROR";
		
		header_size+=8;//header_size+=sizeof('data')+sizeof(data_size)
		
		if(hr=this->buf->Lock(0,data_size,(void**)&ptr1,&size1,(void**)&ptr2,&size2,0)!=DS_OK)throw "failed locking sound buffer";

		memcpy(ptr1,(BYTE*)local_buffer+header_size,data_size);

		if(hr=this->buf->Unlock(ptr1,data_size,NULL,0)!=DS_OK)throw "failed unlocking sound buffer";
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	catch(const char* error)
	{
		err.PostError("eaxsrc::FillBuffer", -1, error,__FILE__,__LINE__);
		DataMgr::Release(filename->c_str());
		return 0;
	}
#endif
	catch(...)
	{
		DataMgr::Release(filename->c_str());
		return 0;
	}

    return DS_OK;
}

//
// Detect if source is playing
//
bool eaxsrc::IsPlaying()const
{
    // if buffer is playing
    if(buf)
    {  
        DWORD dwStatus = 0;
        buf->GetStatus( &dwStatus );
#ifdef DSPL__DEB_VER_STORE_ERRORS
        if(dwStatus & DSBSTATUS_LOCHARDWARE)logFile[log_name]("Playing in hardware...");
        if(dwStatus & DSBSTATUS_LOCSOFTWARE)logFile[log_name]("Playing in software...");
        if(dwStatus & DSBSTATUS_LOOPING)logFile[log_name]("Playing loop...");
        return(( dwStatus & DSBSTATUS_PLAYING) != 0 );
#else
        return(( dwStatus & DSBSTATUS_PLAYING) != 0 );
#endif
    }
    else
        return false;
}

//
// Set source position
//
void eaxsrc::SetPos(const point3& pos)
{
    D3DVALUE x=pos.x,
             y=pos.y,
             z=pos.z;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    HRESULT hr=0;
    if(this->buf3d&&FAILED(this->buf3d->SetPosition(x,y,z,DS3D_DEFERRED)))
        err.PostError("IDirectSound3DBuffer::SetPosition()", hr, "failed in eaxsrc::SetPos()",__FILE__,__LINE__);
#else
    if(this->buf3d)this->buf3d->SetPosition(x, y, z, DS3D_DEFERRED);
#endif
}
 
//
// Set source velocity
//
void eaxsrc::SetVel(const point3& vel)
{
    D3DVALUE x=vel.x,
             y=vel.y,
             z=vel.z;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    HRESULT hr=0;
    if(this->buf3d&&FAILED(this->buf3d->SetVelocity(x,y,z,DS3D_DEFERRED)))
        err.PostError("IDirectSound3DBuffer::SetVelocity()", hr, "failed in eaxsrc::SetPos()",__FILE__,__LINE__);
#else
    if(this->buf3d)this->buf3d->SetVelocity(x,y,z,DS3D_DEFERRED);
#endif
}

//
// Slow volume decay
//
void eaxsrc::SmoothVolumeDecay()
{
    LONG curr_volume=0;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    //logFile[log_name]("Smoothing down...\n");
    HRESULT hr=0;
    if(FAILED(hr = this->buf->GetVolume(&curr_volume)))
        err.PostError("eaxsrc::SmoothVolumeDecay()", hr, "failed getting volume",__FILE__,__LINE__);
#else
    this->buf->GetVolume(&curr_volume);
#endif
	//curr_volume-=VOLUME_TUNING;
	curr_volume--;
	curr_volume*=2.0f;
	//logFile[log_name]("Smoothing down...[volume=%d]\n",curr_volume);
    if(curr_volume<=VOLUME_LOW_LIMIT)
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
		this->_filename ?
			logFile[log_name]("[%s]has been smoothed down...\n",this->_filename->c_str()) :
			logFile[log_name]("[%s]has been smoothed down...\n",this->_mydesc->_name.c_str());
#endif
        this->_exec_source=NULL;
		if(this->_mydesc&&(_mydesc->_Flags&MPEG3FILE))
			iomp3::RemoveMusic();
		else
			buf->Stop();
        return;
    }

#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(hr=this->buf->SetVolume(curr_volume)))
        err.PostError("eaxsrc::SmoothVolumeDecay()",hr,"failed setting volume",__FILE__,__LINE__);
#else
    this->buf->SetVolume(curr_volume);
#endif
}

//
// Slow volume expand
//
void eaxsrc::SmoothVolumeExpand()
{
    LONG curr_volume=0;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    //logFile[log_name]("Smoothing up...\n");
    HRESULT hr=0;
    if(FAILED(hr=this->buf->GetVolume(&curr_volume)))
        err.PostError("eaxsrc::SmoothVolumeExpand()", hr, "failed getting volume",__FILE__,__LINE__);
#else
    this->buf->GetVolume(&curr_volume);
#endif
    //curr_volume+=VOLUME_TUNING;
	curr_volume/=2.f;
	//logFile[log_name]("Smoothing up...[volume=%d]\n",curr_volume);
    if(curr_volume>=Volume)
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
		this->_filename ?
			logFile[log_name]("[%s,id=%d]has been smoothed up...\n",this->_filename->c_str(),this->get_id()) :
			logFile[log_name]("[%s,id=%d]has been smoothed up...\n",this->_mydesc->_name.c_str(),this->get_id());
#endif
        this->_exec_source=NULL;
        return;
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(hr=this->buf->SetVolume(curr_volume)))
        err.PostError("eaxsrc::SmoothVolumeExpand()", hr, "failed setting volume",__FILE__,__LINE__);
#else
    this->buf->SetVolume(curr_volume);
#endif

}

//
// Smooth playing
//
void eaxsrc::SmoothPlay()
{
	// if MP3-type source, first attempt to get MP3-pipe to play itself
	// if successed, then start playing and growing up it's volume
	if(this->_mydesc->_Flags&MPEG3FILE)
	{
		const std::string* filename=NULL;

		this->_filename ? filename = this->_filename : filename = &this->_mydesc->_name;

		unsigned int result=iomp3::PlayMusic(*filename,this);

#ifdef DSPL__DEB_VER_STORE_ERRORS
		logFile[log_name]("Source named[%s] awaits to be played...\n",filename->c_str());
#endif
		if(result)
		{
			// if MP3-pipe is busy - we have to wait until it's not, so we can't expand volume
			if(result==IOMP3_BUSY)
				return;

			if(sdsrc::in_queue > 0)
				sdsrc::in_queue--;

#ifdef DSPL__DEB_VER_STORE_ERRORS
			logFile[log_name]("Source named[%s] has got MPEG3 decoder resource and will be played...\n",filename->c_str());
#endif
		}
		else
		{
			// all attempts to start smooth playing file failed
			// delete source from global map
			this->_lifetype=BRIEF;
#ifdef DSPL__DEB_VER_STORE_ERRORS
			err.PostError("eaxsrc::SmoothPlay()",-1,"failed waiting for MPEG3 pipe",__FILE__,__LINE__);
#endif
		}
	}
	// Expand sound volume
	SRCFUNC func=NULL;
	func=&sdsrc::SmoothVolumeExpand;
	this->SetSourceExec(func);
}

/***************************************************************************

                                 END OF FILE

***************************************************************************/
