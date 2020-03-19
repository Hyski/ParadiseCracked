/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Classes for Sound Engine including                                                                 
                DirectSound support
                DSDevice.cpp
  
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                

#include "DSDevice.h"
#include "abnormal.h"
//#include "sndcompiler.h"
#include "..\shell\shell.h"

#include "mmsystem.h"

//SndCompiler        compiler;
//extern DSDevice    SndDevice;

const char* script_filename = "sndscript";

#ifdef DSPL__DEB_VER_STORE_ERRORS
Abnormal    err; 
#endif

inline BufCtrlCard::BufCtrlCard()
{
    played_times   = 0;
    birth_time     = 0;
    last_time_used = 0;
    buf_size       = 0;

    duplicated  = false;
};

inline FileCtrlCard::FileCtrlCard()
{
    birth_time         = 0;
    total_played_times = 0; 
    
    frequency          = 0;

    lifetime           = 0;
    single_buf_size    = 0;
    total_mem_occupied = 0;
    num_of_bufs        = 0;
};


/***************************************************************************
****************************************************************************

                            DSDevice realization

***************************************************************************
***************************************************************************/

DSDevice::DSDevice() : dsound(NULL), dsound_3dlistener(NULL), 
                             total_memory_occupied(0), 
                             MaxMemoryValue(0),
                             hard_mem_occupied(0),
                             soft_mem_occupied(0)
{         
}

DSDevice::~DSDevice()
{
}

int DSDevice::InitializeSound(HWND hwnd)
{
    HRESULT             hr = 0;
    DSBUFFERDESC        dsbdesc;
    LPDIRECTSOUNDBUFFER primbuf;

    if(FAILED(hr = DirectSoundCreate(NULL, &dsound, NULL)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("DirectSoundCreate()", hr, "failed in DSDevice::InitializeSound");
#endif
        return 0; // smthg is wrong, you've got to handle
    }

    if(FAILED(hr = dsound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("SetCooperativeLevel()", hr, "failed in DSDevice::InitializeSound");
#endif

        return 0; // smthg is wrong, you've got to handle
    }

    //dsound_caps.dwSize=sizeof(DSCAPS);
/*    if(FAILED(hr = dsound->GetCaps(&dsound_caps)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("GetCaps()", hr, "failed in DSDevice::InitializeSound");
#endif
    }*/

    // build the listener

    // Obtain primary buffer, asking it for 3D control
    ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
    if( FAILED( hr = dsound->CreateSoundBuffer( &dsbdesc, &primbuf, NULL ) ) )
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("Createsoundbuffer()", hr, "failed in DSDevice::InitializeSound");
#endif
        return 0;
    }

    if( FAILED( hr = primbuf->QueryInterface( IID_IDirectSound3DListener, 
                                              (VOID**)&dsound_3dlistener ) ) )
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundbuffer::QueryInterface()", hr, "failed in DSDevice::InitializeSound");
#endif

        return 0;
    }

    // Get listener parameters
    listener_3dparms.dwSize = sizeof(DS3DLISTENER);
    dsound_3dlistener->GetAllParameters( &listener_3dparms );

    if(primbuf)
        primbuf->Release();

    // if initializations are successfull, then we'll
    // read the script for sound descriptions
    //compiler.Compile(script_filename);
    return 1;
}

void DSDevice::CloseSound()
{
    // release previously allocated objects
    if(dsound)            {            dsound->Release();            dsound = NULL; };
    if(dsound_3dlistener) { dsound_3dlistener->Release(); dsound_3dlistener = NULL; };
}

SndHandle DSDevice::Play(const std::string Name, const std::string SndDesc)
{
    // SndHandle of played buffer will be returned
    SndHandle shdl;
    // if DirectSound initialization failed so boil out
    if(!dsound) 
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("DSDevice::Play()", -1, "returning incorrect sound handle");
#endif
        return shdl;
    }

    // locate object with .Name.
    std::map<std::string, DSSoundFile>::iterator p = SoundFile_Map.find(Name);

    // if it does not exist
    if(p==SoundFile_Map.end())
    {
        // create new DSSoundFile : it means new sound file have to played
        DSSoundFile sfile(Name);

        // description is supposeed to be taken from map<string, DSSndDesc>
        DSSndDesc desc;

        int init_res = sfile.InitializeSnd(desc);
        // create new record <Name, sfile>
        SoundFile_Map[Name] = sfile; // create new 

        if(init_res)
            shdl = SoundFile_Map[Name].Play();
#ifdef DSPL__DEB_VER_STORE_ERRORS
        else
        {
            err.PostError("sfile.InitializeSnd()", init_res, "failed in DSDevice::Play");
        }
#endif
    }
    else
    {
        if(p->second.NonEmpty())
            shdl = p->second.Play();
#ifdef DSPL__DEB_VER_STORE_ERRORS
        else
        {
            err.PostNote("sfile.Play()", "failed in DSDevice::Play, attempt to play non-existing file");
        }
#endif
    }

    return shdl;
}

