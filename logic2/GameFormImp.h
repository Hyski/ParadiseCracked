//
// реализация игрового меню
//

#ifndef _PUNCH_GAMEFORMIMP_H_
#define _PUNCH_GAMEFORMIMP_H_

#include "form.h"
#include "spawn.h"
#include "entityaux.h"
#include "visutils3.h"
#include "enemydetection.h"

#include "../interface/gamescreen.h"

class Activity;

class BaseEntity;
class HumanEntity;
class TraderEntity;
class VehicleEntity;

class Marker;
class ShotTracer;
class EntityObserver;

class GameScreenController;
class DialogScreenController;

//
// класс для проигрывания действия  
//
class ActionManager{
public:

    virtual ~ActionManager(){}

    //создать действие для подхода к месту action'a
    virtual Activity* CreateMove() { return 0; }
    //создать действие для action'a
    virtual Activity* CreateAction() { return 0; }
    //создать поворот для use'а
    virtual Activity* CreateRotate() { return 0; }

    //нужно ли двигаться к месту action'а?
    virtual bool NeedMove() const { return false; }
    //нужно ли развернуться к месту action'a?
    virtual bool NeedRotate() const { return false; }

    //завершить action
    virtual void Shut() {}
};

//
// низкоуровневая оболочка для мышки
//
class CursorDevice{
public:

    virtual ~CursorDevice(){}

    enum menu_part{
        MP_NONE,    //нет конкретной части
        MP_WEAPON,  //мышь над меню оружия
    };

    //узнать существо на кот указывает мышь
    virtual BaseEntity* GetEntity() const = 0;
    //узнать название объекта на кот указывает мышь
    virtual bool GetObject(rid_t* str) const = 0;
    //получить точку трассировщика
    virtual bool GetTracePnt(point3* pnt) const = 0;
    //получить hex в кот указывает мышь
    virtual bool GetLevelHex(ipnt2_t* pnt) const = 0;
    //узнать на что показывает мышь в интерфейсе
    virtual bool GetMenuPart(menu_part* part) const = 0;

    enum cursor_type{
        CT_USE,
        CT_MEDIC,
        CT_SELECT,
        CT_UNLOAD,
        CT_NORMAL,
        CT_PICKUP,
        CT_SPECIAL,
        CT_SHIPMENT,
        CT_MOVE_ENABLE,
        CT_MOVE_DISABLE,
        CT_ATTACK_ENABLE,
        CT_ATTACK_DISABLE,
    };

    enum color_type{
        DEFAULT_COLOR = 0x00ff00,
        DENIAL_COLOR  = 0xff0000,
    };

    //текстовое поле курсора
    struct text{
        std::string m_text;
        unsigned    m_color;

        text(int value, unsigned color = DEFAULT_COLOR)
        {
            m_color = color;
            std::ostringstream ostr;
            ostr << value;
            m_text = ostr.str();
        }

        text(const std::string& text = std::string(), unsigned color = DEFAULT_COLOR) : 
            m_text(text), m_color(color) {}
    };

    //установить текстовые поля и тип курсора
    virtual void SetCursor( cursor_type type,
                            const text& t1 = text(),
                            const text& t2 = text(), 
                            const text& t3 = text()) = 0;
};

//
// стратегия на курсор
//
class CursorManager{
public:

    virtual ~CursorManager(){}

    //выполнить команду курсора
    virtual void Execute() = 0;
    //можно ли выполнить команду курсора
    virtual bool CanExecute(CursorDevice* tracer) = 0;
};

//
// управление курсором с помощью клавиатуры и меню
//
class CursorAdviser{
public:

    virtual ~CursorAdviser(){}

    //нужен курсор использования
    virtual bool NeedUse() = 0;
    //нужен курсор взлома
    virtual bool NeedHack() = 0;
    //нужен курсор атаки
    virtual bool NeedAttack() = 0;
    //нужен курсор поворота
    virtual bool NeedRotate() = 0;
};

//
// класс для выбора стратегии курсора
//
class CursorSelector{
public:

    virtual ~CursorSelector(){}

    //возможные типы курсоров
    enum cursor_type{
        CUR_NULL,
        CUR_USE,       //использование
        CUR_MENU,      //курсор меню
        CUR_HACK,      //взлом
        CUR_MOVE,      //движение
        CUR_SELECT,    //выбор персонажа
        CUR_ROTATE,    //поворот
        CUR_ATTACK,    //атака
        CUR_LANDING,   //высадка из техники
        CUR_SHIPMENT,  //посадка в технику
        CUR_TREATMENT, //лечение

        MAX_CURSORS,
    };

    //выбрать нужную стратегию отображения курсора
    virtual cursor_type Select(CursorDevice* tracer, CursorAdviser* adviser) = 0;
};

//
// умный курсор
//
class SmartCursor{
public:

    SmartCursor();
    ~SmartCursor();
    
    //установить нормальный курсор
    void SetUsual();
    //поставить явный запрос на обновление
    void Invalidate(){ m_requests++; }
    //обновить мышиный курсор
    void Update();

    //узнать время последнего click'а
    float GetLastClickTime() const { return m_click_time; }

private:

    int m_requests;

    float m_upd_time;
    float m_click_time; //время последнего клика мышью

    CursorManager*  m_current;
    CursorAdviser*  m_adviser;
    CursorSelector* m_selector;

    CursorManager*  m_cursors[CursorSelector::MAX_CURSORS];
};

//
// класс для дохождения существ
//
class Walkers : private SpawnObserver,
                private EntityObserver                
{
public:

    Walkers();
    ~Walkers();

    //поставлен ли на дохождение
    bool IsGoing(BaseEntity* ent) const;
    //добавить в спиcок существ для дохождения
    void Insert(BaseEntity* entity, const ipnt2_t& to);

    //получить действие для проигрывания ActionManager
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //если функция возвращает 0 ==> действий больше нет 
    //
    ActionManager* CreateAction(BaseEntity* entity = 0);   

    //сохранение/загрузка 
    void MakeSaveLoad(SavSlot& slot);

private:

    //найти сущетство в массиве
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возвращает -1 если сущ-ва нет
    int Ent2Index(BaseEntity* entity) const;


    //присоед наблюдателя
    void LinkObserver(BaseEntity* entity);
    //обработка событий перехода на уровень
    void Update(Spawner* spawn, SpawnObserver::event_t event, SpawnObserver::info_t info);
    //обработка убийства существа
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);

    struct info_s;

    //удалить сущетсво из списков
    void Remove(info_s& info);
    //создать действие по внутренним данным 
    ActionManager* CreateAction(info_s& info);

private:

    struct info_s{

        info_s() : m_entity(0){}

        //сохранить/загрузить структуру
        void MakeSaveLoad(SavSlot& slot);
        //тоже самое сущ-во
        bool IsSame(BaseEntity* entity) const;
        //нужно ли идти
        bool NeedMove() const;
        //ячейка пуста
        bool IsEmpty() const { return m_entity == 0; }
        
        BaseEntity* m_entity;
        ipnt2_t     m_point;
    };

    info_s m_infos[MAX_TEAMMATES];
};

//
// класс на иконки врагов
// 
class EnemyIcons :  private SpawnObserver,
                    private SpectatorObserver,
                    private RelationsObserver
{
public:

    EnemyIcons();
    ~EnemyIcons();

    //получить существо с опред индексом
    BaseEntity* Icon2Entity(unsigned index);
    //обработка выбора существа
    void HandleSelectReq(BaseEntity* entity);
    //сохранить/загррузить иконки врагов
    void MakeSaveLoad(SavSlot& slot);

private:

    //обработк событий расстоновки существ
    void Update(Spawner* spawner, SpawnObserver::event_t event, SpawnObserver::info_t info);
    //обработка изменения сотояния наблюдателя
    void Update(Spectator* spectator, SpectatorObserver::event_t event, SpectatorObserver::info_t);    
    //обработка изменения враждебности игрока
    void changed(RelationsObserver::event_type et, void* info);

    //добавить иконку врага
    void Insert(Marker* marker);
    //удалить иконку врага
    void Remove(Marker* marker);
    //установить флаги
    void SetFlags(BaseEntity* entity, Marker* marker);

    //присоеденить/отсоеденить наблюдателей 
    void AttachObservers();
    void DetachObservers();

private:

    BaseEntity* m_current;
};

