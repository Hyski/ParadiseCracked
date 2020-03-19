#include "precomp.h"
#include "skanim.h"
#include "skeleton.h"
#include "skin.h"
#include "../common/graphpipe/simpletexturedobject.h"
#include "../common/utils/d3dutil.h"
#include "../common/utils/optslot.h"
#include "../common/utils/utils.h"
#include "../common/utils/profiler.h"

/***********************************************************\
|*          реализация класса SkState	      							 *|
\***********************************************************/
static const int SKVERSION=0x8001;
void SkState::Alloc(int bone_num) 
	{
	if(m_BoneNum==bone_num) return;
		if(m_SkelState) delete m_SkelState;
		if(bone_num)	m_SkelState = new BoneState[bone_num];
		else          m_SkelState = NULL;

		m_BoneNum=bone_num;
		for(int i=0;i<m_BoneNum;i++)
			{
			D3DUtil_SetIdentityMatrix(m_SkelState[i].World);
			}
	};
/***********************************************************\
|*          реализация класса SkAnim	      							 *|
\***********************************************************/
float SkSpeed=2;
point3 SkAnim::GetOffset(const AnimaData *dta,float CurTime) const //смещение скелета относительно нуля координат в указанный момент времени
	{
STACK_GUARD("SkAnim::GetOffset");
	point3 t; 
	m_Offset.GetTrans(&t,SkSpeed*(CurTime-dta->StartTime)); 
	return t;
	}

const SkState& SkAnim::Get(const AnimaData *dta, float Time) //возвращает мгновенное состояние скелета в указанный момент
{
STACK_GUARD("SkAnim::Get");
//CodeProfiler CP("skel.get");
	float time=SkSpeed*(Time-dta->StartTime);
  if(time<0.f) time=0.f;   //fixme:?
	for(int i=0;i<m_BoneNum;i++)
		{
  	SkState::BoneState &bone_st=m_InstantState.m_SkelState[i];
		m_Skeleton[i].Anima.GetAngle(&bone_st.Orient,time);
		m_Skeleton[i].Anima.GetTrans(&bone_st.Trans,time);

		if(time<dta->Int_Time)
			{
			if(dta->LastState.m_BoneNum<m_InstantState.m_BoneNum)
				{
				throw CASUS ("error Bones number!!!!");
				}
    	SkState::BoneState &last_bone=dta->LastState.m_SkelState[i];
			float alpha=time/dta->Int_Time;
			bone_st.Orient=Quaternion::Slerp(alpha, last_bone.Orient, bone_st.Orient);
			Interpolate(&bone_st.Trans, bone_st.Trans, last_bone.Trans, alpha);
			}
		bone_st.Orient.ToRotationMatrix(bone_st.World.m);
		*(point3 *)&bone_st.World._41=bone_st.Trans;
	}
	return m_InstantState;
}

//=====================================================================================//
//                        point3 SkState::GetNearestBone() const                        //
//=====================================================================================//
point3 SkState::GetNearestBone(const point3& where) const
{
	int best =-1;
	float bestfunc=0;
	for(int i=0;i<m_BoneNum;i++)
	{
		point3 dir = m_SkelState[i].Trans-where;
		float dist = dir.Length();
		dir = Normalize(dir);
		point3 flatdir = dir;flatdir.z=0;
		flatdir = Normalize(flatdir);
		float angle = flatdir.Dot(dir);
		float func = -dist-angle;
		if(best==-1 || func>bestfunc)
		{
			best = i;
			bestfunc=func;
		}

	}
	if(best==-1) return where;
	else		 return m_SkelState[best].Trans;
}
SkAnim::SkAnim(Skeleton *OldSkel):m_Skeleton(NULL),m_BoneNum(0)
  {
STACK_GUARD("SkAnim::SkAnim");
	int i;
	Alloc(OldSkel->BonesNum);
	m_InstantState.Alloc(m_BoneNum);
  
  point3 BaseOffs;
  std::set<float> times;
  std::set<float>::iterator times_it;
  
  for(i=0;i<m_BoneNum;i++)
		OldSkel->Body[i].Keys.GetKeyTimes(&times);
	float error=1000,ot=0;
  for(times_it=times.begin();times_it!=times.end();		ot = *times_it,times_it++)
		{   
		error +=*times_it-ot;
		if(error<0.1) continue;
		error=0;
		OldSkel->UpdateOnTime(*times_it);
		OldSkel->CalcTransform();
		point3 tr=OldSkel->GetTranslation();
		tr.z=0;
		if(times_it==times.begin()) BaseOffs=tr;
		m_Offset.SetKey(*times_it,Quaternion::IDENTITY,tr-BaseOffs);
		for(i=0;i<m_BoneNum;i++)
			{
			m_Skeleton[i].Anima.SetKey(*times_it,OldSkel->Body[i].WorldAngles,OldSkel->Body[i].Transform.trans-tr);
			}
		}
  for(i=0;i<m_BoneNum;i++)
		{
		m_Skeleton[i].Anima.Decimate(0.005);
		m_Skeleton[i].Name=OldSkel->Body[i].GetName();
		m_Bones2ID[m_Skeleton[i].Name]=i;
		}
  m_Duration=*times.rbegin();
	}

