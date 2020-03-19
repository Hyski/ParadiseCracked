/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Aureal arranged sound library
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/

#include "..\precomp.h"
#include "sndcompiler.h"
#include "sound.h"
#include "a3ddev.h"
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

// A3D Include Files.
#include <initguid.h>
#include "ia3dutil.h"
#include "ia3dapi.h"

/***************************************************************************
****************************************************************************

                            a3ddev realization

***************************************************************************
***************************************************************************/

a3ddev::a3ddev() : sddev(),device(NULL),camera(NULL),reverb(NULL),GlobalVolume(1.f),MusicVolume(1.f)
{         
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    struct tm *newtime;        
    time_t long_time;
    time( &long_time );                // Get time as long integer. 
    newtime = localtime( &long_time ); // Convert to local time. 

    logFile[log_name]("Sound log is opened now...\t%.19s \nAureal support is connected to sound engine.\n", asctime(newtime));
#endif
}

a3ddev::~a3ddev()
{
	// Clean-Up the list of emitters and the list of sources
	this->Close();
    // release previously allocated objects
    if(this->device){ this->device->Shutdown(); this->device = NULL; };

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
// Create Aureal source with owner id (for sound emitters)(smart)
//
HRESULT a3ddev::CreateSound(void* info,void* buf, const std::string& fname,const unsigned int& _owner_id,bool& duplicated) const
{
    REGISTERED_EMITTERS::const_iterator ci;
    sdsrc* src=NULL;

    for(ci=this->_emitmap.begin();ci!=this->_emitmap.end();++ci)
    {
        // trying to get duplication copy
        if((ci->first!=_owner_id) && (src=ci->second->browse(fname)))
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
            logFile[log_name]("**COMMENT** Duplicating: found source for duplicating [%s]...\n",fname.c_str());
#endif
            duplicated=true;
            return this->device->DuplicateSource(reinterpret_cast<IA3dSource2*>(src->GetBuf()),reinterpret_cast<IA3dSource2**>(buf));
        }
    }
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("**COMMENT** Creating original: no source's been found duplicating... creating original one [%s].\n",fname.c_str());
#endif

    return this->device->NewSource(A3DSOURCE_INITIAL_RENDERMODE_A3D,reinterpret_cast<IA3dSource2**>(buf));
}

//
// Create Aureal source (smart)
//
HRESULT a3ddev::CreateSound(void* info,void* buf, const std::string& fname,bool& duplicated) const
{
    OWNEDBY_DEVICE_SOURCES::const_iterator ci=this->_devsrcmap.find(fname);

    if(ci!=this->_devsrcmap.end())
    {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        logFile[log_name]("**COMMENT** Duplicating: found source for duplicating [%s]...\n",fname.c_str());
#endif
        duplicated=true;
        return this->device->DuplicateSource(reinterpret_cast<IA3dSource2*>(ci->second->GetBuf()),reinterpret_cast<IA3dSource2**>(buf));
    }
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("**COMMENT** Creating original: no source's been found duplicating... creating original one [%s].\n",fname.c_str());
#endif

    // if info is not NULL, then creating with 3d enhacements
    // else - NATIVE source
    if(info)
    {
        srcdsc* description=reinterpret_cast<srcdsc*>(info);

		return (description->_Flags&DISABLE_3D)?
            this->device->NewSource(A3DSOURCE_INITIAL_RENDERMODE_NATIVE,reinterpret_cast<IA3dSource2**>(buf)):
            this->device->NewSource(A3DSOURCE_INITIAL_RENDERMODE_A3D,   reinterpret_cast<IA3dSource2**>(buf));
    }

    return this->device->NewSource(A3DSOURCE_INITIAL_RENDERMODE_NATIVE, (IA3dSource2**)buf);
}

//
// Duplicate Aureal source with owner id (for sound emitters)
//
HRESULT a3ddev::DuplicateSound(void* buf, const std::string& fname,const unsigned int& _owner_id) const
{
    REGISTERED_EMITTERS::const_iterator ci;
    sdsrc* src=NULL;

    for(ci=this->_emitmap.begin();ci!=this->_emitmap.end();++ci)
    {
        // trying to get duplication copy
        if((ci->first!=_owner_id) && (src=ci->second->browse(fname)))
        {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
            logFile[log_name]("**COMMENT** Duplicating: found source for duplicating [%s]...\n",fname.c_str());
#endif
            return this->device->DuplicateSource(reinterpret_cast<IA3dSource2*>(src->GetBuf()),reinterpret_cast<IA3dSource2**>(buf));
        }
    }
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("**COMMENT** Creating original: no source's been found duplicating... creating original one [%s].\n",fname.c_str());
#endif

    return A3DERROR_NOT_VALID_SOURCE;
}

