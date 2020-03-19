/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Class for error messaging in Virtuality Sound Engine
                                                                                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                

#ifndef __ABNORMAL_H___
#define __ABNORMAL_H___

#define DSPL__DEBUG_VERSION
#define DSPL__DEB_VER_STORE_ERRORS

#include <map>
#include <string>
#include <windows.h>
#include <iostream>

class Abnormal{
    protected:
        struct FailInfo
        {
            int error_num;          // how many errors occured
            int notes_num;          // how many notes  occured

            FailInfo(int en=0, int nn=0) : error_num(en), notes_num(nn) {};
        };

        std::map<std::string, FailInfo> Bad_Func_Map;
                                // <func_name, num of errors>
    public:

        Abnormal () {};

        ~Abnormal() {};

        void PostError(const std::string FuncName, HRESULT hr=0, const char* note=NULL);
        void PostNote (const std::string FuncName, const char* note=NULL);

        void GiveStats() const;
};

#endif __ABNORMAL_H___