//
// ���������� � ���������
//

#ifndef _PUNCH_ENTITY_H_
#define _PUNCH_ENTITY_H_

#ifndef _PUNCH_INFOUTILS_H_
#include "infoutils.h"
#endif

#ifndef _PUNCH_ENTITYAUX_H_
#include "entityaux.h"
#endif

class AIContext;
class EntityVisitor;
class EntityObserver;

class BaseThing;
class WeaponThing;

class GraphHuman;
class GraphEntity;
class GraphVehicle;

class BaseEntity;
class HumanEntity;
class TraderEntity;
class VehicleEntity;

class MoveStrategy;
class CrewStrategy;
class DeathStrategy;
class SoundStrategy;
class PanicTypeSelector;
class FastAccessStrategy;

class EntityValidator;

class Marker;
class Spectator;

//
// ���������� � ����
//
class ForceInfo{
public:

    ForceInfo(const rid_t& rid);
    ~ForceInfo();

    enum force_type{
        FT_PRIVATE,   
        FT_GOVERMENT,
    };

    int GetType() const;

    const rid_t& GetRID() const;
    force_type GetForceType() const;

    const std::string& GetName() const;
    const std::string& GetDesc() const;

private:

    friend class ForceInfoReader;

    rid_t      m_rid;
    force_type m_type;

    std::string m_name;
    std::string m_desc;
};

//
// ������� ���������� � ��������
//
class ForceInfoFactory{
public:

    ForceInfo* Create(int type, const rid_t& rid);
};

//
// ��������� �������� � ���������
//
typedef InfoArchive<ForceInfo, int, ForceInfoFactory> ForceInfoArchive;

//
// ������������ �� ������������
//
typedef InfoHandbook<ForceInfoArchive> ForceHandbook;

//
// ������� ���������� � ��������
//
class EntityInfo{
public:

    EntityInfo(const rid_t& rid, entity_type type);
    virtual ~EntityInfo();

    const rid_t& GetRID() const;
    entity_type GetType() const;

    const std::string& GetName() const;
    const std::string& GetDesc() const;

protected:

    rid_t       m_rid;  
    std::string m_name;
    std::string m_desc;
    entity_type m_type;
};

//
// ���������� � ��������
//
class HumanInfo : public EntityInfo{
public:

    HumanInfo(const rid_t& rid);
    ~HumanInfo();

    enum human_type{
        HT_NONE,
        HT_HERO,
        HT_ENEMY,
        HT_CIVILIAN,
        HT_ANTIHERO,
        HT_QUESTHOLDER,
    };

    //������� �������
    bool IsHero()  const { return m_human_type == HT_HERO; }
    bool IsEnemy() const { return m_human_type == HT_ENEMY; }
    bool IsCivilian() const { return m_human_type == HT_CIVILIAN; }
    bool IsAntihero() const { return m_human_type == HT_ANTIHERO; }
    bool IsQuestHolder() const { return m_human_type == HT_QUESTHOLDER; };

    //��� hacker?
    bool IsHacker() const;

    //����� �� �������� ������ ��������
    bool CanUseScuba() const;

    int GetExp4Kill() const;
    int GetLevelupBase() const;
    int GetLevelupRise() const;

    int GetDangerPoints() const;

    int GetMorale() const;
    int GetWisdom() const;
    int GetHealth() const;
    int GetStrength() const;
    int GetReaction() const;
    int GetAccuracy() const;
    int GetMechanics() const;
    int GetDexterity() const;

    int GetShockRes() const;
    int GetFlameRes() const;
    int GetElectricRes() const;

    float GetFibre() const;
    float GetSightAngle() const;
    float GetSightRadius() const;

    const std::string& GetModel() const;
    const std::string* GetSkins() const;

    const ForceInfo* GetForce() const;

private:

    friend class HumanInfoReader;
    
    int m_exp4kill;
    int m_levelup_base;
    int m_levelup_rise;

    int m_danger_points;

    int m_health;    
    int m_morale;
    int m_wisdom;
    int m_strength;
    int m_reaction;
    int m_accuracy;
    int m_mechanics;
    int m_dexterity;
    
    int m_shock_res;
    int m_flame_res;
    int m_electric_res;

