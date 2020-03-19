/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Sound library
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/

#include <windows.h>

#include "..\precomp.h"

#include "sound.h"
#include "eax.h"
#include "dsdev.h"
#include "eaxdev.h"
#include "a3ddev.h"
#include "placebo.h"
#include "..\..\options\options.h"

#if defined(DSPL__DEB_VER_STORE_ERRORS)||defined(DSPL__DEB_VER_SHOW_COMMENT)||defined(DSPL__DEBUG_INFO)
#include "..\..\globals.h"
const char* log_name="sound.log";
#endif

#ifdef DSPL__DEB_VER_STORE_ERRORS
#include "abnormal.h"
Abnormal err; 
#endif

namespace sound{

// descriptions
SOURCE_DESCRIPTIONS dscmap;

// listener description
lstdsc ListenerDefs;

const char* script_filename="sound.spt";

sddev* dev=NULL;

const srcdsc sdsrc::default_desc;

}

//
// Initializations switching libraries
//
void sound::Init(HWND hwnd, Library lib)
{
    bool success=true;

	if(dev)delete dev;dev=NULL;

    switch(lib)
    {
        case DirectSound:
            dev = new dsdev();
            break;
        case Aureal:
            dev = new a3ddev();
            break;
        case EAX:
            dev = new eaxdev();
            break;
        default:
            dev = new Placebo();
    }

    if(!success==dev->Initialize(hwnd)) 
    { 
        delete dev; 
        dev=new Placebo();
    }
}

//
// Initialization DirectSound
//
void sound::InitDirectSound(HWND hwnd)
{
    dev = new dsdev();
    if(!dev->Initialize(hwnd)) 
    { 
        delete dev; 
        dev=new Placebo();
    }
}

//
// Initialization EAX
//
void sound::InitEAX(HWND hwnd)
{
    dev = new eaxdev();
    if(!dev->Initialize(hwnd)) 
    { 
        delete dev; 
        dev=new Placebo();
    }
}

//
// Initialization Aureal
//
void sound::InitAureal(HWND hwnd)
{
    dev = new a3ddev();
    if(!dev->Initialize(hwnd)) 
    { 
        delete dev; 
        dev=new Placebo();
    }
}

//
// Initialization Placebo (fictive sound)
//
void sound::InitPlacebo(HWND hwnd)
{
    dev=new Placebo();
}

//
// Effect of one music fading away and another music appearing
//
void sound::SmoothMusicTransmission(const sound::tray& tr_out, 
									const sound::tray& tr_in)
{
	static unsigned request_num = 1;
	logFile[log_name]("Request #%d from:%s to:%s\n",request_num,tr_out._cell2->c_str(),tr_in._cell2->c_str());
	request_num++;

    dev->SmoothMusicDecay(tr_out); // NOTE: SmoothMusicDecay() will kill source
								   //		even its not playing but waiting for MP3 resource
    dev->SmoothPlay(tr_in);
}

//
// Shutting down
//
void sound::ShutDown()
{							   
    if(dev)
		delete dev;

	dev=NULL;
}

//
// Init camera coordinates
//
void sound::InitCameraAsListener(const point3& pos,
								 const point3& vel,
								 const point3& front,
								 const point3& up)
{
    if(dev)
		dev->SetXYZ(pos,vel,front,up);
}

//
// Updating camera coordinates
//
void sound::UpdateCameraAsListener(const point3& pos,
								   const point3& vel,
								   const point3& front,
								   const point3& up)
{
	// first, be sure that not too frequent calls
	static float  _prev_game_time = 0.f;
	float	       _cur_game_time = Timer::GetSeconds();
	float _min_game_time_interval = 1.f / Options::GetFloat("system.sound.update_freq");

	if((_cur_game_time - _prev_game_time) < _min_game_time_interval) 
		return;

	_prev_game_time = _cur_game_time;

    if(dev)
		dev->SetXYZ(pos,vel,front,up);
}

//
// Enumerating devices
//
BOOL CALLBACK sound::EnumerateSoundDevices(GUID* guid,
										   LPSTR desc,
										   LPSTR drv_name,
										   VOID* ptr)
{
    static GUID  AudioDriverGUIDs[MAX_ENUM_DRIVERS];
    static DWORD dwAudioDriverIndex=0;

    GUID* Temp=NULL;

    if(guid)
    {
        if(dwAudioDriverIndex>=MAX_ENUM_DRIVERS)
			return TRUE;

        Temp=&AudioDriverGUIDs[dwAudioDriverIndex++];
        memcpy(Temp,guid,sizeof(GUID));

#ifdef DSPL__DEB_VER_SHOW_COMMENT
		logFile[log_name]("Enumerating sound devices... found driver:[%s],description:[%s]\n",drv_name,desc);
#endif
    }

    return TRUE;
}

