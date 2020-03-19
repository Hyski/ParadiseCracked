/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: DirectSound arranged sound library
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/

#include "..\precomp.h"

#include "sndcompiler.h"
#include "mmsystem.h"
#include "sound.h"
#include <dsound.h>
#include "dsdev.h"
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

                            dsdev realization

***************************************************************************
***************************************************************************/

dsdev::dsdev() : sddev(),
				 _device(NULL),
				 _camera(NULL),
				 _global_volume(-1)
{         
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    struct tm *newtime;        
    time_t long_time;
    time(&long_time);              // Get time as long integer. 
    newtime=localtime(&long_time); // Convert to local time.

    logFile[log_name]("Sound log is opened now...\t%.19s \nDirectSound support is connected to sound engine.\n", asctime(newtime));
#endif
}

dsdev::~dsdev()
{
    // Disconnecting MP3-pipe
    iomp3::DisconnectMP3Pipe();
#ifdef DSPL__DEB_VER_STORE_ERRORS
    logFile[log_name]("MPEG-3 Pipe has been disconnected...\n");
#endif

	// Clean-Up the list of emitters and the list of sources
	this->Close();

    // release previously allocated objects
    if(this->_camera)
	{ 
		this->_camera->Release(); 
		this->_camera = NULL; 
	};

    if(this->_device)
	{ 
		this->_device->Release(); 
		this->_device = NULL; 
	};

#ifdef DSPL__DEB_VER_SHOW_COMMENT
    err.GiveStats();
    struct tm *newtime;        
    time_t long_time;
    time(&long_time);              // Get time as long integer.
    newtime=localtime(&long_time); // Convert to local time.

    logFile[log_name]("Closing sound log at [%.19s]... Done.\n", asctime(newtime));
#endif
}

//
// Create sound source with owner id(smart function: first will try to duplicate if original exists)
//
HRESULT dsdev::CreateSound(void* dsbd,void* _buf,const std::string& fname,const unsigned int& _owner_id,bool& duplicated) const
{
    REGISTERED_EMITTERS::const_iterator ci;
    sdsrc* src=NULL;

    for(ci=this->_emitmap.begin();ci!=this->_emitmap.end();++ci)
    {
        // trying to get duplication copy
        if((ci->first!=_owner_id)&&(src=ci->second->browse(fname)))
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
            logFile[log_name]("**COMMENT** Duplicating: found source for duplicating...\n");
#endif
            duplicated=true;
            return this->_device->DuplicateSoundBuffer(reinterpret_cast<LPDIRECTSOUNDBUFFER>(src->GetBuf()),reinterpret_cast<LPDIRECTSOUNDBUFFER*>(_buf));
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
			if(SUCCEEDED(this->_device->GetCaps(&caps)))
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

    return this->_device->CreateSoundBuffer(buf_desc,reinterpret_cast<LPDIRECTSOUNDBUFFER*>(_buf),NULL);
}

//
// Create sound source (smart function: first will try to duplicate if original exists)
//
HRESULT dsdev::CreateSound(void* dsbd,void* _buf, const std::string& fname,bool& duplicated) const
{
    OWNEDBY_DEVICE_SOURCES::const_iterator ci=this->_devsrcmap.find(fname);

    if(ci!=this->_devsrcmap.end())
    {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        logFile[log_name]("**COMMENT** Duplicating: found source for duplicating...\n");
#endif
        duplicated=true;
        return this->_device->DuplicateSoundBuffer(reinterpret_cast<LPDIRECTSOUNDBUFFER>(ci->second->GetBuf()),reinterpret_cast<LPDIRECTSOUNDBUFFER*>(_buf));
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
			if(SUCCEEDED(this->_device->GetCaps(&caps)))
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

    return this->_device->CreateSoundBuffer(buf_desc,reinterpret_cast<LPDIRECTSOUNDBUFFER*>(_buf), NULL);
}

//
// (Smart function)
// Creating sound buffer for source with owner, so the map of emitters will be browsed
// for possible source duplication 
//
HRESULT dsdev::DuplicateSound(void* _buf, const std::string& fname,const unsigned int& _owner_id) const
{
    REGISTERED_EMITTERS::const_iterator ci;
    sdsrc* src=NULL;

    for(ci=_emitmap.begin();ci!=_emitmap.end();++ci)
    {
        // trying to get duplication copy
        if((ci->first!=_owner_id)&&(src=ci->second->browse(fname)))
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
            logFile[log_name]("**COMMENT** Duplicating with id: found source for duplicating...\n");
#endif
            return this->_device->DuplicateSoundBuffer(reinterpret_cast<LPDIRECTSOUNDBUFFER>(src->GetBuf()),reinterpret_cast<LPDIRECTSOUNDBUFFER*>(_buf));
        }
    }
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("**COMMENT** Duplicating with id: no source's been found duplicating... try to create original.\n");
#endif

    return DSERR_INVALIDCALL;
}

//
// (Smart function)
// Creating sound buffer for source without owner, so the map of _device sources will be browsed
// for possible source duplication
//
HRESULT dsdev::DuplicateSound(void* _buf, const std::string& fname) const
{
    OWNEDBY_DEVICE_SOURCES::const_iterator ci=this->_devsrcmap.find(fname);

    if(ci!=this->_devsrcmap.end())
    {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        logFile[log_name]("**COMMENT** Duplicating: found source for duplicating...\n");
#endif
        return this->_device->DuplicateSoundBuffer(reinterpret_cast<LPDIRECTSOUNDBUFFER>(ci->second->GetBuf()),reinterpret_cast<LPDIRECTSOUNDBUFFER*>(_buf));
    }
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("**COMMENT** Duplicating: no source's been found duplicating... try to create original.\n");
#endif

    return DSERR_INVALIDCALL;
}

//
// Raw creating sound source
//
HRESULT dsdev::CreateSound(void* dsbd,void* _buf) const
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
			if(SUCCEEDED(this->_device->GetCaps(&caps)))
			{
				if(caps.dwFreeHwMemBytes>buf_desc->dwBufferBytes)
					buf_desc->dwFlags|=DSBCAPS_STATIC;
#ifdef DSPL__DEB_VER_SHOW_COMMENT
				logFile[log_name]("Free HW memory:%d, size of buffer:%d\n",caps.dwFreeHwMemBytes,buf_desc->dwBufferBytes);
#endif
			}
#ifdef  DSPL__DEB_VER_STORE_ERRORS
			else
				err.PostError("IDirectSoundbuffer::GetCaps()",-1,"failed trying to get free memory info for ISA card memory management",__FILE__,__LINE__);
#endif
		}
	}
	LPDIRECTSOUNDBUFFER* __dummy=reinterpret_cast<LPDIRECTSOUNDBUFFER*>(_buf);

	return this->_device->CreateSoundBuffer(buf_desc,__dummy,NULL);
}

