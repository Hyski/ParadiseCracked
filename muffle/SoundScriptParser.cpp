/* $ANTLR 2.7.1: "grammar.g" -> "SoundScriptParser.cpp"$ */
// line 8 "grammar.g"

#include "precomp.h"

#include "SoundScriptParser.hpp"
#include "antlr/NoViableAltException.hpp"
#include "antlr/SemanticException.hpp"
// line 1 "grammar.g"

SoundScriptParser::SoundScriptParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,k)
{
	setTokenNames(_tokenNames);
}

SoundScriptParser::SoundScriptParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,2)
{
	setTokenNames(_tokenNames);
}

SoundScriptParser::SoundScriptParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,k)
{
	setTokenNames(_tokenNames);
}

SoundScriptParser::SoundScriptParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,2)
{
	setTokenNames(_tokenNames);
}

SoundScriptParser::SoundScriptParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(state,2)
{
	setTokenNames(_tokenNames);
}

void SoundScriptParser::program() {
	
	try {      // for error handling
		{
		for (;;) {
			if ((_tokenSet_0.member(LA(1)))) {
				script();
			}
			else {
				goto _loop3;
			}
			
		}
		_loop3:;
		}
		match(ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
}

void SoundScriptParser::script() {
	
	try {      // for error handling
		switch ( LA(1)) {
		case KEYWORD_THEME:
		case KEYWORD_EFFECT:
		case KEYWORD_SPEECH:
		case KEYWORD_MENU:
		case KEYWORD_AMBIENT:
		case KEYWORD_DEBUG:
		{
			sound_script();
			break;
		}
		case KEYWORD_PRECACHE:
		case KEYWORD_DISCARD:
		{
			cache_script();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_2);
	}
}

void SoundScriptParser::sound_script() {
	
	try {      // for error handling
		// line 36 "grammar.g"
		
								ISound::Channel channel;
								Script *script = 0;
								std::string name;
							
		channel=sound_type();
		name=identifier();
		// line 42 "grammar.g"
		
								script = createScript(name,channel);
							
		match(LCURLY);
		{
		for (;;) {
			if (((LA(1) >= KEYWORD_FILE && LA(1) <= KEYWORD_CONE))) {
				sound_script_info(script);
			}
			else {
				goto _loop7;
			}
			
		}
		_loop7:;
		}
		match(RCURLY);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_2);
	}
}

void SoundScriptParser::cache_script() {
	
	try {      // for error handling
		cache_type();
		match(IDENTIFIER);
		match(LCURLY);
		match(RCURLY);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_2);
	}
}

ISound::Channel  SoundScriptParser::sound_type() {
	// line 125 "grammar.g"
	ISound::Channel c;
	
	try {      // for error handling
		switch ( LA(1)) {
		case KEYWORD_THEME:
		{
			match(KEYWORD_THEME);
			// line 126 "grammar.g"
			c = ISound::cThemes;
			break;
		}
		case KEYWORD_EFFECT:
		{
			match(KEYWORD_EFFECT);
			// line 127 "grammar.g"
			c = ISound::cEffects;
			break;
		}
		case KEYWORD_SPEECH:
		{
			match(KEYWORD_SPEECH);
			// line 128 "grammar.g"
			c = ISound::cSpeech;
			break;
		}
		case KEYWORD_MENU:
		{
			match(KEYWORD_MENU);
			// line 129 "grammar.g"
			c = ISound::cMenu;
			break;
		}
		case KEYWORD_AMBIENT:
		{
			match(KEYWORD_AMBIENT);
			// line 130 "grammar.g"
			c = ISound::cAmbient;
			break;
		}
		case KEYWORD_DEBUG:
		{
			match(KEYWORD_DEBUG);
			// line 131 "grammar.g"
			c = ISound::cDebug;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_3);
	}
	return c;
}

std::string  SoundScriptParser::identifier() {
	// line 173 "grammar.g"
	std::string rid;
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	try {      // for error handling
		id = LT(1);
		match(IDENTIFIER);
		// line 174 "grammar.g"
		rid = id->getText();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_4);
	}
	return rid;
}

void SoundScriptParser::sound_script_info(
	Script *script
) {
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case KEYWORD_FILE:
		{
			sound_script_file(script);
			break;
		}
		case KEYWORD_REPEAT:
		{
			sound_script_repeat(script);
			break;
		}
		case KEYWORD_USE3D:
		{
			sound_script_use3d(script);
			break;
		}
		case KEYWORD_NO3D:
		{
			sound_script_no3d(script);
			break;
		}
		case KEYWORD_DISTANCE:
		{
			sound_script_distance(script);
			break;
		}
		case KEYWORD_CONE:
		{
			sound_script_cone(script);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(SEMICOLON);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_5);
	}
}

