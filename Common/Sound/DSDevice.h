/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Classes for Sound Engine including                                                                 
                DirectSound support
                                                                                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                

#ifndef __DSPL_DIRECT_SOUND_H___
#define __DSPL_DIRECT_SOUND_H___

#include <dsound.h>
#include "WavRead.h"
#include "sound.h"

// 3d parameters
#define DSPL_3D_DOPPLERFACTOR   1.0f
#define DSPL_3D_ROLLOFFFACTOR   1.0f
#define DSPL_3D_MINDISTANCE     0.5f 
#define DSPL_3D_MAXDISTANCE     100.0f

#define DEFAULT_MODE            DS3DMODE_NORMAL
#define HEADRELATIVE            DS3DMODE_HEADRELATIVE
#define DISABLE3D               DS3DMODE_DISABLE

#define IMMEDIATELY             DS3D_IMMEDIATE
#define DEFERRED                DS3D_DEFERRED

#define NOT_DEF_YET             -1

struct DSSndDesc{
    bool     looped;
    bool     Use3D;
    LONG     PanValue;
    LONG     FreqValue;
    LONG     VolValue;
    D3DVALUE MaxDist;
    D3DVALUE MinDist;
    DWORD    Mode;   
    D3DVALUE Rolloff;
    D3DVALUE DopplerFactor;

    DSSndDesc() : looped       (true),
                  Use3D        (true),
                  PanValue     (NOT_DEF_YET),
                  FreqValue    (NOT_DEF_YET),
                  VolValue     (NOT_DEF_YET),
                  MaxDist      (DSPL_3D_MAXDISTANCE),
                  MinDist      (DSPL_3D_MINDISTANCE),
                  Mode         (DEFAULT_MODE),
                  Rolloff      (DSPL_3D_ROLLOFFFACTOR),
                  DopplerFactor(DSPL_3D_DOPPLERFACTOR) {};
};

class DSSoundFile;

class DSDevice : public SoundDevice{
    private:
        LPDIRECTSOUND               dsound;              // DirectSound Interface
        LPDIRECTSOUND3DLISTENER     dsound_3dlistener;
        DS3DLISTENER                listener_3dparms;

        std::map<std::string, DSSoundFile> SoundFile_Map; // map is to help managing sounds in memory
        //std::map<std::string, DSSndDesc>   SNDDesc_Map;   // map of sound descriptions, string is the name of description

        //statistics
        int total_memory_occupied; // how many bytes buffers occupied at all(hardware+software)
        int MaxMemoryValue;        // max value for memory limit (use SetMemSize to set this item)
        int hard_mem_occupied;     // how many bytes in hardware memory buffers occupied
        int soft_mem_occupied;     // how many bytes in software memory buffers occupied 

    public:
        DSDevice();
        virtual ~DSDevice(); 

        // class interface
        SndHandle    Play(const std::string Name, const std::string SndDesc);

        int    InitializeSound(HWND);
        void   CloseSound(); 

        LPDIRECTSOUND           RequestDSound()     const { return dsound; };
        LPDIRECTSOUND3DLISTENER RequestDSListener() const { return dsound_3dlistener; };
        DS3DLISTENER*           RequestDSListenerParms()  { return &listener_3dparms; }; 

        void   UpdateListenerXYZ(const point3& pos, const point3& vel, const point3& front, const point3& up, DWORD flag=IMMEDIATELY);
        void   UpdateBufferXYZ  (const SndHandle& shdl, const point3& pos, const point3& vel, DWORD flag=IMMEDIATELY);

        void   UpdateListenerPos(const point3& pos, DWORD flag=IMMEDIATELY);
        void   UpdateListenerVel(const point3& vel, DWORD flag=IMMEDIATELY);

        void   UpdateListenerOrientation(const point3& front, const point3& up, DWORD flag=IMMEDIATELY);

        void   SetDistanceFactor(D3DVALUE new_factor, DWORD flag=IMMEDIATELY);
        void   SetRolloff       (D3DVALUE new_rolloff,DWORD flag=IMMEDIATELY);
        void   SetDopplerFactor (D3DVALUE new_doppler_factor, DWORD flag=IMMEDIATELY);

