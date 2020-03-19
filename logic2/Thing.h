//
// описание свойств предметов игрового мира
//

#ifndef _PUNCH_THING_H_
#define _PUNCH_THING_H_

#ifndef _PUNCH_INFOUTILS_H_
#include "infoutils.h"
#endif

class Spectator;

class StoreBox;
class BaseEntity;
class GraphThing;

//
// базовая информация о предмете  
//
class ThingInfo{
public:

    ThingInfo(const rid_t& rid, thing_type type);
    virtual ~ThingInfo();

    //получить тип предмета
    thing_type GetType() const;

    bool IsAmmo() const    { return m_type == TT_AMMO;    }
    bool IsMoney() const   { return m_type == TT_MONEY;   }
    bool IsArmor() const   { return m_type == TT_ARMOUR;  }
    bool IsCamera() const  { return m_type == TT_CAMERA;  }
    bool IsShield() const  { return m_type == TT_SHIELD;  }
    bool IsWeapon() const  { return m_type == TT_WEAPON;  }
    bool IsGrenade() const { return m_type == TT_GRENADE; } 
    bool IsImplant() const { return m_type == TT_IMPLANT; }
    bool IsMedikit() const { return m_type == TT_MEDIKIT; }
    bool IsScanner() const { return m_type == TT_SCANNER; }
    
    //селекторы свойств предмета
    int GetCost() const;
    float GetWeight() const;
    const rid_t& GetRID() const;
    const ipnt2_t& GetSize() const;
    const std::string& GetName() const;
    const std::string& GetDesc() const;
    const std::string& GetShader() const;
    const std::string& GetInitials() const;
    const std::string& GetItemModel() const;

    //необх для меню описания предметов и энциклопедии
    struct traits_string{
        std::string m_left_col;  //левая колонка
        std::string m_right_col; //правая колонка
    };

    virtual void GetTraitsString(traits_string* traits) const;

protected:

    friend class EquipmentInfoReader;

    rid_t      m_rid;
    thing_type m_type;

    int     m_cost;   //стоимость
    ipnt2_t m_size;   //размер item'a в меню
    float   m_weight; //вес    

    std::string m_name;      //имя 
    std::string m_desc;      //описание
    std::string m_shader;    //название шейдера для картинки в Inventory
    std::string m_initials;  //сокращенное название снаряжения
    std::string m_item_model;//моделька для inventory
};

//
// информация о гранате
//
class GrenadeInfo : public ThingInfo{
public:

    GrenadeInfo(const rid_t& rid);

    enum grenade_type{
        MINE,
        GRENADE,
    };

    bool IsPlasmaGrenade() const;

    int GetMp2Act() const;
    int GetWisdom() const;
    int GetStrength() const;

    float GetDmgRadius() const;

    const damage_s& GetADmg() const;
    const damage_s& GetBDmg() const;

    const std::string& GetHitEffect() const;
    const std::string& GetHoldModel() const;

    void GetTraitsString(traits_string* traits) const;

private:

    friend class GrenadeInfoReader;

    damage_s m_bdmg;    
    damage_s m_admg;
        
    int m_mp2act;
    int m_wisdom;
    int m_strength;

    float m_dmg_radius;
    
    std::string  m_hit_effect;    
    std::string  m_hold_model;

    grenade_type m_gtype;
};

//
// информация о патронах 
//
class AmmoInfo : public ThingInfo{
public:

    AmmoInfo(const rid_t& rid);

    int GetRange() const;
    int GetQuality() const;
    int GetFalloff() const;

    float GetCaliber() const;
    float GetDmgRadius() const;

    const damage_s& GetADmg() const;
    const damage_s& GetBDmg() const;

    const std::string& GetHitEffect() const;
    const std::string& GetTraceEffect() const;
    const std::string& GetShellsEffect() const;

    void GetTraitsString(traits_string* traits) const;

private:

    friend class AmmoInfoReader;

    damage_s m_bdmg;
    damage_s m_admg;

