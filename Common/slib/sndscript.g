//
//   ANTLR grammar for shader
//
//============================================
//   FILE:  shader.g   
//

//include some defs in ShdParser header
//
#header
<<
>>

<<
>>

#token	T_EOF "@"

#token  T_LCURLYBRACE "\{"
#token  T_RCURLYBRACE "\}"
#token  T_LPAREN      "\("
#token  T_RPAREN      "\)"
#token  T_LSRCBRACE   "\<"
#token  T_RSRCBRACE   "\>"
#token  T_LLSTBRACE   "\["
#token  T_RLSTBRACE   "\]"
#token  T_EQUALITY    "\="

#token  T_LOOP					"loop"	
#token  T_USE3D				    "disable3d"	
#token  T_NAME					"filename"	
#token  T_CTRLPAN				"ctrlpan"	
#token  T_CTRLFREQ				"ctrlfreq"	
#token  T_CTRLVOLUME			"ctrlvolume"	
#token  T_MAXDISTANCE			"maxdistance"	
#token  T_MINDISTANCE			"mindistance"	
#token  T_MODE					"mode"	
#token  T_STREAMED				"streamed"	
#token  T_MP3					"mpeg3"	
#token  T_INSIDECONEANGLE		"insideconeangle"	
#token  T_OUTSIDECONEANGLE		"outsideconeangle"	
#token  T_CONEOUTSIDEVOLUME		"coneoutsidevolume"	
#token  T_ROLLOFF		        "rolloff"	
#token  T_DOPPLERFACTOR         "dopplerfactor"	
#token  T_LEFTHCOORDINATESYSTEM	"LeftHandCoordinateSystem"
#token  T_RIGHTHCOORDINATESYSTEM"RightHandCoordinateSystem"
#token  T_PRIORITY		        "priority"	
#token  T_MEMORYSTATIC	        "static"	
#token  T_PERMANENT				"permanent"	
#token  T_LOWSRCQUALITY			"low_quality"	
#token  T_MEDSRCQUALITY			"medium_quality"	
#token  T_HIGHSRCQUALITY		"high_quality"	
#token  T_LOWPRIORITY			"low_priority"	
#token  T_MEDPRIORITY			"medium_priority"
#token  T_HIGHPRIORITY			"high_priority"	

// 3d enhanced tokens
#token  T_SOURCEDIRECT			"SourceDirect"
#token  T_SOURCEDIRECTHF		"SourceDirectHF"
#token  T_SOURCEROOM			"SourceRoom"
#token  T_SOURCEROOMHF			"SourceRoomHF"
#token  T_SOURCEROOLLOFF		"SourceRolloff"
#token  T_SOURCEOUTSIDE			"SourceOutside"
#token  T_SOURCEABSORPTION		"SourceAbsorption"
#token  T_SOURCEFLAGS			"SourceFlags"
#token  T_SOURCEOBSTRUCTION		"SourceObstruction"
#token  T_SOURCEOBSTRUCTIONHF	"SourceObstructionLF"
#token  T_SOURCEOCCLUSION		"SourceOcclusion"
#token  T_SOURCEOCCLUSIONLF		"SourceOcclusionLF"
#token  T_SOURCEOCCLUSIONROOM	"SourceOcclusionRoom"
#token  T_POSX					"PosX"
#token  T_POSY					"PosY"
#token  T_POSZ					"PosZ"
#token  T_SOURCEAFFECTDIRECTHF	"SourceAffectDirectHF"
#token  T_SOURCEAFFECTROOM		"SourceAffectRoom"
#token  T_SOURCEAFFECTROOMHF	"SourceAffectRoomHF"


// listener enhanced tokens
#token	T_ROOM				    "Room"
#token  T_ROOMHF				"RoomHF"
#token  T_ROOMROLLOFF			"RoomRolloff"
#token  T_DECAYTIME				"DecayTime"
#token  T_DECAYHFRATIO			"DecayHFRatio"
#token  T_REFLECTIONS			"Reflections"
#token  T_REFLECTIONSDELAY		"ReflectionsDelay"
#token  T_REVERB				"Reverb"
#token  T_REVERBDELAY			"ReverbDelay"
#token  T_ENVIROMENT			"Environment"
#token  T_ENVIROMENTSIZE		"EnvironmentSize"
#token  T_ENVIROMENTDIFFUSION	"EnvironmentDiffusion"
#token  T_AIRABSOPTION			"AirAbsorption"
#token  T_FLAGS					"Flags"
#token  T_SCALETIME				"ScaleDecayTime"
#token  T_SCALEREFLECTIONS		"ScaleReflections"
#token  T_SCALEREFLECTIONSDELAY	"ScaleReflectionsDelay"
#token  T_SCALEREVERB			"ScaleReverb"
#token  T_SCALEREVERBDELAY		"ScaleReverbDelay"
#token  T_CLIPDECAYHF			"ClipDecayHF"

