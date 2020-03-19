// Bone.cpp: implementation of the Bone class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include "Bone.h"

#ifdef COMPILEFORMAX
#include "../../myexport.h"
#include "../../sceneexport.h"
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Bone::Bone()
{
  UseHierarchy=0;
  ChildNum=0;
  Parent=NULL;
}

Bone::~Bone()
{

  }

//»нтерполировать позицию дл€ указанного времени
void Bone::UpdateOnTime(ScelTime Time, float delay, Bone *Ancestor)
  {
  //  lower_bound;
  //  upper_bound
  Keys.GetTrans(&Translate,Time);
  Keys.GetAngle(&CurAngles,Time);
  if(Ancestor&&Time<0)
    {
    float in;
    in=(float)(Time-delay)/(-delay);
    Translate=Translate*in+Ancestor->Translate*(1.f-in);
    CurAngles=CurAngles.Slerp(in,Ancestor->CurAngles,CurAngles);
    } 
  }

//подсчитать окончательную матрицу позиции
void Bone::CalcTransform(Transformation *World)
  {
  if(UseHierarchy&&Parent)
    {
    WorldAngles=CurAngles*Parent->WorldAngles;
    WorldAngles.ToRotationMatrix(Transform.rot);
    point3 x(Parent->Transform.rot[0][0],Parent->Transform.rot[0][1],Parent->Transform.rot[0][2]);
    point3 y(Parent->Transform.rot[1][0],Parent->Transform.rot[1][1],Parent->Transform.rot[1][2]);
    point3 z(Parent->Transform.rot[2][0],Parent->Transform.rot[2][1],Parent->Transform.rot[2][2]);
    Transform.trans=Parent->Transform.trans+(Translate.x*x+Translate.y*y+Translate.z*z);
    }
  else
    {
    if(World)
      {
      Quaternion q;
      q.FromRotationMatrix(World->rot);

      Transform.trans=q*Translate+World->trans;
      q.w=-q.w;
      WorldAngles=CurAngles*q;
      }
    else
      {
      Transform.trans=Translate;
      WorldAngles=CurAngles;
      }
      WorldAngles.ToRotationMatrix(Transform.rot);
    }
  }

#ifdef COMPILEFORMAX
float GetScale()
  {
  int type;
  float scale;
  /*GetMasterUnitInfo(&type,&scale);
  switch(type)
    {
    case UNITS_INCHES: scale*=0.0252;break;
    case UNITS_FEET:break;
    case UNITS_MILES:break;
    case UNITS_MILLIMETERS:scale*=0.001;break;
    case UNITS_CENTIMETERS:scale*=0.01;break;
    case UNITS_METERS:scale*=1;break;
    case UNITS_KILOMETERS:scale*=1000;break;
    }  */
  scale=GetMasterScale(UNITS_METERS);
  return scale;
  }