    float m_fibre;
    float m_sight_angle; 
    float m_sight_radius;

    //���� ��� �������
    std::string m_model; 
    std::string m_graph_aux[6];

    human_type m_human_type;

    const ForceInfo* m_force;
};

//
// ���������� � �������
//
class VehicleInfo : public EntityInfo{
public:

    VehicleInfo(const rid_t& rid);

    enum vehicle_size{
        VS_TINY,
        VS_HUGE,
        VS_SMALL,
        VS_LARGE,
    };

    enum vehicle_type{
        VT_TECH,
        VT_ROBOT,
    };

    damage_type  GetArmorType() const;
    vehicle_type GetVehicleType() const;

    bool IsTech() const;
    bool IsRobot() const;

    bool IsTinySize() const;
    bool IsHugeSize() const;
    bool IsSmallSize() const;
    bool IsLargeSize() const;

    int GetRange() const;
    int GetWisdom() const;
    int GetHealth() const;
    int GetQuality() const;
    int GetMp4Shot() const;
    int GetCapacity() const;
    int GetReaction() const;
    int GetAccuracy() const;
    int GetMovepnts() const;
    int GetDexterity() const;
    int GetMechanics() const;
    int GetAmmoCount() const;
    int GetDangerPoints() const;

    int GetExp4Kill() const;
    int GetExp4Hack() const;

    float GetDmgRadius() const;
    float GetSightAngle() const;
    float GetSightRadius() const;
    float GetDeathRadius() const;
    
    //�������� �� � ������� �������
    bool HaveShellOutlet() const;
    const point3& GetShellOutlet() const;

    const damage_s& GetBDmg() const;
    const damage_s& GetADmg() const;
    const damage_s& GetDeathDmg() const;
    
    const std::string& GetModel() const;

    const std::string& GetShotSound() const;
    const std::string& GetMoveSound() const;
    const std::string& GetDamageSound() const;
    const std::string& GetEngineSound() const;
    const std::string& GetExplodeSound() const;

    const std::string& GetHitEffect() const;
    const std::string& GetDeathEffect() const;
    const std::string& GetTraceEffect() const;
    const std::string& GetFlashEffect() const;
    const std::string& GetShellsEffect() const;

    const std::string& GetAmmoInfo() const;
    const std::string& GetWeaponIcon() const;

private:

    friend class VehicleInfoReader;

    damage_type  m_armor_type;
    vehicle_size m_vehicle_size;
    vehicle_type m_vehicle_type;

    int m_range;
    int m_health;
    int m_wisdom;
    int m_mp4shot;
    int m_quality;
    int m_accuracy;
    int m_movepnts;
    int m_reaction;
    int m_capacity;
    int m_mechanics;
    int m_dexterity;
    int m_ammo_count;
    int m_danger_points;

    int m_exp4kill;
    int m_exp4hack;

    point3 m_shell;
    
    damage_s m_bdmg;
    damage_s m_admg;
    damage_s m_death_dmg;

    float m_dmg_radius;
    float m_sight_angle;
    float m_sight_radius;
    float m_death_radius;

    std::string m_model;
    
    std::string m_shot_sound;
    std::string m_move_sound;
    std::string m_damage_sound;
    std::string m_engine_sound;
    std::string m_explode_sound;

    std::string m_hit_effect;
    std::string m_trace_effect;
    std::string m_flash_effect;
    std::string m_death_effect;
    std::string m_shells_effect;

    std::string m_ammo_info;
    std::string m_weapon_icon;
};

//
// ���������� � ��������
//
class TraderInfo : public EntityInfo{
public:

    TraderInfo(const rid_t& rid);

    const std::string& GetSkin() const;
    const std::string& GetModel() const;

    //������ ���� �� ���. �������� ����� �������
    int GetBuyPrice(const BaseThing* thing) const; 
    //������ ���� �� ���. �������� ������� �������
    int GetSellPrice(const BaseThing* thing) const;

private:

    struct ratio_s{
        float m_buy;
        float m_sell;        

        ratio_s() : m_buy(0.7), m_sell(1){}
    };

    typedef std::map<rid_t, ratio_s> ratio_map_t;

