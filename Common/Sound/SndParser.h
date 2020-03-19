/*
 * SndParser: P a r s e r  H e a d e r 
 *
 * Generated from: sndscript.g
 *
 * Terence Parr, Russell Quong, Will Cohen, and Hank Dietz: 1989-1999
 * Parr Research Corporation
 * with Purdue University Electrical Engineering
 * with AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR20
 */

#ifndef SndParser_h
#define SndParser_h

#ifndef ANTLR_VERSION
#define ANTLR_VERSION 13320
#endif

#include "AParser.h"


class SndParser : public ANTLRParser {
public:
	static  const ANTLRChar *tokenName(int tk);
protected:
	static  const ANTLRChar *_token_tbl[];
private:

protected:

    //
//these func's will be overriden in SndRecorder
//

	virtual void BeginSnd(const char* name) = 0;
virtual void EndSnd  ()   = 0;

	virtual void GiveReport() const = 0;

	// record parametres for DirectSound
virtual void RecLOOP			    (const char* ans)    = 0;
virtual void RecUSE3D		        (const char* ans)    = 0;
virtual void RecCTRLPAN			    (unsigned int value) = 0;
virtual void RecCTRLFREQ	        (unsigned int value) = 0;
virtual void RecCTRLVOLUME		    (unsigned int value) = 0;
virtual void RecMAXDISTANCE	        (float value)        = 0;
virtual void RecMINDISTANCE		    (float value)        = 0;
virtual void RecINSIDECONEANGLE     (float value)        = 0;
virtual void RecOUTSIDECONEANGLE    (float value)        = 0;
virtual void RecROLLOFF             (float value)        = 0;
virtual void RecDOPPLERFACTOR       (float value)        = 0;
//
protected:
	static SetWordType TCL_NUM_set[8];
	static SetWordType err2[8];
	static SetWordType setwd1[38];
private:
	void zzdflthandlers( int _signal, int *_retsignal );

public:
	SndParser(ANTLRTokenBuffer *input);
	void start(void);
	void snd_body(void);
	void snd_desc(void);
	static SetWordType TCL_SND_LIB_NAME_set[8];
};

#endif /* SndParser_h */
