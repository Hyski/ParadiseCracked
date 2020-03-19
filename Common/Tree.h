// Tree.h: interface for the Tree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TREE_H__D86F6FC1_B4AD_11D3_81EA_0060520A50A9__INCLUDED_)
#define AFX_TREE_H__D86F6FC1_B4AD_11D3_81EA_0060520A50A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>

template<class T> class Tree  
  {
  protected:
    enum{MAXCHILDS=100};
    int numchilds;
  public:
    Tree<T> **childs;
    Tree<T> *Parent;
    typedef Tree<T>* TreeP;
    T Data;
    Tree();
    Tree(T data);
    virtual ~Tree();

    virtual Tree<T> *NewNode(){return new Tree<T>;}

    void Trim() {while(numchilds)  delete childs[--numchilds];};
    int GetChildsNum()    {return numchilds;}
    void CalcSize(int *i)
      {
      (*i)++;
      for(int q=0;q<numchilds;q++)
        childs[q]->CalcSize(i);
      };
    void Show(FILE *f)
      {
      int i;
      Data(f);
      fprintf(f,"has %d Children.\n",numchilds);

      for(i=0;i<numchilds;i++) 
        childs[i]->Show(f);
      };
    void Save(FILE *f);
    void Load(FILE *f);
    void SaveBin(FILE *f);
    void LoadBin(VFile *f);
    void AddChild(Tree<T> *Child) 
      { 
      if(!Child) return ;
      if(numchilds<MAXCHILDS-1)
        {
        childs[numchilds++]=Child;
        Child->Parent=this;
        }
      }
  };

template<class T> Tree<T>::Tree()
{
   numchilds=0;
   childs=new TreeP[MAXCHILDS];
   Parent=NULL;
}
template<class T> Tree<T>::Tree(T data)
{
   Data=data;
   numchilds=0;
   childs=new (Tree<T>*)[MAXCHILDS];
   Parent=NULL;
}

template<class T> Tree<T>::~Tree()
{
   while(numchilds)
     {
     delete childs[--numchilds];
     }
   delete childs;
}
template<class T> void Tree<T>::Save(FILE *f)
      {
      int i;
      Data.Save(f);
      fprintf(f,"Children:%d\n",numchilds);

      for(i=0;i<numchilds;i++) 
        childs[i]->Save(f);
      }
template<class T> void Tree<T>::Load(FILE *f)
      {
      int i;
      Data.Load(f);
      fscanf(f,"Children:%d\n",&numchilds);

      for(i=0;i<numchilds;i++) 
        {
        childs[i]=NewNode();
        childs[i]->Parent=this;
        childs[i]->Load(f);
        }
      }
template<class T> void Tree<T>::SaveBin(FILE *f)
      {
      int i;
      Data.SaveBin(f);
      fwrite(&numchilds,1,sizeof(numchilds),f);

      for(i=0;i<numchilds;i++) 
        childs[i]->SaveBin(f);
      }
template<class T> void Tree<T>::LoadBin(VFile *f)
      {
      int i;
      Data.LoadBin(f);
      f->Read(&numchilds,sizeof(numchilds));

      for(i=0;i<numchilds;i++) 
        {
        childs[i]=NewNode();
        childs[i]->Parent=this;
        childs[i]->LoadBin(f);
        }
      }


#endif // !defined(AFX_TREE_H__D86F6FC1_B4AD_11D3_81EA_0060520A50A9__INCLUDED_)
