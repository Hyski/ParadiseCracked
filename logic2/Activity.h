//
// интерфейс на действия
//

#ifndef _PUNCH_ACTIVITY_H_
#define _PUNCH_ACTIVITY_H_

class Activity;

class BaseEntity;
class HumanEntity;
class VehicleEntity;

//
// команды для управления действиями
//
enum activity_command{
    AC_STOP,     //остановить действие
    AC_TICK,     //выполнить обычный обсчет действия
    AC_SPEED_UP, //ускорить выполнение действия
};

//
// наблюдатель за действием
//
class ActivityObserver : public Observer<Activity*>{
public:

    enum event_type{
        EV_NONE, 

        EV_NO_LOF,            //нет линнии стрельбы
        EV_MEET_ENTITY,       //существо наткнулось на кого-то  
        EV_DANGER_ZONE,       //существо зашло в опасную зону
        EV_ENEMY_REACTED,     //на существо среагировал враг 
        EV_ENEMY_SPOTTED,     //остановка замечен враг
        EV_STEPS_EXPIRIED,    //у существа нет больше шагов (м.б. перегруз человека)
        EV_MOVEPNTS_EXPIRIED, //у существа кончились ходы
    };

    //информация о замеченном враге
    struct enemy_spotted_s{
        BaseEntity* m_enemy;
        enemy_spotted_s(BaseEntity* entity) : m_enemy(entity) {}
    };

    //информация о прореаг. противнике
    struct enemy_reacted_s{
        BaseEntity* m_enemy;
        enemy_reacted_s(BaseEntity* entity) : m_enemy(entity) {}
    };

    //информация о существе на кот наткнулись
    struct meet_entity_s{
        BaseEntity* m_entity;
        meet_entity_s(BaseEntity* entity) : m_entity(entity) {}
    };
};

//
// базовое действие
//
class Activity{
public:

    virtual ~Activity(){}

    //проигрывание действия нотация как while(act.Run())
    virtual bool Run(activity_command cmd) = 0;

    //работа с наблюдателями за действием
    void Detach(ActivityObserver* observer);
    void Attach(ActivityObserver* observer, ActivityObserver::event_t type);
    void Notify(ActivityObserver::event_t event, ActivityObserver::info_t info = 0);

private:

    typedef ObserverManager<ActivityObserver> obs_mgr_t;
    obs_mgr_t m_observers;
};

//
// фабрика действий
//
class ActivityFactory{
public:

    //singleton
    static ActivityFactory* GetInst();

    enum controller_type{
        CT_NONE        = 0,
        CT_UPDATE_MPS  = 1 << 0,   //считать ходы во время действия 
        CT_UPDATE_VIS  = 1 << 1,   //пересчитывать видимость в конце действия
        CT_MPS_LIMIT   = 1 << 2,   //ограничить действие количеством ходов  
        CT_STEP_LIMIT  = 1 << 3,   //ограничить действие колличеством шагов
        CT_PATH_SHOW   = 1 << 4,   //показывать путь со время действия
        CT_USUAL_PASS  = 1 << 5,   //проверка проходмости для ходьбы
        CT_BUS_PASS    = 1 << 6,   //проверка проходимости для автобуса
        CT_STAY_CHANGE = 1 << 7,   //смена стойки в зависимости от кол-ва врагов

        CT_ENEMY_STOP         = 1 << 8,  //остановить сущеcтво если виден враг 
        CT_PLAY_REACTION      = 1 << 9,  //проиграть реакцию персонажа
        CT_PLAY_FIRST_PHRASE  = 1 << 10, //проиграть первую фразу

        CT_ACCURACY_DECREASE  = 1 << 11, //уменьшать точность если существо бежит

        CT_HURT_SHOOT  = 1 << 12,  //стратегия стрельбы считающая поражения
        CT_TRACE_LOF   = 1 << 13,  //трассировать линию стрельбы 

        CT_SEND_EVENTS = 1 << 14,  //рассылать события
        CT_PRINT_MSGS  = 1 << 15,  //выводить сообщения
        CT_PRINT_DMGS  = 1 << 16,  //выводить сообщения о повреждении

        CT_SWITCH_OBJECT = 1 << 17, //переключить состояние объекта (UseActivity only)

