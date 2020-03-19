//
// утилиты для работы с AI
//

#ifndef _PUNCH_AIUTILS_H_
#define _PUNCH_AIUTILS_H_

struct damage_s;

class SpawnTag;
class Activity;

class BaseThing;
class AmmoThing;
class WeaponThing;
class ShieldThing;
class CameraThing;
class MedikitThing;
class GrenadeThing;

class Spectator;

class BaseEntity;
class HumanEntity;
class VehicleEntity;

class HumanContext;
class VehicleContext;

//
// таблица очков на levelup'ы
//
class LevelupMgr{
public:

    static LevelupMgr* GetInst();

    //узнать текущий уровень
    int GetCurrentLevel(int experience);
    //узнать кол-во очков на текущий level
    int GetLevelPoints(int level);

protected:

    LevelupMgr();

private:

    typedef std::vector<int> int_vec_t;
    int_vec_t m_points; 
};

//
// менеджер денег
//
class MoneyMgr{
public:

    static MoneyMgr* GetInst();

    void Reset();

    //узнеть количество денег
    int GetMoney() const;
    //установить количество денег
    void SetMoney(int count);
    //сохраниение/запись
    void MakeSaveLoad(SavSlot& slot);

private:

    MoneyMgr();

    int m_count;
};

//
// менеджер отслеживающий объекты на кот был Use
//
class GameObjectsMgr {
public:

    static GameObjectsMgr* GetInst();

    void Reset();

    //сохранить/восстановить объекты на уровне
    void MakeSaveLoad(SavSlot& st);

    //был ли объект использован
    bool IsUsed(const rid_t& rid) const;
    //был ли объект уничтожен
    bool IsDestroyed(const rid_t& rid) const;

    //пометить объект как ипользованный
    void MarkAsUsed(const rid_t& rid);
    //пометить оъект как убитый
    void MarkAsDestroyed(const rid_t& rid);

private:

    GameObjectsMgr(){}

private:

    rid_set_t m_used;
    rid_set_t m_destroyed;
};

//
// наблюдатель от команды игрока
//
class PlayerSpectator{
public:

    ~PlayerSpectator();

    static PlayerSpectator* GetInst();

    //получить указатель на наблюдателя
    Spectator* GetSpectator() { return m_spectator; } 

    //переиниц данные
    void Reset();
    //загрузка/сохранение
    void MakeSaveLoad(SavSlot& slot);

private:

    PlayerSpectator();

    Spectator* m_spectator;
};

//
// массив брошенных на уровнь камер
//
class CameraPool{
public:

    static CameraPool* GetInst();

    ~CameraPool();

    //обработать уход с уровня
    void Reset();
    //сохранение/загрузка камер
    void MakeSaveLoad(SavSlot& slot);
    //поместить камеру в массив
    void Insert(player_type player, const CameraThing* camera, const point3& pos);

    //обработка передачи хода
    void HandleNextTurn();

private:

    CameraPool(){}

private:

    typedef std::vector<Spectator*> cam_vec_t;
    cam_vec_t   m_cams;
};

//
// массив брошенных на землю силовых щитов
//
class ShieldPool{
public:

    //
    // информация для работы с щитом брощенным на уровень
    //
    class ShieldTraits{
    public:

        ShieldTraits(const std::string& effect, const point3& pos, float radius, int turns);
        ~ShieldTraits();

        //получить радиус
        float GetRadius() const { return m_radius; }
        //получить позицию
        const point3& GetPos() const { return m_pos; }
        //узнать название эффекта для щита
        const std::string& GetEffectName() const { return m_effect_name; }
        
        //учесть ход
        void DecTurns() { m_turns --; }
        //узнать кол-во ходов
        int GetTurns() const { return m_turns; }

    private:

        point3   m_pos;
        int      m_turns;
        float    m_radius;

        unsigned    m_effect;
        std::string m_effect_name;
    };

    static ShieldPool* GetInst();

    ~ShieldPool();

    //обработать выход с уровня
    void Reset();
    //сохрание / загрузка
    void MakeSaveLoad(SavSlot& slot);
    //поместить щит в массив
    void Insert(const ShieldThing* shield, const point3& pos);

    //обработка след хода
    void HandleNextTurn();

    class Iterator; typedef Iterator iterator;

    iterator end();
    iterator begin();

private:

    ShieldPool();

private:

    friend class Iterator;

