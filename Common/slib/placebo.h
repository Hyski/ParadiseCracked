/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Sound Engine, NextStep
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                
#ifndef DSPL__PLACEBO_H_
#define DSPL__PLACEBO_H_

///////////////////////////////////////////////// Placebo
class sound::Placebo  : public sddev{

friend void Init(HWND,Library);
friend void InitPlacebo(HWND);
friend void InitDirectSound(HWND);
friend void InitEAX(HWND);
friend void InitAureal(HWND);

public:
    virtual ~Placebo(); 

    // initializations
    bool Initialize(HWND){ return true; };
    void Close(){};

    void Register(sdemit*) {};
    void Register(sdemit&) {};
    void UnRegister(const sdemit*) {};

    void RegisterSource(sdsrc*){};

    HRESULT CreateSound   (void*,void*,const std::string&,const unsigned int&,bool&) const { return HRESULT(0); };
    HRESULT DuplicateSound(void*,void*)const { return HRESULT(0); };
    HRESULT CreateSound   (void*,void*,const std::string&,bool&) const { return HRESULT(0); };
    HRESULT CreateSound   (void*,void*)const { return HRESULT(0); };
    HRESULT DuplicateSound(void*,const std::string&,const unsigned int&)const{return HRESULT(0);};
    HRESULT DuplicateSound(void*,const std::string&)const{return HRESULT(0);};
    sdsrc*  PurchaseSource() const;
    sdsrc*  PurchaseSource(const unsigned int&)const;
    void    PurchaseSource(sdsrc**)const{};
    void    PurchaseSource(sdsrc**,const unsigned int&)const{};

    void    ThrashSource(sdsrc*)const{};

    void SetCoordinateSystem(CoordinateSystem){};    

	// set properties
	void SetDeviceProperty(lprops,void*){};

    void Play(const std::string&, const point3&){};
    void Play(const std::string&, const point3* new_pos=NULL){};
    void Play(const std::string&, const std::string&, const point3&){};
    void Play(const tray&, const point3* new_pos=NULL){};

    void SmoothPlay(const tray&, const point3* new_pos=NULL){};
    void SmoothMusicDecay(const tray&,DWORD LiveAfterStop=BRIEF){};
    void SmoothMusicExpand(const tray&){};


    void AdjustGlobalVolume(int value){};
    void GetGlobalVolume(void* value)const{};  
    void GetMusicVolume  (void* value)const{};
    void AdjustMusicVolume(int value){};

    // updating all geometric and kinematic parameters at once
    void Update(const point3& pos,const point3& vel,const point3& front,const point3& up){};
    void Update(){};

    void GetPos(point3& pos)const{};
    void GetVel(point3& vel)const{};

    void SetPos (const point3&) {};
    void SetVel (const point3&) {};
    void BeOriented(const point3&,const point3&) {};
    void SetXYZ (const point3&,const point3&,const point3&,const point3&) {};

    void Stop   (const std::string&)const {};  
	void StopEx (const std::string&){};  
    void Pause  (const std::string&)const {};
    void Resume (const std::string&)const {};
    void Stop  (const tray&)const{};
    void StopEx(const tray&){};
    void Pause (const tray&)const{};
    void Resume(const tray&)const{};

    void PauseAll ()const{};
    void ResumeAll()const{};
private:
	explicit Placebo();
};

class sound::plsrc : public sdsrc{
public:
    explicit plsrc() {};
    virtual ~plsrc() {};

    bool Initialize(const srcdsc&,const char* ovrfname=NULL){return true;};

    void Play(){};

    bool IsPlaying() const{ return false; };
    void SmoothVolumeDecay(){};
    void SmoothVolumeExpand(){};
    void SmoothPlay(){};
    void Execute(){};

    void Stop() const{};
    void Pause(){};
    void Resume(){};
    void Reset() const{};

    void Volume(int value)const{};
    void AdjustVolume(int value)const{};
    void Freq  (unsigned int value)const{};
    void Pan   (int value)const{};

    void SetPos(const point3&){};
    void SetVel(const point3&){};
    // Be Careful!
    void* GetBuf()const {return NULL;};
	unsigned int&	GetLifeType(){ return this->_lifetype; };
};

#endif //DSPL__PLACEBO_H_