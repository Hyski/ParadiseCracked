// Level.cpp: implementation of the Level class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include "../common/graphpipe/culling.h"
#include "bsp.h"
//#include "relaxator.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
Plane    *Bsp::Planes=NULL;    //набор секущих планов
int       Bsp::PlanesNum=0;
unsigned *Bsp::PlaneFlags=NULL;//и их флаги

LevelVertex *Bsp::Verts=NULL;   //массив вершин
int          Bsp::VertNum=0; //количество вершин
short       *Bsp::VertIdx=NULL;  

unsigned *Bsp::Faces=NULL;   // набор индексов по 3 на грань
unsigned *Bsp::FaceShd=NULL; //номер шейдера для грани
unsigned *Bsp::PlaneRef=NULL; //номер шейдера для грани
int      *Bsp::IsRoof=NULL;   //номер шейдера для грани

int       Bsp::FaceNum;       //количество граней
std::vector<std::string> Bsp::ShaderNames;
unsigned     *Bsp::ShdFlags=NULL;

Bsp      *Bsp::Nodes=NULL; //узлы BSP дерева
int       Bsp::NodesNum;   //и их количество

unsigned short *Bsp::VisTris[MAXSHADERS]; //массивы номеров видимых граней по материалам
unsigned short *Bsp::IdxTris=NULL; //массивы индексов видимых граней по материалам
unsigned  Bsp::VisNum[MAXSHADERS]; //количество видимых треугольников для каждого материала


enum{PLANESNUM=20000,VERTSNUM=200000,FACESNUM=250000,NODESNUM=500000};

void Bsp::Init()
  {
  Planes=new Plane[PLANESNUM];
  PlanesNum=0;
  PlaneFlags=new unsigned[PLANESNUM];

  Verts=new LevelVertex[VERTSNUM];
  VertNum=0;

  Faces=new unsigned[FACESNUM*3];
  FaceShd = new unsigned[FACESNUM];
  IsRoof= new int[FACESNUM];
  PlaneRef = new unsigned[FACESNUM];
  FaceNum = 0;

  Nodes=new Bsp[NODESNUM];
  NodesNum=0;


  for(int i=0;i<MAXSHADERS;i++)
    {
    VisTris[i]=NULL;
    VisNum[i]=0;
    }

  }
void Bsp::Close()
  {
  FREE(Planes);
  DESTROY(PlaneFlags);
  FREE(Verts);
  DESTROY(Faces);
  DESTROY(FaceShd);
  DESTROY(VertIdx);
  DESTROY(IsRoof);
  DESTROY(PlaneRef);
  FREE(Nodes);
  NodesNum=0;
  PlanesNum=0;
  VertNum=0;
  FaceNum = 0;
  for(int i=0;i<MAXSHADERS;i++)
    {
    DESTROY(VisTris[i]);
    }
  DESTROY(IdxTris);
  DESTROY(ShdFlags);
  }
bool Bsp::MakeSaveLoad(SavSlot &slot)
  {
  if(slot.IsSaving())
    {
    int i;
    unsigned int Version=0x0200;
    slot<<Version;
    
    slot<<ShaderNames.size();
    for(i=0;i<ShaderNames.size();i++)
      slot<<ShaderNames[i];
    slot.Save(ShdFlags,sizeof(unsigned)*ShaderNames.size());

    slot<<VertNum;
    for(i=0;i<VertNum;i++)
      {
      slot<<Verts[i].Pos;
      slot<<Verts[i].uv.u<<Verts[i].uv.v;
      slot<<Verts[i].lightmapuv.u<<Verts[i].lightmapuv.v;
      slot<<Verts[i].Normal;
      slot.Save(&Verts[i].Color,sizeof(long));
      }
    
    slot<<FaceNum;
    for(i=0;i<FaceNum*3;i++)
      slot<<Faces[i];
    for(i=0;i<FaceNum;i++)
      slot<<IsRoof[i];
    for(i=0;i<FaceNum;i++)
      slot<<FaceShd[i];
    
    slot<<PlanesNum;
    for(i=0;i<PlanesNum;i++)
      slot.Save(Planes+i,sizeof(Plane));
    slot<<NodesNum;
    for(i=0;i<NodesNum;i++)
      {
      slot.Save(&Nodes[i],sizeof(Bsp));
      }
    }
  else
    {
    int i;
    Close();
    unsigned int Version=0x0200;
    slot>>Version;
    
    ShaderNames.clear();
    slot>>i;
    for(;i;i--)
      {
      std::string t;
      slot>>t;
      ShaderNames.push_back(t);
      }
    ShdFlags=new unsigned[ShaderNames.size()];
    slot.Load(ShdFlags,sizeof(unsigned)*ShaderNames.size());

    slot>>VertNum;
    Verts= new LevelVertex[VertNum];
		//int numbytes=VertNum*sizeof(LevelVertex);
    for(i=0;i<VertNum;i++)
      {
      slot>>Verts[i].Pos;
      slot>>Verts[i].uv.u>>Verts[i].uv.v;
      slot>>Verts[i].lightmapuv.u>>Verts[i].lightmapuv.v;
      slot>>Verts[i].Normal;
			Verts[i].Normal = Normalize(Verts[i].Normal);
      slot.Load(&Verts[i].Color,sizeof(long));
      }
    
    slot>>FaceNum;
    Faces=new unsigned[FaceNum*3];
    FaceShd = new unsigned[FaceNum];
    IsRoof= new int[FaceNum];
    PlaneRef = new unsigned[FaceNum];
    VertIdx=new short[VertNum];
    
    
    
    for(i=0;i<FaceNum*3;i++)
      slot>>Faces[i];
    for(i=0;i<FaceNum;i++)
      slot>>IsRoof[i];
    for(i=0;i<FaceNum;i++)
      slot>>FaceShd[i];
    
    memset(VisNum,0,sizeof(unsigned*)*MAXSHADERS);
    for(i=0;i<FaceNum;i++)
      VisNum[FaceShd[i]]++;
    int maxn=0;
    for(i=0;i<MAXSHADERS;i++)
      {
      if(VisNum[i])
        {
        VisTris[i]=new unsigned short[3*VisNum[i]];
        if(VisNum[i]>maxn) maxn=VisNum[i];
        
        }
      else 
        {
        VisTris[i]=NULL;
        IdxTris=NULL;
        }
      VisNum[i]=0;
      }
    IdxTris=new unsigned short[maxn*3];
    
    slot>>PlanesNum;
    Planes=new Plane[PlanesNum];
    PlaneFlags=new unsigned[PlanesNum];
    for(i=0;i<PlanesNum;i++)
      {
      slot.Load(Planes+i,sizeof(Plane));
      Planes[i].Update();
      }
    slot>>NodesNum;
    Nodes = new Bsp[NodesNum];
    for(i=0;i<NodesNum;i++)
      {
      slot.Load(&Nodes[i],sizeof(Bsp));
      }
    
    }
  return true;
  }

