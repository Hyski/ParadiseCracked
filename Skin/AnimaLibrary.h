// AnimaLibrary.h: interface for the AnimaLibrary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMALIBRARY_H__2D021F0A_8A4E_45F5_9BD5_E00AE8AD1D95__INCLUDED_)
#define AFX_ANIMALIBRARY_H__2D021F0A_8A4E_45F5_9BD5_E00AE8AD1D95__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../common/datamgr/txtfile.h"

class Skeleton;
class SkAnim;
class SkSkin;
class Skin;

class AnimaLibrary  
  {
  typedef std::map<std::string,Skeleton*> AnimaStorage;
  typedef std::map<std::string,SkAnim*> SkAnimaStorage;
  typedef std::map<std::string,SkSkin*> SkSkinsStorage;
  typedef std::string namestring;
  typedef std::map<std::string,TxtFile> XlsStorage;
  protected:
    static AnimaLibrary Inst;
    AnimaStorage Animas;  //сюда загружаютс€ анимации дл€ совместного использовани€
    SkAnimaStorage SkAnimas;  //сюда загружаютс€ анимации дл€ совместного использовани€
    SkSkinsStorage SkSkins;  
    XlsStorage Tables;    //кеш дл€ .xls файлов
  public:
    Skeleton *GetAnimation(const namestring &Name);
    SkAnim *GetSkAnimation(const namestring &Name);
    Skin *GetSkin(const namestring &Name);
    SkSkin *GetSkSkin(const namestring &Name);
    TxtFile *GetTable(const std::string& TableName);
    void Clear();
  private:
    AnimaLibrary();
  public:
    virtual ~AnimaLibrary();
  public:
    static AnimaLibrary* GetInst(){return &Inst;};
  };


#endif // !defined(AFX_ANIMALIBRARY_H__2D021F0A_8A4E_45F5_9BD5_E00AE8AD1D95__INCLUDED_)
