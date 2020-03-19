/*
 * ShdParser: P a r s e r  H e a d e r 
 *
 * Generated from: shd_grammar.g
 *
 * Terence Parr, Russell Quong, Will Cohen, and Hank Dietz: 1989-1999
 * Parr Research Corporation
 * with Purdue University Electrical Engineering
 * with AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR20
 */

#ifndef ShdParser_h
#define ShdParser_h

#ifndef ANTLR_VERSION
#define ANTLR_VERSION 13320
#endif

#include "AParser.h"


#ifndef _PRECOMP_H_
//	Naughty: этот файл нигде не используется
//#include "../3d/quat.h"
#include <string>
#endif

    class OutputWrapper;
class ShdParser : public ANTLRParser {
public:
	static  const ANTLRChar *tokenName(int tk);
protected:
	static  const ANTLRChar *_token_tbl[];
private:

protected:

    //
//these func's will be overriden in ShdRecorder
//

    virtual void NewPath(const char* str) = 0;
virtual void AddPath(const char* str) = 0;

    virtual void BegShader(const char* name) = 0;
virtual void EndShader() = 0;

    virtual void BegStage() = 0;
virtual void EndStage() = 0;

    virtual void SetArmour(float val) = 0;
virtual void SetSteps(int type) = 0;
virtual void SetImpact(int type) = 0;
virtual void SetMarks(int type) = 0;

    virtual void SetFillMode(int type) = 0;
virtual void SetZBias(int value) = 0;
virtual void SetAlphaFunc(int type) = 0;

    virtual void SetCull(int type) = 0;
virtual void SetSurfParam(int type) = 0;

    virtual void SetDeformVerts(int type) = 0;
virtual void SetPolygonOffset() = 0;

    virtual void SetLightmap() = 0;
virtual void SetMap(const char* path, bool clampmap = false) = 0;
virtual void SetAnimMap(float freq) = 0;
virtual void AddTexture(const char* path) = 0;
virtual void SetBlendFunc(int src, int dst) = 0;
virtual void SetTcGenFunc(int type) = 0;
virtual void SetTcModFunc(int type, float arg[], int wave = 0) = 0;
virtual void SetDepthFunc(int type) = 0;
virtual void SetDepthWrite() = 0;
virtual void SetNoMipMap() = 0;
virtual void SetNoPicMip() = 0;

    virtual void SetRAGen(int gen, int type, int wave_form = 0, float arg[] = 0) = 0;

    virtual const char* GetPath() = 0;

    OutputWrapper*  p_output;
protected:
	static SetWordType TCL_STEPS_set[20];
	static SetWordType TCL_IMPACT_set[20];
	static SetWordType TCL_MARKS_set[20];
	static SetWordType TCL_CULL_set[20];
	static SetWordType TCL_SURFPARAM_set[20];
	static SetWordType TCL_NUM_set[20];
	static SetWordType TCL_DEFORMVERTS_set[20];
	static SetWordType TCL_BLEND_SHORTCUTS_set[20];
	static SetWordType TCL_BLEND_SRC_set[20];
	static SetWordType TCL_BLEND_DST_set[20];
	static SetWordType TCL_TCGEN_FUNC_set[20];
	static SetWordType TCL_DEPTHFUNC_set[20];
	static SetWordType TCL_WAVE_FORM_set[20];
	static SetWordType TCL_FILLMODE_set[20];
	static SetWordType TCL_ALPHAFUNC_set[20];
	static SetWordType TCL_RA_GEN_set[20];
	static SetWordType setwd1[134];
	static SetWordType TCL_RA_FUNC_set[20];
	static SetWordType setwd2[134];
private:
	void zzdflthandlers( int _signal, int *_retsignal );

public:
	ShdParser(ANTLRTokenBuffer *input);
	void start(int *_retsignal);
	void shader_desc(int *_retsignal);
	void shader_body(int *_retsignal);
	void stage_body(int *_retsignal);
	void path(int *_retsignal);
	static SetWordType TCL_RESYNC_set[20];
};

#endif /* ShdParser_h */
