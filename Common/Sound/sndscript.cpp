/*
 * A n t l r  T r a n s l a t i o n  H e a d e r
 *
 * Terence Parr, Will Cohen, and Hank Dietz: 1989-1999
 * Purdue University Electrical Engineering
 * With AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR20
 *
 *   C:\TRAINING\SNDSCR~1\BIN\ANTLR.EXE -CC sndscript.g -glms
 *
 */

#define ANTLR_VERSION	13320
#include "pcctscfg.h"
#include "pccts_stdio.h"
#line 1 "sndscript.g"
#include "tokens.h"

#include "AParser.h"
#include "SndParser.h"
#include "DLexerBase.h"
#include "ATokPtr.h"
#ifndef PURIFY
#define PURIFY(r,s) memset((char *) &(r),'\0',(s));
#endif
#line 14 "sndscript.g"


void
SndParser::start(void)
{
#line 190 "sndscript.g"
	zzRULE;
#line 192 "sndscript.g"
	{
		while ( (LA(1)==T_ID) ) {
#line 192 "sndscript.g"
			snd_body();
		}
	}
#line 194 "sndscript.g"
	
	GiveReport();
#line 197 "sndscript.g"
	zzmatch(T_EOF);
	 consume();
	return;
fail:
	syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
	resynch(setwd1, 0x1);
}

void
SndParser::snd_body(void)
{
#line 200 "sndscript.g"
	zzRULE;
	ANTLRTokenPtr id=NULL;
#line 200 "sndscript.g"
	zzmatch(T_ID);
	
	id = (ANTLRTokenPtr)LT(1);
 consume();
#line 202 "sndscript.g"
	zzmatch(T_LCURLYBRACE);
	
#line 204 "sndscript.g"
	
	BeginSnd(id->getText());
 consume();
#line 207 "sndscript.g"
	{
		while ( (setwd1[LA(1)]&0x2) ) {
#line 207 "sndscript.g"
			snd_desc();
		}
	}
#line 209 "sndscript.g"
	zzmatch(T_RCURLYBRACE);
	
#line 211 "sndscript.g"
	
	EndSnd();
 consume();
	return;
fail:
	syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
	resynch(setwd1, 0x4);
}

