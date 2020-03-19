#pragma warning(disable:4786)

#include "logicdefs.h"


#include "../common/graphpipe/graphpipe.h"
#include "../GameLevel/gamelevel.h"
#include "../Gamelevel/bsp.h"

#include "Entity.h"
#include "Graphic.h"
#include "TraceUtils.h"
#include "DirtyLinks.h"
#include "aiutils.h"
#include "../common/utils/profiler.h"

void ShotTracer::Calc()
  {
	//CodeProfiler CP("SHOTtracer");
  const float BOGUS=100000.f;
  float BestDist=BOGUS;
  material_type BestMat=MT_AIR;
  Touch.TouchPoint=Ray.Origin+Ray.Direction*50.f;
  
  //проверим Bsp
  float Dist; point3 Norm;
  if(Bsp::TraceRay(Ray,&Dist,&Norm,(!VisRoofs||DirtyLinks::GetGameLevel()->RoofVisible)?Bsp::ROOFS:0))
    {
    if((Radius==0.f)||(Dist<Radius))
      {
      Touch.TouchPoint=Ray.Origin+Ray.Direction*Dist;
      Touch.TouchNorm=Norm;
      BestDist=Dist;
      BestMat=MT_WALL;
      }
    }
  //проверим Gamelevel
  //...
  
  GameLevel *g=DirtyLinks::GetGameLevel();
  std::string Name;
  point3 p;
  if(g->LevelObjects.TraceRay(Ray.Origin,Ray.Direction,&p,&Norm,&Name, m_sight?DynObjectPool::TT_SIGHT:DynObjectPool::TT_PHYSIC))
    {
    float Dist=(p-Ray.Origin).Length();
    if(!(Dist>=BestDist||(Radius!=0.f&&Dist>Radius)))
      {
      Touch.TouchPoint=Ray.Origin+Ray.Direction*Dist;
      Touch.TouchNorm=Norm;
      Touch.ObjName=Name;
      BestDist=Dist;
      BestMat=MT_OBJECT;
      }
    }
  
  //проход по массиву существ
  EntityPool::iterator itor     = EntityPool::GetInst()->begin(ET_ALL_ENTS, PT_ALL_PLAYERS, EA_NOT_INCREW);
  EntityPool::iterator end_itor = EntityPool::GetInst()->end();
  
  for(;itor != end_itor; ++itor)
    {
    //id существа это
    unsigned CurId = itor->GetEID();
    
    if(id==CurId || itor->IsRaised(EA_INCREW) || (m_fskip_invisible_ents && !itor->GetGraph()->IsVisible()))
      continue;
    
    //получить доступ к граф. оболочке
    GraphEntity *gi= itor->GetGraph(); //указат. на Person или Vehicle
    if(!gi->TraceRay(Ray,&Dist,&Norm,m_ent_as_box)) continue;
    if(Dist>=BestDist)  continue;
    if(Radius==0.f||Dist<Radius)
      {
      Touch.TouchPoint=Ray.Origin+Ray.Direction*Dist;
      Touch.TouchNorm=Norm;
      BestDist=Dist;
      BestMat=MT_ENTITY;
      Touch.Ent=CurId;
      }
    }
  if(!m_skip_shields)
    {
    /*ShieldPool::iterator s1,se;
    s1=ShieldPool::GetInst()->begin();
    se=ShieldPool::GetInst()->end();
    
    point3 DirN=Normalize(Ray.Direction); 
    point3 D=DirN;D.z=0;
    point3 P0=Ray.Origin;P0.z=0;
    float Dlen=D.Length();
    for(;s1!=se;s1++)
      {
      point3 pos=s1->GetPos();
      point3 pos1=pos;pos1.z=0;
      float  rad=s1->GetRadius();
      float tPer=D.Dot(pos1-P0);
      float dist=(pos1-(P0+D*tPer)).Length();
      if(dist>rad) continue;
      
      float q=sqrt(rad*rad-dist*dist);
      point3 p1;
      
      p1=Ray.Origin+DirN*(tPer/Dlen-q);
      if(tPer/Dlen-q>0&&p1.z>pos.z&&p1.z<pos.z+3)
        {
        Touch.TouchPoint=p1;
        Touch.TouchNorm=Normalize(p1-pos);
        BestDist=(tPer/Dlen-q);
        BestMat=MT_SHIELD;
        }
      }*/
     ShieldPool::iterator s1=ShieldPool::GetInst()->begin(),se=ShieldPool::GetInst()->end();
    point3 N;     float D;
    for(;s1!=se;s1++)
      {
      Cylinder c(s1->GetPos(),3, s1->GetRadius());
      if(c.TraceRay(Ray,&D,&N))
        {
        Touch.TouchPoint=Ray.Origin+Ray.Direction*D;
        Touch.TouchNorm=N;
        BestDist=D;
        BestMat=MT_SHIELD;
        }
      }

    }
  Touch.Mat=BestMat;
}
void _GrenadeTracer::Calc()
{
	point3 From,To;
	From=Ray.Origin;
	To=From+Normalize(Ray.Direction)*Radius;
	point3 ProjDir=(To-From);ProjDir.z=0;
	float dist=ProjDir.Length();
	const float Speed=dist*0.82;
	float Time=dist/Speed;
	float Vy=(To.z-From.z+10*Time*Time/2)/Time;
	point3 Dir=Normalize(ProjDir)*Speed+Vy*AXISZ;
	void TraceGrenade(const point3 &_pos, const point3 &_dir, KeyAnimation *trace, unsigned Skip, float MaxTime);
	TraceGrenade(From,Dir,&ka,SkipEnt, m_Time);
  ka.GetTrans(&Touch.TouchPoint,ka.GetLastTime());
  Touch.TouchNorm=AXISZ;
  Touch.Ent=0;
  Touch.Mat=MT_AIR;
}

