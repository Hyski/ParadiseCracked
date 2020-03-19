/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: MP3 decoder for Sound Library
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/

#include <dsound.h>
#include "..\precomp.h"
#include "sound.h"
#include "proto.h"
#include <process.h>

#if defined(DSPL__DEB_VER_STORE_ERRORS)||defined(DSPL__DEB_VER_SHOW_COMMENT)||defined(DSPL__DEBUG_INFO)
#include "..\..\globals.h"
extern const char* log_name;
#endif

int bytes_read=0;
int DS_BUFFERLEN=0;
int DS_PROCPOWER=0;

namespace iomp3{
	int start_blocks=2;

	const unsigned char* local_buffer=NULL;
	char* store=NULL;
	int file_length=0;

	std::string fname;
	LPDIRECTSOUNDBUFFER  sbuffer=NULL; 
	LPDIRECTSOUNDBUFFER  placebo_buffer=NULL; 
	
	bool played=false;
	bool already_stopped=true;
	bool paused=false;

	HANDLE readpipe=NULL;
	HANDLE writepipe=NULL;
	HANDLE h_playthread=NULL; 
	HANDLE h_unpackthread=NULL;
	HANDLE signal=NULL;

	unsigned  unpack_thraddr=0;
	unsigned  play_thraddr=0;

	int killed1=0;
	int killed2=0;
	
	int start_y=0;
	
	BOOL terminate1=FALSE;
	BOOL terminate2=FALSE;

	int half=0;
	struct AUDIO_HEADER header;
	int cnt=0;
	int wpos=0;
	
	DWORD cpos_s=0;

	enum defaults{
		WAITING_SLICE=10,
		WAITING_SLICE_2=100,
		WAITING_SECONDS=3000
	};
};

using namespace iomp3;

//====================
//	Clear sound buffer
//====================
void sound::iomp3::ClearBuffer()
{
    BYTE* Data =NULL;
    BYTE* Data2=NULL;
    DWORD Length=0,Length2=0;
    char* buf=NULL;

    sbuffer->Lock(0,
				  DS_BUFFERLEN,
				  (void**)&Data,
				  &Length,
				  (void**)&Data2,
				  &Length2,
				  0L);

    buf = (char*)Data;

	// if Lock() was successful
    if(buf)
	{
		memset(buf,0,DS_BUFFERLEN);
		sbuffer->Unlock(buf,DS_BUFFERLEN,NULL,0);
		if(readpipe)CloseHandle(readpipe);
		if(writepipe)CloseHandle(writepipe);
		
#ifdef DSPL__DEB_VER_STORE_ERRORS
		if(!CreatePipe(&readpipe,&writepipe,NULL,512*256))
			logFile[log_name]("...[CreatePipe]::[failed creating pipe].\n");
#else
		CreatePipe(&readpipe,&writepipe,NULL,512*256);
#endif
	}
}

inline void sound::iomp3::ProcPower(DWORD res)
{ 
	DS_PROCPOWER=res; 
}

//=======================
//	write to sound buffer
//=======================
void WriteBuffer(int count,int pos)
{
    BYTE* Data =NULL;
    BYTE* Data2=NULL;
    DWORD Length=0,Length2=0;
    char* buf=NULL;
    char* buf_temp=NULL;

    sbuffer->Lock(pos,
				  count,
				  (void**)&Data,
				  &Length,
				  (void**)&Data2,
				  &Length2,
				  0L);

	buf = (char*)Data;

	// if Lock() was successful
    if(buf)
	{
		DWORD readed=0;
		
		ReadFile(readpipe,
				 buf,
				 count,
				 &readed,
				 NULL);

		//Hm...Hm... clearing 2 first blocks from sounds.
		//Have to find correct way to fix sound clicks 
		if(start_blocks-- > 0)
			memset(buf,0,count);

		sbuffer->Unlock(buf,count,NULL,0);
	}
}

