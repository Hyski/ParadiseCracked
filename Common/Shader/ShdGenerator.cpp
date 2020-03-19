//
// ShdGenerator class methods
//

#include "precomp.h"

#include "ShdGenerator.h"

ShdGenerator::ShdGenerator(const char* file_name):
    m_file(0)
{
    if(file_name) SetFile(file_name);
}

ShdGenerator::~ShdGenerator()
{
    if(m_file) fclose(m_file);
}

int ShdGenerator::Dump(const char* shd_name, const shader_struct& shd)
{
    if(!m_file)  return 0;
    
    //shader begins
    print("\n%s\n{\n", shd_name);

    PrintGlobals(shd);

    for(int i = 0; i < shd.num_stages; i++){
        PrintStage(shd.stage[i]);
    }

    //end shader
    print("}\n");
    
    return 1;
}

void ShdGenerator::print(char* str, ...)
{
    va_list arg;

    va_start( arg, str );
    vfprintf( m_file, str, arg );
    va_end  ( arg);

    fflush(m_file);
}

void ShdGenerator::PrintCull(unsigned flags)
{
    if(flags & SHD_CULL_BACK)  print("\tcull  back\n");
    if(flags & SHD_CULL_FRONT) print("\tcull  front\n");
    //if(!(flags & (SHD_CULL_BACK|SHD_CULL_FRONT))) print("\tcull  none\n");
}

void ShdGenerator::PrintVerts(unsigned flags)
{
    if(flags & VERTS_AUTOSPRITE) print("\tdeformVertexes  autosprite\n");                
    if(flags & VERTS_AUTOSPRITE2) print("\tdeformVertexes  autosprite2\n");
}

void ShdGenerator::PrintSteps(SHD_STEPS_ATTR type)
{
    switch(type){
    case STP_SILENT:
        //print("\tsteps  silent\n");
        break;

    case STP_METAL:
        print("\tsteps  metal\n");
        break;

    case STP_GROUND:
        print("\tsteps  ground\n");
        break;

    case STP_WATER:
        print("\tsteps  water\n");
        break;

    case STP_ASPHALT:
        print("\tsteps asphalt\n");
        break;

    case STP_PLASTIC:
        print("\tsteps  plastic\n");
        break;
    }
}

void ShdGenerator::PrintImpact(SHD_IMPACT_ATTR type)
{
    switch(type){
    case IMP_NONE:
        //print("\timpact  none\n");
        break;

    case IMP_BLOOD:
        print("\timpact  blood\n");
        break;

    case IMP_DUST:
        print("\timpact  dust\n");
        break;

    case IMP_SPARKLES:
        print("\timpact  sparkles\n");
        break;
    }
}

void ShdGenerator::PrintMarks(SHD_MARKS_ATTR type)
{
    switch(type){
    case MRK_NONE:
        //print("\tmarks  none\n");
        break;

    case MRK_BLACK:
        print("\tmarks  black\n");
        break;

    case MRK_WHITE:
        print("\tmarks  white\n");
        break;

    case MRK_HOT:
        print("\tmarks  hot\n");
        break;
    }
}

void ShdGenerator::PrintSurfParams(unsigned flags)
{
    if(flags & SHD_ALPHASHADOW) print("\tsurfaceParam  alphashadow\n");
    if(flags & SHD_FLESH)  print("\tsurfaceParam  flesh\n");
    if(flags & SHD_FOG)  print("\tsurfaceParam  fog\n");
    if(flags & SHD_NODLIGHT)  print("\tsurfaceParam  nodlight\n");
    if(flags & SHD_NODRAW)  print("\tsurfaceParam  nodraw\n");
    if(flags & SHD_NOLIGHTMAP)  print("\tsurfaceParam  nolightmap\n");
    if(flags & SHD_NONSOLID)  print("\tsurfaceParam  nonsolid\n");
    if(flags & SHD_ORIGIN)  print("\tsurfaceParam  origin\n");
    if(flags & SHD_PLAYERCLIP)  print("\tsurfaceParam  playerclip\n");
    if(flags & SHD_TRANS)  print("\tsurfaceParam  trans\n");
    if(flags & SHD_FLAMMABLE) print("\tsurfaceParam flammable\n");
}