#token  T_LISTENER				"camera"	

// EAX tokens
#token T_EAX_ENVIRONMENT_GENERIC         "EAX_ENVIRONMENT_GENERIC"
#token T_EAX_ENVIRONMENT_PADDEDCELL      "EAX_ENVIRONMENT_PADDEDCELL"
#token T_EAX_ENVIRONMENT_ROOM            "EAX_ENVIRONMENT_ROOM"
#token T_EAX_ENVIRONMENT_BATHROOM        "EAX_ENVIRONMENT_BATHROOM"
#token T_EAX_ENVIRONMENT_LIVINGROOM      "EAX_ENVIRONMENT_LIVINGROOM"
#token T_EAX_ENVIRONMENT_STONEROOM       "EAX_ENVIRONMENT_STONEROOM"
#token T_EAX_ENVIRONMENT_AUDITORIUM      "EAX_ENVIRONMENT_AUDITORIUM"
#token T_EAX_ENVIRONMENT_CONCERTHALL     "EAX_ENVIRONMENT_CONCERTHALL"
#token T_EAX_ENVIRONMENT_CAVE            "EAX_ENVIRONMENT_CAVE"
#token T_EAX_ENVIRONMENT_ARENA           "EAX_ENVIRONMENT_ARENA"
#token T_EAX_ENVIRONMENT_HANGAR          "EAX_ENVIRONMENT_HANGAR"
#token T_EAX_ENVIRONMENT_CARPETEDHALLWAY "EAX_ENVIRONMENT_CARPETEDHALLWAY"
#token T_EAX_ENVIRONMENT_HALLWAY         "EAX_ENVIRONMENT_HALLWAY"
#token T_EAX_ENVIRONMENT_STONECORRIDOR   "EAX_ENVIRONMENT_STONECORRIDOR"
#token T_EAX_ENVIRONMENT_ALLEY           "EAX_ENVIRONMENT_ALLEY"
#token T_EAX_ENVIRONMENT_FOREST          "EAX_ENVIRONMENT_FOREST"
#token T_EAX_ENVIRONMENT_CITY            "EAX_ENVIRONMENT_CITY"
#token T_EAX_ENVIRONMENT_MOUNTAINS       "EAX_ENVIRONMENT_MOUNTAINS"
#token T_EAX_ENVIRONMENT_QUARRY          "EAX_ENVIRONMENT_QUARRY"
#token T_EAX_ENVIRONMENT_PLAIN           "EAX_ENVIRONMENT_PLAIN"
#token T_EAX_ENVIRONMENT_PARKINGLOT      "EAX_ENVIRONMENT_PARKINGLOT"
#token T_EAX_ENVIRONMENT_SEWERPIPE       "EAX_ENVIRONMENT_SEWERPIPE"
#token T_EAX_ENVIRONMENT_UNDERWATER      "EAX_ENVIRONMENT_UNDERWATER"
#token T_EAX_ENVIRONMENT_DRUGGED         "EAX_ENVIRONMENT_DRUGGED"
#token T_EAX_ENVIRONMENT_DIZZY           "EAX_ENVIRONMENT_DIZZY"
#token T_EAX_ENVIRONMENT_PSYCHOTIC       "EAX_ENVIRONMENT_PSYCHOTIC"

#token T_DIRECTSOUND_TAG "directsound"
#token T_AUREAL_TAG	     "aureal"  
#token T_EAX_TAG	     "eax"


#token  T_DECIMALINT  "{[\+ \-]}[0-9]+"
#token  T_FLOATONE    "{[\+ \-]}([0-9]+.[0-9]* | [0-9]*.[0-9]+) {[eE]{[\-\+]}[0-9]+}"
#token  T_FLOATTWO    "{[\+ \-]}[0-9]+ [eE]{[\-\+]}[0-9]+" 
#token  T_ABC		  "[a-z]"	

