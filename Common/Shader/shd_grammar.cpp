/*
 * A n t l r  T r a n s l a t i o n  H e a d e r
 *
 * Terence Parr, Will Cohen, and Hank Dietz: 1989-1999
 * Purdue University Electrical Engineering
 * With AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR20
 *
 *   C:\PCCTS\BIN\ANTLR.EXE -CC shd_grammar.g -glms
 *
 */
#include "precomp.h"

#define ANTLR_VERSION	13320
#include "pcctscfg.h"
#include "pccts_stdio.h"
#line 1 "shd_grammar.g"
#include "tokens.h"

#ifndef _PRECOMP_H_
//	Naughty: этот файл нигде не используется
//#include "../3d/quat.h"
#include <string>
#endif

    class OutputWrapper;
#define EXCEPTION_HANDLING
#define NUM_SIGNALS 3
#include "AParser.h"
#include "ShdParser.h"
#include "DLexerBase.h"
#include "ATokPtr.h"
#ifndef PURIFY
#define PURIFY(r,s) memset((char *) &(r),'\0',(s));
#endif
#line 20 "shd_grammar.g"

#include "OutWrapper.h"

void ShdParser::
zzdflthandlers( int _signal, int *_retsignal )
{
	*_retsignal = _signal;
}


void
ShdParser::start(int *_retsignal)
{
#line 318 "shd_grammar.g"
	zzRULE;
	int _sva=1;
	int _signal=NoSignal;
	*_retsignal = NoSignal;
#line 318 "shd_grammar.g"
	{
		int zzcnt=1;
		do {
#line 318 "shd_grammar.g"
			shader_desc(&_signal); if (_signal) goto _handler;
		} while ( (LA(1)==T_ID_ONE) );
	}
#line 318 "shd_grammar.g"
	zzmatch_wsig(T_EOF, _handler);
	 consume();
	return;
_handler:
	zzdflthandlers(_signal,_retsignal);
	return;
}

void
ShdParser::shader_desc(int *_retsignal)
{
#line 321 "shd_grammar.g"
	zzRULE;
	int _sva=1;
	int _signal=NoSignal;
	*_retsignal = NoSignal;
#line 322 "shd_grammar.g"
	path(&_signal); if (_signal) goto _handler;
#line 322 "shd_grammar.g"
	zzmatch_wsig(T_LCURLYBRACE, _handler);
	
#line 323 "shd_grammar.g"
	
	p_output->ResetErrFlag();
	BegShader(GetPath());
 consume();
#line 329 "shd_grammar.g"
	{
		while ( (setwd1[LA(1)]&0x2) ) {
#line 328 "shd_grammar.g"
			shader_body(&_signal); if (_signal) goto _handler;
		}
	}
#line 330 "shd_grammar.g"
	zzmatch_wsig(T_RCURLYBRACE, _handler);
	
#line 331 "shd_grammar.g"
	
	EndShader();
 consume();
	return;
	/* exception handlers */
_handler:
	switch ( _signal ) {
	case NoSignal: break;  /* MR7 */
	case NoViableAlt :
	case NoSemViableAlt :
	case MismatchedToken :
		
		p_output->OnError(SC_PARSER_ERR);
		consumeUntil(TCL_RESYNC_set);
		break;  /* MR7 */
	default :
		zzdflthandlers(_signal,_retsignal);
	}
	return;
}