void SoundScriptParser::sound_script_file(
	Script *script
) {
	
	try {      // for error handling
		// line 62 "grammar.g"
		
								std::string fname;
							
		match(KEYWORD_FILE);
		fname=file_name();
		// line 66 "grammar.g"
		
								script->setFileName(fname);
							
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_6);
	}
}

void SoundScriptParser::sound_script_repeat(
	Script *script
) {
	
	try {      // for error handling
		// line 71 "grammar.g"
		
								bool repeat = true;
								unsigned times = 0xFFFFFFFF;
							
		match(KEYWORD_REPEAT);
		{
		switch ( LA(1)) {
		case INTEGER:
		{
			times=integer();
			break;
		}
		case SEMICOLON:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		// line 76 "grammar.g"
		
								if(times==0)
								{
									script->setRepeat(false);
								}
								else
								{
									script->setRepeat(true);
								}
							
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_6);
	}
}

void SoundScriptParser::sound_script_use3d(
	Script *script
) {
	
	try {      // for error handling
		match(KEYWORD_USE3D);
		// line 90 "grammar.g"
		
								script->set3d(true);
							
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_6);
	}
}

void SoundScriptParser::sound_script_no3d(
	Script *script
) {
	
	try {      // for error handling
		match(KEYWORD_NO3D);
		// line 97 "grammar.g"
		
								script->set3d(false);
							
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_6);
	}
}

void SoundScriptParser::sound_script_distance(
	Script *script
) {
	
	try {      // for error handling
		// line 103 "grammar.g"
		
								float minDist;
								float maxDist;
							
		match(KEYWORD_DISTANCE);
		minDist=float_();
		maxDist=float_();
		// line 108 "grammar.g"
		
								script->setDistances(minDist,maxDist);
							
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_6);
	}
}

void SoundScriptParser::sound_script_cone(
	Script *script
) {
	
	try {      // for error handling
		// line 114 "grammar.g"
		
								float minAngle;
								float maxAngle;
								float outVol;
							
		match(KEYWORD_CONE);
		minAngle=float_();
		maxAngle=float_();
		outVol=float_();
		// line 120 "grammar.g"
		
								script->setCone(minAngle,maxAngle,outVol);
							
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_6);
	}
}

std::string  SoundScriptParser::file_name() {
	// line 144 "grammar.g"
	std::string fname;
	ANTLR_USE_NAMESPACE(antlr)RefToken  name = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	try {      // for error handling
		name = LT(1);
		match(FILE_NAME);
		// line 146 "grammar.g"
		
								fname = name->getText().substr(1,name->getText().length()-2);
							
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_6);
	}
	return fname;
}

unsigned  SoundScriptParser::integer() {
	// line 165 "grammar.g"
	unsigned n;
	ANTLR_USE_NAMESPACE(antlr)RefToken  t1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	try {      // for error handling
		t1 = LT(1);
		match(INTEGER);
		// line 167 "grammar.g"
		
								std::istringstream sstr(t1->getText());
								sstr >> n;
							
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_6);
	}
	return n;
}

float  SoundScriptParser::float_() {
	// line 151 "grammar.g"
	float n;
	ANTLR_USE_NAMESPACE(antlr)RefToken  t1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  t2 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case INTEGER:
		{
			t1 = LT(1);
			match(INTEGER);
			// line 153 "grammar.g"
			
										std::istringstream sstr(t1->getText());
										sstr >> n;
									
			break;
		}
		case FLOAT:
		{
			t2 = LT(1);
			match(FLOAT);
			// line 158 "grammar.g"
			
										std::istringstream sstr(t2->getText());
										sstr >> n;
									
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_7);
	}
	return n;
}

