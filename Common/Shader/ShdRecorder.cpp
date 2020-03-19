//
// ShdRecorder methods
//

#include "precomp.h"

#include "tokens.h"
#include "ShdRecorder.h"
#include "ShdCompiler.h"
#include "OutWrapper.h"

#include <algorithm>

ShdRecorder::ShdRecorder(ANTLRTokenBuffer* p_buff, OutputWrapper* p_out):
  ShdParser(p_buff) 
{
    p_output = p_out;
}

ShdRecorder::~ShdRecorder()
{
}

void ShdRecorder::SetSteps(int type)
{
    if(shader.steps != STP_SILENT)
        p_output->OnError(SC_SECOND_STEPS_ERR);

    switch(type){
    case T_SILENT:
        shader.steps = STP_SILENT;
        break;

    case T_METAL:
        shader.steps = STP_METAL;
        break;

    case T_GROUND:
        shader.steps = STP_GROUND;
        break;

    case T_WATER:
        shader.steps = STP_WATER;
        break;

    case T_ASPHALT:
        shader.steps = STP_ASPHALT;
        break;

    case T_PLASTIC:
        shader.steps = STP_PLASTIC;
        break;
    }
}

void ShdRecorder::SetImpact(int type)
{
    if(shader.impact != IMP_NONE)
        p_output->OnError(SC_SECOND_IMPACT_ERR);

    switch(type){
    case T_NONE:
        shader.impact = IMP_NONE;
        break;

    case T_BLOOD:
        shader.impact = IMP_BLOOD;
        break;

    case T_DUST:
        shader.impact = IMP_DUST;
        break;

    case T_SPARKLES:
        shader.impact = IMP_SPARKLES;
        break;
    }
}

void ShdRecorder::SetMarks(int type)
{
    if(shader.marks != MRK_NONE)
        p_output->OnError(SC_SECOND_MARKS_ERR);

    switch(type){
    case T_NONE:
        shader.marks = MRK_NONE;
        break;

    case T_BLACK:
        shader.marks = MRK_BLACK;
        break;

    case T_WHITE:
        shader.marks = MRK_WHITE;
        break;

    case T_HOT:
        shader.marks = MRK_HOT;
        break;
    }
}

void ShdRecorder::SetNoMipMap()
{
    stage_struct*  stg = &shader.stage[shader.num_stages];
    
    if(stg->flags & STG_NOMIPMAP) 
        p_output->OnError(SC_SECOND_NOMIPMAP_ERR);

    stg->flags |= STG_NOMIPMAP;
}

void ShdRecorder::SetNoPicMip()
{
    stage_struct*  stg = &shader.stage[shader.num_stages];
    
    if(stg->flags & STG_NOPICMIP) 
        p_output->OnError(SC_SECOND_NOPICMIP_ERR);

    stg->flags |= STG_NOPICMIP;
}

void ShdRecorder::SetLightmap()
{
    stage_struct* p_stg = &shader.stage[shader.num_stages];

    if(p_stg->map_type != STG_NULL_MAP)
        p_output->OnError(SC_SECOND_MAP_ERR);

    p_stg->map_type = STG_LIGHTMAP;
}

void ShdRecorder::SetMap(const char* path, bool clampmap)
{
    stage_struct* p_stg = &shader.stage[shader.num_stages];

    if(p_stg->map_type != STG_NULL_MAP)
        p_output->OnError(SC_SECOND_MAP_ERR);

    AddTexture(path);

    if(clampmap)
        p_stg->map_type = STG_CLAMPMAP; 
    else
        p_stg->map_type = STG_MAP; 
}

void ShdRecorder::SetAnimMap(float freq)
{
    stage_struct* p_stg = &shader.stage[shader.num_stages];

    if(p_stg->map_type != STG_NULL_MAP)
        p_output->OnError(SC_SECOND_MAP_ERR);

    p_stg->map_type = STG_ANIMMAP;
    p_stg->anim_freq = freq;
}