void ShdGenerator::PrintGlobals(const shader_struct& shd)
{    
    PrintCull(shd.flags);
    PrintMarks(shd.marks);
    PrintImpact(shd.impact);
    PrintSteps(shd.steps);
    if(shd.armour) print("\tarmour  %.3f\n", shd.armour);

    if(shd.flags & SHD_SURFACEPAM) PrintSurfParams(shd.surfaceParam);
    if(shd.flags & SHD_DEFORMVERTEXES) PrintVerts(shd.deformVertexes);
    if(shd.flags & SHD_POLYGONOFFSET)  print("\tpolygonOffset\n");
}

void ShdGenerator::PrintStage(const stage_struct& stg)
{
    // stage begins
    print("\t{\n");

    PrintStgMap( stg);

    if(stg.flags & STG_TCGEN_FUNC) PrintTcGenFunc(stg.tcGen);
    
    if((stg.flags & STG_TCMOD_FUNC) || stg.num_tcMods){
        for(int i=0; i< stg.num_tcMods; i++){
            PrintTcModFunc(stg.tcMod[i]);
        }
    }

    if(stg.flags & STG_RGBGEN_FUNC) PrintRAGen(true, stg.rgbGen);
    if(stg.flags & STG_ALPHAGEN_FUNC) PrintRAGen(false, stg.alphaGen);

    if(stg.flags & STG_BLEND_FUNC){
        print("\t\tblendFunc ");
        PrintBlendArg(stg.fnBlend.src);
        PrintBlendArg(stg.fnBlend.dst);
        print("\n");
    }

    PrintDepthFunc(stg.depth_func);

    if(    stg.fm_type != FM_SOLID
        || stg.af_type != AF_NONE
        || stg.flags & STG_DEPTHWRITE
        || stg.zBias){
        
        if(stg.fm_type != FM_SOLID) PrintFillMode(stg.fm_type);
        if(stg.af_type != AF_NONE) PrintAlphaFunc(stg.af_type);
        if(stg.flags & STG_DEPTHWRITE)print("\t\tdepthWrite\n");
        if(stg.zBias)PrintZBias(stg.zBias);
    }

    if(stg.flags & STG_NOMIPMAP) print("\t\tnomipmap\n");
    if(stg.flags & STG_NOPICMIP) print("\t\tnopicmip\n");

    // stage end
    print("\t}\n");
}


void ShdGenerator::PrintStgMap(const stage_struct& stg)
{
    int j;

    switch(stg.map_type){
    case STG_LIGHTMAP:
        print("\t\tmap  $lightmap\n");
        break;

    case STG_MAP:
         print("\t\tmap  %s\n",stg.texture[0]);
         break;

    case STG_ANIMMAP:
         print("\t\tanimmap %.3f \n",stg.anim_freq);
         
         for(j = 0;  j < stg.num_textures; j++){
            print("\t\t\t\t%s\n", stg.texture[j]);
         }
    
         break;

    case STG_CLAMPMAP:
         print("\t\tclampmap  %s\n", stg.texture[0]);
         break;
    }
}

void ShdGenerator::PrintBlendArg(int type)
{
    switch(type){
    case BLEND_ONE:
        print(" ONE");
        break;

    case BLEND_ZERO:
        print(" ZERO");
        break;

    case BLEND_DST_COLOR:
        print(" DST_COLOR");
        break;

    case BLEND_SRC_COLOR:
        print(" SRC_COLOR");
        break;

    case BLEND_SRC_ALPHA:
        print(" SRC_ALPHA");
        break;

    case BLEND_INV_SRC_ALPHA:
        print(" INV_SRC_ALPHA");
        break;

    case BLEND_INV_DST_COLOR:
        print(" INV_DST_COLOR");
        break;

    case BLEND_INV_SRC_COLOR:
        print(" INV_SRC_COLOR");
        break;
    }
}

