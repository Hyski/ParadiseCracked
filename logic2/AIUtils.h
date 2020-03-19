//
// ������� ��� ������ � AI
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
// ������� ����� �� levelup'�
//
class LevelupMgr{
public:

    static LevelupMgr* GetInst();

    //������ ������� �������
    int GetCurrentLevel(int experience);
    //������ ���-�� ����� �� ������� level
    int GetLevelPoints(int level);

protected:

    LevelupMgr();

private:

    typedef std::vector<int> int_vec_t;
    int_vec_t m_points; 
};

//
// �������� �����
//
class MoneyMgr{
public:

    static MoneyMgr* GetInst();

    void Reset();

    //������ ���������� �����
    int GetMoney() const;
    //���������� ���������� �����
    void SetMoney(int count);
    //�����������/������
    void MakeSaveLoad(SavSlot& slot);

private:

    MoneyMgr();

    int m_count;
};

//
// �������� ������������� ������� �� ��� ��� Use
//
class GameObjectsMgr {
public:

    static GameObjectsMgr* GetInst();

    void Reset();

    //���������/������������ ������� �� ������
    void MakeSaveLoad(SavSlot& st);

    //��� �� ������ �����������
    bool IsUsed(const rid_t& rid) const;
    //��� �� ������ ���������
    bool IsDestroyed(const rid_t& rid) const;

    //�������� ������ ��� �������������
    void MarkAsUsed(const rid_t& rid);
    //�������� ����� ��� ������
    void MarkAsDestroyed(const rid_t& rid);

private:

    GameObjectsMgr(){}

private:

    rid_set_t m_used;
    rid_set_t m_destroyed;
};

//
// ����������� �� ������� ������
//
class PlayerSpectator{
public:

    ~PlayerSpectator();

    static PlayerSpectator* GetInst();

    //�������� ��������� �� �����������
    Spectator* GetSpectator() { return m_spectator; } 

    //�������� ������
    void Reset();
    //��������/����������
    void MakeSaveLoad(SavSlot& slot);

private:

    PlayerSpectator();

    Spectator* m_spectator;
};

//
// ������ ��������� �� ������ �����
//
class CameraPool{
public:

    static CameraPool* GetInst();

    ~CameraPool();

    //���������� ���� � ������
    void Reset();
    //����������/�������� �����
    void MakeSaveLoad(SavSlot& slot);
    //��������� ������ � ������
    void Insert(player_type player, const CameraThing* camera, const point3& pos);

    //��������� �������� ����
    void HandleNextTurn();

private:

    CameraPool(){}

private:

    typedef std::vector<Spectator*> cam_vec_t;
    cam_vec_t   m_cams;
};

//
// ������ ��������� �� ����� ������� �����
//
class ShieldPool{
public:

    //
    // ���������� ��� ������ � ����� ��������� �� �������
    //
    class ShieldTraits{
    public:

        ShieldTraits(const std::string& effect, const point3& pos, float radius, int turns);
        ~ShieldTraits();

        //�������� ������
        float GetRadius() const { return m_radius; }
        //�������� �������
        const point3& GetPos() const { return m_pos; }
        //������ �������� ������� ��� ����
        const std::string& GetEffectName() const { return m_effect_name; }
        
        //������ ���
        void DecTurns() { m_turns --; }
        //������ ���-�� �����
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

    //���������� ����� � ������
    void Reset();
    //�������� / ��������
    void MakeSaveLoad(SavSlot& slot);
    //��������� ��� � ������
    void Insert(const ShieldThing* shield, const point3& pos);

    //��������� ���� ����
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
// �������� �� ������� �����
//
class ShieldPool::Iterator{
public:
    
    Iterator(info_vec_t* vec = 0, size_t first = 0);

    Iterator& operator ++();
    Iterator  operator ++(int)
    { Iterator tmp = *this; operator++(); return tmp; }
    
    //��������� ��� �������� ������
    const ShieldTraits* operator ->(){ return (*m_infos)[m_first];  }
    const ShieldTraits& operator * (){ return *(*m_infos)[m_first]; }
    
    //��������� �� !=
    friend bool operator != (const Iterator& i1, const Iterator& i2)
    { return i1.m_first != i2.m_first; } 
    
private:
    
    size_t      m_first;
    info_vec_t* m_infos;
};

//
// ����� - ������ ������ �������
//
class DeadList{
public:

    //�������� ������ ������ �����
    static DeadList* GetHeroesList();
    //�������� ������ ������ ���������
    static DeadList* GetTradersList();
    //�������� ������ ������ �������
    static DeadList* GetVehiclesList();
    //�������� ������ ������ ��� ��������
    static DeadList* GetRespawnerDeadList();

