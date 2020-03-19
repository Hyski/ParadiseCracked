// Skin.cpp: implementation of the Skin class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include "Skin.h"
#include "../common/graphpipe/culling.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Skin::Skin()
{

}

Skin::~Skin()
{
}

void Skin::Save(FILE *f)
  {
  /*
  //fprintf(f,"Saving Skin...\n");
  fprintf(f,"Number of vertices:%d\n",PointsNum);
  for(int p=0;p<PointsNum;p++)
    {
     fprintf(f,"Number of links:%d\n",Cutis[p].LinksNum);
     for(int l=0;l<Cutis[p].LinksNum;l++)
       {
       SkinPoint::Link *ll=&Cutis[p].BoneLinks[l];
        fprintf(f,"Node:%s\n",ll->NodeName.c_str());
        fprintf(f,"Offset:(%f %f %f) ",ll->Offset.x,ll->Offset.y,ll->Offset.z);
        fprintf(f,"Weight:%f\n",ll->Weight);
       }
    }

    {
    fprintf(f,"Number of parts:%d\n",PartsNum);
    for(int p=0;p<PartsNum;p++)
      {
      fprintf(f,"Number of faces:%d\n",Parts[p].FacesNum);
      for(int l=0;l<Parts[p].FacesNum;l++)
        {
        aFace *ll=&Parts[p].Faces[l];
        fprintf(f,"Verts: %d %d %d\n",ll->p[0],ll->p[1],ll->p[2]);
        fprintf(f,"UVs:[%f %f][%f %f][%f %f]\n",ll->u[0],ll->v[0],ll->u[1],ll->v[1],ll->u[2],ll->v[2]);
        }
      }
    }
    */
  }
void Skin::SaveBin(FILE *f)
  {
#define _SAVE(a) fwrite(&a,1,sizeof(a),f);

  float Version=1.0;
  _SAVE(Version);
  _SAVE(PartNum);
  for(int p=0;p<PartNum;p++)
    {
    int i;
    _SAVE(Parts[p]->PntNum);
    SavePascalString(f,Parts[p]->MaterialName);
    _SAVE(Parts[p]->IdxNum);
    for(i=0;i<Parts[p]->PntNum;i++)
       _SAVE(Parts[p]->uv[i]);
    for(i=0;i<Parts[p]->IdxNum;i++)
       _SAVE(Parts[p]->GetIndexesRO()[i]);
    }
  _SAVE(LnkNum);
  for(int i=0;i<LnkNum;i++)
    {
     SavePascalString(f,Lnk[i].BoneName);
     _SAVE(Lnk[i].NumPoints);
     for(int j=0;j<Lnk[i].NumPoints;j++)
       {
       _SAVE(Lnk[i].Points[j].Offset);
       _SAVE(Lnk[i].Points[j].Weight);
       _SAVE(Lnk[i].Points[j].Norm);
       _SAVE(Lnk[i].Points[j].PartNum);
       _SAVE(Lnk[i].Points[j].PntNum);
       }
    }
#undef _SAVE// fread(&a,1,sizeof(a),f);
  }

