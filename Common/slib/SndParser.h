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

	virtual void GiveReport  () const = 0;
virtual void ThrowUnknown(const char* unknown) const = 0;

	// record parametres
virtual void RecLOOP			    ()					 = 0;
virtual void RecUSE3D		        ()  				 = 0;
virtual void RecSTREAMED	        ()  				 = 0;
virtual void RecMP3 				()                   = 0;
virtual void RecPERMANENT           ()					 = 0;
virtual void RecLOWSRCQUALITY       ()					 = 0;
virtual void RecMEDSRCQUALITY       ()					 = 0;
virtual void RecHIGHSRCQUALITY      ()					 = 0;
virtual void RecLOWPRIORITY         ()					 = 0;
virtual void RecMEDPRIORITY         ()					 = 0;
virtual void RecHIGHPRIORITY        ()					 = 0;
virtual void RecNAME				(const char* name)   = 0; 
virtual void RecCTRLPAN			    (int value)			 = 0;
virtual void RecCTRLFREQ	        (unsigned int value) = 0;
virtual void RecCTRLVOLUME		    (int value)			 = 0;
virtual void RecMAXDISTANCE	        (float value)        = 0;
virtual void RecMINDISTANCE		    (float value)        = 0;
virtual void RecINSIDECONEANGLE     (unsigned int value) = 0;
virtual void RecOUTSIDECONEANGLE    (unsigned int value) = 0;
virtual void RecCONEOUTSIDEVOLUME   (int value)			 = 0;
virtual void RecROLLOFF             (float value)        = 0;
virtual void RecDOPPLERFACTOR       (float value)        = 0;
virtual void RecMODE				(unsigned int value) = 0;
virtual void RecLCOORDSYSTEM		()        			 = 0;
virtual void RecRCOORDSYSTEM		()		        	 = 0;
virtual void RecPRIORITY			(float value)		 = 0;
virtual void RecMEMORYSTATIC        ()					 = 0;

	virtual void RecSOURCEDIRECT        (int value)          = 0;
virtual void RecSOURCEDIRECTHF		(int value)			 = 0;
virtual void RecSOURCEROOM			(int value)			 = 0;
virtual void RecSOURCEROOMHF		(int value)			 = 0;
virtual void RecSOURCEROLLOFF		(float value)		 = 0;
virtual void RecSOURCEOUTSIDE		(int value)			 = 0;
virtual void RecSOURCEABSORPTION	(float value)		 = 0;
virtual void RecSOURCEFLAGS			(unsigned int value) = 0;
virtual void RecSOURCEOBSTRUCTION	(int value)			 = 0;
virtual void RecSOURCEOBSTRUCTIONLF	(float value)		 = 0;
virtual void RecSOURCEOCCLUSION		(int value)			 = 0;
virtual void RecSOURCEOCCLUSIONLF	(float value)		 = 0;
virtual void RecSOURCEOCCLUSIONROOM	(float value)		 = 0;
virtual void RecPOSX				(float value)		 = 0;
virtual void RecPOSY				(float value)		 = 0;
virtual void RecPOSZ				(float value)		 = 0;
virtual void RecSOURCEAFFECTDIRECTHF()					 = 0;
virtual void RecSOURCEAFFECTROOM	()					 = 0;
virtual void RecSOURCEAFFECTROOMHF	()					 = 0;

	virtual void RecROOM				(int value)          = 0;
virtual void RecROOMHF				(int value)          = 0;
virtual void RecROOMROLLOFF			(float value)        = 0;
virtual void RecDECAYTIME			(float value)        = 0;
virtual void RecDECAYHFRATIO		(float value)        = 0;
virtual void RecREFLECTIONS			(int value)          = 0;
virtual void RecREFLECTIONSDELAY	(float value)        = 0;
virtual void RecREVERB				(int value)          = 0;
virtual void RecREVERBDELAY			(float value)        = 0;
virtual void RecENVIROMENT			(unsigned int value) = 0;
virtual void RecENVIROMENTSIZE		(float value)        = 0;
virtual void RecENVIROMENTDIFFUSION	(float value)        = 0;
virtual void RecAIRABSOPTION		(float value)        = 0;
virtual void RecFLAGS				(unsigned int value) = 0;
virtual void RecSCALETIME			()			         = 0;
virtual void RecSCALEREFLECTIONS	()			         = 0;
virtual void RecSCALEREFLECTIONSDELAY()			         = 0;
virtual void RecSCALEREVERB			()			         = 0;
virtual void RecSCALEREVERBDELAY	()			         = 0;
virtual void RecCLIPDECAYHF			()			         = 0;

  
protected:
	static SetWordType TCL_NUM_set[16];
	static SetWordType err2[16];
	static SetWordType err3[16];
	static SetWordType setwd1[126];
	static SetWordType err4[16];
	static SetWordType ENVIRONMENTS_set[16];
	static SetWordType err6[16];
	static SetWordType setwd2[126];
	static SetWordType err7[16];
	static SetWordType setwd3[126];
private:
	void zzdflthandlers( int _signal, int *_retsignal );

public:
	SndParser(ANTLRTokenBuffer *input);
	void start(void);
	void listener_body(void);
	void listener_desc(void);
	void unexpected_for_listener(void);
	void snd_body(void);
	void snd_desc(void);
	void unexpected_for_snd(void);
	void unexpected(void);
	static SetWordType TCL_SND_LIB_NAME_set[16];
	static SetWordType TCL_RESYNC_set[16];
};

#endif /* SndParser_h */