//==========================
//	Set new volume for music
//==========================
void sound::iomp3::VolumeMusic(int volume)
{
    if(volume<DSBVOLUME_MIN)
		volume=DSBVOLUME_MIN;

    if(volume>DSBVOLUME_MAX)
		volume=DSBVOLUME_MAX;

    sbuffer->SetVolume(volume);
}

//=====================
//	Adjust music volume
//=====================
void sound::iomp3::AdjustVolumeMusic(int value)
{
    LONG volume=0;

    if(SUCCEEDED(sbuffer->GetVolume(&volume)))
    {
        volume+=value;
        if(volume<DSBVOLUME_MIN)
			volume=DSBVOLUME_MIN;
        if(volume>DSBVOLUME_MAX)
			volume=DSBVOLUME_MAX;

        sbuffer->SetVolume(volume);
    }
}

//====================
//	Write to data pipe
//====================
void sndPrintOut(void)
{
    DWORD written;

    if(nch==2)
        WriteFile(writepipe,
				  stereo_samples,
				  2*2*32*18,
				  &written,
				  NULL);
    else
        WriteFile(writepipe,
				  mono_samples,
				  2*32*18,
				  &written,
				  NULL);
}

//====================
//	Process MP3 header
//====================
int __inline sndProcessHeader(struct AUDIO_HEADER *header, int cnt)
{
    int g = gethdr(header);

    if(g!=0)
		return 1;
    if(header->protection_bit==0)
		getcrc();

    return 0;
}

//==================
//	Unpacking thread
//==================
unsigned int _stdcall sndUnpack(void *dummy)
{
	DWORD res=0;

    while(!terminate2)
    {
		WaitForSingleObject(signal,INFINITE);

		Sleep(DS_PROCPOWER);

		if(bytes_read>=file_length)
		{
			cnt = 0;
			in_file = store;
			bytes_read=0;
		}

#ifdef DSPL__DEB_VER_SHOW_COMMENT	
		if(sndProcessHeader(&header,cnt))
		{
			logFile[log_name]("[music]:[error]:error processing header.skipping frame...\n");
			continue;
		}
		if(layer3_frame(&header,cnt))    
			logFile[log_name]("[music]:[error]:error processing layer3 frame.skipping frame...\n");
#else
		sndProcessHeader(&header,cnt);
		layer3_frame(&header,cnt);
#endif
		cnt++;
    }

    killed2=1;
	return res;
}

//================================
//	Filling sound buffer with data
//================================
void sndFillHalf()
{
    int to_fill=(DS_BUFFERLEN/2)/(2*32*18*nch);

    for(int i=0;i<to_fill;i++)
    {
        if(nch==2)
        {
            WriteBuffer(2*32*18*2,wpos+(half-1)*(DS_BUFFERLEN)/2);
            wpos+=2*32*18*2;
        }
        else
        {
            WriteBuffer(2*32*18,wpos+(half-1)*(DS_BUFFERLEN)/2);
            wpos+=2*32*18;
        }

		if(wpos>(DS_BUFFERLEN)/2)
			wpos=0;
    }

    wpos=0;
}

//===================================
//	Localize current playing position
//===================================
int sndTick()
{
    DWORD pos=0;

    if(sbuffer->GetCurrentPosition(&pos,NULL)!=DS_OK)
		pos=0;

	if(half==1)
	{
		if(pos<DS_BUFFERLEN/2)
		{
	        half=2;
		    sndFillHalf();
			return 1;
		}
		return 0;
	}

	if(half==2)
	{
		if(pos>=DS_BUFFERLEN/2)
		{
	        half=1;
		    sndFillHalf();
			return 1;
		}
		return 0;
	}

    return 0;
}

//================
//	Playing thread
//================
unsigned int _stdcall sndPlayer(void *dummy)
{
	DWORD res=0;

    while(!terminate1)
    {
		WaitForSingleObject(signal,INFINITE);
		while(!sndTick())
			Sleep(WAITING_SLICE_2);

        if(!start_y)
        {
           sbuffer->Play(0,0,DSBPLAY_LOOPING);
           start_y=1;
        }
    }
    killed1=1;
	return res;
}

