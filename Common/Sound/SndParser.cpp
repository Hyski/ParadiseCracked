/*
 * SndParser: P a r s e r  S u p p o r t
 *
 * Generated from: sndscript.g
 *
 * Terence Parr, Russell Quong, Will Cohen, and Hank Dietz: 1989-1999
 * Parr Research Corporation
 * with Purdue University Electrical Engineering
 * with AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR20
 */

#define ANTLR_VERSION	13320
#include "pcctscfg.h"
#include "pccts_stdio.h"
#define ANTLR_SUPPORT_CODE
#include "tokens.h"
#include "SndParser.h"

const ANTLRChar *SndParser::tokenName(int tok)   { return _token_tbl[tok]; }

const ANTLRChar *SndParser::_token_tbl[]={
	/* 00 */	"Invalid",
	/* 01 */	"T_EOF",
	/* 02 */	"T_LCURLYBRACE",
	/* 03 */	"T_RCURLYBRACE",
	/* 04 */	"T_LPAREN",
	/* 05 */	"T_RPAREN",
	/* 06 */	"T_LOOP",
	/* 07 */	"T_USE3D",
	/* 08 */	"T_CTRLPAN",
	/* 09 */	"T_CTRLFREQ",
	/* 10 */	"T_CTRLVOLUME",
	/* 11 */	"T_MAXDISTANCE",
	/* 12 */	"T_MINDISTANCE",
	/* 13 */	"T_MODE",
	/* 14 */	"T_INSIDECONEANGLE",
	/* 15 */	"T_OUTSIDECONEANGLE",
	/* 16 */	"T_ROLLOFF",
	/* 17 */	"T_DOPPLERFACTOR",
	/* 18 */	"T_DIRECTSOUND_TAG",
	/* 19 */	"T_AUREAL_TAG",
	/* 20 */	"T_EAX_TAG",
	/* 21 */	"T_DECIMALINT",
	/* 22 */	"T_FLOATONE",
	/* 23 */	"T_FLOATTWO",
	/* 24 */	"T_ABC",
	/* 25 */	"T_ID",
	/* 26 */	"T_NEWLINE",
	/* 27 */	"/\\*",
	/* 28 */	"//~[\\n@]*",
	/* 29 */	"[\\r\\t\\ ]+",
	/* 30 */	"T_ERR_TOKEN",
	/* 31 */	"[\\n\\r]",
	/* 32 */	"\\*/",
	/* 33 */	"\\*",
	/* 34 */	"~[\\*\\n\\r]+",
	/* 35 */	"TCL_NUM",
	/* 36 */	"T_DECIMALIN",
	/* 37 */	"TCL_SND_LIB_NAME"
};

SndParser::SndParser(ANTLRTokenBuffer *input) : ANTLRParser(input,1,0,0,8)
{
	token_tbl = _token_tbl;
	traceOptionValueDefault=0;		// MR10 turn trace OFF
}

SetWordType SndParser::TCL_NUM_set[8] = {0x0,0x0,0xc0,0x0, 0x10,0x0,0x0,0x0};
SetWordType SndParser::err2[8] = {0xc0,0xdf,0x3,0x0, 0x0,0x0,0x0,0x0};
SetWordType SndParser::setwd1[38] = {0x0,0x5,0x0,0x8,0x0,0x0,0xa,
	0xa,0xa,0xa,0xa,0xa,0xa,0x0,0xa,
	0xa,0xa,0xa,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x4,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0};
SetWordType SndParser::TCL_SND_LIB_NAME_set[8] = {0x0,0x0,0x1c,0x0};
