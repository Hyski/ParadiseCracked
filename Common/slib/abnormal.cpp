/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Class for error messaging in Virtuality Sound Engine
                                                                                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                
#include <dsound.h>
#include "..\precomp.h"
#include "..\..\globals.h"
#include "abnormal.h"

extern const char* log_name;

void Abnormal::PostError(const std::string& FuncName, HRESULT hr,const std::string& MoreInfo,const std::string& ErrFileName,unsigned int LineNum)
{
    std::map<std::string, FailInfo>::iterator p = Bad_Func_Map.find(FuncName);

    if(p != Bad_Func_Map.end())Bad_Func_Map[FuncName].error_num++;
    else
    {
        FailInfo fail_info;
        fail_info.error_num++;

        Bad_Func_Map[FuncName] = fail_info;
    }

    switch(hr)
    {
        case DSERR_GENERIC:
            logFile[log_name]("**** Error **** (!) Failed >>%s<<: an undetermined error occurred inside the DirectSound subsystem,%s.\nFile %s,\nLine:%d\n", FuncName.c_str(), MoreInfo.c_str(), ErrFileName, LineNum);
            break;
        case DSERR_ALLOCATED:
            logFile[log_name]("**** Error **** (!) Failed >>%s<<: probably resources, such as priority level, are already in use by someone, %s. \nFile %s, \nLine:%d\n", FuncName.c_str(), MoreInfo.c_str(), ErrFileName, LineNum);
            break;
        case DSERR_INVALIDPARAM:
            logFile[log_name]("**** Error **** (!) Failed >>%s<<: invalid parameter(s), %s. \nFile %s, \nLine:%d\n", FuncName.c_str(), MoreInfo.c_str(), ErrFileName, LineNum);
            break;
        case DSERR_NOAGGREGATION:
            logFile[log_name]("**** Error **** (!) Failed >>%s<<: object does not support aggregation, %s. \nFile %s, \nLine:%d\n", FuncName.c_str(), MoreInfo.c_str(), ErrFileName, LineNum);
            break;
        case DSERR_NODRIVER:
            logFile[log_name]("**** Error **** (!) Failed >>%s<<: no sound driver is available, %s. \nFile %s, \nLine:%d\n", FuncName.c_str(), MoreInfo.c_str(), ErrFileName, LineNum);
            break;
        case DSERR_OUTOFMEMORY:
            logFile[log_name]("**** Error **** (!) Failed >>%s<<: DirectSound subsystem could not allocate sufficent memory to complete request, %s. \nFile %s, \nLine:%d\n", FuncName.c_str(), MoreInfo.c_str(), ErrFileName, LineNum);
            break;
        case DSERR_INVALIDCALL:
            logFile[log_name]("**** Error **** (!) Failed >>%s<<: function is not valid for current state of object, %s. \nFile %s, \nLine:%d\n", FuncName.c_str(), MoreInfo.c_str(), ErrFileName, LineNum);
            break;
        case DSERR_UNINITIALIZED:
            logFile[log_name]("**** Error **** (!) Failed >>%s<<: method IDirectSound::Initialize didn't succeed, %s. \nFile %s, \nLine:%d\n", FuncName.c_str(), MoreInfo.c_str(), ErrFileName, LineNum);
            break;
        case DSERR_UNSUPPORTED:
            logFile[log_name]("**** Error **** (!) Failed >>%s<<: function has been called at mismatched time, %s.. \nFile %s, \nLine:%dn", FuncName.c_str(), MoreInfo.c_str(), ErrFileName, LineNum);
            break;
        case DSERR_BUFFERLOST:
            logFile[log_name]("**** Error **** (!) Failed >>%s<<: buffer is lost, %s. \nFile %s, \nLine:%d\n", FuncName.c_str(), MoreInfo.c_str(), ErrFileName, LineNum);
            break;
        case DSERR_PRIOLEVELNEEDED:
            logFile[log_name]("**** Error **** (!) Failed >>%s<<: priority is needed, %s. \nFile %s, \nLine:%d\n", FuncName.c_str(), MoreInfo.c_str(), ErrFileName, LineNum);
            break;
        // DisPell Error Classification
        default:  
            logFile[log_name]("**** Error **** (!) Failed >>%s<<: %s.\nFile %s,\nLine:%d\n", FuncName.c_str(), MoreInfo.c_str(), ErrFileName.c_str(), LineNum);
    }
}

void Abnormal::PostNote(const std::string& FuncName,const std::string& MoreInfo,const std::string& ErrFileName,unsigned int LineNum)
{
    std::map<std::string, FailInfo>::iterator p = Bad_Func_Map.find(FuncName);

    if(p != Bad_Func_Map.end())
        Bad_Func_Map[FuncName].notes_num++;
    else
    {
        FailInfo fail_info;
        fail_info.notes_num++;

        Bad_Func_Map[FuncName] = fail_info;
    }

    logFile[log_name]("** Note ** (!) Function >>%s<< note you: %s. \nFile %s, \nLine:%d\n", FuncName.c_str(), MoreInfo.c_str(), ErrFileName, LineNum);
}

void Abnormal::GiveStats() const
{
    std::map<std::string, FailInfo>::const_iterator p;

    int total_errors = 0;
    int total_notes  = 0;
    int bad_func_num = 0;

    logFile[log_name]("\n\nFail Information: \n");
    for(p=Bad_Func_Map.begin(); p!=Bad_Func_Map.end();++p)
    {
        total_errors+=p->second.error_num;
        total_notes +=p->second.notes_num;
        bad_func_num++;
        logFile[log_name]("%\t function >> %s << failed %d times, noted %d times.\n",
                            p->first.c_str(), p->second.error_num, p->second.notes_num);
    }

    logFile[log_name]("\nTotal: %d functions failed or noted smthg, %d errors occured, %d notes occured.\n", 
                            bad_func_num, total_errors, total_notes);
}
