// Bone.h: interface for the Bone class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_Bone_H__919A3421_B7A7_11D3_81EA_0060520A50A9__INCLUDED_)
#define AFX_Bone_H__919A3421_B7A7_11D3_81EA_0060520A50A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable:4786)
#ifdef COMPILEFORMAX
   #include <max.h>
   #include "../../sceneexport.h"
   #include "../3d/quat.h"
   #include "../3d/geometry.h"
#else
   #include "../common/3d/geometry.h"
   #include "../common/3d/quat.h"
   #include "keyanimation.h"
#endif



typedef float ScelTime;

class Skeleton;
class Bone  
  {
  public:   

    int UseHierarchy;
    std::string Name;
    KeyAnimation Keys;
    //std::map<ScelTime,point3> Trans;       //ключи для смещения объекта
    //std::map<ScelTime,Quaternion> Angles;  //ключи для поворота объекта
    Bone *Parent;
    int ChildNum;
    Quaternion CurAngles,WorldAngles;
    point3 Translate;

  public:
    Transformation Transform;
    friend class Skeleton;
    Bone();
    virtual ~Bone();
     
    const std::string &GetName() const {return Name;}
    //Интерполировать позицию для указанного времени
    void UpdateOnTime(ScelTime Time, float delay=0, Bone *Ancestor=NULL);
    //подсчитать окончательную трансформацию узла
    void CalcTransform(Transformation *World=NULL);
    //сконструировать узел исходя из анимации в 3D Max 3.0
#ifdef COMPILEFORMAX
    void ParseKeys(INode *node, SkeletonExporter *sc, bool First);
#endif
    //Записать на диск
    int Save(FILE *f);
    //Прочитать с диска
    int Load(FILE *f);
    //Записать на диск
    int SaveBin(FILE *f);
    //Прочитать с диска
    int LoadBin(VFile *f);
    //оптимизирует ключи, выбрасывая дублирующиеся
    void OptimizeKeys();
    ScelTime GetLastTime() const 
      {
      return Keys.GetLastTime();
      };
    Bone & operator =(const Bone &a)
      {
      UseHierarchy=a.UseHierarchy;
      Name=a.Name;
      Keys=a.Keys;
      //Trans=a.Trans;       //ключи для смещения объекта
      //Angles=a.Angles;  //ключи для поворота объекта
      CurAngles=a.CurAngles;
      WorldAngles=a.WorldAngles;
      Translate=a.Translate;
      Parent=NULL;
      Transform=a.Transform;
      
      return *this;
      }
  };
#endif // !defined(AFX_Bone_H__919A3421_B7A7_11D3_81EA_0060520A50A9__INCLUDED_)
