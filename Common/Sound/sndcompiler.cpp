#include "tokens.h"
#include "AToken.h"
#include "atokenbuffer.h"
#include "SndLexer.h"
#include "SndRecorder.h"

#include <iostream>
#include <map>

#include "sndcompiler.h"

bool SndCompiler::Compile(const char* file_name)
{
    FILE*   file;
    m_error = SUCCESSED;

    file = fopen(file_name, "rt");
    if(!file){
        std::cout << "file open error\n";
        m_error = FILE_OPEN_ERROR;
        m_line  = 0;

        return false;
    }

    //make sound parser
    DLGFileInput        in(file);
    SndLexer            lexer(&in);
    ANTLRTokenBuffer    pipe(&lexer,1);
    FastToken           aToken;
    std::map<std::string, DSSndDesc> Map_of_Descs;
    SndRecorder         recorder(&pipe, &Map_of_Descs);

    lexer.setToken(&aToken);

    try{
        //compile file
        recorder.Run();
    }
    //catch(ErrLimit)     { /*printf("\nerror limit\n");*/ }
    catch(LexerErr)     { std::cout << "lexer error\n";        m_error = LEXER_ERROR; }
    catch(ParserErr)    { std::cout << "parser error\n";       m_error = PARSER_ERROR;}
    catch(TokenErr)     { std::cout << "token error\n";        m_error = TOKEN_ERROR;}
    catch(TokBufferErr) { std::cout << "token buffer error\n"; m_error = TOKEN_BUFFER_ERROR;}
    catch(PCCTS_Err)    { std::cout << "unknown error\n";      m_error = UNKNOWN_ERROR;}
    
    //get last line & error count
    m_line = lexer.line();
    
    return (m_error == SUCCESSED);
}