        //profile для поворота
        CT_CIVILIAN_ROTATE = CT_UPDATE_MPS|CT_UPDATE_VIS,
        CT_TALK_ROTATE     = CT_UPDATE_VIS|CT_STAY_CHANGE,
        CT_PLAYER_ROTATE   = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT|CT_STAY_CHANGE|CT_ENEMY_STOP|CT_PRINT_MSGS,
		CT_ENEMY_LOOKROUND = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT|CT_STAY_CHANGE|CT_ENEMY_STOP,
		CT_ENEMY_ROTATE    = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT|CT_STAY_CHANGE,

        //profile для перемещения
        CT_BUS_MOVE       = CT_UPDATE_VIS/*|CT_PATH_SHOW*/|CT_BUS_PASS,
        CT_CIVILIAN_CAR_MOVE = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT|CT_BUS_PASS,
        CT_PLAYER_MOVE    = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_STEP_LIMIT|CT_PATH_SHOW|CT_USUAL_PASS|CT_STAY_CHANGE|CT_PLAY_FIRST_PHRASE|CT_ENEMY_STOP|CT_PLAY_REACTION|CT_PRINT_MSGS|CT_ACCURACY_DECREASE,
        CT_CIVILIAN_MOVE  = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT/*|CT_PATH_SHOW*/|CT_USUAL_PASS,
        CT_ENEMY_MOVE     = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT/*|CT_PATH_SHOW*/|CT_USUAL_PASS|CT_STAY_CHANGE|CT_ENEMY_STOP|CT_PLAY_REACTION,
        CT_ENEMY_MOVEIGNORE  = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT/*|CT_PATH_SHOW*/|CT_USUAL_PASS|CT_PLAY_REACTION,
        CT_ENEMY_RETREAT = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT/*|CT_PATH_SHOW*/|CT_USUAL_PASS|CT_PLAY_REACTION,
 
        //profile для use
        CT_PLAYER_USE_FAILED  = CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE,
        CT_PLAYER_USE_SUCCEED = CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE|CT_SWITCH_OBJECT,

        //profile для погрузки
        CT_PLAYER_SHIPMENT = /*CT_UPDATE_MPS|*/CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE,

        //profile для выгрузки
        CT_PLAYER_LANDING  = /*CT_UPDATE_MPS|*/CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE,

        //profile для стрельбы
        CT_PLAYER_SHOOT       = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_PRINT_MSGS|CT_TRACE_LOF,
        CT_CIVILIAN_SHOOT     = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_TRACE_LOF,
        CT_ENEMY_SHOOT        = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_TRACE_LOF,
        CT_REACTION_SHOOT     = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_TRACE_LOF,
        CT_ENEMY_SHOOT2       = CT_PRINT_DMGS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_STAY_CHANGE,
        CT_BERSERK_SHOOT      = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS,
        CT_ENEMY_THROW        = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS,
        CT_SCRIPT_SCENE_SHOOT = 0,    

        //profile для броска гранаты
        CT_PLAYER_GRENADE_THROW = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_PRINT_MSGS,
    };

    //информация о стрельбе (необх для просчета линнии стрельбы)
    struct shoot_info_s{
        rid_t m_object;  //объект по кот стреляют
        eid_t m_entity;  //существо по кот стреляют

        shoot_info_s() : m_entity(0) {}
        shoot_info_s(eid_t eid) : m_entity(eid) {}
        shoot_info_s(const rid_t& rid) : m_object(rid), m_entity(0) {}
    };

    //создать поворот существа
    Activity* CreateRotate(BaseEntity* entity, float angle, unsigned controller = CT_NONE);
    //создать use
    Activity* CreateUse(BaseEntity* entity, const rid_t& rid, unsigned deorators = CT_NONE);
    //создать высадку из техники
    Activity* CreateLanding(HumanEntity* human,const ipnt2_t& to, unsigned decorators = CT_NONE);
    //создать движение существом
    Activity* CreateMove(BaseEntity* entity, const pnt_vec_t& path, unsigned controller = CT_NONE);
    //создать посадку в технику
    Activity* CreateShipment(HumanEntity* human, VehicleEntity* vehicle, unsigned decorators = CT_NONE);
    //создать бросок гранаты
    Activity* CreateThrow(HumanEntity* human, const point3& to, unsigned flags = CT_NONE, const shoot_info_s& info = shoot_info_s(), float accuracy = 0.0f);
    //создать стрельбу
    Activity* CreateShoot(BaseEntity* entity, const point3& to, float accuracy, unsigned controller = CT_NONE, const shoot_info_s& info = shoot_info_s());

protected:

    ActivityFactory(){}
};

#endif // _PUNCH_ACTIVITY_H_