//
// Duplicate Aureal source
//
HRESULT a3ddev::DuplicateSound(void* buf, const std::string& fname) const
{
    OWNEDBY_DEVICE_SOURCES::const_iterator ci=this->_devsrcmap.find(fname);

    if(ci!=this->_devsrcmap.end())
    {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        logFile[log_name]("**COMMENT** Duplicating: found source for duplicating [%s]...\n",fname.c_str());
#endif
        return this->device->DuplicateSource(reinterpret_cast<IA3dSource2*>(ci->second->GetBuf()),reinterpret_cast<IA3dSource2**>(buf));
    }
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("**COMMENT** Creating original: no source's been found duplicating... creating original one [%s].\n",fname.c_str());
#endif

    return A3DERROR_NOT_VALID_SOURCE;
}

//
// Create Aureal source(simple)
//
HRESULT a3ddev::CreateSound(void* info,void* buf) const
{
    if(info)
    {
        srcdsc* description=reinterpret_cast<srcdsc*>(info);

		return (description->_Flags&DISABLE_3D)?
            this->device->NewSource(A3DSOURCE_INITIAL_RENDERMODE_NATIVE,reinterpret_cast<IA3dSource2**>(buf)):
            this->device->NewSource(A3DSOURCE_INITIAL_RENDERMODE_A3D,   reinterpret_cast<IA3dSource2**>(buf));
    }
	return 0;
}

//
// Duplicate Aureal source(simple)
//
HRESULT a3ddev::DuplicateSound(void* srcbuf,void* destbuf) const
{
    return this->device->DuplicateSource(reinterpret_cast<IA3dSource2*>(srcbuf),reinterpret_cast<IA3dSource2**>(destbuf));
}

//
// Get Aureal source
//
inline sound::sdsrc* a3ddev::PurchaseSource()const{ return new a3dsrc; };

//
// Get Aureal source
//
inline void a3ddev::PurchaseSource(sound::sdsrc** source)const{ *source=new a3dsrc; };

//
// Get Aureal source with owner id
//
inline void a3ddev::PurchaseSource(sound::sdsrc** source,const unsigned int& id)const{ *source=new a3dsrc(id); };

//
// Get Aureal source with owner id
//
inline sound::sdsrc* a3ddev::PurchaseSource(const unsigned int& id) const 
{ 
    return new a3dsrc(id); 
};

//
// Set reverberation
//
bool a3ddev::BindReverb()
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(device->IsFeatureAvailable(A3D_REVERB) == FALSE)
    {
	    err.PostError("a3ddev::BindReverb()",-1,"reverb is not available",__FILE__,__LINE__);
        return false;
    }
    // Create a Reverb object.
	if(FAILED(device->NewReverb(&reverb)))
	{
	    err.PostError("a3ddev::BindReverb()",-1,"failed setting reverb",__FILE__,__LINE__);
        return false;
    }
	// Bind it to A3D.
	if(FAILED(device->BindReverb(reverb)))
    {
	    err.PostError("a3ddev::BindReverb()",-1,"failed binding reverb",__FILE__,__LINE__);
        return false;
    }
    return true;
#else
    if(this->device->IsFeatureAvailable(A3D_REVERB))
    {
        this->device->NewReverb(&this->reverb);
        this->device->BindReverb(this->reverb);
		return true;
    }
    return false;
#endif
}