//
// Raw duplicating sound source
//
HRESULT dsdev::DuplicateSound(void* srcbuf,void* destbuf) const
{
    return this->_device->DuplicateSoundBuffer(reinterpret_cast<LPDIRECTSOUNDBUFFER>(srcbuf),reinterpret_cast<LPDIRECTSOUNDBUFFER*>(destbuf));
}

//
// Get dssrc exempliar
//
inline sound::sdsrc* dsdev::PurchaseSource()const{ return new dssrc; };

//
// Get dssrc exempliar(sign.2)
//
inline void dsdev::PurchaseSource(sound::sdsrc** source)const{ *source=new dssrc; };

//
// Get dssrc exempliar with id 
//
inline void dsdev::PurchaseSource(sound::sdsrc** source,const unsigned int& id)const{ *source=new dssrc(id); };

//
// Get dssrc exempliar with id(sign.2)
//
inline sound::sdsrc* dsdev::PurchaseSource(const unsigned int& id) const 
{ 
    return new dssrc(id); 
};

//
// Initialize sound _device
//
bool dsdev::Initialize(HWND hwnd)
{
    HRESULT             hr=0;
    DSBUFFERDESC        dsbdesc;
    LPDIRECTSOUNDBUFFER primbuf;
	SndCompiler			compiler;

    if(FAILED(hr=DirectSoundEnumerate((LPDSENUMCALLBACK)EnumerateSoundDevices,NULL)))
	{
#ifdef  DSPL__DEB_VER_STORE_ERRORS
        err.PostError("DirectSoundEnumerate()", hr, "failed in sddev::Initialize",__FILE__,__LINE__);
        logFile[log_name]("\n\nCRITICAL ERROR! ...DirectSound enumeration has failed. You will not be able to operate with sound...\n\n");
#endif
        return false; // smthg is wrong, you've got to handle
	}
 
    if(FAILED(hr=DirectSoundCreate(NULL,&this->_device,NULL)))
    {
#ifdef  DSPL__DEB_VER_STORE_ERRORS
        err.PostError("DirectSoundCreate()", hr, "failed in sddev::Initialize",__FILE__,__LINE__);
        logFile[log_name]("\n\nCRITICAL ERROR! ...Initialization of DirectSound object has failed. You will not be able to operate with sound...\n\n");
#endif
        return false; // smthg is wrong, you've got to handle
    }

    if(FAILED(hr=this->_device->SetCooperativeLevel(hwnd,DSSCL_PRIORITY)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("SetCooperativeLevel()", hr, "failed in sddev::Initialize",__FILE__,__LINE__);
#endif

        return false; // smthg is wrong, you've got to handle
    }

    // build the listener

    // Obtain primary buffer, asking it for 3D control
    ZeroMemory(&dsbdesc,sizeof(DSBUFFERDESC));
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_CTRL3D|DSBCAPS_PRIMARYBUFFER;
    if(FAILED(hr=this->_device->CreateSoundBuffer(&dsbdesc,&primbuf,NULL)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("Createsoundbuffer()", hr, "failed in sddev::Initialize",__FILE__,__LINE__);
#endif
        return false;
    }

    if(FAILED(hr=primbuf->QueryInterface(IID_IDirectSound3DListener,(VOID**)&this->_camera)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundbuffer::QueryInterface()", hr, "failed in sddev::Initialize",__FILE__,__LINE__);
#endif
        return false;
    }

    // Set primary buffer format to 22kHz and 16-bit output.
    WAVEFORMATEX wfx;
    ZeroMemory(&wfx,sizeof(WAVEFORMATEX)); 
    wfx.wFormatTag      = WAVE_FORMAT_PCM; 
    wfx.nChannels       = 2; 
    wfx.nSamplesPerSec  = 22050; 
    wfx.wBitsPerSample  = 16; 
    wfx.nBlockAlign     = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    if(FAILED(hr=primbuf->SetFormat(&wfx)))
	{
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundbuffer::SetFormat()", hr, "failed in sddev::Initialize",__FILE__,__LINE__);
#endif
        return false;
	}

    if(primbuf)primbuf->Release();

	DSCAPS caps;
    caps.dwSize=sizeof(DSCAPS);
    if(SUCCEEDED(hr=this->_device->GetCaps(&caps)))
	{
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        logFile[log_name]("Total sound card memory available[%dKB], free[%dKB], static buffers[%d], all buffers[%d]\n",caps.dwTotalHwMemBytes>>10,caps.dwFreeHwMemBytes>>10,caps.dwFreeHw3DStaticBuffers,caps.dwFreeHw3DAllBuffers);
#endif
		// detect if ISA card
		if(caps.dwMaxHwMixingStreamingBuffers!=0)
		{
#ifdef DSPL__DEB_VER_SHOW_COMMENT
			logFile[log_name]("Proceeding on PCI card.\n");
#endif
			this->_device_flags|=PCI_CARD;
		}
		else
		{
			if(caps.dwMaxHwMixingStaticBuffers!=0)
			{
#ifdef DSPL__DEB_VER_SHOW_COMMENT
				logFile[log_name]("Proceeding on ISA card.\n");
#endif
				this->_device_flags|=ISA_CARD;
			}
		}
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("IDirectSound::GetCaps()",hr,"Failed getting _device capabilities",__FILE__,__LINE__);
#endif

    // Get listener parameters
	DS3DLISTENER camera_props;
    camera_props.dwSize=sizeof(DS3DLISTENER);
    this->_camera->GetAllParameters(&camera_props);

    // if initializations are successfull, then we'll
    // read the script for sound descriptions
    // note: compile function call is supposed to be between
    //       listener->GetParms and listener->SetParms
    //       cause script contains settings for listener
    //       and it will be overwritten by listener->GetParms
    //       if you call .compile before GetParms and will not
    //       take any effect if call .compile after SetParms
    compiler.Compile(script_filename);

	if(ListenerDefs._doppler_factor.changed)camera_props.flDopplerFactor=ListenerDefs._doppler_factor.value;
	if(ListenerDefs._rolloff.changed)      camera_props.flRolloffFactor=ListenerDefs._rolloff.value;

    this->_camera->SetAllParameters(&camera_props,DS3D_IMMEDIATE);

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

    return true;
}

//
// Set properties for _device
//
void dsdev::SetDeviceProperty(lprops prop,void* valaddr)
{
	if(valaddr)
	{
		switch(prop)
		{
		case PROP_ROLLOFF:
			this->_camera->SetRolloffFactor(*reinterpret_cast<float*>(valaddr),DS3D_IMMEDIATE);
			break;
		case PROP_DOPPLER_FACTOR:
			this->_camera->SetDopplerFactor(*reinterpret_cast<float*>(valaddr),DS3D_IMMEDIATE);
			break;
		default:
		// these properties currerntly are not supported by Direct Sound
		case PROP_ROOM:
		case PROP_ROOM_HF:
		case PROP_REFLECTIONS:
		case PROP_ENVIRONMENT:
		case PROP_PROP_FLAGS:
		case PROP_REVERB_DELAY:
		case PROP_REFLECTIONS_DELAY:
		case PROP_ROOM_ROLLOFF:
		case PROP_DECAY_TIME:
		case PROP_DECAY_HF_RATIO:
		case PROP_ENVIRONMENT_SIZE:
		case PROP_ENVIRONMENT_DIFFUSION:
		case PROP_AIR_ABSORPTION:
		case PROP__FLAGS:
			this->_device_flags|=*reinterpret_cast<DWORD*>(valaddr);
			break;
		}
	}
}

//
// Playing sound (effect of slow volume growing)
//
void dsdev::SmoothPlay(const tray& tr, const point3* new_pos)
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

				if(new_pos)
					source->SetPos(*new_pos);

				source->Play();

				typedef OWNEDBY_DEVICE_SOURCES::value_type Pairs;
				Pairs new_pair(filename,source);

				this->_devsrcmap.insert(new_pair);
			}
			else
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("dssrc::Initialize()", -1, "failed in dsdev::SmoothPlay()",__FILE__,__LINE__);
#endif
				this->ThrashSource(source);
			}
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("dsdev::SmoothPlay()", -1, "description was not found:" + *tr._cell1,__FILE__,__LINE__);
#endif
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("dsdev::SmoothPlay()", -1, "description was not found(NULL)",__FILE__,__LINE__);
#endif
}

//
// Update sound _device (as listener changed its position)
//
void dsdev::Update(const point3& pos,
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

	//  updating _camera
    D3DVALUE px=pos.x,py=pos.y,pz=pos.z,
             vx=vel.x,vy=vel.y,vz=vel.z,
             fx=front.x,fy=front.y,fz=front.z,
             tx=up.x,ty=up.y,tz=up.z;

    // setting new parameteres
    this->_camera->SetPosition   (px, py, pz, DS3D_DEFERRED);
    this->_camera->SetVelocity   (vx, vy, vz, DS3D_DEFERRED);
    this->_camera->SetOrientation(fx, fy, fz, tx, ty, tz, DS3D_DEFERRED);
//
//  updating sound emitters (such as moving objects e.t.c.)
//
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
            err.PostError("dsdev::Update()",-1,"attempt to use missed emitter in map",__FILE__,__LINE__);
#endif
    }
//
//  updating sound sources (static points)
//
    OWNEDBY_DEVICE_SOURCES::iterator cis;
    for(cis=this->_devsrcmap.begin();cis!=this->_devsrcmap.end();++cis)
    {
		// if source is playing or it's _lifetype is PERMANENT
		// then let it go, else delete from the memory

		bool playing=cis->second->IsPlaying();
        if(playing||(cis->second->GetLifeType()==PERMANENT))
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT

			int sid = cis->second->get_id();

			if(playing)
				logFile[log_name]("File [%s]::[playing]\n",cis->first);
			else
				logFile[log_name]("File [%s]::[permanent]\n",cis->first);
#endif

            cis->second->Execute();
        }
        else
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
            logFile[log_name]("File [%s]::[to be destructed]\n",cis->first);
#endif
            this->ThrashSource(cis->second);
            this->_devsrcmap.erase(cis);
        }
    }

    // applying settings
    this->_camera->CommitDeferredSettings();

