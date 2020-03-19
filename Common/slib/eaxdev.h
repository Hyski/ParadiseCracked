/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: EAX Sound device 
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                

#ifndef DSPL__EAXDEV_H_
#define DSPL__EAXDEV_H_

//
// add new classes to the namespace
//
namespace sound{

class eaxdev;
class eaxsrc;

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//																										//
//												EAX Classes Family										//
//																										//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Sound Device is central object of Sound Engine manages sound sources
//
class sound::eaxdev : public sddev{
// friendly functions have permission to create an object of dsdev class
friend void Init(HWND,Library);
friend void InitEAX(HWND);

private:
	// EAX device
    LPDIRECTSOUND           device;
	// EAX listener
    LPDIRECTSOUND3DLISTENER camera;
	// Properties of listener
    LPKSPROPERTYSET         props_set;
	// EAX support variable
    DWORD                   eax_supports;

	// Global level of sound volume
    LONG GlobalVolume;
	// Global level of music
    LONG MusicVolume;
public:
    virtual ~eaxdev();

    // initializations
    bool Initialize(HWND);

	// create a EAX buffer with given properties
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

	// set coordinate system for 3-dimensional sources 
    void SetCoordinateSystem(CoordinateSystem){};    

	// set properties
	void SetDeviceProperty(lprops,void*);

    void SmoothPlay(const tray&, const point3* new_pos=NULL);

    // increase/decrease volume for all sources in system
    void AdjustGlobalVolume(int);
    void GetGlobalVolume   (void* value)const{*static_cast<int*>(value)=GlobalVolume;};  
    void GetMusicVolume    (void* value)const{*static_cast<int*>(value)=MusicVolume;};
    void AdjustMusicVolume (int);

    // updating sources when listener moved
    void Update(const point3& pos,const point3& vel,const point3& front,const point3& up);
	// updating sources if listener didnt move
	// note: not so expensive like previous
    void Update();

	// get current position of listener(camera)
    void GetPos(point3&)const;
	// get current velocity of listener(camera)
    void GetVel(point3&)const;

	// set position for listener(camera)
    void SetPos (const point3&);
	// set velocity for listener(camera)
    void SetVel (const point3&);
	// set orientation vectors for listener(camera)
    void BeOriented(const point3&,const point3&);
	// composition of 3 previous functions
    void SetXYZ (const point3&,const point3&,const point3&,const point3&);

    // Information
    void GiveReport() const;
private:
	// creating sound device manually is forbidden
    explicit eaxdev();
    // utility functions
    bool CreatePropertySet();
    bool QuerySupport(ULONG ulQuery);
    bool CreateEAX();
	// set various EAX parameters
    bool SetAll(LPEAXLISTENERPROPERTIES);
    bool SetRoom(LONG);
    bool SetRoomHF(LONG);
    bool SetRoomRolloff(float);
    bool SetDecayTime(float);
    bool SetDecayHFRatio(float);
    bool SetReflections(LONG);
    bool SetReflectionsDelay(float);
    bool SetReverb(LONG);
    bool SetReverbDelay(float);
    bool SetEAXEnvironment(DWORD);
    bool SetEAXEnvironmentSize(float);
    bool SetEnvironmentDiffusion(float);
    bool SetAirAbsorption(float);
    bool SetScaleDecayTime(bool);
    bool SetClipDecayHF(bool);
    bool SetScaleReflections(bool);
    bool SetScaleReflectionsDelay(bool);
    bool SetScaleReverb(bool);
    bool SetScaleReverbDelay(bool);
    bool SetFlags(DWORD);
	// set various EAX parameters 
    bool GetAll(LPEAXLISTENERPROPERTIES);
    bool GetDecayTime(float*);
    bool GetReflections(long*);
    bool GetReflectionsDelay(float*);
    bool GetReverb(long*);
    bool GetReverbDelay(float*);
    bool SetListenerRolloff(float);
};

//
// Sound source presents concept of single sound in Sound Engine
//
class sound::eaxsrc : public sdsrc{
protected:
    // EAX buffer
    LPDIRECTSOUNDBUFFER   buf;
	// EAX buffer with 3-dimensional interface
    LPDIRECTSOUND3DBUFFER buf3d;
	// EAX property
    LPKSPROPERTYSET       props_set;
    // source volume
    int Volume;

public:
    explicit eaxsrc();
    eaxsrc(const eaxsrc&); // copy constructor
    explicit eaxsrc(unsigned int); 
       
    virtual ~eaxsrc();
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
    bool IsPlaying() const;
	// interface to sound data
    void Stop()  const;
    void Pause();
    void Resume();
    void Reset() const;
	// adjusting properties of sound data
    void AdjustVolume(int)const;
    void Freq  (unsigned int)const;
    void Pan   (int) const;  
	// get pointer to sound buffer
    void* GetBuf() const { return buf;  };
	// execute a state of source
	// note: being updated by sound device in Sound Engine sound source has an ability
	//       for self-modification
    void Execute(){ if(this->_exec_source)(this->*_exec_source)(); };
private:
	// initializing source as MP3-type
    int  Initialize2(const srcdsc&,const char* ovrfname=NULL);
	// utilities for source
    HRESULT RestoreBuffers() const;
    HRESULT FillBuffer() const;
	// set various EAX parameters
    bool SetSourceAll(LPEAXBUFFERPROPERTIES);
    bool SetSourceDirect(LONG );
    bool SetSourceDirectHF(LONG);
    bool SetSourceRoom(LONG);
    bool SetSourceRoomHF(LONG);
    bool SetSourceRolloff(float);
    bool SetSourceOutside(LONG);
    bool SetSourceAbsorption(float);
    bool SetSourceObstruction(LONG);
    bool SetSourceObstructionLF(float);
    bool SetSourceOcclusion(LONG);
    bool SetSourceOcclusionLF(float);
    bool SetSourceOcclusionRoom(float);
    bool SetSourceAffectDirectHF(bool);
    bool SetSourceAffectRoom(bool);
    bool SetSourceAffectRoomHF(bool);
    bool SetSourceFlags(DWORD);
    bool SetPositionX(float);
    bool SetPositionY(float);
    bool SetPositionZ(float);
    bool SetConeOrientationX(float);
    bool SetConeOrientationY(float);
    bool SetConeOrientationZ(float);
    bool SetConeInsideAngle(DWORD);
    bool SetConeOutsideAngle(DWORD);
    bool SetConeOutsideVolume(DWORD);
    bool SetMinDistance(float);
    bool SetMaxDistance(float);
};

#endif //DSPL__EAXDEV_H_

/***************************************************************************

                                 END OF FILE

***************************************************************************/