void DSDevice::Stop(const SndHandle &shdl) const
{
    // if DirectSound initialization failed so boil out

    if(!dsound) 
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("DSDevice::Stop()", -1, "aborting function");
#endif
        return;
    }


    // locate object with .Name.
    std::string name_of = shdl.Name;
    std::map<std::string, DSSoundFile>::const_iterator p = SoundFile_Map.find(name_of);

    // stop the buffer
    if(p!=SoundFile_Map.end())
    {
        // locate DSSoundFile object and call Stop 
        int id_buf_to_stop = shdl.id;
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("Stopping sound, name=%s ...\n", name_of.c_str());
#endif
        ((DSSoundFile&)(p->second)).Stop(id_buf_to_stop);
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
    {
        err.PostNote("DSDevice::Stop()", "attempt to stop non-existing file");
    }
#endif
}

void DSDevice::Volume(const SndHandle& shdl, LONG value) const
{
    // if DirectSound initialization failed so boil out
    if(!dsound) 
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("DSDevice::Volume()", -1, "aborting function");
#endif
        return;
    }

    if(value < DSBVOLUME_MIN || value > DSBVOLUME_MAX)
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostNote("DSDevice::Volume()", "applied volume is out of range");
#endif
        return;
    }

    // locate object with .Name.
    std::string name_of = shdl.Name;
    std::map<std::string, DSSoundFile>::const_iterator p = SoundFile_Map.find(name_of);

    // stop the buffer
    if(p!=SoundFile_Map.end())
    {
        // locate DSSoundFile object and call Stop 
        int id_of_buf = shdl.id;
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("Setting new volume for sound, name=%s ...\n", name_of.c_str());
#endif
        p->second.Volume(id_of_buf, value);
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
    {
        err.PostNote("DSDevice::Volume()", "attempt to modify volume for non-existing file");
    }
#endif
}

void DSDevice::Pause(const SndHandle& shdl) const
{
    // if DirectSound initialization failed so boil out
    if(!dsound) 
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("DSDevice::Pause()", -1, "aborting function");
#endif
        return;
    }

    // locate object with .Name.
    std::string name_of = shdl.Name;
    std::map<std::string, DSSoundFile>::const_iterator p = SoundFile_Map.find(name_of);

    // stop the buffer
    if(p!=SoundFile_Map.end())
    {
        // locate DSSoundFile object and call Stop 
        int id_buf = shdl.id;
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("Pausing sound, name=%s ...\n", name_of.c_str());
#endif
        p->second.Pause(id_buf);
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
    {
        err.PostNote("DSDevice::Pause()", "attempt to pause non-existing file");
    }
#endif
}

void DSDevice::Resume(const SndHandle& shdl) const
{
    // if DirectSound initialization failed so boil out
    if(!dsound) 
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("DSDevice::Resume()", -1, "aborting function");
#endif
        return;
    }


    // locate object with .Name.
    std::string name_of = shdl.Name;
    std::map<std::string, DSSoundFile>::const_iterator p = SoundFile_Map.find(name_of);

    // stop the buffer
    if(p!=SoundFile_Map.end())
    {
        // locate DSSoundFile object and call Stop 
        int id_buf = shdl.id;
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("Resuming sound, name=%s ...\n", name_of.c_str());
#endif
        p->second.Resume(id_buf);
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS        
    else
    {
        err.PostNote("DSDevice::Resume()", "attempt to resume non-existing file");
    }
#endif
}

