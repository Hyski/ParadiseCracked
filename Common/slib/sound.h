/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Sound Engine based on DirectSound, EAX and Aureal
                Including internal source-property-set script for
                manual design of sound picture and basic
                sound memory management
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/
#if 0
#ifndef DSPL__SOUND__ENGINE_H_
#define DSPL__SOUND__ENGINE_H_

namespace sound{

// managing the log
#define DSPL__DEB_VER_STORE_ERRORS
#define DSPL__DEB_VER_SHOW_COMMENT
//#define DSPL__DEBUG_INFO
//#define DSPL__ENABLE_EXTENDED_SRC_SETTINGS

//
//  Default values
//
    enum defaults{
        DEFAULT_VALUE				 = 0, 
		DEFAULT_ID					 = 1, 
		VOLUME_TUNING				 = 160, 
		VOLUME_LOW_LIMIT			 = -10000, 
		VOLUME_HIGH_LIMIT			 = 0,
		IOMP3_SUCCESS				 = 1,
		IOMP3_FAILED				 = 0,
		IOMP3_BUSY					 = 2,
		MAX_WAITING_SOURCES			 = 16,
		MAX_ENUM_DRIVERS			 = 20,
		MAX_SOUND_DEVICES			 = 1,
		MIN_SOUND_DEVICES			 = 0,
		ISA_CARD					 = 0x00000000,
		PCI_CARD					 = 0x00000001,
		MP3_INITIALIZED_SUCCESSFULLY = 0x00000002
    };

//
//  Coordinate system
//
    enum CoordinateSystem{
        LEFT_HANDED, RIGHT_HANDED
    };

//
//  Avalaible libraries
//
    enum Library{
        DirectSound, Aureal, EAX
    };

//
//  Wav file defaults
//
    enum wfdefaults{
        RIFF_FORMATTED_FILE    = 0x46464952, // is equal to hex string 'RIFF'
		WAVE_FORMATTED_FILE    = 0x45564157, // is equal to hex string 'WAVE'
		WAVE_DATA_KEYWORD      = 0x61746164, // is equal to hex string 'data'
        AVG_BYTES_OFFSET	   = 7,
		BLOCK_ALIGN_OFFSET     = 16,
		CHANNELS_OFFSET        = 11,
        SAMPLE_PER_SEC_OFFSET  = 6,
		BITS_PER_SAMPLE_OFFSET = 17,
        FORMAT_TAG_OFFSET      = 10
    };

//
//  Source flags 
//
    enum srcflags{
        LOOPED          = 0x00000001,  
        DISABLE_3D      = 0x00000002,  
        STREAMED        = 0x00000004,    
        STATIC          = 0x00000008,
        S_AFFECT_DIRHF  = 0x00000010,
        S_AFFECT_ROOM   = 0x00000020,
        S_AFFECT_ROOMHF = 0x00000040,
        MPEG3FILE       = 0x00000080,
        PERMANENT       = 0x00000100,
        BRIEF           = 0x00000200,
        LOWPRIORITY     = 0x00000400,
        MEDPRIORITY     = 0x00000800,
        HIGHPRIORITY    = 0x00001000,
		LOWQUALITY	    = 0x00002000,
		MEDQUALITY	    = 0x00004000,
		HIGHQUALITY	    = 0x00008000
    };

//
//  Listener flags 
//
    enum lflags{
        SCALE_DECAY_TIME        = 0x00000001, 
        SCALE_REFLECTIONS       = 0x00000002,  
        SCALE_REFLECTIONS_DELAY = 0x00000004,
        SCALE_REVERB            = 0x00000008,
        SCALE_REVERB_DELAY      = 0x00000010,
        CLIP_DECAY_HF           = 0x00000020
    };

//
//  Listener properties
//
    enum lprops{
		PROP_ROOM,
		PROP_ROOM_HF,
		PROP_REFLECTIONS,
		PROP_REVERB,
		PROP_ENVIRONMENT,
		PROP_PROP_FLAGS,
		PROP_REVERB_DELAY,
		PROP_REFLECTIONS_DELAY,
		PROP_ROOM_ROLLOFF,
		PROP_DECAY_TIME,
		PROP_DECAY_HF_RATIO,
		PROP_ENVIRONMENT_SIZE,
		PROP_ENVIRONMENT_DIFFUSION,
		PROP_AIR_ABSORPTION,
		PROP_ROLLOFF,
		PROP_DOPPLER_FACTOR,
		PROP__FLAGS
	};

//
//  PreDefined environments
//
    enum Environment
    {
        GENERIC, 
		PADDEDCELL, 
		ROOM, 
		BATHROOM, 
		LIVINGROOM,
        STONEROOM, 
		AUDITORIUM, 
		CONCERTHALL, 
		CAVE, 
		ARENA,
        HANGAR, 
		CARPETEDHALLWAY, 
		HALLWAY, 
		STONECORRIDOR,
        ALLEY, 
		FOREST, 
		CITY, 
		MOUNTAINS, 
		QUARRY, 
		PLAIN, 
		PARKINGLOT,
        SEWERPIPE, 
		UNDERWATER, 
		DRUGGED, 
		DIZZY, 
		PSYCHOTIC, 
		END_OF_ENVIRONMENT_LIST
    };