    //��������� ��������� ������ �������� �� �������
    int CalcBuyPrice(const ratio_map_t& map, const BaseThing* thing) const;
    int CalcSellPrice(const ratio_map_t& map, const BaseThing* thing) const;

private:

    friend class TraderInfoReader;

    //���� ��� �������
    std::string m_skin;
    std::string m_model;

    ratio_map_t m_ammo;
    ratio_map_t m_armor;
    ratio_map_t m_camera;
    ratio_map_t m_shield;
    ratio_map_t m_weapon;
    ratio_map_t m_grenade;
    ratio_map_t m_medikit;
    ratio_map_t m_scanner;
    ratio_map_t m_implant;
};

//
// ������� ���������� � ��������
//
class EntityInfoFactory{
public:

    EntityInfo* Create(entity_type type, const rid_t& rid);
};

//
// ��������� �������� � ���������
//
typedef InfoArchive<EntityInfo, entity_type, EntityInfoFactory> EntityInfoArchive;

//
// ������������ �� ��������
//
typedef InfoHandbook<EntityInfoArchive> EntityHandbook;

//
// �������� �������
//
class EntityContext{
public:

    EntityContext(BaseEntity* entity = 0);
    virtual ~EntityContext();

    DCTOR_ABS_DEF(EntityContext)

    //������� ����������/��������
    virtual void MakeSaveLoad(SavSlot& slot);

    //��������/���������� ����� ��������
    const std::string& GetAIModel() const;
    void SetAIModel(const std::string& label);

    //����������/�������� ���������� � �����������
    const std::string& GetSpawnZone() const;
    void SetSpawnZone(const std::string& info);

    //�������� ������ �� �����������
    Spectator* GetSpectator();
    //������� ����������� (��������� �������)
    void SetSpectator(Spectator* spectator);

    //���������� ������
    Marker* GetMarker();
    //������ ������ (��������� ������)
    void SetMarker(Marker* marker);

    //���������� ��������� ������
    void SetDeathStrategy(DeathStrategy* strategy);
    //��������� �������� ������
    void PlayDeath(BaseEntity* killer);

    //���������� ��������� ����� ��� �������
    void SetSoundStrategy(SoundStrategy* strategy);

protected:

    //�������� ������ �� ��������
    BaseEntity* GetParent();

private:

    std::string m_ai_model;
    std::string m_spawn_zone;
            
    BaseEntity* m_entity;

    Marker*    m_marker;
    Spectator* m_spectator;

    SoundStrategy* m_sound_strategy;   
    DeathStrategy* m_death_strategy;
};

//
// �������� �������� ������� (humans and vehicles)
//
class ActiveContext : public EntityContext{
public:

    ActiveContext(BaseEntity* entity);
    ~ActiveContext();
    
    //��������/����������
    void MakeSaveLoad(SavSlot& slot);

    //������ � ����������
    bool IsSelected() const;
    void Select(bool flag);

    //���������� ��������� ������������
    void SetMoveStrategy(MoveStrategy* strategy);

    //������ ��������� ������ hex'��
    int GetHexCost() const;
    //������ ���-�� ����� ��� ����� ������� ���-��
    int GetStepsCount() const;

private:

    bool   m_fselected;
    BaseEntity* m_entity;
    MoveStrategy* m_move_strategy;
};

//
// �������� ��������
//
class TraderContext : public EntityContext{
public:

    TraderContext(TraderEntity* trader = 0);
    ~TraderContext();

    DCTOR_DEF(TraderContext)

    void MakeSaveLoad(SavSlot& slot);

    class Iterator;
    typedef Iterator iterator;

    iterator end();
    iterator begin(unsigned mask = TT_ALL_ITEMS);

    //�������� ������� � pack
    tid_t InsertThing(BaseThing* thing);
    //������� ������� �� pack'�
    void RemoveThing(BaseThing* thing);

    //������ ���-�� ��������� ��������� ����
    size_t GetCount(BaseThing* thing) const;

    //�������� ��������� ���������
    struct less_thing{

        bool operator() (const BaseThing* t1, const BaseThing* t2) const;
    };

private:

    friend class Iterator;        

    typedef std::multiset<BaseThing*, less_thing> pack_t;
    pack_t  m_pack;
    tid_t   m_tids;

    TraderEntity* m_trader;
};