#ifdef DSPL__DEBUG_INFO
	static DSCAPS caps;
	caps.dwSize=sizeof(DSCAPS);
	if(SUCCEEDED(this->_device->GetCaps(&caps)))
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
// Update sound _device (as listener is still there)
//
void dsdev::Update() 
{
	// first, be sure that not too frequent calls
	static float  _prev_game_time = 0.f;
	float	       _cur_game_time = Timer::GetSeconds();
	float _min_game_time_interval = 1.f / Options::GetFloat("system.sound.update_freq");

	if((_cur_game_time - _prev_game_time) < _min_game_time_interval) return;
	_prev_game_time = _cur_game_time;

//  updating sound emitters (such as moving objects e.t.c.)
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
            err.PostError("dsdev::Update()",-1,"attempt to use missed emitter in map",__FILE__,__LINE__);
#endif
    }

//  updating sound sources (static points)
    OWNEDBY_DEVICE_SOURCES::iterator cis;
    for(cis=this->_devsrcmap.begin();cis!=this->_devsrcmap.end();++cis)
    {
		bool playing=cis->second->IsPlaying();
        if(playing||(cis->second->GetLifeType()==PERMANENT))
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
			int sid = cis->second->get_id();

			if(playing)
				logFile[log_name]("File [%s]::[playing]\n",cis->first);
			else
				logFile[log_name]("File [%s]::[permanent]\n",cis->first);
#endif
            cis->second->Execute();
        }
        else
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
            logFile[log_name]("File [%s]::[to be destructed]\n",cis->first);