    //�������������/���������������
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //�������� �������� � ������
    virtual void Insert(BaseEntity* entity) = 0;
    //���������/��������� ������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //������ ������� ����� �������
    virtual int GetKilledCount(const SpawnTag& tag) const = 0;
};

//
// �������� ������������ ������ ��������
//
class PanicPlayer{
public:

    virtual ~PanicPlayer(){}

    //singleton
    static PanicPlayer* GetInst();
    
    //���������� �������� 
    virtual void Init(BaseEntity* entity) = 0;
    //���������� ������ (�������� �� false)
    virtual bool Execute() = 0;

protected:

    PanicPlayer(){} 
};

//
// �������� ������� �������� ��������
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

    //�������� ��������� �������� ������� ������
    static std::string GetReason(reason_type reason);

    enum scheme_type{
        ST_GROUND,  //������ ��� �������� � �����
        ST_USUAL,   //������ ��� �������� ��������
    };

    //������������ �������� ������� (�������� ����� ������)
    virtual void Activate(scheme_type scheme, HumanEntity* human, BaseThing* thing, human_pack_type hpk) = 0;

    //����� �� ������� ������������ ��������� � ���������
    virtual bool IsSuitable(BaseThing* thing, BaseThing* second) const = 0;
    //����� �� ������� ������������ �������
    virtual bool IsSuitable(HumanEntity* human, BaseThing* thing) const = 0;

    //����� �� �������� ������� � ����
    virtual bool CanInsert(scheme_type scheme, HumanEntity* human, BaseThing* thing, human_pack_type hpk, reason_type* res = 0) const = 0;

protected:
    
    ThingDelivery(){}
};

//
// ���������� �� ������� �� ��� �� ����
//
class EpisodeMapMgr{
public:

    static EpisodeMapMgr* GetInst();

    //����� ���������?
    bool IsUpdated() const;
    //�������� ������� ����������
    void DropUpdatedFlag();

    //�������� ���������� ������
    void Reset();
    //�������� ������� � ������
    void Insert(const rid_t& rid);
    //����������/������������
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

    //����������� ��� ������� �� ������
    iterator begin();
    iterator end();

private:

    EpisodeMapMgr();

private:

    rid_set_t m_levels;
    bool      m_fupdated;
};

//
// ��������� ��������
//
class NewsPool {
public:

    virtual ~NewsPool(){}

    //singleton
    static NewsPool* GetInst();

    //�������������
    virtual void Init() = 0;
    //���������������
    virtual void Shut() = 0;

    //
    //���������� � �������
    //
    class Info{
    public:

        virtual ~Info(){}

        virtual const rid_t& GetRID() const = 0;

        virtual const std::string& GetDesc() const = 0;
        virtual const std::string& GetShader() const = 0;
        virtual const std::string& GetChannel() const = 0;
    };

    //�������� ������� � ���������
    virtual void Push(const rid_t& rid) = 0;
    //����������/�������� ��������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //
    // ����������� �������� �� ���������
    // 
    class Iterator{
    public:
        
        virtual ~Iterator(){}

        //������� �� �������� ������� 
        virtual void Next() = 0;
        //������� �� ������ �������
        virtual void First() = 0;

        //�������� ������ � ��������
        virtual const Info* Get() = 0;
        //������������ ��� �� ���������?
        virtual bool IsNotDone() const = 0;
    };

    //�������� ����������� ��������
    virtual Iterator* CreateIterator() = 0;  
    
protected:

    NewsPool(){}    
};

//
// ��������� ������� ��������
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
    // ������ � ��������
    //
    class Record{
    public:

        Record(const std::string& info = std::string(), key_t key_up = ROOT_KEY,
               unsigned flags = 0, key_t key = ROOT_KEY):
            m_info(info), m_key(key), m_key_up(key_up), m_flags(flags){}

        key_t GetKey() const { return m_key; }
        key_t GetKeyUp() const { return m_key_up; }
        const std::string& GetInfo() const { return m_info; }

        //��������������� ��� ������� � �������
        void SetKey(key_t key) { m_key = key; }

        enum flag_type{
            F_NEW = 1 << 0, //������� ����� ������
        };

        //������ � �������
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

    //�������� ������ �� ������
    virtual Record* Get(key_t key) = 0;
    //������� ������
    virtual void Delete(key_t key) = 0;
    //�������� ������
    virtual key_t Insert(const Record& rec) = 0;

    //���������/��������� �������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //
    // �������� 
    //
    class Iterator{
    public:

        virtual ~Iterator(){}

        //������� �� �������� ������� 
        virtual void Next() = 0;
        //������� �� ������ �������
        virtual void First(key_t key_up = NULL_KEY) = 0;

        //�������� ������ � ��������
        virtual Record* Get() = 0;
        //������������ ��� �� ���������?
        virtual bool IsNotDone() const = 0;
    };

    //������� ��������
    virtual Iterator* CreateIterator() = 0;

protected:

    DiaryManager(){}
};

//
// ���������� ����������
//
class HelperManager{
public:

    virtual ~HelperManager(){}

    //�������� ��������� �� ���������
    static HelperManager* GetInst();

    //�������������/���������������
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //��������� ���������
    virtual void Tick() = 0;
    //����������/�������� helper'a
    virtual void MakeSaveLoad(SavSlot& slot) = 0;
};

namespace AIUtils{

    enum reason_type{
        RT_NONE,          //��� �������
        RT_NO_AMMO,       //��� ��������
        RT_NO_WEAPON,     //��� ������
        RT_NO_MOVEPNTS,   //��� �����
        RT_OUT_OF_RANGE,  //��� �������� ������������
        RT_NOT_THROWABLE, //���� ������� �� ���������
    };

    //�������� hex'��
    void HideLandField();
    void HidePassField();

    //��������� ����������� ���� ���������
    struct fos_s{
        float m_back;
        float m_front;
        float m_angle;
    };

    //�������� ���� ��������� ��������
    void GetEntityFOS(BaseEntity* entity, fos_s* fos);

    //�������� ������ � ������ ��� ��������
    std::string GetThingNumber(BaseThing* thing);
    
    //���������� �������� ������ rid_t'��
    void SaveLoadRIDSet(SavSlot& slot, rid_set_t& rid_set);

    //���������� ������ ��������
    void ChangeHumanStance(HumanEntity* human, BaseEntity* skip_entity = 0);

    //��������� � �������� �������� ���� ������
    void CalcAndShowActiveExits();

    //��������� � �������� ���� ����� �����������
    void CalcAndShowPassField(BaseEntity* entity);
    //��������� � �������� ���� ������� �� �������
    void CalcAndShowLandField(VehicleEntity* vehicle);

    //��������� ��������� �������� ����� ������ ��������
    void CalcHumanAllyDeath(BaseEntity* entity);
    //��������� ���-�� ����� �� ������ ����
    void CalcMovepnts4Turn(BaseEntity* entity);

    //�������� ����� 
    void AddExp4Kill(int experience, BaseEntity* killer);
    void AddExp4Hack(int experience, HumanEntity* human);
    void AddExp4Quest(int experience, BaseEntity* actor);

    //����� �� ������� � �������� �� ���� ���
    bool IsAccessibleEntity(BaseEntity* walker, BaseEntity* near_ent);

    //������� ������� ������ � ������
    std::string Reason2Str(reason_type reason);
    //������� ���� �������� � ������
    std::string Shot2Str(shot_type type);
    //������� ������ � ��� ���������
    damage_type Str2Dmg(const std::string& str);
    //������� ��������� � ������
    std::string Dmg2Str(damage_type type, bool full = true);

    //������� ������� ��������
    void ChangeEntityPlayer(BaseEntity* entity, player_type new_player);

    //��������� ����������� ������ �������
    float CalcHackProbability(HumanEntity* actor, VehicleEntity* vehicle);

    //��������� ������������� �������� ��������
    float NormalizeAccuracy(BaseEntity* entity, int accuracy);

    //������ �������� ��� �������� � �����. ����� ����� ������� (������ �.�. ��������)
    int CalcShootAccuracy(HumanEntity* human, WeaponThing* weapon, const point3& to);
    //����� �� �������� ��������� �� ����� ������ � �������� �����
    bool CanShoot(VehicleEntity* vehicle, const point3& to, reason_type* reason = 0);
    bool CanShoot(HumanEntity* human, WeaponThing* weapon, const point3& to, reason_type* reason = 0);

    //������� ��������� ������ �������
    float CalcThrowRange(HumanEntity* human);
    //����� �� ������� ������� �������?
    bool CanThrow(HumanEntity* human, BaseThing* thing, const point3& to, reason_type* reason = 0);

    //����� �� guard ������������� �� intruder
    bool CanReact(BaseEntity* intruder, BaseEntity* guard);
    //����� �� ������� ������� ����
    bool CanChangePose(HumanEntity* human, reason_type* reason = 0);

    //��������� ���-�� ����� ��� ��������� ��������
    int CalcMps2Act(BaseThing* thing);
    //���������� ���-�� ����� �� �������� �������� c �����
    int CalcMps4ThingPickUp(BaseThing* thing);
    //��������� ���-�� ����� ��� �������� ��������
    int CalcMps4EntityRotate(BaseEntity* entity, const point3& dir);
    //��������� ���-�� ����� ��� levelup'a
    int CalcLevelupPoints(HumanEntity* human, int cur_lvl, int old_lvl);

    //���������� �������
    void MakeTreatment(HumanEntity* doctor, HumanEntity* sick, MedikitThing* medikit);

	// ���������� ��������� (�� � � �) ���������� ����� ����������� �������
	float dist2D(const point3& p1, const point3& p2);
}

#endif // _PUNCH_AIUTILS_H_