//
// �������� �� ������ ��������� � ��������
//
class TraderContext::Iterator{
public:

    typedef TraderContext::pack_t pack_t; 

    Iterator(pack_t* pack, pack_t::iterator first, unsigned mask = 0);

    Iterator& operator ++();
    Iterator  operator ++(int)
    { Iterator tmp = *this; operator++(); return tmp; }
    
    //��������� ��� �������� ������
    BaseThing* operator ->(){ return *m_first;    }
    BaseThing& operator * (){ return *(*m_first); }
    
    friend bool operator != (const Iterator& i1, const Iterator& i2)
    { return i1.m_first != i2.m_first; } 

    friend bool operator == (const Iterator& i1, const Iterator& i2)
    { return i1.m_first == i2.m_first; } 

private:

    bool IsSuitable(const BaseThing* thing) const;

private:

    unsigned  m_mask;
    pack_t*   m_pack;
    pack_t::iterator m_first;
};

//
// �������� ��������
//
class HumanContext : public ActiveContext{
public:

    HumanContext(HumanEntity* human = 0);
    ~HumanContext();

    DCTOR_DEF(HumanContext)

    void MakeSaveLoad(SavSlot& slot);

    //�������� ������ ����� ��������
    shot_type GetShotType() const;
    void SetShotType(shot_type type);

    //��������/���������� ������ �� �������
    VehicleEntity* GetCrew();
    void SetCrew(VehicleEntity* vehicle);

    //������ � ���������������� �������������
    bool HaveLongDamage(damage_type type) const;
    int  GetLongDamage(damage_type type, int moment) const;
    void SetLongDamage(damage_type type, int value, int moment = DMG_DURATION-1);

    //������ � ������������� ������ ����
    int GetBodyPartDamage(body_parts_type type) const;
    void SetBodyPartDamage(body_parts_type type, int dmg);

    class Traits;
    class Limits;

    //�������� ������ �� ���������
    Traits* GetTraits();
    //�������� ������ �� �������
    Limits* GetLimits();

    class Iterator;
    typedef Iterator iterator;

    //����������� �� ������ ��������� ��������
    iterator begin(human_pack_type pack, unsigned mask = TT_ALL_ITEMS);
    iterator end();

    //�������� ������� �� ��������
    tid_t InsertThing(human_pack_type pack, BaseThing* thing);
    //�������� ���� ������� �������� (����� ��������)
    void RemoveThing(iterator& itor);
    //������� �������
    void RemoveThing(BaseThing* thing);

    //���� ��������?
    bool IsFull(human_pack_type pack) const;

    enum hands_mode{
        HM_HANDS,        //� ����� ������� �� ����� ���
        HM_FAST_ACCESS,  //� ����� ������� ��������� �������� �������
    };

    //����������/������ ����� ������ ���
    hands_mode GetHandsMode() const;
    void SetHandsMode(hands_mode mode);

    //�������� ������� � ����� ��������
    BaseThing* GetThingInHands(unsigned mask = TT_ALL_ITEMS);

    //��������/������� ��������� �������� ������ � ��������
    FastAccessStrategy* GetFastAccessStrategy();
    void SetFastAccessStrategy(FastAccessStrategy* strategy);

    //���������� ��������� ������ ������
    void SetPanicSelector(PanicTypeSelector* panic);

    //��������� ������
    human_panic_type SelectPanicType();
    //������ ������� ����� ������
    human_panic_type GetPanicType() const;
    //������������� ���������� ����� ������
    human_panic_type SetPanicType(human_panic_type type);

    //��� ������� �� ������� hex'���
    int  GetFlameSteps() const;
    void SetFlameSteps(int steps);

    //����� �� ������� ������
    bool CanRun() const;
    //����� �� ������� �����
    bool CanSit() const;

    //
    // ���������������� ����� ��� ������� ����������� ��� ����������� ������
    //
    class reload_notifier{
    public:

        ~reload_notifier();
        reload_notifier(HumanEntity* human, WeaponThing* thing);

    private:

        rid_t m_ammo_rid;
        int   m_ammo_count;

        HumanEntity* m_actor;
        WeaponThing* m_weapon;
    };
    
private:

    friend class Iterator;

    typedef std::vector<BaseThing*> pack_t;
    pack_t m_pack;

