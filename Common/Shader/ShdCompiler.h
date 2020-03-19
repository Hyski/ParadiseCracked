//
//   ShdCompiler class description
//
//============================================
//   FILE:  ShdCompiler.h   
//

#ifndef _PUNCH_SHD_COMPILER_
#define _PUNCH_SHD_COMPILER_

class ShaderOutput;

class ShdCompiler{
public:
    ShdCompiler(ShaderOutput* p_out = 0);
    ~ShdCompiler();

    //Compile shader file
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // return:
    //      true -  success, no error
    //      flase - was compiled with some erorrs
    //
    bool Compile(const char* file_name);

    // lines parsed 
    //
    int LinesCount() const;

    // number of errors while parsing the file
    //
    int ErrorsCount() const;

    // set link to ShaderOutput
    //
    ShaderOutput* SetOutput(ShaderOutput* pOut);

private:

    ShaderOutput*   p_output;
    
    int   last_line;         // lines number
    int   errors_count;      // errors number
};

//
//~~~~~~~~~~~~~~~~~~~~~~~~~inlines~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

inline int ShdCompiler::LinesCount() const 
{
    return last_line;
}

inline int ShdCompiler::ErrorsCount() const
{
    return errors_count;
}

#endif //_PUNCH_SHD_COMPILER_

