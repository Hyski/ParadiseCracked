#ifndef _SKANIM
#define _SKANIM
/*********************************************************************\
* ����� ������� ��� ��������� ��������.                              * 
* author: Grom																											 *
\*********************************************************************/

#include "keyanimation.h"

class OptSlot;


class Skeleton;	 //fixme: ��� ������ � ������ ������ �� ������ ������������!
class Skin;   	 //fixme: ��� ������ � ������ ������ �� ������ ������������!


class SkAnim;
//���������� ��������� �������
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

public: //��������� �������
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

//���� ��������� ��������
	struct AnimaData
		{
		float StartTime;   //����� ������ ��������
		float Int_Time;    //����� �� ������������ �� ������ ���������
		SkState LastState; //������ ��� ������������
		float EndTime;
		};
class SkAnim
{
public:
public:
		const SkState& Get(const AnimaData *dta, float Time); //���������� ���������� ��������� ������� � ��������� ������
		AnimaData Start(float CurTime, float IntTime=0, const SkState* LastState=NULL); //���������� ����� ���������� ��������
		point3 GetOffset(const AnimaData *dta,float CurTime) const;//�������� ������� ������������ ���� ��������� � ��������� ������ �������
		int GetBoneID(const std::string &BoneName)const;
		SkAnim():m_Skeleton(NULL),m_BoneNum(0)  {;};
		SkAnim(Skeleton *OldSkel);
		~SkAnim(){Alloc(0);};

public: //��������� �������
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

	SkState m_InstantState; //����� ����� ������������ ���������
	KeyAnimation m_Offset;

	std::map<std::string,int> m_Bones2ID; //����� ��������� ���� ������ �� �������

private://����������, ������ � �������� ��������
	float m_Duration;    //������������ ��������

private: //��������� �������
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
  public://��������� ������
  	struct Pnt
		{
		PntPtr Point;
		point3 Offset;
		float  Weight;
		int PartNum;
		int PntNum;
		};
	public://��������� �������
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
		 void Alloc(int n)	//�������� ����� ��� ����� (���� 0, �� ������������ ������)
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
//����, ������� ������� �� ������
class SkSkin
{
public:
	void Update(const SkState *Skel); //�������� ��������� ����
	void ValidateLinks(const SkAnim *Anim); //������ � ������������ ����� ������� � ����
	SkSkin():m_Mesh(NULL),m_BonesNum(0){;};
	SkSkin(Skin *oldskin);
	SkSkin(const SkSkin &skin):m_Mesh(NULL),m_BonesNum(0){*this=skin;};
	~SkSkin();
public: //��������� �������
	BBox GetBBox();
	const TexObject* GetMesh(){return m_Mesh;};
	SkSkin& operator= (const SkSkin& a);
	bool Save(OptSlot &slot);	
	bool Load(OptSlot &slot);	
private:
	TexObject *m_Mesh;
	SkBoneToPnts m_Links[256]; //��������: ����������� ���������� ������
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
	m_Duration=a.m_Duration;  //������������ ��������
	m_Offset=a.m_Offset;
	return *this;
}









#endif