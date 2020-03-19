//
// наблюдение за игровыми событи€ми
//

#ifndef _PUNCH_GAMEOBSERVER_H_
#define _PUNCH_GAMEOBSERVER_H_

class BaseEntity;

//
// класс - наблюдатель за игровыми событи€ми
//
class GameObserver : public Observer<int>{
public:

    //игровые событи€
    enum event_type{

        EV_NONE,

        EV_HIT,      //было попадание
        EV_TURN,     //передача хода
        EV_KILL,     //убийство существа
        EV_HURT,     //ранение существа
        EV_SHOOT,    //где-то стрельба
        EV_USE,      //use 

        EV_NEW_NEWS,   //старые новости исчезли, по€вились новые
        EV_NEW_GAME,   //начало новой игры        
        EV_NEW_QUEST,  //начало нового квеста
        EV_NEW_SSCENE, //по€вилась нова€ скриптова€ сцена

        EV_MENU,         //переключени€ меню
        EV_START_GAME,   //рассылаетс€ всем при старте игры
        EV_DIARY_CHANGE, //изменилс€ журнал

        EV_INIT_GAME_LOAD,  //подготовка к загрузке игры
        EV_INIT_ENTLOAD,    //подготовка к загрузке существ
        EV_ENITITY_LOADED,  //из Save'a загружено существо
        EV_INIT_LEVEL_EXIT, //подготовка к выходу с уровн€

        EV_QUEST_FINISHED,  //закончен квест
    };

    //информаци€ дл€ выхода с уровн€
    struct exit_level_info{
        std::string m_2level;
        exit_level_info(const std::string& level) : m_2level(level) {}        
    };

    //информаци€ о загрузке существ
    struct entload_info{
        int m_count;
        entload_info(int count = 0) : m_count(count) {}
    };

    //иноформаци€ о существе
    struct entity_info{
        BaseEntity* m_entity;
        entity_info(BaseEntity* ent) : m_entity(ent) {}
    };

    //типы меню
    enum menu_type{
        MT_NONE,
        MT_GAME,
        MT_TRADE,
        MT_HIDDEN,
        MT_JOURNAL,
        MT_INVENTORY,
    };

    //информаци€ о меню
    struct menu_info{
        menu_type   m_menu;
        menu_info(menu_type type = MT_NONE) : m_menu(type){}
    };

    //событи€ св€занные с предачей хода
    enum turn_event_type{
        TEV_BEG,         //начало хода
        TEV_END,         //конец хода  
    };
   
    //информаци€ о передаче хода
    struct turn_info {
        player_type     m_player;
        turn_event_type m_event;

        bool IsEnd() const      { return m_event == TEV_END; }
        bool IsBegining() const { return m_event == TEV_BEG; }

        turn_info(player_type player, turn_event_type event) : 
            m_player(player), m_event(event) {}       
    };

    //событи€ св€занные c убийством
    enum kill_event_type{
        KEV_KILL,        //убийство существа
        KEV_TEAM_CHANGE, //смена команды
    };

    //информаци€ об убийстве существа
    struct kill_info {
        BaseEntity*  m_victim;
        BaseEntity*  m_killer;

        kill_event_type m_event;

        bool IsKill() const { return m_event == KEV_KILL; }

        kill_info(BaseEntity* k, BaseEntity* v, kill_event_type ev = KEV_KILL) :
            m_killer(k), m_victim(v), m_event(ev) {}
    };

    //информаци€ о ранении существа
    struct hurt_info {
        BaseEntity* m_victim;
        BaseEntity* m_killer;

        hurt_info(BaseEntity* k, BaseEntity* v) :
            m_killer(k), m_victim(v) {}
    };

    //информаци€ о стрельбе
    struct shoot_info{
        BaseEntity* m_actor;  //кто стрел€л
        BaseEntity* m_victim; //сущеcтво по которому стрел€ют
        rid_t       m_object; //объект по которому стрел€ют
        point3      m_point;  //точка выстрела

        shoot_info(const point3& pnt, BaseEntity* actor = 0, BaseEntity* victim = 0, const rid_t& object = rid_t()) : 
            m_point(pnt), m_actor(actor), m_victim(victim), m_object(object) {}
    };

    //информаци€ о попадании
    struct hit_info{
        BaseEntity* m_actor;  //кто попал 
        BaseEntity* m_victim; //существо по которому попали

        point3 m_point;       //место попадани€
        rid_t  m_hit_effect;  //название эффекта на попадание
        
        hit_info(BaseEntity* actor, BaseEntity* victim, const point3& point, const rid_t& effect) :
            m_actor(actor), m_victim(victim), m_point(point), m_hit_effect(effect) {}
    };

    //информаци€ о use'е
    struct use_info{
        BaseEntity* m_actor;
        BaseEntity* m_ent4use;
        rid_t       m_obj4use; 

        use_info(BaseEntity* actor, BaseEntity* used) :
            m_actor(actor), m_ent4use(used) {}

        use_info(BaseEntity* actor, const rid_t& rid) :
            m_actor(actor), m_obj4use(rid), m_ent4use(0){}
    };

    //информаци€ об изменени€х в журнале
    struct diary_change{

        enum change_type{
            CT_ADD,  //добавили запись
            CT_DEL,  //удалили запись   
        };

        change_type m_change_type;

        bool IsAdd() const { return m_change_type == CT_ADD; }
        bool IsDel() const { return m_change_type == CT_DEL; }

        int m_record;

        diary_change(change_type change, int rec) : m_change_type(change), m_record(rec) {}
    };

    //информаци€ о квестах
    struct quest_info{
        std::string m_quest;
        quest_info(const std::string& quest) : m_quest(quest) {}
    };
};

//
// рассылка игровых сообщений
//
class GameEvMessenger{
public:

    //singleton
    static GameEvMessenger* GetInst();

    //подписатьс€ на рассылку сообщений
    void Attach(GameObserver* observer, GameObserver::event_t event)
    { m_observers.Attach(observer, event); }

    //отписатьс€ от сообщений
    void Detach(GameObserver* observer)
    { m_observers.Detach(observer); }

    //уведомить всех подписавшихс€ о событии
    void Notify(GameObserver::event_t event, GameObserver::info_t info = 0)
    { m_observers.Notify(0, event, info); }

private:

    GameEvMessenger() {}

private:

    typedef ObserverManager<GameObserver> obs_mgr_t;   
    obs_mgr_t m_observers;
};

#endif // _PUNCH_GAMEOBSERVER_H_

