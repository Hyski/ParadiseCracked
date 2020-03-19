// SimpleTexturedObject.cpp: implementation of the SimpleTexturedObject class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include "SimpleTexturedObject.h"
#include "culling.h"
#include "../saveload/saveload.h"
#include "../utils/optslot.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// создадим функции для доступа к индексам - на чтение и на чтение/запись
short int * SimpleTexturedObject::GetIndexesFull()
{
	return indexes;
}

const short int*  const SimpleTexturedObject::GetIndexesRO()
{
	return indexes;
}


void SimpleTexturedObject::FreeAll()
{
	FREE(uv);
	FREE(Points);
	FREE(Normals);
	
	FREE(indexes);

	IdxNum=0;
	PntNum=0;
}

void SimpleTexturedObject::Load (FILE *f)
  {
  //char t[200];
  FreeAll();

  fscanf(f,"Material:");
  LoadString(f,&MaterialName);
  
  
  fscanf(f,"Vertex number:%d\n",&(PntNum));
  if(PntNum)
    {
  Points=new point3[PntNum];
  Normals=new point3[PntNum];
  uv=new texcoord[PntNum];
    }
  else
    {
  Points=NULL;
  Normals=NULL;
  uv=NULL;
    }
  //Roof=new bool[PntNum];
  
  for(int v=0;v<PntNum;v++)
    {
    fscanf(f,"Pos:%f %f %f\n",
      &Points[v].x,
      &Points[v].y,
      &Points[v].z);
    fscanf(f,"Normal:%f %f %f\n",
      &Normals[v].x,
      &Normals[v].y,
      &Normals[v].z);
		Normals[v] = Normalize(Normals[v]);
    //Points[v].y=-Points[v].y;
    //Normals[v].y=-Normals[v].y;
    fscanf(f,"uv:%f %f\n",&uv[v].u,&uv[v].v);
    int Roof;
    fscanf(f,"Roof:%d\n",&Roof);
    }
  fscanf(f,"Index number:%d\n",&IdxNum);
  if(IdxNum)
    indexes=new short int[IdxNum];
  else indexes=NULL;
  for(int id=0;id<IdxNum;id++)
    {
    fscanf(f,"%d ",&indexes[id]);
    }
  fscanf(f,"\n");  
  
  }
void SimpleTexturedObject::Save(FILE *f)
  {
  fprintf(f,"Material:");
  SaveString(f,MaterialName);
  
  
  fprintf(f,"Vertex number:%d\n",(PntNum));
  
  for(int v=0;v<PntNum;v++)
    {
    fprintf(f,"Pos:%f %f %f\n",
      Points[v].x,
      Points[v].y,
      Points[v].z);
    fprintf(f,"Normal:%f %f %f\n",
      Normals[v].x,
      Normals[v].y,
      Normals[v].z);
    fprintf(f,"uv:%f %f\n",uv[v].u,uv[v].v);
    }
  fprintf(f,"Index number:%d\n",IdxNum);
  
  for(int id=0;id<IdxNum;id++)
    {
    fprintf(f,"%d ",indexes[id]);
    }
  fprintf(f,"\n"); 
  }
bool SimpleTexturedObject::MakeSaveLoad(SavSlot &slot)
  {
  if(slot.IsSaving())
    {
    slot<<MaterialName;
    slot<<PntNum;
    for(int v=0;v<PntNum;v++)
      {
      slot<<Points[v].x<<Points[v].y<<Points[v].z;
      slot<<Normals[v].x<<Normals[v].y<<Normals[v].z;
      slot<<uv[v].u<<uv[v].v;
      }
    slot<<IdxNum;
    for(int id=0;id<IdxNum;id++)
      slot<<indexes[id];
    }
  else
    {
    FreeAll();
    slot>>MaterialName;
    slot>>PntNum;
    if(PntNum)
      {
      Points=new point3[PntNum];
      Normals=new point3[PntNum];
      uv=new texcoord[PntNum];
      }
    else
      {
      Points=NULL;
      Normals=NULL;
      uv=NULL;
      }
    for(int v=0;v<PntNum;v++)
      {
      slot>>Points[v].x>>Points[v].y>>Points[v].z;
      slot>>Normals[v].x>>Normals[v].y>>Normals[v].z;
			Normals[v] = Normalize(Normals[v]);
      slot>>uv[v].u>>uv[v].v;
      }
    slot>>IdxNum;
    if(IdxNum)
      indexes=new short int[IdxNum];
    else indexes=NULL;
    for(int id=0;id<IdxNum;id++)
      slot>>indexes[id];
    }
  return true;
  }
