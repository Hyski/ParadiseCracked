// SimpleTexturedObject.h: interface for the SimpleTexturedObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLETEXTUREDOBJECT_H__6768B6C7_51FF_4B3E_8364_6EDFA0894213__INCLUDED_)
#define AFX_SIMPLETEXTUREDOBJECT_H__6768B6C7_51FF_4B3E_8364_6EDFA0894213__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class SavSlot;
class OptSlot;
class SimpleTexturedObject //Кусок кожи, имеющий один материал
  {
  private:
    void Init()
      {
      uv=NULL;
      indexes=NULL;
      Points=NULL;
      Normals=NULL;
      IdxNum=0;
      PntNum=0;
      };
  public:
     void FreeAll();
     void Alloc(int _PntNum, int _IdxNum)
       {
       Init();
       IdxNum=_IdxNum;
       PntNum=_PntNum;
       if(IdxNum)
         {
         indexes=new short int[IdxNum];
         }
       if(PntNum)
         {
         Points=new point3[PntNum];
         Normals=new point3[PntNum];
         uv=new texcoord[PntNum];
         }
       }
    std::string MaterialName;     //имя материала для этого куска
    
    int PntNum;             //количество вершин/нормалей/... в этом куске
    int IdxNum;             //количество индексов
    point3 *Points;
    point3 *Normals;
    texcoord *uv;
	// 
  //private:
    short int *indexes;     //индекы, создающие грани
  public:
	// создадим функции для доступа к индексам - на чтение и на чтение/запись
	short int * GetIndexesFull();
	const short int * const GetIndexesRO();
	typedef short int * short_int_ptr;


    void Load(FILE *f);
    void Save(FILE *f);
    bool MakeSaveLoad(SavSlot &slot);
    void Load(OptSlot &slot);
    void Save(OptSlot &slot);

    SimpleTexturedObject &operator=(const SimpleTexturedObject &a)
      {
      if (this==&a) return *this;
      FreeAll();
      MaterialName=a.MaterialName;

      Alloc(a.PntNum,a.IdxNum);
      if(IdxNum)
        {
        memcpy(indexes,a.indexes,sizeof(short int)*IdxNum);
        }
      if(PntNum)
        {
        memcpy(Points,a.Points,sizeof(point3)*PntNum);
        memcpy(Normals,a.Normals,sizeof(point3)*PntNum);
        memcpy(uv,a.uv,sizeof(texcoord)*PntNum);
        }
      return *this;
      }
    virtual ~SimpleTexturedObject()
      {
      FreeAll();
      }
      SimpleTexturedObject()
      {
      Init();
      }
      SimpleTexturedObject(const SimpleTexturedObject &a)
      {
      Init();
      *this=a;
      }

  };
class TexObject
  {
  protected:
    typedef SimpleTexturedObject STObject;
  public:
    STObject *Parts[500];//максимум 500 кусков у объектов
    int PartNum;
  public:
    TexObject():PartNum(0)
      {}
    virtual ~TexObject()
	{
		Release();
	}
    void Load(FILE *f);
    void Save(FILE *f);
    bool MakeSaveLoad(SavSlot &slot);
    void Load(OptSlot &slot);
    void Save(OptSlot &slot);
    virtual bool TraceRay(const ray &r, float *Pos, point3 *Norm) const;
    //освобождение занимаемой памяти
    void Release()
      {
      //FIXME:
      for(int i=0;i<PartNum;i++) DESTROY(Parts[i]);
      PartNum=0;        
      }
    TexObject &operator =(const TexObject &a);

	// by Flif !!!
		TexObject(const TexObject &a):PartNum(0) { *this = a; }
    virtual BBox GetBBox();
  };


#endif // !defined(AFX_SIMPLETEXTUREDOBJECT_H__6768B6C7_51FF_4B3E_8364_6EDFA0894213__INCLUDED_)
