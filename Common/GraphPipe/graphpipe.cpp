/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Этот объект реализует графический pipeline
              т.е. сюда отгружаются графические объекты, которые 
              отсекаются камерой, у которых удаляются невидимые линии
              и тому подобное. Здесь же происходит сортировка
              по шейдерам.
     Author: Grom 
   Creation: 12 апреля 2000
***************************************************************/                
#include "precomp.h"
#include "simpletexturedobject.h"
#include "../utils/profiler.h"

#include "graphpipe.h"
    D3DMATRIX supermat;
enum {DYNBUFSIZE=4096*4/*65000*/,PARTBUFSIZE=10000}; //размер буфера вершин под шейдер

GraphPipe::GraphPipe()
  {
	CacheTextures=true;
  Cam.SetProjection(TORAD(60),1,420,768.f/1024.f);
  Cam.SetLocation(point3(10,10,10),point3(1,1,-1));
  DynBuf=NULL;
  }

GraphPipe::~GraphPipe()
  {
  Release();
  }

//создание буфера вершин  
void GraphPipe::CreateVBuffer()
  {
   unsigned FVFflags=D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX2|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1);
  //unsigned FVFflags=D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_DIFFUSE ;
	 if(DynBuf) delete DynBuf;
   DynBuf=new DynamicVB( D3D,FVFflags,DYNBUFSIZE,sizeof(float)*3*2+4*sizeof(float)+sizeof(int));

  }
//инициализация данных
void GraphPipe::Init(IDirect3D7 *_D3D, IDirect3DDevice7 *_D3DDev, float X, float Y)
  {  
			StatesManager::Clear();
      D3DUtil_SetTranslateMatrix(supermat,-1,1,0);
    supermat._11=2.f/D3DKernel::ResX();
    supermat._22=-2.f/D3DKernel::ResY();

  D3D=_D3D;D3DDev=_D3DDev;ResX=X;ResY=Y;
  //не забыть обновить шейдеры      ]
  CreateVBuffer();
  PipeLog("GraphPipe Init\n");
  StringSet::iterator ss=ShaderFiles.begin(),sse=ShaderFiles.end();
  while(ss!=sse)
    {
		LoadShaders(*ss);
    ss++;
    }
  LoadShaders("shaders.sys/error.shader");

  Cam.SetProjection(TORAD(60),1,420,768.f/1024.f);

  Cam.Move(point3(0,0,0));
  }
void GraphPipe::SwitchRenderMode(RENDER_MODE Mode)
	{
	switch(Mode)
		{
		case RM_2D:
    if(LastSpace==RM_3D||LastSpace==RM_NONE)
      {
			//logFile["testing.log"]("switched to 2D mode\n");
			StatesManager::SetTransform( D3DTRANSFORMSTATE_VIEW, (D3DMATRIX*)&supermat);
			StatesManager::SetTransform( D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX*)IDENTITYMAT);
			StatesManager::SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX*)IDENTITYMAT);
      LastSpace=RM_2D;
      }
		break;
		case RM_3D:
    if(LastSpace==RM_2D||LastSpace==RM_NONE)//FIXME
      {
			//logFile["testing.log"]("switched to 3D mode\n");
      StatesManager::SetTransform( D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)Cam.GetProj());
      StatesManager::SetTransform( D3DTRANSFORMSTATE_VIEW, (D3DMATRIX*)Cam.GetView());
      LastSpace=RM_3D;
      }
		break;
		case RM_NONE:
			//logFile["testing.log"]("switched to initial mode\n");
      LastSpace=RM_NONE;
			break;
		}
	}

