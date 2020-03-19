// AnimaLibrary.cpp: implementation of the AnimaLibrary class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include "skeleton.h"
#include "skAnim.h"
#include "skin.h"
#include "AnimaLibrary.h"
#include  "../globals.h"
#include  "../common/utils/optslot.h"

AnimaLibrary AnimaLibrary::Inst;

AnimaLibrary::AnimaLibrary()
  {
  }

AnimaLibrary::~AnimaLibrary()
  {
  Clear();
  }

void AnimaLibrary::Clear()
  {
STACK_GUARD("AnimaLibrary::Clear");
  AnimaStorage::iterator it=Animas.begin();
  while(it!=Animas.end())
    {
    DESTROY(it->second);
    it++;
    }
  Animas.clear();
  SkAnimaStorage::iterator it1=SkAnimas.begin();
  while(it1!=SkAnimas.end())
    {
    DESTROY(it1->second);
    it1++;
    }
  SkAnimas.clear();
  SkSkinsStorage::iterator it2=SkSkins.begin();
  while(it2!=SkSkins.end())
    {
    DESTROY(it2->second);
    it2++;
    }
  SkSkins.clear();
  }
Skin *AnimaLibrary::GetSkin(const namestring &Name)
  {
STACK_GUARD("AnimaLibrary::GetSkin");
  Skin *s=new Skin;
	bool ret=s->Load(Name);
	if(!ret) {delete s;s=NULL;}
	return s;
  }
SkSkin *AnimaLibrary::GetSkSkin(const namestring &Name)
  {
STACK_GUARD("AnimaLibrary::GetSkSkin");

  SkSkinsStorage::iterator it;
  it=SkSkins.find(Name);
  if(it!=SkSkins.end())
    return (new SkSkin(*it->second));
		

  //1. Попробуем загрузить новый формат
  VFile *in=DataMgr::Load(Name.c_str());
  if(!in->Size()) 
    {
    DataMgr::Release(Name.c_str());
    return NULL;
    }
  char ID[5];
  in->Read(ID,5);
  if(!strcmp(ID,"SKIN"))
    {
    SkSkin *s=new SkSkin;
    OptSlot data;
    data.Load(in);
    s->Load(data);
    DataMgr::Release(Name.c_str());

		SkSkins[Name]=new SkSkin(*s);
    return s;
    }
  else
    {
    //2. попробуем загрузить старый формат
    DataMgr::Release(Name.c_str());
    Skin *s=new Skin;
    bool ret=s->Load(Name);
    if(!ret) {delete s;s=NULL;return NULL;}
    
    SkSkin *a=new SkSkin(s);
    delete s;
    //3. запишем в новом формате
    FILE *f=fopen(Name.c_str(),"w+b");
    if(f)
      {
      char ID[5]="SKIN";
      fwrite(ID,1,5,f);
      OptSlot data;
      a->Save(data);
      data.Save(f);
      fclose(f);
      }
    
		SkSkins[Name]=new SkSkin(*a);
    return a;
    }
  }
Skeleton *AnimaLibrary::GetAnimation(const namestring &Name)
  {
STACK_GUARD("AnimaLibrary::GetAnimation");
  AnimaStorage::iterator it;
  it=Animas.find(Name);
  if(it!=Animas.end())
    return it->second;
  else
    {
    Skeleton *t=new Skeleton();
    if(t->Load(Name))
      {
      Animas[Name]=t;
      t->SetLinks("Bip01 R Foot","Bip01 L Foot");
      if((!t->Link1||!t->Link2)&&t->BonesNum)
        t->SetLinks(t->Body[0].Name,t->Body[0].Name);
      //установим привязку к нулю
      Quaternion q;
      Transformation World;
        q.FromAngleAxis(0,NULLVEC);
        q.ToRotationMatrix(World.rot);
        World.trans=NULLVEC;
      t->SetWorld(&World);
      
      return t;
      }
    else
      delete t;
     return NULL;
    }
  }
SkAnim *AnimaLibrary::GetSkAnimation(const namestring &Name)
  {
STACK_GUARD("AnimaLibrary::GetSkAnimation");
  //logFile["failed_skels.log"]("чтение анимации: '%s'\n",Name.c_str()); 
  SkAnimaStorage::iterator it;
  it=SkAnimas.find(Name);
  if(it!=SkAnimas.end())
    return it->second;
  else
    {
	  //1. попробуем найти на диске новый формат
	  VFile *in=DataMgr::Load(Name.c_str());
	  if(!in) return NULL;
	  char ID[5];
	  in->Read(ID,5);
	  if(!strcmp(ID,"SKEL"))
	  {
        SkAnim *tt=new SkAnim;
        OptSlot data;
        data.Load(in);
        tt->Load(data);
        SkAnimas[Name]=tt;
        DataMgr::Release(Name.c_str());
        return tt;
	  }
	  else
	  {
		  //2. если нового формата нет, найдем старый и запишем в новом формате.
		  DataMgr::Release(Name.c_str());
      
      Skeleton *t=new Skeleton();
      if(t->Load(Name))
        {
        Animas[Name]=t;
        t->SetLinks("Bip01 R Foot","Bip01 L Foot");
        if((!t->Link1||!t->Link2)&&t->BonesNum)
          t->SetLinks(t->Body[0].Name,t->Body[0].Name);
        
        //установим привязку к нулю
        Quaternion q;
        Transformation World;
        q.FromAngleAxis(0,NULLVEC);
        q.ToRotationMatrix(World.rot);
        World.trans=NULLVEC;
        t->SetWorld(&World);
        SkAnim *tt=new SkAnim(t);
        
        //осуществить запись на диск нового формата
        FILE *f=fopen(Name.c_str(),"w+b");
        if(f)
          {
          char ID[5]="SKEL";
          fwrite(ID,1,5,f);
          OptSlot data;
          tt->Save(data);
          data.Save(f);
          fclose(f);
          }
        SkAnimas[Name]=tt;
        return tt;

        }
      else
        delete t;
      }
    
    //if(Name.size()) logFile["failed_skels.log"]("ошибка при загрузке файла: '%s'\n",Name.c_str()); 
    return NULL;
    }
  }

TxtFile *AnimaLibrary::GetTable(const std::string& TableName)
  {
STACK_GUARD("AnimaLibrary::GetTable");
  XlsStorage::iterator it;
  it=Tables.find(TableName);
  if(it!=Tables.end())
    return &it->second;
  else
    {
    Tables[TableName]=TxtFile(DataMgr::Load(TableName.c_str()));
    return &Tables[TableName];
    }
  }