void Bsp::Load(FILE *f)
  {
  int i;
  Close();
  char ID[]="MLBSP";
  unsigned int Version=0x0200;
  fread(ID,5,1,f);
  fread(&Version,sizeof(Version),1,f);

  ShaderNames.clear();
  fread(&i,sizeof(i),1,f);
  for(;i;i--)
    {
    char t[50],n;
    fread(&n,sizeof(n),1,f);
    fread(t,n,1,f);
    ShaderNames.push_back(t);
    }
  ShdFlags=new unsigned[ShaderNames.size()];
  fread(ShdFlags,sizeof(unsigned),ShaderNames.size(),f);
  /* by Flif
  for(i=0;i<ShaderNames.size();i++)
    {
    unsigned f=ShdFlags[i];
    const char * t=ShaderNames[i].c_str();
    }
	*/
  fread(&VertNum,sizeof(VertNum),1,f);
  Verts= new LevelVertex[VertNum];
  for(i=0;i<VertNum;i++)
    {
    fread(&Verts[i].Pos,sizeof(Verts[i].Pos),1,f);
    fread(&Verts[i].uv,sizeof(Verts[i].uv),1,f);
    fread(&Verts[i].lightmapuv,sizeof(Verts[i].lightmapuv),1,f);
    fread(&Verts[i].Normal,sizeof(Verts[i].Normal),1,f);
    fread(&Verts[i].Color,sizeof(Verts[i].Color),1,f);
    }

  fread(&FaceNum,sizeof(FaceNum),1,f);
  Faces=new unsigned[FaceNum*3];
  FaceShd = new unsigned[FaceNum];
  IsRoof= new int[FaceNum];
  PlaneRef = new unsigned[FaceNum];
  VertIdx=new short[VertNum];



  for(i=0;i<FaceNum*3;i++)
    {
    fread(Faces+i,sizeof(unsigned),1,f);
    }
  for(i=0;i<FaceNum;i++)
    fread(IsRoof+i,sizeof(int),1,f);
  for(i=0;i<FaceNum;i++)
    fread(FaceShd+i,sizeof(unsigned),1,f);

  memset(VisNum,0,sizeof(unsigned*)*MAXSHADERS);
  for(i=0;i<FaceNum;i++)
     VisNum[FaceShd[i]]++;
  int maxn=0;
  for(i=0;i<MAXSHADERS;i++)
    {
    if(VisNum[i])
      {
      VisTris[i]=new unsigned short[3*VisNum[i]]; //FIXME: what for 3*?
      if(VisNum[i]>maxn) maxn=VisNum[i];

      }
    else 
      {
      VisTris[i]=NULL;
      IdxTris=NULL;
      }
    VisNum[i]=0;
    }
  IdxTris=new unsigned short[maxn*3];

  fread(&PlanesNum,sizeof(PlanesNum),1,f);
  Planes=new Plane[PlanesNum];
  PlaneFlags=new unsigned[PlanesNum];
  for(i=0;i<PlanesNum;i++)
    {
    fread(Planes+i,sizeof(Plane),1,f);
    Planes[i].Update();
    }
  fread(&NodesNum,sizeof(int),1,f);
  Nodes = new Bsp[NodesNum];
  for(i=0;i<NodesNum;i++)
    {
    fread(&Nodes[i],sizeof(Bsp),1,f);
    }
  }
void Bsp::Save(FILE *f)
  {
  /*
  [MLBSP]     - char[5]
  VERSION     - unsigned
  NUMSHADERS  - unsigned 
  SHADERNAMES - char[50]
  NumVerts    - unsigned
  Verts       - LevelVertex[NumVerts]
  NumFaces    - unsigned
  Faces       - unsigned[NumFaces*3]
  IsRoof      - bool[NumFaces]
  FaceShd     - unsigned[NumFaces]
  PlanesNum   - unsigned
  Planes      - Plane[PlanesNum]
  LMpresent   - bool[NumShaders]
  LightMaps   - Texture[NumLmapsPresent]
  */
  int i;
  char ID[]="MLBSP";
  unsigned int Version=0x0200;
  fwrite(ID,5,1,f);
  fwrite(&Version,sizeof(Version),1,f);

  i=ShaderNames.size();
  fwrite(&i,sizeof(i),1,f);
  for(i=0;i<ShaderNames.size();i++)
    {
    char t=1+strlen(ShaderNames[i].c_str());
    fwrite(&t,sizeof(t),1,f);
    fwrite(ShaderNames[i].c_str(),t,1,f);
    }
  fwrite(ShdFlags,sizeof(unsigned),ShaderNames.size(),f);
  fwrite(&VertNum,sizeof(VertNum),1,f);
  for(i=0;i<VertNum;i++)
    {
    fwrite(&Verts[i].Pos,sizeof(Verts[i].Pos),1,f);
    fwrite(&Verts[i].uv,sizeof(Verts[i].uv),1,f);
    fwrite(&Verts[i].lightmapuv,sizeof(Verts[i].lightmapuv),1,f);
    fwrite(&Verts[i].Normal,sizeof(Verts[i].Normal),1,f);
    fwrite(&Verts[i].Color,sizeof(Verts[i].Color),1,f);
    }

  fwrite(&FaceNum,sizeof(FaceNum),1,f);
  for(i=0;i<FaceNum*3;i++)
    fwrite(Faces+i,sizeof(unsigned),1,f);
  for(i=0;i<FaceNum;i++)
    fwrite(IsRoof+i,sizeof(int),1,f);
  for(i=0;i<FaceNum;i++)
    fwrite(FaceShd+i,sizeof(unsigned),1,f);

  fwrite(&PlanesNum,sizeof(PlanesNum),1,f);
  for(i=0;i<PlanesNum;i++)
    fwrite(Planes+i,sizeof(Plane),1,f);
  fwrite(&NodesNum,sizeof(int),1,f);
  for(i=0;i<NodesNum;i++)
    {
    fwrite(&Nodes[i],sizeof(Bsp),1,f);
    }
  
  }