class LOSdraw
  {
  public:
    enum{LINES=50};
  private:
    static point3 Lines[LINES*2];
    static unsigned col[LINES*2];
    static int num;
  public:
    static void AddLine(point3 &from, point3 &to, unsigned colfrom=0xffffff,unsigned colto=0xff2f2f)
      {
      if(num==LINES)
        {
        memmove(&Lines[0],&Lines[1*2],(num-1)*sizeof(point3)*2);
        memmove(&col[0],&col[1*2],(num-1)*sizeof(unsigned)*2);
        }
      if(num<LINES) num++;
      Lines[num*2-2]=from;
      Lines[num*2-1]=to;
      col[num*2-2]=colfrom;
      col[num*2-1]=colto;
      }
    static void Draw()
      {
      if(!num) return;
      static Primi prim;
      static texcoord tuv[LINES*2];
      prim.Diffuse=col;
      prim.Pos=Lines;
      prim.UVs[0]=tuv;
      prim.Prim=Primi::LINES;
      prim.Contents=Primi::NEEDTRANSFORM;
      prim.VertNum=num*2;
      prim.IdxNum=0;
      DirtyLinks::GetGraphPipe()->Chop("vertexwhite",&prim);
      }
  };
point3 LOSdraw::Lines[LOSdraw::LINES*2];
unsigned LOSdraw::col[LOSdraw::LINES*2];

int LOSdraw::num=0;
void DrawIt()
  {
  LOSdraw::Draw();
  }


bool LOSTracer::CalcLOS()
  {
	CodeProfiler CP("LOStracer");

//#define _DRAW_LOF_

  SightClear=true;
  float Dist;
  point3 Norm,dot;
	std::string Name;
  
  ray r(Source,Destination-Source);
	point3 right=0.5*Normalize(r.Direction.Cross(AXISZ));
	point3 up=1*Normalize(r.Direction.Cross(right));
  GameLevel *g=DirtyLinks::GetGameLevel();

  
  bool LOSforEnt=did?true:false;
  bool LOSforLev=!LOSforEnt;
  bool LOSforObj=m_ObjName.size()?true:false;

  int counter=1;
	int NumTests=7;                               
#ifdef _DRAW_LOF_
  LOSdraw::AddLine(Destination,Destination+0.2*AXISZ);LOSdraw::AddLine(Destination,Destination-0.2*AXISZ);
  LOSdraw::AddLine(Destination,Destination+0.2*AXISX);LOSdraw::AddLine(Destination,Destination-0.2*AXISX);
  LOSdraw::AddLine(Destination,Destination+0.2*AXISY);LOSdraw::AddLine(Destination,Destination-0.2*AXISY);
#endif

  float RayLen=r.Direction.Length();
	for(int i=0;i<NumTests;i++)
		{
		ray Ray(Source,Destination-Source+right*(frand() - 0.5f)+ up*(frand() - 0.5f));	//fixme: необходимо задать отклонение
		bool LevelHit=false,ObjHit=false;

		if(Bsp::TraceRay(Ray,&Dist,&Norm,Bsp::ROOFS))
            if(Dist<1.f) LevelHit=true;

		if(!LevelHit&&g->LevelObjects.TraceRay(Ray.Origin,Ray.Direction,&dot,&Norm,&Name))
				if((dot-Ray.Origin).Length()-0.5<RayLen&&m_ObjName!=Name)     ObjHit=true;
#ifdef _DRAW_LOF_
    if(LevelHit)     LOSdraw::AddLine(Source,Ray.Origin+Ray.Direction*Dist,0x20ff20,0xff2020);
    else if(ObjHit)  LOSdraw::AddLine(Source,dot,0x2020ff,0xff2020);
    else             LOSdraw::AddLine(Source,Ray.Origin+Ray.Direction,0x7f7f7f,0xffffff);
#endif      
		if(LevelHit||ObjHit) counter--;
		else 
      {
      //fixme:
      counter=1;
      break;
      //^^^^^^^^^^^^
      counter++;
      }
	}
	SightClear=counter>0?true:false;

  return SightClear;
  }