#token  T_ID          "[a-z_][a-z_0-9]*"
#token  T_FILENAME    "\"([a-z]*[0-9]*[_]*[a-z]*[0-9]*[/])*([a-z]*[0-9]*[_]*)+.([w][a][v]|[m][p][3]|[a][c][4])\""

#token  T_NEWLINE     "\n"   <<newline(); skip();>>

#token  "/\*"       << mode(COMMENT); skip();>>
#token  "//~[\n@]*" << skip(); >>                //cpp comments
#token  "[\r\t\ ]+" << skip(); >>                //white space, tabs

#token  T_ERR_TOKEN   "~[\r\n]" 

#lexclass COMMENT
#token  "[\n\r]"		<< skip(); newline(); >>
#token  "\*/"		    << mode(START); skip(); >>
#token  "\*"	    	<< skip(); >>
#token  "~[\*\n\r]+"	<< skip(); >>

//================================================================

#lexclass START

#tokclass TCL_NUM {
    T_DECIMALINT    	T_FLOATONE		T_FLOATTWO
}

#tokclass TCL_SND_LIB_NAME{
	T_DIRECTSOUND_TAG	T_AUREAL_TAG	T_EAX_TAG
}

#tokclass ENVIRONMENTS{
	T_EAX_ENVIRONMENT_GENERIC
	T_EAX_ENVIRONMENT_PADDEDCELL
	T_EAX_ENVIRONMENT_ROOM
	T_EAX_ENVIRONMENT_BATHROOM
	T_EAX_ENVIRONMENT_LIVINGROOM
	T_EAX_ENVIRONMENT_STONEROOM
	T_EAX_ENVIRONMENT_AUDITORIUM
	T_EAX_ENVIRONMENT_CONCERTHALL
	T_EAX_ENVIRONMENT_CAVE
	T_EAX_ENVIRONMENT_ARENA
	T_EAX_ENVIRONMENT_HANGAR
	T_EAX_ENVIRONMENT_CARPETEDHALLWAY
	T_EAX_ENVIRONMENT_HALLWAY
	T_EAX_ENVIRONMENT_STONECORRIDOR
	T_EAX_ENVIRONMENT_ALLEY
	T_EAX_ENVIRONMENT_FOREST
	T_EAX_ENVIRONMENT_CITY
	T_EAX_ENVIRONMENT_MOUNTAINS
	T_EAX_ENVIRONMENT_QUARRY
	T_EAX_ENVIRONMENT_PLAIN
	T_EAX_ENVIRONMENT_PARKINGLOT
	T_EAX_ENVIRONMENT_SEWERPIPE
	T_EAX_ENVIRONMENT_UNDERWATER
	T_EAX_ENVIRONMENT_DRUGGED
	T_EAX_ENVIRONMENT_DIZZY
	T_EAX_ENVIRONMENT_PSYCHOTIC
}

#tokclass TCL_RESYNC {
   T_ID T_EOF   
}

//================================================================
//
//  parser class
//
//================================================================

class SndParser{
<<     
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

>>

start		:  

				(listener_body)*

    		    (snd_body)*

				<<
					GiveReport();
				>>
				T_EOF
			;


listener_body:  
				T_LISTENER
				

				T_LCURLYBRACE 

			    ( listener_desc | unexpected | unexpected_for_listener )* 

			    T_RCURLYBRACE 

		    ;

listener_desc:
				T_ROLLOFF       T_EQUALITY valROLLOFF: TCL_NUM
				<<
					RecROLLOFF(atof(valROLLOFF->getText()));
				>> 

			|	T_DOPPLERFACTOR T_EQUALITY valDOPPLERFACTOR: TCL_NUM
				<<
					RecDOPPLERFACTOR(atof(valDOPPLERFACTOR->getText()));
				>>

        	|	T_ROOM          T_EQUALITY valROOM: T_DECIMALINT
				<<
					RecROOM(atoi(valROOM->getText()));
				>> 

        	|	T_ROOMHF        T_EQUALITY valROOMHF: T_DECIMALINT
				<<
					RecROOMHF(atoi(valROOMHF->getText()));
				>> 

        	|	T_ROOMROLLOFF   T_EQUALITY valROOMROLLOFF: TCL_NUM
				<<
					RecROOMROLLOFF(atof(valROOMROLLOFF->getText()));
				>> 

        	|	T_DECAYTIME	    T_EQUALITY valDECAYTIME: TCL_NUM
				<<
					RecDECAYTIME(atof(valDECAYTIME->getText()));
				>> 