void DSDevice::GiveReport() const
{
#ifdef DSPL__DEBUG_VERSION
    std::map<std::string, DSSoundFile>::const_iterator p;
    //SoundLog[log_name]("\n>>>> Files played:\n");

    // print information about files
    for(p = SoundFile_Map.begin();p != SoundFile_Map.end();++p)
    {
        p->second.GiveReport();
    }

    //SoundLog[log_name]("\n>>>> Memory report:\n");
    HRESULT hr = 0;
    DSCAPS  dscaps;

    dscaps.dwSize=sizeof(DSCAPS);
    if(SUCCEEDED(hr = dsound->GetCaps(&dscaps)))
    {
        //SoundLog[log_name]("%\t total memory available on sound card is %d bytes\n%\t total free memory is %d bytes\n%\t used memory is %d bytes:\n", 
        //                   dscaps.dwTotalHwMemBytes,
        //                   dscaps.dwFreeHwMemBytes,
        //                   dscaps.dwTotalHwMemBytes-dscaps.dwFreeHwMemBytes);
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS        
    else
    {
        err.PostError("GetCaps()", hr, "failed while reporting");
    }
#endif
#endif
}

void DSDevice::UpdateListenerXYZ(const point3& pos, const point3& vel, const point3& front, const point3& up, DWORD flag)
{
    D3DVECTOR vps, vvl, vfrt, vup;

    vps.x = pos.x;
    vps.y = pos.y;
    vps.z = pos.z;

    vvl.x = vel.x;
    vvl.y = vel.y;
    vvl.z = vel.z;

    vfrt.x = front.x;
    vfrt.y = front.y;
    vfrt.z = front.z;

    vup.x = up.x;
    vup.y = up.y;
    vup.z = up.z;

    memcpy( &listener_3dparms.vPosition,    &vps,  sizeof(D3DVECTOR) );
    memcpy( &listener_3dparms.vVelocity,    &vvl,  sizeof(D3DVECTOR) );
    memcpy( &listener_3dparms.vOrientFront, &vfrt, sizeof(D3DVECTOR) );
    memcpy( &listener_3dparms.vOrientTop,   &vup,  sizeof(D3DVECTOR) );

    HRESULT hr = 0;

    if( dsound_3dlistener )
    {
        if(FAILED(hr = dsound_3dlistener->SetAllParameters( &listener_3dparms, DS3D_IMMEDIATE )))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirect3DListener::SetAllParameters()", hr, "failed in SoundDevice::UpdateListener");
#endif
        }
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("SndBuffer3D::SetParams", hr, "failed in SoundDevice::UpdateListener");
#endif
}

void DSDevice::UpdateBufferXYZ(const SndHandle& shdl, const point3& pos, const point3& vel, DWORD flag)
{
    if(!dsound) 
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        //err.PostError("DSDevice::UpdateBufferXYZ()", DSPL__DOESNT_EXIST, "aborting function");
#endif
        return;
    }

    // locate object with .Name.
    std::string name_of = shdl.Name;
    std::map<std::string, DSSoundFile>::iterator p = SoundFile_Map.find(name_of);

    // update the buffer
    if(p!=SoundFile_Map.end())
    {
        // locate DSSoundFile object and call Stop 
        D3DVECTOR vps, vvl;

        vps.x = pos.x;
        vps.y = pos.y;
        vps.z = pos.z;

        vvl.x = vel.x;
        vvl.y = vel.y;
        vvl.z = vel.z;

        int id_buf_to_update = shdl.id;
        p->second.UpdateBufferXYZ(id_buf_to_update, vps, vvl, flag);
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostNote("SoundDevice::UpdateBufferXYZ()", "attempt to update non-existing file");
#endif
}

inline void DSDevice::SetDistanceFactor(D3DVALUE new_factor, DWORD flag)
{ 
    dsound_3dlistener->SetDistanceFactor(new_factor, flag); 
};

inline void DSDevice::SetRolloff(D3DVALUE new_rolloff,DWORD flag)
{ 
    dsound_3dlistener->SetRolloffFactor(new_rolloff, flag); 
};

inline void DSDevice::SetDopplerFactor(D3DVALUE new_doppler_factor, DWORD flag)
{ 
    dsound_3dlistener->SetDopplerFactor(new_doppler_factor, flag); 
};

inline void DSDevice::CommitDeferredSets()
{ 
    dsound_3dlistener->CommitDeferredSettings(); 
};

void DSDevice::UpdateListenerPos(const point3& pos, DWORD flag) 
{
    D3DVALUE new_x=pos.x, new_y=pos.y, new_z=pos.z;
    //D3DVECTOR cur_pos;

    dsound_3dlistener->SetPosition(new_x, new_y, new_z, flag);
    //dsound_3dlistener->GetPosition(&cur_pos);

    //std::cout << "x=" << cur_pos.x << " y=" << cur_pos.y << " z=" << cur_pos.z << "\n";
};

void DSDevice::UpdateListenerVel(const point3& vel, DWORD flag) 
{
    D3DVALUE new_vx=vel.x, new_vy=vel.y, new_vz=vel.z;

    dsound_3dlistener->SetVelocity(new_vx, new_vy, new_vz, flag);
};

void DSDevice::UpdateListenerOrientation(const point3& front, const point3& up, DWORD flag) 
{
    D3DVALUE new_frx = front.x, 
             new_fry = front.y, 
             new_frz = front.z, 
             new_upx =    up.x, 
             new_upy =    up.y, 
             new_upz =    up.z; 

    dsound_3dlistener->SetOrientation(new_frx, new_fry, new_frz, new_upx, new_upy, new_upz, flag);
};

/***************************************************************************
****************************************************************************

                            DSSoundFile realization

***************************************************************************
***************************************************************************/

int DSSoundFile::NextId = DEFAULT_ID;

DSSoundFile::DSSoundFile(const std::string Name) : Name(Name), 
                                                   FileExists(false)
{  
}

DSSoundFile::~DSSoundFile() {};
   
int DSSoundFile::InitializeSnd(const DSSndDesc& desc)
{
    // set up id 
    int id;

    DSBuffer *sndbuf = NULL;

    // try to find non-playing buffer to duplicate it
    typedef std::map<int, DSBuffer*>::const_iterator CI;

    for(CI p = Map_of_Bufs.begin(); p != Map_of_Bufs.end(); ++p)
    {
        //DSBuffer3D& sb = *(DSBuffer3D *)p->second;

        if(desc.Use3D)
            sndbuf = new DSBuffer3D(*(DSBuffer3D *)p->second);
        else
            sndbuf = new DSBuffer(*(DSBuffer *)p->second);
        

        if((sndbuf->GetBufCard())->duplicated) // duplication has succeeded
        {
            card.num_of_bufs++;    
            id = NextId++;
            FileExists = true;

            if(desc.Use3D)
            {
                DSBuffer3D* buf3d = (DSBuffer3D *)sndbuf;
                memcpy(buf3d->GetDesc(), &desc, sizeof(desc));
                buf3d->SetParams();
            }
            //make pair <id, *sndbuf>
            Map_of_Bufs[id] = sndbuf;
            break;
        }
        else
        {
            if(sndbuf) delete sndbuf;
            sndbuf = NULL;
        }
    }
    // all attempts to duplicate buffer has failed
    if(p==Map_of_Bufs.end())
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostNote("DSSoundFile::InitializeSnd()", "all attempts to get dublication copy failed.Building original one...");
#endif
    
        if(desc.Use3D)
            sndbuf = new DSBuffer3D();
        else
            sndbuf = new DSBuffer();

        // read wav and init
        // hint: InitializeBuf() returns id of created buffer

        if(sndbuf->InitializeBuf(Name))
        {
            // set up id 
            id = NextId++;

            // file exists
            FileExists = true;
            card.num_of_bufs++;    

            if(desc.Use3D)
            {
                DSBuffer3D* buf3d = (DSBuffer3D *)sndbuf;
                memcpy(buf3d->GetDesc(), &desc, sizeof(desc));
                buf3d->SetParams();
            }

            // make pair <id, *sndbuf>
            Map_of_Bufs[id] = sndbuf;
        }
        else
            return 0;
    }

    // use only for 3d settings
    /*if(use_3d)
    {
        DSBuffer3D *sndbuf3d = (DSBuffer3D *)sndbuf;
        sndbuf3d->SetParams();
    }*/
    //DSBuffer3D *sndbuf3d = (DSBuffer3D *)sndbuf;
    //sndbuf3d->SetParams();

#ifdef DSPL__DEB_VER_SHOW_COMMENT
    //SoundLog[log_name]("Buffer initialization: Successed... returning buffer id=%d\n", id);
#endif
    return id;
}