    int   m_range;
    int   m_quality;
    int   m_falloff;
    float m_caliber;   
    float m_dmg_radius;

    std::string m_hit_effect;
    std::string m_trace_effect;
    std::string m_shells_effect;
};

//
// информация об оружии
//
class WeaponInfo : public ThingInfo{
public:

    WeaponInfo(const rid_t& rid);
    
    const damage_s& GetADmg() const;
    const damage_s& GetBDmg() const;

    //может ли стрелять очередями
    bool IsAuto() const;
    //это пулемет
    bool IsCannon() const;

    int GetRange() const;
    int GetWisdom() const;
    int GetAimQty() const;
    int GetAutoQty() const;
    int GetQuality() const;
    int GetStrength() const;
    int GetAccuracy() const;
    int GetMovepnts() const;
    float GetCaliber() const;
	int GetMps2Reload() const;

    hold_type GetHoldType() const;

    bool IsRocket() const;
    bool IsAutoCannon() const;

    const point3& GetBarrel() const;

    //вылет гильз при стрельбе
    bool HaveShellOutlet() const;
    const point3& GetShellOutlet() const;
    
    const std::string& GetHoldModel() const;
    const std::string& GetFlashEffect() const;
    const std::string& GetAmmoLoadSound() const;
    const std::string& GetShotSound(shot_type type) const;

    void GetTraitsString(traits_string* traits) const;

private:

    friend class WeaponInfoReader;

    damage_s m_bdmg;
    damage_s m_admg;

    int   m_range;
    int   m_wisdom;
    int   m_aim_qty;
    int   m_quality;
    int   m_strength;
    int   m_auto_qty;
    int   m_accuracy;
    int   m_movepnts;
    float m_caliber;

    point3 m_shell;
    point3 m_barrel;

    hold_type m_htype;

    std::string m_shot_sound;
    std::string m_autoshot_sound;
    std::string m_ammo_load_sound;

    std::string m_hold_model;
    std::string m_flash_effect;
	//Grom
	int m_mp2reload;
};

//
// информация об имланте
//
class ImplantInfo : public ThingInfo{
public:

    ImplantInfo(const rid_t& rid);

    enum parameter_type {
        PT_SIGHT,
        PT_WISDOM,
        PT_HEALTH,
        PT_STRENGTH,
        PT_REACTION,
        PT_ACCURACY,
        PT_SHOCK_RES,
        PT_FLAME_RES,
        PT_ELECTRIC_RES,
    };

    int GetPower() const;
    int GetWisdom() const;
    int GetMp2Act() const;

    parameter_type GetParameter() const;    

    void GetTraitsString(traits_string* traits) const;

private:

    friend class EquipmentInfoReader;

    parameter_type m_parameter;

    int m_power;
    int m_wisdom;    
    int m_mp2act;
};

//
// свойства броника
//
class ArmorInfo : public ThingInfo{
public:

    ArmorInfo(const rid_t& rid);

    bool IsHelmet() const;
    bool IsSpaceSuit() const;

    float GetBody() const;
    float GetLegs() const;
    float GetHead() const;
    float GetHands() const;

    int GetMp2Act() const;
    int GetStrength() const;

    armor_type GetSuit() const;
    damage_type GetDmgType() const; 

    float GetRearAmplify() const;

    void GetTraitsString(traits_string* traits) const;

private:

    friend class ArmorInfoReader;

    armor_type  m_suit;
    damage_type m_dmg_type;

    float m_body;
    float m_legs;
    float m_head;
    float m_hands;

    int m_mp2act;
    int m_strength;
    
    float m_rear_amplify;    
};

//
// информация об аптечке
//
class MedikitInfo : public ThingInfo{
public:

    MedikitInfo(const rid_t& rid);

    int GetDose() const;
    int GetWisdom() const;
    int GetMp2Act() const;
    int GetCharge() const;

    void GetTraitsString(traits_string* traits) const;

private:

    friend class EquipmentInfoReader;