        void   CommitDeferredSets();

        void   Stop(const SndHandle&)  const;  
        void   StopAll(int type)       const {};     

        void   Pause(const SndHandle&) const;
        void   Resume(const SndHandle&)const;

        void   Volume(const SndHandle&, LONG)const;
        void   VolumeType(int type, int value)      const {};

        // memory operations
        void OccupyHardRoom(int buf_size) { hard_mem_occupied+=buf_size; total_memory_occupied+=buf_size; };
        void OccupySoftRoom(int buf_size) { soft_mem_occupied+=buf_size; total_memory_occupied+=buf_size; };

        void SetMemMaxValue(int value) { MaxMemoryValue = value; };

        // Information
        void GiveReport() const;
};

class DSBuffer;

class DSSoundFile : public SoundFile{
    private:
        std::string              Name;    // name of file
        static int               NextId;  // start value for the id
        bool                     FileExists;
        std::map<int, DSBuffer*> Map_of_Bufs;

    public:
        DSSoundFile() {};
        DSSoundFile(const std::string Name);

        virtual ~DSSoundFile();

        int  InitializeSnd(const DSSndDesc&);
        //int  InitializeSnd();

        bool NonEmpty() const { return FileExists; };

        SndHandle Play();    

        void Stop(int id);   
        void StopAll() const {};   

        void Pause(int id)  const;  
        void Resume(int id) const; 

        void Volume(int id, LONG value) const;

        void UpdateBufferXYZ(int id, const D3DVECTOR& pos, const D3DVECTOR& vel, DWORD flag);

        // get info
        void GiveReport() const;
        // returns overall statistics
        FileCtrlCard* GetFileCard() { return &card; };
};

class DSBuffer : public SndBuffer{
    private:
        bool looped;

    protected:
        LPDIRECTSOUNDBUFFER     buf;        // pointer to DirectSoundBuffer interface
        CWaveSoundRead          *wav_file;  // use to manipulate wav files

    public:
        DSBuffer(); 
        DSBuffer(const DSBuffer&);        // copy constructor
        
        virtual ~DSBuffer();

        virtual int InitializeBuf(const std::string wav_name);

        virtual void Play();
        bool IsPlaying()const;

        void Stop()  const;
        void Pause() const;

        void Volume(LONG value) const;
        void Freq  (LONG value) const {};
        void Pan   (LONG value) const {};  
        
        virtual HRESULT RestoreBuffers() const;
        virtual HRESULT FillBuffer() const;

        BufCtrlCard*        GetBufCard ()       { return &card; };
        LPDIRECTSOUNDBUFFER GetBuf     () const { return buf;   };
};

class DSBuffer3D : public DSBuffer{
    private:
        DSSndDesc desc;

    protected:
        LPDIRECTSOUND3DBUFFER   buf3d;
        DS3DBUFFER              buf3dparms;

    public:
        DSBuffer3D();

        DSBuffer3D(const DSBuffer3D&); // copy constructor

        virtual ~DSBuffer3D();

        int InitializeBuf(const std::string wav_name);

        void Play();

        // 3D advancements
        DSSndDesc* GetDesc() { return &desc; };

        void UpdateBufferXYZ(const D3DVECTOR *pos, const D3DVECTOR *vel, DWORD flag);

        DS3DBUFFER* Get3DParms() { return &buf3dparms; };

        void SetObjectProps(D3DVECTOR* pos, D3DVECTOR* vel);

        void SetParams(FLOAT fDopplerFactor=DSPL_3D_DOPPLERFACTOR,
                       FLOAT fRolloffFactor=DSPL_3D_ROLLOFFFACTOR,
                       FLOAT fMinDistance  =DSPL_3D_MINDISTANCE,   
                       FLOAT fMaxDistance  =DSPL_3D_MAXDISTANCE);    

};

#endif __DSPL_DIRECT_SOUND_H___