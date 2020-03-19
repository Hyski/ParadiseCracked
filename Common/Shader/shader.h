//
//   Shader struct description
//
//============================================
//   FILE:  shader.h   
//

#ifndef _PUNCH_SHADER_H_
#define _PUNCH_SHADER_H_

enum SHD_LIMITS {
    MAX_STAGES      = 5,   // max stages number
    MAX_ANIM_FRAMES = 8,   // max animmaps textures
    MAX_TEXTUREPATH = 80,
    MAX_TCMOD_FUNCS = 5
};

enum SHD_SURFACE_ATTRS{
    SHD_CULL_BACK     = 1 << 0,  // cull back
    SHD_CULL_FRONT    = 1 << 1,  // cull front

    SHD_SURFACEPAM    = 1 << 2,  // if set, check surfaceparam

    SHD_DEFORMVERTEXES= 1 << 3,  // deformVertexes 
    SHD_POLYGONOFFSET = 1 << 4,  // polygonOffset
};

//marks attribute
enum SHD_MARKS_ATTR{
    MRK_NONE,
    MRK_BLACK,
    MRK_WHITE,
    MRK_HOT
};

//impact attribute
enum SHD_IMPACT_ATTR{
    IMP_NONE,
    IMP_BLOOD,
    IMP_DUST,
    IMP_SPARKLES,
};

enum SHD_STEPS_ATTR{
    STP_SILENT,
    STP_METAL,
    STP_GROUND,
    STP_WATER,
    STP_ASPHALT,
    STP_PLASTIC
};

// surfaceparam attributes
enum SHD_SURF_ATTRS{
    SHD_ALPHASHADOW=   1 << 0,
    SHD_FLAMMABLE  =   1 << 1,
    SHD_FLESH      =   1 << 2,
    SHD_FOG        =   1 << 3,
    SHD_NODLIGHT   =   1 << 4,
    SHD_NODRAW     =   1 << 5,
    SHD_NOLIGHTMAP =   1 << 6,
    SHD_NONSOLID   =   1 << 7,
    SHD_ORIGIN     =   1 << 8,
    SHD_PLAYERCLIP =   1 << 9,
    SHD_TRANS      =   1 << 10,
};

//deformVertexes attributes
enum SHD_DEFVERTS_ATTRS {
    VERTS_AUTOSPRITE  = 1 << 0,
    VERTS_AUTOSPRITE2 = 1 << 1
};

enum SHD_STAGE_FLAGS {
    STG_BLEND_FUNC = 1 << 0,     

    STG_TCGEN_FUNC = 1 << 1,    //tcGen declaration exists 
    STG_TCMOD_FUNC = 1 << 2,
    
    STG_DEPTHWRITE = 1 << 3,
    STG_RGBGEN_FUNC   = 1 << 4,
    STG_ALPHAGEN_FUNC = 1 << 5,

    STG_NOMIPMAP  = 1 << 6,
    STG_NOPICMIP  = 1 << 7
};

enum STG_MAP_TYPE {
    STG_NULL_MAP,
    STG_MAP,
    STG_ANIMMAP,
    STG_LIGHTMAP,
    STG_CLAMPMAP
};

//wave types for:
//      tcMod    stretch
//      rgbGen   wave
//      alphaGen wave      
//
enum WAVE_FORM{
    WF_NONE,
    WF_SIN,
    WF_TRIANGLE,
    WF_SQUARE,
    WF_SAWTOOTH,
    WF_INVSAWTOOTH
};

enum DEPTH_FUNC{
    DF_LEQUAL,
    DF_EQUAL,
    DF_ALWAYS
};

enum BLEND_TYPE {
    BLEND_NONE,
    BLEND_ONE,
    BLEND_ZERO,
    BLEND_DST_COLOR,
    BLEND_SRC_COLOR,
    BLEND_SRC_ALPHA,
    BLEND_INV_SRC_ALPHA,
    BLEND_INV_DST_COLOR,
    BLEND_INV_SRC_COLOR
};

typedef struct{
    BLEND_TYPE  src;
    BLEND_TYPE  dst;
} blend_func;

enum TCGEN_TYPE{
    TCGEN_NONE,
    TCGEN_ENVIROMENT,
    TCGEN_LIGHTSPOT,
    TCGEN_REFLECTION,  // tcGen reflection
    TCGEN_SPHERE       // tcGen sphere 
};

enum TCMOD_TYPE{
    TCMOD_NONE,         //no any tcMod declarations
    TCMOD_ROTATE,
    TCMOD_SCALE,
    TCMOD_SCROLL,
    TCMOD_STRETCH,
    TCMOD_TURB,
    TCMOD_EMBOSS
};
 
typedef struct{
    TCMOD_TYPE type;
    WAVE_FORM  wave_form; 
    float      arg[4];
} tcMod_func;

enum FILLMODE_TYPE{
    FM_POINT, 
    FM_WIRE,
    FM_SOLID
};

enum ALPHAFUNC_TYPE{
    AF_NONE,
    AF_GT0,
    AF_GE128,
    AF_LT128
};

enum COLOR_GEN_TYPE{
    CG_NONE,
    CG_IDENTITY,
    CG_WAVE,
    CG_LIGHTINGDIFFUSE,
    CG_VERTEX
};

//for grbGen, alphagen
typedef struct{
    COLOR_GEN_TYPE  type;
    WAVE_FORM       wave_form; 
    float           arg[4];      
} ColorGen_func;