    int m_dose;
    int m_wisdom;    
    int m_mp2act;
    int m_charge;
};

//
// информация о деньгах
//
class MoneyInfo : public ThingInfo{
public:

    MoneyInfo();

    const std::string& GetUseSound() const;

private:

    friend class EquipmentInfoReader;

    std::string m_use_sound;
};

//
// информация о ключе
//
class KeyInfo : public ThingInfo{
public:

    KeyInfo(const rid_t& rid);

    const std::string& GetUseSound() const;

private:

    friend class EquipmentInfoReader;

    std::string m_use_sound;
};

//
// информация о камере
//
class CameraInfo : public ThingInfo{
public:

    CameraInfo(const rid_t& rid);

    int GetMp2Act() const;
    int GetWisdom() const;
    int GetCharge() const;
    float GetRadius() const;    

    const std::string& GetHoldModel() const;
    const std::string& GetVisualEffect() const;

    void GetTraitsString(traits_string* traits) const;

private:

    int m_charge;
    int m_mp2act;
    int m_wisdom;

    float m_radius;

    std::string m_hold_model;
    std::string m_effect_name;

    friend class EquipmentInfoReader;
};

//
// информация о щите
//
class ShieldInfo : public ThingInfo{
public:

    ShieldInfo(const rid_t& rid);

    int GetMp2Act() const;
    int GetWisdom() const;
    int GetCharge() const;
    float GetRange() const;

    const std::string& GetHoldModel() const;
    const std::string& GetVisualEffect() const;


    void GetTraitsString(traits_string* traits) const;

private:

    int m_charge;
    int m_mp2act;
    int m_wisdom;

    float m_range;

    std::string m_hold_model;
    std::string m_effect_name;

    friend class EquipmentInfoReader;
};

//
// информация о сканнере
//
class ScannerInfo : public ThingInfo{
public:

    ScannerInfo(const rid_t& rid);

    int GetMp2Act() const;
    int GetWisdom() const;    
    float GetRadius() const;

    const std::string& GetUseSound() const;
    const std::string& GetHoldModel() const;

    void GetTraitsString(traits_string* traits) const;

private:

    int   m_mp2act;
    int   m_wisdom;
    float m_radius;

    std::string m_use_sound;
    std::string m_hold_model;

    friend class EquipmentInfoReader;
};

//
// фабрика для создания информации
//
class ThingInfoFactory{
public:

    //создать предмет типа type, с типом rid
    ThingInfo* Create(thing_type type, const rid_t& rid);
};

//
// хранилище информации о предметах
//
typedef InfoArchive<ThingInfo, thing_type, ThingInfoFactory> ThingInfoArchive;

//
// энциклопедия предметов для игрока
//
typedef InfoHandbook<ThingInfoArchive> ThingHandbook;

//
// базовый класс на Thing
//
class BaseThing{
public:
    
    BaseThing(thing_type type, const rid_t& rid);
    virtual ~BaseThing();

    DCTOR_ABS_DEF(BaseThing)

    //сохранить/загрузить объект
    virtual void MakeSaveLoad(SavSlot& st);

    //узнать/установить существо кот. принадлежит предмет
    BaseEntity* GetEntity();
    virtual void SetEntity(BaseEntity* ent);

    //узнать/установить ссылку на ящик в кот лежит предмет
    StoreBox* GetStoreBox();
    virtual void SetStoreBox(StoreBox* box);

    //установить/узнать tid_t
    void  SetTID(tid_t tid) { m_tid = tid;  }
    tid_t GetTID() const    { return m_tid; }

    //получить/установить координаты предмета в упаковке
    const ipnt2_t& GetPakPos() const;
    void SetPakPos(const ipnt2_t& pnt);

    //получить стоимость предмета
    virtual int GetCost() const;
    //получить вес предмета
    virtual float GetWeight() const;

    //получить доступ к информации о предмете
    const ThingInfo* GetInfo() const;

private:

    tid_t   m_tid;
    ipnt2_t m_pak_pos;

