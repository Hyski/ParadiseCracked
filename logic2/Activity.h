//
// ��������� �� ��������
//

#ifndef _PUNCH_ACTIVITY_H_
#define _PUNCH_ACTIVITY_H_

class Activity;

class BaseEntity;
class HumanEntity;
class VehicleEntity;

//
// ������� ��� ���������� ����������
//
enum activity_command{
    AC_STOP,     //���������� ��������
    AC_TICK,     //��������� ������� ������ ��������
    AC_SPEED_UP, //�������� ���������� ��������
};

//
// ����������� �� ���������
//
class ActivityObserver : public Observer<Activity*>{
public:

    enum event_type{
        EV_NONE, 

        EV_NO_LOF,            //��� ������ ��������
        EV_MEET_ENTITY,       //�������� ���������� �� ����-��  
        EV_DANGER_ZONE,       //�������� ����� � ������� ����
        EV_ENEMY_REACTED,     //�� �������� ����������� ���� 
        EV_ENEMY_SPOTTED,     //��������� ������� ����
        EV_STEPS_EXPIRIED,    //� �������� ��� ������ ����� (�.�. �������� ��������)
        EV_MOVEPNTS_EXPIRIED, //� �������� ��������� ����
    };

    //���������� � ���������� �����
    struct enemy_spotted_s{
        BaseEntity* m_enemy;
        enemy_spotted_s(BaseEntity* entity) : m_enemy(entity) {}
    };

    //���������� � �������. ����������
    struct enemy_reacted_s{
        BaseEntity* m_enemy;
        enemy_reacted_s(BaseEntity* entity) : m_enemy(entity) {}
    };

    //���������� � �������� �� ��� ����������
    struct meet_entity_s{
        BaseEntity* m_entity;
        meet_entity_s(BaseEntity* entity) : m_entity(entity) {}
    };
};

//
// ������� ��������
//
class Activity{
public:

    virtual ~Activity(){}

    //������������ �������� ������� ��� while(act.Run())
    virtual bool Run(activity_command cmd) = 0;

    //������ � ������������� �� ���������
    void Detach(ActivityObserver* observer);
    void Attach(ActivityObserver* observer, ActivityObserver::event_t type);
    void Notify(ActivityObserver::event_t event, ActivityObserver::info_t info = 0);

private:

    typedef ObserverManager<ActivityObserver> obs_mgr_t;
    obs_mgr_t m_observers;
};

//
// ������� ��������
//
class ActivityFactory{
public:

    //singleton
    static ActivityFactory* GetInst();

    enum controller_type{
        CT_NONE        = 0,
        CT_UPDATE_MPS  = 1 << 0,   //������� ���� �� ����� �������� 
        CT_UPDATE_VIS  = 1 << 1,   //������������� ��������� � ����� ��������
        CT_MPS_LIMIT   = 1 << 2,   //���������� �������� ����������� �����  
        CT_STEP_LIMIT  = 1 << 3,   //���������� �������� ������������ �����
        CT_PATH_SHOW   = 1 << 4,   //���������� ���� �� ����� ��������
        CT_USUAL_PASS  = 1 << 5,   //�������� ����������� ��� ������
        CT_BUS_PASS    = 1 << 6,   //�������� ������������ ��� ��������
        CT_STAY_CHANGE = 1 << 7,   //����� ������ � ����������� �� ���-�� ������

        CT_ENEMY_STOP         = 1 << 8,  //���������� ����c��� ���� ����� ���� 
        CT_PLAY_REACTION      = 1 << 9,  //��������� ������� ���������
        CT_PLAY_FIRST_PHRASE  = 1 << 10, //��������� ������ �����

        CT_ACCURACY_DECREASE  = 1 << 11, //��������� �������� ���� �������� �����

        CT_HURT_SHOOT  = 1 << 12,  //��������� �������� ��������� ���������
        CT_TRACE_LOF   = 1 << 13,  //������������ ����� �������� 

        CT_SEND_EVENTS = 1 << 14,  //��������� �������
        CT_PRINT_MSGS  = 1 << 15,  //�������� ���������
        CT_PRINT_DMGS  = 1 << 16,  //�������� ��������� � �����������

        CT_SWITCH_OBJECT = 1 << 17, //����������� ��������� ������� (UseActivity only)

