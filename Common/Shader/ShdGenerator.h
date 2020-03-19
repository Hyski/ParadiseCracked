//
//   ShdGenerator class - generates shader script from 
//                        shader sturct
//
//============================================
//   FILE:  ShdGenerator.h   
//

#ifndef _PUNCH_SHD_GENERATOR_H_
#define _PUNCH_SHD_GENERATOR_H_

#include "shader.h"

class ShdGenerator {
public:

    ShdGenerator(const char* file_name = 0);
    ~ShdGenerator();

    // Generate shader script
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // args:
    //      file        -   output file handler
    //      shd_name    -   shader name
    //      itor        -   iterator in ShaderTable
    // retuns:
    //     != 0   -   all OK
    //     == 0   -   wrong shader name or bad iterator
    //
    int Dump(const char* shd_name, const shader_struct& shd);

    bool SetFile(const char* file_name);
    
private:

    void PrintSteps(SHD_STEPS_ATTR type);
    void PrintImpact(SHD_IMPACT_ATTR type);
    void PrintMarks(SHD_MARKS_ATTR type);
    void PrintCull (unsigned flags);
    void PrintVerts(unsigned flags);
    void PrintSurfParams(unsigned flags);
    void PrintGlobals(const shader_struct& shd);
    void PrintStage(const stage_struct& stg);
    void PrintStgMap(const stage_struct& stg);
    void PrintBlendArg(int type);
    void PrintTcGenFunc(TCGEN_TYPE type);
    void PrintTcModFunc(const tcMod_func& tcMod);
    void PrintDepthFunc(DEPTH_FUNC type);
    void PrintZBias(int value);
    void PrintAlphaFunc(int value);
    void PrintFillMode(int value);
    void PrintRAGen(bool is_rgbGen, const ColorGen_func& func);
        
    const char* PrintWaveForm(int value);

    void print(char* str, ...);

private:

    FILE*   m_file;
};

//
//~~~~~~~~~~~~~~~~~inlines~~~~~~~~~~~~~~~~~~~~~
//
inline bool ShdGenerator::SetFile(const char* file_name)
{
    FILE* file = fopen(file_name,"wt");

    if(file){
        if(m_file) fclose(m_file);
        m_file = file;
    }

    return (file != 0);
}   

#endif //_PUNCH_SHD_GENERATOR_H_