bool Skin::Load(const std::string &Name)
{
  FILE *in=fopen(Name.c_str(),"rb");
  if(!in) return false;
		//преобразованиев бинарный формат
	char str[14];
	fread(str,1,13,in);
	str[13]=0;
	if(strcmp(str,"Skin version:"))
		{
		VFile inp(in);
		fclose(in);
		LoadBin(&inp);
		return true;
	 }

	rewind(in);
  Load(in);
  fclose(in);
	in=fopen(Name.c_str(),"w+b");
	if(in)
		{
		SaveBin(in);
		fclose(in);
	}
  return true;

}
void Skin::Load(FILE *f)
  {
  typedef std::vector<BoneToPnts::Pnt> pntvec;
  typedef std::map<std::string,pntvec> bonemap;
  //временное хранилище
  bonemap ourskin;
  
  float Version;
  fscanf(f,"Skin version:%f\n",&Version);
  fscanf(f,"PartsNumber:%d\n",&PartNum);

  for(int i=0;i<PartNum;i++)
  {
	  Parts[i]=new STObject;
	  fscanf(f,"Material:");
	  LoadString(f,&Parts[i]->MaterialName);
	  
	  fscanf(f,"Vertex number:%d\n",&Parts[i]->PntNum);
	  if(Parts[i]->PntNum)
      {
		  Parts[i]->uv=new texcoord[Parts[i]->PntNum];
		  Parts[i]->Normals=new point3[Parts[i]->PntNum];
		  Parts[i]->Points=new point3[Parts[i]->PntNum];
      }

    for(int v=0;v<Parts[i]->PntNum;v++)
      {
		  fscanf(f,"uv:%f %f\n",&Parts[i]->uv[v].u,&Parts[i]->uv[v].v);
		  int lnknum;
		  fscanf(f,"Link number:%d\n",&lnknum);
		  for(int l=0;l<lnknum;l++)
		  {
			  char t[200];
			  Link lnk,norm;
			  fscanf(f,"Bone:");
			  fgets(t,199,f);
			  while(t[strlen(t)-1]=='\n'||t[strlen(t)-1]=='\r')t[strlen(t)-1]=0;
			  //norm.NodeName=lnk.NodeName=t;
			  
			  
			  fscanf(f,"Offset:(%f %f %f) NormOffset:(%f %f %f) Weight:%f\n",
				  &lnk.Offset.x, &lnk.Offset.y, &lnk.Offset.z,
				  &norm.Offset.x, &norm.Offset.y, &norm.Offset.z,
				  &lnk.Weight);
			  norm.Weight=lnk.Weight;
			  
			  BoneToPnts::Pnt a;
			  a.PartNum=i;
			  a.PntNum=v;
			  a.Norm=false;
			  a.Offset=lnk.Offset;
			  a.Point=&Parts[i]->Points[v];
			  a.Weight=lnk.Weight;
			  ourskin[t].push_back(a);
			  a.Norm=true;
			  a.Offset=norm.Offset;
			  a.Point=&Parts[i]->Normals[v];
			  a.Weight=norm.Weight;
			  ourskin[t].push_back(a);
		  }
      }

	  fscanf(f,"Index number:%d\n",&Parts[i]->IdxNum);
	  if(Parts[i]->IdxNum)
      {
		  Parts[i]->indexes = new short int[Parts[i]->IdxNum];
      }
	  for(int id=0;id<Parts[i]->IdxNum;id++)
      {
		  int tmp;
		  fscanf(f,"%d ",&tmp);
		  Parts[i]->GetIndexesFull()[id] = tmp;
      }
	  fscanf(f,"\n");  
  } 

  bonemap::iterator it,ite;
  it=ourskin.begin();
  ite=ourskin.end();
  LnkNum=ourskin.size();
  for(int n=0;it!=ite;it++,n++)
    {
    Lnk[n].Alloc(it->second.size());
    Lnk[n].BoneName=it->first;
    for(int i=0;i<it->second.size();i++)
      {
      Lnk[n].Points[i]=it->second[i];
      }
    }
  }
void Skin::LoadBin(VFile *f)
  {
#define _LOAD(a) f->Read(&a,sizeof(a));

  float Version=1.0;
  _LOAD(Version);
  _LOAD(PartNum);
  for(int p=0;p<PartNum;p++)
    {
    Parts[p]=new STObject;
    _LOAD(Parts[p]->PntNum);
    int  s;
    char name[100];
    _LOAD(s);
    f->Read(name,s);
		name[s]=0;
    strlwr(name);
    Parts[p]->MaterialName=name;
    _LOAD(Parts[p]->IdxNum);

    if(Parts[p]->PntNum)
      {
      Parts[p]->uv=new texcoord[Parts[p]->PntNum];
      Parts[p]->Normals=new point3[Parts[p]->PntNum];
      Parts[p]->Points=new point3[Parts[p]->PntNum];
      }
    if(Parts[p]->IdxNum)
      Parts[p]->indexes=new short int[Parts[p]->IdxNum];
    int i;
    for(i=0;i<Parts[p]->PntNum;i++)
       _LOAD(Parts[p]->uv[i]);
    for(i=0;i<Parts[p]->IdxNum;i++)
      {
       _LOAD(Parts[p]->GetIndexesFull()[i]);
      }
    }

  _LOAD(LnkNum);
  for(int i=0;i<LnkNum;i++)
    {
    int numpoints;
    int  s;char name[100];
    _LOAD(s);  f->Read(name,s);  name[s]=0;/*strlwr(name);*/

     _LOAD(numpoints);
     Lnk[i].Alloc(numpoints);
		 Lnk[i].BoneName=name;
     for(int j=0;j<numpoints;j++)
       {
       _LOAD(Lnk[i].Points[j].Offset);
       _LOAD(Lnk[i].Points[j].Weight);
       _LOAD(Lnk[i].Points[j].Norm);
       _LOAD(Lnk[i].Points[j].PartNum);
       _LOAD(Lnk[i].Points[j].PntNum);
			 Lnk[i].Points[j].Point=&Parts[Lnk[i].Points[j].PartNum]->Points[Lnk[i].Points[j].PntNum];
       }
    }

#undef _LOAD
  }