void Bsp::LoadFromModel(FILE *f)
  {
    //char t[200];
  int BaseIndex=0;

  float Version;
  int PartsNum;

  fscanf(f,"Object version:%f\n",&Version);
  fscanf(f,"PartsNumber:%d\n",&PartsNum);
  ShaderNames.resize(PartsNum,"");
  ShdFlags=new unsigned[PartsNum];
  memset(ShdFlags,0,sizeof(unsigned)*PartsNum);
  bool *Roofs=new bool[VERTSNUM];
  for(int CurPart=0;CurPart<PartsNum;CurPart++)
    {
    fscanf(f,"Material:");
    LoadString(f,&ShaderNames[CurPart]);
    int VNum;
    fscanf(f,"Vertex number:%d\n",&VNum);
    BaseIndex=VertNum;
    for(int vn=0;vn<VNum;vn++)
      {
      LevelVertex v;
      fscanf(f,"Pos:%f %f %f\n",
        &v.Pos.x,&v.Pos.y,&v.Pos.z);
      fscanf(f,"Normal:%f %f %f\n",
        &v.Normal.x,&v.Normal.y,&v.Normal.z);
      v.Normal=Normalize(v.Normal);
      fscanf(f,"uv:%f %f\n",&v.uv.u,&v.uv.v);

      fscanf(f,"Roof:%d\n",&Roofs[VertNum]);
      //status("Loading Vertices %d of %d (Part %d of %d)",vn,VNum,CurPart,PartsNum);
      v.lightmapuv.u=-1;v.lightmapuv.v=-1;
      Verts[VertNum++]=v;
      //FindVertex(&v);
      //VertNum++;
      }
    int INum;
    fscanf(f,"Index number:%d\n",&INum);
    for(int id=0;id<INum/3;id++)
      {
      int id3=FaceNum*3;
      IsRoof[FaceNum]=false;
      for(int k=0;k<3;k++)
        {
        fscanf(f,"%d ",&Faces[id3+k]);
        Faces[id3+k]+=BaseIndex;
        if(Roofs[Faces[id3+k]]) IsRoof[FaceNum]=true;
        }
			/*	float l1,l2,l3;
			l1=(Verts[Faces[id3+0]].Pos-Verts[Faces[id3+1]].Pos).Length();
			l2=(Verts[Faces[id3+1]].Pos-Verts[Faces[id3+2]].Pos).Length();
			l3=(Verts[Faces[id3+0]].Pos-Verts[Faces[id3+2]].Pos).Length();
			if(l1<0.001||l2<0.001||l3<0.001) {id--;continue;}*/

        FaceShd[FaceNum]=CurPart;
      //status("Loading indexes %d of %d",id,INum/3);
      FaceNum++;
      }
    //BaseIndex+=VNum;
    }
  delete Roofs;
   for(int i=0;i<FaceNum;i++)
     {
     Plane a(Verts[Faces[i*3]].Pos,Verts[Faces[i*3+1]].Pos,Verts[Faces[i*3+2]].Pos);
     PlaneRef[i]=FindPlane(&a);
     //status("Indexing planes %d of %d",i,FaceNum);
     }
  }

int Bsp::FindPlane(const Plane* const p)         //поиск плоскости в уже готовых
  {
  Plane *cp=Planes;
  for(int i=0;i<PlanesNum;i++,cp++)
    {
    if(*cp==*p||*cp==-(*p)) return i;
    }

  Planes[PlanesNum]=*p;
  return PlanesNum++;
  }
int Bsp::FindVertex(const LevelVertex* const v)  //поиск вершины в уже готовых
  {
  LevelVertex *lv=Verts;
  for(int i=0;i<VertNum;i++,lv++)
    {
    if(v->lightmapuv.u!=lv->lightmapuv.u) continue;
    if(v->lightmapuv.v!=lv->lightmapuv.v) continue;
    if(v->uv.u        !=lv->uv.u) continue;
    if(v->uv.v        !=lv->uv.v) continue;
    if(v->Pos.x       !=lv->Pos.x) continue;
    if(v->Pos.y       !=lv->Pos.y) continue;
    if(v->Pos.z       !=lv->Pos.z) continue;
    if(v->Normal.x    !=lv->Normal.x) continue;
    if(v->Normal.y    !=lv->Normal.y) continue;
    if(v->Normal.z    !=lv->Normal.z) continue;
    return i;
    }
  //if(VertNum==65535) throw std::string("can't divide more than 65535 vertexes");
  Verts[VertNum]=*v;
  VertNum++;
  return VertNum-1;
  }

static int Progress=0;
Bsp* Bsp::ConstructBSP()
  {
   FaceList f;
   Face face;

   Progress=0;

   //теперь нужно распределить текстурные координаты для карт освещенности
  //ParseLMCoords();
   //создаем список из граней
   int fn=FaceNum;
   for(int i=0;i<FaceNum;i++)
     {
     face.v[0]=Faces[i*3+0];
     face.v[1]=Faces[i*3+1];
     face.v[2]=Faces[i*3+2];
     face.PlaneRef=PlaneRef[i];
     face.Shader=FaceShd[i];
     face.Roof=IsRoof[i];
     f.push_back(face);

     }
  //print("MlOBJ Loaded\n\t%d Verts\n\t%d Faces\n\t%d Planes",VertNum,FaceNum,PlanesNum);

  //очищаем массив граней, далее информация скинется туда при создании BSP
  FaceNum = 0;
   //status("Started Bsp generation...");
  Divide(NodesNum++,f);
  print("\nBsp Constructed.(%d)\n"
    "\tNodes:%d\n"
    "\tLeafs:%d\n"
    "\tTris:%d\n"
    "\tVertex number:%d\n",fn,Nodes->CountNodes(),Nodes->CountLeafs(),
                         Nodes->CountTris(),VertNum);
    
  return Nodes;
  }
