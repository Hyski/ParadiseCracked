#ifndef _SKANIM
#define _SKANIM
/*********************************************************************\
* набор классов для скелетной анимации.                              * 
* author: Grom																											 *
\*********************************************************************/

#include "keyanimation.h"

class OptSlot;


class Skeleton;	 //fixme: Эти классы в релизе вообще не должны существовать!
class Skin;   	 //fixme: Эти классы в релизе вообще не должны существовать!


class SkAnim;
//мгновенное состояние скелета
class SkState
{
	friend class SkAnim;
	friend class SkSkin;
public:
	SkState():m_BoneNum(0),m_SkelState(NULL){};
	SkState(const SkState& a):m_BoneNum(0),m_SkelState(NULL){*this=a;};
	const D3DMATRIX *GetBone(int ID) const {return &m_SkelState[ID].World;};
	~SkState(){Alloc(0);};
	point3 GetNearestBone(const point3& where) const;

public: //утилитные функции
	SkState& operator= (const SkState &a);

private:
	struct BoneState
		{
		point3 Trans;
		Quaternion Orient;
		D3DMATRIX World;
	};

public:
		BoneState* m_SkelState;
		int m_BoneNum;

private:
	void Alloc(int bone_num);
};

//сама скелетная анимация
	struct AnimaData
		{
		float StartTime;   //Время старта анимации
		float Int_Time;    //время на интерполяцию со старой анимацией
		SkState LastState; //скелет для интерполяции
		float EndTime;
		};
class SkAnim
{
public:
public:
		const SkState& Get(const AnimaData *dta, float Time); //возвращает мгновенное состояние скелета в указанный момент
		AnimaData Start(float CurTime, float IntTime=0, const SkState* LastState=NULL); //возвращает время завершения анимации
		point3 GetOffset(const AnimaData *dta,float CurTime) const;//смещение скелета относительно нуля координат в указанный момент времени
		int GetBoneID(const std::string &BoneName)const;
		SkAnim():m_Skeleton(NULL),m_BoneNum(0)  {;};
		SkAnim(Skeleton *OldSkel);
		~SkAnim(){Alloc(0);};

public: //утилитные функции
	SkAnim& operator= (const SkAnim &a);
	bool Save(OptSlot &slot);	
	bool Load(OptSlot &slot);	
private:
		struct Bone
		{
			std::string Name;
			KeyAnimation Anima;
		};
		
	Bone* m_Skeleton;
	int m_BoneNum;

	SkState m_InstantState; //здесь будет возвращаться состояние
	KeyAnimation m_Offset;

	std::map<std::string,int> m_Bones2ID; //карта отражения имен костей на индексы

private://переменные, нужные в процессе анимации
	float m_Duration;    //длительность анимации

private: //утилитные функции
	void Alloc(int bone_num) 
	{
		if(m_Skeleton) delete[] m_Skeleton;
		if(bone_num)	m_Skeleton = new Bone[bone_num];
		else          m_Skeleton = NULL;
		m_BoneNum=bone_num;
	};
	
};



class SkBoneToPnts
{
friend class SkSkin;
  typedef point3* PntPtr;
  public://структуры данных
  	struct Pnt
		{
		PntPtr Point;
		point3 Offset;
		float  Weight;
		int PartNum;
		int PntNum;
		};
	public://утилитные функции
	bool Save(OptSlot &slot);	
	bool Load(OptSlot &slot);	
    SkBoneToPnts& operator=(const SkBoneToPnts &a)
		{
      Alloc(a.m_NumPoints);
      m_BoneName=a.m_BoneName;
      m_BoneID=a.m_BoneID;
      for(int i=0;i<m_NumPoints;i++)
        m_Points[i]=a.m_Points[i];
      return *this;
		}
	public:
    std::string m_BoneName;
    int m_NumPoints;
    int m_BoneID;
    Pnt *m_Points;
		
	public:
    SkBoneToPnts(){m_Points=NULL;m_BoneID=0;}
   ~SkBoneToPnts(){Alloc(0);}
	 protected:
		 void Alloc(int n)	//выделяет место под связи (если 0, то освобождение памяти)
			 {
			 m_NumPoints=0;DESTROY(m_Points);m_BoneID=0;  
			 if(n)
				 {
				 m_NumPoints=n;
				 m_Points=new Pnt[m_NumPoints];
				 memset(m_Points,0,sizeof(Pnt)*m_NumPoints);
				 }
			 }
		 
};




class TexObject;
//Кожа, которая ложится на скелет
class SkSkin
{
public:
	void Update(const SkState *Skel); //обновить состояние кожи
	void ValidateLinks(const SkAnim *Anim); //ставит в соответствие кости скелета и кожи
	SkSkin():m_Mesh(NULL),m_BonesNum(0){;};
	SkSkin(Skin *oldskin);
	SkSkin(const SkSkin &skin):m_Mesh(NULL),m_BonesNum(0){*this=skin;};
	~SkSkin();
public: //утилитные функции
	BBox GetBBox();
	const TexObject* GetMesh(){return m_Mesh;};
	SkSkin& operator= (const SkSkin& a);
	bool Save(OptSlot &slot);	
	bool Load(OptSlot &slot);	
private:
	TexObject *m_Mesh;
	SkBoneToPnts m_Links[256]; //внимание: ограничение количества костей
	int m_BonesNum;
};

inline SkState& SkState::operator= (const SkState &a)
{
	Alloc(a.m_BoneNum);
	for(int i=0;i<m_BoneNum;i++)
		m_SkelState[i]=a.m_SkelState[i];
	return *this;
}

inline SkAnim& SkAnim::operator= (const SkAnim &a)
{
	m_InstantState=a.m_InstantState;
	Alloc(a.m_BoneNum);
	for(int i=0;i<m_BoneNum;i++)
		m_Skeleton[i]=a.m_Skeleton[i];
	m_Duration=a.m_Duration;  //длительность анимации
	m_Offset=a.m_Offset;
	return *this;
}









#endif