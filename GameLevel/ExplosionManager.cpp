#include "precomp.h"
#include "ExplosionManager.h"

#include "../iworld.h"
#include "../character.h"
#include "gamelevel.h"

ExplosionManager *ExplosionManager::m_Instance;

class ParadiseExpMan:public ExplosionManager
  {
  public:
    ParadiseExpMan() {m_Instance=this;};
    ~ParadiseExpMan() {};
    
    //уведомление о попадании в воздух
    void OnAirHit(int  actor, const hit_s& hit);
    //уведомление о попадании в уровень
    void OnLevelHit(int  actor, const hit_s& hit);
    //уведомление о попадании в щит
    void OnShieldHit(int  actor, const hit_s& hit); 
    //уведомление о попадании в существо
    void OnEntityHit(int  actor, int  victim, const hit_s& hit);
    //уведомление о попадании в объект
    void OnObjectHit(int  actor, const std::string& obj, const hit_s& hit);
    //burn
    void BurnObject(const std::string &obj, float damage);
  private:
    void CommonExp(int  actor, const std::string& obj, const hit_s& hit);
  };

ParadiseExpMan ExplosionManagerInstance;//единственный экземпляр

//уведомление о попадании в воздух
void ParadiseExpMan::OnAirHit(int  actor, const hit_s& hit)
  {
    CommonExp(actor,"",hit);
  }
//уведомление о попадании в уровень
void ParadiseExpMan::OnLevelHit(int  actor, const hit_s& hit)
  {
    CommonExp(actor,"",hit);
  }
//уведомление о попадании в щит
void ParadiseExpMan::OnShieldHit(int  actor, const hit_s& hit)
  {
    CommonExp(actor,"",hit);
  } 
//уведомление о попадании в существо
void ParadiseExpMan::OnEntityHit(int  actor, int  /*victim*/, const hit_s& hit) 
  {
  CommonExp(actor,"",hit);
  }
//уведомление о попадании в объект
void ParadiseExpMan::OnObjectHit(int  actor, const std::string& obj, const hit_s& hit)
  {
  CommonExp(actor,obj,hit);
  }
void ParadiseExpMan::BurnObject(const std::string &obj, float damage)
  {
 	DynObjectPool *ObjPool=&IWorld::Get()->GetLevel()->LevelObjects;
   hit_s hit;
    hit.m_to=NULLVEC;            //куда
    hit.m_from=NULLVEC;          //откуда
    hit.m_radius=0;        //радиус взрыва
    hit.m_dmg[0].Type=hit_s::DT_FLAME; //список повреждений
    hit.m_dmg[0].Value=damage; //список повреждений
    hit.m_dmg[1].Type=hit_s::DT_NONE; //список повреждений
    hit.m_dmg[1].Value=0; //список повреждений
    hit.m_dmg[2].Type=hit_s::DT_NONE; //список повреждений
    hit.m_dmg[2].Value=0; //список повреждений
	ObjPool->DoExplosion(obj,hit,m_EDManager);

  }
void ParadiseExpMan::CommonExp(int  actor, const std::string& obj, const hit_s& hit)
  {
	STACK_GUARD("ExplosionManager::CommonExp");
 	DynObjectPool *ObjPool=&IWorld::Get()->GetLevel()->LevelObjects;
	CharacterPool *CharPool=IWorld::Get()->GetCharacterPool();
	//обойдем людей и технику
	CharPool->DoExplosion(actor, hit,m_EDManager);
	//обойдем объекты
	ObjPool->DoExplosion(obj,hit,m_EDManager);
  //m_EDManager->LightFire(hit.m_to, hit.m_radius, 15);

    
  }