//загрузка примитивов в pipeline
void GraphPipe::Chop(const std::string &Shader, const Primi *Prim)
  {
  if(!Prim->IdxNum&&!Prim->VertNum) return;
  VShader *vs=FindShader(Shader);
  if(!vs) //не смогли найти даже  ошибочный шейдер
    {
    std::string _e=Shader;
    _e=std::string(">>>'")+_e+"'<<<\n мало того, что не найден шейдер\n"
      "нет даже стандартного шейдера 'noshader'\n\n";
    throw CASUS(_e.c_str());
    }

  if(Prim->Contents&Primi::NEEDTRANSFORM)
    {//просто скопировать вершины в буффер
		SwitchRenderMode(RM_3D);
    }
  else
    {//записать в буффер вершины после трансформации
		SwitchRenderMode(RM_2D);
    }
	
		//logFile["testing.log"]("chopping shader '%s' in %s mode\n",Shader.c_str(),LastSpace==RM_2D?"2d":(LastSpace==RM_3D?"3d":"error"));
    vs->Chop(Prim,D3DDev,DynBuf);

  }

void GraphPipe::LoadShaders(std::string FileName)
{
ShaderFile=FileName;
 PipeLog("Загружаем шейдер:%s\n",FileName.c_str());
 ShdCompiler Loader(this);
 Loader.Compile(FileName.c_str());

}

void GraphPipe::Release()
  {
  DESTROY(DynBuf);
  ShaderMap::iterator i=Shaders.begin();
  while(i!=Shaders.end())
    {
    i->second.Release(D3DDev);
    i++;
    }
  Shaders.clear();
  PipeLog("GraphPipe Release\n");
  }
/*
int GraphPipe::TransformPoints(int Size, const point3 *Input, point3 *Out)
  {
  if(!Size) return 0;
  static D3DDRAWPRIMITIVESTRIDEDDATA PSD;
  LastSpace=LAST3D;
  PSD.position.lpvData=(void*)Input;     PSD.position.dwStride=sizeof(point3);
  PSD.normal.lpvData=NULL;      PSD.normal.dwStride=sizeof(point3);
  PSD.diffuse.lpvData=NULL;  PSD.diffuse.dwStride=sizeof(unsigned int);
  PSD.textureCoords[0].lpvData=NULL;  PSD.textureCoords[0].dwStride=sizeof(texcoord);

  D3DDev->SetTransform( D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)&Cam.ProjUploaded);
  D3DDev->SetTransform( D3DTRANSFORMSTATE_VIEW, (D3DMATRIX*)Cam.GetView());


  //проецирование координат
  HRESULT h=ParticleBuf->ProcessVerticesStrided(D3DVOP_TRANSFORM,
                                    0,Size,&PSD,0,D3DDev,0);
  if(FAILED(h))
    {
    return 0;
    }
  //возвращаем спроецированные координаты в массив
  float *bufpos;
  point3 *out=Out;
  int pntnum=0;
  if(SUCCEEDED(ParticleBuf->Lock(DDLOCK_WAIT|DDLOCK_READONLY,(void**)&bufpos,NULL)))
    {
    while(Size--)
      {
      out->x=*bufpos++;
      out->y=*bufpos++;
      out->z=*bufpos++;
      bufpos++;
      if(out->x<0||out->y<0||out->x>=ResX||out->y>=ResY||
      out->z<0||out->z>1)
        {
        out->z=-1;
        //continue;
        }
      out++;
      pntnum++;
      }
    ParticleBuf->Unlock();
    }
  return pntnum;
  }                                                        */
int GraphPipe::TransformPoints(int Size, const point3 *Input, point3 *Out)
  {
  //FIXME: переделать цикл умножения (матрицу необходимо сделать сразу со всеми преобразованиями)
  if(!Size) return 0;
  D3DMATRIX m,m2;
  D3DMath_MatrixMultiply(m,*(D3DMATRIX*)Cam.GetView(),*(D3DMATRIX*)Cam.GetProj());
  
  point3 *out=Out;
  const point3 *in=Input;
  /* by Flif float f,n,fov;*/
  int size=Size;
  while(size--)
    {
    PointMatrixMultiply(TODXVECTOR(*out),
      TODXVECTOR(*in),m);
    out->x+=1;
    out->y=1-out->y;
    out->x*=D3DKernel::ResX()/2;
    out->y*=D3DKernel::ResY()/2;
    if(out->x<0||out->y<0||out->x>=ResX||out->y>=ResY||
      out->z<0||out->z>1)
      {
      out->z=-1;
      //continue;
      }
    out++;
    in++;
    }
  return Size;
  }
