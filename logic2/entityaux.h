//
// классы для включения в другие модули из модуля существ
//

#ifndef _PUNCH_ENTITYAUX_H_
#define _PUNCH_ENTITYAUX_H_

class BaseThing;

class BaseEntity;
class HumanEntity;
class TraderEntity;
class VehicleEntity;

//
// класс посетителя для существа
//
class EntityVisitor{
public:

    virtual ~EntityVisitor(){}

    virtual void Visit(HumanEntity* human) {}
    virtual void Visit(TraderEntity* trader) {}
    virtual void Visit(VehicleEntity* vehicle) {}
};

//
// класс для реализации pattern'a наблюдатель
//
class EntityObserver : public Observer<BaseEntity*>{
public:

    virtual ~EntityObserver() ;

    //события для срабатывания обновления
    enum event_type{
        EV_NONE,

        EV_DESTROY,      //существо разрушено в памяти (delete entity)
        EV_QUIT_TEAM,    //выход из команды
        EV_JOIN_TEAM,    //приход в команду
        EV_FOS_CHANGE,   //изменение поля видимости
        EV_DEATH_PLAYED, //посылается после вызова сценария DeathStrategy
        EV_PREPARE_DEATH,//посылается до вызова сценария DeathStrategy

        EV_CREW_CHANGE,      //изменение состава экипажа (humans and vehicle)
        EV_HEALTH_CHANGE,    //изменение здоровья существа (humans and vehicle)
        EV_LEVELUP_CHANGE,   //изменение кол-во очков на levelup (humans only)
        EV_MOVEPNTS_CHANGE,  //изменение кол-ва movepoints существа (humans and vehicle)
        EV_SELECTION_CHANGE, //изменение выделения существа
        EV_BODY_PART_CHANGE, //изменение состояния части тела существа

        EV_WISDOM_CHANGE,    //изменение параметра мудрости (humans only)
        EV_MORALE_CHANGE,    //изменение параметра морали (humans only)
        EV_ACCURACY_CHANGE,  //изменение параметра точности (humans only)
        EV_REACTION_CHANGE,  //изменение параметра реакции (humans only)
        EV_MECHANICS_CHANGE, //изменение параметра вождения (humans only)
        EV_EXPERIENCE_CHANGE,//изменение параметра опыта (humans only) 

        EV_NO_LOF,           //отсутствует линия стрельбы
        EV_PANIC_CHANGE,     //изменение режима паники        
        EV_HEX_COST_CHANGE,  //изменение стоимости hex'a
        EV_DEXTERITY_CHANGE, //изменение подвижности
        EV_MOVE_TYPE_CHANGE, //изменение типа передвижения (humans only)
        EV_POSE_TYPE_CHANGE, //смена позы (humans only)
        EV_WEIGHT_CHANGE,    //изменение параметров веса существа (humans only)
        EV_STRENGTH_CHANGE,  //изменение параметра силы

        EV_INSERT_THING,     //вставили предмет
        EV_REMOVE_THING,     //удалили предмет

        EV_CAN_MAKE_FAST_ACCESS, //стратегией быстрого доступа к предмету можно пользоваться

        EV_BODY_PACK_CHANGE,   //изменение содержимого на теле (humans only)
        EV_HANDS_PACK_CHANGE,  //изменение содержимого в руках (humans only)
        EV_LONG_DAMAGE_CHANGE, //изменение массива продолжительных повреждений (humans only)

        EV_TAKE_MONEY,         //персонаж взял деньги  
        EV_INGOING_CHANGE,     //изменение режима дохождения (humans + vehicles)
		EV_RELATION_CHANGE,    //изменение отношения подкоманды игрока к существу
        
        EV_WEAPON_RELOAD,      //перезарядка оружия  
        EV_WEAPON_STATE_CHANGE,//изменение кол-ва патронов в оружии существа (humans and vehicles)

        EV_LUCKY_STRIKE,       //существо нанесло критическое повреждение другому
        EV_CRITICAL_HURT,      //было критическое повреждение (humans only)

        EV_MOVE_COMMAND,       //команда на передвижение
        EV_SELECT_COMMAND,     //команда на выделение персонажа
        EV_ATTACK_COMMAND,     //команда на атаку 

        EV_ACTIVITY_INFO,      //состояние действия связанного с существом

        EV_TEAMMATE_DISMISS,   //увольнение персонажа из команды (humans only)
        EV_TEAMMATE_LEAVE,     //персонаж уходит из команды (humans only)
        EV_TEAMMATE_COME,      //персонаж пришел в команду (humans only)
        EV_TEAMMATE_JOIN,      //персонаж уволился из команды (humans only)

        EV_OPEN_FAILED,
        EV_OPEN_SUCCEED,

        EV_LEVELUP,            //произошел levelup
        EV_CAR_DRIVE_FAILED,   //не получилось вести машину

        EV_USE_PLASMA_GRENADE, //бросок полазменной гранаты 
    };

    //информация о существе 
    struct entity_info_s{
        BaseEntity* m_entity;
        entity_info_s(BaseEntity* entity) : m_entity(entity) {}
    };

    //состояние действия
    struct activity_info_s{

        enum activity_event {
            AE_BEG, //начало
            AE_END, //конец
        };

        activity_event m_event;

        bool IsEnd() const      { return m_event == AE_END; } 
        bool IsBegining() const { return m_event == AE_BEG; }

        activity_info_s(activity_event event) : m_event(event) {}
    };    

    //информация о предмете
    struct thing_info_s{
        BaseThing* m_thing;

        thing_info_s(BaseThing* thing) : m_thing(thing) {}
    };
};

#endif // _PUNCH_ENTITYAUX_H_