AnimaData SkAnim::Start(float CurTime, float IntTime, const SkState* LastState) //возвращает время завершения анимации
	{
STACK_GUARD("SkAnim::Start");
	AnimaData dta;
	dta.StartTime=CurTime;
	dta.EndTime=CurTime+m_Duration/SkSpeed;
	dta.Int_Time=IntTime;
	if(LastState) dta.LastState=*LastState;
	else dta.LastState.Alloc(m_BoneNum);
	return dta;
	}
int SkAnim::GetBoneID(const std::string &BoneName)const
	{
STACK_GUARD("SkAnim::GetBoneID");
	std::map<std::string, int>::const_iterator it;
	it=m_Bones2ID.find(BoneName);
	if(it==m_Bones2ID.end()) return -1;
	return it->second;
	}

bool SkAnim::Save(OptSlot &slot)
		{
STACK_GUARD("SkAnim::Save");
    slot<<(int)SKVERSION;
    slot<<m_BoneNum;
    m_Offset.Save(slot);
    
    for(int i=0;i<m_BoneNum;i++)
      {
      slot<<m_Skeleton[i].Name;
      m_Skeleton[i].Anima.Save(slot);
      }
    return true;
		}
bool SkAnim::Load(OptSlot &slot)
		{
STACK_GUARD("SkAnim::Load");
    int ver;
    slot>>ver;
    if(ver!=SKVERSION) return false;
    slot>>m_BoneNum;
    Alloc(m_BoneNum);
    m_InstantState.Alloc(m_BoneNum);
    
    m_Offset.Load(slot);
    m_Bones2ID.clear();
    for(int i=0;i<m_BoneNum;i++)
      {
      slot>>m_Skeleton[i].Name;
      m_Skeleton[i].Anima.Load(slot);
      m_Bones2ID[m_Skeleton[i].Name]=i;
      }
    
    m_Duration=0;
    for(int i=0;i<m_BoneNum;i++)
      m_Duration=(std::max)(m_Skeleton[i].Anima.GetLastTime(),m_Duration);
    return true;
		}

/***********************************************************\
|*          реализация класса SkSkin	      							 *|
\***********************************************************/
SkSkin::SkSkin(Skin *oldskin):m_Mesh(NULL),m_BonesNum(0)
	{
STACK_GUARD("SkSkin::SkSkin");
	m_Mesh=new TexObject;
	*m_Mesh=*static_cast<TexObject*>(oldskin);

	 m_BonesNum=oldskin->LnkNum;
  for(int i=0;i<m_BonesNum;i++)
    {
		m_Links[i].Alloc(oldskin->Lnk[i].NumPoints);
		 m_Links[i].m_BoneName=oldskin->Lnk[i].BoneName;
		 int rn=0;
     for(int j=0;j<m_Links[i].m_NumPoints;j++)
       {
			 if(oldskin->Lnk[i].Points[j].Norm) continue;
       m_Links[i].m_Points[rn].Offset=oldskin->Lnk[i].Points[j].Offset;
       m_Links[i].m_Points[rn].Weight=oldskin->Lnk[i].Points[j].Weight;
       m_Links[i].m_Points[rn].PartNum=oldskin->Lnk[i].Points[j].PartNum;
       m_Links[i].m_Points[rn].PntNum=oldskin->Lnk[i].Points[j].PntNum;
			 m_Links[i].m_Points[rn].Point=&m_Mesh->Parts[m_Links[i].m_Points[rn].PartNum]->
				                      Points[m_Links[i].m_Points[rn].PntNum];
			 rn++;
       }
		 m_Links[i].m_NumPoints=rn;
    }
	};

SkSkin::~SkSkin()
{
STACK_GUARD("SkSkin::~SkSkin");
if(m_Mesh)
 delete m_Mesh;
}
SkSkin& SkSkin::operator= (const SkSkin& a)
	{
  if(m_Mesh) delete m_Mesh;
	m_Mesh=new TexObject;
	*m_Mesh=*a.m_Mesh;
	m_BonesNum=a.m_BonesNum;
	for(int i=0;i<m_BonesNum;i++)
		{
		m_Links[i]=a.m_Links[i];
		
		for(int j=0;j<m_Links[i].m_NumPoints;j++)
			{
			m_Links[i].m_Points[j].Point=&m_Mesh->Parts[m_Links[i].m_Points[j].PartNum]->
				Points[m_Links[i].m_Points[j].PntNum];
			}
		}

	return *this;
	}
#include  "../globals.h"

