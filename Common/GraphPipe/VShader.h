#if !defined(AFX_VSHADER_H__1F984AC1_EAB9_11D3_81EA_0060520A50A9__INCLUDED_)
#define AFX_VSHADER_H__1F984AC1_EAB9_11D3_81EA_0060520A50A9__INCLUDED_
/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ���������� ������� ��� ������������� � ��������
                ��������� �� ��������. �������� ������� ��� ��������                                                               
                
                ������ ��������� ��������������������� � �����
                �������, ��� ��������, ��������������� �������
                � �.�.
                                                                                
     Author: Grom 
   Creation: 12 ������ 2000
***************************************************************/                

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DynamicVB.h"
#include "camera.h"
#include "../shader/shader.h"

class StatesManager
	{

	public:
		static void Clear()
			{
			m_RenderStates.clear();
			m_Transforms.clear();
			m_TexStages0.clear();
			m_TexStages1.clear();
			D3DDev=D3DKernel::GetD3DDevice();
			};
		static HRESULT SetRenderState(D3DRENDERSTATETYPE RenderStateType,unsigned RenderState);
		static HRESULT SetTransform(D3DTRANSFORMSTATETYPE TransformStateType,D3DMATRIX *Matrix);
    static HRESULT SetTextureStageState(unsigned Stage, D3DTEXTURESTAGESTATETYPE State, unsigned Value);
		static HRESULT SetTexture(unsigned Stage,LPDIRECTDRAWSURFACE7 Texture);
	public:
  	typedef std::map<D3DRENDERSTATETYPE,unsigned> render_types_t;
		typedef std::map<D3DTRANSFORMSTATETYPE ,D3DMATRIX *> transforms_t;
		typedef std::map<D3DTEXTURESTAGESTATETYPE ,unsigned> tex_stages_t;

	private:
		static render_types_t m_RenderStates;
		static transforms_t   m_Transforms;
		static tex_stages_t   m_TexStages0;
		static tex_stages_t   m_TexStages1;
    static IDirect3DDevice7 *D3DDev;
	};


class LMTexHolder
{
private:
	std::vector<IDirectDrawSurface7*> LMs;
    IDirectDrawSurface7* Substitute;
	bool adjusted;
public:
	IDirectDrawSurface7* GetLM(int Num)
	{
		if(!adjusted) Adjust();
		return Num>=LMs.size() ? NULL : LMs[Num]; 
	};
	IDirectDrawSurface7* GetSubstitute(){return Substitute; };
	void Adjust();
	void Clear();
	~LMTexHolder() {Clear();};
	LMTexHolder():Substitute(NULL),adjusted(false) {};
};