void Skin::ApplyOn()
  {
  float x,y,z;
  for(int k=0;k<PartNum;k++)  //цикл по текстурным кускам
    {
    int PntNum;
    if(!(PntNum=Parts[k]->PntNum)) continue;
    point3 *CurNrm=Parts[k]->Normals,*CurPnt=Parts[k]->Points;
    _asm //очищаем массивы точек и нормалей нулями
      {
      emms
      xor eax,eax
      pxor mm0,mm0
      mov edi,CurNrm;
      mov esi,CurPnt;
      mov ecx,PntNum
      shr ecx,1 //одно нечетное поле
      jnc l1
      movq [edi],mm0
      mov [edi+8],eax
      movq [esi],mm0
      mov [esi+8],eax
      add edi,12
      add esi,12
   l1:
      movq [edi],mm0
      movq [edi+8],mm0
      movq [edi+16],mm0
      movq [esi],mm0
      movq [esi+8],mm0
      movq [esi+16],mm0
      add edi,24
      add esi,24
      dec ecx
      jnz l1
    emms
      }
    }
  Bound.Degenerate();
  BoneToPnts *CurLnk;
  for(CurLnk=Lnk+LnkNum-1;CurLnk>=Lnk;CurLnk--)
    {
    Transformation *Bone=&CurLnk->bone->Transform;

    Bound.Enlarge(Bone->trans);

    matrix3 &m=Bone->rot;
    BoneToPnts::Pnt *CurPnt=CurLnk->Points+CurLnk->NumPoints-1;
    for(;CurPnt>=CurLnk->Points;CurPnt--)
      {
      x=CurPnt->Offset.x;
      y=CurPnt->Offset.y;
      z=CurPnt->Offset.z;
      if(CurPnt->Norm)
        {
        /*
        CurPnt->Point->x+=CurPnt->Weight*(x*m[0][0]+y*m[1][0]+z*m[2][0]);
        CurPnt->Point->y+=CurPnt->Weight*(x*m[0][1]+y*m[1][1]+z*m[2][1]);
        CurPnt->Point->z+=CurPnt->Weight*(x*m[0][2]+y*m[1][2]+z*m[2][2]);
        */
        }
      else
        {
        CurPnt->Point->x+=CurPnt->Weight*(Bone->trans.x+x*m[0][0]+y*m[1][0]+z*m[2][0]);
        CurPnt->Point->y+=CurPnt->Weight*(Bone->trans.y+x*m[0][1]+y*m[1][1]+z*m[2][1]);
        CurPnt->Point->z+=CurPnt->Weight*(Bone->trans.z+x*m[0][2]+y*m[1][2]+z*m[2][2]);
        }
      }

    }
//Bound=GetBBox();
  }
void Skin::StickOn(Skeleton *Body)
  {
  for(int i=0;i<LnkNum;i++)
    {
         Bone *u=Body->FindBone(Lnk[i].BoneName);
         if(u)
           Lnk[i].bone=u;
         else
           {
           Lnk[i].bone=NULL;
           throw CASUS(Lnk[i].BoneName+"-Wrong Skeleton");
           }
    }
  }
bool Skin::TraceRay(const ray &r, float *Pos, point3 *Norm)
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