#endif
            this->ThrashSource(cis->second);
            this->_devsrcmap.erase(cis);
        }
    }

    // applying settings
    this->_camera->CommitDeferredSettings();

#ifdef DSPL__DEBUG_INFO
	static DSCAPS caps;
	caps.dwSize=sizeof(DSCAPS);
	if(SUCCEEDED(this->_device->GetCaps(&caps)))
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
void dsdev::AdjustGlobalVolume(int value)
{
    // setting global volume for all sources except
    // remember last value
    this->_global_volume+=value;

    if(this->_global_volume<DSBVOLUME_MIN)
		this->_global_volume=DSBVOLUME_MIN;

    if(this->_global_volume>DSBVOLUME_MAX)
		this->_global_volume=DSBVOLUME_MAX;

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
void dsdev::AdjustMusicVolume(int value)
{
    iomp3::AdjustVolumeMusic(value);
}

//
// get _camera position 
//
void dsdev::GetPos(point3& pos)const
{
    D3DVECTOR campos;
    HRESULT hr=0;
    if(SUCCEEDED(hr=this->_camera->GetPosition(&campos)))
    {
        pos.x=campos.x; pos.y=campos.y; pos.z=campos.z;
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("GetVelocity()",hr,"failed in dsdev::GetVel",__FILE__,__LINE__);
#endif
}

//
// get _camera velocity 
//
void dsdev::GetVel(point3& vel)const
{
    D3DVECTOR camvel;
    HRESULT hr=0;
    if(SUCCEEDED(hr=this->_camera->GetVelocity(&camvel)))
    {
        vel.x=camvel.x; vel.y=camvel.y; vel.z=camvel.z;
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("GetVelocity()",hr,"failed in dsdev::GetVel",__FILE__,__LINE__);
#endif
}

//
// set _camera position 
//
void dsdev::SetPos(const point3& pos)
{
    D3DVALUE x=pos.x,
             y=pos.y,
             z=pos.z;

    this->_camera->SetPosition(x, y, z, DS3D_DEFERRED);
}

//
// set _camera velocity 
//
void dsdev::SetVel(const point3& vel)
{
    D3DVALUE x=vel.x,
             y=vel.y,
             z=vel.z;

    this->_camera->SetVelocity(x, y, z, DS3D_DEFERRED);
}

//
// set _camera orientation 
//
void dsdev::BeOriented(const point3& front, const point3& top)
{
    D3DVALUE fx=front.x,
             fy=front.y,
             fz=front.z,
             tx=top.x,
             ty=top.y,
             tz=top.z;

    this->_camera->SetOrientation(fx, fy, fz, tx, ty, tz, DS3D_DEFERRED);
}

//
// Set _camera position
//
void dsdev::SetXYZ(const point3& pos,const point3& vel,const point3& front,const point3& top)
{
    D3DVALUE px=pos.x,py=pos.y,pz=pos.z,
             vx=vel.x,vy=vel.y,vz=vel.z,
             fx=front.x,fy=front.y,fz=front.z,
             tx=top.x  ,ty=top.y  ,tz=top.z;

    // setting new parameteres
    this->_camera->SetPosition   (px, py, pz, DS3D_DEFERRED);
    this->_camera->SetVelocity   (vx, vy, vz, DS3D_DEFERRED);
    this->_camera->SetOrientation(fx, fy, fz, tx, ty, tz, DS3D_DEFERRED);
    this->_camera->CommitDeferredSettings();
}

/***************************************************************************
****************************************************************************

                            dssrc realization

***************************************************************************
***************************************************************************/

dssrc::dssrc() : sdsrc(),
			    _buf(NULL),
				_buf3d(NULL),
				_volume(-1){}

dssrc::dssrc(unsigned int id) : sdsrc(id),
							   _buf(NULL),
							   _buf3d(NULL),
							   _volume(-1){}

dssrc::dssrc(const dssrc& sndbuf) : _buf(NULL),
									_buf3d(NULL)
{
}

dssrc::~dssrc()
{
    if(this->_buf)
	{ 
		this->_buf->Release();
		this->_buf=NULL;   
	};

    if(this->_buf3d)
	{ 
		this->_buf3d->Release();
		this->_buf3d=NULL; 
	};
}

//
// Initialize sound source
//
bool dssrc::Initialize(const srcdsc& sdc,const char* ovrfname)
{
    HRESULT hr=0;

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
		this->_owner_id ? hr=sound::dev->DuplicateSound(&this->_buf, *this->_filename, this->_owner_id): 
						 hr=sound::dev->DuplicateSound(&this->_buf, *this->_filename); 

	}
	else
	{
		file_to_load=sdc._name.c_str();
		this->_owner_id ? hr=sound::dev->DuplicateSound(&this->_buf, sdc._name, this->_owner_id): 
						 hr=sound::dev->DuplicateSound(&this->_buf, sdc._name); 

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
			if((*dword_lpv)!=RIFF_FORMATTED_FILE)throw "dssrc::error message::NOT_RIFF_FILE"; // it's not RIFF-formatted file, canceling...
			dword_lpv+=2;
			if((*dword_lpv)!=WAVE_FORMATTED_FILE)throw "dssrc::error message::NOT_WAVE_FILE"; // it's not WAVE-formatted file, canceling...
			
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
				err.PostError("dssrc::Initialize", hr, "Reading WAV-header error:Size of the data from WAV is greater than filesize",__FILE__,__LINE__);
#endif
				throw "dssrc::error message::WRONG_WAVE_DATA_SIZE:probably wave header is bad";
			}

			header_size+=8;//header_size+=sizeof('data')+sizeof(data_size)
			
			// buffer charasteristics
			WAVEFORMATEX wave;
			
			wave.wFormatTag     = *(( WORD*)local_buffer + FORMAT_TAG_OFFSET);//WAVE_FORMAT_PCM;
			if(wave.wFormatTag!=WAVE_FORMAT_PCM)
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("dssrc::Initialize", hr, "Attempt to read non-PCM file",__FILE__,__LINE__);
#endif
				throw "dssrc::error message::WRONG_FORMAT_TAG:library currently doesnt work with non-PCM files";
			}
			wave.nChannels      = *(( WORD*)local_buffer + CHANNELS_OFFSET);
			wave.nSamplesPerSec = *((DWORD*)local_buffer + SAMPLE_PER_SEC_OFFSET);
			wave.wBitsPerSample = *(( WORD*)local_buffer + BITS_PER_SAMPLE_OFFSET);
			wave.nBlockAlign    = *(( WORD*)local_buffer + BLOCK_ALIGN_OFFSET);
			wave.nAvgBytesPerSec= *((DWORD*)local_buffer + AVG_BYTES_OFFSET);
			wave.cbSize			= 0;

			//logFile[log_name]("FormatTag:%f\nChannels:%f\nSamplesPerSec:%f\nBitsPerSample:%f\nBlockAlign:%f\nAvgBytesPerSec:%f\n",wave.wFormatTag,wave.nChannels,wave.nSamplesPerSec,wave.wBitsPerSample,wave.nBlockAlign,wave.nAvgBytesPerSec);
			
			DSBUFFERDESC dsbd;
			ZeroMemory(&dsbd,sizeof(DSBUFFERDESC));
			dsbd.dwSize =sizeof(DSBUFFERDESC);
			//dsbd.dwFlags=DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY;
			dsbd.dwFlags=DSBCAPS_CTRLVOLUME;

			// 3d buffer
			if(buffer3d)
			{
				dsbd.dwFlags|=DSBCAPS_CTRL3D|DSBCAPS_MUTE3DATMAXDISTANCE; 
				// set 3d algoritm
				switch(sdc._quality.value)
				{
				default:
				case HIGHQUALITY:
					dsbd.guid3DAlgorithm=DS3DALG_HRTF_FULL;
					break;
				case MEDQUALITY:
					dsbd.guid3DAlgorithm=DS3DALG_HRTF_LIGHT;
					break;
				case LOWQUALITY:
					dsbd.guid3DAlgorithm=DS3DALG_NO_VIRTUALIZATION;
					break;
				}
			}
			
			// ask sound _device for hardware accleration
			if(sdc._Flags&LOOPED)dsbd.dwFlags|=DSBCAPS_STATIC; 
			
			dsbd.dwBufferBytes = data_size;
			dsbd.lpwfxFormat   = &wave;
			
			if(FAILED(hr=sound::dev->CreateSound(&dsbd,&this->_buf)))
				throw "dssrc::error message::FAILED_CREATING_SOUND_SOURCE";
			
			if(this->_buf->Lock(0,data_size,(void**)&ptr1,&size1,(void**)&ptr2,&size2,0)!=DS_OK)
				throw "dssrc::error message::FAILED_LOCKING_SOURCE";

			memcpy(ptr1,(BYTE*)local_buffer+header_size,data_size);

			if(this->_buf->Unlock(ptr1,data_size,NULL,0)!=DS_OK)
				throw "dssrc::error message::FAILED_UNLOCKING_SOURCE";

			// release memory
			DataMgr::Release(file_to_load);

			// calculaye average playing time
			this->_avg_playing_time = data_size / wave.nAvgBytesPerSec;
		}
		
		if(buffer3d)
		{
			// Get the 3D buffer from the secondary buffer
			if(FAILED(hr=_buf->QueryInterface(IID_IDirectSound3DBuffer,(VOID**)&this->_buf3d)))
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("IDirectSoundBuffer::QueryInterface()", hr, "failed in dssrc::Initialize",__FILE__,__LINE__);
#endif
				throw "dssrc::error message::QueryInterface()";
			}
			
			// Get the 3D buffer parameters
			DS3DBUFFER  buf3dparms;
			buf3dparms.dwSize=sizeof(DS3DBUFFER);
			
			if(FAILED(hr=this->_buf3d->GetAllParameters(&buf3dparms)))
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("IDirectSound3DBuffer::GetAllParameters()", hr, "failed in dssrc::Initialize",__FILE__,__LINE__);
#endif
			}
			
			// Set new 3D buffer parameters
			//buf3dparms.dwMode        = sdc.Mode;
			buf3dparms.dwMode        = DS3DMODE_NORMAL;
			buf3dparms.flMinDistance = sdc._min_dist.value;
			buf3dparms.flMaxDistance = sdc._max_dist.value;

