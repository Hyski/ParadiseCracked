#ifndef _EXPLOSIONMANAGER_HEADER_
#define _EXPLOSIONMANAGER_HEADER_
//
//информация о попадании    
//
struct hit_s{
    
    enum{          
        BW_DMG,   //basic weapon damage
        AW_DMG,   //additional weapon damage
        AA_DMG,   //additional ammo damage
            
        MAX_DMSGS,
    };
    
    enum damage_type
    {
    DT_NONE,
    DT_STRIKE,
    DT_SHOCK,
    DT_ENERGY,
    DT_ELECTRIC,
    DT_EXPLOSIVE,
    DT_FLAME,
};

    struct damage
    {
    	damage_type Type;
    	float       Value;
   };


    point3   m_to;            //куда
    point3   m_from;          //откуда
    float    m_radius;        //радиус взрыва
    damage   m_dmg[MAX_DMSGS]; //список повреждений
};

//
// расчет повреждений существ
//
class LogicDamageStrategy{
public:

    virtual ~LogicDamageStrategy(){}

    //нести повреждение существу
    virtual void DamageEntity(int  entity, const hit_s& hit) = 0;
    virtual void DestroyObject(const std::string &ObjName) = 0;
    virtual void LightFire(const point3 &pos, float radius, float damage) = 0;
};

//
// класс - просчитывает объемный взрыв в зависимости от типа попадания
//

class ExplosionManager{
  public:
    virtual ~ExplosionManager(){}
    
    //уведомление о попадании в воздух
    virtual void OnAirHit(int  actor, const hit_s& hit) = 0;
    //уведомление о попадании в уровень
    virtual void OnLevelHit(int  actor, const hit_s& hit) = 0;
    //уведомление о попадании в щит
    virtual void OnShieldHit(int  actor, const hit_s& hit) = 0; 
    //уведомление о попадании в существо
    virtual void OnEntityHit(int  actor, int  victim, const hit_s& hit) = 0;
    //уведомление о попадании в объект
    virtual void OnObjectHit(int  actor, const std::string& obj, const hit_s& hit) = 0;
    //burn
    virtual void BurnObject(const std::string &obj, float damage) = 0;
    
  public:
    static ExplosionManager* Get(){return m_Instance;};
    void SetLogicDamageStrategy(LogicDamageStrategy *dm){m_EDManager=dm;};


  protected:
    static ExplosionManager *m_Instance;
    LogicDamageStrategy *m_EDManager;
  };
#endif