const int PRLEV=10;
void Bsp::Divide(int Index, FaceList &l)
  {
//#define _TEST_
  Bsp *Root = Nodes+Index;
  static int Level=0;
  int i;
  Level++;
//    status("Making Bsp: %3.2f%%. Faces:%d, Verts:%d",200.f*Progress/((1<<PRLEV)-1),FaceNum,VertNum);
  if(Level==6) print(".");
  //находим делящую плоскость
  int plane;
  /*if(Level<4)
    plane=FindBestSplitterM(l);
  else*/
    plane=FindBestSplitter(l);
  Root->PlaneIdx=plane;
  //рассекаем все треугольники
  FaceList FrontList,BackList,ColFront,ColBack;
  Split(l,plane,&FrontList,&BackList,&ColFront,&ColBack);
  //заносим информацию о коллинеарных треугольниках
  //??
  Root->StartFace=FaceNum;
  FaceList::iterator it=ColFront.begin();
  Root->Bound.Degenerate();
  std::map<int,int> nums;
  for(i=0;i<ColFront.size();i++,it++)
    {
    int offs=FaceNum*3;
    Faces[offs+0]=it->v[0];
    Faces[offs+1]=it->v[1];
    Faces[offs+2]=it->v[2];
    FaceShd[FaceNum]=it->Shader;
    PlaneRef[FaceNum]=it->PlaneRef;
    IsRoof[FaceNum]=it->Roof;
    Root->Bound.Enlarge(Verts[Faces[offs+0]].Pos);
    Root->Bound.Enlarge(Verts[Faces[offs+1]].Pos);
    Root->Bound.Enlarge(Verts[Faces[offs+2]].Pos);
#ifdef _TEST_
    if(nums.find(FaceShd[FaceNum])==nums.end())
    nums[FaceShd[FaceNum]]=0;
    else
      nums[FaceShd[FaceNum]]++;
#endif //_TEST
    FaceNum++;
    }
#ifdef _TEST_
  for(std::map<int,int>::iterator n=nums.begin();n!=nums.end();n++)
    {
    if(n->second)
      print("%d ",n->second);
    }
  nums.clear();
#endif //_TEST
  Root->FrontNum=ColFront.size();
  it=ColBack.begin();
  for(i=0;i<ColBack.size();i++,it++)
    {
    int offs=FaceNum*3;
    Faces[offs+0]=it->v[0];
    Faces[offs+1]=it->v[1];
    Faces[offs+2]=it->v[2];
    FaceShd[FaceNum]=it->Shader;
    PlaneRef[FaceNum]=it->PlaneRef;
    IsRoof[FaceNum]=it->Roof;
    Root->Bound.Enlarge(Verts[Faces[offs+0]].Pos);
    Root->Bound.Enlarge(Verts[Faces[offs+1]].Pos);
    Root->Bound.Enlarge(Verts[Faces[offs+2]].Pos);
#ifdef _TEST_
    if(nums.find(FaceShd[FaceNum])==nums.end())
    nums[FaceShd[FaceNum]]=0;
    else
      nums[FaceShd[FaceNum]]++;
#endif //_TEST
    FaceNum++;
    }
#ifdef _TEST_
    {
  for(std::map<int,int>::iterator n=nums.begin();n!=nums.end();n++)
    {
    if(n->second)
    print("%d ",n->second);
    }
    }
  nums.clear();
#endif //_TEST
  Root->AllFacesNum=ColBack.size()+ColFront.size();

  //отрабатываем тоже самое с половинками
  Root->Front=Root->Back=0;
  if(FrontList.size())
    {
    Root->Front=NodesNum++;
    Divide(Root->Front,FrontList);
    Root->Bound.Enlarge(Nodes[Root->Front].Bound);
    }
  else  if(Level<PRLEV)
    {
    Progress+=1<<(PRLEV-Level-1);
    }
  if(BackList.size())
    {
    Root->Back=NodesNum++;
    Divide(Root->Back,BackList);
    Root->Bound.Enlarge(Nodes[Root->Back].Bound);
    }
  else if(Level<PRLEV)
    {
    Progress+=1<<(PRLEV-Level-1);
    }
  
  
  Root->TotalNum=FaceNum-Root->StartFace;
  Root->Bound.Degenerate();
  for(int tn=Root->StartFace;tn<FaceNum;tn++)
    {
    Root->Bound.Enlarge(Verts[Faces[tn*3+0]].Pos);
    Root->Bound.Enlarge(Verts[Faces[tn*3+1]].Pos);
    Root->Bound.Enlarge(Verts[Faces[tn*3+2]].Pos);
    }
  
  if(Level==PRLEV)
    {
    Progress++;
    }
  Level--;
  }
int Bsp::FindBestSplitter(const FaceList &l)
  {
  int Front,Back,CFront,CBack,Div,Mark = 0,size=l.size();
  int BestF=1000000,found = 0;

  std::set<int> ParsedPlanes;

  std::pair<std::set<int>::iterator,bool> pp;

  FaceList::const_iterator it=l.begin(),ei=l.end();
  for(;it!=ei;it++)
    {
    int planeref=it->PlaneRef;
    pp=ParsedPlanes.insert(planeref);
    if(!pp.second) continue;
    CountSplitter(l,planeref,Front,Back,CFront,CBack,Div);
    switch(1)
      {
      case 0:
        Mark=abs(Front-Back)+(Div<<5)-((CFront+CBack)<<3);
        break;
      case 1:
        {
        int splittted=Front+Back+CFront+CBack-size;
        Mark=abs(Front-Back)/10+splittted-((CFront+CBack)/*<<3*/);
        break;
        }
      }
    
    if(Mark<BestF)
      {
      BestF=Mark;
      found=planeref;
      }
    }
  return found;
  }
