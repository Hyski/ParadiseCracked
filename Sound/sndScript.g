#header
<<
class cc_SMParserFacet;
#include "Script.h"
#include <list>

class cc_ScriptMgr;
class cc_SegmentMgr;
>>

<<
const char *cc_SndScriptParser::m_LogFile = "gvz_sound_parser.log";
>>


#token  "//~[\n@]*"				<<skip();>>		// One-line comment (from Punch)

#token	END_OF_FILE	"@"

#token	"[\ \t]+"				<<skip();>>
#token	NEWLINE		"\n"		<<skip(); newline();>>

#token TKN_INTEGER	"{[\+ \-]}[0-9]+"
#token TKN_FLOAT	"{[\+ \-]}[0-9]+{.[0-9]+ {[eE]{[\-\+]}[0-9]+}}"
#token TKN_STRING	"\"~[\"]*\""

#token	KWD_THEME		"theme"
#token	KWD_EFFECT		"effect"
#token	KWD_SPEECH		"speech"
#token	KWD_MENU		"menu"
#token	KWD_AMBIENT		"ambient"
#token	KWD_DEBUG		"debug"

#tokclass KWD_SNDUNIT
					{
						KWD_THEME
						KWD_EFFECT
						KWD_SPEECH
						KWD_MENU
						KWD_AMBIENT
						KWD_DEBUG
					}

#token	KWD_PRECACHE	"precache"
#token	KWD_DISCARD		"discard"

#tokclass KWD_CACHE
					{
						KWD_PRECACHE
						KWD_DISCARD
					}

#token	KWD_EXCEPT		"except"
					
#token	PROP_FILE		"file"
#token	PROP_REPEAT		"repeat"
#token	PROP_NO3D		"no3D"
#token	PROP_USE3D		"use3D"
#token	PROP_PAN		"pan"
#token	PROP_DISTANCE	"distance"
#token	PROP_CONE		"cone"
#token	PROP_VOLUME		"volume"
#token	PROP_PERMANENT	"permanent"

#token	TKN_IDENTIFIER	"[a-zA-Z_][a-zA-Z_0-9]*"

#token LCURLY		"\{"
#token RCURLY		"\}"

#token SEMICOLON	";"

class cc_SndScriptParser
{
<<
	static const char *m_LogFile;


	cc_SndParams m_ScriptParams;

	std::string m_UnitName;

	// Для скриптов
	void setDefaults(ANTLRTokenType);
	void setName(const char *);
	void setFile(const char *);
	void setRepeat(unsigned);
	void set3D(bool);
	void setPan(float);
	void setDistances(float,float);
	void setCone(float,float,float);
	void setVolume(float);
	void setPermanent();
	void pushScript();

	// Для кешей
	void addFileMask(const char *);
	void addExceptMask(const char *);
	void setCacheType(ANTLRTokenType);
	void pushCache();
	void clean();

	std::list<std::string> m_FileMasks;
	std::list<std::string> m_ExceptMasks;
	ANTLRTokenType m_CacheType;

	cc_ScriptMgr *m_scriptMgr;
	cc_SegmentMgr *m_segmentMgr;
public:
	void setManagers(cc_ScriptMgr *, cc_SegmentMgr *);

public:
>>
/*
	start :   (
                   tok:. 
                   <<
						char buffer[1024];
                        sprintf(buffer, "%s = %s\n", _token_tbl[tok->getType()], tok->getText());
						CORE_LOG(m_LogFile, buffer);
                   >>
                )+ 
                END_OF_FILE
            ;
*/
	start			:	(sndUnit|sndCache)*
					;
	
	sndUnit			:	unitType:KWD_SNDUNIT
						<<
							setDefaults(unitType->getType());
						>> 
						unitName:TKN_IDENTIFIER 
						<<
							setName(unitName->getText());
						>> 
						{LCURLY (sndProperty)+ RCURLY}
						<<
							pushScript();
						>>
					;
		
	sndProperty		:	(
							sndPropFile	
						|	sndPropRepeat
						|	sndPropUse3D
						|	sndPropNo3D 
						|	sndPropPan 
						|	sndPropDistance 
						|	sndPropCone 
						|	sndPropVolume
						|	sndPropPermanent
						) 
						SEMICOLON
					;
		
	sndPropFile		:	PROP_FILE 
						fileName:TKN_STRING
						<<
							setFile(fileName->getText()); 
						>>
					;
	
	sndPropRepeat	:	PROP_REPEAT
						<<
							setRepeat(0xFFFFFFFF);
						>>
						{
							times:TKN_INTEGER
							<<
								setRepeat(atol(times->getText()));
							>>
						}
					;
	
	sndPropUse3D	:	PROP_USE3D
						<< 
							set3D(true);
						>>
					;

	sndPropNo3D		:	PROP_NO3D
						<< 
							set3D(false);
						>>
					;
	
	sndPropPan		:	PROP_PAN pan:TKN_FLOAT
						<<
							setPan(static_cast<float>(atof(pan->getText())));
						>>
					;
	
	sndPropDistance :	PROP_DISTANCE minDist:TKN_FLOAT maxDist:TKN_FLOAT
						<<
							setDistances(static_cast<float>(atof(minDist->getText())),
										 static_cast<float>(atof(maxDist->getText())));
						>>
					;
	
	sndPropCone		:	PROP_CONE minAngle:TKN_FLOAT maxAngle:TKN_FLOAT outVol:TKN_FLOAT
						<<
							setCone(static_cast<float>(atof(minAngle->getText())),
									static_cast<float>(atof(maxAngle->getText())),
									static_cast<float>(atof(outVol->getText())));
						>>
					;
	
	sndPropVolume	:	PROP_VOLUME volume:TKN_FLOAT
						<<
							setVolume(static_cast<float>(atof(volume->getText())));
						>>
					;

	sndPropPermanent:	PROP_PERMANENT
						<<
							setPermanent();
						>>
					;

	sndCache		:	type:KWD_CACHE
						<<
							clean();
							setCacheType(type->getType());
						>>
						id:TKN_IDENTIFIER
						<<
							setName(id->getText());
						>>
						LCURLY
						(sndCacheEntry SEMICOLON)*
						RCURLY
						<<
							pushCache();
						>>
					;

	sndCacheEntry	:	(
							mask1:TKN_STRING
							<<
								addFileMask(mask1->getText());
							>>
						)+
					|	KWD_EXCEPT 
						(
							mask2:TKN_STRING
							<<
								addExceptMask(mask2->getText());
							>>
						)+
					;
}