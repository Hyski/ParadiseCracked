#ifndef _VirtualityPipeLine
#define _VirtualityPipeLine
/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ���� ������ ��������� ����������� pipeline
              �.�. ���� ����������� ����������� �������, ������� 
              ���������� �������, � ������� ��������� ��������� �����
              � ���� ��������. ����� �� ���������� ����������
              �� ��������.
     Author: Grom 
   Creation: 12 ������ 2000
***************************************************************/                
//#include <d3dtypes.h>

#include "VShader.h"
//#include "PrimitiveContaner.h"
#include "..\Shader\ShdCompiler.h"
#include "..\..\globals.h"
#include "DynamicVB.h"

#define PipeLog //logFile["graphpipe.log"]
class TexObject;
class GraphPipe:
                   public ShaderOutput    //��� �������� ��������
  {
  protected:
    
    typedef std::map<std::string,VShader> ShaderMap;
    typedef std::set<std::string> StringSet;
  protected:
    std::string ShaderFile;//��� �����, �� �������� � ������ ������ ����������� �������
    StringSet ShaderFiles;//������ ������ �� ������� ������� �������
    ShaderMap Shaders; //����� �������� �� ������
		bool CacheTextures;
  private:
    IDirect3D7 *D3D;
    IDirect3DDevice7 *D3DDev;

    float ResX,ResY;       //������� ����������
    Camera Cam;            //������
    
  public:
    enum RENDER_MODE{RM_2D=0x01,RM_3D=0x02,RM_NONE=0x04};
    RENDER_MODE LastSpace;//��� ���������� ��������� 2D/3D
    DynamicVB *DynBuf;
    
  private:
    //�������� ������ ������
    void CreateVBuffer();


  public:
    GraphPipe();
    ~GraphPipe();
    //������������� ������
    void Init(IDirect3D7 *_D3D, IDirect3DDevice7 *_D3DDev, float X, float Y);
    void Release();
    //�������� ��������
    void LoadShaders(std::string FileName);
    //��������� ��� �������, ����������� �� ���������� �����
    void UnloadShaderFile(const std::string &FileName);
    //��������� ��� ������, � ��������� ������
    void UnloadShader(const std::string &ShaderName);
    //� ���������� ��� ����������������� �� ����� ����� ������ �������� �������
    void RegisterShaderFile(const std::string &FileName, bool CacheTextures=true);
    //
		void SwitchRenderMode(RENDER_MODE Mode);
    void StartFrame();
    void EndFrame();
    //�������� ���������� � pipeline
    void Chop(const std::string &Shader, const Primi *Prim);
//    void Chop(PrimitiveContaner *Cont);
    void Chop(const TexObject *Obj);
    enum SKIP_FLAGS{SF_TRANSPARENT=0x01, SF_SOLID=0x20};
    void Chop(TexObject *Obj,const std::vector<const D3DMATRIX*> &Worlds, DynamicVB *Buf,int StartVert, unsigned skip_flags=0);
    void PushBspTrianglesDyn(VShader *sh,int ShdNum, DynamicVB *DynBuf);
    //��������������� ����� 3D ����� � �������� ����������
	  int TransformPoints(int Size ,const point3 *Input, point3 *Out);
  public:
    void GetCamPos(point3 *Pos, point3 *Front,point3 *Up){*Pos=Cam.GetPos();*Front=Cam.GetFront();*Up=Cam.GetUp();}
    //�������� ������
    Camera*  GetCam(){return &Cam;};
    //�������� ����������
    void GetResolution(int *resx, int *resy){*resx=ResX;*resy=ResY;};
    //�������� ���������� � ������ � �������� ������
    void GetProjOptions(float *_Far, float *_Near, float *_Fov){Cam.GetProjOptions(_Far,_Near,_Fov);};
    //���������� ������ � ������
    void BindToGrid(const GeneralGrid *G,const point3 &p){Cam.LinkToGrid(G,p);};
    void ApplyCamera(){Cam.Apply(D3DDev);};
    VShader *FindShader(const std::string &Name);

    //---------------------------------------------------
  public://��� �������� ��������
	  void AdjustHW();
	  void DrawBBox(const BBox &Box,unsigned long Col);
    virtual bool GraphPipe::IsExist(const char* shd_name);
    virtual void GraphPipe::OnShader(const char* shd_name, const shader_struct& shd);
    virtual bool GraphPipe::OnError(const char* file_name, int line, SC_ERRORS type);
    //---------------------------------------------------
  };

inline VShader *GraphPipe::FindShader(const std::string &Name)
  {
  ShaderMap::iterator s=Shaders.find(Name),e=Shaders.end();
  if(s==e)  
    {
    s=Shaders.find("noshader");
    if(s==e) return NULL;
    }
  return &s->second;
  }

#endif