//
// класс на иконки людей
//
class HumanIcons : private SpawnObserver,
                   private EntityObserver                   
{
public:

    HumanIcons();
    ~HumanIcons();
    
    //конвертация иконки в существо
    HumanEntity* Icon2Entity(int id);
    //преобразовать иконку в существо
    int Entity2Icon(HumanEntity* human);
    //сохранение/загрузка 
    void MakeSaveLoad(SavSlot& slot);
 
private:

    //приоед. наблюдателя к существу
    void LinkObserver(HumanEntity* human);

    //вставить иконку
    void Insert(HumanEntity* human, bool insert2front);

    enum remove_type{
        RT_USUAL,
        RT_DEATH,
    };

    //удалить иконку
    void Remove(HumanEntity* human, remove_type type = RT_USUAL);
    
    //узнать heartbeat
    int GetHumanHeartbeat(HumanEntity* human);
    //узнать флаги иконки
    unsigned GetHumanFlags(HumanEntity* human);

    //обработка сообщений о расстановке
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //обработка изменения состояния существа
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);

private:

    static const float m_wound_lvl;
    static const int   m_dead_lvl;
};

//
// иконки на технику
//
class TechIcons : private SpawnObserver,
                  private EntityObserver
{
public:

    TechIcons();
    ~TechIcons();

    //обновить иконки на технику
    void Update();
    //добавить иконку на технику
    void Insert(VehicleEntity* vehicle);
    //удалить иконку на технику
    void Remove(VehicleEntity* vehicle);

    //получить ссылку на технику по номеру иконки
    VehicleEntity* Icon2Entity(unsigned icon);
    //получить номер иконки по ссылке на технику
    int Entity2Icon(VehicleEntity* vehicle);

    //сохранение/загрузка 
    void MakeSaveLoad(SavSlot& slot);

private:

    //присоеденить наблюдателя
    void LinkObserver(BaseEntity* vehicle);
    //отрисовать иконку для техники
    void DrawIcon(int icon_id, VehicleEntity* vehicle);

    //обработка сообщений о расстановке
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //обработка изменения состояния существа
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);

private:

    struct info_s{
        int            m_requests;
        VehicleEntity* m_vehicle;

        info_s() : m_requests(0), m_vehicle(0){}
    };
    
    info_s    m_icons[MAX_TEAMMATES];
};

//
// иконки взломанной техники
//
class RobotIcons :  private SpawnObserver,
                    private EntityObserver
{
public:

    RobotIcons();
    ~RobotIcons();

    //узнать существо с иконкой index 
    VehicleEntity* Icon2Entity(unsigned index);
    //сохранить/загррузить иконки взломанной техники
    void MakeSaveLoad(SavSlot& slot);

private:

    //добавить иконку
    void Insert(VehicleEntity* robot);
    //удалить иконку
    void Remove(VehicleEntity* robot);
    //установить состояние иконки по существу
    void SetRobotState(VehicleEntity* robot);

    //обработка сообщений о расстановке
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //обработка изменения состояния существа
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);
};

//
// класс для обработки ввода
//
class InputManager{
public:

    virtual ~InputManager(){}

    //обработать ввод пользователя
    virtual void HandleInput() = 0;
};

//
// стратегия для перезарядки оружия
//
class Reloader{
public:

    Reloader();
    ~Reloader();

    //можно ли произвести перезарядку
    bool CanReload(HumanEntity* human) const;

    //перезарядить оружие человека
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //ВНИМАНИЕ: 
    //         в целях оптимизации предполагается что операция
    //         будет вызываться только если CanReload() == true    
    //
    void DoReload(HumanEntity* human);

private:

    //оружие заряжено полностью? 
    bool CanLoadAmmo(const WeaponThing* weapon) const;
};

//
// собственно само игровое меню
//
class GameFormImp : public  BaseForm,
                    private SpawnObserver,
                    private EntityObserver,
                    private GameScreenController
{
public:

    GameFormImp();
    ~GameFormImp();

    //показать меню
    void Show();
    //инициализация
    void Init(const ini_s& ini);
    //обработка ввода 
    void HandleInput(state_type* st);
    //сохранить меню
    void MakeSaveLoad( SavSlot& st);
    //показать/спрятать меню
    void ShowControls(bool flag);

    //
    // функции ниже нужны для работы *Icons, Controller, SmartCursor
    //
   
    //получить доступ к менеджеру доожденя
    Walkers* GetWalkers();
    //узнать текущее существо в интефейсе
    BaseEntity* GetCurEnt();
    //получить доступ к курсору
    SmartCursor* GetCursor();
    //получить доступ к стратегии перезарядки
    Reloader* GetReloader();
    //получить доступ к икнокам персонажей
    HumanIcons* GetHumanIcons();
    //получить доступ к иконкам техники
    TechIcons* GetTechIcons();
    //получить доступ к иконкам врагов
    EnemyIcons* GetEnemyIcons();
    //получить доступ к иконкам роботов
    RobotIcons* GetRobotIcons();

    //можно ли показать поле видимости
    bool CanShowFOS() const;
    //можно ли атаковать
    bool CanAttack() const;
    //можно ли сделать use
    bool CanMakeUse() const;
    //можно ли сделать Hack
    bool CanMakeHack() const;

    //показать обычный курсор
    void ShowUsualCursor();

    //обоработка нажатия на кнопку use
    void HandleSwitchUseReq();
    //обработка нажатия на кнопку показа поля видимости
    void HandleSwitchFOSReq();
    //обработка нажатия на кнопку hack
    void HandleSwitchHackReq();
    //обработка нажатия на оружие
    void HandleSwitchAttackReq();
    //обработка конца хода
    void HandleEndOfTurnReq();
    //обработка показа журнала
    void HandleShowJournalReq();
    //обработка запроса на показ меню опций
    void HandleShowOptionsReq();

    enum selection_type{
        SEL_SIMPLE,  //просто выделение (напр в конце действия)
        SEL_COMMAND, //явная команда на выделение персонажа 
    };

    //обработка курсора выбора существа
    void HandleSelectReq(BaseEntity* entity, selection_type type = SEL_SIMPLE);
    //обработка курсора на use
    void HandlePlayActionReq(ActionManager* mgr);
    //обработка показа меню inventory
    void HandleShowInventoryReq(HumanEntity* human, AbstractGround* ground, AbstractScroller* scroller, AbstractBin* bin);
    //обработка показа меню торговли
    void HandleShowShopReq(HumanEntity* hero, TraderEntity* trader, AbstractGround* ground, AbstractScroller* scroller, AbstractBin* bin);
  
    //обработка переключение типа стрельбы
    void HandleReloadWeapon();
    void HandleSwitch2Aimshot();
    void HandleSwitch2Autoshot();
    void HandleSwitch2Snapshot();

    void HandleSwitchHandsMode();
    void HandleNextFastAccessThing();
    void HandlePrevFastAccessThing();

    //переключение стойки существа
    void HandleSwitch2Run();
    void HandleSwitch2Sit();
    void HandleSwitch2Stand();

    //проиграть действие
    void RunActivity(int activity_command);

private:

    //проиниц. форму в первый раз при зарузке
    void Init();

    //попробовать выйти на другой уровень
    bool CanExitLevel(std::string* level);

    //обработка окончания проигрывания use
    bool TrapActionEnd();
    //обработка окончания подхода для use
    bool TrapActionMoveEnd();
    //обработка окончания поворота сщества для use
    bool TrapActionRotateEnd();

    //восстановить выделение при переходе другой уровень и Save'е
    void RestoreSelection();
    //сбросить кнопки управления курсором
    void ResetCursorModeButtons();

    //обработка сообщений о расстановке
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //обработка изменения состояния существа
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);

    //
    // обработка событий меню
    //

    //реакция на клик мыши на иконке оружия
    void OnWeaponIconClick();
    //реакция на клик мыши на иконке врага
    void OnEnemyIconClick(Marker* marker);
    //обработка нажатия кнопки
    void OnButtonClick(GameScreen::BUTTON id);
    //обработка нажатия кнопки
    void OnCheckBoxClick(GameScreen::CHECKBOX id);
    //реакция на клик мыши на иконке персонажа
    void OnIconClick(unsigned int id,GameScreen::BN_CLICK click_type);
    //реакция на клик мыши на иконке врага
    void OnHackVehicleClick(VehicleEntity* ve,GameScreen::BN_CLICK click_type);
    //реакция на клик мыши на иконке техники
    void OnVehicleIconClick(unsigned int icon_number,GameScreen::BN_CLICK click_type);
    
