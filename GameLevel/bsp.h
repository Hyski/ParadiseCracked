// bsp.h: interface for the Level class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEVEL_H__5B93A1D6_9D22_4E42_AC34_4F44B57AA2F4__INCLUDED_)
#define AFX_LEVEL_H__5B93A1D6_9D22_4E42_AC34_4F44B57AA2F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//!!!!!!!!!!!!!������ ����������� ������ �������� �� ���������.
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
          int v[3]; //��� ������� �� �������
      unsigned Shader; //������ �������
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
    //���������
    static Plane    *Planes;    //����� ������� ������
    static int       PlanesNum;
    static unsigned *PlaneFlags;//� �� �����
    //�������
    static LevelVertex *Verts;//������ ������
    static int VertNum;       //���������� ������
    static short    *VertIdx;  //��� ������ ������ � VB ����� 
                               //����� ���� ������� ����� ������� �
                               //VB ���� -1 ���� ������� ��� �� � VB
    //������������
    static unsigned *Faces;    //����� �������� �� 3 �� �����
    static unsigned *FaceShd;  //����� ������� ��� �����
    static int      *IsRoof;   //����� ������� ��� �����
    static unsigned *PlaneRef; //����� ��������� ��� �����
    static int       FaceNum;  //���������� ������

    //���� ������
    static std::vector<std::string> ShaderNames;
    static unsigned     *Bsp::ShdFlags;//������ ����� ����� ������������

    static Bsp      *Nodes; //���� BSP ������
    static int       NodesNum;   //� �� ����������
    //���������
    static unsigned short *VisTris[MAXSHADERS]; //������� ������� ������� ������ �� ����������
    static unsigned short *IdxTris; //������� �������� ������� ������ �� ����������
    static unsigned        VisNum[MAXSHADERS]; //���������� ������� ������������� ��� ������� ���������
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

    static int FindPlane(const Plane* const p);         //����� ��������� � ��� �������
    static int FindVertex(const LevelVertex* const v);  //����� �������� � ��� �������
    static int FindNode(const point3 &Pos,int Index=0);
    static bool TraceRay(const ray &_ray,float *Dist, point3 *Norm,unsigned flags=ROOFS,int Index=0);
    static bool TraceRay(const ray &_ray,Triangle *tri,unsigned flags=ROOFS,int Index=0);
  public: 
    //����� ������ � ������������ ������� �������������
    static void Traverse(int Index, const point3 &Pos, Frustum *f, unsigned flags);
  public:
    unsigned Front, Back;//�������� ����
    int PlaneIdx;        //������ �������� �����
    //BBox b;
    int StartFace;       //������ ������ �����
    int FrontNum;        //���������� ����� ���������� ������
    int AllFacesNum;     //����� ���������� ������
    int TotalNum;        //���������� ������ � ������ �������� �����
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