int Bsp::FindBestSplitterM(const FaceList &l)
  {
  int found;

  FaceList::const_iterator it=l.begin(),ei=l.end();
  BBox bound;
  bound.Degenerate();
  for(;it!=ei;it++)
    {
    bound.Enlarge(Verts[it->v[0]].Pos);
    bound.Enlarge(Verts[it->v[1]].Pos);
    bound.Enlarge(Verts[it->v[2]].Pos);
    }
  point3 center(bound.GetCenter());
  Plane a;
  switch(bound.GetBigAxis())
    {
    default:
    case 0:
      a=Plane(point3(1,0,0),center);
      found=FindPlane(&a);
      break;
    case 1:
      a=Plane(point3(0,1,0),center);
      found=FindPlane(&a);
      break;
    case 2:
      a=Plane(point3(0,0,1),center);
      found=FindPlane(&a);
      break;
    }
  return found;
  }
void Bsp::Split(const FaceList &l,int plane,
                FaceList *Front,     FaceList *Back,
                FaceList *ColFront,  FaceList *ColBack  )
  {
  static int cyc[3+2]={0,1,2,0,1};
  float r[3];
  unsigned cFlags, zFlags;
  Plane *p=&Planes[plane];
  
  FaceList::const_iterator it=l.begin(),ei=l.end();
  for(;it!=ei;it++)
    {
    point3 *tri[3]={&Verts[it->v[0]].Pos,&Verts[it->v[1]].Pos,&Verts[it->v[2]].Pos};
    if(it->PlaneRef==plane)
      {
      point3 Normal=(*tri[0]-*tri[1]).Cross(*tri[2]-*tri[1]);
      if(Normal.Dot(p->Normal)>0.f)  ColFront->push_back(*it);
      else                           ColBack->push_back(*it);
      continue;
      }
    cFlags=zFlags=0;
    if((r[0]=p->TestPointEps(*tri[0]))>0.f)
      cFlags=4;
    else if(r[0]==0.f) zFlags=4;
    if((r[1]=p->TestPointEps(*tri[1]))>0.f)
      cFlags|=2;
    else if(r[1]==0.f) zFlags|=2;
    if((r[2]=p->TestPointEps(*tri[2]))>0.f)
      cFlags|=1;
    else if(r[2]==0.f) zFlags|=1;
    
    if(cFlags==0x00) //все вершины <=0.f
      {//вставка в один из списков треугольников, лежащих на плоскости
      Back->push_back(*it);
      continue;
      }
    if((cFlags|zFlags)==0x7) //все вершины >0.f
      {//вставка в позитивный список
      Front->push_back(*it);
      continue;
      }
    
    const int *V=it->v;
    if(zFlags) //одна вершина на плоскости
      {
      int LastZero;
      switch(zFlags)
        {
        case 0x04: LastZero=0;break; 
        case 0x02: LastZero=1;break; 
        case 0x01: LastZero=2;break; 
          
        default: LastZero=0;break; 
        }
      
      LevelVertex vert;
      float a,b;
      a=fabs(r[cyc[LastZero+1]]);
      b=fabs(r[cyc[LastZero+2]]);
      
      float alpha=a/(a+b);

      vert=Slerp(&Verts[V[cyc[LastZero+1]]],&Verts[V[cyc[LastZero+2]]],alpha);
      
      int newvert=FindVertex(&vert);
      Face facep,facen; //положит., отриц.
      facen.v[0]=V[LastZero];
      facen.v[1]=newvert;
      facen.v[2]=V[cyc[LastZero+2]];
      
      facep.v[0]=V[LastZero];
      facep.v[1]=V[cyc[LastZero+1]];
      facep.v[2]=newvert;
      facen.PlaneRef=facep.PlaneRef=it->PlaneRef;
      facen.Shader=facep.Shader=it->Shader;
      facen.Roof=facep.Roof=it->Roof;
      if(cFlags&(0x04>>(cyc[LastZero+1])))
        {
        Front->push_back(facen);  
        Back->push_back(facep);
        }
      else
        {
        Front->push_back(facep); 
        Back->push_back(facen);
        }
      }
    else //на плоскости не лежит ни одной вершины
      {
      int LonelyV = 0;//одинокая вершина
      switch(cFlags)
        {
        case 0x01: case 0x06: LonelyV=2;break; //001 110
        case 0x02: case 0x05: LonelyV=1;break; //010 101
        case 0x04: case 0x03: LonelyV=0;break; //100 011
          //default: throw CASUS("Error triming tris!");
        }
      float a,b,c;
      LevelVertex v1,v2;
      
      a=fabs(r[LonelyV]);
      b=fabs(r[cyc[LonelyV+1]]);
      c=fabs(r[cyc[LonelyV+2]]);
      
      float alpha1=a/(a+b);
      float alpha2=a/(a+c);

      v1=Slerp(&Verts[V[LonelyV]],&Verts[V[cyc[LonelyV+1]]],alpha1);
      v2=Slerp(&Verts[V[LonelyV]],&Verts[V[cyc[LonelyV+2]]],alpha2);

      int new1,new2;
      new1=FindVertex(&v1);
      new2=FindVertex(&v2);
      
      Face a1,a2,a3;
      a1.v[0]=new1;       a1.v[1]=V[cyc[LonelyV+1]];a1.v[2]=new2;  
      a2.v[0]=new2;       a2.v[1]=V[cyc[LonelyV+1]];a2.v[2]=V[cyc[LonelyV+2]];
      a3.v[0]=V[LonelyV]; a3.v[1]=new1;             a3.v[2]=new2;
      a1.PlaneRef=a2.PlaneRef=a3.PlaneRef=it->PlaneRef;
      a1.Shader=a2.Shader=a3.Shader=it->Shader;
      a1.Roof=a2.Roof=a3.Roof=it->Roof;
      
      if(cFlags&(0x04>>LonelyV))
        {
        Front->push_back(a3); 
        Back->push_back(a1);
        Back->push_back(a2);
        }
      else
        {
        Back->push_back(a3);
        Front->push_back(a1);
        Front->push_back(a2);
        }
      }
    }
  }