        	|	T_DECAYHFRATIO  T_EQUALITY valDECAYHFRATIO: TCL_NUM
				<<
					RecDECAYHFRATIO(atof(valDECAYHFRATIO->getText()));
				>> 

        	|	T_REFLECTIONS   T_EQUALITY valREFLECTIONS: T_DECIMALINT
				<<
					RecREFLECTIONS(atoi(valREFLECTIONS->getText()));
				>> 

        	|	T_REFLECTIONSDELAY T_EQUALITY valREFLECTIONSDELAY: TCL_NUM
				<<
					RecREFLECTIONSDELAY(atof(valREFLECTIONSDELAY->getText()));
				>> 

        	|	T_REVERB        T_EQUALITY valREVERB: T_DECIMALINT
				<<
					RecREVERB(atoi(valREVERB->getText()));
				>> 

        	|	T_REVERBDELAY   T_EQUALITY valREVERBDELAY: TCL_NUM
				<<
					RecREVERBDELAY(atof(valREVERBDELAY->getText()));
				>> 

        	|	T_ENVIROMENT    T_EQUALITY valENVIROMENT: T_DECIMALINT
				<<
					RecENVIROMENT(atoi(valENVIROMENT->getText()));
				>> 

        	|	T_ENVIROMENTSIZE T_EQUALITY valENVIROMENTSIZE: TCL_NUM
				<<
					RecENVIROMENTSIZE(atof(valENVIROMENTSIZE->getText()));
				>> 

        	|	T_ENVIROMENTDIFFUSION T_EQUALITY valENVIROMENTDIFFUSION: TCL_NUM
				<<
					RecENVIROMENTDIFFUSION(atof(valENVIROMENTDIFFUSION->getText()));
				>> 

        	|	T_AIRABSOPTION  T_EQUALITY valAIRABSOPTION: TCL_NUM
				<<
					RecAIRABSOPTION(atof(valAIRABSOPTION->getText()));
				>> 

        	|	T_FLAGS         T_EQUALITY valFLAGS: T_DECIMALINT
				<<
					RecFLAGS(atoi(valFLAGS->getText()));
				>> 

        	|	T_SCALETIME
				<<
					RecSCALETIME();
				>> 

        	|	T_SCALEREFLECTIONS
				<<
					RecSCALEREFLECTIONS();
				>> 

        	|	T_SCALEREFLECTIONSDELAY
				<<
					RecSCALEREFLECTIONSDELAY();
				>> 

        	|	T_SCALEREVERB
				<<
					RecSCALEREVERB();
				>> 

        	|	T_SCALEREVERBDELAY
				<<
					RecSCALEREVERBDELAY();
				>> 

        	|	T_CLIPDECAYHF
				<<
					RecCLIPDECAYHF();
				>> 
			|   T_LEFTHCOORDINATESYSTEM
				<<
					RecLCOORDSYSTEM();
				>>
			|   T_RIGHTHCOORDINATESYSTEM
				<<
					RecRCOORDSYSTEM();
				>>

			;

unexpected_for_listener:
				( T_LOOP | T_USE3D | T_MODE | T_CTRLPAN | T_MP3 |
				  T_CTRLFREQ | T_CTRLVOLUME | T_MAXDISTANCE | T_MEMORYSTATIC |
				  T_MINDISTANCE | T_INSIDECONEANGLE | T_STREAMED |
				  T_OUTSIDECONEANGLE | T_CONEOUTSIDEVOLUME | T_LCURLYBRACE 
				  T_SOURCEDIRECT | T_SOURCEDIRECTHF | T_SOURCEROOM | T_PRIORITY |
				  T_SOURCEROOMHF | T_SOURCEROOLLOFF | T_SOURCEOUTSIDE |
				  T_SOURCEABSORPTION | T_SOURCEFLAGS | T_SOURCEOBSTRUCTION |
				  T_SOURCEOBSTRUCTIONHF | T_SOURCEOCCLUSION | T_SOURCEOCCLUSIONLF |
				  T_SOURCEOCCLUSIONROOM | T_POSX | T_POSY | T_POSZ |
				  T_SOURCEAFFECTDIRECTHF | T_SOURCEAFFECTROOM | T_SOURCEAFFECTROOMHF | T_PERMANENT |
				  T_LOWSRCQUALITY | T_MEDSRCQUALITY | T_HIGHSRCQUALITY |
				  T_LOWPRIORITY | T_MEDPRIORITY | T_HIGHPRIORITY
				 )
				<<
					ThrowUnknown("unexpected token for listener");
				>>
		