void SkSkin::Update(const SkState *Skel) //обновить состояние кожи
{
STACK_GUARD("SkSkin::Update");
//CodeProfiler CP("skin.update");
  float x,y,z;
  for(int k=0;k<m_Mesh->PartNum;k++)  //цикл по текстурным кускам
    {
    int PntNum;
    if(!(PntNum=m_Mesh->Parts[k]->PntNum)) continue;
    point3 *CurNrm=m_Mesh->Parts[k]->Normals,*CurPnt=m_Mesh->Parts[k]->Points;
    _asm //очищаем массивы точек и нормалей нулями
      {
      emms
      xor eax,eax
      pxor mm0,mm0
      //mov edi,CurNrm;
      mov esi,CurPnt;
      mov ecx,PntNum
      shr ecx,1 //одно нечетное поле
      jnc l1
      //movq [edi],mm0
      //mov [edi+8],eax
      movq [esi],mm0
      mov [esi+8],eax
      //add edi,12
      add esi,12
   l1:
      //movq [edi],mm0
      //movq [edi+8],mm0
      //movq [edi+16],mm0
      movq [esi],mm0
      movq [esi+8],mm0
      movq [esi+16],mm0
      //add edi,24
      add esi,24
      dec ecx
      jnz l1
    emms
      }

    }
  SkBoneToPnts *CurLnk;
  for(CurLnk=m_Links+m_BonesNum-1;CurLnk>=m_Links;CurLnk--)
    {
    int BoneID=CurLnk->m_BoneID;
		if(BoneID<0) continue;
    D3DMATRIX  &m=Skel->m_SkelState[BoneID].World;
    SkBoneToPnts::Pnt *CurPnt=CurLnk->m_Points+CurLnk->m_NumPoints-1;
    for(;CurPnt>=CurLnk->m_Points;CurPnt--)
      {
      x=CurPnt->Offset.x;
      y=CurPnt->Offset.y;
      z=CurPnt->Offset.z;
        CurPnt->Point->x+=CurPnt->Weight*(m._41+x*m._11+y*m._21+z*m._31);
        CurPnt->Point->y+=CurPnt->Weight*(m._42+x*m._12+y*m._22+z*m._32);
        CurPnt->Point->z+=CurPnt->Weight*(m._43+x*m._13+y*m._23+z*m._33);
      }

    }
}
void SkSkin::ValidateLinks(const SkAnim *Anim) //ставит в соответствие кости скелета и кожи
{
STACK_GUARD("SkSkin::ValidateLinks");
	for(int i=0;i<m_BonesNum;i++)
	{
		m_Links[i].m_BoneID=Anim->GetBoneID(m_Links[i].m_BoneName);
	}
}


BBox SkSkin::GetBBox()
	{
STACK_GUARD("SkSkin::GetBBox");
  if(m_Mesh)
	return m_Mesh->GetBBox();
  else
    {
    BBox b;
    b.Box(NULLVEC,0);
    return b;
    }
	}
	bool SkSkin::Save(OptSlot &slot)
		{
		STACK_GUARD("SkSkin::Save");
		if(m_Mesh)
			{
			slot<<(int)SKVERSION;
			m_Mesh->Save(slot);
			slot<<m_BonesNum;
			for(int i=0;i<m_BonesNum;i++)
				m_Links[i].Save(slot);
			}else slot<<(int)0;

		return true;
		}
	bool SkSkin::Load(OptSlot &slot)
		{
		STACK_GUARD("SkSkin::Load");
		int i;
		slot>>i;
		if(i==SKVERSION)
			{
			m_Mesh = new TexObject;
			m_Mesh->Load(slot);
			slot>>m_BonesNum;
			for(int i=0;i<m_BonesNum;i++)
				{
				m_Links[i].Load(slot);
				for(int j=0;j<m_Links[i].m_NumPoints;j++)
					{
					m_Links[i].m_Points[j].Point=&m_Mesh->Parts[m_Links[i].m_Points[j].PartNum]->
						Points[m_Links[i].m_Points[j].PntNum];
					}
				}
			}
		return true;
		}
	bool SkBoneToPnts::Save(OptSlot &slot)
		{
		STACK_GUARD("SkBoneToPnts::Save");
		slot<<m_BoneName<<m_NumPoints;
		for(int i=0;i<m_NumPoints;i++)
			slot<<m_Points[i].Point<<m_Points[i].Offset<<m_Points[i].Weight
			    <<m_Points[i].PartNum<<m_Points[i].PntNum;
		return true;
		}
	bool SkBoneToPnts::Load(OptSlot &slot)
		{
		STACK_GUARD("SkBoneToPnts::Load");
		slot>>m_BoneName>>m_NumPoints;
		Alloc(m_NumPoints);
		for(int i=0;i<m_NumPoints;i++)
			slot>>m_Points[i].Point>>m_Points[i].Offset
			    >>m_Points[i].Weight>>m_Points[i].PartNum
			    >>m_Points[i].PntNum;
		return true;
		}