//
// Set environment
//
void a3ddev::SetEnvironment(Environment env)
{
    DWORD value=0;
    switch(env)
    {
        case GENERIC: value=A3DREVERB_PRESET_GENERIC;break;
        case PADDEDCELL: value=A3DREVERB_PRESET_PADDEDCELL;break;
        case ROOM:   value=A3DREVERB_PRESET_ROOM;break;
        case BATHROOM:value=A3DREVERB_PRESET_BATHROOM; break;
        case LIVINGROOM:value=A3DREVERB_PRESET_LIVINGROOM; break;
        case STONEROOM:value=A3DREVERB_PRESET_STONEROOM; break;
        case AUDITORIUM:value=A3DREVERB_PRESET_AUDITORIUM; break;
        case CONCERTHALL:value=A3DREVERB_PRESET_CONCERTHALL; break;
        case CAVE:value=A3DREVERB_PRESET_CAVE; break;
        case ARENA:value=A3DREVERB_PRESET_ARENA; break;
        case HANGAR:value=A3DREVERB_PRESET_HANGAR; break;
        case CARPETEDHALLWAY:value=A3DREVERB_PRESET_CARPETEDHALLWAY; break;
        case HALLWAY:value=A3DREVERB_PRESET_HALLWAY; break;
        case STONECORRIDOR:value=A3DREVERB_PRESET_STONECORRIDOR; break;
        case ALLEY:value=A3DREVERB_PRESET_ALLEY; break;
        case FOREST:value=A3DREVERB_PRESET_FOREST; break;
        case CITY:value=A3DREVERB_PRESET_CITY; break;
        case MOUNTAINS:value=A3DREVERB_PRESET_MOUNTAINS; break;
        case QUARRY:value=A3DREVERB_PRESET_QUARRY; break;
        case PLAIN:value=A3DREVERB_PRESET_PLAIN; break;
        case PARKINGLOT:value=A3DREVERB_PRESET_PARKINGLOT; break;
        case SEWERPIPE:value=A3DREVERB_PRESET_SEWERPIPE; break;
        case UNDERWATER:value=A3DREVERB_PRESET_UNDERWATER; break;
        case DRUGGED:value=A3DREVERB_PRESET_DRUGGED; break;
        case DIZZY:value=A3DREVERB_PRESET_DIZZY; break;
        default: value=A3DREVERB_PRESET_GENERIC;
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(this->reverb->SetReverbPreset(value)))
        err.PostError("a3ddev::SetEnvironment()",-1,"failed setting environment",__FILE__,__LINE__);
#else
    this->reverb->SetReverbPreset(value);
#endif
}

//
// Set coordinate system
//
void a3ddev::SetCoordinateSystem(CoordinateSystem system)
{
// Set coordinate system
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(this->device->SetCoordinateSystem((system==LEFT_HANDED)?A3D_LEFT_HANDED_CS:A3D_RIGHT_HANDED_CS)))
        err.PostError("a3ddev::Initialize()", -1, "failed setting coordianete system",__FILE__,__LINE__);
#else
    this->device->SetCoordinateSystem((system==LEFT_HANDED)?A3D_LEFT_HANDED_CS:A3D_RIGHT_HANDED_CS);
#endif
}

//
// Initialize Aureal Sound
//
bool a3ddev::Initialize(HWND hwnd)
{
    HRESULT hResult = 0;
	SndCompiler compiler;

    try
    { 
        // Initialize COM libraries.
    	hResult = CoInitialize(NULL);
	    if(FAILED(hResult))
	    {
	        throw "Failed to initialize the COM libraries";
		}

    	// Create and instantiate the A3D interface.
  		hResult = CoCreateInstance(CLSID_A3dApi,NULL,CLSCTX_INPROC_SERVER,IID_IA3d5,(void**)&this->device);
		if(FAILED(hResult))
		{
			throw "Failed to CoCreate() IID_IA3d5 interface";
		}

		hResult = this->device->A3dEnumerate((LPA3DENUMCALLBACK)EnumerateSoundDevices,NULL);
		if(FAILED(hResult))
		{
			throw "Failed enumerating Sound Devices";
		}

        DWORD Features = A3D_REVERB|A3D_DISABLE_SPLASHSCREEN;
        //DWORD Features = A3D_DISABLE_SPLASHSCREEN;// no Reverb
      	hResult = this->device->InitEx(NULL, Features, A3DRENDERPREFS_DEFAULT, hwnd, A3D_CL_NORMAL);
        //hResult = device->InitEx(NULL, NULL, A3DRENDERPREFS_DEFAULT, hwnd, A3D_CL_NORMAL);
        if(FAILED(hResult))
        {
            throw "Failed to initialize A3D";
        }

		// Query for the listener object used for 3D sound positioning.
      	hResult = this->device->QueryInterface(IID_IA3dListener,(void **)&this->camera);
		if(FAILED(hResult))
		{
			throw "Failed to create listener";
		}
		hResult=this->device->SetOutputGain(this->GlobalVolume);
		if(FAILED(hResult))
		{
			throw "Failed to create listener";
		}
		// Disable A3D GL Debug Viewer
		hResult=this->device->DisableViewer();
		if(FAILED(hResult))
		{
			throw "Failed disabling DebugViewer";
		}
    }
	catch(char* szError)
	{
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("a3ddev::Initialize()", -1, szError,__FILE__,__LINE__);
#endif
        return false;
	}

    compiler.Compile(script_filename);

	if(ListenerDefs._doppler_factor.changed)this->device->SetDopplerScale(ListenerDefs._doppler_factor.value);
	if(ListenerDefs._rolloff.changed)      this->device->SetDistanceModelScale(ListenerDefs._rolloff.value);

    // bind reverbaration to the camera
    if(this->BindReverb())
    {
        if(ListenerDefs._environment.changed)this->reverb->SetReverbPreset(ListenerDefs._environment.value);
/*        A3DREVERB_CUSTOM     reverb_preset;
        A3DREVERB_PROPERTIES reverb_props;
        reverb_preset.dwSize=sizeof(A3DREVERB_PRESET);
        reverb_props.dwType =sizeof(A3DREVERB_TYPE_PRESET); 
        reverb_props.dwSize =sizeof(A3DREVERB_PROPERTIES);

        if(ListenerDefs._environment.changed)
            reverb_props.uval.preset.dwEnvPreset=ListenerDefs._environment.value;
        reverb->SetAllProperties(&reverb_props);*/
    }
    
    return true;
}