//===============================
//	Pause currently playing music
//===============================
void sound::iomp3::PauseMusic()
{
	// dont allow to pause already paused source
	// it may cause unwanted results from SuspendThread
	if(paused)return;
	paused=true;
	// set back priority to normal
	SetThreadPriority(h_unpackthread,THREAD_PRIORITY_NORMAL);
	SetThreadPriority(h_playthread,THREAD_PRIORITY_NORMAL);

    SuspendThread(h_playthread);
    SuspendThread(h_unpackthread);

    if(sbuffer->GetCurrentPosition(&cpos_s,NULL)!=DS_OK)cpos_s=0;

    sbuffer->Stop();
}

//=====================
//	Resume paused music
//=====================
void sound::iomp3::ResumeMusic()
{
	// dont allow to resumed non-paused source
	// it may cause unwanted results from ResumeThread
	if(!paused)return;
	paused=false;
	// rise up priority to play music
	SetThreadPriority(h_unpackthread,THREAD_PRIORITY_HIGHEST);
	SetThreadPriority(h_playthread,THREAD_PRIORITY_HIGHEST);		

    ResumeThread(h_playthread);
    ResumeThread(h_unpackthread);

    sbuffer->SetCurrentPosition(cpos_s);
    sbuffer->Play(0,0,DSBPLAY_LOOPING);
}

//==============================
//	Stop currently playing music
//==============================
void sound::iomp3::StopMusic()
{
	if(already_stopped)return;
	already_stopped=true;
	// set back priority to normal
	SetThreadPriority(h_unpackthread,THREAD_PRIORITY_NORMAL);
	SetThreadPriority(h_playthread,THREAD_PRIORITY_NORMAL);

#ifdef DSPL__DEB_VER_SHOW_COMMENT	
	logFile[log_name]("..[music]:[stop].\n");
#endif

	ResetEvent(signal);

    sbuffer->Stop();
	sbuffer->SetCurrentPosition(0L);
}

//========================================
//	Terminate unpacking and playing thread
//========================================
void sound::iomp3::KillMusic()
{
    if(terminate1||terminate2)
		return;

#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("killing music threads...\n");
#endif
	DWORD msecs=0;

    terminate2=TRUE;
    while(!killed2)
	{
		Sleep(WAITING_SLICE);
		msecs+=WAITING_SLICE;
		// if during 3 seconds hasn't finished immediate terminating
		if(msecs>=WAITING_SECONDS)
		{
			DWORD exit_code=0; 
#ifdef DSPL__DEB_VER_SHOW_COMMENT
			logFile[log_name]("unpacking thread has spent it's exiting time...terminating...\n");
			if(!TerminateThread(h_unpackthread,exit_code))
				logFile[log_name]("failed terminating unpacking thread...\n");
#else
			TerminateThread(h_unpackthread,exit_code);
#endif
			CloseHandle(h_unpackthread);
			killed2=1;
		}
	}

	msecs=0;
    terminate1=TRUE;
    while(!killed1)
	{
		Sleep(WAITING_SLICE);
		msecs+=WAITING_SLICE;
		// if during 3 seconds hasn't finished immediate terminating
		if(msecs>=WAITING_SECONDS)
		{
			DWORD exit_code=0; 
#ifdef DSPL__DEB_VER_SHOW_COMMENT
			logFile[log_name]("playing thread has spent it's exiting time...terminating...\n");
			if(!TerminateThread(h_playthread,exit_code))
				logFile[log_name]("failed terminating playing thread...\n");
#else
			TerminateThread(h_playthread,exit_code);
#endif
			CloseHandle(h_playthread);
			killed1=1;
		}
	}
    sbuffer->Stop();
}

