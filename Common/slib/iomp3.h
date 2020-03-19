/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: MP3 connection, based BESTiARY MPEG-3 decoding 
                engine.
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                
#ifndef DSPL_IOMP3_H
#define DSPL_IOMP3_H

namespace iomp3{
    // connect/disconnect
    bool InitMP3Pipe();
    void DisconnectMP3Pipe();

    void ProcPower(DWORD res);
    void ClearBuffer();
    
    void VolumeMusic(int volume);
    void AdjustVolumeMusic(int value);
    
    void StopMusic();
	void KillMusic();
	int  PlayMusic(const std::string& infile,void* source);
    void PauseMusic();
    void ResumeMusic();
    void RemoveMusic();
}

#endif //DSPL_IOMP3_H