void Bsp::CountSplitter(const FaceList &l,int plane,
                int &Front,     int &Back,
                int &ColFront,  int &ColBack  , int &Divided)
  {
  static const int cyc[3+2]={0,1,2,0,1};
  float r[3];
  unsigned cFlags, zFlags;
  Plane *p=&Planes[plane];
  
  Front=Back=ColFront=ColBack=Divided=0;

  FaceList::const_iterator it=l.begin(),ei=l.end();
  for(;it!=ei;it++)
    {
    point3 *tri[3]={&Verts[it->v[0]].Pos,&Verts[it->v[1]].Pos,&Verts[it->v[2]].Pos};
    if(it->PlaneRef==plane)
      {
      point3 Normal=(*tri[0]-*tri[1]).Cross(*tri[2]-*tri[1]);
      if(Normal.Dot(p->Normal)>0.f)    ColFront++;
      else                             ColBack++;
      continue;
      }

    cFlags=zFlags=0;
    if((r[0]=p->TestPoint(*tri[0]))>0.f)
      cFlags=4;
    else if(r[0]==0.f) zFlags=4;
    if((r[1]=p->TestPoint(*tri[1]))>0.f)
      cFlags|=2;
    else if(r[1]==0.f) zFlags|=2;
    if((r[2]=p->TestPoint(*tri[2]))>0.f)
      cFlags|=1;
    else if(r[2]==0.f) zFlags|=1;
    
    if(!cFlags/*cFlags==0x00*/) //все вершины <=0.f
      {
        Back++;
      continue;
      }
    if((cFlags|zFlags)==0x7) //все вершины >0.f
      {//вставка в позитивный список
      Front++;
      continue;
      }
    
    Divided++;
    if(zFlags) //одна вершина на плоскости
      {
      Front++;
      Back++;
      }
    else //на плоскости не лежит ни одной вершины
      {
      int LonelyV;//одинокая вершина
      switch(cFlags)
        {
        case 0x01: case 0x06: LonelyV=0x04>>2;break; //001 110
        case 0x02: case 0x05: LonelyV=0x04>>1;break;  //010 101
        case 0x04: case 0x03: LonelyV=0x04>>0;break; //100 011
          //default: throw CASUS("Error triming tris!");
        }
      if(cFlags&LonelyV)
        {
        Front++;         Back++;      //  Back++;
        }
      else
        {
        Back++;          Front++;     //  Front++;
        }
      }
    }
  }
int Bsp::FindNode(const point3 &Pos,int Index)
  {
  Bsp *Root=Nodes+Index;
  float dist=Planes[Root->PlaneIdx].TestPoint(Pos);
  if(dist<0)
    {
    if(Root->Back) return FindNode(Pos,Root->Back);
    else return Index;
    }
  else
    {
    if(Root->Front) return FindNode(Pos,Root->Front);
    else return Index;
    }
  }
//char toout[1000]="";
bool Bsp::TraceRay(const ray &_ray,Triangle *tri,unsigned flags,int Index)
  {//true - луч пересекся на расстоянии Dist.
  float t,u,v;
  int leaf1,leaf2;
  point3 _p;
  
  Bsp *Root=Nodes+Index;
  Plane *p=Planes+Root->PlaneIdx;
  
  if(!Root->Bound.IntersectRay(_ray,&_p)) return false;
  
  
  float dist=p->TestPoint(_ray.Origin);
  if(dist<0) {leaf1=Root->Back;leaf2=Root->Front;}
  else       {leaf2=Root->Back;leaf1=Root->Front;}
  
  if(leaf1&&TraceRay(_ray,tri,flags,leaf1))
    return true;
  //проверка на пересечение с треугольниками плоскости
  /* by Flif float rrr=_ray.Direction.Length();*/
  Triangle a;
  int st_face=Root->StartFace;
  int all_faces=Root->AllFacesNum;
  int frnt_num=st_face+Root->FrontNum;
  for(int i=st_face;i<st_face+all_faces;i++)
    {
    int shd=FaceShd[i];
    if(!(ShdFlags[shd]&SHDPICK))    continue;
    if(!(ShdFlags[shd]&SHDSHADOW))  continue;
    if(!(flags&ROOFS)&&IsRoof[i])   continue;
    if(!(ShdFlags[shd]&SHD2SIDED)) //отсекаем одностороние материалы
      {//а правильной ли стороной он повернут?
      if((dist<0&&i<frnt_num)||(dist>=0&&i>=frnt_num))
        continue;
      }
    a.V[0]=Verts[Faces[i*3  ]].Pos;
    a.V[1]=Verts[Faces[i*3+1]].Pos;
    a.V[2]=Verts[Faces[i*3+2]].Pos;
    if(a.RayTriangle(_ray.Origin,_ray.Direction,&t,&u,&v)&&t>0/*&&(t/rrr>0.001)*/)
      {
      *tri=a;
      return true;
      }
    }
  if(leaf2&&TraceRay(_ray,tri,flags,leaf2))
    return true;
  return false;
  }

bool Bsp::TraceRay(const ray &_ray,float *Dist, point3 *Norm,unsigned flags, int Index)
  {//true - луч пересекся на расстоянии Dist.
  float d;
  float t,u,v;
  int leaf1,leaf2;
  point3 _p;
  
  Bsp *Root=Nodes+Index;
  Plane *p=Planes+Root->PlaneIdx;
  
  if(!Root->Bound.IntersectRay(_ray,&_p)) return false;
  
  
  float dist=p->TestPoint(_ray.Origin);
  if(dist<0) {leaf1=Root->Back;leaf2=Root->Front;}
  else       {leaf2=Root->Back;leaf1=Root->Front;}
  
  if(leaf1&&TraceRay(_ray,&d,Norm,flags,leaf1))
    {
    *Dist=d;
    return true;
    }
  //проверка на пересечение с треугольниками плоскости
/* by Flif  float rrr=_ray.Direction.Length(); */
  Triangle a;
  int st_face=Root->StartFace;
  int all_faces=Root->AllFacesNum;
  int frnt_num=st_face+Root->FrontNum;
  for(int i=st_face;i<st_face+all_faces;i++)
    {
    int shd=FaceShd[i];
    if(!(ShdFlags[shd]&SHDPICK))    continue;
    if(!(ShdFlags[shd]&SHDSHADOW))  continue;
    if(IsRoof[i]&&!(flags&ROOFS))   continue;
    if(!(ShdFlags[shd]&SHD2SIDED)) //отсекаем одностороние материалы
      {
      //а правильной ли стороной он повернут?
      if((dist<0&&i<frnt_num)||(dist>=0&&i>=frnt_num))
        continue;
      }
    a.V[0]=Verts[Faces[i*3  ]].Pos;
    a.V[1]=Verts[Faces[i*3+1]].Pos;
    a.V[2]=Verts[Faces[i*3+2]].Pos;
    if(a.RayTriangle(_ray.Origin,_ray.Direction,&t,&u,&v)&&t>=0/*&&(t/rrr>0.001)*/)
      {
      /*if(f[0]) *Dist=(t<d1)?t:d1;  else    *Dist=t;*/
      if(i<frnt_num) *Norm = Planes[Root->PlaneIdx].Normal;
      else                   *Norm = -Planes[Root->PlaneIdx].Normal;
      *Dist=t;
      return true;
      }
    }
  if(leaf2&&TraceRay(_ray,&d,Norm,flags,leaf2))
    {
    *Dist=d;
    return true;
    }
  return false;
  }