class VShader  
  {
//========= ���������� ����� � ��� � ��� ������� ==================
  protected:
    //Primi InHouseBuf;  //��� �����
    //int CurIndex;      //������ �� �������� ����� �������� 
  public:
    std::string FileName,ShaderName;//��� ����� �� �������� �������� � ��� �������
    //���������� ������ ��� ������ �� ������ VertNum ������ 
//    Primi PrepareFill(int VertNum);
    //��������� ������� ������ ������������� ���� �������� � ������
   // void FilledSize(int VertNum) { CurIndex+=VertNum;  }
//======================================================
  public://��� ������ ����������
    void Chop( const Primi *Prim,IDirect3DDevice7 *D3DDev_,
                   DynamicVB *DynBuf);
void VShader::Chop(D3DPRIMITIVETYPE PrimType,DynamicVB *DynBuf,
									  unsigned short StartVert,int vertnum,unsigned short *idxs,int idxnum,
										const std::vector<const D3DMATRIX*> &Worlds);


  protected: //������ ������� ������������ ��� ������ ���������
    static float GetWave(WAVE_FORM WType, float *args,float Time);
    static DWORD GetGenCol(ColorGen_func &RGBGen,ColorGen_func &AlphaGen,float Time);
    bool NoDataChanges(int stagenum);
    void TuneRGBALPHAgen(int stage,int texstage);
		void LinkTextures();
  protected:
	static LMTexHolder CachedLM; //������������ �������� ���� ������������
    int StageNum; //���������� ���������� �������
    SHD_DEFVERTS_ATTRS VertDeform;
    D3DCULL CullMode;

    D3DCMPFUNC ZFunc[MAX_STAGES]; //������� ��������� ��� z-������
    bool NoMipMap[MAX_STAGES];
    bool NoPicMip[MAX_STAGES];
    bool ZWrite[MAX_STAGES];  //����� �� ������ � ������
    int ZBias[MAX_STAGES];// �������� � z-������

    bool AlphaBlend[MAX_STAGES]; //���������� �������� ������ ��� ������������
    D3DBLEND SrcBlend[MAX_STAGES];
    D3DBLEND DstBlend[MAX_STAGES];
    bool AlphaTest[MAX_STAGES];//���������� �������� ��������� �������� � ����������� �� �����-������
    DWORD AlphaRef[MAX_STAGES];//� ���� ��������� ����� ���� ���������
    D3DCMPFUNC AlphaFunc[MAX_STAGES];//������� �� ������� ���������� ���������
    
    D3DFILLMODE FillMode[MAX_STAGES];//debug:����� ��������� ��������������� (�����.�����.������������)

    int TexNum[MAX_STAGES];//���������� ������� � �������� � stage #...
    STG_MAP_TYPE MapType[MAX_STAGES];
    float AnimFreq[MAX_STAGES];
    std::string TexMap[MAX_STAGES][MAX_ANIM_FRAMES];
    D3DTEXTUREADDRESS MapCoords[MAX_STAGES];
    unsigned Wrapping[MAX_STAGES];
    //��������� � ����������� ���������� ���������
    TCGEN_TYPE TexGen[MAX_STAGES];
    int TModNum[MAX_STAGES];//���������� �����������
    tcMod_func TexMod[MAX_STAGES][MAX_TCMOD_FUNCS];
    ColorGen_func   RGBGen[MAX_STAGES];
    ColorGen_func   AlphaGen[MAX_STAGES];

  protected://��� ������������ VB
    static IDirect3DDevice7 *D3DDev;
    IDirectDrawSurface7 *DxTex[MAX_STAGES][MAX_ANIM_FRAMES];
    bool SetBlending(int i);
	  int ApplyStates(IDirect3DDevice7 *D3DDev,int stagenum);
    int FlushDynBuffer(D3DPRIMITIVETYPE PrimType,unsigned short *idx,int idxnum,int StartVertex,int VertNum,int i,int lmNum, DynamicVB *DynBuf);
    int FlushDynBuffer(D3DPRIMITIVETYPE PrimType,unsigned short *idx,int idxnum,int StartVertex,int VertNum,int i,DynamicVB *DynBuf, const std::vector<const D3DMATRIX*> &Worlds);
    int LMNum;//����� ����� ���������
		bool TexturesLinked;//��������� �� �������� ������ � ����������� �� ��������
  public:
    bool Transparent;
  public:
	  void CreateTexModMatrix(float Time, int stagenum, D3DMATRIX *mat);
    void SetTexModMatrix(int stagenum, int texstage=0);
    void SetTexGenMatrix(int stagenum, int texstage=0);

    void PushBspTrianglesDyn(int ShdNum, DynamicVB *DynBuf);
    void PushBspTrianglesDynOld(int ShdNum, DynamicVB *DynBuf);

    VShader(const shader_struct &shader);
    IDirectDrawSurface7* GetTexture(const char *Nam, int stage);
    void Release(IDirect3DDevice7 *D3DDev);
    void AdjustHW(IDirect3DDevice7 *D3DDev);

    VShader();
    virtual ~VShader();
    
  };

#endif // !defined(AFX_VSHADER_H__1F984AC1_EAB9_11D3_81EA_0060520A50A9__INCLUDED_)
