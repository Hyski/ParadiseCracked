/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Placebo device is void sound device.
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/
#include "..\precomp.h"
#include "sound.h"
#include "placebo.h"

#if defined(DSPL__DEB_VER_STORE_ERRORS)||defined(DSPL__DEB_VER_SHOW_COMMENT)||defined(DSPL__DEBUG_INFO)
#include "..\..\globals.h"
extern const char* log_name;
#endif

sound::sdsrc* sound::Placebo::PurchaseSource() const { return new plsrc; };
sound::sdsrc* sound::Placebo::PurchaseSource(const unsigned int&) const { return new plsrc; };

sound::Placebo::Placebo()
{         
#ifdef DSPL__DEB_VER_STORE_ERRORS
    logFile[log_name]("Using Sound Placebo... Sound log is opened now... \n");
#endif
}

sound::Placebo::~Placebo()
{         
#ifdef DSPL__DEB_VER_STORE_ERRORS
    logFile[log_name]("Deleting Placebo... Closing log... Done.\n");
#endif
}