    ThingInfo*  m_info; 
    BaseEntity* m_entity;
    StoreBox*   m_store_box;
};

//
// операторы сравнения предметов
//
bool operator < (const BaseThing& b1, const BaseThing& b2);

inline bool operator != (const BaseThing& b1, const BaseThing& b2) { return b1 < b2 || b2 < b1; }
inline bool operator == (const BaseThing& b1, const BaseThing& b2) { return !(b1 < b2) && !(b2 < b1); }

//
// класс на патроны
//
class AmmoThing : public BaseThing{
public:

    AmmoThing(const rid_t& rid = rid_t());
    ~AmmoThing();

    DCTOR_DEF(AmmoThing)

    //количество патрон в обойме
    int GetCount() const; 
    //добавить патронов в обойму
    void SetCount(int count);
    
    //узнать стоимость
    int GetCost() const;
    //узнать вес передмета
    float GetWeight() const;

    //сохранение/загрузка
    void MakeSaveLoad(SavSlot& st);   

    //получить ссылку на информацию
    const AmmoInfo* GetInfo() const;

private:

    int m_count;
};

//
// класс на оружие
//
class WeaponThing : public BaseThing{
public:

    WeaponThing(const rid_t rid = rid_t());
    ~WeaponThing();

    DCTOR_DEF(WeaponThing)

    //сохранение/загрузка
    void MakeSaveLoad(SavSlot& st);   

    //при смене хозяина сменить хозяина и для патронов
    void SetEntity(BaseEntity* ent);
    void SetStoreBox(StoreBox* box);

    //получить ссылку на патроны
    AmmoThing* GetAmmo() const;
    //зарядить оружие, патронами
    AmmoThing* Load(AmmoThing* new_ammo);

    //узнать стоимость
    int GetCost() const;
    //узнать вес
    float GetWeight() const;
    //получить ссылку на информацию
    const WeaponInfo* GetInfo() const; 

    //узнать кол-во ходов на выстрел
    int GetMovepnts(shot_type type) const;
    //узнать кол-во выстрелов в соотв режиме
    int GetShotCount(shot_type type) const;
    
private:

    AmmoThing*   m_ammo;    
};

//
//класс на гранату
//
class GrenadeThing : public BaseThing{
public:

    GrenadeThing(const rid_t& rid = rid_t());
    ~GrenadeThing();

    DCTOR_DEF(GrenadeThing)

    //получить ссылку на информацию
    const GrenadeInfo* GetInfo() const;
};

//
//класс на Implants
//
class ImplantThing : public BaseThing{
public:

    ImplantThing(const rid_t& rid = rid_t());
    ~ImplantThing();

    DCTOR_DEF(ImplantThing)

    //получить ссылку на информацию
    const ImplantInfo* GetInfo() const;
};


//
// класс на аптечку
//
class MedikitThing: public BaseThing{
public:

    MedikitThing(const rid_t& rid = rid_t());
    ~MedikitThing();

    DCTOR_DEF(MedikitThing)
   
    void MakeSaveLoad(SavSlot& st);   

    //узнать заряд аптечки
    int GetCharge() const;
    //зарядить аптечку
    void SetCharge(int charge);
    //получить ссылку на информацию
    const MedikitInfo* GetInfo() const;

private:
    
    int m_charge;    
};

//
//класс на броники
//
class  ArmorThing : public BaseThing{
public:

    ArmorThing(const rid_t& rid = rid_t());
    ~ArmorThing();

    DCTOR_DEF(ArmorThing)

    //вычислмть коэф. комсанции
    float GetAbsorber(damage_type type, body_parts_type part, bool fback) const;
    //получить сылку на информацию о предмете
    const ArmorInfo* GetInfo() const;
};

//
//класс на деньги
//
class MoneyThing : public BaseThing{
public:

    MoneyThing(int sum = 0);
    ~MoneyThing();

    DCTOR_DEF(MoneyThing)
   
    void MakeSaveLoad(SavSlot& st);   