    //����� ������� � �����
    hands_mode m_hands_mode;

    Traits* m_traits;
    Limits* m_limits;

    shot_type m_shot_type;

    //���-�� ����� �� ������� hex'���
    int m_flame_steps;

    //����������� ������ ����
    int m_body_part_damage[MAX_BODY_PARTS];

    //��������������� �����������
    int m_long_shock_damage[DMG_DURATION];
    int m_long_flame_damage[DMG_DURATION];
    int m_long_electric_damage[DMG_DURATION];
    
    HumanEntity*   m_human;
    VehicleEntity* m_vehicle;

    FastAccessStrategy* m_fast_access;
    PanicTypeSelector*  m_panic_selector;

    static const int m_pack_map[MAX_PACKS];
};

//
//�������� �� ��������� � ���� ��������
//
class HumanContext::Iterator{
public:

    Iterator(HumanContext::pack_t* pack = 0, int first = 0, int last = 0, unsigned mask = 0);

    //��������
    Iterator& operator ++();
    Iterator  operator ++(int)
    { Iterator tmp = *this; operator++(); return tmp;}
    
    //��������� ��� �������� ������
    BaseThing* operator ->(){return (*m_pack)[m_first];}
    BaseThing& operator * (){return *(*m_pack)[m_first];}
    
    //��������� �� !=
    friend bool operator != (const Iterator& i1, const Iterator& i2)
    { return i1.m_first != i2.m_first || i1.m_last != i2.m_last; }

private:

    bool IsSuitable(const BaseThing* thing) const;

private:

    int m_first;
    int m_last;
        
    unsigned  m_mask;    
    HumanContext::pack_t* m_pack;
};

//
// ��������� ��������
//
class HumanContext::Traits{
public:

    Traits(HumanEntity* human);

    void MakeSaveLoad(SavSlot& slot);

    //�������� ��������
    int GetHealth() const;
    //�������� ���������� ����. ������� (�� �������) 
    int GetConstHealth() const;

    float GetWeight() const;

    int GetWisdom() const;
    int GetMorale() const;
    int GetMovepnts() const;
    int GetAccuracy() const;
    int GetStrength() const;
    int GetReaction() const;
    int GetMechanics() const;
    int GetDexterity() const;

    int GetShockRes() const;
    int GetFlameRes() const;
    int GetElectricRes() const;

    float GetSightAngle() const;
    float GetBackRadius() const;
    float GetFrontRadius() const;

    int GetLevel() const;
    int GetExperience() const;
    int GetLevelupPoints() const;
   
    //���������� �������� ��� ���������
    void SetMovepnts(int val);
    
    //��������� � ��� ��������� ��������
    void AddHealth(int val);
    void AddWisdom(int val);
    void AddMorale(int val);
    void AddMovepnts(int val);
    void AddAccuracy(int val);
    void AddStrength(int val);
    void AddReaction(int val);
    void AddShockRes(int val);
    void AddFlameRes(int val);
    void AddMechanics(int val);
    void AddDexterity(int val);
    void AddExperience(int val);
    void AddElectricRes(int val);
    void AddFrontRadius(float val);
    void AddLevelupPoints(int val);

private:

    friend class HumanContext;
    void HandleInsertImplant(int parameter, int value);

private:

    float m_sight_angle;
    float m_front_radius;

    int m_movepoints;

    int m_level;
    int m_experience;
    int m_levelup_points;

    int m_health;
    int m_wisdom;
    int m_morale;
    int m_accuracy;
    int m_strength;
    int m_reaction;
    int m_dexterity;
    int m_mechanics;

    int m_shock_res;
    int m_flame_res;
    int m_electric_res;

    //���������� ����������� �� ���������
    int m_const_wisdom;
    int m_const_health;
    int m_const_accuracy;
    int m_const_strength;
    int m_const_reaction;

    int m_const_shock_res;
    int m_const_flame_res;
    int m_const_electric_res;

    float m_const_front_radius;

    HumanEntity* m_human;
};

//
// ������� ��������
//
class HumanContext::Limits{
public:

    Limits(HumanEntity* human);

    void MakeSaveLoad(SavSlot& slot);

    float GetWeight() const;
    float GetFrontRadius() const;

