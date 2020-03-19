//
//   ShdRecorder class - records shader struct and inserts 
//                       it in the storage
//
//============================================
//   FILE:  ShdRecorder.h   
//

#ifndef _PUNCH_SHD_RECORDER_H_
#define _PUNCH_SHD_RECORDER_H_

#include "shader.h"
#include "ShdParser.h"

class ShdRecorder: protected ShdParser{
public:

    ShdRecorder(ANTLRTokenBuffer* p_buff, OutputWrapper* p_out);
    ~ShdRecorder();

    void Run();
    
private:
    
    void NewPath(const char* str);
    void AddPath(const char* str);

    void BegShader(const char* name);
    void EndShader();

    void BegStage();
    void EndStage();

    void SetFillMode(int type);
    void SetZBias(int value);
    void SetAlphaFunc(int type);

    void SetArmour(float val);
    void SetSteps(int type);
    void SetImpact(int type);
    void SetMarks(int type);
    void SetCull(int type);
    void SetSurfParam(int type);

    void SetDeformVerts(int type);
    void SetPolygonOffset();

    void SetLightmap();
    void SetMap(const char* path, bool clampmap = false);
    void SetAnimMap(float freq);
    void AddTexture(const char* path);
    void SetBlendFunc(int src, int dst);
    void SetTcGenFunc(int type);
    void SetTcModFunc(int type, float arg[], int wave = 0);
    void SetDepthFunc(int type);
    void SetDepthWrite();
    void SetNoMipMap();
    void SetNoPicMip();


    void SetRAGen(int gen, int type, int wave_form = 0, float arg[] = 0);
    
    const char* GetPath();

    BLEND_TYPE GetBlendArg(int type);
    WAVE_FORM GetWaveForm(int type);
    COLOR_GEN_TYPE GetColorGen(int type);

    void SetRAGenFunc(ColorGen_func& func, COLOR_GEN_TYPE type,
                     WAVE_FORM wave_form, float arg[]);
    
private:
   
    std::string     path;   
    shader_struct   shader;
    
    char            shd_name[MAX_TEXTUREPATH];
};

//
//~~~~~~~~~~~~~~~~~~~ShdRecorder inlines~~~~~~~~~~~~~~~~~~~
//

inline void ShdRecorder::Run()
{
    int signal;

    init();
    start(&signal);
}

inline void ShdRecorder::NewPath(const char* str)
{
    path.erase();
    path += str;
}

inline void ShdRecorder::AddPath(const char* str)
{
    path += str;
}

inline const char* ShdRecorder::GetPath()
{
    return path.c_str();
}

inline void ShdRecorder::SetPolygonOffset()
{
    shader.flags |= SHD_POLYGONOFFSET;
}

inline void ShdRecorder::SetDepthWrite()
{
    shader.stage[shader.num_stages].flags |= STG_DEPTHWRITE;
}

inline void ShdRecorder::SetArmour(float val)
{
    shader.armour = val;
}

#endif //_PUNCH_SHD_RECORDER_H_