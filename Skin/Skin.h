// Skin.h: interface for the Skin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKIN_H__C0F85181_BBCF_11D3_81EA_0060520A50A9__INCLUDED_)
#define AFX_SKIN_H__C0F85181_BBCF_11D3_81EA_0060520A50A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "skeleton.h"
#include "../common/graphpipe/simpletexturedobject.h"
 struct Link
   {
    std::string NodeName;      //им€ кости
    point3 Offset;        //смещение в координатах кости
    float Weight;         //вес кости
    Transformation *Bone; //заполн€етс€ при приклеивании кожи к скелету
    Link(){Bone=NULL;}
     Link &operator=(const Link &a)
       {
       NodeName=a.NodeName;
       Offset=a.Offset;
       Weight=a.Weight;
       Bone=a.Bone;
       return *this;
       }
   };  
 inline bool operator <(const Link &a,const Link &l)
      {    
      if(a.NodeName<l.NodeName) return true;
      else  if(l.NodeName<a.NodeName) return false;

      if(a.Offset.x<l.Offset.x) return true;
      else if(l.Offset.x<a.Offset.x) return false;
      if(a.Offset.y<l.Offset.y) return true;
      else if(l.Offset.y<a.Offset.y) return false;
      if(a.Offset.z<l.Offset.z) return true;
      else if(l.Offset.z<a.Offset.z) return false;

      if(a.Weight<l.Weight) return true;
      else return false;
      }
class BoneToPnts
  {
  typedef point3* PntPtr;
  public:
    int NumPoints;
    struct Pnt
      {
      PntPtr Point;
      point3 Offset;
      float  Weight;
      bool Norm;
      int PartNum;
      int PntNum;
      };
    std::string BoneName;
    Bone *bone;
    Pnt *Points;
    BoneToPnts(){Points=NULL;bone=NULL;}
    ~BoneToPnts(){Free();}
    void Free(){NumPoints=0;DESTROY(Points);bone=NULL;}
    void Alloc(int n)
      {
      Free();
      NumPoints=n;
      Points=new Pnt[NumPoints];
      memset(Points,0,sizeof(Pnt)*NumPoints);
      }
    BoneToPnts& operator=(const BoneToPnts &a)
      {
      Free();
      BoneName=a.BoneName;
      bone=a.bone;
      NumPoints=a.NumPoints;
      Alloc(NumPoints);
      for(int i=0;i<NumPoints;i++)
        {
        Points[i]=a.Points[i];
        //FIXME: необходимо скорректировать адреса точек
        }
      return *this;
      }
  };

class Skin:public TexObject  
  {
  public:
    BBox Bound;
    BoneToPnts Lnk[237];
    int LnkNum;
  public:      
    Skin(); 
    virtual ~Skin();
    void ApplyOn();
    void StickOn(Skeleton *Body);
    void Save(FILE *f);
    void SaveBin(FILE *f);
    void Load(FILE *f);
    bool Load(const std::string &Name);
    void LoadBin(VFile *f);
    virtual bool TraceRay(const ray &r, float *Pos, point3 *Norm);

  };

#endif // !defined(AFX_SKIN_H__C0F85181_BBCF_11D3_81EA_0060520A50A9__INCLUDED_)