#ifdef DSPL__ENABLE_EXTENDED_SRC_SETTINGS
			if(sdc.InsideConeAngle.changed)buf3dparms.dwInsideConeAngle=sdc.InsideConeAngle.value;
			if(sdc.OutsideConeAngle.changed)buf3dparms.dwOutsideConeAngle=sdc.OutsideConeAngle.value;
			if(sdc.ConeOutsideVolume.changed)buf3dparms.lConeOutsideVolume=sdc.ConeOutsideVolume.value;
#endif
			if(FAILED(hr = this->_buf3d->SetAllParameters( &buf3dparms, DS3D_IMMEDIATE )))
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("IDirectSound3DBuffer::SetAllParameters()", hr, "failed in dssrc::Initialize",__FILE__,__LINE__);
#endif
			}
		}
		
		int volume=0;
		sound::dev->GetGlobalVolume(&volume);
		if(sdc._vol_value.changed)
		{
			volume+=sdc._vol_value.value;

			if(volume<DSBVOLUME_MIN)
				volume=DSBVOLUME_MIN;
			if(volume>DSBVOLUME_MAX)
				volume=DSBVOLUME_MAX;

			this->_buf->SetVolume(volume);
		}
		else
			this->_buf->SetVolume(volume);
		
		if(sdc._freq_value.changed)
			this->_buf->SetFrequency(sdc._freq_value.value);

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
int  dssrc::Initialize2(const srcdsc& sdc,const char* ovrfname)
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
		// set 3d algoritm
		switch(sdc._quality.value)
		{
		default:
		case HIGHQUALITY:
			dsbd.guid3DAlgorithm=DS3DALG_HRTF_FULL;
			break;
		case MEDQUALITY:
			dsbd.guid3DAlgorithm=DS3DALG_HRTF_LIGHT;
			break;
		case LOWQUALITY:
			dsbd.guid3DAlgorithm=DS3DALG_NO_VIRTUALIZATION;
			break;
		}
	}
	
	dsbd.dwBufferBytes=20*2*2*32*18;
	dsbd.lpwfxFormat=&wf;

	if(FAILED(hr=sound::dev->CreateSound(&dsbd,&this->_buf)))return 0;

	if(enable3d)
	{
        // Get the 3D buffer from the secondary buffer
        if(FAILED(hr=_buf->QueryInterface(IID_IDirectSound3DBuffer,(VOID**)&this->_buf3d)))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirectSoundBuffer::QueryInterface()", hr, "failed in dssrc::Initialize2",__FILE__,__LINE__);
#endif
            return 0;
        }
        // Get the 3D buffer parameters
        DS3DBUFFER  buf3dparms;
        buf3dparms.dwSize = sizeof(DS3DBUFFER);
        if(FAILED(hr = this->_buf3d->GetAllParameters( &buf3dparms )))
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
        if(FAILED(hr = this->_buf3d->SetAllParameters( &buf3dparms, DS3D_IMMEDIATE )))
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

        if(volume<DSBVOLUME_MIN)
			volume=DSBVOLUME_MIN;

        if(volume>DSBVOLUME_MAX)
			volume=DSBVOLUME_MAX;

        this->_buf->SetVolume(volume);
    }
    else
        this->_buf->SetVolume(volume);

	if(ovrfname)
		this->_filename=new std::string(ovrfname);

	// set permanent _lifetype for the MP3-source to deal with Sound Engine
	this->_lifetype=PERMANENT;

	return 1;
};

