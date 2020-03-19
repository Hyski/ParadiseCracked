//
// Shader Compiler methods
//
//============================================
//   FILE:  shdcomp.h   
//

#include "precomp.h"

#include "shader.h"

#include "tokens.h"
#include "AToken.h"
#include "ShdLexer.h"
#include "ShdRecorder.h"
#include "ShdCompiler.h"
#include "PakStream.h"
#include "OutWrapper.h"

const char* ShaderOutput::error_desc[] =
{    
    /*SC_SUCCESS,*/             "нет ошибок",
    /*SC_TOKBUFFER_ERR,*/       "ошибка класса TokenBuffer",
    /*SC_TOKEN_ERR,*/           "ошибка класса Token",
    /*SC_PARSER_ERR,*/          "был вызов функции syn или panic парсера",
    /*SC_LEXER_ERR,*/           "ошибка лексера",
	/*SC_FILE_OPEN_ERR,*/       "ошибка открытия файла",
    /*SC_NAME_REPLICATION,*/    "повторение названия шейдера",
    /*SC_SECOND_CULL,*/         "второе описание cull",
    /*SC_SECOND_ZBIAS,*/        "второе описание zBias",
    /*SC_SECOND_ALPHAFUNC,*/    "второе определение alphaFunc",
    /*SC_SECOND_FILLMODE,*/     "второе описание FillMode",
    /*SC_SECOND_RGBGEN,*/       "второе описание rgbGen",
    /*SC_SECOND_ALPHAGEN,*/     "второе описание alphaGen",
    /*SC_STAGE_NUM_ERR,*/       "слишком много stages в шейдере",
    /*SC_NO_MAP_ERR,*/          "в stage'е отсутствует описание map | clampmap | animmap",
    /*SC_SECOND_MAP_ERR,*/      "два описания map(animmap)",
    /*SC_ANIM_FRAMES_ERR,*/     "слишком много фреймов в animmap",
    /*SC_SECOND_BLEND_ERR,*/    "второе описание blendfunc",
    /*SC_SECOND_TCGEN_ERR,*/    "второе описание tcGen",
    /*SC_TCMODS_FUNCS_ERR,*/    "слишком много описаний tcMod",
    /*SC_SECOND_NOMIPMAP_ERR,*/ "второе описание nomipmap",
    /*SC_SECOND_NOPICMIP_ERR,*/ "второе описание nopicmip",
    /*SC_SECOND_STEPS_ERR,*/    "второе описание steps",
    /*SC_SECOND_IMPACT_ERR,*/   "второе описание impact",
    /*SC_SECOND_MARKS_ERR,*/    "второе описание marks",
    /*SC_SECOND_DEPTHFUNC_ERR,*/"второе описание depthFunc",
    /*SC_UNKNOWN_ERR,*/         "неизвестная ошибка",
};

ShdCompiler::ShdCompiler(ShaderOutput* p_out):
    p_output(p_out)
{
}

ShdCompiler::~ShdCompiler()
{
}

ShaderOutput* ShdCompiler::SetOutput(ShaderOutput* out)
{
    ShaderOutput* old_ptr = p_output;

    p_output = out;
    return old_ptr;
}

bool ShdCompiler::Compile(const char* file_name)
{
    DLGPakStream in(file_name);
    if(in.IsError()){
        p_output->OnError(file_name, 0, SC_FILE_OPEN_ERR);
        errors_count = 1;
        last_line = 0;

        return false;
    }

    //make shader parser
    ShdLexer            lexer(&in);
    ANTLRTokenBuffer    pipe(&lexer,1);
    FastToken           aToken;
    OutputWrapper       wrapper(file_name, p_output, &lexer);
    ShdRecorder         recorder(&pipe, &wrapper);

    lexer.setToken(&aToken);

    try{
        //compile file
        recorder.Run();
    }
    catch(ErrLimit)     { /*printf("\nerror limit\n");*/ }
    catch(LexerErr)     { wrapper.OnError(SC_LEXER_ERR); }

    catch(ParserErr)    { wrapper.OnError(SC_PARSER_ERR);}
    catch(TokenErr)     { wrapper.OnError(SC_TOKEN_ERR); }
    catch(TokBufferErr) { wrapper.OnError(SC_TOKBUFFER_ERR);}
    catch(PCCTS_Err)    { wrapper.OnError(SC_UNKNOWN_ERR);}
    
    //get last line & error count
    last_line    = lexer.line();
    errors_count = wrapper.GetErrorsCount();

    return (errors_count == 0);
}