	//
	//  Slot template
	//
    template <class type>
    struct slot{
        bool changed;
        type value;
        slot() : changed(false),
				 value(DEFAULT_VALUE){};
        slot(const slot& sl) : changed(sl.changed),
							   value(sl.value){};

		operator bool()const throw(){ return this->changed; };
    };

	//
	//  Tray-structure is to be used with sddev::Play(...) callings
	//
	struct tray{

		const std::string* _cell1; // is used to point to the description string
		const std::string* _cell2; // is used to point to the filename (may be NULL)

		tray(const std::string* s) : _cell1(s),
									 _cell2(NULL){};

		tray(const std::string* s1,const std::string* s2) : _cell1(s1),
															_cell2(s2) {};

		tray(const tray& tr):_cell1(tr._cell1),
							 _cell2(tr._cell2){}

		const tray& operator=(const tray& tr)
		{
			if(this != &tr)
			{
				this->_cell1 = tr._cell1;
				this->_cell2 = tr._cell2;
			}

			return *this;
		};
	};

	// sound descriptions
    struct srcdsc;

	// listener description
    struct lstdsc;

	// main classes
    class sddev; // sound device
    class sdemit;// sound emitter
    class sdsrc; // sound source

	// Placeboes
    class Placebo;
    class plsrc;

    typedef void (sdsrc::*SRCFUNC)();

	// descriptions
    typedef std::map<std::string,srcdsc> SOURCE_DESCRIPTIONS;
    extern SOURCE_DESCRIPTIONS dscmap;

    extern lstdsc ListenerDefs;

	// script filename
    extern const char* script_filename;

	// objects
    extern sddev* dev;

	// initialization switching libraries
    void Init(HWND,Library);

	// initializtions
    void InitDirectSound(HWND);
    void InitEAX(HWND);
    void InitAureal(HWND);
    void InitPlacebo(HWND);

    void ShutDown();

	// sound interface
    void InitCameraAsListener(const point3& pos,
							  const point3& vel,
							  const point3& front,
							  const point3& up);

    void UpdateCameraAsListener(const point3& pos,
								const point3& vel,
								const point3& front,
								const point3& up);

    // Currently playing music will slowly fade away
    // and new music name will appear 
    void SmoothMusicTransmission(const tray&, const tray&);

	BOOL CALLBACK EnumerateSoundDevices(GUID* guid,
										LPSTR desc,
										LPSTR drv_name,
										VOID* ptr);

	//  Interface for MPEG-3 decoder
	namespace iomp3
	{
		bool InitMP3Pipe();					// Initialize MPEG-3 decoder
		void DisconnectMP3Pipe();			// Close MPEG-3 decoder
		
		void ProcPower(DWORD res);
		void ClearBuffer();

		void VolumeMusic(int volume);		// Set volume for MPEG-3 music
		void AdjustVolumeMusic(int value);	// Adjust volume

		void StopMusic();	// Stop playing music.NOTE: sound source is still in the memory,
							//							MPEG-3 decoder is considered to be busy
		void RemoveMusic();	// Stop playing music.NOTE: sound source will be removed from memory,
							//							MPEG-3 decoder is considered to be free for other sources

		int  PlayMusic(const std::string& infile, sdsrc* source); // Play specified file
		void PauseMusic();										  // Pause music
		void ResumeMusic();										  // Resume paused music

		void KillMusic();										  // Kills music threads
	}
};