//
// Set source position
//
void dssrc::SetPos(const point3& pos)
{
    D3DVALUE x=pos.x,
             y=pos.y,
             z=pos.z;

    if(this->_buf3d)
		_buf3d->SetPosition(x,y,z,DS3D_DEFERRED);
}
 
//
// Set source velocity
//
void dssrc::SetVel(const point3& vel)
{
    D3DVALUE x=vel.x,
             y=vel.y,
             z=vel.z;

    if(this->_buf3d)
		_buf3d->SetVelocity(x,y,z,DS3D_DEFERRED);
}

//
// Play source
//
void dssrc::Play()
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
#ifdef DSPL__DEB_VER_STORE_ERRORS
		if(FAILED(hr=RestoreBuffers()))
		{
			err.PostError("RestoreBuffers()", hr, "failed in dssrc::Play()",__FILE__,__LINE__);
		}
#else
		RestoreBuffers();
#endif
		// Play buffer 
		DWORD flags=0;
		DWORD priority=0;

		if(this->_mydesc->_Flags&LOOPED)flags=DSBPLAY_LOOPING;

		// If PCI-card then use Voice Allocation and Voice Manager
		if(sound::dev->TestDeviceFlags(PCI_CARD))
		{
			if(!(this->_mydesc->_Flags&LOWPRIORITY))
				this->_mydesc->_Flags&HIGHPRIORITY ? priority=0xffffffff : priority=0xefffffff;

			if(this->_mydesc->_Flags&STATIC)
				flags|=DSBPLAY_TERMINATEBY_PRIORITY|DSBPLAY_LOCHARDWARE;
			// if there are free voices load buffer in hardware to be mixed
		}

		// reset to the beginning of buffer
		// reason: if buffer play cursor reached end of the buffer 
		//         setting cursor back to the beginning is necessary,
		//         first of all it's got done for PERMANENT buffer that were played and now got to be played again
		_buf->SetCurrentPosition(0);

