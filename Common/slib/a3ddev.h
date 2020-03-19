/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Aureal Sound device 
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                

#ifndef DSPL__A3DDEV_H_
#define DSPL__A3DDEV_H_

//
// add new classes to the namespace
//
namespace sound{

class a3ddev;
class a3dsrc;

};

struct IA3d3;
struct IA3d4;
struct IA3d5;
struct IA3dGeom;
struct IA3dGeom2;
struct IA3dSource;
struct IA3dSource2;
struct IA3dListener;
struct IA3dList;
struct IA3dMaterial;
struct IA3dEnvironment;
struct IA3dPropertySet;
struct IA3dReverb;
struct IA3dReflection;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//																										//
//											Aureal Classes Family										//
//																										//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Sound Device is central object of Sound Engine manages sound sources
//
class sound::a3ddev : public sddev{
// friendly functions have permission to create an object of dsdev class
friend void Init(HWND,Library);
friend void InitAureal(HWND);

private:
	// Aureal device
    IA3d5*        device;
	// Aureal listener
    IA3dListener* camera;
	// Aureal reverb
    IA3dReverb*   reverb;  

	// Global level of sound volume
    float GlobalVolume;
	// Global level of music
    float MusicVolume;

public:
    virtual ~a3ddev(); 

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

	// set coordinate system for 3-dimensional sources 
    void SetCoordinateSystem(CoordinateSystem);    

	// set properties
	void SetDeviceProperty(lprops,void*);
 
    // volume fluently grows from nothing to set value
    void SmoothPlay(const tray&, const point3* new_pos=NULL);

    // increase/decrease volume for all sources in system
    void GetGlobalVolume (void* value)const{*static_cast<float*>(value)=GlobalVolume;};
    void GetMusicVolume  (void* value)const{*static_cast<float*>(value)=MusicVolume;};
    void AdjustGlobalVolume(int value);
    void AdjustMusicVolume (int value);

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
    explicit a3ddev();
	// utitlities for device
    bool BindReverb();
    // set sound environment(reverb effect)
    void SetEnvironment(Environment);
};

//
// Sound source presents concept of single sound in Sound Engine
//
class sound::a3dsrc : public sdsrc{
protected:
    // Aureal interface for source
    IA3dSource2* src;
    // source volume
    float Volume;
public:
    explicit a3dsrc();
    explicit a3dsrc(unsigned int id); 
    a3dsrc(const a3dsrc&); // copy constructor
        
    virtual ~a3dsrc();
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
    void Stop()const;
    void Pause();
    void Resume();
    void Reset() const;
	// adjusting properties of sound data
    void AdjustVolume(int)const;
    void Freq  (unsigned int) const;
    void Pan   (int) const;  
	// get pointer to sound buffer
    void* GetBuf ()const{return src;};
	// execute a state of source
	// note: being updated by sound device in Sound Engine sound source has an ability
	//       for self-modification
    void  Execute()
	{ 
		if(this->_exec_source)
			(this->*_exec_source)(); 
	};
};

#endif //DSPL__A3DDEV_H_

/***************************************************************************

                                 END OF FILE

***************************************************************************/