//================
//	Close MP3 pipe
//================
void sound::iomp3::DisconnectMP3Pipe()
{
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("disconnecting MP3 pipe...\n");
#endif
    if(sbuffer)
    {
		sbuffer->SetVolume(DSBVOLUME_MIN);
		// if music paused, then resume it to escape 'forever' waiting
		sound::iomp3::ResumeMusic();

		if(already_stopped)
		{
			SetEvent(signal);
			//Hm...Hm...move play cursor 
			sbuffer->Play(0,0,DSBPLAY_LOOPING);
		}
		iomp3::KillMusic();
		DataMgr::Release(fname.c_str());
		local_buffer=NULL;
    }

	// sbuffer and placebo_buffer point to the same DirectSoundBuffer object
	if(placebo_buffer)
		placebo_buffer->Release(); 
	placebo_buffer=NULL; 
	sbuffer=NULL;

	if(signal)
		CloseHandle(signal);

	if(readpipe)
		CloseHandle(readpipe);

	if(writepipe)
		CloseHandle(writepipe);

#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("...MP3 pipe is disconnected.\n");
#endif
}

//===============
//	Play MP3 file
//===============
int sound::iomp3::PlayMusic(const std::string& infile,sound::sdsrc* source)
{
    DWORD len=0;

	if(!already_stopped)
		return IOMP3_BUSY;

	// if user tries to play again currently stopped file,
	// then cancel loading file into memory and just re-initialize
	// your decoding variables.

	// attempt to play new file
	if(fname!=infile)
	{
#ifdef DSPL__DEB_VER_SHOW_COMMENT
		logFile[log_name]("..[music]:[playing][%s,id=%d].\n",infile.c_str(),source->get_id());
#endif
		// if we didnt release resources for previous file
		if(local_buffer) 
		{
			DataMgr::Release(fname.c_str());
			local_buffer=NULL;
		}
		
		VFile* VFile = DataMgr::Load(infile.c_str());
		
		len=VFile->Size();
		if(!len)
		{
			// failed loading file
#ifdef DSPL__DEB_VER_SHOW_COMMENT
			logFile[log_name]("..[music]:[failed loading file %s]::[canceling].\n",infile.c_str());
#endif
			DataMgr::Release(infile.c_str());local_buffer=NULL;
			return IOMP3_FAILED;
		}
		else 
			local_buffer=VFile->Data();

		// new file will be played
		fname=infile;

		// remember the length of the file
		file_length=len;

		// use pointer to the original buffer
		// Note: do not destruct sbuffer: source will make it by itself and sbuffer will be not valid
		// (in that case there is placebo_buffer)
		sbuffer = reinterpret_cast<LPDIRECTSOUNDBUFFER>(source->GetBuf());

		killed1=0;
		killed2=0;
		terminate1=FALSE;
		terminate2=FALSE;

		in_file = (char*)local_buffer;
		store   = in_file;

		half=1;
		wpos=0;
		nch=2;
		start_y=0;
		cnt=0;
		append=0;data=0;
		played=true;

		//  set frequency for the sound buffer
		int searching_for_header=1;
		int frequency=22050;
		while(searching_for_header)
		{
			if(!sndProcessHeader(&header,cnt))
			{
				// found valid header
				searching_for_header=0;
				if(header.ID)
				{
					frequency=38000;
					if(!header.sampling_frequency)frequency=44100;
					else
					{
						if(header.sampling_frequency==1)frequency=48000;
					}
					sbuffer->SetFrequency(frequency);
				}
				else
				{
					frequency=16000;
					if(!header.sampling_frequency)frequency=22050;
					else
					{
						if(header.sampling_frequency==1)frequency=24000;
					}
					sbuffer->SetFrequency(frequency);
				}
			}
			cnt++;
		}
	}
#ifdef DSPL__DEB_VER_SHOW_COMMENT
	else
		logFile[log_name]("..[music]:[playing again].\n");
#endif

	// rise up priority for playing music
	SetThreadPriority(h_unpackthread,THREAD_PRIORITY_HIGHEST);
	SetThreadPriority(h_playthread,THREAD_PRIORITY_HIGHEST);

	half=1;
	wpos=0;
	bytes_read=0;
	cnt=0;
	in_file=store;
	append=0;
	data=0;
	start_blocks=2;
	already_stopped=false;

	ClearBuffer();

	start_y=0;
	// Start both threads
 	SetEvent(signal);

    return IOMP3_SUCCESS;
}

