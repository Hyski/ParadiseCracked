// 
// OutputWrapper - ShaderOutputwrapper
//
//==============================================
// File: OutWrapper.h
//

#ifndef _PUNCH_OUTWRAPPER_H_
#define _PUNCH_OUTWRAPPER_H_

#ifndef _PUNCH_SHADER_H_
#include "shader.h"
#endif 

#include "dlexerbase.h"

class ErrLimit{}; //error overflaw handling

class OutputWrapper{
public: 
    OutputWrapper(const char* fname, ShaderOutput* p_out, DLGLexerBase* p_lex):
      p_lexer(p_lex), p_output(p_out), file_name(fname), error_counter(0), no_error(true){}

    bool IsExist(const char* shd_name);
    void OnShader(const char* shd_name, const shader_struct& shd);

    void ResetErrFlag();
    void OnError(SC_ERRORS type);

    int  GetErrorsCount() const;

private:
    DLGLexerBase*  p_lexer;
    ShaderOutput*  p_output;
    std::string    file_name;
    
    int  error_counter;
    bool no_error;
};

//
//~~~~~~~~~~~~~~~~~~~OutputWrapper inlines~~~~~~~~~~~~~~~~~~~
//

inline void OutputWrapper::ResetErrFlag()
{
    no_error = true;
}

inline bool OutputWrapper::IsExist(const char* shd_name)
{
    return p_output->IsExist(shd_name);
}

inline void OutputWrapper::OnShader(const char* shd_name, const shader_struct& shd)
{
   if(no_error) p_output->OnShader(shd_name, shd);
}

inline void OutputWrapper::OnError(SC_ERRORS type)
{
    if(no_error){
        error_counter++;
        no_error = false;

        if(!p_output->OnError(file_name.c_str(), p_lexer->line(), type))
            throw ErrLimit();
    }
}

inline int OutputWrapper::GetErrorsCount() const
{
    return error_counter;
}

#endif // _PUNCH_OUTWRAPPER_H_