void GraphPipe::Chop(const TexObject *Obj)
  {
  Primi  Prim;
  int ptnum;
  for(ptnum=0;ptnum<Obj->PartNum;ptnum++)
    {
    SimpleTexturedObject *s=Obj->Parts[ptnum];
    Prim.Pos=s->Points;
    Prim.Norm=s->Normals;
    Prim.UVs[0]=s->uv;
    Prim.IdxNum=s->IdxNum;
    Prim.Idxs=(unsigned short*)s->GetIndexesFull();
    Prim.Prim=Primi::TRIANGLE;
    Prim.Contents=Primi::NEEDTRANSFORM;
    Prim.VertNum=s->PntNum;
    if(!Prim.VertNum&&!Prim.IdxNum)continue;
    if(Prim.VertNum)
      Chop(s->MaterialName,&Prim);
    }
  
  }
void GraphPipe::Chop(TexObject *Obj,const std::vector<const D3DMATRIX*> &Worlds,
										 DynamicVB *Buf,int StartVert, unsigned skip_flags)
  {
  Primi  Prim;
  int ptnum;
  for(ptnum=0;ptnum<Obj->PartNum;ptnum++)
    {
    SimpleTexturedObject *s=Obj->Parts[ptnum];
    if(!s->PntNum&&!s->IdxNum)continue;
		VShader *vs=FindShader(s->MaterialName);
		if(!vs) //не смогли найти даже  ошибочный шейдер
			{
			std::string _e=s->MaterialName;
			_e=std::string(">>>'")+_e+"'<<<\n мало того, что не найден шейдер\n"
				"нет даже стандартного шейдера 'noshader'\n\n";
			throw CASUS(_e.c_str());
			}

		if(vs->Transparent)
      {      if(skip_flags&SF_TRANSPARENT) {StartVert+=s->PntNum;continue; } }
    else
      {      if(skip_flags&SF_SOLID) {StartVert+=s->PntNum;continue; }   }

		SwitchRenderMode(RM_3D);

    vs->Chop(D3DPT_TRIANGLELIST,Buf,StartVert,s->PntNum,(unsigned short*)s->GetIndexesFull(),
			s->IdxNum,Worlds);
		StartVert+=s->PntNum;
		}
  
  }

void GraphPipe::DrawBBox(const BBox &Box,unsigned long Col)
  {
	SwitchRenderMode(RM_3D);
  D3DLVERTEX corners[8]={
    D3DLVERTEX(D3DVECTOR(Box.minx,Box.miny,Box.minz),Col,0,0,0),
    D3DLVERTEX(D3DVECTOR(Box.maxx,Box.miny,Box.minz),Col,0,0,0),
    D3DLVERTEX(D3DVECTOR(Box.maxx,Box.maxy,Box.minz),Col,0,0,0),
    D3DLVERTEX(D3DVECTOR(Box.minx,Box.maxy,Box.minz),Col,0,0,0),
    D3DLVERTEX(D3DVECTOR(Box.minx,Box.miny,Box.maxz),Col,0,0,0),
    D3DLVERTEX(D3DVECTOR(Box.maxx,Box.miny,Box.maxz),Col,0,0,0),
    D3DLVERTEX(D3DVECTOR(Box.maxx,Box.maxy,Box.maxz),Col,0,0,0),
    D3DLVERTEX(D3DVECTOR(Box.minx,Box.maxy,Box.maxz),Col,0,0,0),
    };
  unsigned short idxs[12*2]={
    0,1, 1,2, 2,3, 3,0,
    4,5, 5,6, 6,7, 7,4,
    0,4, 1,5, 2,6, 3,7,
    };
  //настройка прозрачности
  StatesManager::SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);
  StatesManager::SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE,FALSE);
  //работа с Z-буфером
  StatesManager::SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,TRUE);
  StatesManager::SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
  StatesManager::SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
  StatesManager::SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  StatesManager::SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  StatesManager::SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE );

  D3DDev->SetTexture(0,0);
  D3DDev->DrawIndexedPrimitive(D3DPT_LINELIST,D3DFVF_LVERTEX,corners,8,idxs,12*2,0);
  }