    int GetWisdom() const;
    int GetMorale() const;
    int GetHealth() const;
    int GetMovepnts() const;
    int GetAccuracy() const;
    int GetStrength() const;
    int GetReaction() const;
    int GetDexterity() const;
    int GetMechanics() const;

    void SetHealth(int val);
    void SetWisdom(int val);
    void SetAccuracy(int val);
    void SetStrength(int val);
    void SetReaction(int val);
    void SetMechanics(int val);
    void SetDexterity(int val);
    void SetFrontRadius(float val);

private:

    int m_health;
    int m_wisdom;
    int m_morale;
    int m_accuracy;
    int m_strength;
    int m_reaction;
    int m_mechanics;
    int m_dexterity;

    float m_front_radius;

    HumanEntity* m_human;
};

//
// �������� �������
//
class VehicleContext : public ActiveContext{
public:

    VehicleContext(VehicleEntity* context = 0);
    ~VehicleContext();

    DCTOR_DEF(VehicleContext)

    void MakeSaveLoad(SavSlot& slot);

    //������ �� ���������
    int GetHealth() const;
    void SetHealth(int val);

    //������ � ������ �������
    int GetMovepnts() const;
    void SetMovepnts(int value);

    //������ � ����������
    int GetAmmoCount() const;
    void SetAmmoCount(int value);

    //���������� ��������� ������ � �������
    void SetCrewStrategy(CrewStrategy* strategy);

    //����� �������. � �������?
    bool CanJoinCrew(HumanEntity* human) const;
    //�������� � �������
    void IncCrew(HumanEntity* human);
    //������� �� �������
    void DecCrew(HumanEntity* human);
    //���������/��������� ������� (����� ��� ���������)
    void EnableShipment(bool flag);
    //������ ������ �������
    int GetCrewSize() const;

    //����������/�������� ������ �� ��������
    HumanEntity* GetDriver();

private:

    int m_health;
    int m_movepnts;
    int m_ammo_count;

    CrewStrategy*  m_crew_strategy;
    VehicleEntity* m_vehicle;
};

//
// ������� ��������
//
class BaseEntity{
public:

    BaseEntity(entity_type type = ET_NONE, const rid_t& rid = rid_t());
    virtual ~BaseEntity();
 
    DCTOR_ABS_DEF(BaseEntity)

    //������� ����������/��������
    virtual void MakeSaveLoad(SavSlot& slot);

    //����������� �����������
    void Detach(EntityObserver* observer);
    //������� �����������
    void Attach(EntityObserver* observer, EntityObserver::event_t event);
		bool IsAttached(EntityObserver* observer) {return m_observers.isAttached(observer);};
    //��������� ������������
    void Notify(EntityObserver::event_t event, EntityObserver::info_t info = 0);

    //pattern visitor
    virtual void Accept(EntityVisitor& visitor) = 0;

    //������ dynamic_cast
    virtual HumanEntity* Cast2Human();
    virtual TraderEntity* Cast2Trader();
    virtual VehicleEntity* Cast2Vehicle();

    //������ id ������� � ����
    eid_t GetEID() const;
    //���������� id c������� (������ ���������� ������ � EntityPool::Push)
    void SetEID(eid_t eid);

    //�������� ��� ������ � �������
    void RaiseFlags(unsigned flags);
    void DropFlags(unsigned flags);

    //��������� ������������� �� �����
    bool IsRaised(unsigned flags) const;

    //��������� ������ �� �������
    entity_type GetType() const;  
    player_type GetPlayer() const;
    entity_attribute GetAttributes() const; 

    //�������� ������ �� ���������� ��� AI
    AIContext* GetAIContext();
    //���������� ����� �������� AI (������ ���������)
    void SetAIContext(AIContext* new_context);

    //�������� ������ �� ������ ��������
    GraphEntity* GetGraph();
    //�������� ������ �� ��������
    EntityContext* GetEntityContext();
    //�������� ������ �� ����������
    const EntityInfo* GetInfo() const;

protected:

    //���������� ������. ��������
    void SetGraph(GraphEntity* graph);
    //���������� �������� ��������
    void SetEntityContext(EntityContext* context);

private:

    eid_t    m_eid;
    unsigned m_flags;