SndHandle DSSoundFile::Play()
{
    SndHandle shdl;
    typedef std::map<int, DSBuffer*>::const_iterator CI;

    for(CI p = Map_of_Bufs.begin(); p != Map_of_Bufs.end(); ++p)
    {
        // find first non-playing buffer
        if(!p->second->IsPlaying()) 
        {
            // use Play() method    
            card.total_played_times++;
            shdl.Name = Name;
            shdl.id   = p->first;

            p->second->Play();
            break;
        }
    }
    if(p==Map_of_Bufs.end())
    {
            DSSndDesc desc;
            if(int id = InitializeSnd(desc))
            {
                card.total_played_times++;
                // create sound handle
                shdl.Name = Name;
                shdl.id   = id;

                Map_of_Bufs[id]->Play();
            }
            else
            {
                // trouble
                // create sound handle with error signature
                shdl.Name = Name;
                shdl.id   = -1;
            }
    }

    return shdl;
}

void DSSoundFile::Stop(int id)
{
    std::map<int, DSBuffer*>::iterator p = Map_of_Bufs.find(id);

    // locate the buffer
    if(p != Map_of_Bufs.end())
    {
        // stop id buffer
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("Stopping %s, buffer id=%d...\n", Name.c_str(), id);
#endif
        p->second->Stop();
    }
    else
    {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("Attempting to stop non-existing buffer of %s.Canceling...\n", Name.c_str());
#endif
    }
}

void DSSoundFile::Volume(int id, LONG value) const
{
    std::map<int, DSBuffer*>::const_iterator p = Map_of_Bufs.find(id);

    // locate the buffer
    if(p != Map_of_Bufs.end())
    {
        // stop id buffer
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("Setting new volume for %s, buffer id=%d...\n", Name.c_str(), id);
#endif
        p->second->Volume(value);
    }
    else
    {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("**** Error **** (!) Attempting to set volume for non-existing file(_%s_).Canceling...\n", Name.c_str());
#endif
    }
}

void DSSoundFile::Pause(int id) const
{
    std::map<int, DSBuffer*>::const_iterator p = Map_of_Bufs.find(id);

    // locate the buffer
    if(p != Map_of_Bufs.end())
    {
        // stop id buffer
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("Pausing %s, buffer id=%d...\n", Name.c_str(), id);
#endif
        p->second->Pause();
    }
    else
    {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("Attempting to pause non-existing buffer of %s.Canceling...\n", Name.c_str());
#endif
    }
}

