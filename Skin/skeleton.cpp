#include "precomp.h"
#include "skeleton.h"

Skeleton::Skeleton(Tree<Bone> *tree):Cycled(false)
  {
  Link1=Link2=NULL;
  ParseTree(tree);
    MaxTime=GetLastTime();
    Body=NULL;
  }
Skeleton::Skeleton(const Skeleton &a):Cycled(false)
  {
  Link1=Link2=NULL;
  Body=new Bone[a.BonesNum];
  BonesNum=a.BonesNum;
  Bone *t=a.Body;
  Bone *t1=Body;
  NamesMap.clear();
  while(t1<Body+BonesNum)
    {
    *t1=*t;
    NamesMap[t1->GetName()]=t1;
    if(t->Parent)
      t1->Parent=FindBone(t->Parent->GetName());
    else t1->Parent=NULL;
    t1++;
    t++;
    }
  World=a.World;
    MaxTime=GetLastTime();
    if(a.Link1&&a.Link2)
      {
      Link1=&(FindBone(a.Link1Name)->Transform.trans);
      Link2=&(FindBone(a.Link2Name)->Transform.trans);
      }

  }
Bone *Skeleton::FindBone(std::string Name)
  {
  std::map<std::string,Bone *>::iterator f;
  f=NamesMap.find(Name);
  if(f==NamesMap.end()) return NULL;
  else
  return f->second;
  }
void Skeleton::ParseTree(Tree<Bone> *Parent)
  {
  Body[BonesNum]=Parent->Data;
  if(Parent->Parent)   Body[BonesNum].Parent=FindBone(Parent->Parent->Data.GetName());
  else                 Body[BonesNum].Parent=NULL;
  Body[BonesNum].ChildNum=Parent->GetChildsNum();

  NamesMap[Body[BonesNum].GetName()]=Body+BonesNum;
  BonesNum++;
  
  for(int i=0;i<Parent->GetChildsNum();i++)
    {
    ParseTree(Parent->childs[i]);
    }
  }

int Skeleton::Load(FILE *f)
  {
  Tree<Bone> *tree;
  tree= new Tree<Bone>;
  tree->Load(f);
  int size=0;
  tree->CalcSize(&size);

  FREE(Body);
  Body=new Bone[size];
  ParseTree(tree);
  delete tree;
    MaxTime=GetLastTime();
  return 1;
  }
int Skeleton::Load(const std::string &Name)
  {
  FILE *f=fopen(Name.c_str(),"rb");
  if(!f) return 0;
	
	//преобразованиев бинарный формат
	char str[13];
	fread(str,1,12,f);
	str[10]=0;
	rewind(f);
	if(strcmp(str,"Bone Name:"))
		{
		VFile inp(f);
		fclose(f);
		return LoadBin(&inp);
	 }

  Tree<Bone> *tree;
  tree= new Tree<Bone>;
  tree->Load(f);
  int size=0;
  tree->CalcSize(&size);
  FREE(Body);
  Body=new Bone[size];
  ParseTree(tree);
  delete tree;

  fclose(f);
  MaxTime=GetLastTime();
  f=fopen(Name.c_str(),"w+b");
	SaveBin(f);
	fclose(f);
  return 1;
  }

void Skeleton::Save(FILE *f)
  {
  for(int i=0;i<BonesNum;i++)
    {
    Body[i].Save(f);
    fprintf(f,"Children:%d\n",Body[i].ChildNum);
    }
  }
void Skeleton::SaveBin(FILE *f)
  {
  for(int i=0;i<BonesNum;i++)
    {
    Body[i].SaveBin(f);
    fwrite(&Body[i].ChildNum,1,sizeof(Body[i].ChildNum),f);
    }
  }
int Skeleton::LoadBin(VFile *f)
  {
  Tree<Bone> *tree;
  tree= new Tree<Bone>;
  tree->LoadBin(f);
  int size=0;
  tree->CalcSize(&size);

  FREE(Body);
  Body=new Bone[size];
  ParseTree(tree);
  delete tree;
  MaxTime=GetLastTime();
  return 1;
  }

void Skeleton::UpdateOnTime(ScelTime Time, float delay, Skeleton *Ancestor)
  {
  Bone *t=Body+BonesNum;
  //if(Time>MaxTime&&Cycled) Time=fmod(Time,MaxTime);
  if(Ancestor)
    {
    Bone *t1=Ancestor->Body+Ancestor->BonesNum;
    while(--t>=Body)
      {
      --t1;//!!!:Skelets must have same structure
      t->UpdateOnTime(Time,delay,t1);
      }
    }
  else
    {
    while(--t>=Body)
      {
      t->UpdateOnTime(Time);
      }
    }
  }
void Skeleton::CalcTransform()
  {
  Bone *t=Body,*u=Body+BonesNum;
  if(BonesNum)
    {
    t->CalcTransform(&World);
    //t->Transform.trans+=World.trans;
    //t->Transform.trans=Transform(World,t->Transform.trans);
    //t->Transform.rot=t->Transform.rot*World.rot;
    t++;
    }
  while(t<u)
    {
    t->CalcTransform();
   //if(t==Body) t->Transform.trans+=World.trans;
    t++;
    }
    //Bone *i =FindBone("Bip01 R Foot");
    //Bone *i1=FindBone("Bip01 L Foot");
    //point3 o=(i->Transform.trans+i1->Transform.trans)/2.0;o.z=0;
    //point3 p(Offset);
    //o=p-o;

//Body->Transform.trans+=Offset;
  }

ScelTime Skeleton::GetLastTime()
{
   Bone *t=Body+BonesNum;
   ScelTime time=0;
  while(--t>=Body)
    {
    ScelTime r=t->GetLastTime();
    if(time<r) time=r;
    }
   return time;
}

void Skeleton::Translate(const point3 &vec)
{
   Bone *t=Body;

   t->Translate+=vec;
/*   ScelTime time=0;
  while(--t>=Body)
    {
    ScelTime r=t->GetLastTime();
    if(time<r) time=r;
    }*/

}


void Skeleton::SetLinks(std::string LinkA, std::string LinkB)
{
Bone *i1=FindBone(LinkA);
Bone *i2=FindBone(LinkB);
if(i1&&i2)
  {
  Link1=&(i1->Transform.trans);
  Link2=&(i2->Transform.trans);
  Link1Name=LinkA;Link2Name=LinkB;
  }
}

void Skeleton::SetWorld(Transformation *t)
  {        //¬ызываетс€ дл€ новой анимации
 /* World.trans=point3(0,0,0);
  UpdateOnTime(0);
  CalcTransform();
  point3 p(GetTranslation());*/
  World=*t;
  /*p.z=0;
  World.trans-=p;*/
  }  

void Skeleton::PrepareForSmooth(Transformation *t,Transformation *w)
{//готовит старую анимацию дл€ сглаживани€ с новой
//float z=Body->Translate.z;
  point3 p(GetTranslation());
  point3 b(Body->Transform.trans);
  point3 e(w->trans-(p+t->trans));
  e.z=0;
  Body->Translate=b+e;
}

void Skeleton::FillTransAngles(const Skeleton &From)
  {
  Bone *t=Body+BonesNum;
  Bone *t1=From.Body+From.BonesNum;
  while(--t>=Body)
    {
    --t1;
    t->CurAngles=t1->CurAngles;
    t->Translate=t1->Translate;
    t->Transform=t1->Transform;
    }
  }