        //profile ��� ��������
        CT_CIVILIAN_ROTATE = CT_UPDATE_MPS|CT_UPDATE_VIS,
        CT_TALK_ROTATE     = CT_UPDATE_VIS|CT_STAY_CHANGE,
        CT_PLAYER_ROTATE   = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT|CT_STAY_CHANGE|CT_ENEMY_STOP|CT_PRINT_MSGS,
		CT_ENEMY_LOOKROUND = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT|CT_STAY_CHANGE|CT_ENEMY_STOP,
		CT_ENEMY_ROTATE    = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT|CT_STAY_CHANGE,

        //profile ��� �����������
        CT_BUS_MOVE       = CT_UPDATE_VIS/*|CT_PATH_SHOW*/|CT_BUS_PASS,
        CT_CIVILIAN_CAR_MOVE = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT|CT_BUS_PASS,
        CT_PLAYER_MOVE    = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_STEP_LIMIT|CT_PATH_SHOW|CT_USUAL_PASS|CT_STAY_CHANGE|CT_PLAY_FIRST_PHRASE|CT_ENEMY_STOP|CT_PLAY_REACTION|CT_PRINT_MSGS|CT_ACCURACY_DECREASE,
        CT_CIVILIAN_MOVE  = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT/*|CT_PATH_SHOW*/|CT_USUAL_PASS,
        CT_ENEMY_MOVE     = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT/*|CT_PATH_SHOW*/|CT_USUAL_PASS|CT_STAY_CHANGE|CT_ENEMY_STOP|CT_PLAY_REACTION,
        CT_ENEMY_MOVEIGNORE  = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT/*|CT_PATH_SHOW*/|CT_USUAL_PASS|CT_PLAY_REACTION,
        CT_ENEMY_RETREAT = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT/*|CT_PATH_SHOW*/|CT_USUAL_PASS|CT_PLAY_REACTION,
 
        //profile ��� use
        CT_PLAYER_USE_FAILED  = CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE,
        CT_PLAYER_USE_SUCCEED = CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE|CT_SWITCH_OBJECT,

        //profile ��� ��������
        CT_PLAYER_SHIPMENT = /*CT_UPDATE_MPS|*/CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE,

        //profile ��� ��������
        CT_PLAYER_LANDING  = /*CT_UPDATE_MPS|*/CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE,

        //profile ��� ��������
        CT_PLAYER_SHOOT       = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_PRINT_MSGS|CT_TRACE_LOF,
        CT_CIVILIAN_SHOOT     = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_TRACE_LOF,
        CT_ENEMY_SHOOT        = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_TRACE_LOF,
        CT_REACTION_SHOOT     = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_TRACE_LOF,
        CT_ENEMY_SHOOT2       = CT_PRINT_DMGS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_STAY_CHANGE,
        CT_BERSERK_SHOOT      = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS,
        CT_ENEMY_THROW        = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS,
        CT_SCRIPT_SCENE_SHOOT = 0,    

        //profile ��� ������ �������
        CT_PLAYER_GRENADE_THROW = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_PRINT_MSGS,
    };

    //���������� � �������� (����� ��� �������� ������ ��������)
    struct shoot_info_s{
        rid_t m_object;  //������ �� ��� ��������
        eid_t m_entity;  //�������� �� ��� ��������

        shoot_info_s() : m_entity(0) {}
        shoot_info_s(eid_t eid) : m_entity(eid) {}
        shoot_info_s(const rid_t& rid) : m_object(rid), m_entity(0) {}
    };

    //������� ������� ��������
    Activity* CreateRotate(BaseEntity* entity, float angle, unsigned controller = CT_NONE);
    //������� use
    Activity* CreateUse(BaseEntity* entity, const rid_t& rid, unsigned deorators = CT_NONE);
    //������� ������� �� �������
    Activity* CreateLanding(HumanEntity* human,const ipnt2_t& to, unsigned decorators = CT_NONE);
    //������� �������� ���������
    Activity* CreateMove(BaseEntity* entity, const pnt_vec_t& path, unsigned controller = CT_NONE);
    //������� ������� � �������
    Activity* CreateShipment(HumanEntity* human, VehicleEntity* vehicle, unsigned decorators = CT_NONE);
    //������� ������ �������
    Activity* CreateThrow(HumanEntity* human, const point3& to, unsigned flags = CT_NONE, const shoot_info_s& info = shoot_info_s(), float accuracy = 0.0f);
    //������� ��������
    Activity* CreateShoot(BaseEntity* entity, const point3& to, float accuracy, unsigned controller = CT_NONE, const shoot_info_s& info = shoot_info_s());

protected:

    ActivityFactory(){}
};

#endif // _PUNCH_ACTIVITY_H_