//======================================================
//	Stop and delete from decoder currently playing music
//======================================================
void sound::iomp3::RemoveMusic()
{
	fname="";
	StopMusic();
	sbuffer=placebo_buffer;
}

//==========================
//	Initializing MP3 decoder
//==========================
bool sound::iomp3::InitMP3Pipe()
{
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("Initializing MPEG-3...\n");
#endif
    premultiply();
    imdct_init();
    iomp3::ProcPower(10);
    DS_BUFFERLEN=20*2*2*32*18;

//  Creating placebo buffer
	HRESULT hr=0;

	WAVEFORMATEX wf;

	wf.wFormatTag=WAVE_FORMAT_PCM;
	wf.nChannels=2;
	wf.nSamplesPerSec=22050;
	wf.nAvgBytesPerSec=22050*2*16/8;
	wf.nBlockAlign=2*16/8;
	wf.wBitsPerSample=16;
	
	DSBUFFERDESC    dsbd;
	memset(&dsbd,0,sizeof(DSBUFFERDESC));
	dsbd.dwSize =sizeof(DSBUFFERDESC);
	dsbd.dwFlags=DSBCAPS_CTRLVOLUME|DSBCAPS_GETCURRENTPOSITION2;
	dsbd.dwBufferBytes=20*2*2*32*18;
	dsbd.lpwfxFormat=&wf;

//
//  Initializing MP3 threads
//
	const char* mes[]={"<failed creting placebo buffer for MP3 threads>",
					   "<failed creting unpacking thread>",
					   "<failed creting playing thread>"};

	try{
		if(FAILED(hr=sound::dev->CreateSound(&dsbd,&placebo_buffer)))
			throw mes[0];

		placebo_buffer->SetVolume(DSBVOLUME_MIN);
		
		// create synchronization signal
		if(signal)
			CloseHandle(signal);

		signal = CreateEvent(NULL,
							 TRUE,
							 FALSE,
							 NULL);
		
		// Start unpacking thread:
		// actually, it will be waiting until user call PlayMusic and SetEvent
		if(h_unpackthread)CloseHandle(h_unpackthread);

		h_unpackthread=(HANDLE)_beginthreadex(0,
											  0,
											  sndUnpack,
											  0,
											  0,
											  &unpack_thraddr);
		
		if(!h_unpackthread)
			throw mes[1];
		//SetThreadPriority(h_unpackthread,THREAD_PRIORITY_HIGHEST);
		// Start playing thread:
		// actually, it will be waiting until user call PlayMusic and SetEvent
		if(h_playthread)
			CloseHandle(h_playthread);

		h_playthread = (HANDLE)_beginthreadex(0,
											  0,
											  sndPlayer,
											  0,
											  0,
											  &play_thraddr);
		if(!h_playthread)
		{
			//failed creating playing thread - kill threads
			killed2=1; // playing thread is killed already
			// note: no reason to kill playing thread - it does not exist
			iomp3::KillMusic();
			throw mes[2];
		}
		//SetThreadPriority(h_playthread,THREAD_PRIORITY_HIGHEST);
	}
	catch(const char* exception_message)
	{
#ifdef DSPL__DEB_VER_SHOW_COMMENT
		logFile[log_name]("Initializing MP3::%s happened.\n",exception_message);
#endif
		return false;
	}
	catch(...)
	{
#ifdef DSPL__DEB_VER_SHOW_COMMENT
		logFile[log_name]("Initializing MP3::unknown exception has happened!...\n");
#endif
		return false;
	}

	return true;
}

/***************************************************************************

                                 END OF FILE

***************************************************************************/