void GraphPipe::UnloadShader(const std::string &ShaderName)
  {
  ShaderMap::iterator s=Shaders.find(ShaderName),e=Shaders.end();
  if(s!=e)  
    {
      s->second.Release(D3DDev);
      Shaders.erase(s);
    }
  }
void GraphPipe::UnloadShaderFile(const std::string &FileName)
  {
  PipeLog("выгружаем файл с шейдерами: '%s' \n",FileName.c_str());
  ShaderMap::iterator s=Shaders.begin(),e=Shaders.end(),s1;
  while(s!=e)  
    {
    if(s->second.FileName==FileName)
      {
      PipeLog("выгружаем шейдер: '%s' \n",s->second.ShaderName.c_str());
      s1=s++;
      s1->second.Release(D3DDev);
      Shaders.erase(s1);
      }
    else
      s++;
    }
  ShaderFiles.erase(FileName);
  }
void GraphPipe::RegisterShaderFile(const std::string &FileName, bool cachetextures)
  {
	CacheTextures=cachetextures;
  ShaderFiles.insert(FileName);
  LoadShaders(FileName);
  }

void GraphPipe::AdjustHW()
  {
  //PipeLog("ahw in\n");
  StatesManager::Clear();
  //Release();
  //Init(D3DKernel::GetD3D(),D3DKernel::GetD3DDevice(),D3DKernel::ResX(),D3DKernel::ResY());
  D3DKernel::RestoreSurfaces();

  ShaderMap::iterator s=Shaders.begin(),e=Shaders.end();
  while(s!=e)  
  {
  s->second.AdjustHW(D3DDev);
  s++;
  }
	StatesManager::Clear();
  //PipeLog("ahw out\n");
  
  }
bool GraphPipe::IsExist(const char* shd_name)
  {
  bool isexist=Shaders.find(shd_name)!=Shaders.end();
  if (isexist) PipeLog("Уже загружен: '%s' \n",shd_name);
  return isexist;
  }
void GraphPipe::OnShader(const char* shd_name, const shader_struct& shd)
  {
  Shaders[shd_name]=VShader(shd);
  Shaders[shd_name].ShaderName=shd_name;
	if(CacheTextures)
		Shaders[shd_name].AdjustHW(D3DDev);
  Shaders[shd_name].FileName=ShaderFile;
  PipeLog("Успешно загружен шейдер '%s'\n",shd_name);
  }
bool GraphPipe::OnError(const char* file_name, int line, SC_ERRORS type)
  {
  PipeLog("Ошибка в шейдере '%s' строка:%d, тип:%s\n",
    file_name,line,ShaderOutput::error_desc[(int)type]);
  return true;
  }
void GraphPipe::StartFrame()
  {
  DynBuf->FlushAtFrameStart();
  SwitchRenderMode(RM_NONE);
	CodeProfiler::FrameStart();
  }
void GraphPipe::EndFrame()
  {
	ApplyCamera();
  }

void GraphPipe::PushBspTrianglesDyn(VShader *sh,int ShdNum, DynamicVB *DynBuf)
  {
	SwitchRenderMode(RM_3D);

	extern  bool _OldRender;
  if(_OldRender)
  sh->PushBspTrianglesDynOld(ShdNum,this->DynBuf);
	else
  sh->PushBspTrianglesDyn(ShdNum,DynBuf);
  }
