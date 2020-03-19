//
// абстракция оператора управляющего камерой
//

#ifndef _PUNCH_CAMERAMAN_H_
#define _PUNCH_CAMERAMAN_H_

//
// класс - абстракция кинодубля
//
class Camerawork{
public:
    
    virtual ~Camerawork(){}
    
    //снимать кино пока действие активно
    virtual bool Film(bool factivity_running) = 0;
};

//
// установить устройство камеры
//
class CameraDevice{
public:

    virtual ~CameraDevice(){}

    //sengleton
    static CameraDevice* GetInst();

    //видима ли точка в камере
    virtual bool IsInCone(const point3& pnt3) const = 0;
    //переместить камеру
    virtual void Move(const point3& pnt, float time) = 0;
    //снимать (спрятать экран HM, показать уровень)
    virtual void DoFilm(bool flag, BaseEntity* entity = 0) = 0;    
};

//
// абстракция оператора камеры
//
class Cameraman{
public:

    virtual ~Cameraman(){}

    //singleton
    static Cameraman* GetInst();

    enum flag_type{
        
        //показывать определенные действия
        F_SHOW_USE      = 1 << 0, 
        F_SHOW_MOVE     = 1 << 1,
        F_SHOW_SHOOT    = 1 << 2,
        F_SHOW_THROW    = 1 << 3,
        F_SHOW_ROTATE   = 1 << 4,
        F_SHOW_LANDING  = 1 << 5,
        F_SHOW_SHIPMENT = 1 << 6,
        F_SHOW_DAMAGES  = 1 << 7,

        F_SHOW_ALL_ACTIONS =  F_SHOW_USE|F_SHOW_MOVE|F_SHOW_SHOOT|F_SHOW_THROW|F_SHOW_ROTATE
                             |F_SHOW_LANDING|F_SHOW_SHIPMENT|F_SHOW_DAMAGES,

        F_SHOW_ENEMY_RELATED_MOVE    = 1 << 8,  //показывать перемещение враждебных персонажей
        F_SHOW_ENEMY_RELATED_SHOOT   = 1 << 9,  //показывать стрельбу враждебных персонажей
        F_SHOW_FRIEND_RELATED_MOVE   = 1 << 10,  //показывать перемещение дружественных персонажей
        F_SHOW_FRIEND_RELATED_SHOOT  = 1 << 11,  //показывать стрельбу дружественных персонажей
        F_SHOW_NEUTRAL_RELATED_MOVE  = 1 << 12,  //показывать перемещение нейтральных персонажей
        F_SHOW_NEUTRAL_RELATED_SHOOT = 1 << 13,  //показывать стрельбу нейтральных персонажей

        F_SHOW_CIVILIAN_VEHICLES_MOVE        = 1 << 14,  //показывать перемещение мирной техники
        F_SHOW_CIVILIAN_VEHICLES_SHOOT       = 1 << 15,  //показывать стрельбу мирной техники
        F_SHOW_CIVILIAN_HUMANS_TRADERS_MOVE  = 1 << 16,  //показывать перемещение мирных жителей
        F_SHOW_CIVILIAN_HUMANS_TRADERS_SHOOT = 1 << 17,  //показывать стрельбу мирных жителей

        F_SHOW_ALL_OPTIONS =  F_SHOW_ENEMY_RELATED_MOVE|F_SHOW_ENEMY_RELATED_SHOOT|F_SHOW_FRIEND_RELATED_MOVE
                             |F_SHOW_FRIEND_RELATED_SHOOT|F_SHOW_NEUTRAL_RELATED_MOVE|F_SHOW_NEUTRAL_RELATED_SHOOT
                             |F_SHOW_CIVILIAN_VEHICLES_MOVE|F_SHOW_CIVILIAN_VEHICLES_SHOOT|F_SHOW_CIVILIAN_HUMANS_TRADERS_MOVE
                             |F_SHOW_CIVILIAN_HUMANS_TRADERS_SHOOT,

        F_SHOW_NOTHING = 0,
        F_SHOW_ALL     = F_SHOW_ALL_ACTIONS|F_SHOW_ALL_OPTIONS,                            
    };

    //сконфигурировать оператора
    virtual void Configure(unsigned flags) = 0;

    //сфокусировать камеру на существе
    virtual void FocusEntity(BaseEntity* entity, float time = 0.2f) = 0;
    
    //сфокусировать камеру на ходьбе
    virtual Camerawork* FilmMove(BaseEntity* ent) = 0;
    //сфокусировать на повороте
    virtual Camerawork* FilmRotate(BaseEntity* ent) = 0;
    //сфокусировать на высадке
    virtual Camerawork* FilmLanding(HumanEntity* hum) = 0;
    //сфокусировать на использовании
    virtual Camerawork* FilmUse(BaseEntity* ent, const rid_t& rid) = 0;
    //сфокусировать на стрельбе
    virtual Camerawork* FilmShoot(BaseEntity* actor, BaseEntity* victim) = 0;
    //сфокусировать на броске
    virtual Camerawork* FilmThrow(HumanEntity* actor, BaseEntity* victim) = 0;
    //сфокусировать на погрузке
    virtual Camerawork* FilmShipment(HumanEntity* hum, VehicleEntity* veh) = 0;
};

#endif // _PUNCH_CAMERAMAN_H_