void DSSoundFile::Resume(int id) const
{
    std::map<int, DSBuffer*>::const_iterator p = Map_of_Bufs.find(id);

    // locate the buffer
    if(p != Map_of_Bufs.end())
    {
        // stop id buffer
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("Resuming %s, buffer id=%d...\n", Name.c_str(), id);
#endif
        p->second->Play();
    }
    else
    {
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("Attempting to resume non-existing buffer of %s.Canceling...\n", Name.c_str());
#endif
    }
}

void DSSoundFile::GiveReport() const
{
    std::map<int, DSBuffer*>::const_iterator p;
#ifdef DSPL__DEBUG_VERSION
    //SoundLog[log_name]("\nFile %s has %d buffers played %d times:\n", Name.c_str(), card.num_of_bufs, card.total_played_times);
#endif

#ifdef DSPL__DEBUG_VERSION
    for(p = Map_of_Bufs.begin(); p != Map_of_Bufs.end(); ++p)
    {
        //p->second->GetBufCard()->duplicated ?
            //SoundLog[log_name]("%\t Buffer id=%d was duplicated, played %d times\n", p->first, p->second->GetBufCard()->played_times):
            //SoundLog[log_name]("%\t Buffer id=%d was created, played %d times, buffer size=%d\n", p->first, p->second->GetBufCard()->played_times, p->second->GetBufCard()->buf_size);
    }
#endif
}

void DSSoundFile::UpdateBufferXYZ(int id, const D3DVECTOR& pos, const D3DVECTOR& vel, DWORD flag)
{
    std::map<int, DSBuffer*>::iterator p = Map_of_Bufs.find(id);

    // locate the buffer
    if(p != Map_of_Bufs.end())
    {
        // update id buffer
        ((DSBuffer3D *)p->second)->UpdateBufferXYZ(&pos, &vel, flag);
    }
}

/***************************************************************************
****************************************************************************

                            DSBuffer realization

***************************************************************************
***************************************************************************/

DSBuffer::DSBuffer() : buf(NULL),
                       wav_file(NULL)
{
}