Cache::CacheType  SoundScriptParser::cache_type() {
	// line 139 "grammar.g"
	Cache::CacheType t;
	
	try {      // for error handling
		switch ( LA(1)) {
		case KEYWORD_PRECACHE:
		{
			match(KEYWORD_PRECACHE);
			// line 140 "grammar.g"
			t = Cache::ctPrecache;
			break;
		}
		case KEYWORD_DISCARD:
		{
			match(KEYWORD_DISCARD);
			// line 141 "grammar.g"
			t = Cache::ctDiscard;
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_3);
	}
	return t;
}

const char* SoundScriptParser::_tokenNames[] = {
	"<0>",
	"EOF",
	"<2>",
	"NULL_TREE_LOOKAHEAD",
	"LCURLY",
	"RCURLY",
	"SEMICOLON",
	"\"file\"",
	"\"repeat\"",
	"\"use3D\"",
	"\"no3D\"",
	"\"distance\"",
	"\"cone\"",
	"\"theme\"",
	"\"effect\"",
	"\"speech\"",
	"\"menu\"",
	"\"ambient\"",
	"\"debug\"",
	"IDENTIFIER",
	"\"precache\"",
	"\"discard\"",
	"FILE_NAME",
	"INTEGER",
	"FLOAT",
	"\"except\"",
	"\"volume\"",
	"\"permanent\"",
	"WHITESPACE",
	"ML_COMMENT_DATA",
	"COMMENT",
	"DIGIT",
	"LETTER",
	"UNDERSCORE",
	"DOT",
	"INT_OR_FLOAT",
	0
};

const unsigned long SoundScriptParser::_tokenSet_0_data_[] = { 3661824UL, 0UL, 0UL, 0UL };
// "theme" "effect" "speech" "menu" "ambient" "debug" "precache" "discard" 
const ANTLR_USE_NAMESPACE(antlr)BitSet SoundScriptParser::_tokenSet_0(_tokenSet_0_data_,4);
const unsigned long SoundScriptParser::_tokenSet_1_data_[] = { 2UL, 0UL, 0UL, 0UL };
// EOF 
const ANTLR_USE_NAMESPACE(antlr)BitSet SoundScriptParser::_tokenSet_1(_tokenSet_1_data_,4);
const unsigned long SoundScriptParser::_tokenSet_2_data_[] = { 3661826UL, 0UL, 0UL, 0UL };
// EOF "theme" "effect" "speech" "menu" "ambient" "debug" "precache" "discard" 
const ANTLR_USE_NAMESPACE(antlr)BitSet SoundScriptParser::_tokenSet_2(_tokenSet_2_data_,4);
const unsigned long SoundScriptParser::_tokenSet_3_data_[] = { 524288UL, 0UL, 0UL, 0UL };
// IDENTIFIER 
const ANTLR_USE_NAMESPACE(antlr)BitSet SoundScriptParser::_tokenSet_3(_tokenSet_3_data_,4);
const unsigned long SoundScriptParser::_tokenSet_4_data_[] = { 16UL, 0UL, 0UL, 0UL };
// LCURLY 
const ANTLR_USE_NAMESPACE(antlr)BitSet SoundScriptParser::_tokenSet_4(_tokenSet_4_data_,4);
const unsigned long SoundScriptParser::_tokenSet_5_data_[] = { 8096UL, 0UL, 0UL, 0UL };
// RCURLY "file" "repeat" "use3D" "no3D" "distance" "cone" 
const ANTLR_USE_NAMESPACE(antlr)BitSet SoundScriptParser::_tokenSet_5(_tokenSet_5_data_,4);
const unsigned long SoundScriptParser::_tokenSet_6_data_[] = { 64UL, 0UL, 0UL, 0UL };
// SEMICOLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet SoundScriptParser::_tokenSet_6(_tokenSet_6_data_,4);
const unsigned long SoundScriptParser::_tokenSet_7_data_[] = { 25165888UL, 0UL, 0UL, 0UL };
// SEMICOLON INTEGER FLOAT 
const ANTLR_USE_NAMESPACE(antlr)BitSet SoundScriptParser::_tokenSet_7(_tokenSet_7_data_,4);


