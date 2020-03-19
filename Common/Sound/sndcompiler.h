#ifndef  __SNDCOMPILER_H__
#define  __SNDCOMPILER_H__

enum SND_ERRORS{
    SUCCESSED,
    FILE_OPEN_ERROR,
    LEXER_ERROR,
    PARSER_ERROR,
    TOKEN_ERROR,
    TOKEN_BUFFER_ERROR,
    UNKNOWN_ERROR
};

class SndCompiler{
public:
    SndCompiler() { m_line = 0; m_error = 0; };
    ~SndCompiler() {};

    bool Compile     (const char* file_name);
    int  GetLine     () const { return m_line; }
    int  GetLastError() const { return m_error;} 

private:

    int m_line;
    int m_error;
};

#endif //__SNDCOMPILER_H__