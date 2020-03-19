/**************************************************************                 
                                                                                
                             Paradise Cracked
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Direct Sound device
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                

#ifndef DSPL__DSDEV_H_
#define DSPL__DSDEV_H_

//
// add new classes to the namespace
//
namespace sound{

	class dsdev;
	class dssrc;

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//																										//
//										DirectSound Classes Family										//
//																										//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Sound Device is central object of Sound Engine manages sound sources
//
class sound::dsdev : public sddev{

// friendly functions have permission to create an object of dsdev class
friend void Init(HWND,Library);
friend void InitDirectSound(HWND);

public:
    
    virtual ~dsdev(); 

    // initializations
    bool Initialize(HWND);

	// create a DirectSound buffer with given properties
	// note: function is smart enough to create buffers
	//       using information about BUS (ISA/PCI) also
	//       first trying to duplicate buffer to save sound memory resources
    HRESULT CreateSound   (void*,void*,const std::string&,const unsigned int&,bool&)const;
    HRESULT CreateSound   (void*,void*,const std::string&,bool&)const;
    HRESULT CreateSound   (void*,void*)const;

	// duplicate buffer using existing buffer in sound card
	// note: no attempts will be made to create new buffer in sound memory
    HRESULT DuplicateSound(void*,void*)const;
    HRESULT DuplicateSound(void*,const std::string&,const unsigned int&)const;
    HRESULT DuplicateSound(void*,const std::string&)const;

	// get a sound::sdsrc object to be managed by sound device
	// note: one may ask for sound::sdsrc(source) to play sound
    sdsrc*  PurchaseSource()const;
    sdsrc*  PurchaseSource(const unsigned int&)const;
    void    PurchaseSource(sdsrc**)const;
    void    PurchaseSource(sdsrc**,const unsigned int&)const;

	
    void SetCoordinateSystem(CoordinateSystem){};// set coordinate system for 3-dimensional sources 
	void SetDeviceProperty(lprops,void*);		 // set properties

    void SmoothPlay(const tray&, const point3* new_pos=NULL);// volume fluently grows from nothing to value set

    // increase/decrease volume for all sources in system
    void AdjustGlobalVolume(int);

    void GetGlobalVolume(void* value)const{ *static_cast<int*>(value) = this->_global_volume; };
    void GetMusicVolume (void* value)const{ *static_cast<int*>(value) = this->_music_volume;  };

    void AdjustMusicVolume(int);

    // updating sources when listener moved
    void Update(const point3& pos,
				const point3& vel,
				const point3& front,
				const point3& up);
	// updating sources if listener didnt move
	// note: not so expensive like previous one
    void Update();

    void GetPos(point3&)const;// get current position of listener(camera)
	void GetVel(point3&)const;// get current velocity of listener(camera)

    void SetPos (const point3&);// set position for listener(camera)
    void SetVel (const point3&);// set velocity for listener(camera)
	void BeOriented(const point3&,const point3&);// set orientation vectors for listener(camera)
	
    void SetXYZ (const point3&,const point3&,const point3&,const point3&);// composition of 3 previous functions

    void GiveReport()const;// Information

protected:
	// creating sound device manually is forbidden
	explicit dsdev();

    LPDIRECTSOUND           _device;// DirecSound device
	LPDIRECTSOUND3DLISTENER _camera;// DirecSound listener

    int _global_volume;// Global level of sound volume
    int _music_volume; // Global level of music
};

//
// Sound source presents concept of single sound in Sound Engine
//
class sound::dssrc : public sdsrc
{
public:
    explicit dssrc();
    explicit dssrc(unsigned int id);

    dssrc(const dssrc&);

    virtual ~dssrc();

	// initialize sound source
    bool Initialize(const srcdsc&,const char* ovrfname=NULL);
	// set position for sound source
    void SetPos(const point3&);
	// set velocity for sound source
    void SetVel(const point3&);
	// play buffer with sound data
    void Play();
	// sound effects starting/stopping source based on managing volume
    void SmoothVolumeDecay();
    void SmoothVolumeExpand();
    void SmoothPlay();
	// if source is playing...
    bool IsPlaying()const;
	// interface to sound data
    void Stop()  const;
    void Pause();
    void Resume();
    void Reset() const;
	// adjusting properties of sound data
    void  AdjustVolume(int)const;
    void  Freq   (unsigned int) const;
    void  Pan    (int) const;  
	// get pointer to sound buffer
    void* GetBuf ()const{ return this->_buf; };
	// execute a state of source
	// note: being updated by sound device in Sound Engine sound source has an ability
	//       for self-modification
    void  Execute()
	{ 
		if(this->_exec_source)
			(this->*_exec_source)(); 
	};

protected:
	// initializing source as MP3-type
    int Initialize2(const srcdsc&,const char* ovrfname=NULL);

	// utilities for source
    HRESULT RestoreBuffers() const;
    HRESULT FillBuffer() const;
    
    LPDIRECTSOUNDBUFFER   _buf;  // DirectSound buffer
    LPDIRECTSOUND3DBUFFER _buf3d;// DirectSound buffer with 3-dimensional interface
    
    int _volume;// volume of source
};

#endif //DSPL__DSDEV_H_

/***************************************************************************

                                 END OF FILE

***************************************************************************/
