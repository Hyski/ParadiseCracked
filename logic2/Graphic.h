//
// Модуль соединяющий логику с графической частью игры
//

#ifndef _PUNCH_GRAPHIC_H_
#define _PUNCH_GRAPHIC_H_

#ifndef _PUNCH_ENTITYAUX_H_
#include "EntityAux.h"
#endif

class KeyAnimation;

class NetDriver;
class GraphHexGrid;
class ScatteredItem;

class BaseThing;
class BaseEntity;

class GraphHuman;
class GraphVehicle;

class EntityObserver;
class CallbackBridge;

//
// утилиты для работы с графикой
//
namespace GraphUtils{
    //находися ли существо в окрестности точки
    bool IsInVicinity(const point3& pos, const ipnt2_t& pnt, float eps = 0.3);
    //удалить существ проигрывающих анимацию смерти
    void DestroyCorpses();
}

//
// callbacks событий при ходьбе
//
class MoveEvents{
public:

    enum cmd_type{
        CT_MOVE,     //продолжить движение
        CT_STOP,     //прекратить движение
        CT_SPEED_UP, //ускорить движение
    };

    enum event_type{
        ET_STEP,     //сделан переход в новый hex
        ET_END,      //был переход в конечную точку пути
    };

    //уведомление о переходе в новый hex
    virtual cmd_type OnStep(event_type event) = 0;
};

//
// оболочка на графический hex_grid
//
class GraphGrid{
public:

    ~GraphGrid();

    //получить инстанцию класса
    static GraphGrid* GetInst();

    enum hex_type{
        HT_PATH,
        HT_FRONT,
        HT_JOINTS,
        HT_LAND,

        HT_MAX,
    };

    //уведомление об изменении опций
    void HandleOptionsChange();

    //отрисовать
    void Draw();
    //показать / спрятать массив точек
    void Show(hex_type type, bool flag);
    //установить поле hex'совое поле
    void SetHexes(hex_type type, const pnt_vec_t& vec);

protected:

    GraphGrid();

private:
    
    bool m_fshow[HT_MAX];

    GraphHexGrid* m_graph;
};

//
// оболочка вокруг эффектов
//
class EffectMgr{
public:

    //singleton
    static EffectMgr* GetInst();

    ~EffectMgr();

    //эффект попадания
    void MakeHit(const rid_t& name, const point3& from, const point3& to);
    //эффект выстрела из оружия
    void MakeFlash(const rid_t& name, const point3& from, const point3& to);
    //эффект полета пули
    float MakeTracer(const rid_t& name, const point3& from, const point3& to);
    //эффект на бросок гранаты
    float MakeGrenadeFlight(const KeyAnimation& ka, const std::string& skin, point3* end);

    //показать эффект лечения для человека
    void MakeHumanTreatment(const GraphHuman* graph);
    //взорвать технику
    void MakeVehicleExplosion(const GraphVehicle* graph);

    //показать лужу крови
    void MakeBloodSpot(const point3& where);
    //показать брызги крови
    void MakeHumanBlood(const point3& from, const point3& to);
    //показать вылет гильз
    void MakeShellOutlet(BaseEntity* entity, const rid_t& name, const point3& from, const point3& to);

    //создать постоянный эффект в опед точке уровня
    unsigned MakeConst(const rid_t& rid, const point3& to);
    //удалить постоянный эффект
    void DestroyConst(unsigned id);

    //показать эффект взрыва человека
    void MakeMeat(const GraphHuman* human, const point3& from);
    
protected:

    EffectMgr();
};

//
// Абстракция предмета лежащего на уровне
//
class GraphThing{
public:

    GraphThing(gid_t gid = 0, BaseThing* thing = 0);
    ~GraphThing();

    //запретить/разрешить разрушение оболочек
    static void EnableDestroy(bool flag);

    DCTOR_DEF(GraphThing)
    //сохранение/загрузка предмета
    void MakeSaveLoad(SavSlot& st);

    //узнать позицию предмета на уровне
    point3  GetPos3() const;
    ipnt2_t GetPos2() const;

    //получить ссылку на родителя
    BaseThing* GetParent() {return m_thing;}
    //получить графич идент. предмета
    gid_t GetGID() const {return m_gid;}

private:

    BaseThing* m_thing;
    gid_t      m_gid; 

    static bool m_fcan_destroy;
};

//
// Абстракция графического существа
//
class GraphEntity : public EntityObserver{
public:
    GraphEntity(gid_t gid, BaseEntity* entity);
    virtual ~GraphEntity();

    DCTOR_ABS_DEF(GraphEntity)
    //сохранить/загрузить графич оболочку
    virtual void MakeSaveLoad(SavSlot& st);