void ShdRecorder::SetBlendFunc(int src, int dst)
{
    stage_struct* p_stg = &shader.stage[shader.num_stages];

    if(p_stg->flags & STG_BLEND_FUNC)
        p_output->OnError(SC_SECOND_BLEND_ERR);

    p_stg->flags |= STG_BLEND_FUNC;

    p_stg->fnBlend.src = GetBlendArg(src);
    p_stg->fnBlend.dst = GetBlendArg(dst);
}

void ShdRecorder::SetDepthFunc(int type)
{
   stage_struct* p_stg = &shader.stage[shader.num_stages];
   
   if(p_stg->depth_func != DF_LEQUAL)
       p_output->OnError(SC_SECOND_DEPTHFUNC_ERR);

   switch(type){
   case T_EQUAL:
       p_stg->depth_func = DF_EQUAL;
       break;

   case T_LEQUAL:
       p_stg->depth_func = DF_LEQUAL;
       break;

   case T_ALWAYS:
       p_stg->depth_func = DF_ALWAYS;
       break;
   }
}

void ShdRecorder::SetZBias(int value)
{
   stage_struct* p_stg = &shader.stage[shader.num_stages];

   //fool protection
   if(p_stg->zBias)
       p_output->OnError(SC_SECOND_ZBIAS);

   p_stg->zBias = std::max(std::min(0,value),16);
}

void ShdRecorder::BegShader(const char* name)
{
    //already existing shader?
    if( p_output->IsExist(name) )
        p_output->OnError(SC_NAME_REPLICATION);  

    //set shader name and init shader
    strncpy(shd_name, name, MAX_TEXTUREPATH);
    shd_name[MAX_TEXTUREPATH-1] = 0;

    shader.init();
}

void ShdRecorder::EndShader()
{
    p_output->OnShader(shd_name, shader);
}

void ShdRecorder::SetCull(int type)
{    
    //test: cull was already set?
    if(shader.flags & (SHD_CULL_BACK|SHD_CULL_FRONT))
        p_output->OnError(SC_SECOND_CULL);

    switch(type){
    case T_FRONT:
        shader.flags |= SHD_CULL_FRONT;
        break;

    case T_BACK:
        shader.flags |= SHD_CULL_BACK;
        break;

    case T_NONE:
        shader.flags |= ~(SHD_CULL_BACK|SHD_CULL_FRONT);
        break;
    }
}

void ShdRecorder::SetSurfParam(int type)
{
    shader.flags |= SHD_SURFACEPAM;

    switch(type){
    case T_ALPHASHADOW:
        shader.surfaceParam |= SHD_ALPHASHADOW;
        break;

    case T_FLESH:
        shader.surfaceParam |= SHD_FLESH;
        break;

    case T_FOG:
        shader.surfaceParam |= SHD_FOG;
        break;

    case T_NODLIGHT:
        shader.surfaceParam |= SHD_NODLIGHT;
        break;

    case T_NODRAW:
        shader.surfaceParam |= SHD_NODRAW;
        break;

    case T_NOLIGHTMAP:
        shader.surfaceParam |= SHD_NOLIGHTMAP;
        break;

    case T_NONSOLID:
        shader.surfaceParam |= SHD_NONSOLID;
        break;

    case T_ORIGIN:
        shader.surfaceParam |= SHD_ORIGIN;
        break;

    case T_PLAYERCLIP:
        shader.surfaceParam |= SHD_PLAYERCLIP;
        break;

    case T_TRANS:
        shader.surfaceParam |= SHD_TRANS;
        break;

    case T_FLAMMABLE:
        shader.surfaceParam |= SHD_FLAMMABLE;
        break;
    }        
}

void ShdRecorder::SetDeformVerts(int type)
{
    shader.flags |= SHD_DEFORMVERTEXES;

    switch(type){
    case T_AUTOSPRITE:
        shader.deformVertexes |= VERTS_AUTOSPRITE;
        break;

    case T_AUTOSPRITE2:
        shader.deformVertexes |= VERTS_AUTOSPRITE2;
        break;
    }
}

void ShdRecorder::BegStage()
{
    if(shader.num_stages >= MAX_STAGES)
        p_output->OnError(SC_STAGE_NUM_ERR);

    shader.stage[shader.num_stages].init();
}