//
// Set properties for device
//
void a3ddev::SetDeviceProperty(lprops prop,void* valaddr)
{
	if(valaddr)
	{
		switch(prop)
		{
		case PROP_ROLLOFF:
			this->device->SetDistanceModelScale(*reinterpret_cast<float*>(valaddr));
			break;
		case PROP_DOPPLER_FACTOR:
			this->device->SetDopplerScale(*reinterpret_cast<float*>(valaddr));
			break;
		case PROP_ENVIRONMENT:
			this->SetEnvironment(*reinterpret_cast<Environment*>(valaddr));
			break;
		case PROP_AIR_ABSORPTION:
			this->device->SetHFAbsorbFactor(*reinterpret_cast<float*>(valaddr));
			break;
		case PROP_REFLECTIONS_DELAY:
			this->device->SetMaxReflectionDelayTime(*reinterpret_cast<A3DVAL*>(valaddr));
			break;
		default:
		case PROP_ROOM:
		case PROP_ROOM_HF:
		case PROP_REFLECTIONS:
		case PROP_PROP_FLAGS:
		case PROP_REVERB_DELAY:
		case PROP_ROOM_ROLLOFF:
		case PROP_DECAY_TIME:
		case PROP_DECAY_HF_RATIO:
		case PROP_ENVIRONMENT_SIZE:
		case PROP_ENVIRONMENT_DIFFUSION:
		case PROP__FLAGS:
			this->_device_flags|=*reinterpret_cast<DWORD*>(valaddr);
			break;
		}
	}
}

//
// Playing sound (effect of slow volume growing)
//
void a3ddev::SmoothPlay(const tray& tr, const point3* new_pos)
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
				SRCFUNC func=&sdsrc::SmoothVolumeExpand;
				source->SetSourceExec(func);
				((IA3dSource2*)(source->GetBuf()))->SetGain(0.f);
				if(new_pos)source->SetPos(*new_pos);
				source->Play();
				typedef OWNEDBY_DEVICE_SOURCES::value_type Pairs;
				Pairs new_pair(filename,source);
				this->_devsrcmap.insert(new_pair);
			}
			else
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("a3dsrc::Initialize()", -1, "failed in a3ddev::SmoothPlay()",__FILE__,__LINE__);
#endif
				this->ThrashSource(source);
			}
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("a3ddev::SmoothPlay()", -1, "description was not found:" + *tr._cell1,__FILE__,__LINE__);
#endif
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("a3ddev::SmoothPlay()", -1, "description was not found(NULL)",__FILE__,__LINE__);
#endif
}

//
// Update sound device (as listener changed its position)
//
void a3ddev::Update(const point3& pos,
					const point3& vel,
					const point3& front,
					const point3& up) 
{
	//
	// WARNING: TESTING AREA.
	//
	// first, be sure that not too frequent calls
	static float  _prev_game_time = 0.f;
	float	       _cur_game_time = Timer::GetSeconds();
	float _min_game_time_interval = 1.f / Options::GetFloat("system.sound.update_freq");

	if((_cur_game_time - _prev_game_time) < _min_game_time_interval) return;
	_prev_game_time = _cur_game_time;

    // clear sound frame
    this->device->Clear();

    // updating camera
#ifdef DSPL__DEB_VER_STORE_ERRORS 
    if(FAILED(this->camera->SetPosition3f(pos.x,pos.y,pos.z)))
    {
        err.PostError("a3ddev::Update()", -1, "failed setting position",__FILE__,__LINE__);
    }
    if(FAILED(camera->SetVelocity3f(vel.x,vel.y,vel.z)))
    {
        err.PostError("a3ddev::Update()", -1, "failed setting velocity",__FILE__,__LINE__);
    }
    if(FAILED(camera->SetOrientation6f(front.x,front.y,front.z,up.x,up.y,up.z)))
    {
        err.PostError("a3ddev::Update()", -1, "failed setting orientation",__FILE__,__LINE__);
    }
#else
    this->camera->SetPosition3f(pos.x,pos.y,pos.z);
    this->camera->SetVelocity3f(vel.x,vel.y,vel.z);
    this->camera->SetOrientation6f(front.x,front.y,front.z,up.x,up.y,up.z);
#endif

    // updating emitters
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
            err.PostError("a3ddev::Update()",-1,"attempt to use missed emitter in map",__FILE__,__LINE__);
#endif
    }

    this->device->Flush();

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
}

