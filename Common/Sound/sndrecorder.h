#ifndef __SNDRECORDER_H__
#define __SNDRECORDER_H__

#include "SndParser.h"

class SndRecorder : protected SndParser{
public:
    SndRecorder(ANTLRTokenBuffer* p_buff) : SndParser(p_buff) {};
    ~SndRecorder() {};

    void Run();

private:

	void BeginSnd(const char* name);
	void EndSnd();

	// record parametres for DirectSound
	void RecLOOP			    (const char* ans);
	void RecUSE3D		        (const char* ans);
	void RecCTRLPAN			    (unsigned int value);
	void RecCTRLFREQ	        (unsigned int value);
	void RecCTRLVOLUME		    (unsigned int value);
	void RecMAXDISTANCE	        (float value);
	void RecMINDISTANCE		    (float value);
	void RecINSIDECONEANGLE     (float value);
	void RecOUTSIDECONEANGLE    (float value);
    void RecROLLOFF             (float value);
	void RecDOPPLERFACTOR       (float value);
	//
};

#endif __SNDRECORDER_H__