static int Quality(float &TS,float &psstart, float &psstep)
  {
  TS=0.15;//0,1 см на элемент текстуры (тексел)
  psstart=0.15;
  psstep=0.05;
  int ret=0;
  for(int i=1;i<__argc;i++)
    {
    if(!stricmp(__argv[i],"-qhigh"))
      {
      TS=0.05;//0,1 см на элемент текстуры (тексел)
      psstart=0.05;
      psstep=0.01;
      ret=0;
      }
    else if(!stricmp(__argv[i],"-qmedium"))
      {
      TS=0.20;//0,1 см на элемент текстуры (тексел)
      psstart=0.2;
      psstep=0.07;
      ret=1;
      }
    else if(!stricmp(__argv[i],"-qlow"))
      {
      TS=0.5;//0,1 см на элемент текстуры (тексел)
      psstart=0.5;
      psstep=0.2;
      ret=2;
      }
    }
  return ret;
  }
void Bsp::ParseLMCoords(LMDataMap &/*LMmap*/)
  {
/*
  float DesiredTexSize=0.05;//0,1 см на элемент текстуры (тексел)
  float psstart=0.05;
  float psstep=0.05;
  int qqq=Quality(DesiredTexSize,psstart,psstep);
  float ps=psstart;
int LMx,LMy;
  ProgressBar bar;
  for(int shd=0;shd<ShaderNames.size();shd++)
    {
    //status("Generating Lightmap coords...%3.2f%%",(float)shd*100.f/(ShaderNames.size()-1));
    if (ps>10.f) 
		{
      status("can't generate coords for %s",ShaderNames[shd].c_str());
			ShdFlags[shd]&=~SHDLM;
		}
    if(!(ShdFlags[shd]&SHDLM)) continue;


    TriSet tris;
    float GlobArea=0;
    float GlobPer=0;
    for(int i=0;i<FaceNum;i++)
      {
      if(FaceShd[i]!=shd) continue;		 
			Triangle tt;
			tt.V[0]=Verts[Faces[i*3+0]].Pos;tt.V[1]=Verts[Faces[i*3+1]].Pos;tt.V[2]=Verts[Faces[i*3+2]].Pos;
      float Area=tt.Area();
      float l[3]={
        (Verts[Faces[i*3+0]].Pos-Verts[Faces[i*3+1]].Pos).Length(),
        (Verts[Faces[i*3+1]].Pos-Verts[Faces[i*3+2]].Pos).Length(),
        (Verts[Faces[i*3+2]].Pos-Verts[Faces[i*3+0]].Pos).Length()};
        triinfo t;
        t.len[0]=l[0];t.len[1]=l[1];t.len[2]=l[2];
        t.Area=Area;
        t.Per=0;
        t.Key=i;
        tris.insert(TriSet::value_type(-Area,t));
        GlobArea+=Area;
        GlobPer+=l[0]+l[1]+l[2];
      }
    GlobArea*=1.5;
    if(!LMmap[shd].LMx)
      {
      
       LMx=sqrt(GlobPer/DesiredTexSize+GlobArea/(DesiredTexSize*DesiredTexSize));
       if(LMx<8) LMx=8;
       else if(LMx<16) LMx=16;
       else if(LMx<32) LMx=32;
       else if(LMx<64) LMx=64;
       else if(LMx<128) LMx=128;
       else LMx=256;
       while(tris.size()>(LMx-2)*(LMx-2)/9)
         LMx<<=1;
       if(LMx>256) 
         {
      status("can't generate coords for %s.",ShaderNames[shd].c_str());
				 ShdFlags[shd]&=~SHDLM; continue;
			 }
       LMy=LMx;
       LMmap[shd].LMx=LMx;
       LMmap[shd].LMy=LMy;
      }

    if(ps*ps<GlobArea/(LMx*LMy))
      ps=sqrt(GlobArea/(LMx*LMy));

    Relaxator rel(qqq==2?1:8);
    rel.Init(Relaxator::Pnt(ps,ps),LMx,LMy);
    //status("Generating LM coords..%s(%3.2f%%) ps=%f, %dx%d %d tris.",
    //  ShaderNames[shd].c_str(),100*(float)shd/ShaderNames.size(),ps,LMx,LMy,tris.size());
		
    TriSet::iterator it=tris.begin(),et=tris.end();
    bool Repeat=false;
		while(it!=et)
		{
      Repeat=!rel.AddTri(it->second.Key,it->second.len,it->second.Area);
      if(Repeat)
        break;
      it++;
		}
		if(Repeat)
		{
        {ps+=psstep;}
       // while((ps*ps-ops*ops<ar/(LMx*LMy)));

      shd--;
      continue;
      }
    bar.print("Generated LM coords..%s(%3.2f) ps=%f, %dx%d %d tris.",
      ShaderNames[shd].c_str(),100*(float)shd/ShaderNames.size(),ps,LMx,LMy,tris.size());

    //LMmap[shd].Tris=tris;
    LMmap[shd].PixelSize=ps;
    ps=psstart;
    it=tris.begin();
    Relaxator::TriSet::iterator notri=rel.Tris.end();

    while(it!=et)
      {
      Relaxator::TriSet::iterator itri=rel.Tris.find(it->second.Key);
      if(itri==notri) continue;
      Relaxator::Tri *tri=&(itri->second);
      for(int i=0;i<3;i++)
        {
        int offs=it->second.Key*3;
        static LevelVertex v;
        if(Bsp::Verts[Bsp::Faces[offs+i]].lightmapuv.u!=-1)
          {
          v=Bsp::Verts[Bsp::Faces[offs+i]];
          v.lightmapuv.u=tri->Points[i].x;
          v.lightmapuv.v=tri->Points[i].y;
          Bsp::Verts[VertNum]=v;
          Bsp::Faces[offs+i]=VertNum++;
          }
        else
          {
          Bsp::Verts[Bsp::Faces[offs+i]].lightmapuv.u=tri->Points[i].x;
          Bsp::Verts[Bsp::Faces[offs+i]].lightmapuv.v=tri->Points[i].y;
          
          }
        }
      it++;
      }
    }*/
  }