    //узнать сумму денег
    int GetSum() const;
    //установить сумму денег
    void SetSum(int sum);

    const MoneyInfo* GetInfo() const;

private:

    int m_sum;
};

//
// класс на ключи
//
class KeyThing : public BaseThing{
public:

    KeyThing(const rid_t& rid = rid_t());
    ~KeyThing();

    DCTOR_DEF(KeyThing)

    const KeyInfo* GetInfo() const;
};

//
// класс на камеру
//
class CameraThing : public BaseThing{
public:

    CameraThing(const rid_t& rid = rid_t());
    
    DCTOR_DEF(CameraThing)

    //получить информацию о предмете
    const CameraInfo* GetInfo() const;
};

//
// класс на силовое поле
//
class ShieldThing : public BaseThing{
public:
    
    ShieldThing(const rid_t& rid = rid_t());

    DCTOR_DEF(ShieldThing)

    //получить информацию о предмете
    const ShieldInfo* GetInfo() const;
};

//
// предмет - сканнер
//
class ScannerThing : public BaseThing{
public:

    ScannerThing(const rid_t& rid = rid_t());

    DCTOR_DEF(ScannerThing)

    //получить информацию о предмете
    const ScannerInfo* GetInfo() const;    
};

//
// Собственно склад предметов на уровне
//
class Depot{
public:

    ~Depot();

    //singleton
    static Depot* GetInst();

    //сбросить список
    void Reset();
    //сохранить/загрузить список
    void MakeSaveLoad(SavSlot& st);
    //добавить предмет в список
    tid_t Push(const ipnt2_t& pnt, BaseThing* bit);
    //получить ссылка на предмет по id
    BaseThing* Get(tid_t tid);
    
    enum rm_mode{
        RM_DELETE,  //удалить предмет
        RM_UNLINK,  //просто отсоед. предмет
    };

    //удалить предмет из списка, зная его id
    void Remove(tid_t tid, rm_mode mode = RM_UNLINK);

    //
    //инструменты для прохода по списку
    //
    class Iterator;
    typedef Iterator iterator;

    //удалить предмет, по итератору
    void Remove(iterator it, rm_mode mode = RM_UNLINK);

    //итерация по предметам в поле видимости существа
    iterator begin(BaseEntity* entity, unsigned type = TT_ALL_ITEMS);
    //итерация по предметам в опред радиусе вокруг точки
    iterator begin(const ipnt2_t& center, float radius = 1.0f, unsigned type = TT_ALL_ITEMS);

    iterator end();

protected:

    Depot();

private:

    friend class Iterator;

    struct info_s{
        BaseThing*  m_thing;
        GraphThing* m_graph;

        info_s() : m_thing(0), m_graph(0) {}
    };

    typedef std::vector<info_s> info_vec_t;
    info_vec_t m_infos;
};

class Depot::Iterator{
public:

    //условие итерации
    class Condition{
    public:
        
        virtual ~Condition(){}

        //нам подходит этот предмет?
        virtual bool IsSuitable(BaseThing* thing, GraphThing* graph) const = 0;
    };

    Iterator(Depot::info_vec_t* vec = 0, unsigned first = 0, Condition* condition = 0);
    ~Iterator();

    Iterator& operator++();
    Iterator  operator++(int)
    { Iterator tmp = *this; ++(*this); return tmp;}

    //доступ к вещи
    BaseThing* operator-> (){ return (*m_infos)[m_first].m_thing;   }
    BaseThing& operator * (){ return *((*m_infos)[m_first].m_thing);}

    //получить доступ к графич оболочке
    GraphThing* GetGraph() {return (*m_infos)[m_first].m_graph;}

    friend bool operator != (const Iterator& i1, const Iterator& i2)
    { return i1.m_first != i2.m_first; }

private:

    friend class Depot;

    Condition* m_condition;

    unsigned m_first;
    Depot::info_vec_t* m_infos;
};

#endif // _PUNCH_THING_H_