#ifdef DSPL__DEB_VER_STORE_ERRORS
		if(FAILED(hr=_buf->Play(0,priority,flags)))
		{
			err.PostError("IDirectSoundBuffer::Play()", hr, "failed in dssrc::Play()",__FILE__,__LINE__);
		}
#else
		_buf->Play(0,priority,flags);
#endif
	}
}

//
// Stop source
//
void dssrc::Stop()const
{
    HRESULT hr=0;
	if(this->_mydesc->_Flags&MPEG3FILE)
	{
		iomp3::RemoveMusic();
	}
	else
	{
		if(FAILED(hr=_buf->Stop()))
		{
#ifdef DSPL__DEB_VER_STORE_ERRORS
			err.PostError("IDirectSoundBuffer::Stop()", hr, "failed in dssrc::Stop()",__FILE__,__LINE__);
#endif
			return;
		}
		if(FAILED(hr=_buf->SetCurrentPosition(0L)))
		{
#ifdef DSPL__DEB_VER_STORE_ERRORS
			err.PostError("IDirectSoundBuffer::SetCurrentPosition()", hr, "failed in dssrc::Stop()",__FILE__,__LINE__);
#endif
		}
	}
}

//
// Pause source
//
void dssrc::Pause()
{
    HRESULT hr=0;

	// to prevent sound _device deleteing paused source from Common-List-of Sources,
	// (non-playing and BRIEF-type sources are to be deleted)
	// temporary(!) set _lifetype of source to PERMANENT
	this->_lifetype=PERMANENT;

	if(this->_mydesc->_Flags&MPEG3FILE)
	{
		iomp3::PauseMusic();
	}
	else
	{
		if(FAILED(hr=this->_buf->Stop()))
		{
#ifdef DSPL__DEB_VER_STORE_ERRORS
			err.PostError("IDirectSoundBuffer::Stop()", hr, "failed in dssrc::Stop()",__FILE__,__LINE__);
#endif
			return;
		}
	}
}

//
// Resume source
//
void dssrc::Resume()
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
			err.PostError("RestoreBuffers()", hr, "failed in dssrc::Play()",__FILE__,__LINE__);
#endif
		}
	    // Resume buffer playing
		this->Play();
	}
}

//
// Reset source
//
void dssrc::Reset()const
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    HRESULT hr=0;
    if(FAILED(hr=this->_buf->SetCurrentPosition(0)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("dssrc::Reset()", hr, "failed setting position to 0",__FILE__,__LINE__);
#endif
    }
#else
    this->_buf->SetCurrentPosition(0);
#endif
}

//
// Adjust source volume
//
void dssrc::AdjustVolume(int new_volume)const
{
    if(new_volume < DSBVOLUME_MIN)
		new_volume = DSBVOLUME_MIN;

    if(new_volume > DSBVOLUME_MAX)
		new_volume = DSBVOLUME_MAX;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    HRESULT hr = 0;
    if(FAILED(hr = this->_buf->SetVolume(new_volume)))
    {
		err.PostError("IDirectSoundBuffer::SetVolume()", hr, "failed in dssrc::_volume()",__FILE__,__LINE__);
		return;
	}
#else
    this->_buf->SetVolume(new_volume);
#endif
}