DSBuffer::DSBuffer(const DSBuffer& sndbuf) : 
                       buf(NULL),
                       wav_file(NULL)
{
    HRESULT hr = 0;

    if(FAILED(hr = Shell::SndDevice->RequestDSound()->DuplicateSoundBuffer(sndbuf.buf, &buf)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("DublicateSoundBuffer()", hr, "failed in DSBuffer::DSBuffer");    
#endif
    }
    else
        card.duplicated = true;
}

DSBuffer::~DSBuffer()
{
    if(buf)      { buf->Release(); buf = NULL; };
    if(wav_file) { delete wav_file; wav_file = NULL; };
}

int DSBuffer::InitializeBuf(std::string wav_name)
{
	DWORD len; // длина файла
	// указатели
	BYTE * ptr1 = NULL;
	BYTE * ptr2 = NULL;
	// размеры
	DWORD size1;
	DWORD size2;
	// handle для файла и проекции файла
	HANDLE hFile, hMap;
	// временный указатель для проекции файла
	void * lpv;

	// откроем файл, узнаем его размер
	hFile = CreateFile(wav_name.c_str(), GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	// определяем длину
	len = GetFileSize(hFile, NULL);

	// создаем проекцию
	hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY,
		0, 0, NULL);
	// закрываем файл
	CloseHandle(hFile);
	if(!hMap)
	{
		return 0;
	}
	// передаем физическую память проекции
	lpv = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	CloseHandle(hMap);
	if(!lpv)
	{
		return 0;
	}

    // check, if it's really a wave-formatted file
    // and if it ain't so cancel initializing
    
    // check if ain't RIFF
    if( ! (
            * (char *)lpv      == 'R' &&
            *((char *)lpv + 1) == 'I' &&
            *((char *)lpv + 2) == 'F' &&
            *((char *)lpv + 3) == 'F' 
           ) 
      ) return 0; // it's not RIFF-formatted file, canceling...

    // check if ain't WAVE
    if( ! (
            *((char *)lpv + 8)     == 'W' &&
            *((char *)lpv + 1 + 8) == 'A' &&
            *((char *)lpv + 2 + 8) == 'V' &&
            *((char *)lpv + 3 + 8) == 'E' 
           ) 
      ) return 0; // it's not WAVE-formatted file, canceling...

	// buffer charasteristics
	LPWAVEFORMATEX wave   = new WAVEFORMATEX;

	wave->cbSize          = sizeof(WAVEFORMATEX);

	wave->nAvgBytesPerSec = *((DWORD *)lpv + 7);
	wave->nBlockAlign     = *(( WORD *)lpv + 16);
	wave->nChannels       = *(( WORD *)lpv + 11);
	wave->nSamplesPerSec  = *((DWORD *)lpv + 6);
	wave->wBitsPerSample  = *(( WORD *)lpv + 17);
	wave->wFormatTag      = *(( WORD *)lpv + 10);//WAVE_FORMAT_PCM;

	DSBUFFERDESC dsbd;
	memset(&dsbd, 0, sizeof(dsbd));
	dsbd.dwBufferBytes = len - 44;
	dsbd.dwFlags = DSBCAPS_LOCDEFER | DSBCAPS_CTRLVOLUME;
	dsbd.dwSize = sizeof(dsbd);
	dsbd.lpwfxFormat = wave;
	// создадим буфер
	if(Shell::SndDevice->RequestDSound()->CreateSoundBuffer(&dsbd, &buf, NULL) != DS_OK)
	{
		CloseHandle(hFile);
		delete wave;
		return 0;
	}

	delete wave;

	// заполнение звукового буфера  lock-memcpy-unlock
	if(buf->Lock(0, len - 44 - 55,
		(void **)&ptr1, &size1,	(void **)&ptr2, &size2, 0) != DS_OK)
	{
		// забираем физическую память у проекции
		UnmapViewOfFile(lpv);
		return 0;
	}
	// копируем
	memcpy (ptr1, (BYTE *)lpv + 44, size1);
	if(buf->Unlock(ptr1, len - 44 - 55, NULL, 0) != DS_OK)
	{
		// забираем физическую память у проекции
		UnmapViewOfFile(lpv);
		return 0;
	}
	// забираем физическую память у проекции
	UnmapViewOfFile(lpv);

	// буфер создан
	if(buf->SetCurrentPosition(0) != DS_OK)
	{
		return 0;
	}

    return 1;    
}

HRESULT DSBuffer::FillBuffer() const
{
    // data for wave file reading and
    // filling created buffer
    HRESULT hr = 0;
    BYTE*   pbWavData; // Pointer to actual wav data 
    UINT    cbWavSize; // Size of data
    VOID*   pbData  = NULL;
    VOID*   pbData2 = NULL;
    DWORD   dwLength;
    DWORD   dwLength2;

    // fill the buffer
    // The size of wave data is in pWaveFileSound->m_ckIn
    INT nWaveFileSize = wav_file->m_ckIn.cksize;

    // Allocate that buffer.
    pbWavData = new BYTE[ nWaveFileSize ];
    if( NULL == pbWavData )
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS    
        err.PostNote("DSBuffer::FillBuffer()", "out of memory");
#endif
        return E_OUTOFMEMORY;
    }

    if( FAILED( hr = wav_file->Read( nWaveFileSize, 
                                     pbWavData, 
                                     &cbWavSize ) ) )           
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("DSBuffer::FillBuffer()", hr, "failed reading wav file");
#endif
        return hr;
    }

    // Reset the file to the beginning 
    wav_file->Reset();

    // Lock the buffer down
    if( FAILED( hr = buf->Lock( 0, card.buf_size, &pbData, &dwLength, 
                                   &pbData2, &dwLength2, 0L ) ) )
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundBuffer::Lock()", hr, "failed in DSBuffer::FillBuffer()");
#endif
        return hr;
    }

    // Copy the memory to it.
    memcpy( pbData, pbWavData, card.buf_size );

    // Unlock the buffer, we don't need it anymore.
    buf->Unlock( pbData, card.buf_size, NULL, 0 );
    pbData = NULL;

    // We dont need the wav file data buffer anymore, so delete it 
    if( pbWavData ) { delete pbWavData; pbWavData = NULL; };

    return S_OK;
}

bool DSBuffer::IsPlaying() const
{
    // if buffer is playing
    if(buf)
    {  
        DWORD dwStatus = 0;
        buf->GetStatus( &dwStatus );
        return(( dwStatus & DSBSTATUS_PLAYING) != 0 );
    }
    else
    {
        return FALSE;
    }

    return 1;
}

void DSBuffer::Play()
{
    HRESULT hr = 0;

    card.played_times++;

    if( FAILED( hr = RestoreBuffers() ) )
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("RestoreBuffers()", hr, "failed in DSBuffer::Play()");
#endif
    }

    // Play buffer 

    DWORD LoopIt = (looped) ? DSBPLAY_LOOPING : 0L;
    if( FAILED( hr = buf->Play( 0, 0, LoopIt ) ) )
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundBuffer::Play()", hr, "failed in DSBuffer::Play()");    
#endif
    }
}

void DSBuffer::Stop() const
{
    HRESULT hr = 0;

    if(FAILED(hr = buf->Stop()))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundBuffer::Stop()", hr, "failed in DSBuffer::Stop()");
#endif
        return;
    }
    if(FAILED(hr = buf->SetCurrentPosition(0L)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundBuffer::SetCurrentPosition()", hr, "failed in DSBuffer::Stop()");
#endif;
    }

#ifdef DSPL__DEB_VER_SHOW_COMMENT
    //SoundLog[log_name]("DirectSound has successfully stopped sound...\n");
#endif
}

void DSBuffer::Pause() const
{
    HRESULT hr = 0;

    if(FAILED(hr = buf->Stop()))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundBuffer::Stop()", hr, "failed in DSBuffer::Stop()");
#endif
        return;
    }

