#ifndef INC_SoundScriptParser_hpp_
#define INC_SoundScriptParser_hpp_

#include "antlr/config.hpp"
/* $ANTLR 2.7.1: "grammar.g" -> "SoundScriptParser.hpp"$ */
#include "antlr/TokenStream.hpp"
#include "antlr/TokenBuffer.hpp"
#include "SoundScriptParserTokenTypes.hpp"
#include "antlr/LLkParser.hpp"

// line 2 "grammar.g"

#include "cache.h"
#include "script.h"

class SoundScriptParser : public ANTLR_USE_NAMESPACE(antlr)LLkParser, public SoundScriptParserTokenTypes
 {
// line 26 "grammar.g"

private:
	virtual Script *createScript(const std::string &name, ISound::Channel) = 0;
protected:
	SoundScriptParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k);
public:
	SoundScriptParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf);
protected:
	SoundScriptParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k);
public:
	SoundScriptParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer);
	SoundScriptParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state);
	public: void program();
	public: void script();
	public: void sound_script();
	public: void cache_script();
	public: ISound::Channel  sound_type();
	public: std::string  identifier();
	public: void sound_script_info(
		Script *script
	);
	public: void sound_script_file(
		Script *script
	);
	public: void sound_script_repeat(
		Script *script
	);
	public: void sound_script_use3d(
		Script *script
	);
	public: void sound_script_no3d(
		Script *script
	);
	public: void sound_script_distance(
		Script *script
	);
	public: void sound_script_cone(
		Script *script
	);
	public: std::string  file_name();
	public: unsigned  integer();
	public: float  float_();
	public: Cache::CacheType  cache_type();
private:
	static const char* _tokenNames[];
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
	static const unsigned long _tokenSet_2_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_2;
	static const unsigned long _tokenSet_3_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_3;
	static const unsigned long _tokenSet_4_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_4;
	static const unsigned long _tokenSet_5_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_5;
	static const unsigned long _tokenSet_6_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_6;
	static const unsigned long _tokenSet_7_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_7;
};

#endif /*INC_SoundScriptParser_hpp_*/
