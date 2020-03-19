/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Class for error messaging in Virtuality Sound Engine
                                                                                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                

#include "abnormal.h"

void Abnormal::PostError(std::string FuncName, HRESULT hr, const char* note)
{
    std::map<std::string, FailInfo>::iterator p = Bad_Func_Map.find(FuncName);

    if(p != Bad_Func_Map.end())
        Bad_Func_Map[FuncName].error_num++;
    else
    {
        FailInfo fail_info;
        fail_info.error_num++;

        Bad_Func_Map[FuncName] = fail_info;
    }

#ifdef DSPL__DEBUG_VERSION
    if(!note)
        note = "P.S.:no note is applied";

    //switch(hr)
    //{
        //std::cout << "some kind of error has occured\n";
/*
        case DSERR_GENERIC:
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: an undetermined error occurred inside the DirectSound subsystem, %s.\n", FuncName.c_str(), note);
            break;
        case DSERR_ALLOCATED:
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: probably resources, such as priority level, are already in use by someone, %s.\n", FuncName.c_str(), note);
            break;
        case DSERR_INVALIDPARAM:
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: invalid parameter(s), %s.\n", FuncName.c_str(), note);
            break;
        case DSERR_NOAGGREGATION:
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: object does not support aggregation, %s.\n", FuncName.c_str(), note);
            break;
        case DSERR_NODRIVER:
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: no sound driver is available, %s.\n", FuncName.c_str(), note);
            break;
        case DSERR_OUTOFMEMORY:
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: DirectSound subsystem could not allocate sufficent memory to complete request, %s.\n", FuncName.c_str(), note);
            break;
        case DSERR_INVALIDCALL:
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: function is not valid for current state of object, %s\n", FuncName.c_str(), note);
            break;
        case DSERR_UNINITIALIZED:
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: method IDirectSound::Initialize didn't succeed, %s.\n", FuncName.c_str(), note);
            break;
        case DSERR_UNSUPPORTED:
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: function has been called at mismatched time, %s.\n", FuncName.c_str(), note);
            break;
        case DSERR_BUFFERLOST:
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: buffer is lost, %s\n", FuncName.c_str(), note);
            break;
        case DSERR_PRIOLEVELNEEDED:
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: priority is needed, %s\n", FuncName.c_str(), note);
            break;

        // DisPell Error Classification
        case DSPL__DOESNT_EXIST:
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: DirectSound does not exist, %s\n", FuncName.c_str(), note);
            break;
        default:    
            SoundLog[log_name]("**** Error **** (!) Failed >>%s<<: incorrect or unclassified value, %s.\n", FuncName.c_str(), note);
*/
    //}
#endif
}

void Abnormal::PostNote(const std::string FuncName, const char* note)
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
#ifdef DSPL__DEBUG_VERSION
    //SoundLog[log_name]("** Note ** (!) Function >>%s<< note you: %s.\n", FuncName.c_str(), note);
#endif
}

void Abnormal::GiveStats() const
{
    std::map<std::string, FailInfo>::const_iterator p;

    int total_errors = 0;
    int total_notes  = 0;
    int bad_func_num = 0;

#ifdef DSPL__DEBUG_VERSION
//    SoundLog[log_name]("\n\nFail Information: \n");
#endif
    for(p=Bad_Func_Map.begin(); p!=Bad_Func_Map.end();++p)
    {
        total_errors+=p->second.error_num;
        total_notes +=p->second.notes_num;
        bad_func_num++;
#ifdef DSPL__DEBUG_VERSION
        //SoundLog[log_name]("%\t function >> %s << failed %d times, noted %d times.\n",
        //                    p->first.c_str(), p->second.error_num, p->second.notes_num);
#endif
    }

#ifdef DSPL__DEBUG_VERSION
    //SoundLog[log_name]("\nTotal: %d functions failed or noted smthg, %d errors occured, %d notes occured.\n", 
    //                        bad_func_num, total_errors, total_notes);
#endif
}