//
// Update sound device (as listener is still there)
//
void a3ddev::Update() 
{
	//
	// WARNING: TESTING AREA.
	//
	// first, be sure that not too frequent calls
	static float  _prev_game_time = 0.f;
	float	       _cur_game_time = Timer::GetSeconds();
	float _min_game_time_interval = 1.f / Options::GetFloat("system.sound.update_freq");

	if((_cur_game_time - _prev_game_time) < _min_game_time_interval) return;
	_prev_game_time = _cur_game_time;

    // clear sound frame
    this->device->Clear();

    // updating emitters
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
            err.PostError("a3ddev::Update()",-1,"attempt to use missed emitter in map",__FILE__,__LINE__);
#endif
    }

    this->device->Flush();

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

#endif
}

// increase/decrease volume for all sources in system
/*void a3ddev::AdjustGlobalVolume(int value)
{
    float volume=0;
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(device->GetOutputGain(&volume)))
    {
        err.PostError("a3ddev::Volume()", -1, "failed getting volume",__FILE__,__LINE__);
        return;
    }
#else
    device->GetOutputGain(&volume);
#endif

    volume+=((float)value)/10000;
    if(volume<0.f)volume=0.f;
    if(volume>1.f)volume=1.f;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(device->SetOutputGain(volume)))
        err.PostError("a3ddev::Volume()", -1, "failed setting volume",__FILE__,__LINE__);
#else
    device->SetOutputGain(volume);
#endif
}*/

//
// Adjust output volume
//
void a3ddev::AdjustGlobalVolume(int value)
{
    // setting global volume for all sources 
    this->GlobalVolume+=((float)value)/10000;

    if(this->GlobalVolume<0.f)
		this->GlobalVolume=0.f;

    if(this->GlobalVolume>1.f)
		this->GlobalVolume=1.f;

    REGISTERED_EMITTERS::const_iterator cie;
    for(cie=this->_emitmap.begin();cie!=this->_emitmap.end();++cie)
    {
        if(cie->second) 
            cie->second->AdjustVolume(value);
    }
    OWNEDBY_DEVICE_SOURCES::const_iterator cis;
    for(cis=this->_devsrcmap.begin();cis!=this->_devsrcmap.end();++cis)
    {
        //if(cis->second&&(!(cis->second->GetSrcDesc()->mpegfile.mpeg3file))) 
		if(cis->second&&(!(cis->second->GetSrcDesc()->_Flags&MPEG3FILE))) 
            cis->second->AdjustVolume(value);
    }
}

//
// Update only music volume
//
void a3ddev::AdjustMusicVolume(int value)
{
    // setting global volume for all sources except
    this->MusicVolume+=((float)value)/10000;

    if(this->MusicVolume<0.f)
		this->MusicVolume=0.f;

    if(this->MusicVolume>1.f)
		this->MusicVolume=1.f;

    OWNEDBY_DEVICE_SOURCES::const_iterator cis;
    for(cis=this->_devsrcmap.begin();cis!=this->_devsrcmap.end();++cis)
    {
        //if(cis->second&&cis->second->GetSrcDesc()->mpegfile.mpeg3file) 
		if(cis->second&&cis->second->GetSrcDesc()->_Flags&MPEG3FILE) 
            cis->second->AdjustVolume(value);
    }
};

//
// get camera position
//
void a3ddev::GetPos(point3& pos)const
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(this->camera->GetPosition3f(&pos.x,&pos.y,&pos.z)))
    {
        err.PostError("a3ddev::GetPos()", -1, "failed getting position",__FILE__,__LINE__);
    }
#else
    this->camera->GetPosition3f(&pos.x,&pos.y,&pos.z);
#endif
}

//
// get camera velocity 
//
void a3ddev::GetVel(point3& vel)const
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(this->camera->GetVelocity3f(&vel.x,&vel.y,&vel.z)))
    {
        err.PostError("a3ddev::GetVel()", -1, "failed getting velocity",__FILE__,__LINE__);
    }
