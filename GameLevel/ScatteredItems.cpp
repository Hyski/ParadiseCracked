/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Класс для поддержки графического представления
               разбросанных по уровню предметов
     Author: Grom 
   Creation: 20 декабря 2000
***************************************************************/                
#include "precomp.h"
#include "ScatteredItems.h"
#include "../skin/animalibrary.h"
#include "../common/graphpipe/graphpipe.h"
#include "../skin/skanim.h"
#include "../iworld.h"
#include "../common/utils/utils.h"
#include "gamelevel.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static const std::string ITEMSDIR="Animations/skins/items/";
static const std::string ITEMSSKEL="Animations/anims/items/item.skel";
ScatteredItem::SkinCont  ScatteredItem::Skins;
AnimaData ScatteredItem::Skel;
SkAnim* ScatteredItem::Anim;
static const float height_diap=0.1;
BBox ScatteredItem::GetLocalBB()
  {

  static D3DMATRIX World;
	static bool init=false;
	if(!init)
		{
		D3DUtil_SetIdentityMatrix(World);
		init=true;
		}

  Item->ValidateLinks(Anim);
  Skel=Anim->Start(0);
  Skel.LastState.m_SkelState[0].World=World;
  Item->Update(&Skel.LastState);
  return Item->GetBBox();
  }

ScatteredItem::ScatteredItem(const point3 &pos,const std::string Name)
  {
  AnimaLibrary *Lib=AnimaLibrary::GetInst();
  if(!Anim)
    {
    Anim=Lib->GetSkAnimation(ITEMSSKEL);
    Skel=Anim->Start(0);
    }
  
  if(Skins.find(Name)==Skins.end())
    {
    Skins[Name]=Lib->GetSkSkin(ITEMSDIR+Name+".skin");
    if(!Skins[Name])
      {
      //Skins[Name]=Lib->GetSkSkin(ITEMSDIR+"special/electr_key"+".skin");
      //Skins[Name]=Lib->GetSkin(ITEMSDIR+"equipment/advanced_shield"+".skin");
      Skins[Name]=Lib->GetSkSkin(ITEMSDIR+"What_is"+".skin");
      }
    }
  if(Skins[Name])
    {
		Item=Skins[Name];
    BBox b=GetLocalBB();
    Rad=(b.maxx+b.minx)*0.5;
    }
  
  Pos=pos;
  Pos.z=IWorld::Get()->GetLevel()->LevelGrid->Height(Pos)+height_diap;
  Phase=frand()*100;
  Vel=frand()+1;
  this->ItName=Name;
  }

ScatteredItem::~ScatteredItem()
  {
  }
void ScatteredItem::Clear()
  {
  SkinCont::iterator it=Skins.begin(),ite=Skins.end();
  for(;it!=ite;it++)
    delete it->second;
  Skins.clear();
  }

void ScatteredItem::Draw(float Time)
  {
  if(!Item) return;
  GraphPipe *Pipe=IWorld::Get()->GetPipe();
  
  float ltime=Time*Vel+Phase;
  point3 offs=point3(cos(ltime)*Rad,sin(ltime)*Rad,-height_diap*sin(ltime/3)*sin(ltime/3));
  float cosf(cos(ltime)),sinf(sin(ltime));
	/*D3DMATRIX t1,t2,t3,t4;
	D3DUtil_SetTranslateMatrix(t1,-Rad,0,0);
	D3DUtil_SetRotateZMatrix(t2,ltime);
	D3DUtil_SetTranslateMatrix(t3,Pos.x,Pos.y,Pos.z);
	D3DMath_MatrixMultiply(t4,t1,t2);
	D3DMath_MatrixMultiply(t1,t4,t3);*/


	 D3DMATRIX mat(
cosf, sinf, 0, 0, 
-sinf, cosf, 0, 0, 
0, 0, 1, 0, 
-Rad*cosf+Pos.x, -Rad*sinf+Pos.y, offs.z+Pos.z, 1
);


/*
  D3DMATRIX mat(
    cosf, -sinf, 0, 0, 
    (sinf), cosf, 0, 0, 
    0, 0, 1, 0, 
    offs.x*cosf+offs.y*(sinf)+Pos.x, offs.x*(-sinf)+offs.y*cosf+Pos.y, offs.z+Pos.z, 1
    );*/
  Skel.LastState.m_SkelState[0].World=mat;
//  Skel.LastState.m_SkelState[0].World=t1;
  Item->Update(&Skel.LastState);
  Pipe->Chop(Item->GetMesh());
  }
