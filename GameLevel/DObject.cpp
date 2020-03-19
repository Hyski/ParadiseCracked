// DObject.cpp: implementation of the DObject class.
//
//////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include "../skin/skin.h"

#include "DObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DObject::DObject()
{
  PartsNum=0;
  Name="UnInitialized";
  LastTime=0;
  Animation=false;
  State=0;
}

DObject::~DObject()
{

}

void DObject::UpdateOnTime(float Time)
{
   std::map<float, point3>::iterator st,et;
  et=Trans.upper_bound(Time);

    if(et==Trans.end())
      {//не нужно интерпол€ции - да, прошли те времена
      et--; st=et;
      Translate=et->second;
      }
    else
      {
      st=et;
      if(et==Trans.begin())
        {//Ќе нужно интерпол€ции - врем€ анимации еще не пришло
        Translate=et->second;
        }
      else
        {
        //интерпол€ци€
        st--;
        float in;
        in=(float)(Time-st->first)/(et->first-st->first);
        Translate=et->second*in+st->second*(1-in);
        }
      }
  std::map<ScelTime, Quaternion>::iterator sq,eq;
  eq=Angles.upper_bound(Time);

    if(eq==Angles.end())
      {//не нужно интерпол€ции - да, прошли те времена
      eq--; sq=eq;
      Angle=eq->second;
      }
    else
      {
      sq=eq;
      if(eq==Angles.begin())
        {//Ќе нужно интерпол€ции - врем€ анимации еще не пришло
        Angle=eq->second;
        }
      else
        {
        //интерпол€ци€
        sq--;
        float in;
        in=(float)(Time-sq->first)/(eq->first-sq->first);
        Angle=Angle.Slerp(in,sq->second,eq->second);
        }
      }
    matrix3 rot;
    Angle.ToRotationMatrix(rot);
    D3DUtil_SetIdentityMatrix(World);
    World._41=Translate.x;World._42=Translate.y;World._43=Translate.z;
    World._11=rot[0][0]; World._12=rot[0][1]; World._13=rot[0][2];
    World._21=rot[1][0]; World._22=rot[1][1]; World._23=rot[1][2];
    World._31=rot[2][0]; World._32=rot[2][1]; World._33=rot[2][2];
}

void DObject::Load(FILE *f)
{
  char t[200];
  int kn;
  int i;
  fscanf(f,"Name:");
  fgets(t,199,f);
  while(t[strlen(t)-1]=='\n'||t[strlen(t)-1]=='\r') t[strlen(t)-1]=0;
  strlwr(t);
  Name=t;
  fscanf(f,"KeyNumber:%d\n",&kn);
    {
  float x,y,z,w,t;
  
  for(i=0;i<kn;i++)
    {
    fscanf(f,"%f[%f %f %f %f] ",&t,&x,&y,&z,&w);
    Angles[t]=Quaternion(w,x,y,z);
    }
  fscanf(f,"\n");
  
  fscanf(f,"KeyNumber:%d\n",&kn);
  for(i=0;i<kn;i++)
    {
    fscanf(f,"%f[%f %f %f] ",&t,&x,&y,&z);
    Trans[t]=point3(x,y,z);
    }
  fscanf(f,"\n");
    }
  PartsVec.clear();
  fscanf(f,"PartsNumber:%d\n",&PartsNum);
  LocalBound.Degenerate();
  for(i=0;i<PartsNum;i++)
    {
    SimpleTexturedObject *Obj=new SimpleTexturedObject;
    Obj->Load(f);
    PartsVec.push_back(*Obj);
    Parts[i]=&PartsVec[i];
    for(int p=0;p<Parts[i]->PntNum;p++)
      LocalBound.Enlarge(Parts[i]->Points[p]);
    //delete Obj;
    }
  UpdateOnTime(0);
  LastTime=GetLastTime();
  UpdateBound();
}
void DObject::ChangeState(float state, float Time)
  {
  if(Animation)
    {
  float state=fabs(EndState-State)*LastTime;
  float TimeDelta=(Time-StartTime)/state;
  if(TimeDelta>1) TimeDelta=1;
  State=(State+(EndState-State)*TimeDelta);
    }
   Animation=true;
   EndState=state;
   StartTime=Time;
  }

void DObject::Update(float Time)
{
if(Animation)
  {
  float state=fabs(EndState-State)*LastTime;
  float TimeDelta=(Time-StartTime)/state;
  if(TimeDelta>1){ TimeDelta=1;Animation=false;State=EndState;}
  float CurTime=(State+(EndState-State)*TimeDelta)*LastTime;
  UpdateOnTime(CurTime);
  UpdateBound();
  }
}

void DObject::UpdateBound()
  {
  point3 points[8]={
                    point3(LocalBound.minx,LocalBound.miny,LocalBound.minz),
                    point3(LocalBound.minx,LocalBound.miny,LocalBound.maxz),
                    point3(LocalBound.minx,LocalBound.maxy,LocalBound.minz),
                    point3(LocalBound.minx,LocalBound.maxy,LocalBound.maxz),
                    point3(LocalBound.maxx,LocalBound.miny,LocalBound.minz),
                    point3(LocalBound.maxx,LocalBound.miny,LocalBound.maxz),
                    point3(LocalBound.maxx,LocalBound.maxy,LocalBound.minz),
                    point3(LocalBound.maxx,LocalBound.maxy,LocalBound.maxz)
    };
  point3 tpoints[8];
  VectorMatrixMultiply(TODXVECTOR(tpoints[0]),TODXVECTOR(points[0]),World);
  VectorMatrixMultiply(TODXVECTOR(tpoints[1]),TODXVECTOR(points[1]),World);
  VectorMatrixMultiply(TODXVECTOR(tpoints[2]),TODXVECTOR(points[2]),World);
  VectorMatrixMultiply(TODXVECTOR(tpoints[3]),TODXVECTOR(points[3]),World);
  VectorMatrixMultiply(TODXVECTOR(tpoints[4]),TODXVECTOR(points[4]),World);
  VectorMatrixMultiply(TODXVECTOR(tpoints[5]),TODXVECTOR(points[5]),World);
  VectorMatrixMultiply(TODXVECTOR(tpoints[6]),TODXVECTOR(points[6]),World);
  VectorMatrixMultiply(TODXVECTOR(tpoints[7]),TODXVECTOR(points[7]),World);

  Bound.Degenerate();
  Bound.Enlarge(tpoints[0]);  Bound.Enlarge(tpoints[1]);
  Bound.Enlarge(tpoints[2]);  Bound.Enlarge(tpoints[3]);
  Bound.Enlarge(tpoints[4]);  Bound.Enlarge(tpoints[5]);
  Bound.Enlarge(tpoints[6]);  Bound.Enlarge(tpoints[7]);
  }