// Sound source parameters
struct sound::srcdsc{
    // modes and charasteristics
    slot<unsigned> _mode;   
    slot<unsigned> _quality;
    // values
    slot<unsigned> _freq_value;
    slot<unsigned> _inside_cone_angle;   
    slot<unsigned> _outside_cone_angle;
    slot<unsigned> _source_flags;
    slot<int>      _pan_value;
    slot<int>      _vol_value;
    slot<int>      _cone_outside_volume;   
    // 3D values
    slot<int>      _source_direct;
    slot<int>      _source_directHF;
    slot<int>      _source_room;
    slot<int>      _source_roomHF;
    slot<int>      _source_outside;
    slot<int>      _source_obstruction;
    slot<int>      _source_occlusion;
    slot<float>    _source_obstructionLF;
    slot<float>    _source_absorption;
    slot<float>    _source_rolloff;
    slot<float>    _max_dist;
    slot<float>    _min_dist;
    slot<float>    _priority;
    slot<float>    _source_occlusionLF;
    slot<float>    _source_occlusion_room;
    slot<float>    _posX;
    slot<float>    _posY;
    slot<float>    _posZ;
    
    std::string	   _name; // File name
    DWORD		   _Flags;// Flags 
};

// Listener parameters
struct sound::lstdsc{
    slot<int>     _room;
    slot<int>     _roomHF;
    slot<int>     _reflections;
    slot<int>     _reverb;
    slot<unsigned> _environment;
    slot<unsigned> _flags;
    slot<float>   _reverb_delay;
    slot<float>   _reflections_delay;
    slot<float>   _room_rolloff;
    slot<float>   _decay_time;
    slot<float>   _decay_HFRatio;
    slot<float>   _environment_size;
    slot<float>   _environment_diffusion;
    slot<float>   _air_absorption;
	slot<float>	  _rolloff;
	slot<float>	  _doppler_factor;
    
    DWORD         _lflags;// Flags 
};

//
// classes
//
class sound::sddev{
public:

	sddev():_device_flags(0){};

    virtual ~sddev(){};   

    // initialization
    virtual bool Initialize(HWND)=0;
    // close
    virtual void Close(); 

    // source have owner with id
    virtual HRESULT CreateSound(void* buf_desc,
								void* buf,
								const std::string& _filename,
								const unsigned int& id,
								bool& duplicated)const=0;

    // source haven't owner (interface of aural voice)
    virtual HRESULT CreateSound(void* buf_desc,
								void* buf,
								const std::string& _filename,
								bool& duplicated)const=0;

    // source haven't owner (interface of aural voice)
    virtual HRESULT CreateSound(void* buf_desc,void* buf)const=0;

    // sharing memory among sound buffers in sound card hardware memory...
    virtual HRESULT DuplicateSound(void* src_buf,void* dest_buf)const=0;
    // ... source have owner with id
    virtual HRESULT DuplicateSound(void* buf,
								   const std::string& _filename,
								   const unsigned int& id)const=0;

    // ... source haven't owner (interface of aural voice)
    virtual HRESULT DuplicateSound(void* buf,const std::string& _filename)const=0;

    // purchase source without owner (interface sound)
    virtual sdsrc*  PurchaseSource()const=0;
    // purchase source with owner
    virtual sdsrc*  PurchaseSource(const unsigned int& id)const=0;
    // purchase source without owner (interface sound)
    virtual void    PurchaseSource(sdsrc** source)const=0;
    // purchase source with owner
    virtual void    PurchaseSource(sdsrc** source,const unsigned int& id)const=0;

    // thrash source in case of unusability
    virtual void ThrashSource(sdsrc* src)const;
    // thrash source in case of unusability
    virtual void ThrashSource(const unsigned& id);

    // set coordinate system
    virtual void SetCoordinateSystem(CoordinateSystem system)=0;    

	// set properties
	virtual void SetDeviceProperty(lprops prop,void* valaddr)=0;

    // register emitter
    void Register  (sdemit* emitter);// register through pointer
    void Register  (sdemit& emitter);// register through object
    void UnRegister(const sdemit* emitter);

    // register source
    void RegisterSource(sdsrc* src);

    // play sounds
    virtual void Play  (const std::string& desc_name, const point3& pnt3);
    virtual void Play  (const std::string& desc_name, const point3* new_pos=NULL);
    /**/virtual void Play  (const std::string&, const std::string&, const point3&);
    virtual void Play  (const tray& tr, const point3* new_pos=NULL);
    
    virtual void Stop  (const std::string& desc_name)const;  
	// stops and make all buffer being trashable (see description of sdsrc, lifetype variable)
    virtual void StopEx(const std::string& desc_name);  
    virtual void Pause (const std::string& desc_name)const;
    virtual void Resume(const std::string& desc_name)const;