void ShdRecorder::EndStage()
{
    stage_struct*  p_stg = &shader.stage[shader.num_stages];
    
    //check stage accuracy
    if( p_stg->map_type == STG_NULL_MAP)
        p_output->OnError(SC_NO_MAP_ERR);

    //commit
    shader.num_stages++;
}

BLEND_TYPE ShdRecorder::GetBlendArg(int type)
{
    switch(type){
    case T_ONE:
        return BLEND_ONE;

    case T_ZERO:
        return BLEND_ZERO;

    case T_DST_COLOR:
        return BLEND_DST_COLOR;

    case T_SRC_COLOR:
        return BLEND_SRC_COLOR;

    case T_SRC_ALPHA:
        return BLEND_SRC_ALPHA;

    case T_INV_SRC_ALPHA:
        return BLEND_INV_SRC_ALPHA;

    case T_INV_SRC_COLOR:
        return BLEND_INV_SRC_COLOR;

    case T_INV_DST_COLOR:
        return BLEND_INV_DST_COLOR;
    }

    return BLEND_NONE;
}

void ShdRecorder::SetTcModFunc(int type, float arg[], int wave)
{
    stage_struct*  p_stg = &shader.stage[shader.num_stages];

    if(p_stg->num_tcMods >= MAX_TCMOD_FUNCS)
        p_output->OnError(SC_TCMODS_FUNCS_ERR);

    switch(type){
    case T_EMBOSS:
        p_stg->tcMod[p_stg->num_tcMods].type = TCMOD_EMBOSS;
        p_stg->tcMod[p_stg->num_tcMods].arg[0] = arg[0];
        p_stg->tcMod[p_stg->num_tcMods].arg[1] = arg[1];
        p_stg->tcMod[p_stg->num_tcMods].arg[2] = arg[2];
        break;

    case T_ROTATE:
        p_stg->tcMod[p_stg->num_tcMods].type = TCMOD_ROTATE;
        p_stg->tcMod[p_stg->num_tcMods].arg[0] = arg[0];
        break;

    case T_SCALE:
        p_stg->tcMod[p_stg->num_tcMods].type = TCMOD_SCALE;
        p_stg->tcMod[p_stg->num_tcMods].arg[0] = arg[0];
        p_stg->tcMod[p_stg->num_tcMods].arg[1] = arg[1];
        break;

    case T_SCROLL:
        p_stg->tcMod[p_stg->num_tcMods].type = TCMOD_SCROLL;
        p_stg->tcMod[p_stg->num_tcMods].arg[0] = arg[0];
        p_stg->tcMod[p_stg->num_tcMods].arg[1] = arg[1];
        break;

    case T_STRETCH:
        p_stg->tcMod[p_stg->num_tcMods].type = TCMOD_STRETCH;
        p_stg->tcMod[p_stg->num_tcMods].wave_form = GetWaveForm(wave);
        p_stg->tcMod[p_stg->num_tcMods].arg[0] = arg[0];
        p_stg->tcMod[p_stg->num_tcMods].arg[1] = arg[1];
        p_stg->tcMod[p_stg->num_tcMods].arg[2] = arg[2];
        p_stg->tcMod[p_stg->num_tcMods].arg[3] = arg[3];
        break;

    case T_TURB:
        p_stg->tcMod[p_stg->num_tcMods].type = TCMOD_TURB;
        p_stg->tcMod[p_stg->num_tcMods].arg[0] = arg[0];
        p_stg->tcMod[p_stg->num_tcMods].arg[1] = arg[1];
        p_stg->tcMod[p_stg->num_tcMods].arg[2] = arg[2];
        p_stg->tcMod[p_stg->num_tcMods].arg[3] = arg[3];
        break;
    };

    p_stg->flags |= STG_TCMOD_FUNC;
    p_stg->num_tcMods++;
}

void ShdRecorder::SetTcGenFunc(int type)
{
    stage_struct* p_stg = &shader.stage[shader.num_stages];

    if(p_stg->flags & STG_TCGEN_FUNC)
        p_output->OnError(SC_SECOND_TCGEN_ERR); 
    
    p_stg->flags |= STG_TCGEN_FUNC;

    switch(type){
    case T_ENVIROMENT:
        p_stg->tcGen = TCGEN_ENVIROMENT;
        break;

    case T_REFLECTION:
        p_stg->tcGen = TCGEN_REFLECTION;
        break;

    case T_LIGHTSPOT:
        p_stg->tcGen = TCGEN_LIGHTSPOT;
        break;

    case T_SPHERE:
        p_stg->tcGen = TCGEN_SPHERE;
        break;
    }
}