//
// Set frequency for source
//
void dssrc::Freq(unsigned int value)const
{
    HRESULT hr = 0;

    if(FAILED(hr = this->_buf->SetFrequency(value)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundBuffer::SetFrequency()", hr, "failed in dssrc::Freq()",__FILE__,__LINE__);
#endif
        return;
    }
}

//
// Set panning value
//
void dssrc::Pan(int value)const
{
    HRESULT hr = 0;

    if(FAILED(hr = this->_buf->SetPan(value)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundBuffer::SetPan()", hr, "failed in dssrc::Pan()",__FILE__,__LINE__);
#endif
        return;
    }
}  
        
//
// Restore source if it's lost
//
HRESULT dssrc::RestoreBuffers()const
{
    HRESULT hr = 0;

    if(_buf==NULL)return S_OK;

    DWORD dwStatus;
    if(FAILED(hr=this->_buf->GetStatus(&dwStatus)))return hr;

    if(dwStatus&DSBSTATUS_BUFFERLOST)
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("dssrc::RestoreBuffers()", hr, "buffer has been lost,trying to restore from file="+*this->_filename,__FILE__,__LINE__);
#endif
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control.
        do 
        {
            hr=this->_buf->Restore();
            if(hr==DSERR_BUFFERLOST)Sleep(10);
        }
        while(hr=this->_buf->Restore());

        if(FAILED(hr=this->FillBuffer()))return hr;
    }

    return S_OK;
}

//
// Fill buffer with data
//
HRESULT dssrc::FillBuffer()const
{
	BYTE* ptr1=NULL;
	BYTE* ptr2=NULL;
	DWORD size1=0;
	DWORD size2=0;
	HRESULT hr=0;
	const unsigned char* local_buffer=NULL;
	const std::string* filename=NULL;
	this->_filename ? filename=this->_filename : filename=&_mydesc->_name;
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
		
		if(hr=this->_buf->Lock(0,data_size,(void**)&ptr1,&size1,(void**)&ptr2,&size2,0)!=DS_OK)throw "failed locking sound buffer";

		memcpy(ptr1,(BYTE*)local_buffer+header_size,data_size);

		if(hr=this->_buf->Unlock(ptr1,data_size,NULL,0)!=DS_OK)throw "failed unlocking sound buffer";
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	catch(const char* error)
	{
		err.PostError("dssrc::FillBuffer", -1, error,__FILE__,__LINE__);
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
bool dssrc::IsPlaying()const
{
    // if buffer is playing
    if(_buf)
    {  
        DWORD dwStatus = 0;
        this->_buf->GetStatus( &dwStatus );

#ifdef DSPL__DEB_VER_STORE_ERRORS
        if(dwStatus & DSBSTATUS_LOCHARDWARE)logFile[log_name]("Playing in hardware...");
        if(dwStatus & DSBSTATUS_LOCSOFTWARE)logFile[log_name]("Playing in software...");
        if(dwStatus & DSBSTATUS_LOOPING)    logFile[log_name]("Playing loop...");
        return((dwStatus & DSBSTATUS_PLAYING) != 0 );

#else
        return((dwStatus & DSBSTATUS_PLAYING) != 0 );
#endif
    }
    else
		return false;
}

//
// Slow volume decay
//
void dssrc::SmoothVolumeDecay()
{
    LONG curr_volume=0;

	logFile[log_name]("EXEC:Smoothing down id=%d.\n",this->_sid);

#ifdef DSPL__DEB_VER_STORE_ERRORS
    //logFile[log_name]("Smoothing down...\n");
    HRESULT hr=0;
    if(FAILED(hr = this->_buf->GetVolume(&curr_volume)))
        err.PostError("dssrc::SmoothVolumeDecay()", hr, "failed getting volume",__FILE__,__LINE__);
#else
    this->_buf->GetVolume(&curr_volume);
#endif
	//curr_volume-=VOLUME_TUNING;
	curr_volume--;
	curr_volume*=2.f;
	//logFile[log_name]("Smoothing down...[volume=%d]\n",curr_volume);
    if(curr_volume<=VOLUME_LOW_LIMIT)
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
		this->_filename ?
			logFile[log_name]("[%s,id=%d]has been smoothed down...\n",this->_filename->c_str(),this->get_id()) :
			logFile[log_name]("[%s,id=%d]has been smoothed down...\n",this->_mydesc->_name.c_str(),this->get_id());
#endif
        this->_exec_source=NULL;
		if(this->_mydesc&&(_mydesc->_Flags&MPEG3FILE))
			iomp3::RemoveMusic();
		else
			_buf->Stop();
        return;
    }

#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(hr = this->_buf->SetVolume(curr_volume)))
        err.PostError("dssrc::SmoothVolumeDecay()", hr, "failed setting volume",__FILE__,__LINE__);
#else
    this->_buf->SetVolume(curr_volume);
#endif
}

//
// Slow volume expand
//
void dssrc::SmoothVolumeExpand()
{
    LONG curr_volume=0;

	logFile[log_name]("EXEC:Smoothing up id=%d.\n",this->_sid);

#ifdef DSPL__DEB_VER_STORE_ERRORS
    //logFile[log_name]("Smoothing up...\n");
    HRESULT hr=0;
    if(FAILED(hr=this->_buf->GetVolume(&curr_volume)))
        err.PostError("dssrc::SmoothVolumeExpand()", hr, "failed getting volume",__FILE__,__LINE__);
#else
    this->_buf->GetVolume(&curr_volume);
#endif
    //curr_volume+=VOLUME_TUNING;
	curr_volume/=2.f;
	//logFile[log_name]("Smoothing up...[volume=%d]\n",curr_volume);
    if(curr_volume>=_volume)
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
    if(FAILED(hr=this->_buf->SetVolume(curr_volume)))
        err.PostError("dssrc::SmoothVolumeExpand()", hr, "failed setting volume",__FILE__,__LINE__);
#else
    this->_buf->SetVolume(curr_volume);
#endif
}

//
// Smooth playing
//
void dssrc::SmoothPlay()
{
	// if MP3-type source, first attempt to get MP3-pipe to play itself
	// if successed, then start playing and growing up it's volume
	if(this->_mydesc->_Flags&MPEG3FILE)
	{
		const std::string* filename=NULL;
		this->_filename ? filename=this->_filename : filename=&this->_mydesc->_name;

		unsigned int result=iomp3::PlayMusic(*filename,this);

#ifdef DSPL__DEB_VER_STORE_ERRORS
		logFile[log_name]("Source named[%s, id=%d] awaits to be played...\n",filename->c_str(),this->get_id());
#endif
		if(result)
		{
			// if MP3-pipe is busy - we have to wait until it's not, so we can't expand volume
			if(result==IOMP3_BUSY)
				return;

			if(sdsrc::in_queue > 0)
				sdsrc::in_queue--;

#ifdef DSPL__DEB_VER_STORE_ERRORS
			logFile[log_name]("Source named[%s, id=%d] has got MPEG3 decoder resource and will be played...(in queue N%d).\n",filename->c_str(),this->get_id(),sdsrc::in_queue);
#endif
		}
		else
		{
			// all attempts to start smooth playing file failed
			// delete source from global map
			this->_lifetype=BRIEF;
#ifdef DSPL__DEB_VER_STORE_ERRORS
			err.PostError("dssrc::SmoothPlay()",-1,"failed waiting for MPEG3 pipe",__FILE__,__LINE__);
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