#ifdef DSPL__DEB_VER_SHOW_COMMENT
    //SoundLog[log_name]("DirectSound has successfully paused sound...\n");
#endif
}

void DSBuffer::Volume(LONG value) const
{
    HRESULT hr = 0;

    if(FAILED(hr = buf->SetVolume(value)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundBuffer::SetVolume()", hr, "failed in DSBuffer::Volume()");
#endif
        return;
    }
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    //SoundLog[log_name]("DirectSound has successfully changed volume...\n");
#endif
}

HRESULT DSBuffer::RestoreBuffers() const
{
    HRESULT hr = 0;

    if(buf == NULL)
        return S_OK;

    DWORD dwStatus;
    if( FAILED( hr = buf->GetStatus( &dwStatus ) ) )
        return hr;

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control.
#ifdef DSPL__DEB_VER_SHOW_COMMENT
        //SoundLog[log_name]("Restoring lost buffers...\n");
#endif
        do 
        {
            hr = buf->Restore();
            if( hr == DSERR_BUFFERLOST )
                Sleep( 10 );
        }
        while( hr = buf->Restore() );

        if( FAILED( hr = FillBuffer() ) )
            return hr;
    }

    return S_OK;
};

/***************************************************************************
****************************************************************************

                            DSBuffer3d realization

***************************************************************************
***************************************************************************/

DSBuffer3D::DSBuffer3D() : DSBuffer(), 
                           buf3d(NULL)
{
}

DSBuffer3D::DSBuffer3D(const DSBuffer3D& sndbuf) : 
                           DSBuffer(), 
                           buf3d(NULL)
{
    HRESULT hr = 0;

    if(FAILED(hr = Shell::SndDevice->RequestDSound()->DuplicateSoundBuffer(sndbuf.GetBuf(), &buf)))
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("DublicateSoundBuffer()", hr, "failed in DSBuffer3D::DSBuffer3D");
#endif
    }
    else
    {
        card.duplicated = true;
        HRESULT hr = 0;

        // Get the 3D buffer from the secondary buffer
        if( FAILED( hr = buf->QueryInterface( IID_IDirectSound3DBuffer, 
                                                  (VOID**)&buf3d ) ) )
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirectSoundBuffer::QueryInterface()", hr, "failed in DSBuffer3D::DSBuffer3D()");
#endif
        }

        // Get the 3D buffer parameters
        buf3dparms.dwSize = sizeof(DS3DBUFFER);
        if(FAILED(buf3d->GetAllParameters( &buf3dparms )))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirectBuffer3D::GetAllParameters()", hr, "failed in DSBuffer3D::DSBuffer3D()");
#endif
        }

        // Set new 3D buffer parameters
        buf3dparms.dwMode = DS3DMODE_NORMAL;
        if(FAILED(buf3d->SetAllParameters( &buf3dparms, DS3D_IMMEDIATE )))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirectBuffer3D::SetAllParameters()", hr, "failed in DSBuffer3D::DSBuffer3D()");
#endif
        }
    }
}

DSBuffer3D::~DSBuffer3D()
{
    if(buf)      { buf->Release();   buf      = NULL; };
    if(buf3d)    { buf3d->Release(); buf3d    = NULL; };
    if(wav_file) { delete wav_file;  wav_file = NULL; };
}

int DSBuffer3D::InitializeBuf(const std::string wav_name)
{
    HRESULT hr = 0;

    if(!buf)
    {
        wav_file = new CWaveSoundRead();
        if(FAILED(wav_file->Open((char *)wav_name.c_str())))    
        {
            err.PostError("DSBuffer3D::Initialize()", hr, "error openning wav file");
            return 0;        
        }
    
        DSBUFFERDESC dsbd;
        ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
        dsbd.dwSize        = sizeof(DSBUFFERDESC);
        dsbd.dwFlags       = DSBCAPS_CTRL3D | DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME;
        dsbd.dwBufferBytes = wav_file->m_ckIn.cksize;
        dsbd.lpwfxFormat   = wav_file->m_pwfx;    

        // set 3d algoritm
        /*switch(using_3d)
        {
            case use3DHigh:
#ifdef DSPL__DEB_VER_SHOW_COMMENT
                SoundLog[log_name]("User has choosed the best 3d quality.\n");
#endif
                dsbd.guid3DAlgorithm = DS3DALG_HRTF_FULL;
                break;
            case use3DLight:
#ifdef DSPL__DEB_VER_SHOW_COMMENT
                SoundLog[log_name]("User has choosed light 3d quality.\n");
#endif
                dsbd.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;
                break;
            case use3DLow:
#ifdef DSPL__DEB_VER_SHOW_COMMENT
                SoundLog[log_name]("User has choosed poor 3d quality.\n");
#endif
                dsbd.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
                break;
            default:
#ifdef DSPL__DEB_VER_STORE_ERRORS
                err.PostError("DSBuffer3D::InitializeBuf()", hr, "check use3D<sound quality> parameter(see sound.h), it's probably incorrect");
#endif
                break;
        }*/

        dsbd.guid3DAlgorithm = DS3DALG_HRTF_FULL;
        // Create the static DirectSound buffer 
        // dsound is a part of DSDevice object
        if(FAILED(hr = Shell::SndDevice->RequestDSound()->CreateSoundBuffer( &dsbd, &buf, NULL )))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirectSoundBuffer::CreateSoundBuffer()", hr, "failed in DSBuffer3D::InitializeBuf");
#endif
            return 0;
        }
        // Get the 3D buffer from the secondary buffer
        if( FAILED( hr = buf->QueryInterface( IID_IDirectSound3DBuffer, 
                                                  (VOID**)&buf3d ) ) )
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirectSoundBuffer::QueryInterface()", hr, "failed in DSBuffer3D::InitializeBuf");
#endif
            return 0;
        }

        card.buf_size = dsbd.dwBufferBytes;

        // Get the 3D buffer parameters
        buf3dparms.dwSize = sizeof(DS3DBUFFER);
        if(FAILED(hr = buf3d->GetAllParameters( &buf3dparms )))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirectSound3DBuffer::GetAllParameters()", hr, "failed in DSBuffer3D::InitializeBuf");
