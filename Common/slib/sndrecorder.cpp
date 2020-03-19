/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Sound recorder
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/

#include "tokens.h"
#include "..\3d\quat.h"
#include "sndcompiler.h"
#include "..\precomp.h"
#include "..\..\globals.h"
#include "sndrecorder.h"

extern const char* log_name;

void SndRecorder::Run()
{
    init();
    start();
}

void SndRecorder::BeginSnd(const char* name)
{
    sound::srcdsc srcdesc;

    sound::ListenerDefs._lflags = 0;
    srcdesc._Flags				= 0;
    
    sound::dscmap[name] = srcdesc;
    last_srcdesc = &sound::dscmap[name];
}

void SndRecorder::EndSnd()
{
}

void SndRecorder::GiveReport() const
{
}

void SndRecorder::ThrowUnknown(const char* unknown) const
{
#ifdef DSPL__DEB_VER_SHOW_COMMENT
    logFile[log_name]("unknown token :%s",unknown);
#endif
}


