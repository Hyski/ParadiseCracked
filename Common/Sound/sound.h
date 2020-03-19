/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Classes for Sound Engine including                                                                 
                DirectSound, Aureal and EAX support
                                                                                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                

#ifndef __DSPL_SOUND_H___
#define __DSPL_SOUND_H___

#include <map>
#include <string>
#include <windows.h>

#include "..\3d\quat.h"

// start value for the id's
#define DEFAULT_ID  1

// if debugging
#define DSPL__DEBUG_VERSION
#define DSPL__DEB_VER_STORE_ERRORS
#define DSPL__DEB_VER_SHOW_COMMENT

// handle is to be returned as unique description
// of id-buffer of SoundFile object( which is created for every file.wav )
typedef struct{
    std::string Name;
    int id;
} SndHandle;

// Buffer card
struct BufCtrlCard{
    int played_times;   // how many times buffer was played
    int birth_time;     // when buffer was born           (game time)
    int last_time_used; // when buffer was last time used (game time)
    int buf_size;       // size of one buffer

    bool duplicated; // buffer can share sound memory with another buffer(s)

    BufCtrlCard();
};

// File card
struct FileCtrlCard{
    int birth_time;         // creation time  acording to    the game  time
    int total_played_times; // how many times this     sound was played
    
    float frequency;

    int   lifetime;
    int   single_buf_size;
    int   total_mem_occupied; // how many bytes   in   memory this sound file buffers has occupied
    int   num_of_bufs;        // how many buffers this sound  file have  in   memory

    FileCtrlCard();
};

class SoundFile;

class SoundDevice{
    public:
        virtual ~SoundDevice() {};   

        // class interface

        virtual SndHandle    Play(const std::string Name, const std::string SndDesc)=0;

        virtual int    InitializeSound(HWND)=0;
        virtual void   CloseSound()=0; 

        virtual void   Stop   (const SndHandle&) const=0;  
        virtual void   StopAll(int type)         const=0;     

        virtual void   Pause (const SndHandle&)  const=0;
        virtual void   Resume(const SndHandle&)  const=0;

        virtual void   Volume(const SndHandle&, LONG) const=0;
        virtual void   VolumeType(int type,int value) const=0;

        // Information
        virtual void GiveReport() const=0;
};

class SndBuffer;

class SoundFile{
    protected:

        FileCtrlCard    card;

    public:

        virtual ~SoundFile() {};

        //virtual int  InitializeSnd(const SndDesc*) = 0;
        virtual bool NonEmpty() const= 0;

        virtual SndHandle Play()     = 0;    

        virtual void Stop(int id)    = 0;   
        virtual void StopAll() const = 0;   

        virtual void Pause (int id) const = 0;  
        virtual void Resume(int id) const = 0; 

        virtual void Volume(int id, LONG value) const = 0;

		void UpdateBuffer(int id, point3* pos, point3* vel);
        // get info
        virtual void GiveReport() const = 0;
        // returns overall statistics
        FileCtrlCard* GetFileCard() { return &card; };
};

class SndBuffer{
    protected:
        BufCtrlCard card;

    public:

        virtual ~SndBuffer() {};

        //virtual int InitializeBuf(const smthg)=0; smthg - struct from pack
        virtual int InitializeBuf(const std::string wav_name)=0;

        virtual void Play()=0;
        virtual bool IsPlaying() const=0;

        virtual void  Stop() const=0;
        virtual void Pause() const=0;

        virtual void Volume(LONG value) const=0;

        //virtual void UpdateBufferXYZ(const point3* pos, const point3* vel) = 0;

        virtual BufCtrlCard* GetBufCard()=0 { return &card; };
};

#endif

/*
    E   N   D       O   F       F   I   L   E
*/