void Bsp::Traverse(int Index, const point3 &Pos, Frustum *f, unsigned flags)
  {
  if(!Index)
    {//первичная инициализация
    int i;
     memset(PlaneFlags,0,sizeof(unsigned)*PlanesNum);
     for(i=0;i<NodesNum;i++) Nodes[i].Flags=0;
     for(i=0;i<MAXSHADERS;i++) VisNum[i]=0;
    }
  Bsp *Root=Nodes+Index;
  if(flags&NOTVISIBLE)
    { //если узел не виден, то не видны и его потомки
    Root->Flags=NOTVISIBLE;
    if(Root->Front)  Traverse(Root->Front,Pos,f,flags);
    if(Root->Back)   Traverse(Root->Back, Pos,f,flags);
    return;
    }



  Plane *p=Planes+Root->PlaneIdx;
  unsigned *PFlags=PlaneFlags+Root->PlaneIdx;
  if(!(*PFlags&READY))
    {//Эта плоскость еще ни разу не попадалась
    *PFlags|=READY;
    float dist=p->TestPoint(Pos);
    if(dist>0) *PFlags|=FRONT;
    else       *PFlags|=BACK;
    if(f->WhichSide(p)==Frustum::PARTVISIBLE)
      *PFlags|=INFRUSTUM;
    }
  unsigned VF=Frustum::NOTVISIBLE;
  if(flags&VISIBLE||(Frustum::NOTVISIBLE!=(VF=f->TestBBox(Root->Bound))))
    {//текущий узел виден в камере
    //Game::m_pGraphPipe->DrawBBox(Root->Bound,0xffffff);
    bool ObserverFront=*PFlags&FRONT;
    if(!(flags&VISIBLE)&&VF==Frustum::PARTVISIBLE)  Root->Flags=PARTVISIBLE;
    else                                            Root->Flags=VISIBLE;

    if(VF==Frustum::VISIBLE) flags|=VISIBLE;


    if(!ObserverFront){if(Root->Front)  Traverse(Root->Front,Pos,f,flags);}
    else             {if(Root->Back)   Traverse(Root->Back, Pos,f,flags);}
        //а здесь определить и вывести грани
    int start=Root->StartFace;
    int end=start+Root->AllFacesNum;

    bool hroof=!(flags&ROOFS);
    for(int i=start;i<end;i++)
      {//цикл по граням
       if(hroof&&IsRoof[i]) continue;//при отключенных крышах пропускаем все полигоны к ним принадлежащие
      int shd=FaceShd[i];
      if(!(ShdFlags[shd]&SHD2SIDED))
        {//отсекаем одностороние материалы
        if(ObserverFront)
          {
          if(i-start>=Root->FrontNum)     continue;
          }
        else
          {
          if(i-start<Root->FrontNum)       continue;
          }
        }
      VisTris[shd][VisNum[shd]]=Faces[i*3];
      VisTris[shd][VisNum[shd]+1]=Faces[i*3+1];
      VisTris[shd][VisNum[shd]+2]=Faces[i*3+2];
      VisNum[shd]+=3;
      }


    if(ObserverFront){if(Root->Front)  Traverse(Root->Front,Pos,f,flags);}
    else              {if(Root->Back)   Traverse(Root->Back, Pos,f,flags);}
    }
  else
    { //если узел не виден, то не видны и его потомки
    Root->Flags=NOTVISIBLE;
    flags=NOTVISIBLE;
    if(Root->Front)  Traverse(Root->Front,Pos,f,flags);
    if(Root->Back)   Traverse(Root->Back, Pos,f,flags);
    }
  }
void Bsp::ClearVertIndexes(int ShdNum)
  {
	//fixme: соптимизировать
  int _Size=VisNum[ShdNum];
  unsigned short *idxs=VisTris[ShdNum];

  unsigned short *curidx=idxs;
  short *vidx=VertIdx;
#ifdef _NO_ASM
  for(int i=0;i<_Size;i++)    VertIdx[idxs[i]]=-1;
#else
  _asm
    {
    mov esi,curidx;     //idxs
    mov edi,vidx;   //Bsp::VertIdx
    mov ax,-1
    mov ebx, _Size
    mov ecx,ebx
    and ecx,3
    cmp ecx,2
    ja l2align
    je l1align
    cmp ecx,1
    je l0align
l3align:
    mov cx, [esi+ebx*2-2]
    dec ebx
		and ecx,0x0000ffff
    mov [edi+ecx*2],ax
l2align:
    mov cx, [esi+ebx*2-2]
    dec ebx
		and ecx,0x0000ffff
    mov [edi+ecx*2],ax
l1align:
    mov cx, [esi+ebx*2-2]
    dec bx
		and ecx,0x0000ffff
    mov [edi+ecx*2],ax
l0align:
    mov cx, [esi+ebx*2-2]
		and ecx,0x0000ffff
    mov [edi+ecx*2],ax
    dec ebx
    jnz l3align
    }
#endif
  }