//
// Thrash source
//
void sound::sddev::ThrashSource(sound::sdsrc* source)const
{
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    if(source!=NULL)
    {
		const char* srcname=source->GetName()->c_str();
		logFile[log_name]("...thrashing source=%x,file _name=[%s,id=%d]...",source,srcname,source->get_id());

        delete source;source=NULL;
    }
#ifdef  DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("dsdev::ThrashSource()", -1, "attempt to delete NULL source",__FILE__,__LINE__);
#endif
    logFile[log_name]("...OK.\n");
#else
    if(source!=NULL)delete source;source=NULL;
#endif
};

//
// Thrash source
//
void sound::sddev::ThrashSource(const unsigned& id)
{
    OWNEDBY_DEVICE_SOURCES::iterator iter=this->_devsrcmap.begin();

	while(iter != this->_devsrcmap.end())
	{
		if(iter->second->get_id()==id)
		{
			delete iter->second;
			this->_devsrcmap.erase(iter);

			break;
		}
		iter++;
	}
};

//
// Register sound source
//
void sound::sddev::RegisterSource(sdsrc* source)
{ 
    typedef OWNEDBY_DEVICE_SOURCES::value_type Pairs;

    const std::string fname=*source->GetName();

    Pairs new_pair(fname,source);

    this->_devsrcmap.insert(new_pair);
};

//
// Close sound device
//
void sound::sddev::Close()
{
    REGISTERED_EMITTERS::iterator p;
    OWNEDBY_DEVICE_SOURCES::iterator p2;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    logFile[log_name]("Deleting sound emitters...\n");
#endif
    // empty list of emitter
    for(p=this->_emitmap.begin();p!=this->_emitmap.end();++p)
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        if(!p->second)err.PostError("sddev::close()",-1,"attempt to use missed emitter in map",__FILE__,__LINE__);
#endif
        if(p->second) 
		{
            delete p->second; 
			p->second=NULL; 
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
        else
            err.PostError("sddev::close()",-1,"attempt to use missed emitter in map",__FILE__,__LINE__);
#endif
        this->_emitmap.erase(p->first);
    }

#ifdef DSPL__DEB_VER_STORE_ERRORS
    logFile[log_name]("Deleting sound sources...\n");
#endif
    // empty list of system sources
    for(p2=this->_devsrcmap.begin();p2!=this->_devsrcmap.end();++p2)
    {
        this->ThrashSource(p2->second);
        this->_devsrcmap.erase(p2->first);
    }
}

//<o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o>
//
//						Functions			::	PLAY	::				Family
//
//<o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o>

//
// Playing sound
//
void sound::sddev::Play(const tray& tr, const point3* new_pos)
{
	SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(*tr._cell1);
	if(p!=sound::dscmap.end())
	{
		bool permit_creating_new_source=true;
		const srcdsc& description=p->second;
		const std::string& filename=tr._cell2 ? *tr._cell2 : p->second._name;
		
		if(p->second._Flags&PERMANENT)
		{
			OWNEDBY_DEVICE_SOURCES::iterator iter=this->_devsrcmap.find(filename);
			if(iter!=this->_devsrcmap.end())
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				logFile[log_name]("Found the source with the same key in Coomon-List-of Sources.");
#endif
				sound::sdsrc* source=iter->second;
				permit_creating_new_source=false;
				if(!source->IsPlaying())source->Play();
#ifdef DSPL__DEB_VER_STORE_ERRORS
				else
					logFile[log_name]("Source is currently playing.Skipping play....\n");
#endif
			}
		}
		
		if(permit_creating_new_source)
		{
			sdsrc* source=NULL;
			source=sound::dev->PurchaseSource();
			
			bool src_initialized=false;
			if(tr._cell2)
				src_initialized=source->Initialize(description,filename.c_str());
			else
				src_initialized=source->Initialize(description);
			
			if(src_initialized)
			{
				if(new_pos)
					source->SetPos(*new_pos);

				source->Play();

				// register even if there is error in 'sdsrc::Play'
				// 'coz source is not playing and has BRIEF type (which is given to every source as default)
				typedef OWNEDBY_DEVICE_SOURCES::value_type Pairs;
				Pairs new_pair(filename,source);
				this->_devsrcmap.insert(new_pair);
			}
			else
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("sdsrc::Initialize()", -1, "failed in dsdev::Play()",__FILE__,__LINE__);
#endif
				this->ThrashSource(source);
			}
		}
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("sddev::Play()", -1, "description was not found:"+*tr._cell1,__FILE__,__LINE__);
#endif
}