void SimpleTexturedObject::Save(OptSlot &slot)
  {
    slot<<MaterialName;
    slot<<PntNum;
    for(int v=0;v<PntNum;v++)
      {
      slot<<Points[v].x<<Points[v].y<<Points[v].z;
      slot<<Normals[v].x<<Normals[v].y<<Normals[v].z;
      slot<<uv[v].u<<uv[v].v;
      }
    slot<<IdxNum;
    for(int id=0;id<IdxNum;id++)
      slot<<indexes[id];

	}
void SimpleTexturedObject::Load(OptSlot &slot)
    {
    FreeAll();
    slot>>MaterialName;
    slot>>PntNum;
    if(PntNum)
      {
      Points=new point3[PntNum];
      Normals=new point3[PntNum];
      uv=new texcoord[PntNum];
      }
    else
      {
      Points=NULL;
      Normals=NULL;
      uv=NULL;
      }
    for(int v=0;v<PntNum;v++)
      {
      slot>>Points[v].x>>Points[v].y>>Points[v].z;
      slot>>Normals[v].x>>Normals[v].y>>Normals[v].z;
			Normals[v] = Normalize(Normals[v]);
      slot>>uv[v].u>>uv[v].v;
      }
    slot>>IdxNum;
    if(IdxNum)
      indexes=new short int[IdxNum];
    else indexes=NULL;
    for(int id=0;id<IdxNum;id++)
      slot>>indexes[id];
    }
void TexObject::Load(FILE *f)
  {
  Release();
  fscanf(f,"PartsNumber:%d\n",&PartNum);

  for(int i=0;i<PartNum;i++)
    {
    Parts[i]=new STObject;
    Parts[i]->Load(f);
    }
  }
void TexObject::Save(FILE *f)
  {
  fprintf(f,"PartsNumber:%d\n",PartNum);

  for(int i=0;i<PartNum;i++)
    {
    Parts[i]->Save(f);
    }
  }
BBox TexObject::GetBBox()
  {
  BBox b;
  b.Degenerate();
  for(int i=0;i<PartNum;i++)
    {
    for(int j=0;j<Parts[i]->PntNum;j++)
      b.Enlarge(Parts[i]->Points[j]);
    }
  return b;
  }
bool TexObject::TraceRay(const ray &r, float *Pos, point3 *Norm) const
  {
  enum{BOGUS=1000000};
  *Pos=BOGUS;
  Triangle a;
  float t,u,v;
  for(int i=0;i<PartNum;i++)
    {
    point3 *pnt=Parts[i]->Points;
    short int *idxs=Parts[i]->GetIndexesFull();

    for(int j=0;j<Parts[i]->IdxNum;j+=3)
      {
      a.V[0]=pnt[idxs[j  ]];
      a.V[1]=pnt[idxs[j+1]];
      a.V[2]=pnt[idxs[j+2]];
      if(!(a.RayTriangle(r.Origin,r.Direction,&t,&u,&v)&&t>0)) continue;
      if(t>=*Pos) continue;
      *Pos=t;
      *Norm=Normalize((a.V[1]-a.V[0]).Cross(a.V[2]-a.V[0]));
      }
    }
  return *Pos!=BOGUS;
  }
bool TexObject::MakeSaveLoad(SavSlot &slot)
  {
  if(slot.IsSaving())
    {
    slot<<PartNum;
    }
  else
    {
    Release();
    slot>>PartNum;
    for(int i=0;i<PartNum;i++)
      Parts[i]=new STObject;
    }
  for(int i=0;i<PartNum;i++)
    Parts[i]->MakeSaveLoad(slot);  
  return true;
  }
void TexObject::Save(OptSlot &slot)
  {
    slot<<PartNum;
  for(int i=0;i<PartNum;i++)
    Parts[i]->Save(slot);  
    }

void TexObject::Load(OptSlot &slot)
	{
	Release();
	slot>>PartNum;
	for(int i=0;i<PartNum;i++)
		{
		Parts[i]=new STObject;
    Parts[i]->Load(slot);  
		}
  }
TexObject &TexObject::operator =(const TexObject &a)
	{
	if (this==&a) return *this;
	Release();
	PartNum=a.PartNum;
	for(int i=0;i<PartNum;++i)
		{
		Parts[i]=new STObject;
		*Parts[i]=*a.Parts[i];
		}
	return *this;
	}
