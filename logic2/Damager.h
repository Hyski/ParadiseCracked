//
// механизмы для рассчета повреждений существ и объектов
//

#ifndef _PUNCH_DAMAGER_H_
#define _PUNCH_DAMAGER_H_

class BaseEntity;

//
// абстракция опасного места
//
class RiskSite{
public:

    virtual ~RiskSite(){}

    // получить повреждение для существа
    virtual float GetEntityFlameDmg() const = 0;
};

//
// массив опасных зон на уровне
// 
class RiskArea{
public:

    virtual ~RiskArea(){}

    //получить доступ к экземпляру класса
    static RiskArea* GetInst();

    //сбросить все опасные зоны
    virtual void Reset() = 0;

    //сохранить/загрузить опасные зоны уровня
    virtual void MakeSaveLoad(SavSlot& slot) = 0;
    //произвести взрыв в заданной точке уровня
    virtual void InsertFlame(const point3& center, float radius, float damage) = 0;

    //тестирование
    virtual void TestSmth() = 0;
};

//
// информация о попадании
//
struct hit_info_s{
    
    enum {          
        BW_DMG,   //basic weapon damage
        AW_DMG,   //additional weapon damage
        AA_DMG,   //additional ammo damage
            
        MAX_DMGS,
    };
   
    point3   m_to;            //куда
    point3   m_from;          //откуда
    float    m_radius;        //радиус взрыва
    damage_s m_dmg[MAX_DMGS]; //список повреждений

    hit_info_s(){}

    hit_info_s(const point3& from, const point3& to, float radius, const damage_s dmg[]) :
        m_from(from), m_to(to), m_radius(radius)
    { m_dmg[BW_DMG] = dmg[BW_DMG]; m_dmg[AW_DMG] = dmg[AW_DMG]; m_dmg[AA_DMG] = dmg[AA_DMG];}
};

//краткое имя
typedef hit_info_s hit_t;

//
// расчет повреждений существ
//
class EntityDamager{
public:

    virtual ~EntityDamager(){}

    enum flag_type{
        FT_MAKE_EFFECTS      = 1 << 0, //проигрывать эффекты
        FT_MAKE_DAMAGES      = 1 << 1, //считать повреждения 
        FT_PRINT_MESSAGES    = 1 << 2, //выводить сообщения в лог
        FT_SEND_GAME_EVENTS  = 1 << 3, //рассылать игровые сообщения 
        FT_PLAY_ENTITY_HURT  = 1 << 4, //проигрывать анимацию повреждения
        FT_PLAY_ENTITY_DEATH = 1 << 5, //проигрывать смерть сущесва если после применения повреждений здоровье == 0
        
        CALC_TURN_FALGS = FT_MAKE_EFFECTS|FT_MAKE_DAMAGES|FT_PRINT_MESSAGES|FT_SEND_GAME_EVENTS,
        DEFAULT_FLAGS   = FT_MAKE_EFFECTS|FT_MAKE_DAMAGES|FT_PRINT_MESSAGES|FT_SEND_GAME_EVENTS|FT_PLAY_ENTITY_DEATH|FT_PLAY_ENTITY_HURT,
    };

    //singleton
    static EntityDamager* GetInst();

    //нанести повреждние существу из-за опасной зоны эффекта
    virtual void Damage(BaseEntity* victim, RiskSite* site, unsigned flags = DEFAULT_FLAGS) = 0;
    //нанести повреждение существу обычным способом
    virtual void Damage(BaseEntity* actor, BaseEntity* victim, const hit_t& hit, unsigned flags = DEFAULT_FLAGS) = 0;

protected:

    EntityDamager(){}
};

//
// класс - просчитывает объемный взрыв в зависимости от типа попадания
//
class Damager{
public:

    virtual ~Damager(){}

    //singleton
    static Damager* GetInst();

    //уведомление о попадании в воздух
    virtual void HandleAirHit(BaseEntity* actor, const hit_t& hit, unsigned flags = EntityDamager::DEFAULT_FLAGS) = 0;
    //уведомление о попадании в уровень
    virtual void HandleLevelHit(BaseEntity* actor, const hit_t& hit, unsigned flags = EntityDamager::DEFAULT_FLAGS) = 0;
    //уведомление о попадании в щит
    virtual void HandleShieldHit(BaseEntity* actor, const hit_t& hit, unsigned flags = EntityDamager::DEFAULT_FLAGS) = 0; 
    //уведомление о попадании в объект
    virtual void HandleObjectHit(BaseEntity* actor, const rid_t& obj, const hit_t& hit, unsigned flags = EntityDamager::DEFAULT_FLAGS) = 0;
    //уведомление о попадании в существо
    virtual void HandleEntityHit(BaseEntity* actor, BaseEntity* victim, const hit_t& hit, unsigned flags = EntityDamager::DEFAULT_FLAGS) = 0;

protected:

    Damager(){}
};

#endif // _PUNCH_DAMAGER_H_