//сконструировать узел исход€ из анимации в 3D Max 3.0
void Bone::ParseKeys(INode *node, SkeletonExporter *sc, bool First)
  {
  float scale=GetScale();
  int firstnode=First;
  Matrix3 m;
  float ang[3];
  ScelTime time;

  Interval ToExport;
  UseHierarchy=sc->HierarchyKeys&&(!First);
  switch(sc->UseTimeInterval)
    {
    case 0://все FIXME: далее вычитаетс€ начало интервала
      //ToExport=FOREVER;
      //break;
    case 1:
      ToExport=sc->ip->GetAnimRange();
      break;
    case 2:
      ToExport=Interval(sc->TimeFrom*GetTicksPerFrame(),sc->TimeTo*GetTicksPerFrame());
      break;
    };
  Name=string(node->GetName());
  Control *c=node->GetTMController();
  //firstnode=c->NumSubs();
  
  //MessageBox(0,"",Name.c_str(),MB_OK);
/*  Tab<TimeValue> tms;
  if(firstnode)
    {
    Animatable *vert=c->SubAnim(0);
    Animatable *horz=c->SubAnim(1);
    Animatable *rot=c->SubAnim(2);
    if(vert&&horz&&rot)
      {
      vert->GetKeyTimes(tms,ToExport,0);
      }
    else
      c->GetKeyTimes(tms,ToExport,0);
    }
  else
    c->GetKeyTimes(tms,ToExport,0);
  */


 // if(tms.Count())
  if(1)
    {
    Quaternion toaddang;
    point3     toaddtrans,lastt;
    int waslasta=0,waslastt=0;
    float lastang[3];
    ScelTime angadd,tradd;
    const float err_pos=fabs(sc->PrecisionPos);
    const float err_ang=fabs(sc->PrecisionAng);
    float error_pos=0,error_ang=0;
    
    //начальные установки.
    time=0;//((ScelTime)tms[0]-ToExport.Start())/GetTicksPerFrame()/GetFrameRate();
    m=Get_Relative_Matrix(node,ToExport.Start());
    Quat m1(m);
    m1.Normalize();

    point3 ofs(m.GetTrans().x,m.GetTrans().y,m.GetTrans().z);
    MatrixToEuler(m,(float*)&lastang,EULERTYPE_XYZ);
    lastt=point3(m.GetTrans().x,m.GetTrans().y,m.GetTrans().z);
    if(err_ang+err_pos>0.f)
      {
      Angles[time]=Quaternion(m1.w,m1.x,m1.y,m1.z);
      Trans[time]=scale*lastt;
      }
    //идем по ключам, сохран€ем их только при значительном изменении
    //for(int i=0;i<tms.Count();i++)
    for(int i=ToExport.Start();i<ToExport.End();i+=GetTicksPerFrame())
      {
      //m.IdentityMatrix();
      //c->GetValue(tms[i], (void *)&m, FOREVER);
      if(UseHierarchy)
        m=Get_Relative_Matrix(node,i);
      else
        m=Uniform_Matrix(node->GetNodeTM(i));

      time=((ScelTime)i-ToExport.Start())/GetTicksPerFrame()/GetFrameRate();
      point3 ofs(m.GetTrans().x,m.GetTrans().y,m.GetTrans().z);
      MatrixToEuler(m,(float*)&ang,EULERTYPE_XYZ);
      Quat mq(m);
      mq.Normalize();
      if(err_ang+err_pos==0.f)
        {
        Angles[time]=Quaternion(mq.w,mq.x,mq.y,mq.z);
        Trans[time]=scale*ofs;
        }
      else
        {
        error_ang+=fabs(lastang[0]-ang[0])+fabs(lastang[1]-ang[1])+fabs(lastang[2]-ang[2]);
        point3 hlp(lastt-ofs);
        error_pos+=scale*hlp.Length();
        
        if(error_ang>err_ang)
          {
          error_ang=0;
          if(waslasta)
            {
            Angles[angadd]=toaddang;
            }
          Angles[time]=Quaternion(mq.w,mq.x,mq.y,mq.z);
          memmove(lastang,ang,sizeof(float)*3);
          waslasta=0;
          }
        else
          waslasta=1;
        if(error_pos>err_pos)
          {
          error_pos=0;
          if(waslastt)
            {
            Trans[tradd]=scale*toaddtrans;
            }
          Trans[time]=scale*ofs;
          lastt=ofs;
          waslastt=0;
          }
        else
          waslastt=1;
        
        if(!i) waslastt=waslasta=0;
        if(waslasta)
          {
          toaddang=Quaternion(mq.w,mq.x,mq.y,mq.z);
          angadd=time;
          }
        if(waslastt)
          {
          toaddtrans=ofs;
          tradd=time;
          }
        }
      }
    }
  else//no animation at all
    {
    Matrix3 m;
    m=Get_Relative_Matrix( node,ToExport.Start());
    
    Quat mq(m);
    
    Angles[(ScelTime)0]=Quaternion(mq.w,mq.x,mq.y,mq.z);
    Trans[(ScelTime)0]=point3(m.GetTrans().x,m.GetTrans().y,m.GetTrans().z);
    }
  }
#endif

void Bone::OptimizeKeys()
  {
  }

//ѕрочитать с диска
int Bone::Load(FILE *f)
  { 
  
  char q[100];
  fscanf(f,"Bone Name:",q);
  fgets(q,99,f);
  while(q[strlen(q)-1]=='\n'||q[strlen(q)-1]=='\r')q[strlen(q)-1]=0;
  Name=q;
  
  fscanf(f,"Use Hierarchy:%d\n",&UseHierarchy);
  Keys.Load(f);
  
  return 1;
  }
//«аписать на диск
int Bone::Save(FILE *f)
  {
  
  fprintf(f,"Bone Name:%s\n",Name.c_str());
  
  fprintf(f,"Use Hierarchy:%d\n",UseHierarchy);
  Keys.Save(f);
  return 1;
  }
//ѕрочитать с диска
int Bone::LoadBin(VFile *f)
  { 
  int i;
  char q[100];
  f->Read(&i,sizeof(i));
  f->Read(q,i);
	q[i]=0;
  Name=q;
  
  f->Read(&UseHierarchy,sizeof(UseHierarchy));
  Keys.LoadBin(f);
  return 1;
  }
//«аписать на диск
int Bone::SaveBin(FILE *f)
  {
  int i=Name.size()+1;
  fwrite(&i,1,sizeof(i),f);
  fwrite(Name.c_str(),1,i,f);
  fwrite(&UseHierarchy,1,sizeof(UseHierarchy),f);
  Keys.SaveBin(f);
  return 1;
  }