			;
				

snd_body    :   T_LSRCBRACE

				id: T_ID 

				T_RSRCBRACE				

				T_LCURLYBRACE 
				<<
					BeginSnd(id->getText());
				>>

			    ( snd_desc | unexpected | unexpected_for_snd )* 
			    T_RCURLYBRACE 

			    <<
					EndSnd();
			    >>
		    ;

snd_desc    :   T_LOOP
				<<
					RecLOOP();
				>>
				
			|   T_USE3D
				<<
					RecUSE3D();
				>>

			|	T_STREAMED
				<<
					RecSTREAMED();
				>>

			|   T_MP3
				<<
					RecMP3();
				>>

			|   T_PERMANENT
				<<
					RecPERMANENT();
				>>

			|   T_LOWSRCQUALITY
				<<
					RecLOWSRCQUALITY();
				>>

			|   T_MEDSRCQUALITY
				<<
					RecMEDSRCQUALITY();
				>>

			|   T_HIGHSRCQUALITY
				<<
					RecHIGHSRCQUALITY();
				>>

			|   T_HIGHPRIORITY
				<<
					RecHIGHPRIORITY();
				>>

			|   T_MEDPRIORITY
				<<
					RecMEDPRIORITY();
				>>

			|   T_LOWPRIORITY
				<<
					RecLOWPRIORITY();
				>>

			|   T_MODE        T_EQUALITY valMODE:    T_DECIMALINT
				<<
					RecMODE(atoi(valMODE->getText()));
				>>

			|   T_NAME        T_EQUALITY valNAME:    T_FILENAME
				<<
					RecNAME(valNAME->getText());
				>>

			|	T_CTRLPAN     T_EQUALITY valCTRLPAN: T_DECIMALINT
				<<
					RecCTRLPAN(atoi(valCTRLPAN->getText()));
				>>

			|	T_CTRLFREQ    T_EQUALITY valCTRLFREQ: T_DECIMALINT
				<<
					RecCTRLFREQ(atoi(valCTRLFREQ->getText()));
				>>

			|	T_CTRLVOLUME  T_EQUALITY valCTRLVOLUME: T_DECIMALINT
				<<
					RecCTRLVOLUME(atoi(valCTRLVOLUME->getText()));
				>>

			|	T_MAXDISTANCE T_EQUALITY valMAXDISTANCE: TCL_NUM
				<<
					RecMAXDISTANCE(atof(valMAXDISTANCE->getText()));
				>>

			|	T_MINDISTANCE T_EQUALITY valMINDISTANCE: TCL_NUM
				<<
					RecMINDISTANCE(atof(valMINDISTANCE->getText()));
				>>

			|	T_INSIDECONEANGLE   T_EQUALITY valINSIDECONEANGLE: T_DECIMALINT
				<<
					RecINSIDECONEANGLE(atoi(valINSIDECONEANGLE->getText()));
				>>

			|	T_OUTSIDECONEANGLE  T_EQUALITY valOUTSIDECONEANGLE: T_DECIMALINT
				<<
					RecOUTSIDECONEANGLE(atoi(valOUTSIDECONEANGLE->getText()));
				>>

			|	T_CONEOUTSIDEVOLUME T_EQUALITY valCONEOUTSIDEVOLUME: T_DECIMALINT
				<<
					RecOUTSIDECONEANGLE(atoi(valCONEOUTSIDEVOLUME->getText()));
				>>

			|   T_SOURCEDIRECT      T_EQUALITY valSOURCEDIRECT: T_DECIMALINT		
				<<
					RecSOURCEDIRECT(atoi(valSOURCEDIRECT->getText()));
				>>

			|   T_SOURCEDIRECTHF    T_EQUALITY valSOURCEDIRECTHF: T_DECIMALINT		
				<<
					RecSOURCEDIRECTHF(atoi(valSOURCEDIRECTHF->getText()));
				>>

			|   T_SOURCEROOLLOFF    T_EQUALITY valSOURCEROLLOFF: TCL_NUM
				<<
					RecSOURCEROLLOFF(atof(valSOURCEROLLOFF->getText()));
				>>

