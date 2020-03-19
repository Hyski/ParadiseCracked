#ifndef INC_SoundScriptParserTokenTypes_hpp_
#define INC_SoundScriptParserTokenTypes_hpp_

/* $ANTLR 2.7.1: "grammar.g" -> "SoundScriptParserTokenTypes.hpp"$ */
struct SoundScriptParserTokenTypes {
	enum {
		EOF_ = 1,
		LCURLY = 4,
		RCURLY = 5,
		SEMICOLON = 6,
		KEYWORD_FILE = 7,
		KEYWORD_REPEAT = 8,
		KEYWORD_USE3D = 9,
		KEYWORD_NO3D = 10,
		KEYWORD_DISTANCE = 11,
		KEYWORD_CONE = 12,
		KEYWORD_THEME = 13,
		KEYWORD_EFFECT = 14,
		KEYWORD_SPEECH = 15,
		KEYWORD_MENU = 16,
		KEYWORD_AMBIENT = 17,
		KEYWORD_DEBUG = 18,
		IDENTIFIER = 19,
		KEYWORD_PRECACHE = 20,
		KEYWORD_DISCARD = 21,
		FILE_NAME = 22,
		INTEGER = 23,
		FLOAT = 24,
		KEYWORD_EXCEPT = 25,
		KEYWORD_VOLUME = 26,
		KEYWORD_PERMANENT = 27,
		WHITESPACE = 28,
		ML_COMMENT_DATA = 29,
		COMMENT = 30,
		DIGIT = 31,
		LETTER = 32,
		UNDERSCORE = 33,
		DOT = 34,
		INT_OR_FLOAT = 35,
		NULL_TREE_LOOKAHEAD = 3
	};
};
#endif /*INC_SoundScriptParserTokenTypes_hpp_*/