    typedef std::vector<ShieldTraits*> info_vec_t;
    info_vec_t m_infos;
};

//
// итератор по массиву щитов
//
class ShieldPool::Iterator{
public:
    
    Iterator(info_vec_t* vec = 0, size_t first = 0);

    Iterator& operator ++();
    Iterator  operator ++(int)
    { Iterator tmp = *this; operator++(); return tmp; }
    
    //операторы для удобства работы
    const ShieldTraits* operator ->(){ return (*m_infos)[m_first];  }
    const ShieldTraits& operator * (){ return *(*m_infos)[m_first]; }
    
    //сравнение на !=
    friend bool operator != (const Iterator& i1, const Iterator& i2)
    { return i1.m_first != i2.m_first; } 
    
private:
    
    size_t      m_first;
    info_vec_t* m_infos;
};

//
// класс - список убитых существ
//
class DeadList{
public:

    //получить список убитых людей
    static DeadList* GetHeroesList();
    //получить список убитых торговцев
    static DeadList* GetTradersList();
    //получить список убитой техники
    static DeadList* GetVehiclesList();
    //получить список убитых для респауна
    static DeadList* GetRespawnerDeadList();

    //инициализация/деинициализация
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //добавить существо в список
    virtual void Insert(BaseEntity* entity) = 0;
    //сохранить/загрузить данные
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //узнвть сколько таких существ
    virtual int GetKilledCount(const SpawnTag& tag) const = 0;
};

//
// механизм проигрывания паники существа
//
class PanicPlayer{
public:

    virtual ~PanicPlayer(){}

    //singleton
    static PanicPlayer* GetInst();
    
    //установить существо 
    virtual void Init(BaseEntity* entity) = 0;
    //произвести расчет (вызывать до false)
    virtual bool Execute() = 0;

protected:

    PanicPlayer(){} 
};

//
// механизм давания предмета человеку
//
class ThingDelivery{
public:

    virtual ~ThingDelivery(){}

    //singleton
    static ThingDelivery* GetInst();

    enum reason_type{
       RT_NONE,

       RT_CAN_NOT_USE_SCUBA,

       RT_PACK_IS_FULL,
       RT_UNSUITABLE_PACK,
       RT_NOT_ENOUGH_WISDOM, 
       RT_NOT_ENOUGH_STRENGTH,
       RT_NOT_ENOUGH_MOVEPNTS,

       RT_CANNON_WITHOUT_SPACESUIT,
       RT_SPACESUIT_WITH_NO_CANNON,
    };

    //получить строковое описание причины отказа
    static std::string GetReason(reason_type reason);

    enum scheme_type{
        ST_GROUND,  //расчет для предмета с земли
        ST_USUAL,   //расчет для обычного предмета
    };

    //активировать выданный предмет (вызывать после выдачи)
    virtual void Activate(scheme_type scheme, HumanEntity* human, BaseThing* thing, human_pack_type hpk) = 0;

    //можно ли предмет использовать совместно с предметом
    virtual bool IsSuitable(BaseThing* thing, BaseThing* second) const = 0;
    //может ли человек использовать предмет
    virtual bool IsSuitable(HumanEntity* human, BaseThing* thing) const = 0;

    //можно ли положить предмет в слот
    virtual bool CanInsert(scheme_type scheme, HumanEntity* human, BaseThing* thing, human_pack_type hpk, reason_type* res = 0) const = 0;

protected:
    
    ThingDelivery(){}
};

//
// информация об уровнях на кот мы были
//
class EpisodeMapMgr{
public:

    static EpisodeMapMgr* GetInst();

    //карта обновлена?
    bool IsUpdated() const;
    //сбросить признак обновления
    void DropUpdatedFlag();

    //сбросить содержимое списка
    void Reset();
    //добавить уровень в список
    void Insert(const rid_t& rid);
    //сохранение/восставление
    void MakeSaveLoad(SavSlot& slot);

    class Iterator{
    public:

        Iterator(const rid_set_t::iterator& first) : m_first(first) {}

        const rid_t& operator*() { return *m_first; }
        
        Iterator& operator++ ()  { m_first++; return *this; }
        Iterator  operator++ (int) { Iterator it = *this; ++(*this); return it;}

        friend bool operator != (const Iterator& it1, const Iterator& it2)
        { return it1.m_first != it2.m_first; }
        
    private:

        rid_set_t::iterator m_first;
    };
        