    virtual void Stop  (const tray& tr)const;  
    virtual void StopEx(const tray& tr);  
    virtual void Pause (const tray& tr)const;
    virtual void Resume(const tray& tr)const;

	virtual float GetSrcAvgPlayingTime(const tray& tr)const;

    virtual void PauseAll ()const;
    virtual void ResumeAll()const;

    // increase/decrease volume for all sources in system
    virtual void GetGlobalVolume   (void* valaddr)const=0;
    virtual void GetMusicVolume    (void* valaddr)const=0;

    virtual void AdjustGlobalVolume(int value)=0;
    virtual void AdjustMusicVolume (int value)=0;

	virtual void AdjustSrcVolume   (const tray& tr,int value);
    
    // volume fluently grows from nothing to set value
    virtual void SmoothPlay       (const tray& tr, const point3* new_pos=NULL)=0;
    virtual void SmoothMusicDecay (const tray& tr, DWORD LiveAfterStop=BRIEF);
    virtual void SmoothMusicExpand(const tray& tr);

    // updating all geometric and kinematic parameters
    // update sources+camera
    virtual void Update(const point3& pos,
						const point3& vel,
						const point3& front,
						const point3& up)=0;
    // update only sources
    virtual void Update()=0;

    virtual void GetPos(point3& pos)const=0; // get listener position
    virtual void GetVel(point3& vel)const=0; // get listener velocity

    virtual void SetPos	   (const point3& pos)=0;                      // set listener position
    virtual void SetVel	   (const point3& vel)=0;                      // set listener velocity
    virtual void BeOriented(const point3& front, const point3& top)=0; // set listener orientation

    // set all geometric parameters
    virtual void SetXYZ(const point3& pos,
						const point3& vel,
						const point3& front,
						const point3& top)=0;

	bool TestDeviceFlags(const DWORD& Flag)const throw(){ return this->_device_flags&Flag; };

private:
	// void operator=: no such kind of operation for sddev
	const sddev& operator=(const sddev&){ return *this; };

protected:
    // to-make-life-easier definitions
    // <emitter id(sid), pointer*>
    typedef std::map<int,sdemit*> REGISTERED_EMITTERS;
    REGISTERED_EMITTERS _emitmap;

    typedef std::multimap<std::string,sound::sdsrc*> OWNEDBY_DEVICE_SOURCES;
    OWNEDBY_DEVICE_SOURCES _devsrcmap;// single source map

	DWORD _device_flags;			  // some flags must be stored (bus type for example)
};

//
// Sound emitter
//
class sound::sdemit{
public:
    explicit sdemit() : sid(NextID++), 
						_pos_changed(true), 
						_vel_changed(true){};
    virtual ~sdemit();

	void operator=(const sdemit&){};

    void Play(const std::string& desc_name);
    void Play(const tray& tr);
    //void Play(const std::string&,const std::string&){};

	// functionality is applied to the specified sound source of emitter
    void Stop(const std::string& desc_name)const;
    void Pause(const std::string& desc_name)const;
    void Resume(const std::string& desc_name)const;

    void Stop(const tray& tr)const;
    void Pause(const tray& tr)const;
    void Resume(const tray& tr)const;

	// functionality is applied to all sound sources of emitter
    void Stop()const;
    void Pause()const;
    void Resume()const;

    void AdjustVolume(int value)const;

    void Freq(unsigned int value)const;
    void Pan (int value)const;

    virtual point3& GetPos()=0;
    virtual point3& GetVel()=0;

    virtual void SetPos(const point3& pos);
    virtual void SetVel(const point3& vel);

    sdsrc* browse(const std::string& desc_name)const;// browse srcmap for source with filename

    unsigned mysid()const{ return this->sid; };

    bool IsChangedPos()const throw(){ return this->_pos_changed; };
    bool IsChangedVel()const throw(){ return this->_vel_changed; };

protected:

    static unsigned int NextID;

    const unsigned int sid; // sid - _S_ound_I_D_, id of emitter in _emitmap
    
    typedef std::map<std::string,sdsrc*> OWNEDBY_EMITTER_SOURCES; // to-make-life-easier definition
    OWNEDBY_EMITTER_SOURCES srcmap;

    bool _pos_changed; // change position flag
    bool _vel_changed; // change speed flag
};

//
// Sound source
//
class sound::sdsrc{
public:

    explicit sdsrc() : _owner_id(0),
					   _mydesc(&default_desc),
					   _lifetype(BRIEF),
					   _exec_source(NULL),
					   _avg_playing_time(0.f),
					   _sid(NextID++),
					   _filename(NULL){};