void
SndParser::snd_desc(void)
{
#line 216 "sndscript.g"
	zzRULE;
	ANTLRTokenPtr valLOOP=NULL, valUSE3D=NULL, valCTRLPAN=NULL, valCTRLFREQ=NULL, valCTRLVOLUME=NULL, valMAXDISTANCE=NULL, valMINDISTANCE=NULL, valINSIDECONEANGLE=NULL, valOUTSIDECONEANGLE=NULL, valROLLOFF=NULL, valDOPPLERFACTOR=NULL;
	if ( (LA(1)==T_LOOP) ) {
#line 216 "sndscript.g"
		zzmatch(T_LOOP);
		 consume();
#line 216 "sndscript.g"
		zzmatch(T_ABC);
		
		valLOOP = (ANTLRTokenPtr)LT(1);

#line 217 "sndscript.g"
		
		RecLOOP(valLOOP->getText());
 consume();
	}
	else {
		if ( (LA(1)==T_USE3D) ) {
#line 221 "sndscript.g"
			zzmatch(T_USE3D);
			 consume();
#line 221 "sndscript.g"
			zzmatch(T_ABC);
			
			valUSE3D = (ANTLRTokenPtr)LT(1);

#line 222 "sndscript.g"
			
			RecUSE3D(valUSE3D->getText());
 consume();
		}
		else {
			if ( (LA(1)==T_CTRLPAN) ) {
#line 226 "sndscript.g"
				zzmatch(T_CTRLPAN);
				 consume();
#line 226 "sndscript.g"
				zzmatch(T_DECIMALINT);
				
				valCTRLPAN = (ANTLRTokenPtr)LT(1);

#line 227 "sndscript.g"
				
				RecCTRLPAN(atoi(valCTRLPAN->getText()));
 consume();
			}
			else {
				if ( (LA(1)==T_CTRLFREQ) ) {
#line 231 "sndscript.g"
					zzmatch(T_CTRLFREQ);
					 consume();
#line 231 "sndscript.g"
					zzmatch(T_DECIMALINT);
					
					valCTRLFREQ = (ANTLRTokenPtr)LT(1);

#line 232 "sndscript.g"
					
					RecCTRLFREQ(atoi(valCTRLFREQ->getText()));
 consume();
				}
				else {
					if ( (LA(1)==T_CTRLVOLUME) ) {
#line 236 "sndscript.g"
						zzmatch(T_CTRLVOLUME);
						 consume();
#line 236 "sndscript.g"
						zzmatch(T_DECIMALINT);
						
						valCTRLVOLUME = (ANTLRTokenPtr)LT(1);

#line 237 "sndscript.g"
						
						RecCTRLVOLUME(atoi(valCTRLVOLUME->getText()));
 consume();
					}
					else {
						if ( (LA(1)==T_MAXDISTANCE) ) {
#line 241 "sndscript.g"
							zzmatch(T_MAXDISTANCE);
							 consume();
#line 241 "sndscript.g"
							zzsetmatch(TCL_NUM_set);
							
							valMAXDISTANCE = (ANTLRTokenPtr)LT(1);

#line 242 "sndscript.g"
							
							RecMAXDISTANCE(atof(valMAXDISTANCE->getText()));
 consume();
						}
						else {
							if ( (LA(1)==T_MINDISTANCE) ) {
#line 246 "sndscript.g"
								zzmatch(T_MINDISTANCE);
								 consume();
#line 246 "sndscript.g"
								zzsetmatch(TCL_NUM_set);
								
								valMINDISTANCE = (ANTLRTokenPtr)LT(1);

#line 247 "sndscript.g"
								
								RecMINDISTANCE(atof(valMINDISTANCE->getText()));
 consume();
							}
							else {
								if ( (LA(1)==T_INSIDECONEANGLE) ) {
#line 251 "sndscript.g"
									zzmatch(T_INSIDECONEANGLE);
									 consume();
#line 251 "sndscript.g"
									zzsetmatch(TCL_NUM_set);
									
									valINSIDECONEANGLE = (ANTLRTokenPtr)LT(1);

#line 252 "sndscript.g"
									
									RecINSIDECONEANGLE(atof(valINSIDECONEANGLE->getText()));
 consume();
								}
								else {
									if ( (LA(1)==T_OUTSIDECONEANGLE) ) {
#line 256 "sndscript.g"
										zzmatch(T_OUTSIDECONEANGLE);
										 consume();
#line 256 "sndscript.g"
										zzsetmatch(TCL_NUM_set);
										
										valOUTSIDECONEANGLE = (ANTLRTokenPtr)LT(1);

#line 257 "sndscript.g"
										
										RecOUTSIDECONEANGLE(atof(valOUTSIDECONEANGLE->getText()));
 consume();
									}
									else {
										if ( (LA(1)==T_ROLLOFF) ) {
#line 261 "sndscript.g"
											zzmatch(T_ROLLOFF);
											 consume();
#line 261 "sndscript.g"
											zzsetmatch(TCL_NUM_set);
											
											valROLLOFF = (ANTLRTokenPtr)LT(1);

#line 262 "sndscript.g"
											
											RecROLLOFF(atof(valROLLOFF->getText()));
 consume();
										}
										else {
											if ( (LA(1)==T_DOPPLERFACTOR) ) {
#line 266 "sndscript.g"
												zzmatch(T_DOPPLERFACTOR);
												 consume();
#line 266 "sndscript.g"
												zzsetmatch(TCL_NUM_set);
												
												valDOPPLERFACTOR = (ANTLRTokenPtr)LT(1);

#line 267 "sndscript.g"
												
												RecDOPPLERFACTOR(atof(valDOPPLERFACTOR->getText()));
 consume();
											}
											else {FAIL(1,err2,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return;
fail:
	syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
	resynch(setwd1, 0x8);
}