    typedef Iterator iterator;

    //инструменты для прохода по списку
    iterator begin();
    iterator end();

private:

    EpisodeMapMgr();

private:

    rid_set_t m_levels;
    bool      m_fupdated;
};

//
// хранилище новостей
//
class NewsPool {
public:

    virtual ~NewsPool(){}

    //singleton
    static NewsPool* GetInst();

    //инициализация
    virtual void Init() = 0;
    //деинициализация
    virtual void Shut() = 0;

    //
    //информация о новости
    //
    class Info{
    public:

        virtual ~Info(){}

        virtual const rid_t& GetRID() const = 0;

        virtual const std::string& GetDesc() const = 0;
        virtual const std::string& GetShader() const = 0;
        virtual const std::string& GetChannel() const = 0;
    };

    //добавить новость в хранилище
    virtual void Push(const rid_t& rid) = 0;
    //сохранение/загрузка новостей
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //
    // полиморфный итератор по предметам
    // 
    class Iterator{
    public:
        
        virtual ~Iterator(){}

        //перейти на следущий элемент 
        virtual void Next() = 0;
        //перейти на первый элемент
        virtual void First() = 0;

        //получить доступ к предмету
        virtual const Info* Get() = 0;
        //итерирование еще не закончено?
        virtual bool IsNotDone() const = 0;
    };

    //породить полиморфный итератор
    virtual Iterator* CreateIterator() = 0;  
    
protected:

    NewsPool(){}    
};

//
// хранилище записей дневника
//
class DiaryManager{
public:

    //singleton
    static DiaryManager* GetInst();

    typedef int key_t;

    enum {
        ROOT_KEY = 0,
        NULL_KEY = -1,
    };

    //
    // запись в дневнике
    //
    class Record{
    public:

        Record(const std::string& info = std::string(), key_t key_up = ROOT_KEY,
               unsigned flags = 0, key_t key = ROOT_KEY):
            m_info(info), m_key(key), m_key_up(key_up), m_flags(flags){}

        key_t GetKey() const { return m_key; }
        key_t GetKeyUp() const { return m_key_up; }
        const std::string& GetInfo() const { return m_info; }

        //устанавливается при вставке в таблицу
        void SetKey(key_t key) { m_key = key; }

        enum flag_type{
            F_NEW = 1 << 0, //признак новой записи
        };

        //работа с флагами
        void RaiseFlags(unsigned flags) { m_flags |= flags; }
        void DropFlags(unsigned flags) { m_flags &= ~flags; }
        bool IsRaised(unsigned flags) const { return (m_flags & flags) != 0; }

        friend SavSlot& operator << (SavSlot& slot, const Record& rec)
        { 
            slot << rec.m_key << rec.m_key_up << rec.m_info << rec.m_flags; 
            return slot;
        }

        friend SavSlot& operator >> (SavSlot& slot, Record& rec)
        {
            slot >> rec.m_key >> rec.m_key_up >> rec.m_info >> rec.m_flags;
            return slot;
        }

        void SetInfo(const std::string& info) { m_info = info; }

    private:

        key_t m_key;
        key_t m_key_up;

        std::string m_info;
        unsigned    m_flags;
    };

    //получить ссылку на запись
    virtual Record* Get(key_t key) = 0;
    //удалить запись
    virtual void Delete(key_t key) = 0;
    //добавить запись
    virtual key_t Insert(const Record& rec) = 0;

    //сохранить/загрузить дневник
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //
    // итератор 
    //
    class Iterator{
    public:

        virtual ~Iterator(){}

        //перейти на следущий элемент 
        virtual void Next() = 0;
        //перейти на первый элемент
        virtual void First(key_t key_up = NULL_KEY) = 0;

        //получить доступ к предмету
        virtual Record* Get() = 0;
        //итерирование еще не закончено?
        virtual bool IsNotDone() const = 0;
    };

    //создать итератор
    virtual Iterator* CreateIterator() = 0;

protected:

    DiaryManager(){}
};

//
// управление помощником
//
class HelperManager{
public:

    virtual ~HelperManager(){}

    //получить указатель на инстанцию
    static HelperManager* GetInst();

    //инициализация/деинициализация
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //обработка сообщений
    virtual void Tick() = 0;
    //сохранение/загрузка helper'a
    virtual void MakeSaveLoad(SavSlot& slot) = 0;
};

namespace AIUtils{

