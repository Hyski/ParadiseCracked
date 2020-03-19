//
// стратегии - некотторые алгоритмы работы с существами
//

#ifndef _PUNCH_STRATEGY_H_
#define _PUNCH_STRATEGY_H_

class Marker;
class Spectator;

class BaseEntity;
class HumanEntity;
class VehicleEntity;

//
// базовый класс для всех стратегий
//
class BaseStrategy{
public:
    virtual ~BaseStrategy(){}
    //создание объектов runtime
    DCTOR_ABS_DEF(BaseStrategy)
    //сохранение / загрузка
    virtual void MakeSaveLoad(SavSlot& st) = 0;   
};

//
// класс - предоставляет стратегии приема в экинаж
//
class CrewStrategy : public BaseStrategy{
public:
    //можем присоед к команде?
    virtual bool CanJoin(VehicleEntity* vehicle, HumanEntity* human) const = 0;
    //добавить в команду
    virtual void Inc(VehicleEntity* vehicle, HumanEntity* human) = 0;
    //удалить из команды
    virtual void Dec(VehicleEntity* vehicle, HumanEntity* human) = 0;
    //запретить/разрешить посадку (нужно для автобусов)
    virtual void Enable(VehicleEntity* vehicle, bool flag) = 0;
    //узнать размер команды
    virtual int GetCrewSize(VehicleEntity* vehicle) const = 0;
    //узнать водитля для этой техники
    virtual HumanEntity* GetDriver(VehicleEntity* vehicle) = 0;
};

//
// предоставляет стратегии смерти существа
//
class DeathStrategy : public BaseStrategy{
public:

    // проиграть сценарий смерти
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:  
    //          true  - удалить существо из массива существ
    //          false - не удалять существо из массива существ
    //
    virtual bool MakeDeath(BaseEntity* actor, BaseEntity* killer) = 0;
};

//
// предоставляет стратегии передвижения
//
class MoveStrategy : public BaseStrategy{
public:
    
    //узнать кол-во шагов кот может сделать существо
    virtual int GetSteps() const = 0;
    //узнать стоимость одного hex'са
    virtual int GetHexCost() const = 0;
};

//
// стратегия выбора модели паники
//
class PanicTypeSelector : public BaseStrategy{
public:

    //расчитать поведение в панике
    virtual human_panic_type Select(HumanEntity* human) = 0;
    //узнать расчитаное значение
    virtual human_panic_type GetPanic(HumanEntity* human) const = 0;
    //принудительно установить модель паники
    virtual human_panic_type SetPanic(HumanEntity* human, human_panic_type type) = 0;
};

//
// стратегия быстрого доступа к предметам человека
//
class FastAccessStrategy : public BaseStrategy{
public:

    //
    // итератор по предметам которые можно положить в руки
    //
    class Iterator{
    public:

        virtual ~Iterator(){}

        //установить на начало последовательности
        virtual void First(BaseThing* from = 0, unsigned mask = TT_ALL_ITEMS) = 0;

        //перейти к следущему предмету
        virtual void Next() = 0;
        //итерирование еще не закончено?
        virtual bool IsDone() const = 0;

        //получить ссылку на предмет
        virtual BaseThing* Get() = 0;
    };

    enum iter_type{
        IT_FORWARD,     //итерирование вперед
        IT_BACKWARD,    //итерирование назад
    };

    //узнать текущий предмет быстрого доступа
    virtual BaseThing* GetThing() = 0;
    //установить текущий предмет быстрого доступа
    virtual void SetThing(BaseThing* thing) = 0;

    //возможен ли быстрый доступ к предметам для персонажа
    virtual bool CanMakeFastAccess() = 0;
    //создать итератор по предметам человека
    virtual Iterator* CreateIterator(iter_type type) = 0;
};

//
// стратегия для проигрывания звука
//
class SoundStrategy : public BaseStrategy{
public:

};

// 
// фабрика порождающая классы для существ
//
class StrategyFactory{
public:

    static StrategyFactory* GetInst();

    enum spectator_type{
        ST_USUAL,   //обычная стратегия
        ST_TRADER,  //стратегия для торговца
    };

    //создание наблюдателей
    Spectator* CreateSpectator(spectator_type tepe, BaseEntity* entity);

    enum crew_type{
        CT_USUAL,    //обычная стратегия
        CT_BUSY,     //постоянно занятая машина
        CT_BUS,      //для автобуса
    };

    //создание менеджера команд
    CrewStrategy* CreateCrew(crew_type type, VehicleEntity* vehicle);

    enum death_type{
        DT_HUMAN,    //смерть человека
        DT_TRADER,   //смерть торговца
        DT_VEHICLE,  //смерть техники
        DT_FERALCAR, //смерть нецивилизованной машины
    };

    //создание сценария смерти существа
    DeathStrategy* CreateDeath(death_type, BaseEntity* entity);

    enum move_type{
        MT_HUMAN,    //перемещение человека
        MT_VEHICLE,  //перемещения машины
    };
    
    //создать сценарий перемещения
    MoveStrategy* CreateMove(move_type type, BaseEntity* entity);

    enum marker_type{
        MRK_USUAL, //обычный
    };
    
    //создать маркер
    Marker* CreateMarker(marker_type type, BaseEntity* entity);
   
    enum panic_selector_type{
        PST_USUAL,
    };

    //созадать стратегию расчета паники
    PanicTypeSelector* CreatePanicSelector(panic_selector_type type);

    enum sound_type{
        SND_HUMAN,
        SND_TRADER,
    };

    //создать стратегию звука
    SoundStrategy* CreateSound(sound_type type, BaseEntity* entity);

    enum fast_access_type{
        FAT_USUAL,
    };

    FastAccessStrategy* CreateFastAccess(fast_access_type type, HumanEntity* entity);

protected:

    StrategyFactory(){}
};

#endif // _PUNCH_STRATEGY_H_