    explicit sdsrc(unsigned& id) : _owner_id(id),
								   _mydesc(&default_desc),
								   _lifetype(BRIEF),
								   _avg_playing_time(0.f),
								   _sid(NextID++),
								   _exec_source(NULL){};

    virtual ~sdsrc();

    virtual bool Initialize(const srcdsc& desc,const char* ovrfname=NULL)=0;

    virtual void Play()=0;
    virtual void Resume()=0;
    virtual void Pause()=0;
    virtual void SmoothPlay()=0;

    virtual bool IsPlaying() const=0;

    virtual void Stop() const=0;
    virtual void Reset()const=0;

    virtual void AdjustVolume(int value)const=0;

    virtual void Freq  (unsigned value)const=0;
    virtual void Pan   (int value)const=0;

    virtual void SetPos(const point3& pos)=0;
    virtual void SetVel(const point3& vel)=0;

    unsigned int ownerid()const{ return _owner_id; };

	int get_id();

	void SetSourceExec(SRCFUNC execfunc);

    virtual void Execute()=0;

    virtual void* GetBuf()const=0;

	float			   GetAvgPlayingTime()const throw();
	unsigned&		   GetLifeType();
    const srcdsc*	   GetSrcDesc()const; 
	const std::string* GetName();

    virtual void SmoothVolumeDecay()=0;
    virtual void SmoothVolumeExpand()=0;

private:

	const sdsrc& operator=(const sdsrc&){};

protected:

    static unsigned NextID;
	static unsigned in_queue;

	static const srcdsc default_desc;

    const unsigned _owner_id;
    const srcdsc*  _mydesc;

    unsigned	 _sid;			   // sid - _S_ound_I_D_, id of emitter in _emitmap
	unsigned	 _lifetype;		   // type of source

	std::string* _filename;		   // source file name
    SRCFUNC		 _exec_source;	   // current behaviour
	float		 _avg_playing_time;// average playing time in seconds
};

//
// Sound emitter (supposed to be example for sound utters)
//
class emitter : public sound::sdemit{
public:
    emitter() : _pos(0,0,0), 
				_vel(0,0,0){};

    virtual ~emitter() {};

    void Move(const point3& new_pos)
	{ 
		this->_pos.x = new_pos.x; 
		this->_pos.y = new_pos.y; 
		this->_pos.z = new_pos.z; 

		this->_pos_changed = true; 
	};

    void SpeedUp(const point3& new_vel)
	{ 
		this->_vel.x = new_vel.x; 
		this->_vel.y = new_vel.y; 
		this->_vel.z = new_vel.z; 

		this->_vel_changed = true; 
	};

    point3& GetPos();
    point3& GetVel();

private:

    point3 _pos;
	point3 _vel;
};

//
// Register sound emitter by pointer
//
inline void sound::sddev::Register(sdemit* emitter)
{ 
    this->_emitmap[emitter->mysid()]=emitter; 
};

//
// Register sound emitter by reference
//
inline void sound::sddev::Register(sdemit& emitter)
{ 
    this->_emitmap[emitter.mysid()]=&emitter; 
};

//
// Unregister sound emitter by pointer
//
inline void sound::sddev::UnRegister(const sdemit* emitter)
{ 
    this->_emitmap.erase(emitter->mysid()); 
};

//
//
//
inline void sound::sdsrc::SetSourceExec(SRCFUNC execfunc)
{
	this->_exec_source = execfunc;
};

//
//
//
inline float sound::sdsrc::GetAvgPlayingTime()const throw()
{ 
	return this->_avg_playing_time; 
};

//
//
//
inline const sound::srcdsc* sound::sdsrc::GetSrcDesc()const
{ 
	return this->_mydesc; 
}; 

//
//
//
inline unsigned int& sound::sdsrc::GetLifeType()
{ 
	return this->_lifetype; 
};

//
//
//
inline const std::string* sound::sdsrc::GetName()
{ 
	return this->_filename ? this->_filename : &this->_mydesc->_name; 
};

//
//
//
inline int sound::sdsrc::get_id()
{ 
	return this->_sid; 
};

//
//
//
inline sound::sdsrc::~sdsrc()
{ 
	if(this->_filename)
	{ 
		delete _filename; 
		_filename = NULL; 
	} 
};

#endif // DSPL__SOUND__ENGINE_H_ 

/***************************************************************************

                                 END OF FILE

***************************************************************************/


#endif // if 0