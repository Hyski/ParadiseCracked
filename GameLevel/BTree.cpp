// BTree.cpp: implementation of the BTree class.
//
//////////////////////////////////////////////////////////////////////
//#include "stdafx.h"
#include "precomp.h"
#include "BTree.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BTree::BTree()
  {
  Root=NULL;
  }

BTree::~BTree()
  {
  if(Root) delete Root;
  }
void BTree::MakeTree(TRISET &tris, TriLM *root)
  {
  TRISET::reverse_iterator it1,it=tris.rbegin(),e=tris.rend(),it2;
  if(!root)
    {
    Root=new TriLM;
    }
  if(tris.size()==1)
    {
     if(!root)
       {
       Root->tri=it->second;Root->square=it->first;
       Root->u[0]=0;Root->v[0]=0;
       Root->u[1]=1;Root->v[1]=0;
       Root->u[2]=1;Root->v[2]=1;
       }
     else 
       {root->tri=it->second;root->square=it->first;}
    }
  else
    {
    float Summ=0;
    for(;it!=e;it++)
      Summ+=it->first;
    
    float HalveSumm=0;
    for(it=tris.rbegin();HalveSumm+it->first<=Summ/2;)
      {
      HalveSumm+=it->first;
      it++;
      if(it!=e&&it->first/HalveSumm<0.5)break;
      }
    it2=e;it2--;
    for(;it2!=it&&HalveSumm+it2->first<=Summ/2;it2--)
      {
      HalveSumm+=it2->first;
      }
    it2++;
    //теперь it - первый элемент из второй половины списка
    TRISET a(tris.rbegin(),it),b(it,it2);
    if(it2!=e)
      a.insert(it2,e);
    TriLM *at,*bt;
    at=new TriLM;
    bt=new TriLM;
    if(root)
      {root->Left=at;root->Right=bt;}
    else
      {Root->Left=at;Root->Right=bt;}

    DivideLMTree(root,at,bt);//разобьем большой кусок карты освещенности на два равных
    MakeTree(a,at);
    MakeTree(b,bt);
    }
  }
void BTree::DivideLMTree(TriLM *root,TriLM *at,TriLM *bt)
  {
  if(!root)
    {//самое первое деление
    at->u[0]=0;at->v[0]=0;
    at->u[1]=1;at->v[1]=0;
    at->u[2]=1;at->v[2]=1;

    bt->u[0]=0;bt->v[0]=0;
    bt->u[1]=1;bt->v[1]=1;
    bt->u[2]=0;bt->v[2]=1;
    }
  else
    {//бьем треугольник пополам
    float l[3]={hypot(root->u[0]-root->u[1],root->v[0]-root->v[1]),
      hypot(root->u[2]-root->u[1],root->v[2]-root->v[1]),
      hypot(root->u[0]-root->u[2],root->v[0]-root->v[2])
      };
    int nh,nh1;
    if(l[0]>l[1]&&l[0]>l[2]) {nh=0;nh1=1;}
    if(l[1]>l[0]&&l[1]>l[2]) {nh=1;nh1=2;}
    if(l[2]>l[1]&&l[2]>l[0]) {nh=2;nh1=0;}
    
    float nu,nv;

    nu=(root->u[nh]+root->u[nh1])*0.5;
    nv=(root->v[nh]+root->v[nh1])*0.5;
    switch(nh)
      {
      case 0:
        at->u[0]=root->u[0];      at->v[0]=root->v[0];
        at->u[1]=nu;              at->v[1]=nv;
        at->u[2]=root->u[2];      at->v[2]=root->v[2];
        
        bt->u[0]=nu;              bt->v[0]=nv;
        bt->u[1]=root->u[1];      bt->v[1]=root->v[1];
        bt->u[2]=root->u[2];      bt->v[2]=root->v[2];
        break;
      case 1:
        at->u[0]=root->u[0];      at->v[0]=root->v[0];
        at->u[1]=root->u[1];      at->v[1]=root->v[1];
        at->u[2]=nu;              at->v[2]=nv;
        
        bt->u[0]=root->u[0];      bt->v[0]=root->v[0];
        bt->u[1]=nu;              bt->v[1]=nv;
        bt->u[2]=root->u[2];      bt->v[2]=root->v[2];
        break;
      case 2:
        at->u[0]=root->u[0];      at->v[0]=root->v[0];
        at->u[1]=root->u[1];      at->v[1]=root->v[1];
        at->u[2]=nu;              at->v[2]=nv;
        
        bt->u[0]=root->u[1];      bt->v[0]=root->v[1];
        bt->u[1]=root->u[2];      bt->v[1]=root->v[2];
        bt->u[2]=nu;              bt->v[2]=nv;
        break;
      }
    }
  }