#else
    this->camera->GetVelocity3f(&vel.x,&vel.y,&vel.z);
#endif
}

//
// set camera position 
//
void a3ddev::SetPos(const point3& pos)
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(this->camera->SetPosition3f(pos.x,pos.y,pos.z)))
    {
        err.PostError("a3ddev::SetPos()", -1, "failed setting position",__FILE__,__LINE__);
    }
#else
    this->camera->SetPosition3f(pos.x,pos.y,pos.z);
#endif
}

//
// set camera velocity 
//
void a3ddev::SetVel(const point3& vel)
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(this->camera->SetVelocity3f(vel.x,vel.y,vel.z)))
    {
        err.PostError("a3ddev::SetVel()", -1, "failed setting velocity",__FILE__,__LINE__);
    }
#else
    this->camera->SetVelocity3f(vel.x,vel.y,vel.z);
#endif
}

//
// set camera orientation 
//
void a3ddev::BeOriented(const point3& front, const point3& top)
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(this->camera->SetOrientation6f(front.x,front.y,front.z,top.x,top.y,top.z)))
    {
        err.PostError("a3ddev::SetESWN()", -1, "failed setting orientation",__FILE__,__LINE__);
    }
#else
    this->camera->SetOrientation6f(front.x,front.y,front.z,top.x,top.y,top.z);
#endif
}

//
// Set camera position
//
void a3ddev::SetXYZ(const point3& pos,const point3& vel,const point3& front,const point3& top)
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(this->camera->SetPosition3f(pos.x,pos.y,pos.z)))
    {
        err.PostError("a3ddev::SetXYZ()", -1, "failed setting position",__FILE__,__LINE__);
    }
    if(FAILED(this->camera->SetVelocity3f(vel.x,vel.y,vel.z)))
    {
        err.PostError("a3ddev::SetXYZ()", -1, "failed setting velocity",__FILE__,__LINE__);
    }
    if(FAILED(this->camera->SetOrientation6f(front.x,front.y,front.z,top.x,top.y,top.z)))
    {
        err.PostError("a3ddev::SetXYZ()", -1, "failed setting orientation",__FILE__,__LINE__);
    }
#else
    this->camera->SetPosition3f(pos.x,pos.y,pos.z);
    this->camera->SetVelocity3f(vel.x,vel.y,vel.z);
    this->camera->SetOrientation6f(front.x,front.y,front.z,top.x,top.y,top.z);
#endif
}

/***************************************************************************
****************************************************************************

                            a3dsrc realization

***************************************************************************
***************************************************************************/

inline a3dsrc::a3dsrc()                : sdsrc()  ,src(NULL),Volume(0.f) {}
inline a3dsrc::a3dsrc(unsigned int id) : sdsrc(id),src(NULL),Volume(0.f) {}

a3dsrc::a3dsrc(const a3dsrc& sndbuf) : src(NULL)
{
}

a3dsrc::~a3dsrc()
{
    if(this->src){ this->src->FreeAudioData(); };
    if(this->src){ this->src->Release(); this->src=NULL; };
}

