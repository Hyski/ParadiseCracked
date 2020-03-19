#include "precomp.h"
#include "character.h"
#include "IWorld.h"

#include "skin/skin.h"
#include "skin/person.h"
#include "gamelevel/scatteredItems.h"
#include "interface/banner.h"
#include "gamelevel/explosionmanager.h"
#include "common/graphpipe/vshader.h"

CharacterPool::CharacterPool():
BannerVis(false),FOSVis(false)
  {}

CharacterPool::Handle CharacterPool::CreateHuman()
  {
STACK_GUARD("CharacterPool::CreateHuman");
  IEntity *ent=new Person;
  for(int i=0;i<Pool.size();i++)
    {
    if(!Pool[i]) 
      {
      Pool[i]=ent;
      return i;   
      }
    }
  Pool.push_back(ent);
  return Pool.size()-1;
  }
CharacterPool::Handle CharacterPool::CreateVehicle()
  {
STACK_GUARD("CharacterPool::CreateVehicle");
  IEntity *ent=new Vehicle;
  for(int i=0;i<Pool.size();i++)
    {
    if(!Pool[i]) 
      {
      Pool[i]=ent;
      return i;
      }
    }
  Pool.push_back(ent);
  return Pool.size()-1;
  }

void CharacterPool::Draw()
  {
STACK_GUARD("CharacterPool::Draw");
  Handle i=0,en=Pool.size();
  /* by Flif GraphPipe *Pipe=IWorld::Get()->GetPipe();*/
  for(;i<en;i++)
    if(Pool[i])
      {
      Pool[i]->Draw();
      }
	StatesManager::SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX*)IDENTITYMAT);
  for(i=0;i<en;i++)
    if(Pool[i])
      {
      if(BannerVis)         Pool[i]->DrawBanner();
      if(FOSVis)            Pool[i]->DrawFOS();
      }
  }
void CharacterPool::Update(float Time)
  {
STACK_GUARD("CharacterPool::Update");
  Handle i=0,en=Pool.size();
  for(;i<en;i++)
    if(Pool[i])Pool[i]->UpdateOnTime(Time);
  }
void CharacterPool::ShowDecor(bool Show, DECOR_TYPE dt)
  {
STACK_GUARD("CharacterPool::ShowDecor");
  if(dt==DT_FOS)  FOSVis=Show;
  if(dt==DT_BANNER)
    {
    if(Show)
      {
      }
    BannerVis=Show;
    }
  }
void CharacterPool::MakeSaveLoad(Storage &St)
{
STACK_GUARD("CharacterPool::MakeSaveLoad");
SavSlot Slot(&St,"CPOOL");
	Handle i=0,en=Pool.size();
	if(Slot.IsSaving())
	{
		Slot<<en;
		for(;i<en;i++)
		{
			if(Pool[i])
			{
				Slot<<(Pool[i]->GetHuman()?(int)ET_PERSON:(int)ET_VEHICLE);
				Pool[i]->MakeSaveLoad(Slot);
			}
			else
			{
				Slot<<(int)ET_NONE;
			}
		}
	}
	else
		{
		Clear();
		Slot>>en;
		for(;i<en;i++)
		{
			entity_type type;
			Slot>>(*(int*)&type);
			IEntity *ent = 0;
			switch(type)
			{
			case ET_PERSON:	ent=new Person;
				break;
			case ET_NONE:	  ent=NULL;
				break;
			case ET_VEHICLE:ent=new Vehicle;
				break;
			}
			if(ent) ent->MakeSaveLoad(Slot);
			Pool.push_back(ent);
		}
		
	}
}
CharacterPool::~CharacterPool()
  {
STACK_GUARD("CharacterPool::~CharacterPool");
  Clear();
  }
void CharacterPool::Delete(Handle h)
  {
STACK_GUARD("CharacterPool::Delete");
  if(h<Pool.size()&&h>=0)
    if(Pool[h])
      {
      delete Pool[h];
      Pool[h]=NULL;
      }
  }

void CharacterPool::Clear()
  {
STACK_GUARD("CharacterPool::Clear");
  for(int i=0;i<Pool.size();i++)
    {
    if(Pool[i]) delete Pool[i];
    }
	SoundEventsQueue::Clear();
  Pool.clear();
  }
void CharacterPool::CleanByAction(ANIMATION Animation)
	{
STACK_GUARD("CharacterPool::CleanByAction");
  for(int i=0;i<Pool.size();i++)
    {
    if(Pool[i])
			if(Pool[i]->GetHuman()&&Pool[i]->GetHuman()->CurState==Animation||
				Pool[i]->GetVehicle()&&Pool[i]->GetVehicle()->CurState==Animation)
				Delete(i);
    }
	}
