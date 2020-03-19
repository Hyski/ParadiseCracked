/*
 * A n t l r  T r a n s l a t i o n  H e a d e r
 *
 * Terence Parr, Will Cohen, and Hank Dietz: 1989-1999
 * Purdue University Electrical Engineering
 * With AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR20
 *
 *   C:\TRAINING\PARADI~1\COMMON\SLIB\BIN\ANTLR.EXE -CC sndscript.g -glms
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
#line 295 "sndscript.g"
	zzRULE;
#line 297 "sndscript.g"
	{
		while ( (LA(1)==T_LISTENER) ) {
#line 297 "sndscript.g"
			listener_body();
		}
	}
#line 299 "sndscript.g"
	{
		while ( (LA(1)==T_LSRCBRACE) ) {
#line 299 "sndscript.g"
			snd_body();
		}
	}
#line 301 "sndscript.g"
	
	GiveReport();
#line 304 "sndscript.g"
	zzmatch(T_EOF);
	 consume();
	return;
fail:
	syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
	resynch(setwd1, 0x1);
}

void
SndParser::listener_body(void)
{
#line 308 "sndscript.g"
	zzRULE;
#line 309 "sndscript.g"
	zzmatch(T_LISTENER);
	 consume();
#line 312 "sndscript.g"
	zzmatch(T_LCURLYBRACE);
	 consume();
#line 314 "sndscript.g"
	{
		for (;;) {
			if ( !((setwd1[LA(1)]&0x2))) break;
			if ( (setwd1[LA(1)]&0x4) ) {
#line 314 "sndscript.g"
				listener_desc();
			}
			else {
				if ( (setwd1[LA(1)]&0x8) ) {
#line 314 "sndscript.g"
					unexpected();
				}
				else {
					if ( (setwd1[LA(1)]&0x10) ) {
#line 314 "sndscript.g"
						unexpected_for_listener();
					}
					else break; /* MR6 code for exiting loop "for sure" */
				}
			}
		}
	}
#line 316 "sndscript.g"
	zzmatch(T_RCURLYBRACE);
	 consume();
	return;
fail:
	syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
	resynch(setwd1, 0x20);
}