//
// Playing sound (possibly in 3d location)
//
void sound::sddev::Play(const std::string& _name, const point3* new_pos)
{
    SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(_name);
    if(p!=sound::dscmap.end())
    {
        const srcdsc& description   = p->second;
        const std::string& filename = p->second._name;

		bool permit_creating_new_source=true;

		if(description._Flags&PERMANENT)
		{
			OWNEDBY_DEVICE_SOURCES::iterator iter=this->_devsrcmap.find(filename);
			if(iter!=this->_devsrcmap.end())
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				logFile[log_name]("Found the source with the same key in Coomon-List-of Sources.");
#endif
				sound::sdsrc* source=iter->second;
				permit_creating_new_source=false;
				if(!source->IsPlaying())source->Play();
#ifdef DSPL__DEB_VER_STORE_ERRORS
				else
					logFile[log_name]("Source is currently playing.Skipping play....\n");
#endif
			}
		}

		if(permit_creating_new_source)
		{
			sdsrc* source=NULL;
			source=sound::dev->PurchaseSource();
			
			if(source->Initialize(description))
			{
				if(new_pos)source->SetPos(*new_pos);
				source->Play();
				// register even if there is error in 'sdsrc::Play'
				// 'coz source is not playing and has BRIEF type (which is given to every source as default)
				typedef OWNEDBY_DEVICE_SOURCES::value_type Pairs;
				Pairs new_pair(filename,source);
				this->_devsrcmap.insert(new_pair);
			}
			else
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("sdsrc::Initialize()", -1, "failed in sddev::Play()",__FILE__,__LINE__);
#endif
				this->ThrashSource(source);
			}
		}
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("sddev::Play()", -1, "description was not found:"+_name,__FILE__,__LINE__);
#endif
}

//
// Playing sound (possibly in 3d location)
//
void sound::sddev::Play(const std::string& _name, const point3& new_pos)
{
    SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(_name);
    if(p!=sound::dscmap.end())
    {
        const srcdsc& description   = p->second;
        const std::string& filename = p->second._name;

		bool permit_creating_new_source=true;

		if(description._Flags&PERMANENT)
		{
			OWNEDBY_DEVICE_SOURCES::iterator iter=this->_devsrcmap.find(filename);
			if(iter!=this->_devsrcmap.end())
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				logFile[log_name]("Found the source with the same key in Coomon-List-of Sources.");
#endif
				sound::sdsrc* source=iter->second;
				permit_creating_new_source=false;
				if(!source->IsPlaying())source->Play();
#ifdef DSPL__DEB_VER_STORE_ERRORS
				else
					logFile[log_name]("Source is currently playing.Skipping play....\n");
#endif
			}
		}

		if(permit_creating_new_source)
		{
			sdsrc* source=NULL;
			source=sound::dev->PurchaseSource();
			
			if(source->Initialize(description))
			{
				source->SetPos(new_pos);
				source->Play();
				// register even if there is error in 'sdsrc::Play'
				// 'coz source is not playing and has BRIEF type (which is given to every source as default)
				typedef OWNEDBY_DEVICE_SOURCES::value_type Pairs;
				Pairs new_pair(filename,source);
				this->_devsrcmap.insert(new_pair);
			}
			else
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("sdsrc::Initialize()", -1, "failed in sddev::Play()",__FILE__,__LINE__);
				// initialization has failed:release memory
				logFile[log_name]("now thrashing...\n");
#endif
				this->ThrashSource(source);
			}
		}
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("sddev::Play()", -1, "description was not found:"+_name,__FILE__,__LINE__);
#endif
}

//
// Playing sound, overwriting filename from description by 'filename'(possibly in 3d location)
//
void sound::sddev::Play(const std::string& _name, const std::string& filename, const point3& new_pos)
{
    SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(_name);
    if(p!=sound::dscmap.end())
    {
		bool permit_creating_new_source=true;
		const srcdsc& description=p->second;

		if(p->second._Flags&PERMANENT)
		{
			OWNEDBY_DEVICE_SOURCES::iterator iter=this->_devsrcmap.find(filename);
			if(iter!=this->_devsrcmap.end())
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				logFile[log_name]("Found the source with the same key in Coomon-List-of Sources.");
#endif
				sound::sdsrc* source=iter->second;
				permit_creating_new_source=false;
				if(!source->IsPlaying())source->Play();
#ifdef DSPL__DEB_VER_STORE_ERRORS
				else
					logFile[log_name]("Source is currently playing.Skipping play....\n");
#endif
			}
		}

		if(permit_creating_new_source)
		{
			sdsrc* source=NULL;
			source=sound::dev->PurchaseSource();
			
			if(source->Initialize(description,filename.c_str()))
			{
				source->SetPos(new_pos);
				source->Play();
				// register even if there is error in 'sdsrc::Play'
				// 'coz source is not playing and has BRIEF type (which is given to every source as default)
				typedef OWNEDBY_DEVICE_SOURCES::value_type Pairs;
				Pairs new_pair(filename,source);
				this->_devsrcmap.insert(new_pair);
			}
			else
			{
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("sdsrc::Initialize()", -1, "failed in sddev::Play()",__FILE__,__LINE__);
				// initialization has failed:release memory
				logFile[log_name]("now thrashing...\n");
#endif
				this->ThrashSource(source);
			}
		}
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("sddev::Play()", -1, "description was not found:"+_name,__FILE__,__LINE__);
#endif
}