			|   T_SOURCEOUTSIDE     T_EQUALITY valSOURCEOUTSIDE: T_DECIMALINT		
				<<
					RecSOURCEOUTSIDE(atoi(valSOURCEOUTSIDE->getText()));
				>>

			|   T_SOURCEABSORPTION  T_EQUALITY valSOURCEABSORPTION: TCL_NUM
				<<
					RecSOURCEABSORPTION(atof(valSOURCEABSORPTION->getText()));
				>>

			|   T_SOURCEFLAGS       T_EQUALITY valSOURCEFLAGS: T_DECIMALINT		
				<<
					RecSOURCEFLAGS(atoi(valSOURCEFLAGS->getText()));
				>>

			|   T_SOURCEOBSTRUCTION T_EQUALITY valSOURCEOBSTRUCTION: T_DECIMALINT		
				<<
					RecSOURCEOBSTRUCTION(atoi(valSOURCEOBSTRUCTION->getText()));
				>>

			|   T_SOURCEOBSTRUCTIONHF T_EQUALITY valSOURCEOBSTRUCTIONLF: TCL_NUM
				<<
					RecSOURCEOBSTRUCTIONLF(atof(valSOURCEOBSTRUCTIONLF->getText()));
				>>

			|   T_SOURCEOCCLUSION     T_EQUALITY valSOURCEOCCLUSION: T_DECIMALINT		
				<<
					RecSOURCEOCCLUSION(atoi(valSOURCEOCCLUSION->getText()));
				>>

			|   T_SOURCEOCCLUSIONLF   T_EQUALITY valSOURCEOCCLUSIONLF: TCL_NUM
				<<
					RecSOURCEOCCLUSIONLF(atof(valSOURCEOCCLUSIONLF->getText()));
				>>

			|   T_SOURCEOCCLUSIONROOM T_EQUALITY valSOURCEOCCLUSIONROOM: TCL_NUM
				<<
					RecSOURCEOCCLUSIONROOM(atof(valSOURCEOCCLUSIONROOM->getText()));
				>>

			|   T_POSX T_EQUALITY valPOSX: TCL_NUM
				<<
					RecPOSX(atof(valPOSX->getText()));
				>>

			|   T_POSY T_EQUALITY valPOSY: TCL_NUM
				<<
					RecPOSY(atof(valPOSY->getText()));
				>>

			|   T_POSZ T_EQUALITY valPOSZ: TCL_NUM
				<<
					RecPOSZ(atof(valPOSZ->getText()));
				>>

			|   T_SOURCEAFFECTDIRECTHF
				<<
					RecSOURCEAFFECTDIRECTHF();
				>>

			|   T_SOURCEAFFECTROOM
				<<
					RecSOURCEAFFECTROOM();
				>>

			|   T_SOURCEAFFECTROOMHF
				<<
					RecSOURCEAFFECTROOMHF();
				>>
			|   T_PRIORITY T_EQUALITY valPRIORITY: TCL_NUM
				<<
					RecPRIORITY(atof(valPRIORITY->getText()));
				>>
			|   T_MEMORYSTATIC
				<<
					RecMEMORYSTATIC();
				>>

			;

unexpected_for_snd:
				( T_ROLLOFF | T_DOPPLERFACTOR | T_LCURLYBRACE 
				  T_ROOM | T_ROOMHF | T_ROOMROLLOFF | T_DECAYTIME |
			      T_DECAYHFRATIO | T_REFLECTIONS | T_REFLECTIONSDELAY |
				  T_REVERB | T_REVERBDELAY | T_ENVIROMENT | T_ENVIROMENTSIZE |
				  T_ENVIROMENTDIFFUSION | T_AIRABSOPTION | T_FLAGS |
				  T_SCALETIME | T_SCALEREFLECTIONS | T_SCALEREFLECTIONSDELAY |
			      T_SCALEREVERB | T_SCALEREVERBDELAY | T_CLIPDECAYHF | ENVIRONMENTS |
				  T_LEFTHCOORDINATESYSTEM | T_RIGHTHCOORDINATESYSTEM
				)
				<<
					ThrowUnknown("unexpected token for sound description");
				>>
			;

unexpected: sym:T_ABC 
		    <<
				ThrowUnknown(sym->getText());
		    >>
		|	id: T_ID
		    <<
				ThrowUnknown(id->getText());
			>>

		|   num: TCL_NUM
			<<
				ThrowUnknown(num->getText());
			>>
		;
