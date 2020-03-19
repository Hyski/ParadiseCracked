// BTree.h: interface for the BTree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BTREE_H__A7D24428_E44B_4A96_8376_D0C4BC94337D__INCLUDED_)
#define AFX_BTREE_H__A7D24428_E44B_4A96_8376_D0C4BC94337D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class BTree  
  {
  public:
    struct TriLM
      {
      float u[3],v[3]; //координаты на карте освещенности
      TriLM *Left,*Right; //потомки
      int tri; //индекс треугольника
      float square;
      TriLM(){Left=Right=NULL;}
      ~TriLM(){if(Left) delete Left;if(Right)delete Right;}
      };
    typedef std::multimap<float,int> TRISET;
  public:
    TriLM *Root; //Дерево сгенерированных карт освещенности
  public:
    BTree();
    void MakeTree(TRISET &tris,TriLM *Root);
    void DivideLMTree(TriLM *root,TriLM *at,TriLM *bt);

    virtual ~BTree();
        
  };

#endif // !defined(AFX_BTREE_H__A7D24428_E44B_4A96_8376_D0C4BC94337D__INCLUDED_)