private:

    enum activity_type{
        AT_NONE,          //нет активности 
        AT_ACTION,        //use на предмете
        AT_ACTION_MOVE,   //подход для use на существе
        AT_ACTION_ROTATE, //поворот для use на существе
    };

    activity_type  m_activity_type;
    ActionManager* m_action_mgr;

    enum turn_state{
        TS_START_TURN,      //начать ход
        TS_INCOMPLETE,      //ход еще не закончен
        TS_EXIT_LEVEL,      //попытаться выйти на другой уровень
        TS_WAIT_LEVEL,      //ждать конца загрузки уровня   
        TS_RUN_WALKERS,     //проиграть дохождение
        TS_FINISHED,        //передать ход противнику 
        TS_TEST_LEVEL_EXIT, //проверить выход с уровня
    };

    turn_state m_turn_state;

    enum input_type{
        IT_EMPTY,        //пустой обработчик ввода
        IT_USUAL,        //обработчик обычного пользовательского ввода
        IT_ACTIVITY,     //обработчик ввода при работающем действии

        MAX_INPUT_MGRS,
    };

    InputManager*  m_input_mgr[MAX_INPUT_MGRS];

    enum flag_type{
        FT_CAN_USE    = 1 << 0,
        FT_SHOW_FOS   = 1 << 1,
        FT_CAN_HACK   = 1 << 2,
        FT_CAN_ATTACK = 1 << 3,
        FT_UPDATE_JOURNAL = 1 << 4,
    };

    unsigned m_flags;

    Activity*   m_activity;
    BaseEntity* m_cur_ent;

    Walkers     m_walkers;
    TechIcons   m_vehicles;
    Reloader    m_reloader;
    HumanIcons  m_humans;
    RobotIcons  m_robots;
    EnemyIcons  m_enemies;
    SmartCursor m_cursor;

    float m_last_move;
    float m_auto_turn;

    float m_update_time;
    float m_update_delta;
};

//~~~~~~~~~~~~~~~~~~~~~~~ inline ~~~~~~~~~~~~~~~~~~~~~~~~~~

inline BaseEntity* GameFormImp::GetCurEnt()
{
    return m_cur_ent;
}

inline HumanIcons* GameFormImp::GetHumanIcons()
{
    return &m_humans;
}

inline TechIcons* GameFormImp::GetTechIcons()
{
    return &m_vehicles;
}

inline Walkers* GameFormImp::GetWalkers()
{
    return &m_walkers;
}

inline SmartCursor* GameFormImp::GetCursor()
{
    return &m_cursor;
}

inline EnemyIcons* GameFormImp::GetEnemyIcons()
{
    return &m_enemies;
}

inline RobotIcons* GameFormImp::GetRobotIcons()
{
    return &m_robots;
}

inline bool GameFormImp::CanShowFOS() const
{
    return (m_flags & FT_SHOW_FOS) != 0;
}

inline Reloader* GameFormImp::GetReloader()
{
    return &m_reloader;
}

#endif // _PUNCH_GAMEFORMIMP_H_