void ShdRecorder::AddTexture(const char* path)
{
    stage_struct*   p_stg = &shader.stage[shader.num_stages];
    
    if(p_stg->num_textures >= MAX_ANIM_FRAMES)
        p_output->OnError(SC_ANIM_FRAMES_ERR);
    
    //insert texture
    strncpy(p_stg->texture[p_stg->num_textures], path, MAX_TEXTUREPATH);
    p_stg->texture[p_stg->num_textures][MAX_TEXTUREPATH-1] = '\0';
    
    //set stage fields
    p_stg->num_textures++;
}

void ShdRecorder::SetFillMode(int type)
{
    stage_struct* p_stg = &shader.stage[shader.num_stages];

    if(p_stg->fm_type != FM_SOLID)
        p_output->OnError(SC_SECOND_FILLMODE);

    switch(type){
    case T_POINT:
        p_stg->fm_type = FM_POINT;
        break;

    case T_WIRE:
        p_stg->fm_type = FM_WIRE;
        break;

    case T_SOLID:
        p_stg->fm_type = FM_SOLID;
        break;
    }
}

void ShdRecorder::SetAlphaFunc(int type)
{
   stage_struct* p_stg = &shader.stage[shader.num_stages];

   if(p_stg->af_type != AF_NONE)
       p_output->OnError(SC_SECOND_ALPHAFUNC);

   switch(type){
   case T_GT0:
       p_stg->af_type = AF_GT0;
       break;

   case T_GE128:
       p_stg->af_type = AF_GE128;
       break;

   case T_LT128:
       p_stg->af_type = AF_LT128;
       break;
   }
}

WAVE_FORM ShdRecorder::GetWaveForm(int type)
{
    switch(type){
    case T_SAWTOOTH:
        return WF_SAWTOOTH;

    case T_SIN:
        return WF_SIN;

    case T_SQUARE:
        return WF_SQUARE;

    case T_TRIANGLE:
        return WF_TRIANGLE;
    
    case T_INVSAWTOOTH:
        return WF_INVSAWTOOTH;
    }

    return WF_NONE;
}

COLOR_GEN_TYPE ShdRecorder::GetColorGen(int type)
{
    switch(type){
    case T_IDENTITY:
        return CG_IDENTITY;

    case T_WAVE:
        return CG_WAVE;

    case T_LIGHTINGDIFFUSE:
        return CG_LIGHTINGDIFFUSE;
    
    case T_VERTEX:
        return CG_VERTEX;
    }

    return CG_NONE;
}

void ShdRecorder::SetRAGen(int gen, int type, int wave_form, float arg[])
{
    stage_struct* p_stg = &shader.stage[shader.num_stages];

    if(gen == T_RGBGEN){

        if(p_stg->flags & STG_RGBGEN_FUNC)
            p_output->OnError(SC_SECOND_RGBGEN);

        SetRAGenFunc(p_stg->rgbGen, GetColorGen(type), GetWaveForm(wave_form), arg);

        p_stg->flags |= STG_RGBGEN_FUNC;

    }else{
        if(p_stg->flags & STG_ALPHAGEN_FUNC)
            p_output->OnError(SC_SECOND_ALPHAGEN);

        SetRAGenFunc(p_stg->alphaGen, GetColorGen(type), GetWaveForm(wave_form), arg);

        p_stg->flags |= STG_ALPHAGEN_FUNC;
    }
}
    
void ShdRecorder::SetRAGenFunc(ColorGen_func& func, COLOR_GEN_TYPE type, 
                                      WAVE_FORM wave_form, float arg[])
{
    func.type = type;

    if(func.type == CG_WAVE){

        func.wave_form = wave_form;
        
        func.arg[0] = arg[0];
        func.arg[1] = arg[1];
        func.arg[2] = arg[2];
        func.arg[3] = arg[3];
    }
}