#endif
        }

        // Set new 3D buffer parameters
        buf3dparms.dwMode = DS3DMODE_NORMAL;
        if(FAILED(hr = buf3d->SetAllParameters( &buf3dparms, DS3D_IMMEDIATE )))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
           err.PostError("IDirectSound3DBuffer::SetAllParameters()", hr, "failed in DSBuffer3D::InitializeBuf");
#endif
        }


        FillBuffer();
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
    {
        err.PostNote("DSBuffer3D::InitializeBuf()", "buffer is already initialized");
    }
#endif

    return 1;    
}


void DSBuffer3D::SetParams(FLOAT fDopplerFactor, FLOAT fRolloffFactor,
                           FLOAT fMinDistance,   FLOAT fMaxDistance )
{
    HRESULT hr = 0;
    DS3DLISTENER* listener_3dparms = Shell::SndDevice->RequestDSListenerParms();

    listener_3dparms->flDopplerFactor = fDopplerFactor;
    listener_3dparms->flRolloffFactor = fRolloffFactor;

    LPDIRECTSOUND3DLISTENER  ds3dlistener = Shell::SndDevice->RequestDSListener();

    if( ds3dlistener )
    {
        if(FAILED(hr = ds3dlistener->SetAllParameters( listener_3dparms, DS3D_IMMEDIATE )))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirect3DListener::SetAllParameters()", hr, "failed in SetParams");
#endif
        }
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
    {
        err.PostError("DSBuffer3D::SetParams", hr, "failed in SetParams");
    }
#endif

    buf3dparms.flMinDistance = fMinDistance;
    buf3dparms.flMaxDistance = fMaxDistance;

    if( buf3d )
    {
        if(FAILED(hr = buf3d->SetAllParameters( &buf3dparms, DS3D_IMMEDIATE )))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("IDirectBuffer3D::SetAllParameters()", hr, "failed in SetParams");
#endif
        }
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
    {
     //   err.PostError("DSBuffer3D::SetParams", hr, "probably buf3d does not exist.");
    }
#endif
}

void DSBuffer3D::Play()
{
    HRESULT hr = 0;

    card.played_times++;

    if( FAILED( hr = RestoreBuffers() ) )
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("RestoreBuffers()", hr, "failed in DSBuffer::Play()");
#endif
    }

    DWORD LoopIt = (desc.looped) ? DSBPLAY_LOOPING : 0L;

    if( FAILED( hr = buf->Play( 0, 0, LoopIt ) ) )
    {
#ifdef DSPL__DEB_VER_STORE_ERRORS
        err.PostError("IDirectSoundBuffer::Play()", hr, "failed in DSBuffer::Play()");    
#endif
    }
    
    //fix the clock
    //last_time_used = smthg;
}

void DSBuffer3D::UpdateBufferXYZ(const D3DVECTOR *pos, const D3DVECTOR *vel, DWORD flag)
{
    HRESULT hr = 0;

    memcpy( &buf3dparms.vPosition, pos, sizeof(D3DVECTOR) );
    memcpy( &buf3dparms.vVelocity, vel, sizeof(D3DVECTOR) );

    if( buf3d )
    {
        if(FAILED(hr = buf3d->SetAllParameters( &buf3dparms, DS3D_IMMEDIATE )))
        {
#ifdef DSPL__DEB_VER_STORE_ERRORS
            err.PostError("SndBuffer3D::SetAllProperties()", hr, "failed in SndBuffer3D::Update()");
#endif
        }
    }
#ifdef DSPL__DEB_VER_STORE_ERRORS
    else
        err.PostError("SndBuffer3D::SetObjectProperties()", hr, "buffer does not exist");
#endif

}

/***************************************************************************
****************************************************************************

                   E   N   D       O   F       F   I   L   E

***************************************************************************
***************************************************************************/