void CharacterPool::DoExplosion(int  entity, const hit_s& hit, LogicDamageStrategy *LogicDamage)
  {
STACK_GUARD("CharacterPool::DoExplosion");
  if(hit.m_radius==0||!LogicDamage) return;
  
  hit_s NewHit;
  
  for(int i=0;i<Pool.size();i++)
    {
    if(Pool[i])
      {
      if(i==entity&&hit.m_radius<0.2) continue;

      point3 CurPos=Pool[i]->GetLocation();
      /* by Flif BBox bound;*/
      RealEntity *ent=static_cast<RealEntity *>(Pool[i]);
      
      float dist=ent->SkinBox.DistToPoint(hit.m_to);
      
      //float dist=(CurPos-Pos).Length();
      if(dist<hit.m_radius)
        {
        NewHit=hit;
        float falloff;
        if(dist<hit.m_radius/3)
          falloff=1.f;
        else 
          {
          float dc=dist-hit.m_radius/3+1;
          falloff=(1.f/(dc*dc*dc));
          }
        for(int n=0;n<hit_s::MAX_DMSGS;n++)
          {
          NewHit.m_dmg[n].Value=hit.m_dmg[n].Value*falloff;
          }
        LogicDamage->DamageEntity(i,NewHit);
        }
      }
    }
  
  }

void CharacterPool::DoExplosion(const point3& Pos,float Radius,const point3 &Dir,
		               const IExplosionAPI::Damage _dam[3],
									 std::string ObjName,
									 IExplosionAPI *API)
{
STACK_GUARD("CharacterPool::DoExplosion");
	if(Radius==0) return;
	IExplosionAPI::Damage dam[3];

	for(int i=0;i<Pool.size();i++)
	{
    if(Pool[i])
		{
			point3 CurPos=Pool[i]->GetLocation();
			/* by Flif BBox bound;*/
			RealEntity *ent=static_cast<RealEntity *>(Pool[i]);

			float dist=ent->SkinBox.DistToPoint(Pos);

			//float dist=(CurPos-Pos).Length();
			if(dist<Radius)
			{
      dam[0]=_dam[0];dam[1]=_dam[1];dam[2]=_dam[2];
      float falloff;
      if(dist<Radius/3)
        falloff=1.f;
      else 
        {
        float dc=dist-Radius/3+1;
        falloff=(1.f/(dc*dc*dc));
        }
  		for(int n=0;n<3;n++)
				{
					dam[n].Power=dam[n].Power*falloff;
				}
						API->DamageEntity(Pos, Dir, dam,i);
			}
		}
	}
}
bool CharacterPool::TraceSegment(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm,Handle *Who)
	{
STACK_GUARD("CharacterPool::TraceSegment");
	static const float BOGUS=100000;
	float dist=BOGUS;
	point3 _norm,_res;
	float d;
	Handle who = 0;
	
	for(int i=0;i<Pool.size();i++)
		{
    if(Pool[i])
			{
			if(Pool[i]->TraceSegment(From,Dir,&_res,&_norm))
				{
				d=(_res-From).Length();
				if(dist>d)
					{
					dist=d;
					*Res=_res;*Norm=_norm;
					who=i;
					}
				}
			}
    }
	if(dist==BOGUS)
		return false;
	else
		{
	if(Who)*Who=who;
	return true;
		}
	
	}

ItemsPool::~ItemsPool()
  {
STACK_GUARD("ItemsPool::~ItemsPool");
  Clear();
  }
void ItemsPool::Clear()
  {
STACK_GUARD("ItemsPool::Clear");
  for(int i=0;i<Pool.size();i++)
    {
    if(Pool[i]) delete Pool[i];
    }
  Pool.clear();
  }
ItemsPool::Handle ItemsPool::Create(const std::string &Name, const point3 &Pos)
{
STACK_GUARD("ItemsPool::Create");
  ScatteredItem *ent=new ScatteredItem(Pos,Name);
  for(int i=0;i<Pool.size();i++)
	{
    if(!Pool[i]) 
		{
      Pool[i]=ent;
      return i;   
		}
	}
  Pool.push_back(ent);
  return Pool.size()-1;
}
void ItemsPool::Draw()
  {
STACK_GUARD("ItemsPool::Draw");
  Handle i=0,en=Pool.size();
  /* by Flif GraphPipe *Pipe=IWorld::Get()->GetPipe();*/
  for(;i!=en;i++)
    if(Pool[i])Pool[i]->Draw(Timer::GetSeconds());
  }
void ItemsPool::MakeSaveLoad(SavSlot &Slot)
  {
STACK_GUARD("ItemsPool::MakeSaveLoad");
  if(Slot.IsSaving())
    {
    Slot<<Pool.size();
    for(int i=0;i<Pool.size();i++)
      {
      if(Pool[i]){Slot<<Pool[i]->GetPos()<<Pool[i]->ItName;}
      else{Slot<<NULLVEC<<std::string("");}
      }
    
    }
  else
    {
    Clear();

    point3 Pos;
    std::string Name;
    ScatteredItem *ent;
    int num=0;
    Slot>>num;
      for(int i=0;i<num;i++)
        {
        Slot>>Pos>>Name;
        if(Name.size())ent=new ScatteredItem(Pos,Name);
        else ent=NULL;
        Pool.push_back(ent);
        }
    }
  
  }
void ItemsPool::Delete(Handle h)
  {
STACK_GUARD("ItemsPool::Delete");
  if(h<Pool.size()&&h>=0)
    if(Pool[h])
      {
      delete Pool[h];
      Pool[h]=NULL;
      }
  }