    //обновить ывсе графич оболочки
    static void Update();

    enum flag_type{
        FT_ENABLE_DESTROY = 1 << 0, //запретить разрушение графических оболочек
        FT_ALWAYS_VISIBLE = 1 << 1, //сделать все графич оболочки видимыми всегда

        FT_INIT = FT_ENABLE_DESTROY
    };

    //работа с флагами
    static bool IsRaised(unsigned flag);
    static void DropFlags(unsigned flag);
    static void RaiseFlags(unsigned flag);

    enum decor_type{
        DT_FOS,
        DT_BANNER,
    };

    //разрешить/запретить отрисовку украшений 
    static void ShowDecor(decor_type type, bool flag);

    //уничтожить графич. оболочку
    void Destroy();

    //получить указатели на расширенный интерфейс
    virtual GraphHuman* Cast2Human(){return 0;}
    virtual GraphVehicle* Cast2Vehicle(){return 0;}

    //узнать id графич оболочки
    gid_t GetGID() const {return m_gid;}
    //ссылка на существо обладающее этой оболочкой
    BaseEntity* GetParent() const {return m_entity;}
    
    //надо для трассировки луча
    bool TraceRay(const ray &r, float *Dist, point3 *Norm,bool AsBox=false);
    
    //установить позицию существа
    void SetLoc(const point3& pos, float angle);
    void SetLoc(const ipnt2_t& pnt, float angle);
    
    //установить/получить угол поворота
    float SetAngle(float angle);
    float GetAngle() const;

    //нужен ли rotate для поворота существа в направлении dir
    bool NeedRotate(const point3& dir);

    //узнать положение существа
    point3  GetPos3() const;
    ipnt2_t GetPos2() const;

    //получить набор видимых точек существа
    void GetVisPoints(pnt3_vec_t* pnts) const;
    //расчитать точку выстрела в существо
    point3 GetShotPoint(const point3& hint = NULLVEC) const;

    //узнать координаты ствола пушки
    void GetBarrel(const point3& barrel, point3* from);
    //узнать координаты для вылета патронов
    void GetShellOutlet(const point3& offset, point3* from);

    //операции для раоботы с видимостью с точки зрения логики
    void Visible(bool flag);
    bool IsVisible() const;

    //указать куда двигаться существу
    void MoveTo(const ipnt2_t& to);
    
    enum action_type{
        AT_USE,
        AT_MOVE,
        AT_STAY,
        AT_DEAD,
        AT_HURT,
        AT_TURN,
        AT_FEAR,
        AT_FALL,
        AT_RELOAD,
        AT_SPECIAL,

        AT_LANDING,  //высадка из техники
        AT_SHIPMENT, //посадка в технику

        AT_SHOOT,
        AT_RECOIL,

        AT_AIMSHOOT,
        AT_AIMRECOIL,
    };

    //установить на проигрывание анимацию
    virtual float ChangeAction(action_type action);
    //сделать перемещение существ
    virtual void Move(const pnt_vec_t& path, MoveEvents* events) = 0;

    //типы эффектов для существ
    enum effect_type{
        ET_FLAME,
        ET_SHOCK,
        ET_ELECTRIC,
        ET_SELECTION,

        ET_MAX,
    };

    //режим отображения эффекта
    enum show_mode{
        SM_NONE,
        SM_SHOW,        //показать эффект
        SM_HIDE,        //спрятать эффект
        SM_SLOW_HIDE,   //медленно спрятать эффект
    };

    //показать эффект на графической оболочке
    void ShowEffect(effect_type type, show_mode mode);
                                                      
protected:

    //присоед. наблюдателя
    void LinkObserver();

    //обновить графич оболочку    
    virtual void UpdateEntity();

    enum update_type{
        UT_FOS         = 1 << 0,  //обновить поле видимости
        UT_ARMOR       = 1 << 1,  //обновить броник на человеке
        UT_HANDS       = 1 << 2,  //обновить содержимое в руках
        UT_BANNER      = 1 << 3,  //обновить надпись
        UT_SELECTION   = 1 << 4,  //обновить выделение
        UT_DMG_EFFECT  = 1 << 5,  //обновить эффект на повреждение
        UT_BASE_FLAGS  = 1 << 6,  //изменились флаги на графич оболочки
        UT_TEAM_CHANGE = 1 << 7,  //смена команды существа

        UT_ALL = UT_FOS|UT_ARMOR|UT_HANDS|UT_BANNER|UT_SELECTION|UT_DMG_EFFECT|UT_BASE_FLAGS,
    };

    //работа с флагами обновления существа
    unsigned GetUpdateFlags() const { return m_update_flags; }
    //наблюдатель за существом
    void Update(BaseEntity* entity, event_t event, info_t info);

private:

    gid_t      m_gid;
    bool       m_visible;
    show_mode  m_show_modes[ET_MAX]; 
    unsigned   m_update_flags;

    BaseEntity*  m_entity;
    GraphEntity* m_next;

    static float m_update_time;
    static float m_update_delta;

    static GraphEntity* m_first;
    static unsigned     m_flags;
};

//
// Графический человек
//
class GraphHuman : public GraphEntity{
public:

    GraphHuman(gid_t gid = 0, BaseEntity* human = 0);
    ~GraphHuman();

    DCTOR_DEF(GraphHuman)
    //сохранить/загрузить графич. человека
    void MakeSaveLoad(SavSlot& st);

    GraphHuman* Cast2Human(){return this;}

    enum pose_type{
        PT_NONE,
        PT_SIT,
        PT_STAND,
    };

    enum move_type{
        MT_NONE,
        MT_RUN,
        MT_WALK,
    };

    enum stance_type{
        ST_NONE,
        ST_WAR,
        ST_USUAL,
    };

    //сменить стойку
    void ChangePose(pose_type type);
    //изменить тип передвижения
    void ChangeMoveType(move_type type);
    //установить стойку
    void ChangeStance(stance_type type);
 
    //человек сидит
    bool IsSitPose() const {return m_pose == PT_SIT;}
    //человек должен бегать
    bool IsRunMove() const {return m_move == MT_RUN;}

    //спец обработка нек. анимаций
    float ChangeAction(action_type action);
    //перемещение существа по нек пути
    void Move(const pnt_vec_t& path, MoveEvents* events);

    //типы звуков которые можно играть
    enum sound_type{
        SND_NO_LOF,
        SND_CRITICAL_HURT,
        SND_LUCKY_STRIKE,
        SND_DEATH,
        SND_ENEMY_SIGHTED,
        SND_PRETTY_GIRL,
        SND_SELECT_COMMAND,
        SND_MOVE_COMMAND,
        SND_ATTACK_COMMAND,
        SND_CRITICAL_MISS,
        SND_UNDER_FIRE,
        SND_TREATMENT_MEDIUM,
        SND_TREATMENT_LOW,
        SND_TREATMENT_HIGH,
        SND_TEAMMATE_NEW,
        SND_TEAMMATE_COME,
        SND_TEAMMATE_LEAVE,
        SND_TEAMMATE_DISMISS,
        SND_ENEMY_DIED,
        SND_ENEMY_EXPLODED,
        SND_OPEN_SUCCEED,
        SND_OPEN_FAILED,
        SND_CAR_DRIVE_FAILED,
        SND_LEVELUP,        
        SND_GOT_SHOTGUN,
        SND_GOT_ROCKET_LAUNCHER,
        SND_GOT_AUTOCANNON,
        SND_GOT_ENERGY_GUN,
        SND_USE_PLASMA_GRENADE,
        SND_TERMOPLASMA_EXPLOSION,
        SND_INSERT_SCANNER, 
        SND_REMOVE_SCANNER,
    };

    //проиграть звук соотв типа
    void PlaySound(sound_type sound);

private:

    //обновить графич. оболочку
    void UpdateEntity();
    //установить оружие
    void SetWeapon(hold_type type, const std::string& name);
    //установить броник
    void SetSuit(armor_type suit_type);

private:

    hold_type   m_hold;
    move_type   m_move;
    pose_type   m_pose;
    stance_type m_stance;

    CallbackBridge* m_bridge;
};

//
// Графическая техника
//
class GraphVehicle : public GraphEntity{
public:

    GraphVehicle(gid_t gid = 0, BaseEntity* vehicle = 0);
    ~GraphVehicle();

    DCTOR_DEF(GraphVehicle)

    GraphVehicle* Cast2Vehicle(){return this;}

    //перемещение существа по нек пути
    void Move(const pnt_vec_t& path, MoveEvents* events);

private:

    //обновить графич оболочку
    void UpdateEntity();

private:

    CallbackBridge* m_bridge;
};

//
// фабрика графических оболочек
//
class GraphFactory{
public:

    //singleton
    static GraphFactory* GetInst();
    
    //создать графический предмет
    GraphThing* CreateGraphThing(BaseThing* thing, const ipnt2_t& pos);
    //создать графического человека
    GraphHuman* CreateGraphHuman(BaseEntity* human);
    //создать графическую технику
    GraphVehicle* CreateGraphVehicle(BaseEntity* vehicle);

protected:

    GraphFactory(){}
};

//~~~~~~~~~~~~~~~~~ inlines ~~~~~~~~~~~~~~~~~

inline bool GraphEntity::IsVisible() const
{
    return m_visible;
}

#endif // _PUNCH_GRAPHIC_H_