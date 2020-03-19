#ifndef INC_SoundScriptLexer_hpp_
#define INC_SoundScriptLexer_hpp_

#include "antlr/config.hpp"
/* $ANTLR 2.7.1: "grammar.g" -> "SoundScriptLexer.hpp"$ */
#include "antlr/CommonToken.hpp"
#include "antlr/InputBuffer.hpp"
#include "antlr/BitSet.hpp"
#include "SoundScriptParserTokenTypes.hpp"
#include "antlr/CharScanner.hpp"
// line 2 "grammar.g"

#include "cache.h"
#include "script.h"

class SoundScriptLexer : public ANTLR_USE_NAMESPACE(antlr)CharScanner, public SoundScriptParserTokenTypes
 {
// line 1 "grammar.g"
private:
	void initLiterals();
public:
	bool getCaseSensitiveLiterals() const;
public:
	SoundScriptLexer(ANTLR_USE_NAMESPACE(std)istream& in);
	SoundScriptLexer(ANTLR_USE_NAMESPACE(antlr)InputBuffer& ib);
	SoundScriptLexer(const ANTLR_USE_NAMESPACE(antlr)LexerSharedInputState& state);
	ANTLR_USE_NAMESPACE(antlr)RefToken nextToken();
	public: void mWHITESPACE(bool _createToken);
	protected: void mCOMMENT(bool _createToken);
	protected: void mML_COMMENT_DATA(bool _createToken);
	public: void mIDENTIFIER(bool _createToken);
	protected: void mLETTER(bool _createToken);
	protected: void mUNDERSCORE(bool _createToken);
	protected: void mDIGIT(bool _createToken);
	public: void mLCURLY(bool _createToken);
	public: void mRCURLY(bool _createToken);
	public: void mFILE_NAME(bool _createToken);
	public: void mSEMICOLON(bool _createToken);
	protected: void mDOT(bool _createToken);
	public: void mINT_OR_FLOAT(bool _createToken);
private:
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
};

#endif /*INC_SoundScriptLexer_hpp_*/