void
SndParser::listener_desc(void)
{
#line 320 "sndscript.g"
	zzRULE;
	ANTLRTokenPtr valROLLOFF=NULL, valDOPPLERFACTOR=NULL, valROOM=NULL, valROOMHF=NULL, valROOMROLLOFF=NULL, valDECAYTIME=NULL, valDECAYHFRATIO=NULL, valREFLECTIONS=NULL, valREFLECTIONSDELAY=NULL, valREVERB=NULL, valREVERBDELAY=NULL, valENVIROMENT=NULL, valENVIROMENTSIZE=NULL, valENVIROMENTDIFFUSION=NULL, valAIRABSOPTION=NULL, valFLAGS=NULL;
	if ( (LA(1)==T_ROLLOFF) ) {
#line 321 "sndscript.g"
		zzmatch(T_ROLLOFF);
		 consume();
#line 321 "sndscript.g"
		zzmatch(T_EQUALITY);
		 consume();
#line 321 "sndscript.g"
		zzsetmatch(TCL_NUM_set);
		
		valROLLOFF = (ANTLRTokenPtr)LT(1);

#line 322 "sndscript.g"
		
		RecROLLOFF(atof(valROLLOFF->getText()));
 consume();
	}
	else {
		if ( (LA(1)==T_DOPPLERFACTOR) ) {
#line 326 "sndscript.g"
			zzmatch(T_DOPPLERFACTOR);
			 consume();
#line 326 "sndscript.g"
			zzmatch(T_EQUALITY);
			 consume();
#line 326 "sndscript.g"
			zzsetmatch(TCL_NUM_set);
			
			valDOPPLERFACTOR = (ANTLRTokenPtr)LT(1);

#line 327 "sndscript.g"
			
			RecDOPPLERFACTOR(atof(valDOPPLERFACTOR->getText()));
 consume();
		}
		else {
			if ( (LA(1)==T_ROOM) ) {
#line 331 "sndscript.g"
				zzmatch(T_ROOM);
				 consume();
#line 331 "sndscript.g"
				zzmatch(T_EQUALITY);
				 consume();
#line 331 "sndscript.g"
				zzmatch(T_DECIMALINT);
				
				valROOM = (ANTLRTokenPtr)LT(1);

#line 332 "sndscript.g"
				
				RecROOM(atoi(valROOM->getText()));
 consume();
			}
			else {
				if ( (LA(1)==T_ROOMHF) ) {
#line 336 "sndscript.g"
					zzmatch(T_ROOMHF);
					 consume();
#line 336 "sndscript.g"
					zzmatch(T_EQUALITY);
					 consume();
#line 336 "sndscript.g"
					zzmatch(T_DECIMALINT);
					
					valROOMHF = (ANTLRTokenPtr)LT(1);

#line 337 "sndscript.g"
					
					RecROOMHF(atoi(valROOMHF->getText()));
 consume();
				}
				else {
					if ( (LA(1)==T_ROOMROLLOFF) ) {
#line 341 "sndscript.g"
						zzmatch(T_ROOMROLLOFF);
						 consume();
#line 341 "sndscript.g"
						zzmatch(T_EQUALITY);
						 consume();
#line 341 "sndscript.g"
						zzsetmatch(TCL_NUM_set);
						
						valROOMROLLOFF = (ANTLRTokenPtr)LT(1);

#line 342 "sndscript.g"
						
						RecROOMROLLOFF(atof(valROOMROLLOFF->getText()));
 consume();
					}
					else {
						if ( (LA(1)==T_DECAYTIME) ) {
#line 346 "sndscript.g"
							zzmatch(T_DECAYTIME);
							 consume();
#line 346 "sndscript.g"
							zzmatch(T_EQUALITY);
							 consume();
#line 346 "sndscript.g"
							zzsetmatch(TCL_NUM_set);
							
							valDECAYTIME = (ANTLRTokenPtr)LT(1);

#line 347 "sndscript.g"
							
							RecDECAYTIME(atof(valDECAYTIME->getText()));
 consume();
						}
						else {
							if ( (LA(1)==T_DECAYHFRATIO) ) {
#line 351 "sndscript.g"
								zzmatch(T_DECAYHFRATIO);
								 consume();
#line 351 "sndscript.g"
								zzmatch(T_EQUALITY);
								 consume();
#line 351 "sndscript.g"
								zzsetmatch(TCL_NUM_set);
								
								valDECAYHFRATIO = (ANTLRTokenPtr)LT(1);

#line 352 "sndscript.g"
								
								RecDECAYHFRATIO(atof(valDECAYHFRATIO->getText()));
 consume();
							}
							else {
								if ( (LA(1)==T_REFLECTIONS) ) {
#line 356 "sndscript.g"
									zzmatch(T_REFLECTIONS);
									 consume();
#line 356 "sndscript.g"
									zzmatch(T_EQUALITY);
									 consume();
#line 356 "sndscript.g"
									zzmatch(T_DECIMALINT);
									
									valREFLECTIONS = (ANTLRTokenPtr)LT(1);

#line 357 "sndscript.g"
									
									RecREFLECTIONS(atoi(valREFLECTIONS->getText()));
 consume();
								}
								else {
									if ( (LA(1)==T_REFLECTIONSDELAY) ) {
#line 361 "sndscript.g"
										zzmatch(T_REFLECTIONSDELAY);
										 consume();
#line 361 "sndscript.g"
										zzmatch(T_EQUALITY);
										 consume();
#line 361 "sndscript.g"
										zzsetmatch(TCL_NUM_set);
										
										valREFLECTIONSDELAY = (ANTLRTokenPtr)LT(1);

#line 362 "sndscript.g"
										
										RecREFLECTIONSDELAY(atof(valREFLECTIONSDELAY->getText()));
 consume();
									}
									else {
										if ( (LA(1)==T_REVERB) ) {
#line 366 "sndscript.g"
											zzmatch(T_REVERB);
											 consume();
#line 366 "sndscript.g"
											zzmatch(T_EQUALITY);
											 consume();
#line 366 "sndscript.g"
											zzmatch(T_DECIMALINT);
											
											valREVERB = (ANTLRTokenPtr)LT(1);

#line 367 "sndscript.g"
											
											RecREVERB(atoi(valREVERB->getText()));
 consume();
										}
										else {
											if ( (LA(1)==T_REVERBDELAY) ) {
#line 371 "sndscript.g"
												zzmatch(T_REVERBDELAY);
												 consume();
#line 371 "sndscript.g"
												zzmatch(T_EQUALITY);
												 consume();
#line 371 "sndscript.g"
												zzsetmatch(TCL_NUM_set);
												
												valREVERBDELAY = (ANTLRTokenPtr)LT(1);

#line 372 "sndscript.g"
												
												RecREVERBDELAY(atof(valREVERBDELAY->getText()));
 consume();
											}
											else {
												if ( (LA(1)==T_ENVIROMENT) ) {
#line 376 "sndscript.g"
													zzmatch(T_ENVIROMENT);
													 consume();
#line 376 "sndscript.g"
													zzmatch(T_EQUALITY);
													 consume();
#line 376 "sndscript.g"
													zzmatch(T_DECIMALINT);
													
													valENVIROMENT = (ANTLRTokenPtr)LT(1);

#line 377 "sndscript.g"
													
													RecENVIROMENT(atoi(valENVIROMENT->getText()));
 consume();
												}
												else {
													if ( (LA(1)==T_ENVIROMENTSIZE) ) {
#line 381 "sndscript.g"
														zzmatch(T_ENVIROMENTSIZE);
														 consume();
#line 381 "sndscript.g"
														zzmatch(T_EQUALITY);
														 consume();
#line 381 "sndscript.g"
														zzsetmatch(TCL_NUM_set);
														
														valENVIROMENTSIZE = (ANTLRTokenPtr)LT(1);

#line 382 "sndscript.g"
														
														RecENVIROMENTSIZE(atof(valENVIROMENTSIZE->getText()));
 consume();
													}
													else {
														if ( (LA(1)==T_ENVIROMENTDIFFUSION) ) {
#line 386 "sndscript.g"
															zzmatch(T_ENVIROMENTDIFFUSION);
															 consume();
#line 386 "sndscript.g"
															zzmatch(T_EQUALITY);
															 consume();
#line 386 "sndscript.g"
															zzsetmatch(TCL_NUM_set);
															
															valENVIROMENTDIFFUSION = (ANTLRTokenPtr)LT(1);

#line 387 "sndscript.g"
															
															RecENVIROMENTDIFFUSION(atof(valENVIROMENTDIFFUSION->getText()));
 consume();
														}
														else {
															if ( (LA(1)==T_AIRABSOPTION) ) {
#line 391 "sndscript.g"
																zzmatch(T_AIRABSOPTION);
																 consume();
#line 391 "sndscript.g"
																zzmatch(T_EQUALITY);
																 consume();
#line 391 "sndscript.g"
																zzsetmatch(TCL_NUM_set);
																
																valAIRABSOPTION = (ANTLRTokenPtr)LT(1);

#line 392 "sndscript.g"
																
																RecAIRABSOPTION(atof(valAIRABSOPTION->getText()));
 consume();
															}
															else {
																if ( (LA(1)==T_FLAGS) ) {
#line 396 "sndscript.g"
																	zzmatch(T_FLAGS);
																	 consume();
#line 396 "sndscript.g"
																	zzmatch(T_EQUALITY);
																	 consume();
#line 396 "sndscript.g"
																	zzmatch(T_DECIMALINT);
																	
																	valFLAGS = (ANTLRTokenPtr)LT(1);

#line 397 "sndscript.g"
																	
																	RecFLAGS(atoi(valFLAGS->getText()));
 consume();
																}
																else {
																	if ( (LA(1)==T_SCALETIME) ) {
#line 401 "sndscript.g"
																		zzmatch(T_SCALETIME);
																		
#line 402 "sndscript.g"
																		
																		RecSCALETIME();
 consume();
																	}
																	else {
																		if ( (LA(1)==T_SCALEREFLECTIONS) ) {
#line 406 "sndscript.g"
																			zzmatch(T_SCALEREFLECTIONS);
																			
#line 407 "sndscript.g"
																			
																			RecSCALEREFLECTIONS();
 consume();
																		}
																		else {
																			if ( (LA(1)==T_SCALEREFLECTIONSDELAY) ) {
#line 411 "sndscript.g"
																				zzmatch(T_SCALEREFLECTIONSDELAY);
																				
#line 412 "sndscript.g"
																				
																				RecSCALEREFLECTIONSDELAY();
 consume();
																			}
																			else {
																				if ( (LA(1)==T_SCALEREVERB) ) {
#line 416 "sndscript.g"
																					zzmatch(T_SCALEREVERB);
																					
#line 417 "sndscript.g"
																					
																					RecSCALEREVERB();
 consume();
																				}
																				else {
																					if ( (LA(1)==T_SCALEREVERBDELAY) ) {
#line 421 "sndscript.g"
																						zzmatch(T_SCALEREVERBDELAY);
																						
#line 422 "sndscript.g"
																						
																						RecSCALEREVERBDELAY();
 consume();
																					}
																					else {
																						if ( (LA(1)==T_CLIPDECAYHF) ) {
#line 426 "sndscript.g"
																							zzmatch(T_CLIPDECAYHF);
																							
#line 427 "sndscript.g"
																							
																							RecCLIPDECAYHF();
 consume();
																						}
																						else {
																							if ( (LA(1)==T_LEFTHCOORDINATESYSTEM) ) {
#line 430 "sndscript.g"
																								zzmatch(T_LEFTHCOORDINATESYSTEM);
																								
#line 431 "sndscript.g"
																								
																								RecLCOORDSYSTEM();
 consume();
																							}
																							else {
																								if ( (LA(1)==T_RIGHTHCOORDINATESYSTEM) ) {
#line 434 "sndscript.g"
																									zzmatch(T_RIGHTHCOORDINATESYSTEM);
																									
#line 435 "sndscript.g"
																									
																									RecRCOORDSYSTEM();
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
fail:
	syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
	resynch(setwd1, 0x40);
}

void
SndParser::unexpected_for_listener(void)
{
#line 441 "sndscript.g"
	zzRULE;
#line 442 "sndscript.g"
	{
		if ( (LA(1)==T_LOOP) ) {
#line 442 "sndscript.g"
			zzmatch(T_LOOP);
			 consume();
		}
		else {
			if ( (LA(1)==T_USE3D) ) {
#line 442 "sndscript.g"
				zzmatch(T_USE3D);
				 consume();
			}
			else {
				if ( (LA(1)==T_MODE) ) {
#line 442 "sndscript.g"
					zzmatch(T_MODE);
					 consume();
				}
				else {
					if ( (LA(1)==T_CTRLPAN) ) {
#line 442 "sndscript.g"
						zzmatch(T_CTRLPAN);
						 consume();
					}
					else {
						if ( (LA(1)==T_MP3) ) {
#line 442 "sndscript.g"
							zzmatch(T_MP3);
							 consume();
						}
						else {
							if ( (LA(1)==T_CTRLFREQ) ) {
#line 443 "sndscript.g"
								zzmatch(T_CTRLFREQ);
								 consume();
							}
							else {
								if ( (LA(1)==T_CTRLVOLUME) ) {
#line 443 "sndscript.g"
									zzmatch(T_CTRLVOLUME);
									 consume();
								}
								else {
									if ( (LA(1)==T_MAXDISTANCE) ) {
#line 443 "sndscript.g"
										zzmatch(T_MAXDISTANCE);
										 consume();
									}
									else {
										if ( (LA(1)==T_MEMORYSTATIC) ) {
#line 443 "sndscript.g"
											zzmatch(T_MEMORYSTATIC);
											 consume();
										}
										else {
											if ( (LA(1)==T_MINDISTANCE) ) {
#line 444 "sndscript.g"
												zzmatch(T_MINDISTANCE);
												 consume();
											}
											else {
												if ( (LA(1)==T_INSIDECONEANGLE) ) {
#line 444 "sndscript.g"
													zzmatch(T_INSIDECONEANGLE);
													 consume();
												}
												else {
													if ( (LA(1)==T_STREAMED) ) {
#line 444 "sndscript.g"
														zzmatch(T_STREAMED);
														 consume();
													}
													else {
														if ( (LA(1)==T_OUTSIDECONEANGLE) ) {
#line 445 "sndscript.g"
															zzmatch(T_OUTSIDECONEANGLE);
															 consume();
														}
														else {
															if ( (LA(1)==T_CONEOUTSIDEVOLUME) ) {
#line 445 "sndscript.g"
																zzmatch(T_CONEOUTSIDEVOLUME);
																 consume();
															}
															else {
																if ( (LA(1)==T_LCURLYBRACE) ) {
#line 445 "sndscript.g"
																	zzmatch(T_LCURLYBRACE);
																	 consume();
#line 446 "sndscript.g"
																	zzmatch(T_SOURCEDIRECT);
																	 consume();
																}
																else {
																	if ( (LA(1)==T_SOURCEDIRECTHF) ) {
#line 446 "sndscript.g"
																		zzmatch(T_SOURCEDIRECTHF);
																		 consume();
																	}
																	else {
																		if ( (LA(1)==T_SOURCEROOM) ) {
#line 446 "sndscript.g"
																			zzmatch(T_SOURCEROOM);
																			 consume();
																		}
																		else {
																			if ( (LA(1)==T_PRIORITY) ) {
#line 446 "sndscript.g"
																				zzmatch(T_PRIORITY);
																				 consume();
																			}
																			else {
																				if ( (LA(1)==T_SOURCEROOMHF) ) {
#line 447 "sndscript.g"
																					zzmatch(T_SOURCEROOMHF);
																					 consume();
																				}
																				else {
																					if ( (LA(1)==T_SOURCEROOLLOFF) ) {
#line 447 "sndscript.g"
																						zzmatch(T_SOURCEROOLLOFF);
																						 consume();
																					}
																					else {
																						if ( (LA(1)==T_SOURCEOUTSIDE) ) {
#line 447 "sndscript.g"
																							zzmatch(T_SOURCEOUTSIDE);
																							 consume();
																						}
																						else {
																							if ( (LA(1)==T_SOURCEABSORPTION) ) {
#line 448 "sndscript.g"
																								zzmatch(T_SOURCEABSORPTION);
																								 consume();
																							}
																							else {
																								if ( (LA(1)==T_SOURCEFLAGS) ) {
#line 448 "sndscript.g"
																									zzmatch(T_SOURCEFLAGS);
																									 consume();
																								}
																								else {
																									if ( (LA(1)==T_SOURCEOBSTRUCTION) ) {
#line 448 "sndscript.g"
																										zzmatch(T_SOURCEOBSTRUCTION);
																										 consume();
																									}
																									else {
																										if ( (LA(1)==T_SOURCEOBSTRUCTIONHF) ) {
#line 449 "sndscript.g"
																											zzmatch(T_SOURCEOBSTRUCTIONHF);
																											 consume();
																										}
																										else {
																											if ( (LA(1)==T_SOURCEOCCLUSION) ) {
#line 449 "sndscript.g"
																												zzmatch(T_SOURCEOCCLUSION);
																												 consume();
																											}
																											else {
																												if ( (LA(1)==T_SOURCEOCCLUSIONLF) ) {
#line 449 "sndscript.g"
																													zzmatch(T_SOURCEOCCLUSIONLF);
																													 consume();
																												}
																												else {
																													if ( (LA(1)==T_SOURCEOCCLUSIONROOM) ) {
#line 450 "sndscript.g"
																														zzmatch(T_SOURCEOCCLUSIONROOM);
																														 consume();
																													}
																													else {
																														if ( (LA(1)==T_POSX) ) {
#line 450 "sndscript.g"
																															zzmatch(T_POSX);
																															 consume();
																														}
																														else {
																															if ( (LA(1)==T_POSY) ) {
#line 450 "sndscript.g"
																																zzmatch(T_POSY);
																																 consume();
																															}
																															else {
																																if ( (LA(1)==T_POSZ) ) {
#line 450 "sndscript.g"
																																	zzmatch(T_POSZ);
																																	 consume();
																																}
																																else {
																																	if ( (LA(1)==T_SOURCEAFFECTDIRECTHF) ) {
#line 451 "sndscript.g"
																																		zzmatch(T_SOURCEAFFECTDIRECTHF);
																																		 consume();
																																	}
																																	else {
																																		if ( (LA(1)==T_SOURCEAFFECTROOM) ) {
#line 451 "sndscript.g"
																																			zzmatch(T_SOURCEAFFECTROOM);
																																			 consume();
																																		}
																																		else {
																																			if ( (LA(1)==T_SOURCEAFFECTROOMHF) ) {
#line 451 "sndscript.g"
																																				zzmatch(T_SOURCEAFFECTROOMHF);
																																				 consume();
																																			}
																																			else {
																																				if ( (LA(1)==T_PERMANENT) ) {
#line 451 "sndscript.g"
																																					zzmatch(T_PERMANENT);
																																					 consume();
																																				}
																																				else {
																																					if ( (LA(1)==T_LOWSRCQUALITY) ) {
#line 452 "sndscript.g"
																																						zzmatch(T_LOWSRCQUALITY);
																																						 consume();
																																					}
																																					else {
																																						if ( (LA(1)==T_MEDSRCQUALITY) ) {
#line 452 "sndscript.g"
																																							zzmatch(T_MEDSRCQUALITY);
																																							 consume();
																																						}
																																						else {
																																							if ( (LA(1)==T_HIGHSRCQUALITY) ) {
#line 452 "sndscript.g"
																																								zzmatch(T_HIGHSRCQUALITY);
																																								 consume();
																																							}
																																							else {
																																								if ( (LA(1)==T_LOWPRIORITY) ) {
#line 453 "sndscript.g"
																																									zzmatch(T_LOWPRIORITY);
																																									 consume();
																																								}
																																								else {
																																									if ( (LA(1)==T_MEDPRIORITY) ) {
#line 453 "sndscript.g"
																																										zzmatch(T_MEDPRIORITY);
																																										 consume();
																																									}
																																									else {
																																										if ( (LA(1)==T_HIGHPRIORITY) ) {
#line 453 "sndscript.g"
																																											zzmatch(T_HIGHPRIORITY);
																																											 consume();
																																										}
																																										else {FAIL(1,err3,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
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
#line 455 "sndscript.g"
	
	ThrowUnknown("unexpected token for listener");
	return;
fail:
	syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
	resynch(setwd1, 0x80);
}

void
SndParser::snd_body(void)
{
#line 462 "sndscript.g"
	zzRULE;
	ANTLRTokenPtr id=NULL;
#line 462 "sndscript.g"
	zzmatch(T_LSRCBRACE);
	 consume();
#line 464 "sndscript.g"
	zzmatch(T_ID);
	
	id = (ANTLRTokenPtr)LT(1);
 consume();
#line 466 "sndscript.g"
	zzmatch(T_RSRCBRACE);
	 consume();
#line 468 "sndscript.g"
	zzmatch(T_LCURLYBRACE);
	
#line 469 "sndscript.g"
	
	BeginSnd(id->getText());
 consume();
#line 473 "sndscript.g"
	{
		for (;;) {
			if ( !((setwd2[LA(1)]&0x1))) break;
			if ( (setwd2[LA(1)]&0x2) ) {
#line 473 "sndscript.g"
				snd_desc();
			}
			else {
				if ( (setwd2[LA(1)]&0x4) ) {
#line 473 "sndscript.g"
					unexpected();
				}
				else {
					if ( (setwd2[LA(1)]&0x8) ) {
#line 473 "sndscript.g"
						unexpected_for_snd();
					}
					else break; /* MR6 code for exiting loop "for sure" */
				}
			}
		}
	}
#line 474 "sndscript.g"
	zzmatch(T_RCURLYBRACE);
	
#line 476 "sndscript.g"
	
	EndSnd();
 consume();
	return;
fail:
	syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
	resynch(setwd2, 0x10);
}

void
SndParser::snd_desc(void)
{
#line 481 "sndscript.g"
	zzRULE;
	ANTLRTokenPtr valMODE=NULL, valNAME=NULL, valCTRLPAN=NULL, valCTRLFREQ=NULL, valCTRLVOLUME=NULL, valMAXDISTANCE=NULL, valMINDISTANCE=NULL, valINSIDECONEANGLE=NULL, valOUTSIDECONEANGLE=NULL, valCONEOUTSIDEVOLUME=NULL, valSOURCEDIRECT=NULL, valSOURCEDIRECTHF=NULL, valSOURCEROLLOFF=NULL, valSOURCEOUTSIDE=NULL, valSOURCEABSORPTION=NULL, valSOURCEFLAGS=NULL, valSOURCEOBSTRUCTION=NULL, valSOURCEOBSTRUCTIONLF=NULL, valSOURCEOCCLUSION=NULL, valSOURCEOCCLUSIONLF=NULL, valSOURCEOCCLUSIONROOM=NULL, valPOSX=NULL, valPOSY=NULL, valPOSZ=NULL, valPRIORITY=NULL;
	if ( (LA(1)==T_LOOP) ) {
#line 481 "sndscript.g"
		zzmatch(T_LOOP);
		
#line 482 "sndscript.g"
		
		RecLOOP();
 consume();
	}
	else {
		if ( (LA(1)==T_USE3D) ) {
#line 486 "sndscript.g"
			zzmatch(T_USE3D);
			
#line 487 "sndscript.g"
			
			RecUSE3D();
 consume();
		}
		else {
			if ( (LA(1)==T_STREAMED) ) {
#line 491 "sndscript.g"
				zzmatch(T_STREAMED);
				
#line 492 "sndscript.g"
				
				RecSTREAMED();
 consume();
			}
			else {
				if ( (LA(1)==T_MP3) ) {
#line 496 "sndscript.g"
					zzmatch(T_MP3);
					
#line 497 "sndscript.g"
					
					RecMP3();
 consume();
				}
				else {
					if ( (LA(1)==T_PERMANENT) ) {
#line 501 "sndscript.g"
						zzmatch(T_PERMANENT);
						
#line 502 "sndscript.g"
						
						RecPERMANENT();
 consume();
					}
					else {
						if ( (LA(1)==T_LOWSRCQUALITY) ) {
#line 506 "sndscript.g"
							zzmatch(T_LOWSRCQUALITY);
							
#line 507 "sndscript.g"
							
							RecLOWSRCQUALITY();
 consume();
						}
						else {
							if ( (LA(1)==T_MEDSRCQUALITY) ) {
#line 511 "sndscript.g"
								zzmatch(T_MEDSRCQUALITY);
								
#line 512 "sndscript.g"
								
								RecMEDSRCQUALITY();
 consume();
							}
							else {
								if ( (LA(1)==T_HIGHSRCQUALITY) ) {
#line 516 "sndscript.g"
									zzmatch(T_HIGHSRCQUALITY);
									
#line 517 "sndscript.g"
									
									RecHIGHSRCQUALITY();
 consume();
								}
								else {
									if ( (LA(1)==T_HIGHPRIORITY) ) {
#line 521 "sndscript.g"
										zzmatch(T_HIGHPRIORITY);
										
#line 522 "sndscript.g"
										
										RecHIGHPRIORITY();
 consume();
									}
									else {
										if ( (LA(1)==T_MEDPRIORITY) ) {
#line 526 "sndscript.g"
											zzmatch(T_MEDPRIORITY);
											
#line 527 "sndscript.g"
											
											RecMEDPRIORITY();
 consume();
										}
										else {
											if ( (LA(1)==T_LOWPRIORITY) ) {
#line 531 "sndscript.g"
												zzmatch(T_LOWPRIORITY);
												
#line 532 "sndscript.g"
												
												RecLOWPRIORITY();
 consume();
											}
											else {
												if ( (LA(1)==T_MODE) ) {
#line 536 "sndscript.g"
													zzmatch(T_MODE);
													 consume();
#line 536 "sndscript.g"
													zzmatch(T_EQUALITY);
													 consume();
#line 536 "sndscript.g"
													zzmatch(T_DECIMALINT);
													
													valMODE = (ANTLRTokenPtr)LT(1);

#line 537 "sndscript.g"
													
													RecMODE(atoi(valMODE->getText()));
 consume();
												}
												else {
													if ( (LA(1)==T_NAME) ) {
#line 541 "sndscript.g"
														zzmatch(T_NAME);
														 consume();
#line 541 "sndscript.g"
														zzmatch(T_EQUALITY);
														 consume();
#line 541 "sndscript.g"
														zzmatch(T_FILENAME);
														
														valNAME = (ANTLRTokenPtr)LT(1);

#line 542 "sndscript.g"
														
														RecNAME(valNAME->getText());
 consume();
													}
													else {
														if ( (LA(1)==T_CTRLPAN) ) {
#line 546 "sndscript.g"
															zzmatch(T_CTRLPAN);
															 consume();
#line 546 "sndscript.g"
															zzmatch(T_EQUALITY);
															 consume();
#line 546 "sndscript.g"
															zzmatch(T_DECIMALINT);
															
															valCTRLPAN = (ANTLRTokenPtr)LT(1);

#line 547 "sndscript.g"
															
															RecCTRLPAN(atoi(valCTRLPAN->getText()));
 consume();
														}
														else {
															if ( (LA(1)==T_CTRLFREQ) ) {
#line 551 "sndscript.g"
																zzmatch(T_CTRLFREQ);
																 consume();
#line 551 "sndscript.g"
																zzmatch(T_EQUALITY);
																 consume();
#line 551 "sndscript.g"
																zzmatch(T_DECIMALINT);
																
																valCTRLFREQ = (ANTLRTokenPtr)LT(1);

#line 552 "sndscript.g"
																
																RecCTRLFREQ(atoi(valCTRLFREQ->getText()));
 consume();
															}
															else {
																if ( (LA(1)==T_CTRLVOLUME) ) {
#line 556 "sndscript.g"
																	zzmatch(T_CTRLVOLUME);
																	 consume();
#line 556 "sndscript.g"
																	zzmatch(T_EQUALITY);
																	 consume();
#line 556 "sndscript.g"
																	zzmatch(T_DECIMALINT);
																	
																	valCTRLVOLUME = (ANTLRTokenPtr)LT(1);

#line 557 "sndscript.g"
																	
																	RecCTRLVOLUME(atoi(valCTRLVOLUME->getText()));
 consume();
																}
																else {
																	if ( (LA(1)==T_MAXDISTANCE) ) {
#line 561 "sndscript.g"
																		zzmatch(T_MAXDISTANCE);
																		 consume();
#line 561 "sndscript.g"
																		zzmatch(T_EQUALITY);
																		 consume();
#line 561 "sndscript.g"
																		zzsetmatch(TCL_NUM_set);
																		
																		valMAXDISTANCE = (ANTLRTokenPtr)LT(1);

#line 562 "sndscript.g"
																		
																		RecMAXDISTANCE(atof(valMAXDISTANCE->getText()));
 consume();
																	}
																	else {
																		if ( (LA(1)==T_MINDISTANCE) ) {
#line 566 "sndscript.g"
																			zzmatch(T_MINDISTANCE);
																			 consume();
#line 566 "sndscript.g"
																			zzmatch(T_EQUALITY);
																			 consume();
#line 566 "sndscript.g"
																			zzsetmatch(TCL_NUM_set);
																			
																			valMINDISTANCE = (ANTLRTokenPtr)LT(1);

#line 567 "sndscript.g"
																			
																			RecMINDISTANCE(atof(valMINDISTANCE->getText()));
 consume();
																		}
																		else {
																			if ( (LA(1)==T_INSIDECONEANGLE) ) {
#line 571 "sndscript.g"
																				zzmatch(T_INSIDECONEANGLE);
																				 consume();
#line 571 "sndscript.g"
																				zzmatch(T_EQUALITY);
																				 consume();
#line 571 "sndscript.g"
																				zzmatch(T_DECIMALINT);
																				
																				valINSIDECONEANGLE = (ANTLRTokenPtr)LT(1);

#line 572 "sndscript.g"
																				
																				RecINSIDECONEANGLE(atoi(valINSIDECONEANGLE->getText()));
 consume();
																			}
																			else {
																				if ( (LA(1)==T_OUTSIDECONEANGLE) ) {
#line 576 "sndscript.g"
																					zzmatch(T_OUTSIDECONEANGLE);
																					 consume();
#line 576 "sndscript.g"
																					zzmatch(T_EQUALITY);
																					 consume();
#line 576 "sndscript.g"
																					zzmatch(T_DECIMALINT);
																					
																					valOUTSIDECONEANGLE = (ANTLRTokenPtr)LT(1);

#line 577 "sndscript.g"
																					
																					RecOUTSIDECONEANGLE(atoi(valOUTSIDECONEANGLE->getText()));
 consume();
																				}
																				else {
																					if ( (LA(1)==T_CONEOUTSIDEVOLUME) ) {
#line 581 "sndscript.g"
																						zzmatch(T_CONEOUTSIDEVOLUME);
																						 consume();
#line 581 "sndscript.g"
																						zzmatch(T_EQUALITY);
																						 consume();
#line 581 "sndscript.g"
																						zzmatch(T_DECIMALINT);
																						
																						valCONEOUTSIDEVOLUME = (ANTLRTokenPtr)LT(1);

#line 582 "sndscript.g"
																						
																						RecOUTSIDECONEANGLE(atoi(valCONEOUTSIDEVOLUME->getText()));
 consume();
																					}
																					else {
																						if ( (LA(1)==T_SOURCEDIRECT) ) {
#line 586 "sndscript.g"
																							zzmatch(T_SOURCEDIRECT);
																							 consume();
#line 586 "sndscript.g"
																							zzmatch(T_EQUALITY);
																							 consume();
#line 586 "sndscript.g"
																							zzmatch(T_DECIMALINT);
																							
																							valSOURCEDIRECT = (ANTLRTokenPtr)LT(1);

#line 587 "sndscript.g"
																							
																							RecSOURCEDIRECT(atoi(valSOURCEDIRECT->getText()));
 consume();
																						}
																						else {
																							if ( (LA(1)==T_SOURCEDIRECTHF) ) {
#line 591 "sndscript.g"
																								zzmatch(T_SOURCEDIRECTHF);
																								 consume();
#line 591 "sndscript.g"
																								zzmatch(T_EQUALITY);
																								 consume();
#line 591 "sndscript.g"
																								zzmatch(T_DECIMALINT);
																								
																								valSOURCEDIRECTHF = (ANTLRTokenPtr)LT(1);

#line 592 "sndscript.g"
																								
																								RecSOURCEDIRECTHF(atoi(valSOURCEDIRECTHF->getText()));
 consume();
																							}
																							else {
																								if ( (LA(1)==T_SOURCEROOLLOFF) ) {
#line 596 "sndscript.g"
																									zzmatch(T_SOURCEROOLLOFF);
																									 consume();
#line 596 "sndscript.g"
																									zzmatch(T_EQUALITY);
																									 consume();
#line 596 "sndscript.g"
																									zzsetmatch(TCL_NUM_set);
																									
																									valSOURCEROLLOFF = (ANTLRTokenPtr)LT(1);

#line 597 "sndscript.g"
																									
																									RecSOURCEROLLOFF(atof(valSOURCEROLLOFF->getText()));
 consume();
																								}
																								else {
																									if ( (LA(1)==T_SOURCEOUTSIDE) ) {
#line 601 "sndscript.g"
																										zzmatch(T_SOURCEOUTSIDE);
																										 consume();
#line 601 "sndscript.g"
																										zzmatch(T_EQUALITY);
																										 consume();
#line 601 "sndscript.g"
																										zzmatch(T_DECIMALINT);
																										
																										valSOURCEOUTSIDE = (ANTLRTokenPtr)LT(1);

#line 602 "sndscript.g"
																										
																										RecSOURCEOUTSIDE(atoi(valSOURCEOUTSIDE->getText()));
 consume();
																									}
																									else {
																										if ( (LA(1)==T_SOURCEABSORPTION) ) {
#line 606 "sndscript.g"
																											zzmatch(T_SOURCEABSORPTION);
																											 consume();
#line 606 "sndscript.g"
																											zzmatch(T_EQUALITY);
																											 consume();
#line 606 "sndscript.g"
																											zzsetmatch(TCL_NUM_set);
																											
																											valSOURCEABSORPTION = (ANTLRTokenPtr)LT(1);

#line 607 "sndscript.g"
																											
																											RecSOURCEABSORPTION(atof(valSOURCEABSORPTION->getText()));
 consume();
																										}
																										else {
																											if ( (LA(1)==T_SOURCEFLAGS) ) {
#line 611 "sndscript.g"
																												zzmatch(T_SOURCEFLAGS);
																												 consume();
#line 611 "sndscript.g"
																												zzmatch(T_EQUALITY);
																												 consume();
#line 611 "sndscript.g"
																												zzmatch(T_DECIMALINT);
																												
																												valSOURCEFLAGS = (ANTLRTokenPtr)LT(1);

#line 612 "sndscript.g"
																												
																												RecSOURCEFLAGS(atoi(valSOURCEFLAGS->getText()));
 consume();
																											}
																											else {
																												if ( (LA(1)==T_SOURCEOBSTRUCTION) ) {
#line 616 "sndscript.g"
																													zzmatch(T_SOURCEOBSTRUCTION);
																													 consume();
#line 616 "sndscript.g"
																													zzmatch(T_EQUALITY);
																													 consume();
#line 616 "sndscript.g"
																													zzmatch(T_DECIMALINT);
																													
																													valSOURCEOBSTRUCTION = (ANTLRTokenPtr)LT(1);

#line 617 "sndscript.g"
																													
																													RecSOURCEOBSTRUCTION(atoi(valSOURCEOBSTRUCTION->getText()));
 consume();
																												}
																												else {
																													if ( (LA(1)==T_SOURCEOBSTRUCTIONHF) ) {
#line 621 "sndscript.g"
																														zzmatch(T_SOURCEOBSTRUCTIONHF);
																														 consume();
#line 621 "sndscript.g"
																														zzmatch(T_EQUALITY);
																														 consume();
#line 621 "sndscript.g"
																														zzsetmatch(TCL_NUM_set);
																														
																														valSOURCEOBSTRUCTIONLF = (ANTLRTokenPtr)LT(1);

#line 622 "sndscript.g"
																														
																														RecSOURCEOBSTRUCTIONLF(atof(valSOURCEOBSTRUCTIONLF->getText()));
 consume();
																													}
																													else {
																														if ( (LA(1)==T_SOURCEOCCLUSION) ) {
#line 626 "sndscript.g"
																															zzmatch(T_SOURCEOCCLUSION);
																															 consume();
#line 626 "sndscript.g"
																															zzmatch(T_EQUALITY);
																															 consume();
#line 626 "sndscript.g"
																															zzmatch(T_DECIMALINT);
																															
																															valSOURCEOCCLUSION = (ANTLRTokenPtr)LT(1);

#line 627 "sndscript.g"
																															
																															RecSOURCEOCCLUSION(atoi(valSOURCEOCCLUSION->getText()));
 consume();
																														}
																														else {
																															if ( (LA(1)==T_SOURCEOCCLUSIONLF) ) {
#line 631 "sndscript.g"
																																zzmatch(T_SOURCEOCCLUSIONLF);
																																 consume();
#line 631 "sndscript.g"
																																zzmatch(T_EQUALITY);
																																 consume();
#line 631 "sndscript.g"
																																zzsetmatch(TCL_NUM_set);
																																
																																valSOURCEOCCLUSIONLF = (ANTLRTokenPtr)LT(1);

#line 632 "sndscript.g"
																																
																																RecSOURCEOCCLUSIONLF(atof(valSOURCEOCCLUSIONLF->getText()));
 consume();
																															}
																															else {
																																if ( (LA(1)==T_SOURCEOCCLUSIONROOM) ) {
#line 636 "sndscript.g"
																																	zzmatch(T_SOURCEOCCLUSIONROOM);
																																	 consume();
#line 636 "sndscript.g"
																																	zzmatch(T_EQUALITY);
																																	 consume();
#line 636 "sndscript.g"
																																	zzsetmatch(TCL_NUM_set);
																																	
																																	valSOURCEOCCLUSIONROOM = (ANTLRTokenPtr)LT(1);

#line 637 "sndscript.g"
																																	
																																	RecSOURCEOCCLUSIONROOM(atof(valSOURCEOCCLUSIONROOM->getText()));
 consume();
																																}
																																else {
																																	if ( (LA(1)==T_POSX) ) {
#line 641 "sndscript.g"
																																		zzmatch(T_POSX);
																																		 consume();
#line 641 "sndscript.g"
																																		zzmatch(T_EQUALITY);
																																		 consume();
#line 641 "sndscript.g"
																																		zzsetmatch(TCL_NUM_set);
																																		
																																		valPOSX = (ANTLRTokenPtr)LT(1);

#line 642 "sndscript.g"
																																		
																																		RecPOSX(atof(valPOSX->getText()));
 consume();
																																	}
																																	else {
																																		if ( (LA(1)==T_POSY) ) {
#line 646 "sndscript.g"
																																			zzmatch(T_POSY);
																																			 consume();
#line 646 "sndscript.g"
																																			zzmatch(T_EQUALITY);
																																			 consume();
#line 646 "sndscript.g"
																																			zzsetmatch(TCL_NUM_set);
																																			
																																			valPOSY = (ANTLRTokenPtr)LT(1);

#line 647 "sndscript.g"
																																			
																																			RecPOSY(atof(valPOSY->getText()));
 consume();
																																		}
																																		else {
																																			if ( (LA(1)==T_POSZ) ) {
#line 651 "sndscript.g"
																																				zzmatch(T_POSZ);
																																				 consume();
#line 651 "sndscript.g"
																																				zzmatch(T_EQUALITY);
																																				 consume();
#line 651 "sndscript.g"
																																				zzsetmatch(TCL_NUM_set);
																																				
																																				valPOSZ = (ANTLRTokenPtr)LT(1);

#line 652 "sndscript.g"
																																				
																																				RecPOSZ(atof(valPOSZ->getText()));
 consume();
																																			}
																																			else {
																																				if ( (LA(1)==T_SOURCEAFFECTDIRECTHF) ) {
#line 656 "sndscript.g"
																																					zzmatch(T_SOURCEAFFECTDIRECTHF);
																																					
#line 657 "sndscript.g"
																																					
																																					RecSOURCEAFFECTDIRECTHF();
 consume();
																																				}
																																				else {
																																					if ( (LA(1)==T_SOURCEAFFECTROOM) ) {
#line 661 "sndscript.g"
																																						zzmatch(T_SOURCEAFFECTROOM);
																																						
#line 662 "sndscript.g"
																																						
																																						RecSOURCEAFFECTROOM();
 consume();
																																					}
																																					else {
																																						if ( (LA(1)==T_SOURCEAFFECTROOMHF) ) {
#line 666 "sndscript.g"
																																							zzmatch(T_SOURCEAFFECTROOMHF);
																																							
#line 667 "sndscript.g"
																																							
																																							RecSOURCEAFFECTROOMHF();
 consume();
																																						}
																																						else {
																																							if ( (LA(1)==T_PRIORITY) ) {
#line 670 "sndscript.g"
																																								zzmatch(T_PRIORITY);
																																								 consume();
#line 670 "sndscript.g"
																																								zzmatch(T_EQUALITY);
																																								 consume();
#line 670 "sndscript.g"
																																								zzsetmatch(TCL_NUM_set);
																																								
																																								valPRIORITY = (ANTLRTokenPtr)LT(1);

#line 671 "sndscript.g"
																																								
																																								RecPRIORITY(atof(valPRIORITY->getText()));
 consume();
																																							}
																																							else {
																																								if ( (LA(1)==T_MEMORYSTATIC) ) {
#line 674 "sndscript.g"
																																									zzmatch(T_MEMORYSTATIC);
																																									
#line 675 "sndscript.g"
																																									
																																									RecMEMORYSTATIC();
 consume();
																																								}
																																								else {FAIL(1,err4,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
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
fail:
	syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
	resynch(setwd2, 0x20);
}

void
SndParser::unexpected_for_snd(void)
{
#line 681 "sndscript.g"
	zzRULE;
#line 682 "sndscript.g"
	{
		if ( (LA(1)==T_ROLLOFF) ) {
#line 682 "sndscript.g"
			zzmatch(T_ROLLOFF);
			 consume();
		}
		else {
			if ( (LA(1)==T_DOPPLERFACTOR) ) {
#line 682 "sndscript.g"
				zzmatch(T_DOPPLERFACTOR);
				 consume();
			}
			else {
				if ( (LA(1)==T_LCURLYBRACE) ) {
#line 682 "sndscript.g"
					zzmatch(T_LCURLYBRACE);
					 consume();
#line 683 "sndscript.g"
					zzmatch(T_ROOM);
					 consume();
				}
				else {
					if ( (LA(1)==T_ROOMHF) ) {
#line 683 "sndscript.g"
						zzmatch(T_ROOMHF);
						 consume();
					}
					else {
						if ( (LA(1)==T_ROOMROLLOFF) ) {
#line 683 "sndscript.g"
							zzmatch(T_ROOMROLLOFF);
							 consume();
						}
						else {
							if ( (LA(1)==T_DECAYTIME) ) {
#line 683 "sndscript.g"
								zzmatch(T_DECAYTIME);
								 consume();
							}
							else {
								if ( (LA(1)==T_DECAYHFRATIO) ) {
#line 684 "sndscript.g"
									zzmatch(T_DECAYHFRATIO);
									 consume();
								}
								else {
									if ( (LA(1)==T_REFLECTIONS) ) {
#line 684 "sndscript.g"
										zzmatch(T_REFLECTIONS);
										 consume();
									}
									else {
										if ( (LA(1)==T_REFLECTIONSDELAY) ) {
#line 684 "sndscript.g"
											zzmatch(T_REFLECTIONSDELAY);
											 consume();
										}
										else {
											if ( (LA(1)==T_REVERB) ) {
#line 685 "sndscript.g"
												zzmatch(T_REVERB);
												 consume();
											}
											else {
												if ( (LA(1)==T_REVERBDELAY) ) {
#line 685 "sndscript.g"
													zzmatch(T_REVERBDELAY);
													 consume();
												}
												else {
													if ( (LA(1)==T_ENVIROMENT) ) {
#line 685 "sndscript.g"
														zzmatch(T_ENVIROMENT);
														 consume();
													}
													else {
														if ( (LA(1)==T_ENVIROMENTSIZE) ) {
#line 685 "sndscript.g"
															zzmatch(T_ENVIROMENTSIZE);
															 consume();
														}
														else {
															if ( (LA(1)==T_ENVIROMENTDIFFUSION) ) {
#line 686 "sndscript.g"
																zzmatch(T_ENVIROMENTDIFFUSION);
																 consume();
															}
															else {
																if ( (LA(1)==T_AIRABSOPTION) ) {
#line 686 "sndscript.g"
																	zzmatch(T_AIRABSOPTION);
																	 consume();
																}
																else {
																	if ( (LA(1)==T_FLAGS) ) {
#line 686 "sndscript.g"
																		zzmatch(T_FLAGS);
																		 consume();
																	}
																	else {
																		if ( (LA(1)==T_SCALETIME) ) {
#line 687 "sndscript.g"
																			zzmatch(T_SCALETIME);
																			 consume();
																		}
																		else {
																			if ( (LA(1)==T_SCALEREFLECTIONS) ) {
#line 687 "sndscript.g"
																				zzmatch(T_SCALEREFLECTIONS);
																				 consume();
																			}
																			else {
																				if ( (LA(1)==T_SCALEREFLECTIONSDELAY) ) {
#line 687 "sndscript.g"
																					zzmatch(T_SCALEREFLECTIONSDELAY);
																					 consume();
																				}
																				else {
																					if ( (LA(1)==T_SCALEREVERB) ) {
#line 688 "sndscript.g"
																						zzmatch(T_SCALEREVERB);
																						 consume();
																					}
																					else {
																						if ( (LA(1)==T_SCALEREVERBDELAY) ) {
#line 688 "sndscript.g"
																							zzmatch(T_SCALEREVERBDELAY);
																							 consume();
																						}
																						else {
																							if ( (LA(1)==T_CLIPDECAYHF) ) {
#line 688 "sndscript.g"
																								zzmatch(T_CLIPDECAYHF);
																								 consume();
																							}
																							else {
																								if ( (setwd2[LA(1)]&0x40) ) {
#line 688 "sndscript.g"
																									zzsetmatch(ENVIRONMENTS_set);
																									 consume();
																								}
																								else {
																									if ( (LA(1)==T_LEFTHCOORDINATESYSTEM) ) {
#line 689 "sndscript.g"
																										zzmatch(T_LEFTHCOORDINATESYSTEM);
																										 consume();
																									}
																									else {
																										if ( (LA(1)==T_RIGHTHCOORDINATESYSTEM) ) {
#line 689 "sndscript.g"
																											zzmatch(T_RIGHTHCOORDINATESYSTEM);
																											 consume();
																										}
																										else {FAIL(1,err6,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
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
#line 691 "sndscript.g"
	
	ThrowUnknown("unexpected token for sound description");
	return;
fail:
	syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
	resynch(setwd2, 0x80);
}

void
SndParser::unexpected(void)
{
#line 696 "sndscript.g"
	zzRULE;
	ANTLRTokenPtr sym=NULL, id=NULL, num=NULL;
	if ( (LA(1)==T_ABC) ) {
#line 696 "sndscript.g"
		zzmatch(T_ABC);
		
		sym = (ANTLRTokenPtr)LT(1);

#line 697 "sndscript.g"
		
		ThrowUnknown(sym->getText());
 consume();
	}
	else {
		if ( (LA(1)==T_ID) ) {
#line 700 "sndscript.g"
			zzmatch(T_ID);
			
			id = (ANTLRTokenPtr)LT(1);

#line 701 "sndscript.g"
			
			ThrowUnknown(id->getText());
 consume();
		}
		else {
			if ( (setwd3[LA(1)]&0x1) ) {
#line 705 "sndscript.g"
				zzsetmatch(TCL_NUM_set);
				
				num = (ANTLRTokenPtr)LT(1);

#line 706 "sndscript.g"
				
				ThrowUnknown(num->getText());
 consume();
			}
			else {FAIL(1,err7,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
		}
	}
	return;
fail:
	syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
	resynch(setwd3, 0x2);
}
