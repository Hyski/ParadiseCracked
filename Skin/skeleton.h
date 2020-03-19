#ifndef _Future_Skeleton
#define _Future_Skeleton
#pragma warning(disable:4786)
#include "bone.h"
#include "../common/tree.h"

class Skeleton
  { 
  public:
    int BonesNum;
    Bone *Body; 
    std::map<std::string,Bone*> NamesMap;
    void ParseTree(Tree<Bone> *Parent);
    point3 *Link1,*Link2;//—редн€€ точка между этими двум€ - точка смещени€ скелета
    std::string Link1Name,Link2Name;
  public:
    float MaxTime; 
    bool Cycled;
  public:
	  void FillTransAngles(const Skeleton &From);
    std::map<std::string,Bone*>::const_iterator GetBones(){return NamesMap.begin();};
    std::map<std::string,Bone*>::const_iterator BonesEnd(){return NamesMap.end();};
	  void PrepareForSmooth(Transformation *t,Transformation *w);
    Transformation World;
	  void SetWorld(Transformation *t);
	  void SetLinks(std::string LinkA, std::string LinkB);
	  point3 GetTranslation()
      {
      if(Link1&&Link2)
        {
        return (*Link1+*Link2)/2.f;
        }
      return point3(0,0,0);
      }
	  void Translate(const point3 &vec);
	  ScelTime GetLastTime();
    Skeleton():Cycled(false){Body=NULL;BonesNum=0;Link1=Link2=NULL;}
    Skeleton(Tree<Bone> *tree);
    Skeleton(const Skeleton &a);
    ~Skeleton(){FREE(Body);}
    int Load(FILE *f);
    int Load(const std::string &Name);
    void Save(FILE *f);
    int LoadBin(VFile *f);
    void SaveBin(FILE *f);
    void UpdateOnTime(ScelTime Time, float delay=0, Skeleton *Ancestor=NULL);
    void CalcTransform();
    Bone *FindBone(std::string Name);

    
  };

#endif