    EntityInfo* m_info;
    GraphEntity* m_graph;    

    AIContext*     m_ai_context;
    EntityContext* m_entity_context;

    typedef ObserverManager<EntityObserver> obs_mgr_t;   
    obs_mgr_t m_observers;

    //��������� �������������� ���������� �������
    static EntityValidator* m_validator;
};

//
// ����� �� ��������
//
class HumanEntity : public BaseEntity{
public:

    HumanEntity(const rid_t& rid = rid_t());
    ~HumanEntity();

    DCTOR_DEF(HumanEntity)

    //������ dynamic_cast
    HumanEntity* Cast2Human();

    //pattern visitor
    void Accept(EntityVisitor& visitor);

    //�������� ������ �� �������
    GraphHuman* GetGraph();
    //�������� ������ �� ��������
    HumanContext* GetEntityContext();
    //�������� ������ �� ����������
    const HumanInfo* GetInfo() const;
};

//
// ����� �� ��������
//
class TraderEntity : public BaseEntity{
public:

    TraderEntity(const rid_t& rid = rid_t());
    ~TraderEntity();

    DCTOR_DEF(TraderEntity)

    //������ dynamic_cast
    TraderEntity* Cast2Trader();

    //pattern visitor
    void Accept(EntityVisitor& visitor);

    //�������� ������ �� �������
    GraphHuman* GetGraph();
    //�������� ������ �� ��������
    TraderContext* GetEntityContext();
    //�������� ������ �� ����������
    const TraderInfo* GetInfo() const;
};

//
// ����� �� �������
//
class VehicleEntity : public BaseEntity{
public:

    VehicleEntity(const rid_t& rid = rid_t());
    ~VehicleEntity();

    DCTOR_DEF(VehicleEntity)

    //������ dynamic_cast
    VehicleEntity* Cast2Vehicle();

    //pattern visitor
    void Accept(EntityVisitor& visitor);

    //�������� ������ �� �������
    GraphVehicle* GetGraph();
    //�������� ������ �� ��������
    VehicleContext* GetEntityContext();
    //�������� ������ �� ����������
    const VehicleInfo* GetInfo() const;
};


//
//������ �������
//
class EntityPool{
public:

    //pattern singleton
    static EntityPool* GetInst();

    ~EntityPool();

    void MakeSaveLoad(SavSlot& st);

    //�������� ������ �� ��������
    BaseEntity* Get(eid_t id);
  
    //�������� �������� � ������
    void Insert(BaseEntity* entity);
    //������� �������� �� ������
    void Remove(BaseEntity* entity);

    class Iterator;
    typedef Iterator iterator;

    //�������������� ��� ������� �� ������ �������
    iterator begin(unsigned etype = ET_ALL_ENTS, unsigned ptype = PT_ALL_PLAYERS, unsigned traits = EA_ALL_ATTRIBUTES);
    iterator end();
    
protected:

    EntityPool();

private:
            
    BaseEntity* m_ents[MAX_ENTITIES];
};

//
// �������� �� ���������
//
class EntityPool::Iterator{
public:

    Iterator():m_first(0), m_last(0){}

    Iterator& operator ++();
    Iterator  operator ++(int) 
    { Iterator tmp = *this;  operator++(); return tmp;}
   
    //��������� ������� � ���������
    BaseEntity* operator ->(){ return *m_first;}
    BaseEntity& operator * (){ return *(*m_first);}
    
    //��������� �� !=
    friend bool operator != (const Iterator& i1, const Iterator& i2)
    { return i1.m_first != i2.m_first || i1.m_last != i2.m_last; }

    friend bool operator == (const Iterator& i1, const Iterator& i2)
    { return i1.m_first == i2.m_first && i1.m_last == i2.m_last; }

private:

    friend class EntityPool;
    Iterator(BaseEntity** first, BaseEntity** last, unsigned type, unsigned team, unsigned attr) :
    m_type(type), m_team(team), m_attr(attr), m_first(first), m_last(last) {if(m_first != m_last){ m_first--; operator++();}}
    
    unsigned  m_type;
    unsigned  m_team;
    unsigned  m_attr;

    BaseEntity** m_first;
    BaseEntity** m_last;
};

#endif // _PUNCH_ENTITY_H_