void ShdGenerator::PrintTcGenFunc(TCGEN_TYPE type)
{
    print("\t\ttcGen  ");

    switch(type){
    case TCGEN_ENVIROMENT:
        print("enviroment\n");
        break;

    case TCGEN_REFLECTION:
        print("reflection\n");
        break;

    case TCGEN_LIGHTSPOT:
        print("lightspot\n");
        break;

    case TCGEN_SPHERE:
        print("sphere\n");
        break;
    }
}

void ShdGenerator::PrintTcModFunc(const tcMod_func& tcMod)
{
    print("\t\ttcMod  ");

    switch(tcMod.type){
    case TCMOD_EMBOSS:
        print("emboss  %.3f  %.3f  %.3f\n",tcMod.arg[0], tcMod.arg[1],tcMod.arg[2]);
        break;

    case TCMOD_ROTATE:
        print("rotate  %.3f\n",tcMod.arg[0]);
        break;

    case TCMOD_SCALE:
        print("scale  %.3f  %.3f\n", tcMod.arg[0], tcMod.arg[1]);
        break;

    case TCMOD_SCROLL:
        print("scroll  %.3f  %.3f\n", tcMod.arg[0], tcMod.arg[1]);
        break;

    case TCMOD_TURB:
        print("turb  %.3f  %.3f  %.3f  %.3f\n", 
               tcMod.arg[0], tcMod.arg[1],
               tcMod.arg[2], tcMod.arg[3]);
        break;
        
    case TCMOD_STRETCH:
        print("stretch  %s  %.3f  %.3f  %.3f  %.3f\n",
                PrintWaveForm(tcMod.wave_form), tcMod.arg[0], 
                tcMod.arg[1],tcMod.arg[2], tcMod.arg[3]);
        break;
    }
}

void ShdGenerator::PrintDepthFunc(DEPTH_FUNC type)
{
    switch(type){
    case  DF_LEQUAL:
        //print("\t\tdepthFunc  lequal\n");
        break;

    case DF_EQUAL:
        print("\t\tdepthFunc  equal\n");
        break;

    case DF_ALWAYS:
        print("\t\tdepthFunc  always\n");
        break;
    }
}

void ShdGenerator::PrintZBias(int val)
{
    print("\t\tzBias %i\n",val);
}

void ShdGenerator::PrintAlphaFunc(int val)
{
    switch(val){
    case AF_GT0:
        print("\t\tAlphaFunc  GT0\n");
        break;

    case AF_GE128:
        print("\t\tAlphaFunc  GE128\n");
        break;

    case AF_LT128:
        print("\t\tAlphaFunc  LT128\n");
        break;
    }
}

void ShdGenerator::PrintFillMode(int val)
{
    switch(val){
    case FM_POINT:
        print("\t\tFillMode  point\n");
        break;

    case FM_SOLID:
        print("\t\tFillMode  solid\n");
        break;

    case FM_WIRE:
        print("\t\tFillMode  wire\n");
        break;
    }
}

const char* ShdGenerator::PrintWaveForm(int value)
{
    switch(value){
    case WF_SIN:
        return "sin";

    case WF_TRIANGLE:
        return "triangle";

    case WF_SQUARE:
        return "square";
 
    case WF_SAWTOOTH:
        return "sawtooth";

    case WF_INVSAWTOOTH:
        return "invsawtooth";
    }

    return "unknown";
}

void ShdGenerator::PrintRAGen(bool is_rgbGen, const ColorGen_func& func)
{
    if(is_rgbGen)
        print("\t\trgbGen  ");
    else
        print("\t\talphaGen  ");

    switch(func.type){
    case CG_IDENTITY:
        print("identity\n");
        break;

    case CG_WAVE:
        print("wave  %s  %.3f  %.3f  %.3f  %.3f\n",
               PrintWaveForm(func.wave_form), func.arg[0], 
               func.arg[1], func.arg[2], func.arg[3]);
        break;

    case CG_LIGHTINGDIFFUSE:
        print("lightingDiffuse\n");
        break;

    case CG_VERTEX:
        print("vertex\n");
        break;
    }
}