void
ShdParser::shader_body(int *_retsignal)
{
#line 344 "shd_grammar.g"
	zzRULE;
	int _sva=1;
	ANTLRTokenPtr stp_type=NULL, imp_type=NULL, mrk_type=NULL, cull_type=NULL, surf_type=NULL, arm=NULL, verts_type=NULL;
	int _signal=NoSignal;
	*_retsignal = NoSignal;
	if ( (LA(1)==T_STEPS) ) {
#line 345 "shd_grammar.g"
		zzmatch_wsig(T_STEPS, _handler);
		 consume();
#line 345 "shd_grammar.g"
		zzsetmatch_wsig(TCL_STEPS_set, _handler);
		
		stp_type = (ANTLRTokenPtr)LT(1);

#line 346 "shd_grammar.g"
		
		SetSteps(stp_type->getType());
 consume();
	}
	else {
		if ( (LA(1)==T_IMPACT) ) {
#line 350 "shd_grammar.g"
			zzmatch_wsig(T_IMPACT, _handler);
			 consume();
#line 350 "shd_grammar.g"
			zzsetmatch_wsig(TCL_IMPACT_set, _handler);
			
			imp_type = (ANTLRTokenPtr)LT(1);

#line 351 "shd_grammar.g"
			
			SetImpact(imp_type->getType());
 consume();
		}
		else {
			if ( (LA(1)==T_MARKS) ) {
#line 355 "shd_grammar.g"
				zzmatch_wsig(T_MARKS, _handler);
				 consume();
#line 355 "shd_grammar.g"
				zzsetmatch_wsig(TCL_MARKS_set, _handler);
				
				mrk_type = (ANTLRTokenPtr)LT(1);

#line 356 "shd_grammar.g"
				
				SetMarks(mrk_type->getType());
 consume();
			}
			else {
				if ( (LA(1)==T_POLYGONOFFSET) ) {
#line 360 "shd_grammar.g"
					zzmatch_wsig(T_POLYGONOFFSET, _handler);
					
#line 361 "shd_grammar.g"
					
					SetPolygonOffset();
 consume();
				}
				else {
					if ( (LA(1)==T_CULL) ) {
#line 365 "shd_grammar.g"
						zzmatch_wsig(T_CULL, _handler);
						 consume();
#line 365 "shd_grammar.g"
						zzsetmatch_wsig(TCL_CULL_set, _handler);
						
						cull_type = (ANTLRTokenPtr)LT(1);

#line 366 "shd_grammar.g"
						
						SetCull(cull_type->getType());
 consume();
					}
					else {
						if ( (LA(1)==T_SURFPARAM) ) {
#line 370 "shd_grammar.g"
							zzmatch_wsig(T_SURFPARAM, _handler);
							 consume();
#line 370 "shd_grammar.g"
							zzsetmatch_wsig(TCL_SURFPARAM_set, _handler);
							
							surf_type = (ANTLRTokenPtr)LT(1);

#line 371 "shd_grammar.g"
							
							SetSurfParam(surf_type->getType());
 consume();
						}
						else {
							if ( (LA(1)==T_ARMOUR) ) {
#line 375 "shd_grammar.g"
								zzmatch_wsig(T_ARMOUR, _handler);
								 consume();
#line 375 "shd_grammar.g"
								zzsetmatch_wsig(TCL_NUM_set, _handler);
								
								arm = (ANTLRTokenPtr)LT(1);

#line 376 "shd_grammar.g"
								
								SetArmour(atof(arm->getText()));
 consume();
							}
							else {
								if ( (LA(1)==T_DEFORMVERTS) ) {
#line 380 "shd_grammar.g"
									zzmatch_wsig(T_DEFORMVERTS, _handler);
									 consume();
#line 380 "shd_grammar.g"
									zzsetmatch_wsig(TCL_DEFORMVERTS_set, _handler);
									
									verts_type = (ANTLRTokenPtr)LT(1);

#line 381 "shd_grammar.g"
									
									SetDeformVerts(verts_type->getType());
 consume();
								}
								else {
									if ( (LA(1)==T_LCURLYBRACE) ) {
#line 385 "shd_grammar.g"
										zzmatch_wsig(T_LCURLYBRACE, _handler);
										
#line 386 "shd_grammar.g"
										
										BegStage();
 consume();
#line 389 "shd_grammar.g"
										{
											int zzcnt=1;
											do {
#line 389 "shd_grammar.g"
												stage_body(&_signal); if (_signal) goto _handler;
											} while ( (setwd1[LA(1)]&0x8) );
										}
#line 390 "shd_grammar.g"
										zzmatch_wsig(T_RCURLYBRACE, _handler);
										
#line 391 "shd_grammar.g"
										
										EndStage();
 consume();
									}
									else {
										if (_sva) _signal=NoViableAlt;
										else _signal=NoSemViableAlt;
										goto _handler;  /* MR7 */
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
_handler:
	zzdflthandlers(_signal,_retsignal);
	return;
}

void
ShdParser::stage_body(int *_retsignal)
{
#line 396 "shd_grammar.g"
	zzRULE;
	int _sva=1;
	ANTLRTokenPtr freq=NULL, map_name=NULL, blnd_arg=NULL, src=NULL, dst=NULL, tcGenFunc=NULL, fnDepth_arg=NULL, emb_arg1=NULL, emb_arg2=NULL, emb_arg3=NULL, rot_arg=NULL, scl_arg1=NULL, scl_arg2=NULL, scrl_arg1=NULL, scrl_arg2=NULL, trb_arg1=NULL, trb_arg2=NULL, trb_arg3=NULL, trb_arg4=NULL, WaveForm=NULL, str_arg1=NULL, str_arg2=NULL, str_arg3=NULL, str_arg4=NULL, zBias=NULL, fill=NULL, alpha=NULL, gen_type=NULL, func_type=NULL, wave=NULL, arg1=NULL, arg2=NULL, arg3=NULL, arg4=NULL;
	int _signal=NoSignal;
	*_retsignal = NoSignal;
	if ( (LA(1)==T_NOMIPMAP) ) {
#line 397 "shd_grammar.g"
		zzmatch_wsig(T_NOMIPMAP, _handler);
		
#line 398 "shd_grammar.g"
		
		SetNoMipMap();
 consume();
	}
	else {
		if ( (LA(1)==T_NOPICMIP) ) {
#line 402 "shd_grammar.g"
			zzmatch_wsig(T_NOPICMIP, _handler);
			
#line 403 "shd_grammar.g"
			
			SetNoPicMip();
 consume();
		}
		else {
			if ( (LA(1)==T_ANIMMAP) ) {
#line 407 "shd_grammar.g"
				zzmatch_wsig(T_ANIMMAP, _handler);
				 consume();
#line 407 "shd_grammar.g"
				zzsetmatch_wsig(TCL_NUM_set, _handler);
				
				freq = (ANTLRTokenPtr)LT(1);

#line 408 "shd_grammar.g"
				
				SetAnimMap(atof(freq->getText()));
 consume();
#line 412 "shd_grammar.g"
				{
					int zzcnt=1;
					do {
#line 412 "shd_grammar.g"
						path(&_signal); if (_signal) goto _handler;
#line 413 "shd_grammar.g"
						
						AddTexture(GetPath());
					} while ( (LA(1)==T_ID_ONE) );
				}
			}
			else {
				if ( (LA(1)==T_CLAMPMAP) ) {
#line 418 "shd_grammar.g"
					zzmatch_wsig(T_CLAMPMAP, _handler);
					 consume();
#line 418 "shd_grammar.g"
					path(&_signal); if (_signal) goto _handler;
#line 419 "shd_grammar.g"
					
					SetMap(GetPath(), true);
				}
				else {
					if ( (LA(1)==T_MAP) ) {
#line 423 "shd_grammar.g"
						zzmatch_wsig(T_MAP, _handler);
						 consume();
#line 425 "shd_grammar.g"
						{
							if ( (LA(1)==T_ID_ONE) ) {
#line 425 "shd_grammar.g"
								path(&_signal); if (_signal) goto _handler;
#line 426 "shd_grammar.g"
								
								SetMap(GetPath());
							}
							else {
								if ( (LA(1)==T_LIGHTMAP) ) {
#line 430 "shd_grammar.g"
									zzmatch_wsig(T_LIGHTMAP, _handler);
									
#line 431 "shd_grammar.g"
									
									SetLightmap();
 consume();
								}
								else {
									if (_sva) _signal=NoViableAlt;
									else _signal=NoSemViableAlt;
									goto _handler;  /* MR7 */
								}
							}
						}
					}
					else {
						if ( (LA(1)==T_BLENDFUNC) ) {
#line 436 "shd_grammar.g"
							zzmatch_wsig(T_BLENDFUNC, _handler);
							 consume();
#line 438 "shd_grammar.g"
							{
								if ( (setwd1[LA(1)]&0x20) ) {
#line 438 "shd_grammar.g"
									zzsetmatch_wsig(TCL_BLEND_SHORTCUTS_set, _handler);
									
									blnd_arg = (ANTLRTokenPtr)LT(1);

#line 439 "shd_grammar.g"
									
									switch(blnd_arg->getType()){
										case T_ADD:
										SetBlendFunc(T_ONE, T_ONE);
										break;
										
                        case T_FILTER:
										SetBlendFunc(T_DST_COLOR, T_ZERO);
										break;
										
                        case T_BLEND:
										SetBlendFunc(T_SRC_ALPHA, T_INV_SRC_ALPHA);
										break;
										
						case T_MODULATE2X:
										SetBlendFunc(T_DST_COLOR, T_SRC_COLOR);
										break;
									}
 consume();
								}
								else {
									if ( (setwd1[LA(1)]&0x40) ) {
#line 459 "shd_grammar.g"
										zzsetmatch_wsig(TCL_BLEND_SRC_set, _handler);
										
										src = (ANTLRTokenPtr)LT(1);
 consume();
#line 459 "shd_grammar.g"
										zzsetmatch_wsig(TCL_BLEND_DST_set, _handler);
										
										dst = (ANTLRTokenPtr)LT(1);

#line 460 "shd_grammar.g"
										
										SetBlendFunc(src->getType(), dst->getType());
 consume();
									}
									else {
										if (_sva) _signal=NoViableAlt;
										else _signal=NoSemViableAlt;
										goto _handler;  /* MR7 */
									}
								}
							}
						}
						else {
							if ( (LA(1)==T_TCGEN) ) {
#line 465 "shd_grammar.g"
								zzmatch_wsig(T_TCGEN, _handler);
								 consume();
#line 465 "shd_grammar.g"
								zzsetmatch_wsig(TCL_TCGEN_FUNC_set, _handler);
								
								tcGenFunc = (ANTLRTokenPtr)LT(1);

#line 466 "shd_grammar.g"
								
								SetTcGenFunc(tcGenFunc->getType());
 consume();
							}
							else {
								if ( (LA(1)==T_DEPTHFUNC) ) {
#line 470 "shd_grammar.g"
									zzmatch_wsig(T_DEPTHFUNC, _handler);
									 consume();
#line 470 "shd_grammar.g"
									zzsetmatch_wsig(TCL_DEPTHFUNC_set, _handler);
									
									fnDepth_arg = (ANTLRTokenPtr)LT(1);

#line 471 "shd_grammar.g"
									
									SetDepthFunc(fnDepth_arg->getType());
 consume();
								}
								else {
									if ( (LA(1)==T_DEPTHWRITE) ) {
#line 475 "shd_grammar.g"
										zzmatch_wsig(T_DEPTHWRITE, _handler);
										
#line 476 "shd_grammar.g"
										
										SetDepthWrite();
 consume();
									}
									else {
										if ( (LA(1)==T_TCMOD) ) {
#line 480 "shd_grammar.g"
											
											float tcMod_arg[4];
#line 483 "shd_grammar.g"
											zzmatch_wsig(T_TCMOD, _handler);
											 consume();
#line 485 "shd_grammar.g"
											{
												if ( (LA(1)==T_EMBOSS) ) {
#line 485 "shd_grammar.g"
													zzmatch_wsig(T_EMBOSS, _handler);
													 consume();
#line 485 "shd_grammar.g"
													zzsetmatch_wsig(TCL_NUM_set, _handler);
													
													emb_arg1 = (ANTLRTokenPtr)LT(1);
 consume();
#line 485 "shd_grammar.g"
													zzsetmatch_wsig(TCL_NUM_set, _handler);
													
													emb_arg2 = (ANTLRTokenPtr)LT(1);
 consume();
#line 485 "shd_grammar.g"
													zzsetmatch_wsig(TCL_NUM_set, _handler);
													
													emb_arg3 = (ANTLRTokenPtr)LT(1);

#line 486 "shd_grammar.g"
													
													tcMod_arg[0] = atof(emb_arg1->getText());
													tcMod_arg[1] = atof(emb_arg2->getText());
													tcMod_arg[2] = atof(emb_arg3->getText());
													
                        SetTcModFunc(T_EMBOSS, tcMod_arg);
 consume();
												}
												else {
													if ( (LA(1)==T_ROTATE) ) {
#line 494 "shd_grammar.g"
														zzmatch_wsig(T_ROTATE, _handler);
														 consume();
#line 494 "shd_grammar.g"
														zzsetmatch_wsig(TCL_NUM_set, _handler);
														
														rot_arg = (ANTLRTokenPtr)LT(1);

#line 495 "shd_grammar.g"
														
														tcMod_arg[0] = atof(rot_arg->getText());
														SetTcModFunc(T_ROTATE,tcMod_arg);
 consume();
													}
													else {
														if ( (LA(1)==T_SCALE) ) {
#line 500 "shd_grammar.g"
															zzmatch_wsig(T_SCALE, _handler);
															 consume();
#line 500 "shd_grammar.g"
															zzsetmatch_wsig(TCL_NUM_set, _handler);
															
															scl_arg1 = (ANTLRTokenPtr)LT(1);
 consume();
#line 500 "shd_grammar.g"
															zzsetmatch_wsig(TCL_NUM_set, _handler);
															
															scl_arg2 = (ANTLRTokenPtr)LT(1);

#line 501 "shd_grammar.g"
															
															tcMod_arg[0] = atof(scl_arg1->getText());
															tcMod_arg[1] = atof(scl_arg2->getText());
															
                        SetTcModFunc(T_SCALE, tcMod_arg);
 consume();
														}
														else {
															if ( (LA(1)==T_SCROLL) ) {
#line 508 "shd_grammar.g"
																zzmatch_wsig(T_SCROLL, _handler);
																 consume();
#line 508 "shd_grammar.g"
																zzsetmatch_wsig(TCL_NUM_set, _handler);
																
																scrl_arg1 = (ANTLRTokenPtr)LT(1);
 consume();
#line 508 "shd_grammar.g"
																zzsetmatch_wsig(TCL_NUM_set, _handler);
																
																scrl_arg2 = (ANTLRTokenPtr)LT(1);

#line 509 "shd_grammar.g"
																
																tcMod_arg[0] = atof(scrl_arg1->getText());
																tcMod_arg[1] = atof(scrl_arg2->getText());
																
                        SetTcModFunc(T_SCROLL, tcMod_arg);
 consume();
															}
															else {
																if ( (LA(1)==T_TURB) ) {
#line 516 "shd_grammar.g"
																	zzmatch_wsig(T_TURB, _handler);
																	 consume();
#line 516 "shd_grammar.g"
																	zzsetmatch_wsig(TCL_NUM_set, _handler);
																	
																	trb_arg1 = (ANTLRTokenPtr)LT(1);
 consume();
#line 516 "shd_grammar.g"
																	zzsetmatch_wsig(TCL_NUM_set, _handler);
																	
																	trb_arg2 = (ANTLRTokenPtr)LT(1);
 consume();
#line 516 "shd_grammar.g"
																	zzsetmatch_wsig(TCL_NUM_set, _handler);
																	
																	trb_arg3 = (ANTLRTokenPtr)LT(1);
 consume();
#line 516 "shd_grammar.g"
																	zzsetmatch_wsig(TCL_NUM_set, _handler);
																	
																	trb_arg4 = (ANTLRTokenPtr)LT(1);

#line 517 "shd_grammar.g"
																	
																	tcMod_arg[0] = atof(trb_arg1->getText());
																	tcMod_arg[1] = atof(trb_arg2->getText());
																	tcMod_arg[2] = atof(trb_arg3->getText());
																	tcMod_arg[3] = atof(trb_arg4->getText());
																	
                        SetTcModFunc(T_TURB, tcMod_arg);
 consume();
																}
																else {
																	if ( (LA(1)==T_STRETCH) ) {
#line 527 "shd_grammar.g"
																		zzmatch_wsig(T_STRETCH, _handler);
																		 consume();
#line 527 "shd_grammar.g"
																		zzsetmatch_wsig(TCL_WAVE_FORM_set, _handler);
																		
																		WaveForm = (ANTLRTokenPtr)LT(1);
 consume();
#line 527 "shd_grammar.g"
																		zzsetmatch_wsig(TCL_NUM_set, _handler);
																		
																		str_arg1 = (ANTLRTokenPtr)LT(1);
 consume();
#line 527 "shd_grammar.g"
																		zzsetmatch_wsig(TCL_NUM_set, _handler);
																		
																		str_arg2 = (ANTLRTokenPtr)LT(1);
 consume();
#line 527 "shd_grammar.g"
																		zzsetmatch_wsig(TCL_NUM_set, _handler);
																		
																		str_arg3 = (ANTLRTokenPtr)LT(1);
 consume();
#line 527 "shd_grammar.g"
																		zzsetmatch_wsig(TCL_NUM_set, _handler);
																		
																		str_arg4 = (ANTLRTokenPtr)LT(1);

#line 528 "shd_grammar.g"
																		
																		tcMod_arg[0] = atof(str_arg1->getText());
																		tcMod_arg[1] = atof(str_arg2->getText());
																		tcMod_arg[2] = atof(str_arg3->getText());
																		tcMod_arg[3] = atof(str_arg4->getText());
																		
                        SetTcModFunc(T_STRETCH, tcMod_arg, WaveForm->getType());
 consume();
																	}
																	else {
																		if (_sva) _signal=NoViableAlt;
																		else _signal=NoSemViableAlt;
																		goto _handler;  /* MR7 */
																	}
																}
															}
														}
													}
												}
											}
										}
										else {
											if ( (LA(1)==T_ZBIAS) ) {
#line 539 "shd_grammar.g"
												zzmatch_wsig(T_ZBIAS, _handler);
												 consume();
#line 539 "shd_grammar.g"
												zzmatch_wsig(T_DECIMALINT, _handler);
												
												zBias = (ANTLRTokenPtr)LT(1);

#line 540 "shd_grammar.g"
												
												SetZBias(atoi(zBias->getText()));
 consume();
											}
											else {
												if ( (LA(1)==T_FILLMODE) ) {
#line 545 "shd_grammar.g"
													zzmatch_wsig(T_FILLMODE, _handler);
													 consume();
#line 545 "shd_grammar.g"
													zzsetmatch_wsig(TCL_FILLMODE_set, _handler);
													
													fill = (ANTLRTokenPtr)LT(1);

#line 546 "shd_grammar.g"
													
													SetFillMode(fill->getType());
 consume();
												}
												else {
													if ( (LA(1)==T_ALPHAFUNC) ) {
#line 550 "shd_grammar.g"
														zzmatch_wsig(T_ALPHAFUNC, _handler);
														 consume();
#line 550 "shd_grammar.g"
														zzsetmatch_wsig(TCL_ALPHAFUNC_set, _handler);
														
														alpha = (ANTLRTokenPtr)LT(1);

#line 551 "shd_grammar.g"
														
														SetAlphaFunc(alpha->getType());
 consume();
													}
													else {
														if ( (setwd1[LA(1)]&0x80) ) {
#line 555 "shd_grammar.g"
															zzsetmatch_wsig(TCL_RA_GEN_set, _handler);
															
															gen_type = (ANTLRTokenPtr)LT(1);
 consume();
#line 557 "shd_grammar.g"
															{
																if ( (setwd2[LA(1)]&0x1) ) {
#line 557 "shd_grammar.g"
																	zzsetmatch_wsig(TCL_RA_FUNC_set, _handler);
																	
																	func_type = (ANTLRTokenPtr)LT(1);

#line 558 "shd_grammar.g"
																	
																	SetRAGen(gen_type->getType(),func_type->getType());
 consume();
																}
																else {
																	if ( (LA(1)==T_WAVE) ) {
#line 562 "shd_grammar.g"
																		zzmatch_wsig(T_WAVE, _handler);
																		 consume();
#line 562 "shd_grammar.g"
																		zzsetmatch_wsig(TCL_WAVE_FORM_set, _handler);
																		
																		wave = (ANTLRTokenPtr)LT(1);
 consume();
#line 562 "shd_grammar.g"
																		zzsetmatch_wsig(TCL_NUM_set, _handler);
																		
																		arg1 = (ANTLRTokenPtr)LT(1);
 consume();
#line 562 "shd_grammar.g"
																		zzsetmatch_wsig(TCL_NUM_set, _handler);
																		
																		arg2 = (ANTLRTokenPtr)LT(1);
 consume();
#line 562 "shd_grammar.g"
																		zzsetmatch_wsig(TCL_NUM_set, _handler);
																		
																		arg3 = (ANTLRTokenPtr)LT(1);
 consume();
#line 562 "shd_grammar.g"
																		zzsetmatch_wsig(TCL_NUM_set, _handler);
																		
																		arg4 = (ANTLRTokenPtr)LT(1);

#line 563 "shd_grammar.g"
																		
																		float arg[4];
																		
                        arg[0] = atof(arg1->getText());
																		arg[1] = atof(arg2->getText());
																		arg[2] = atof(arg3->getText());
																		arg[3] = atof(arg4->getText());
																		
                        SetRAGen( gen_type->getType(), T_WAVE,wave->getType(), arg);
 consume();
																	}
																	else {
																		if (_sva) _signal=NoViableAlt;
																		else _signal=NoSemViableAlt;
																		goto _handler;  /* MR7 */
																	}
																}
															}
														}
														else {
															if (_sva) _signal=NoViableAlt;
															else _signal=NoSemViableAlt;
															goto _handler;  /* MR7 */
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
				}
			}
		}
	}
	return;
_handler:
	zzdflthandlers(_signal,_retsignal);
	return;
}

void
ShdParser::path(int *_retsignal)
{
#line 576 "shd_grammar.g"
	zzRULE;
	int _sva=1;
	ANTLRTokenPtr id1=NULL, id2=NULL;
	int _signal=NoSignal;
	*_retsignal = NoSignal;
#line 577 "shd_grammar.g"
	zzmatch_wsig(T_ID_ONE, _handler);
	
	id1 = (ANTLRTokenPtr)LT(1);

#line 578 "shd_grammar.g"
	
	NewPath(strlwr(id1->getText()));
 consume();
#line 586 "shd_grammar.g"
	{
		while ( (LA(1)==T_ID_TWO) ) {
#line 582 "shd_grammar.g"
			zzmatch_wsig(T_ID_TWO, _handler);
			
			id2 = (ANTLRTokenPtr)LT(1);

#line 583 "shd_grammar.g"
			
			AddPath(strlwr(id2->getText()));
 consume();
		}
	}
	return;
_handler:
	zzdflthandlers(_signal,_retsignal);
	return;
}
