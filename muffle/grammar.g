header
{
#include "cache.h"
#include "script.h"
}

header "pre_include_cpp"
{
#include "precomp.h"
}

options
{
	language="Cpp";
	genHashLines=false;
}

//=====================================================================================//
//                       class SoundScriptParser extends Parser;                       //
//=====================================================================================//
class SoundScriptParser extends Parser;
options
{
	k=2;
}
{
private:
	virtual Script *createScript(const std::string &name, ISound::Channel) = 0;
}
program			:	(script)* EOF
				;

script			:	sound_script | cache_script
				;

sound_script	:	{
						ISound::Channel channel;
						Script *script = 0;
						std::string name;
					}
					channel=sound_type name=identifier
					{
						script = createScript(name,channel);
					}
					LCURLY 
						(sound_script_info[script])*
					RCURLY
				;

sound_script_info[Script *script]
				:	(	sound_script_file[script]
					|	sound_script_repeat[script]
					|	sound_script_use3d[script]
					|	sound_script_no3d[script]
					|	sound_script_distance[script]
					|	sound_script_cone[script]
					)
					SEMICOLON
				;

sound_script_file[Script *script]
				:	{
						std::string fname;
					}
					KEYWORD_FILE fname=file_name
					{
						script->setFileName(fname);
					}
				;
sound_script_repeat[Script *script]
				:	{
						bool repeat = true;
						unsigned times = 0xFFFFFFFF;
					}
					KEYWORD_REPEAT (times=integer)?
					{
						if(times==0)
						{
							script->setRepeat(false);
						}
						else
						{
							script->setRepeat(true);
						}
					}
				;

sound_script_use3d[Script *script]
				:	KEYWORD_USE3D
					{
						script->set3d(true);
					}
				;

sound_script_no3d[Script *script]
				:	KEYWORD_NO3D
					{
						script->set3d(false);
					}
				;

sound_script_distance[Script *script]
				:	{
						float minDist;
						float maxDist;
					}
					KEYWORD_DISTANCE minDist=float_ maxDist=float_
					{
						script->setDistances(minDist,maxDist);
					}
				;

sound_script_cone[Script *script]
				:	{
						float minAngle;
						float maxAngle;
						float outVol;
					}
					KEYWORD_CONE minAngle=float_ maxAngle=float_ outVol=float_
					{
						script->setCone(minAngle,maxAngle,outVol);
					}
				;

sound_type returns [ISound::Channel c]
				:	KEYWORD_THEME		{ c = ISound::cThemes; }
				|	KEYWORD_EFFECT		{ c = ISound::cEffects; }
				|	KEYWORD_SPEECH		{ c = ISound::cSpeech; }
				|	KEYWORD_MENU		{ c = ISound::cMenu; }
				|	KEYWORD_AMBIENT		{ c = ISound::cAmbient; }
				|	KEYWORD_DEBUG		{ c = ISound::cDebug; }
				;

cache_script	:	cache_type IDENTIFIER
					LCURLY
					RCURLY
				;

cache_type returns [Cache::CacheType t]
				:	KEYWORD_PRECACHE	{ t = Cache::ctPrecache; }
				|	KEYWORD_DISCARD		{ t = Cache::ctDiscard; }
				;

file_name returns [std::string fname]
				:	name:FILE_NAME
					{
						fname = name->getText().substr(1,name->getText().length()-2);
					}
				;

float_ returns [float n]
				:	(	t1:INTEGER
						{
							std::istringstream sstr(t1->getText());
							sstr >> n;
						}
					|	t2:FLOAT
						{
							std::istringstream sstr(t2->getText());
							sstr >> n;
						}
					)
				;

integer returns [unsigned n]
				:	t1:INTEGER
					{
						std::istringstream sstr(t1->getText());
						sstr >> n;
					}
				;

identifier returns [std::string rid]
				:	id:IDENTIFIER { rid = id->getText(); }
				;

//=====================================================================================//
//                        class SoundScriptLexer extends Lexer;                        //
//=====================================================================================//
class SoundScriptLexer extends Lexer;
options
{
	k=5;
	charVocabulary = '\3'..'\377';
}
tokens
{
	KEYWORD_THEME="theme";
	KEYWORD_EFFECT="effect";
	KEYWORD_SPEECH="speech";
	KEYWORD_MENU="menu";
	KEYWORD_AMBIENT="ambient";
	KEYWORD_DEBUG="debug";

	KEYWORD_PRECACHE="precache";
	KEYWORD_DISCARD="discard";
	KEYWORD_EXCEPT="except";

	KEYWORD_FILE="file";
	KEYWORD_REPEAT="repeat";
	KEYWORD_NO3D="no3D";
	KEYWORD_USE3D="use3D";
	KEYWORD_DISTANCE="distance";
	KEYWORD_CONE="cone";
	KEYWORD_VOLUME="volume";
	KEYWORD_PERMANENT="permanent";
}

WHITESPACE			:	
					(
						options { generateAmbigWarnings=false; }
						:
						(	' '
						|	'\t'
						|	'\r' '\n'		{newline();}
						|	'\r'			{newline();}
						|	'\n'			{newline();}
						|	COMMENT
						) { $setType(antlr::Token::SKIP); }
					)
					;

protected
ML_COMMENT_DATA		:
					(
						options { generateAmbigWarnings=false; }
						:
							{LA(2)!='/'}? '*'
						|	'\r' '\n'		{newline();}
						|	'\r'			{newline();}
						|	'\n'			{newline();}
						|	~('*'|'\n'|'\r')
					)*
					;

protected
COMMENT				:	'/'
						(
							'/'
							{
								while(LA(1) != '\r' && LA(1) != '\n' && LA(1) != EOF)
								{
									consume();
								}
							}
							('\r'|'\n')? { $setType(antlr::Token::SKIP); }
						|	'*' ML_COMMENT_DATA "*/" { $setType(antlr::Token::SKIP); }
						)
					;

IDENTIFIER			:
						(LETTER|UNDERSCORE) (LETTER|UNDERSCORE|DIGIT)*
					;

LCURLY				:	"{" ;
RCURLY				:	"}" ;

FILE_NAME			:	'\"' (~'\"')+ '\"';

SEMICOLON			:	";" ;

protected DIGIT			:	'0'..'9';
protected LETTER		:	(('a'..'z') | ('A'..'Z'));
protected UNDERSCORE	:	'_';
protected DOT			:	'.';

INT_OR_FLOAT			:
							(DIGIT)+ { $setType(INTEGER); }
							(
								DOT (DIGIT)+
								(
									('e'|'E') ('+'|'-')? (DIGIT)+
								)?
								{ $setType(FLOAT); }
							)?
						;
