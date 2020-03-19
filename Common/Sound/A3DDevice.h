/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Classes for Sound Engine including                                                                 
                Aureal support
                                                                                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                

#ifndef __DSPL_AUREAL_SUPPORTING_SOUND_H___
#define __DSPL_AUREAL_SUPPORTING_SOUND_H___

#include <windows.h>

// A3D Include Files.
#include "ia3dutil.h"
#include "ia3dapi.h"

#include "sound.h"

class A3DSndDesc : public SndDesc{
    protected:
    public:
        A3DSndDesc() {};
        A3DSndDesc(const A3DSndDesc&) {};

        virtual ~A3DSndDesc() {};
};

class A3DDevice : public SoundDevice{
    private:
        IA3d5*                       A3d;
        IA3dListener*                A3dListener;

    public:
        A3DDevice()   ;         
        virtual ~A3DDevice();

        // class interface
        SndHandle    Play(const std::string Name, const std::string SndDesc);

        int    InitializeSound(HWND);
        void   CloseSound(); 

        IA3d5*          RequestA3d()         const { return A3d; };
        IA3dListener*   RequestA3dListener() const { return A3dListener; };

        void   Stop   (const SndHandle&) const;  
        void   StopAll(int type)         const;     
        void   Pause  (const SndHandle&) const;
        void   Resume (const SndHandle&) const;

        void   Volume(const SndHandle&, LONG)const;

        // Information
        void GiveReport() const;
};

class A3DSource;

class A3DSoundFile : public SoundFile{
    private:
        std::map<int, A3DSource*> Map_of_Bufs;

    public:

        A3DSoundFile() {};
        A3DSoundFile(const std::string Name);

        virtual ~A3DSoundFile();

        int  InitializeSnd();
        bool NonEmpty() const;

        SndHandle Play();    

        void Stop(int id);   
        void StopAll() const;   

        void Pause (int id);  
        void Resume(int id); 

        void Volume(int id, LONG value) const;

        //void UpdateBufferXYZ(int id, const point3& pos, const point3& vel, DWORD flag);

        // get info
        void GiveReport() const;
        // returns overall statistics
        FileCtrlCard* GetFileCard();
};
 
class A3DSource : public SndBuffer{
    protected:
        IA3dSource2*  src;
        BufCtrlCard   card;       // use to provide statistics

    public:

        A3DSource();
        A3DSource(const A3DSource&); // copy constructor
        
        virtual ~A3DSource();

        int  InitializeBuf(const std::string file_name);
        void Play();
        bool IsPlaying() const;

        void Stop() const;
        void Pause() const;

        void Volume(float value) const;
        
        BufCtrlCard*    GetBufCard ()       { return &card; };
        IA3dSource2*    GetSrc     () const { return src;   };
};

#endif __DSPL_AUREAL_SUPPORTING_SOUND_H___