    enum reason_type{
        RT_NONE,          //нет причины
        RT_NO_AMMO,       //нет патронов
        RT_NO_WEAPON,     //нет оружия
        RT_NO_MOVEPNTS,   //нет ходов
        RT_OUT_OF_RANGE,  //вне пределов досягаемости
        RT_NOT_THROWABLE, //этот предмет не бросается
    };

    //спрятать hex'сы
    void HideLandField();
    void HidePassField();

    //структура описывающая поле видимости
    struct fos_s{
        float m_back;
        float m_front;
        float m_angle;
    };

    //получить поле видимости существа
    void GetEntityFOS(BaseEntity* entity, fos_s* fos);

    //получить строку с числом для предмета
    std::string GetThingNumber(BaseThing* thing);
    
    //сохранение загрузка списка rid_t'ов
    void SaveLoadRIDSet(SavSlot& slot, rid_set_t& rid_set);

    //определить стойку человека
    void ChangeHumanStance(HumanEntity* human, BaseEntity* skip_entity = 0);

    //расчитать и показать активные зоны выхода
    void CalcAndShowActiveExits();

    //расчитать и показать поле фронт проходмости
    void CalcAndShowPassField(BaseEntity* entity);
    //расчитать и показать поле высадки из техники
    void CalcAndShowLandField(VehicleEntity* vehicle);

    //расчитать параметры существа после смерти союзника
    void CalcHumanAllyDeath(BaseEntity* entity);
    //расчитать кол-во ходов на начало хода
    void CalcMovepnts4Turn(BaseEntity* entity);

    //добавить опыта 
    void AddExp4Kill(int experience, BaseEntity* killer);
    void AddExp4Hack(int experience, HumanEntity* human);
    void AddExp4Quest(int experience, BaseEntity* actor);

    //можно ли подойти к существу за один ход
    bool IsAccessibleEntity(BaseEntity* walker, BaseEntity* near_ent);

    //перевод причины отказа в строку
    std::string Reason2Str(reason_type reason);
    //перевод типа стрельбы в строку
    std::string Shot2Str(shot_type type);
    //перевод строки в тип поражения
    damage_type Str2Dmg(const std::string& str);
    //перевод поражения в строку
    std::string Dmg2Str(damage_type type, bool full = true);

    //сменить команду существа
    void ChangeEntityPlayer(BaseEntity* entity, player_type new_player);

    //вычислить вероятность взлома техники
    float CalcHackProbability(HumanEntity* actor, VehicleEntity* vehicle);

    //пролучить нормированное значение точности
    float NormalizeAccuracy(BaseEntity* entity, int accuracy);

    //узнать точность для стрельбы в опред. точку опред оружием (оружие д.б. заряжено)
    int CalcShootAccuracy(HumanEntity* human, WeaponThing* weapon, const point3& to);
    //можно ли стрелять существом из этого оружия в заданную точку
    bool CanShoot(VehicleEntity* vehicle, const point3& to, reason_type* reason = 0);
    bool CanShoot(HumanEntity* human, WeaponThing* weapon, const point3& to, reason_type* reason = 0);

    //вернуть дальность броска гранаты
    float CalcThrowRange(HumanEntity* human);
    //может ли человек бросить предмет?
    bool CanThrow(HumanEntity* human, BaseThing* thing, const point3& to, reason_type* reason = 0);

    //может ли guard отреагировать на intruder
    bool CanReact(BaseEntity* intruder, BaseEntity* guard);
    //может ли человек сменить позу
    bool CanChangePose(HumanEntity* human, reason_type* reason = 0);

    //расчитать кол-во очков для активации предмета
    int CalcMps2Act(BaseThing* thing);
    //расчитаить кол-во ходов на поднятие предмета c земли
    int CalcMps4ThingPickUp(BaseThing* thing);
    //расчитать кол-во ходов для поворота существа
    int CalcMps4EntityRotate(BaseEntity* entity, const point3& dir);
    //расчитать кол-во очков для levelup'a
    int CalcLevelupPoints(HumanEntity* human, int cur_lvl, int old_lvl);

    //произвести лечение
    void MakeTreatment(HumanEntity* doctor, HumanEntity* sick, MedikitThing* medikit);

	// возвращает двумерное (по Х и У) расстояние между трехмерными точками
	float dist2D(const point3& p1, const point3& p2);
}

#endif // _PUNCH_AIUTILS_H_