//
// Initialize sound source
//
bool a3dsrc::Initialize(const srcdsc& sdc,const char* ovrfname)
{
    HRESULT hr=0;

    this->_mydesc=&sdc;
    
    bool duplicated=false;
    bool _mp3file_=sdc._Flags&MPEG3FILE;

	if(sdc._Flags&PERMANENT)
		this->_lifetype=PERMANENT;

	const char* file_to_load=NULL;

	if(_mp3file_)
	{
		// to escape attepmts to duplicate MP3-type sources
		hr=sound::dev->CreateSound(const_cast<srcdsc*>(&sdc), &src); 
		if(ovrfname)
		{
			file_to_load=ovrfname;
			this->_filename=new std::string(ovrfname);
		}
		else
			file_to_load=sdc._name.c_str();
	}
	else
	{
		if(ovrfname)
		{
			file_to_load=ovrfname;
			this->_filename=new std::string(ovrfname);
			this->_owner_id ? hr=sound::dev->CreateSound(const_cast<srcdsc*>(&sdc), &this->src, *this->_filename, this->_owner_id, duplicated): 
				       hr=sound::dev->CreateSound(const_cast<srcdsc*>(&sdc), &this->src, *this->_filename, duplicated); 
		}
		else
		{
			file_to_load=sdc._name.c_str();
			_owner_id ? hr=sound::dev->CreateSound(const_cast<srcdsc*>(&sdc), &this->src, sdc._name, this->_owner_id, duplicated): 
				       hr=sound::dev->CreateSound(const_cast<srcdsc*>(&sdc), &this->src, sdc._name, duplicated)          ; 
		}
	}

	if(FAILED(hr))return false;
            
    if(sdc._min_dist.changed&&sdc._max_dist.changed)
		this->src->SetMinMaxDistance(sdc._min_dist.value,sdc._max_dist.value,A3D_MUTE);

    if(sdc._priority.changed)
		this->src->SetPriority(sdc._priority.value);

    float global_volume=0;
    _mp3file_ ? sound::dev->GetMusicVolume(&global_volume):
			    sound::dev->GetGlobalVolume(&global_volume);
    
    if(sdc._vol_value.changed)
    {
        float gain=((float)sdc._vol_value.value)/10000;
        gain+=global_volume;
        if(gain<0.f)gain=0.f;
        if(gain>1.f)gain=1.f;
        Volume=gain;
    }
    else
        Volume=global_volume;

    this->src->SetGain(Volume);

    if(duplicated)return true;

    DWORD flags=_mp3file_ ? A3DSOURCE_FORMAT_MP3 : A3DSOURCE_FORMAT_WAVE;
    if(sdc._Flags&STREAMED) flags|=A3DSOURCE_STREAMING;

	if(!_mp3file_)
	{
		DWORD size=0;
        BYTE* ptr1=NULL;
	    BYTE* ptr2=NULL;
	    DWORD size1=0;
	    DWORD size2=0;

		const unsigned char* local_buffer=NULL;

		VFile *VFile = DataMgr::Load(file_to_load);

		size=VFile->Size();
		if(!size)
		{
			DataMgr::Release(file_to_load);
			return false;
		}
		else 
			local_buffer=VFile->Data();

		if(FAILED(this->src->LoadWaveData((void*)local_buffer,size)))
			return false;

		DataMgr::Release(file_to_load);
	}
	else
	{
		if(FAILED(hr=this->src->LoadFile(const_cast<char*>(file_to_load),flags)))
			return false;
	}

	// calculate average playing time
	int file_size = this->src->GetAudioSize();

	this->src->SetPlayPosition(file_size);
	this->src->GetPlayTime(&this->_avg_playing_time);
	this->src->SetPlayPosition(0);

    return true;    
}

//
// Set position for sound source
//
void a3dsrc::SetPos(const point3& pos)
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(this->src->SetPosition3f(pos.x,pos.y,pos.z)))
    {
        err.PostError("a3dsrc::SetPos()", -1, "failed setting position",__FILE__,__LINE__);
    }
#else
    this->src->SetPosition3f(pos.x,pos.y,pos.z);
#endif
} 
 
//
// set velocity for sound source
//
void a3dsrc::SetVel(const point3& vel)
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(this->src->SetVelocity3f(vel.x,vel.y,vel.z)))
    {
        err.PostError("a3dsrc::SetVel()", -1, "failed setting velocity",__FILE__,__LINE__);
    }
#else
    this->src->SetVelocity3f(vel.x,vel.y,vel.z);
#endif
}

//
// Play sound source
//
void a3dsrc::Play()
{
    HRESULT hr=0;

    DWORD ToLoopOrNot = this->_mydesc->_Flags&LOOPED ? A3D_LOOPED : A3D_SINGLE;

    if(FAILED(hr = this->src->Play(ToLoopOrNot)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("a3dsrc::Play()", -1, "failed playing sound",__FILE__,__LINE__);
#endif
    }
}

//
// Stop sound source
//
void a3dsrc::Stop()const
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    HRESULT hr=0;
    if(FAILED(hr = this->src->Stop()))
        err.PostError("a3dsrc::Stop()", hr, "failed stopping sound",__FILE__,__LINE__);
    if(FAILED(hr = this->src->SetPlayPosition(0)))
        err.PostError("a3dsrc::SetCurrentPosition()", hr, "failed setting position to 0",__FILE__,__LINE__);
#else
    this->src->Stop();
    this->src->SetPlayPosition(0);
#endif
}

//
// Pause sound source
//
void a3dsrc::Pause()
{
	// to prevent sound device deleteing paused source from Common-List-of Sources,
	// (non-playing and BRIEF-type sources are to be deleted)
	// temporary(!) set _lifetype of source to PERMANENT
	this->_lifetype=PERMANENT;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    HRESULT hr=0;
    if(FAILED(hr = this->src->Stop()))
        err.PostError("a3dsrc::Stop()", hr, "failed pausing sound",__FILE__,__LINE__);
#else
    this->src->Stop();
#endif
}