//===============================================================
//
// shader stage definition
//
//===============================================================
struct stage_struct{
    unsigned short  flags;

    float           anim_freq;     // AnimMap freq
    int             num_textures;  // number of textures in texture array
    
    int             zBias;

    FILLMODE_TYPE   fm_type;
    ALPHAFUNC_TYPE  af_type;

    STG_MAP_TYPE    map_type;  
    char            texture[MAX_ANIM_FRAMES][MAX_TEXTUREPATH];
    
    DEPTH_FUNC      depth_func;
    TCGEN_TYPE      tcGen;    
    blend_func      fnBlend;         
    ColorGen_func   rgbGen;
    ColorGen_func   alphaGen;

    int         num_tcMods;         // number of tcMods in tcMod array
    tcMod_func  tcMod[MAX_TCMOD_FUNCS];

    //reset stage
    void init()
    {
        depth_func = DF_LEQUAL;
        flags = 0;
        map_type = STG_NULL_MAP;
        num_tcMods = 0;
        num_textures = 0;
        af_type = AF_NONE;

        //!!! if this is changed, it must be changed in functions:  
        //       ShdRecorder::SetZBias(int val)
        //       ShdGenerator::void PrintZBias()
        //
        zBias = 0;

        //!!! if this is changed, it must be changed in functions:  
        //       ShdRecorder::SetFillMode(int type)
        //       ShdGenerator::PrintFillMode(int type)
        //
        fm_type = FM_SOLID; 

        tcGen = TCGEN_NONE;    

        fnBlend.src = BLEND_NONE;
        fnBlend.dst = BLEND_NONE;

        rgbGen.type = CG_NONE;
        alphaGen.type = CG_NONE;
    }
};

//===============================================================
//
// shader struct definition
//
//===============================================================
struct shader_struct{
    unsigned short  flags;          // surface attributes flags 
                           
    unsigned short  deformVertexes; // deformVertexes attributes
    unsigned short  surfaceParam;   // surfaceParam args

    SHD_MARKS_ATTR  marks;          // marks attribute    
    SHD_IMPACT_ATTR impact;         // impact attribute
    SHD_STEPS_ATTR  steps;

    //amour value
    float  armour;

    int             num_stages;     // number of stages
    stage_struct    stage[MAX_STAGES];

    //set defaults
    void init()
    {
        marks = MRK_NONE;
        impact = IMP_NONE;
        steps = STP_SILENT;

        flags = 0;
        armour = 0;
        num_stages = 0;
        surfaceParam = 0;
        deformVertexes = 0;
    }
};

//===============================================================
//
// Shaders & Textures storage
//
//===============================================================

// compilation errors 
enum SC_ERRORS{
    SC_SUCCESS,             // no errors
    SC_TOKBUFFER_ERR,       // TokenBuffer erorr
    SC_TOKEN_ERR,           // Token erorr
    SC_PARSER_ERR,          // parser error (syn(..) or panic(..) call) 
    SC_LEXER_ERR,           // unknown token found by lexer
	SC_FILE_OPEN_ERR,       // file open error
    SC_NAME_REPLICATION,    // shader name replication
    SC_SECOND_CULL,         // two cull in shader script exists
    SC_SECOND_ZBIAS,        // second zBias definition in stage exists
    SC_SECOND_ALPHAFUNC,    // two aphaFunc defs in stage
    SC_SECOND_FILLMODE,     // two fillMode defs in stage found
    SC_SECOND_RGBGEN,
    SC_SECOND_ALPHAGEN,
    SC_STAGE_NUM_ERR,       // too many stage sections in a script
    SC_NO_MAP_ERR,
    SC_SECOND_MAP_ERR,      // stage already has map (animmap) declaration
    SC_ANIM_FRAMES_ERR,     // too many anim frames in shader
    SC_SECOND_BLEND_ERR,    // blendfunc already exists in stage description
    SC_SECOND_TCGEN_ERR,    // second tcGen function definition
    SC_TCMODS_FUNCS_ERR,    // too many tcMods lines in script
    SC_SECOND_NOMIPMAP_ERR, // second nomipmap keyword in stage exists
    SC_SECOND_NOPICMIP_ERR, // second nopicmip keyword in stage exists
    SC_SECOND_STEPS_ERR,    // second steps definition
    SC_SECOND_IMPACT_ERR,   // second impact defenition exist
    SC_SECOND_MARKS_ERR,    // second marks defenition exist
    SC_SECOND_DEPTHFUNC_ERR,// second depthfunc exists 
    SC_UNKNOWN_ERR,         // unknown error
};

class ShaderOutput{
public:
    
    //IsExist - check shader name
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // input:
    //    shader_name - new shader name
    //
    // returns:
    //    true  - shader already exist
    //    false - no shader with same name
    //
    virtual bool IsExist(const char* shd_name) = 0;
    
    //OnShader - shader was succesfully read from file
    //
    virtual void OnShader(const char* shd_name, const shader_struct& shd) = 0;

    //OnError - error handler 
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //returns: 
    //     true  - continue file processing
    //     false - stop file compilation
    //
    virtual bool OnError(const char* file_name, int line, SC_ERRORS type) = 0;

    virtual ~ShaderOutput(){}

protected:

    static const char* error_desc[];
};

#endif  // _PUNCH_SHADER_H_