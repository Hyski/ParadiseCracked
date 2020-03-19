// bsp.h: interface for the Level class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEVEL_H__5B93A1D6_9D22_4E42_AC34_4F44B57AA2F4__INCLUDED_)
#define AFX_LEVEL_H__5B93A1D6_9D22_4E42_AC34_4F44B57AA2F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//!!!!!!!!!!!!!каждый треугольник должен сылаться на плоскость.
struct LevelVertex;
class Frustum;
class Bsp
  {
  public:
    struct triinfo
      {
      float Area;
      float Per;
      float len[3];
      int Key;
      };
    
    typedef std::multimap<float,triinfo> TriSet;
    struct LightmapData
      {
      int LMx,LMy;
      float PixelSize;
      //TriSet Tris;
      LightmapData():LMx(0),LMy(0),PixelSize(0){};
      };
    typedef std::map<int, LightmapData> LMDataMap;
    
      protected:
        struct Face
          {
          int v[3]; //три индекса на вершины
      unsigned Shader; //индекс шейдера
      unsigned PlaneRef;
      bool Roof;
      };


    typedef std::list<Face> FaceList;
    enum {MAXSHADERS=300 };
  public:
    enum VISIBILYTY {VISIBLE=0x01, NOTVISIBLE=0x02, PARTVISIBLE=0x04, ROOFS=0x08};
    enum PLANEFLAGS {READY=0x80000000, FRONT=0x01, BACK=0x02, INFRUSTUM=0x04};
    enum SHDFLAGS   {SHDSOLID=0x01,SHDSHADOW=0x02,SHDPICK=0x04,
                     SHDTRANS=0x08,SHDLM=0x10,SHDDLIGHT=0x20,
                     SHD2SIDED=0x40};
    //плоскости
    static Plane    *Planes;    //набор секущих планов
    static int       PlanesNum;
    static unsigned *PlaneFlags;//и их флаги
    //вершины
    static LevelVertex *Verts;//массив вершин
    static int VertNum;       //количество вершин
    static short    *VertIdx;  //при записи вершин в VB здесь 
                               //лежит либо текущий номер вершины в
                               //VB либо -1 если вершина еще не в VB
    //треугольники
    static unsigned *Faces;    //набор индексов по 3 на грань
    static unsigned *FaceShd;  //номер шейдера для грани
    static int      *IsRoof;   //номер шейдера для грани
    static unsigned *PlaneRef; //номер плоскости для грани
    static int       FaceNum;  //количество граней

    //узлы дерева
    static std::vector<std::string> ShaderNames;
    static unsigned     *Bsp::ShdFlags;//шейдер имеет карту освещенности

    static Bsp      *Nodes; //узлы BSP дерева
    static int       NodesNum;   //и их количество
    //видимость
    static unsigned short *VisTris[MAXSHADERS]; //массивы номеров видимых граней по материалам
    static unsigned short *IdxTris; //массивы индексов видимых граней по материалам
    static unsigned        VisNum[MAXSHADERS]; //количество видимых треугольников для каждого материала
  public:
    static void Init();
    static void Close();
  public:

    static void Load(FILE *f);
    static void LoadFromModel(FILE *f);
    static void Save(FILE *f);
    static bool MakeSaveLoad(SavSlot &slot);
    static void ClearVertIndexes(int ShdNum);

    static Bsp* ConstructBSP();
    static void Divide(int Index, FaceList &l);
    static int FindBestSplitter(const FaceList &l);
    static int FindBestSplitterM(const FaceList &l);
    static void Split(const FaceList &l,int plane,
      FaceList *Front,     FaceList *Back,
      FaceList *ColFront,  FaceList *ColBack );
    static void CountSplitter(const FaceList &l,int plane,
                int &Front,     int &Back,
                int &ColFront,  int &ColBack  , int &Divided);
    static void ParseLMCoords(LMDataMap &LMmap);

    static int FindPlane(const Plane* const p);         //поиск плоскости в уже готовых
    static int FindVertex(const LevelVertex* const v);  //поиск веершины в уже готовых
    static int FindNode(const point3 &Pos,int Index=0);
    static bool TraceRay(const ray &_ray,float *Dist, point3 *Norm,unsigned flags=ROOFS,int Index=0);
    static bool TraceRay(const ray &_ray,Triangle *tri,unsigned flags=ROOFS,int Index=0);
  public: 
    //обход дерева с определением видимых треугольников
    static void Traverse(int Index, const point3 &Pos, Frustum *f, unsigned flags);
  public:
    unsigned Front, Back;//дочерние узлы
    int PlaneIdx;        //индекс секущего плана
    //BBox b;
    int StartFace;       //индекс первой грани
    int FrontNum;        //количество прямо повернутых граней
    int AllFacesNum;     //общее количество граней
    int TotalNum;        //количество граней с учетом дочерних узлов
    unsigned Flags;      //
    BBox Bound;
    int CountNodes()
      {
      int i=1;
      if(Back) i+=Nodes[Back].CountNodes();
      if(Front) i+=Nodes[Front].CountNodes();
      return i;
      }
    int CountLeafs()
      {
      int i=0;
      if(Back) i+=Nodes[Back].CountLeafs();
      if(Front) i+=Nodes[Front].CountLeafs();
      if(!Back&&!Front) i++;
      return i;
      }
    int CountTris()
      {
      return TotalNum;
      }
  };

#endif // !defined(AFX_LEVEL_H__5B93A1D6_9D22_4E42_AC34_4F44B57AA2F4__INCLUDED_)