//
// Resume sound source
//
void a3dsrc::Resume()
{
    HRESULT hr=0;

	// if source was paused, it temporary(!) got PERMANENT-_lifetype,
	// so, if original description tells,that source is BRIEF-type, then set it back
	if(this->_mydesc->_Flags&BRIEF)this->_lifetype=BRIEF;

    DWORD ToLoopOrNot = this->_mydesc->_Flags&LOOPED ? A3D_LOOPED : A3D_SINGLE;
    if(FAILED(hr = this->src->Play(ToLoopOrNot)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("a3dsrc::Play()", hr, "failed playing sound",__FILE__,__LINE__);
#endif
    }
}

//
// Reset sound source
//
void a3dsrc::Reset()const
{
#ifdef DSPL__DEB_VER_STORE_ERRORS
    HRESULT hr=0;
    if(FAILED(hr = this->src->Rewind()))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("a3dsrc::SetCurrentPosition()", hr, "failed setting position to 0",__FILE__,__LINE__);
#endif
    }
#else
    this->src->Rewind();
#endif
}

//
// Adjust volume of sound source
//
void a3dsrc::AdjustVolume(int value)const
{
    float new_volume=((float)value)/10000;

    if(new_volume < 0.f)
		new_volume = 0.f;

    if(new_volume > 1.f)
		new_volume = 1.f;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    HRESULT hr = 0;
    if(FAILED(hr = this->src->SetGain(new_volume)))
        err.PostError("a3dsrc::AdjustVolume()", hr, "failed setting volume",__FILE__,__LINE__);
#else
    this->src->SetGain(new_volume);
#endif
}

//
// Set frequency
//
void a3dsrc::Freq(unsigned int value)const
{
}

//
// Set panning
//
void a3dsrc::Pan(int value)const
{
    float val=value;
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(this->src->SetPanValues(2,&val)))
    {
        err.PostError("a3dsrc::SetPos()", -1, "failed setting position",__FILE__,__LINE__);
    }
#else
    this->src->SetPanValues(2,&val);
#endif
}  

//
// Detect if playing
//
bool a3dsrc::IsPlaying()const
{
    // if buffer is playing
    if(this->src)
    {  
        DWORD dwStatus = 0;
        this->src->GetStatus( &dwStatus );
        return(( dwStatus & A3DSTATUS_PLAYING) != 0 );
    }
    else
    {
        return false;
    }
}

//
// Slow sound volume going down
//
void a3dsrc::SmoothVolumeDecay()
{
    float curr_volume=0;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    //logFile[log_name]("Smoothing down...\n");
    HRESULT hr=0;
    if(FAILED(hr = this->src->GetGain(&curr_volume)))
        err.PostError("a3dsrc::AdjustVolume()", hr, "failed getting volume",__FILE__,__LINE__);
#else
    this->src->GetGain(&curr_volume);
#endif

    float volume_adjust_value=VOLUME_TUNING;

    curr_volume-=volume_adjust_value/10000;

    if(curr_volume<=0.001f)
    {
        this->_exec_source=NULL;
        this->src->Stop();
        return;
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(hr = this->src->SetGain(curr_volume)))
        err.PostError("a3dsrc::AdjustVolume()", hr, "failed setting volume",__FILE__,__LINE__);
#else
    this->src->SetGain(curr_volume);
#endif
}

//
// Inversed to a3dsrc::SmoothVolumeDecay
//
void a3dsrc::SmoothVolumeExpand()
{
    float curr_volume=0;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    //logFile[log_name]("Smoothing up...\n");
    HRESULT hr=0;
    if(FAILED(hr = this->src->GetGain(&curr_volume)))
        err.PostError("a3dsrc::AdjustVolume()", hr, "failed getting volume",__FILE__,__LINE__);
#else
    this->src->GetGain(&curr_volume);
#endif
    float volume_adjust_value=VOLUME_TUNING;
    curr_volume+=volume_adjust_value/10000;
    if(curr_volume>=Volume)
    {
        this->_exec_source=NULL;
        return;
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    if(FAILED(hr = this->src->SetGain(curr_volume)))
        err.PostError("a3dsrc::AdjustVolume()", hr, "failed setting volume",__FILE__,__LINE__);
#else
    this->src->SetGain(curr_volume);
#endif
}

//
// Playing with slow volume growing
//
void a3dsrc::SmoothPlay()
{
	// Expand sound volume
	SRCFUNC func=NULL;
	func=&sdsrc::SmoothVolumeExpand;
	this->SetSourceExec(func);
}
/***************************************************************************

                                 END OF FILE

***************************************************************************/