//<o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o>
//
//					Functions			::MUSIC EFFECTS::			Family
//
//<o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o>

//
// Stop sound (effect of slow sound decay)
//
void sound::sddev::SmoothMusicDecay(const tray& tr,DWORD LiveAfterStop)
{
	SOURCE_DESCRIPTIONS::const_iterator sd=sound::dscmap.find(*(tr._cell1));
	if(sd!=sound::dscmap.end())
	{
		const std::string& filename = tr._cell2 ? *(tr._cell2) : sd->second._name;
		
		OWNEDBY_DEVICE_SOURCES::const_iterator cis=this->_devsrcmap.find(filename);
		if(cis!=this->_devsrcmap.end())
		{
			SRCFUNC func = &sdsrc::SmoothVolumeDecay;
			
			cis->second->SetSourceExec(func);
			
			unsigned int& lifetype = cis->second->GetLifeType();
			lifetype = LiveAfterStop;
			
			if(cis->second->IsPlaying())
				logFile[log_name]("Start decaying the playing id=%d,lifetype=%d\n",cis->second->get_id(),cis->second->GetLifeType());
			else
			{
				cis->second->SetSourceExec(NULL);
				logFile[log_name]("Start decaying the non-playing id=%d,lifetype=%d\n",cis->second->get_id(),cis->second->GetLifeType());
			}
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sddev::SmoothMusicDecay()", -1, "failed start decay_smoothing file:"+filename,__FILE__,__LINE__);
#endif
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("sddev::SmoothMusicDecay()", -1, "description for decay_smoothing wasn't found:"+*(tr._cell1),__FILE__,__LINE__);
#endif
};

//
// Slow volume growth
//
void sound::sddev::SmoothMusicExpand(const tray& tr)
{
	SOURCE_DESCRIPTIONS::const_iterator sd=sound::dscmap.find(*(tr._cell1));
	if(sd!=sound::dscmap.end())
	{
		const std::string& filename=tr._cell2 ? *(tr._cell2) : sd->second._name;
		OWNEDBY_DEVICE_SOURCES::const_iterator cis=this->_devsrcmap.find(filename);
		if(cis!=this->_devsrcmap.end())
		{
			SRCFUNC func=&sdsrc::SmoothVolumeExpand;
			cis->second->SetSourceExec(func);
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sddev::SmoothMusicExpand()", -1, "failed start expand_smoothing",__FILE__,__LINE__);
#endif
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("sddev::SmoothMusicExpand()", -1, "description for expand_smoothing wasn't found:"+*(tr._cell1),__FILE__,__LINE__);
#endif
};

//<o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o>
//
//			Functions::			MANAGING PLAYING SOUNDS::			Family
//
//<o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o><o>

//
// Stop sound
//
void sound::sddev::Stop(const std::string& _name)const
{
    SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(_name);
    if(p!=sound::dscmap.end())
    {
        const srcdsc& description   = p->second;
        const std::string& filename = p->second._name;

		OWNEDBY_DEVICE_SOURCES::const_iterator ps=this->_devsrcmap.find(filename);
		// if it does not exist
		if(ps!=this->_devsrcmap.end())
		{   
			ps->second->Stop(); 
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sddev::Stop()", -1, "source to stop hasn't been found",__FILE__,__LINE__);
#endif
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("sddev::Stop()", -1, "description was not found:"+_name,__FILE__,__LINE__);
#endif
}  

//
// Stop sound
//
void sound::sddev::Stop(const tray& tr)const
{
	SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(*tr._cell1);
	if(p!=sound::dscmap.end())
	{
		const srcdsc& description   = p->second;
		const std::string& filename = tr._cell2 ? *tr._cell2 : p->second._name;
		
		OWNEDBY_DEVICE_SOURCES::const_iterator ps=this->_devsrcmap.find(filename);
		// if it does not exist
		if(ps!=this->_devsrcmap.end())
		{   
			ps->second->Stop(); 
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sddev::Stop()", -1, "source to stop hasn't been found",__FILE__,__LINE__);
#endif
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("sddev::Stop()", -1, "description was not found:"+*tr._cell1,__FILE__,__LINE__);
#endif
}  

//
// Stop sound(extended)
//
void sound::sddev::StopEx(const std::string& _name)
{
    SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(_name);
    if(p!=sound::dscmap.end())
    {
        const srcdsc& description   = p->second;
        const std::string& filename = p->second._name;

		OWNEDBY_DEVICE_SOURCES::const_iterator ps=this->_devsrcmap.find(filename);
		// if it does not exist
		if(ps!=this->_devsrcmap.end())
		{   
			sdsrc* source=ps->second;
			unsigned int& lifetype=source->GetLifeType();
			lifetype=BRIEF;

			source->Stop();
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sddev::StopEx()", -1, "source to be stopped hasn't been found",__FILE__,__LINE__);
#endif
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("sddev::StopEx()", -1, "description was not found:"+_name,__FILE__,__LINE__);
#endif
}  

//
// Stop sound(extended)
//
void sound::sddev::StopEx(const tray& tr)
{
	SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(*(tr._cell1));
	if(p!=sound::dscmap.end())
	{
		const srcdsc& description   = p->second;
		const std::string& filename = tr._cell2 ? *(tr._cell2) : p->second._name;
		
		OWNEDBY_DEVICE_SOURCES::const_iterator ps=this->_devsrcmap.find(filename);
		// if it does not exist
		if(ps!=this->_devsrcmap.end())
		{   
			sdsrc* source=ps->second;
			unsigned int& lifetype=source->GetLifeType();
			lifetype=BRIEF;
			
			source->Stop();
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sddev::StopEx()", -1, "source to be stopped hasn't been found",__FILE__,__LINE__);
#endif
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("sddev::StopEx()", -1, "description was not found:"+*tr._cell1,__FILE__,__LINE__);
#endif
}  
 
//
// Pause sound
//
void sound::sddev::Pause(const std::string& _name)const
{
    SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(_name);
    if(p!=sound::dscmap.end())
    {
        const srcdsc& description   = p->second;
        const std::string& filename = p->second._name;

		OWNEDBY_DEVICE_SOURCES::const_iterator ps=this->_devsrcmap.find(filename);
		// if it does not exist
		if(ps!=this->_devsrcmap.end())
		{
			ps->second->Pause(); 
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sddev::Pause()", -1, "emitter to pause hasn't been found",__FILE__,__LINE__);
#endif
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("sddev::Pause()", -1, "description was not found:"+_name,__FILE__,__LINE__);
#endif
}

//
// Pause sound
//
void sound::sddev::Pause(const tray& tr)const
{
	SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(*tr._cell1);

	if(p!=sound::dscmap.end())
	{
		const srcdsc& description   = p->second;
		const std::string& filename = tr._cell2 ? *tr._cell2 : p->second._name;
		
		OWNEDBY_DEVICE_SOURCES::const_iterator ps=this->_devsrcmap.find(filename);
		// if it does not exist
		if(ps!=this->_devsrcmap.end())
		{
			ps->second->Pause(); 
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sddev::Pause()", -1, "emitter to pause hasn't been found",__FILE__,__LINE__);
#endif
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("sddev::Pause()", -1, "description was not found:"+*tr._cell1,__FILE__,__LINE__);
#endif
}

//
// Pause all sounds
//
void sound::sddev::PauseAll()const
{
    REGISTERED_EMITTERS::const_iterator emit_iter;
    OWNEDBY_DEVICE_SOURCES::const_iterator src_iter;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    logFile[log_name]("Pausing all sound sources and sound emitters...\n");
#endif
    for(emit_iter=this->_emitmap.begin();emit_iter!=this->_emitmap.end();++emit_iter)
    {
		emit_iter->second->Pause();
    }

    for(src_iter=this->_devsrcmap.begin();src_iter!=this->_devsrcmap.end();++src_iter)
    {
		src_iter->second->Pause();
    }
}

//
// Resume sound
//
void sound::sddev::Resume(const std::string& _name)const
{
    SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(_name);
    if(p!=sound::dscmap.end())
    {
        const srcdsc& description   = p->second;
        const std::string& filename = p->second._name;

		OWNEDBY_DEVICE_SOURCES::const_iterator ps=this->_devsrcmap.find(filename);
		// if it does not exist
		if(ps!=this->_devsrcmap.end())
		{
			ps->second->Resume(); 
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sddev::Resume()", -1, "emitter to resume hasn't been found",__FILE__,__LINE__);
#endif
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("sddev::Resume()", -1, "description was not found:"+_name,__FILE__,__LINE__);
#endif
}

//
// Resume sound
//
void sound::sddev::Resume(const tray& tr)const
{
	SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(*tr._cell1);
	if(p!=sound::dscmap.end())
	{
		const srcdsc& description   = p->second;
		const std::string& filename = tr._cell2 ? *tr._cell2 : p->second._name;
		
		OWNEDBY_DEVICE_SOURCES::const_iterator ps=this->_devsrcmap.find(filename);
		// if it does not exist
		if(ps!=this->_devsrcmap.end())
		{
			ps->second->Resume(); 
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sddev::Resume()", -1, "emitter to resume hasn't been found",__FILE__,__LINE__);
#endif
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("sddev::Resume()", -1, "description was not found:"+*tr._cell1,__FILE__,__LINE__);
#endif
}

//
// Resume all sounds
//
void sound::sddev::ResumeAll()const
{
    REGISTERED_EMITTERS::const_iterator emit_iter;
    OWNEDBY_DEVICE_SOURCES::const_iterator src_iter;

#ifdef DSPL__DEB_VER_STORE_ERRORS
    logFile[log_name]("Resuming all sound sources and sound emitters...\n");
#endif
    for(emit_iter=this->_emitmap.begin();emit_iter!=this->_emitmap.end();++emit_iter)
    {
		emit_iter->second->Resume();
    }

    for(src_iter=this->_devsrcmap.begin();src_iter!=this->_devsrcmap.end();++src_iter)
    {
		src_iter->second->Resume();
    }
}

//
// Return playing time of the source
//
float sound::sddev::GetSrcAvgPlayingTime(const tray& tr)const
{
	// search for description
	SOURCE_DESCRIPTIONS::const_iterator p = sound::dscmap.find(*tr._cell1);
	if(p!=sound::dscmap.end())
	{
		const srcdsc& description   = p->second;
		const std::string& filename = tr._cell2 ? *tr._cell2 : p->second._name;
		
		// search for source
		OWNEDBY_DEVICE_SOURCES::const_iterator ps = this->_devsrcmap.find(filename);
		// if it does not exist
		if(ps!=this->_devsrcmap.end())
		{   
			return ps->second->GetAvgPlayingTime(); 
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sddev::GetSrcAvgPlayingTime()", -1, "source to stop hasn't been found",__FILE__,__LINE__);
#endif
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("sddev::GetSrcAvgPlayingTime()", -1, "description was not found:"+*tr._cell1,__FILE__,__LINE__);
#endif

	return 0.f;
}

//
// Adjust volume for specified source
//
void sound::sddev::AdjustSrcVolume(const tray& tr,int value)
{
	// search for description
	SOURCE_DESCRIPTIONS::const_iterator p = sound::dscmap.find(*tr._cell1);
	if(p!=sound::dscmap.end())
	{
		const srcdsc& description   = p->second;
		const std::string& filename = tr._cell2 ? *tr._cell2 : p->second._name;
		
		// search for source
		OWNEDBY_DEVICE_SOURCES::const_iterator ps = this->_devsrcmap.find(filename);
		// if it does not exist
		if(ps!=this->_devsrcmap.end())
			ps->second->AdjustVolume(value); 

#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sddev::GetSrcAvgPlayingTime()", -1, "source to stop hasn't been found",__FILE__,__LINE__);
#endif
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("sddev::GetSrcAvgPlayingTime()", -1, "description was not found:"+*tr._cell1,__FILE__,__LINE__);
#endif
}

/***************************************************************************
****************************************************************************

                            sdemit realization

***************************************************************************
***************************************************************************/
// NOTE:
// emitter is testing class and should be deleted
// in final version
point3& emitter::GetPos()
{ 
	return this->_pos; 
};

point3& emitter::GetVel()
{ 
	return this->_vel; 
};

unsigned int sound::sdsrc::NextID=0;

unsigned int sound::sdemit::NextID=DEFAULT_ID;
unsigned sound::sdsrc::in_queue = 0;

//sound::sdemit::sdemit():sid(NextID++) {logFile[log_name]("Creating sound emitter [sid=%d]...\n",sid);};

//
// Trying to find source with _name 'fname' among emitter's sources
//
sound::sdsrc* sound::sdemit::browse(const std::string& fname) const 
{
    OWNEDBY_EMITTER_SOURCES::const_iterator ci=srcmap.find(fname);

    if(ci!=this->srcmap.end())
        return ci->second;
    // BE CAREFUL 
    return NULL;
}

//
// Destructing emitter
//
sound::sdemit::~sdemit() 
{
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("destructing emitter[sid=%d]...\n",sid);
#endif
    OWNEDBY_EMITTER_SOURCES::iterator p;

    for(p=this->srcmap.begin();p!=this->srcmap.end();++p)
    {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        logFile[log_name]("...destructing buffer [%s][owner_id=%d]...", p->first.c_str(),p->second->ownerid());
        if(p->second)delete p->second;
        logFile[log_name]("...OK.\n");
#else
        if(p->second)delete p->second;
#endif
    }

	//
	// unregister from sound device
	//
	sound::dev->UnRegister(this);
}

//
// Playing source with description 'dsc'
//
void sound::sdemit::Play(const std::string& dsc)
{
    SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(dsc);
    if(p!=sound::dscmap.end())
    {
        const srcdsc& description   = p->second;
        const std::string& filename = p->second._name;

        OWNEDBY_EMITTER_SOURCES::const_iterator ps=this->srcmap.find(filename);
        // if it does not exist
        if(ps==this->srcmap.end())
        {
            sdsrc* source=NULL;
            source=sound::dev->PurchaseSource(sid);

			if(source->Initialize(description))
			{
				source->Play();
				this->srcmap[filename]=source; // create new 
			}
			else
			{
				sound::dev->ThrashSource(source);
#ifdef DSPL__DEB_VER_STORE_ERRORS
				err.PostError("sdsrc::Initialize()", -1, "failed in sdemit::Play()",__FILE__,__LINE__);
#endif
			}
        }
        else
        {
            //ps->second->Reset();
            ps->second->Stop();
            ps->second->Play();
        }
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("sdemit::Play()", -1, "description was not found:"+dsc,__FILE__,__LINE__);
#endif
}

//
// Playing source with description 'dsc' using 'filename' instead of filename from description
//
void sound::sdemit::Play(const tray& tr)
{
	if(tr._cell1)
	{
		SOURCE_DESCRIPTIONS::const_iterator p=sound::dscmap.find(*tr._cell1);
		if(p!=sound::dscmap.end())
		{
			const srcdsc& description=p->second;
			const std::string& filename=tr._cell2 ? *tr._cell2 : p->second._name;
			
			OWNEDBY_EMITTER_SOURCES::const_iterator ps=this->srcmap.find(filename);
			// if it does not exist
			if(ps==this->srcmap.end())
			{
				sdsrc* source=NULL;
				source=sound::dev->PurchaseSource(sid);
				bool src_initialized=false;
				if(tr._cell2)
					src_initialized=source->Initialize(description,filename.c_str());
				else
					src_initialized=source->Initialize(description);

				if(src_initialized)
				{
					source->Play();
					this->srcmap[filename]=source; // create new 
				}
				else
				{
					sound::dev->ThrashSource(source);
#ifdef DSPL__DEB_VER_STORE_ERRORS
					err.PostError("sdsrc::Initialize()", -1, "failed in sdemit::Play()",__FILE__,__LINE__);
#endif
				}
			}
			else
			{
				//ps->second->Reset();
				ps->second->Stop();
				ps->second->Play();
			}
		}
#ifdef DSPL__DEB_VER_STORE_ERRORS
		else
			err.PostError("sdemit::Play()", -1, "description was not found:"+*tr._cell1,__FILE__,__LINE__);
#endif
	}
#ifdef DSPL__DEB_VER_STORE_ERRORS
	else
		err.PostError("sdemit::Play()", -1, "description was not found",__FILE__,__LINE__);
#endif
}

//
// Stop playing source with description 'dsc'
//
void sound::sdemit::Stop(const std::string& dsc)const
{
    SOURCE_DESCRIPTIONS::const_iterator ci=sound::dscmap.find(dsc);
    if(ci!=sound::dscmap.end())
    {
        const std::string& filename=ci->second._name;
        OWNEDBY_EMITTER_SOURCES::const_iterator p=this->srcmap.find(filename);
        
        if(p!=this->srcmap.end())
        {
            p->second->Stop();
        }
    }
}

//
// Pause playing source with description 'dsc'
//
void sound::sdemit::Pause(const std::string& dsc)const
{
    SOURCE_DESCRIPTIONS::const_iterator ci=sound::dscmap.find(dsc);
    if(ci!=sound::dscmap.end())
    {
        const std::string& filename=ci->second._name;
        OWNEDBY_EMITTER_SOURCES::const_iterator p=this->srcmap.find(filename);
        
        if(p!=this->srcmap.end())
        {
            p->second->Pause();
        }
    }
}

//
// Resume source with description 'dsc'
//
void sound::sdemit::Resume(const std::string& dsc)const
{
    SOURCE_DESCRIPTIONS::const_iterator ci=sound::dscmap.find(dsc);
    if(ci!=sound::dscmap.end())
    {
        const std::string& filename=ci->second._name;
        OWNEDBY_EMITTER_SOURCES::const_iterator p=this->srcmap.find(filename);
        
        if(p!=this->srcmap.end())
        {
            p->second->Resume();
        }
    }
}

//
// Stop playing source with description 'dsc'
//
void sound::sdemit::Stop(const tray& tr)const
{
    SOURCE_DESCRIPTIONS::const_iterator ci=sound::dscmap.find(*tr._cell1);
    if(ci!=sound::dscmap.end())
    {
        const std::string& filename=tr._cell2 ? *tr._cell2 : ci->second._name;
        OWNEDBY_EMITTER_SOURCES::const_iterator p=this->srcmap.find(filename);
        
        if(p!=this->srcmap.end())
        {
            p->second->Stop();
        }
    }
}

//
// Pause playing source with description 'dsc'
//
void sound::sdemit::Pause(const tray& tr)const
{
    SOURCE_DESCRIPTIONS::const_iterator ci=sound::dscmap.find(*tr._cell1);
    if(ci!=sound::dscmap.end())
    {
        const std::string& filename=tr._cell2 ? *tr._cell2 : ci->second._name;
        OWNEDBY_EMITTER_SOURCES::const_iterator p=this->srcmap.find(filename);
        
        if(p!=this->srcmap.end())
        {
            p->second->Pause();
        }
    }
}

//
// Resume source with description 'dsc'
//
void sound::sdemit::Resume(const tray& tr)const
{
    SOURCE_DESCRIPTIONS::const_iterator ci=sound::dscmap.find(*tr._cell1);
    if(ci!=sound::dscmap.end())
    {
        const std::string& filename=tr._cell2 ? *tr._cell2 : ci->second._name;
        OWNEDBY_EMITTER_SOURCES::const_iterator p=this->srcmap.find(filename);
        
        if(p!=this->srcmap.end())
        {
            p->second->Resume();
        }
    }
}

//
// Stop all playing sources
//
void sound::sdemit::Stop()const
{
    OWNEDBY_EMITTER_SOURCES::const_iterator p;

    // locate the buffer
    for(p=this->srcmap.begin();p!=this->srcmap.end();++p)
    {
        // stop id buffer
        p->second->Stop();
    }
}

//
// Pause all playing sources
//
void sound::sdemit::Pause()const
{
    OWNEDBY_EMITTER_SOURCES::const_iterator p;

    // locate the buffer
    for(p=this->srcmap.begin();p!=this->srcmap.end();++p)
    {
        // stop id buffer
        p->second->Pause();
    }
}

//
// Resume all sources
//
void sound::sdemit::Resume()const
{
    OWNEDBY_EMITTER_SOURCES::const_iterator p;

    // locate the buffer
    for(p=this->srcmap.begin();p!=this->srcmap.end();++p)
    {
        // stop id buffer
        p->second->Resume();
    }
}

//
// Adjust volume for all sources from emitter list
//
void sound::sdemit::AdjustVolume(int value)const
{
    OWNEDBY_EMITTER_SOURCES::const_iterator p;

    // locate the buffer
    for(p=this->srcmap.begin();p!=this->srcmap.end();++p)
    {
        p->second->AdjustVolume(value);
    }
}

//
// Adjust frequency for all sources from emitter list
//
void sound::sdemit::Freq(unsigned int value)const
{
    OWNEDBY_EMITTER_SOURCES::const_iterator p;

    // locate the buffer
    for(p=this->srcmap.begin();p!=this->srcmap.end();++p)
    {
        p->second->Freq(value);
    }
}

//
// Adjust panning for all sources from emitter list
//
void sound::sdemit::Pan(int value)const
{
    OWNEDBY_EMITTER_SOURCES::const_iterator p;

    // locate the buffer
    for(p=this->srcmap.begin();p!=this->srcmap.end();++p)
    {
        p->second->Pan(value);
    }
}  

//
// set emitter position 
//
void sound::sdemit::SetPos(const point3& pos)
{
    OWNEDBY_EMITTER_SOURCES::const_iterator p;

    this->_pos_changed=false;
    // locate the buffer
    for(p=this->srcmap.begin();p!=this->srcmap.end();++p)
    {
        // stop id buffer
        p->second->SetPos(pos);
    }
}

//
//set emitter velocity
//
void sound::sdemit::SetVel(const point3& vel)
{
    OWNEDBY_EMITTER_SOURCES::const_iterator p;

    this->_vel_changed=false;
    // locate the buffer
    for(p=this->srcmap.begin();p!=this->srcmap.end();++p)
    {
        // stop id buffer
        p->second->SetVel(vel);
    }
}

/***************************************************************************

                                 END OF FILE

***************************************************************************/
