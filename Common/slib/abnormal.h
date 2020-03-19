/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Class for error messaging in Virtuality Sound Engine
                                                                                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/                

#ifndef __ABNORMAL_H___
#define __ABNORMAL_H___

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
		
        virtual ~Abnormal() {};
		
        void PostError(const std::string& FuncName, long hr, const std::string& MoreInfo,const std::string& ErrFileName,unsigned int LineNum);
        void PostNote (const std::string& FuncName, const std::string& MoreInfo,const std::string& ErrFileName,unsigned int LineNum);
		
        void GiveStats() const;
};

#endif __ABNORMAL_H___