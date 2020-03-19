#pragma warning(disable:4786)

#include "logicdefs.h"

#include <bitset>

#include "../ScriptScene/ScriptSceneManager.h"

#include "form.h"
#include "Thing.h"
#include "spawn.h"
#include "sscene.h"
#include "player.h"
#include "entity.h"
#include "damager.h"
#include "aiutils.h"
#include "graphic.h"
#include "HexGrid.h"
#include "SndUtils.h"
#include "activity.h"
#include "cameraman.h"
#include "entityaux.h" 
#include "PathUtils.h"
#include "visutils3.h"
#include "TraceUtils.h"
#include "DirtyLinks.h"
#include "questserver.h"
#include "GameObserver.h"
#include "PhraseManager.h"
#include "enemydetection.h"

#include <algorithm>

//======================================================

namespace {
    
    //при превышении этого damage'a проигрывается эффект крови
    const float throw_scatter_radius = 2.0f;
    const float near_shoot_distance  = 1.0f;

    //произвести разброс в окружности
    point3 MakeCircleScatter(const point3& center, float radius)
    {
        point3 dir = AXISX;
        dir = dir * radius * NormRand();

        Quaternion qt;
        qt.FromAngleAxis(TORAD(NormRand()*PIm2), AXISZ);
        
        dir = qt * dir;
        return center + dir; 
    }

//======================================================

//
// отослать состояние действия
//
class ActivityStatusSender: private EntityObserver{
public:

    ActivityStatusSender(BaseEntity* actor) : m_actor(actor)
    {
        if(m_actor){            

            m_actor->Attach(this, EV_PREPARE_DEATH);

            activity_info_s info(activity_info_s::AE_BEG);
            m_actor->Notify(EV_ACTIVITY_INFO, &info);
        }
    }

    ~ActivityStatusSender()
    {
        if(m_actor){        
            
            m_actor->Detach(this);

            activity_info_s info(activity_info_s::AE_END);
            m_actor->Notify(EV_ACTIVITY_INFO, &info);
        }
    }

private:

    void Update(subject_t subj, event_t event, info_t info)
    {
        m_actor->Detach(this);
        m_actor = 0;
    }

private:

    BaseEntity* m_actor;
};

//======================================================

//
// драйвер - класс управляющий действием
//
class Driver{
public:

    virtual ~Driver(){}

    //перехват выполнения действия
    virtual bool Run(Activity* activity, bool activity_result) = 0;

    //расчет одного элементарного шага действия
    virtual activity_command CalcStep(Activity* activity, activity_command user_cmd) = 0;
    //расчет ходов во время действия
    virtual activity_command CalcMovepnts(Activity* activity, activity_command user_cmd) = 0;
};

//======================================================

//
// драйвер - декоратор
//
class DecoratorDrv : public Driver {
public:

    DecoratorDrv(Driver* drv = 0) : m_drv(drv){}
    ~DecoratorDrv(){delete m_drv;}

    activity_command CalcStep(Activity* activity, activity_command user_cmd)
    {
        return m_drv ? m_drv->CalcStep(activity, user_cmd) : user_cmd;
    }

    activity_command CalcMovepnts(Activity* activity, activity_command user_cmd)
    {
        return m_drv ? m_drv->CalcMovepnts(activity, user_cmd) : user_cmd;
    }
    
    bool Run(Activity* activity, bool result)
    {
        return m_drv ? m_drv->Run(activity, result) : result;
    }

private:

    Driver* m_drv;
};

//======================================================

//
// драйвер обновления movepnts
//
class UpdateMPDrv : public DecoratorDrv, private EntityVisitor{
public:

    UpdateMPDrv(BaseEntity* entity, int moves, Driver* drv = 0) :
        DecoratorDrv(drv), m_moves(moves), m_entity(entity){}

    activity_command CalcMovepnts(Activity* activity, activity_command user_cmd)
    {
        m_entity->Accept(*this);
        return DecoratorDrv::CalcMovepnts(activity, user_cmd);
    }

private:

    void Visit(HumanEntity* human)
    {
        HumanContext::Traits* traits = human->GetEntityContext()->GetTraits();
        traits->AddMovepnts( - m_moves);
    }

    void Visit(VehicleEntity* vehicle)
    {
        VehicleContext* context = vehicle->GetEntityContext();
        context->SetMovepnts(context->GetMovepnts() - m_moves);
    }

private:

    int         m_moves;
    BaseEntity* m_entity;
};

//======================================================

//
// Update карты видимости
//
class UpdateVisMapDrv : public DecoratorDrv, private EntityObserver{
public:

    enum {
        F_UPDATE_MARKER    = 1 << 0,
        F_UPDATE_SPECTATOR = 1 << 1,

        F_UPDATE_ALWAYS = 1 << 2,
        F_UPDATE_ONCE   = 1 << 3,    
    };

    UpdateVisMapDrv(BaseEntity* entity, unsigned flags = 0, Driver* drv = 0) :
        DecoratorDrv(drv), m_flags(flags), m_entity(entity)
    { m_entity->Attach(this, EV_DESTROY); }

    ~UpdateVisMapDrv()
    {
        UpdateSpectatorAndMarker();
        if(m_entity) m_entity->Detach(this);
    }

    activity_command CalcStep(Activity* activity, activity_command user_cmd)
    {
        if(m_flags & (F_UPDATE_ONCE|F_UPDATE_ALWAYS)){
            m_flags &= ~F_UPDATE_ONCE;
            UpdateSpectatorAndMarker();
        }

        return DecoratorDrv::CalcStep(activity, user_cmd);
    }

private:

    void UpdateSpectatorAndMarker()
    {
        if(m_entity == 0) return;

        EntityContext* context = m_entity->GetEntityContext();

        if(m_flags & F_UPDATE_MARKER && context->GetMarker())
            VisMap::GetInst()->Update(context->GetMarker(), VisMap::UE_POS_CHANGE);
        
        if(m_flags & F_UPDATE_SPECTATOR && context->GetSpectator())
            VisMap::GetInst()->Update(context->GetSpectator(), VisMap::UE_POS_CHANGE);
    }

    void Update(subject_t subj, event_t event, info_t info)
    {
        m_entity->Detach(this);
        m_entity = 0;
    }

private:

    unsigned    m_flags;
    BaseEntity* m_entity;
};

//======================================================

//
// декоратор для смены стойки
//
class StanceChangeDrv :   public  DecoratorDrv, 
                          private EntityObserver,
                          private MarkerObserver,
                          private SpectatorObserver
{
public:

    StanceChangeDrv(BaseEntity* entity, Driver* drv = 0) : 
        DecoratorDrv(drv), m_entity(entity)
    {
        m_entity->Attach(this, EV_DESTROY);
        m_entity->Attach(this, EV_DEATH_PLAYED);

        EntityContext* context = m_entity->GetEntityContext();

        if(m_entity->Cast2Human()){
            //будeм отслеживать всех кто попдает к нам в поле видимости
            context->GetSpectator()->Attach(this, EV_VISIBLE_MARKER);
            context->GetSpectator()->Attach(this, EV_INVISIBLE_MARKER);
        }

        //будем отслеживать всех кто нас теряет из FOS и замечает
        context->GetMarker()->Attach(this, EV_VISIBLE_FOR_SPECTATOR);
        context->GetMarker()->Attach(this, EV_INVISIBLE_FOR_SPECTATOR);
    }

    ~StanceChangeDrv()
    {
        if(m_entity) DetachAllObservers();
    }

private:

    void Update(Spectator* spectator, SpectatorObserver::event_t event, SpectatorObserver::info_t info)
    {
        AIUtils::ChangeHumanStance(m_entity->Cast2Human());        
    }

    void Update(Marker* marker, MarkerObserver::event_t event, MarkerObserver::info_t info)
    {
        Spectator* spectator = static_cast<spectator_info*>(info)->m_spectator;

        if(HumanEntity* human = spectator->GetEntity() ? spectator->GetEntity()->Cast2Human() : 0)            
            AIUtils::ChangeHumanStance(human);        
    }

    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
    {
        DetachAllObservers();
    }

    void DetachAllObservers()
    {
        m_entity->Detach(this);
        m_entity->GetEntityContext()->GetMarker()->Detach(this);
        m_entity->GetEntityContext()->GetSpectator()->Detach(this);

        m_entity = 0;
    }
 
private:

    BaseEntity* m_entity;
};


//======================================================

//
// декоратор для первой фразы
//
class FirstPhraseDrv :  public DecoratorDrv,
                        private EntityObserver,
                        private SpectatorObserver
{
public:

    FirstPhraseDrv(BaseEntity* entity, Driver* drv = 0) : 
        DecoratorDrv(drv), m_actor(entity), m_fplay(false)
    {
        EntityContext* context = m_actor->GetEntityContext();

        //проверить всех кого видит существо
        VisMap::marker_itor itor = VisMap::GetInst()->marker_begin(context->GetSpectator(), VT_VISIBLE);
        while(itor != VisMap::GetInst()->marker_end()){
            if(BaseEntity* entity = itor->GetEntity()) TestEntity(entity);
            ++itor;
        }

        m_actor->Attach(this, EV_PREPARE_DEATH);
        context->GetSpectator()->Attach(this, EV_VISIBLE_MARKER);    
    }

    ~FirstPhraseDrv()
    {
        if(m_actor) DetachAllObservers();
    }

    activity_command CalcStep(Activity* activity, activity_command user_cmd)
    {
        return DecoratorDrv::CalcStep(activity, m_eids.size() ? AC_STOP : user_cmd);
    }
    
    bool Run(Activity* activity, bool result)
    {
        if(!result) m_fplay = true;

        if(m_fplay = m_fplay && PlayFirstPhrase())
            return true;

        return DecoratorDrv::Run(activity, result);
    }

private:

    struct act_info{
        BaseEntity* m_entity;
        Activity*   m_rotate;        
    };

    //существа кот. должны сказать фразу
    typedef std::set<eid_t>  eid_set_t;
    typedef std::list<act_info> act_lst_t; 

    void Update(Spectator* spectator, SpectatorObserver::event_t event, SpectatorObserver::info_t info)
    {
        if(BaseEntity* entity = static_cast<marker_info*>(info)->m_marker->GetEntity())
            TestEntity(entity);
    }

    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
    {
        DetachAllObservers();
    }

    void TestEntity(BaseEntity* entity)
    {
        if(     PhraseManager::GetFirstPhrases()->CanSay(entity) 
            &&  PhraseManager::GetFirstPhrases()->IsExists(entity))
            m_eids.insert(entity->GetEID());    
    }

    bool PlayFirstPhrase()
    {
        if(m_eids.size()){

            eid_set_t::iterator itor = m_eids.begin();

            while(itor != m_eids.end()){
                act_info info;

                info.m_entity = EntityPool::GetInst()->Get(*itor);

                point3 dir    = m_actor->GetGraph()->GetPos3() - info.m_entity->GetGraph()->GetPos3();
                info.m_rotate = ActivityFactory::GetInst()->CreateRotate(info.m_entity, Dir2Angle(dir), ActivityFactory::CT_TALK_ROTATE);
                
                m_rotates.push_back(info);
                m_eids.erase(itor++);
            }

            return true;
        }

        //проиграть повороты
        act_lst_t::iterator itor = m_rotates.begin();
        while(itor != m_rotates.end()){

            if(!itor->m_rotate->Run(AC_TICK)){

                delete itor->m_rotate;
                
                //скажем первую фразу
                QuestServer::GetInst()->HandleFirstPhrase(m_actor, itor->m_entity);
                m_rotates.erase(itor++);
                return true;
            }

            ++itor;
        }

        return !m_rotates.empty();
    }

    void DetachAllObservers()
    {
        m_actor->Detach(this);
        m_actor->GetEntityContext()->GetSpectator()->Detach(this);

        m_actor = 0;
    }
   
private:
    
    bool  m_fplay;

    BaseEntity* m_actor;
    eid_set_t   m_eids;
    act_lst_t   m_rotates;
};

//======================================================

//
// класс - останавливает действие если замечен враг
//
class OpponentViewDrv : public  DecoratorDrv, 
                        private EntityObserver,
                        private SpectatorObserver
{
public:

    enum flag_type{
        F_PRINT_MSGS = 1 << 1, //выводить сообщения в консоль 
    };

    OpponentViewDrv(BaseEntity* entity, unsigned flags, Driver* drv = 0) : 
        DecoratorDrv(drv), m_entity(entity), m_flags(flags)
    {
        m_entity->Attach(this, EV_PREPARE_DEATH);

        if(m_entity->GetPlayer() == PT_PLAYER)
            PlayerSpectator::GetInst()->GetSpectator()->Attach(this, EV_VISIBLE_MARKER);
        else
            m_entity->GetEntityContext()->GetSpectator()->Attach(this, EV_VISIBLE_MARKER);
    }

    ~OpponentViewDrv()
    {
        if(m_entity) DetachAllObservers();
    }
 
    activity_command CalcStep(Activity* activity, activity_command user_cmd)
    {
        if(m_eids.size()){

            user_cmd = AC_STOP;

            eid_set_t::iterator itor = m_eids.begin();

            while(itor != m_eids.end()){
                ActivityObserver::enemy_spotted_s info(EntityPool::GetInst()->Get(*itor));
                activity->Notify(ActivityObserver::EV_ENEMY_SPOTTED, &info); 
                ++itor;
            }
        }

        return DecoratorDrv::CalcStep(activity, user_cmd);
    }

private:

    typedef std::set<eid_t> eid_set_t;

    void Update(Spectator* spectator, SpectatorObserver::event_t event, SpectatorObserver::info_t info)
    {
        BaseEntity* entity = static_cast<marker_info*>(info)->m_marker->GetEntity();

        if(entity && EnemyDetector::getInst()->isHeEnemy4Me(m_entity, entity)){
            m_eids.insert(entity->GetEID());
            if(m_flags & F_PRINT_MSGS) DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("enemy_spotted").c_str(), m_entity->GetInfo()->GetName().c_str()));
        }
    }       

    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
    {
        DetachAllObservers();
    }

    void DetachAllObservers()
    {
        m_entity->Detach(this);
        m_entity->GetEntityContext()->GetSpectator()->Detach(this);

        PlayerSpectator::GetInst()->GetSpectator()->Detach(this);

        m_entity = 0;
    }

private:

    eid_set_t m_eids;
    unsigned  m_flags;

    BaseEntity* m_entity;
};

//======================================================

//
// класс - менеджер реакций
//
class ReactionDrv : public DecoratorDrv, private EntityObserver, private MarkerObserver{
public:

    ReactionDrv(BaseEntity* entity, Driver* drv = 0) : 
        DecoratorDrv(drv), m_entity(entity), m_fplay(false), m_activity(0)
    {
        m_entity->Attach(this, EV_DESTROY);
        m_entity->Attach(this, EV_DEATH_PLAYED);
        m_entity->Attach(this, EV_PREPARE_DEATH);

        Marker* marker = m_entity->GetEntityContext()->GetMarker();

        marker->Attach(this, EV_VISIBLE_FOR_SPECTATOR);

        //соберем список тех, кто уже на нас реагировал
        VisMap::spectator_itor itor = VisMap::GetInst()->spectator_begin(marker, VT_VISIBLE);
        
        while(itor != VisMap::GetInst()->spectator_end()){

            if(BaseEntity* entity = itor->GetEntity())
                m_can_not_react.insert(entity->GetEID());
            
            ++itor;
        }
    }

    ~ReactionDrv()
    { 
        if(m_entity) DetachAllObservers();
        delete m_activity;
    }

    activity_command CalcStep(Activity* activity, activity_command user_cmd)
    {
        return DecoratorDrv::CalcStep(activity, m_can_react.size() ? AC_STOP : user_cmd);
    }
    
    bool Run(Activity* activity, bool result)
    {
        if(!result) m_fplay = true;
        
        if(m_fplay = m_fplay && PlayReaction(activity))
            return true;

        return DecoratorDrv::Run(activity, result);
    }

private:

    typedef std::set<eid_t> eid_set_t;

    bool PlayReaction(Activity* notifier)
    {  
        if(m_activity){

            if(!m_activity->Run(AC_TICK)){
                delete m_activity;
                m_activity = 0;
            }

            return true;
        }

        //если нас кто-нибудь заметил или перца уже убили
        while(m_can_react.size() && m_entity){

            eid_t eid = *m_can_react.begin();
            m_can_react.erase(m_can_react.begin());
                        
            if(BaseEntity* killer = EntityPool::GetInst()->Get(eid)){

                ActivityObserver::enemy_reacted_s info(killer);
                notifier->Notify(ActivityObserver::EV_ENEMY_REACTED, &info);
                
                int    accuracy = 0;
                point3 pos = m_entity->GetGraph()->GetShotPoint();
                
                if(HumanEntity* human = killer->Cast2Human()){
                    
                    WeaponThing* weapon = static_cast<WeaponThing*>(human->GetEntityContext()->GetThingInHands(TT_WEAPON));
                    accuracy = AIUtils::CalcShootAccuracy(human, weapon, pos);
                    
                }else if(VehicleEntity* vehicle = killer->Cast2Vehicle()){
                    
                    accuracy = vehicle->GetInfo()->GetAccuracy();
                }

                m_activity = ActivityFactory::GetInst()->CreateShoot(killer, pos, AIUtils::NormalizeAccuracy(killer, accuracy), ActivityFactory::CT_REACTION_SHOOT); 
                return true;
            }
        }

        return false;
    }

    void Update(Marker* marker, MarkerObserver::event_t event, MarkerObserver::info_t info)
    {
        if(BaseEntity* entity = static_cast<spectator_info*>(info)->m_spectator->GetEntity()){

            //если перец уже есть в списке 
            if(m_can_not_react.count(entity->GetEID()))
                return;

            if(AIUtils::CanReact(m_entity, entity))
                m_can_react.insert(entity->GetEID());
            else
                m_can_not_react.insert(entity->GetEID());
        }
    }

    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
    {
        if(event == EV_PREPARE_DEATH && m_entity->GetPlayer() == PT_PLAYER)
            Cameraman::GetInst()->FocusEntity(m_entity);

        DetachAllObservers();
    }

    void DetachAllObservers()
    {
        m_entity->Detach(this);
        m_entity->GetEntityContext()->GetMarker()->Detach(this);

        m_entity = 0;
    }

private:
    
    bool m_fplay;

    eid_set_t m_can_react;
    eid_set_t m_can_not_react;

    Activity*   m_activity;
    BaseEntity* m_entity;
};

//======================================================

//
// ограничитель действия по кол-ву ходов
//
class MPLimitDrv : public DecoratorDrv, private EntityVisitor{
public:

    MPLimitDrv(BaseEntity* entity, Driver* drv = 0) : 
      DecoratorDrv(drv), m_entity(entity), m_cmd(AC_TICK){}

    activity_command CalcMovepnts(Activity* activity, activity_command user_cmd)
    {
        m_entity->Accept(*this);

        if(m_cmd == AC_STOP){
            activity->Notify(ActivityObserver::EV_MOVEPNTS_EXPIRIED, 0);
            user_cmd = m_cmd;
        }

        return DecoratorDrv::CalcMovepnts(activity, user_cmd);
    }

private:

    void Visit(HumanEntity* human)
    {
        m_cmd = human->GetEntityContext()->GetTraits()->GetMovepnts() ? AC_TICK : AC_STOP;
    }

    void Visit(VehicleEntity* vehicle)
    {
        m_cmd = vehicle->GetEntityContext()->GetMovepnts() ? AC_TICK : AC_STOP;
    }
        
private:

    activity_command m_cmd;
    BaseEntity*      m_entity;
};


//======================================================

//
// ограничитель действия по кол-ву шагов
//
class StepLimitDrv : public DecoratorDrv, private EntityVisitor{
public:

    enum {
        F_PRINT_MSGS = 1 << 0,  //вывод сообщений в лог
    };

    StepLimitDrv(BaseEntity* entity, unsigned flags, Driver* drv) : 
      DecoratorDrv(drv), m_entity(entity), m_cmd(AC_TICK), m_flags(flags){}

    activity_command CalcStep(Activity* activity, activity_command user_cmd)
    {
        m_entity->Accept(*this);
        
        if(m_cmd == AC_STOP){            
            user_cmd = AC_STOP;
            activity->Notify(ActivityObserver::EV_STEPS_EXPIRIED, 0);
        }

        return DecoratorDrv::CalcStep(activity, user_cmd);
    }

private:

    void Visit(HumanEntity* human)
    {
        HumanContext* context = human->GetEntityContext();

        if(context->GetStepsCount())
            return;

        m_cmd = AC_STOP;
       
        if(m_flags & F_PRINT_MSGS && context->GetTraits()->GetWeight() > context->GetLimits()->GetWeight())
            DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("weight_overload").c_str(), human->GetInfo()->GetName().c_str()));        
    }

    void Visit(VehicleEntity* vehicle)
    {
        if(vehicle->GetEntityContext()->GetStepsCount())
            return;

        m_cmd = AC_STOP;
    }
    
private:

    unsigned    m_flags;
    BaseEntity* m_entity;

    activity_command m_cmd;
};

//======================================================

//
// декоратор показывающий путь
//
class PathShowDrv : public DecoratorDrv {
public:

    PathShowDrv(const pnt_vec_t& path, Driver* drv) :
        m_path(path), DecoratorDrv(drv)
    {
        if(m_path.size()) ShowPath();
    }

    ~PathShowDrv()
    { 
        GraphGrid::GetInst()->Show(GraphGrid::HT_PATH, false);
    }

    activity_command CalcStep(Activity* activity, activity_command user_cmd)
    {
        if(m_path.size()){
            m_path.pop_back();
            ShowPath();
        }

        return DecoratorDrv::CalcStep(activity, user_cmd);
    }    

    bool Run(Activity* activity, bool result)
    {
        if(!result) GraphGrid::GetInst()->Show(GraphGrid::HT_PATH, false);
        return DecoratorDrv::Run(activity, result);
    }

private:

    void ShowPath()
    {
        GraphGrid::GetInst()->SetHexes(GraphGrid::HT_PATH, m_path);
        GraphGrid::GetInst()->Show(GraphGrid::HT_PATH, true);
    }

private:
    
    pnt_vec_t m_path;
};


//======================================================

//
// декоратор для проверки проходмости
//
class PassDrv : public DecoratorDrv, private EntityVisitor{
public:

    PassDrv(BaseEntity* entity, const pnt_vec_t& path, unsigned flags, Driver* drv = 0) :
        m_entity(entity), DecoratorDrv(drv), m_path(path), m_activity(0), m_flags(flags)
    {
        if(m_flags & F_CALC_DANGER_HEXES){

            ipnt2_t pos = entity->GetGraph()->GetPos2();

            if(HexGrid::GetInst()->Get(pos).GetRisk())
                m_flags &= ~F_STOP_ON_DANGER_HEXES;
            else
                m_flags |= F_STOP_ON_DANGER_HEXES;
        }
    }

    enum flags_type{
        F_STOP_BEFORE_HUMAN    = 1 << 0,
        F_STOP_BEFORE_ALLY     = 1 << 1,
        F_CALC_DANGER_HEXES    = 1 << 2,
        F_STOP_ON_DANGER_HEXES = 1 << 3,
    };

    activity_command CalcStep(Activity* activity, activity_command user_cmd)
    {
        m_activity = activity;
        m_command = user_cmd;

        if(m_path.size()) m_entity->Accept(*this);

        return DecoratorDrv::CalcStep(activity, m_command);
    }

private:

    void Visit(HumanEntity* human)
    {
        pnt_vec_t track;
        
        PathUtils::GetInst()->CalcTrackField(m_entity, m_path.back(), &track);
        
        AreaManager area(track);
        AreaManager::iterator itor = area.begin();

        if(itor != area.end()){
            m_command = AC_STOP;
            ActivityObserver::meet_entity_s info(&*itor);
            m_activity->Notify(ActivityObserver::EV_MEET_ENTITY, &info);
        }

        RiskSite* risk = HexGrid::GetInst()->Get(human->GetGraph()->GetPos2()).GetRisk();

        if(risk && m_flags & F_CALC_DANGER_HEXES){
            
            human->GetEntityContext()->SetFlameSteps(human->GetEntityContext()->GetFlameSteps() + 1);

            if((human->GetEntityContext()->GetFlameSteps() % GROUND_FIRE_PATH_LENGTH) == 0){

                if(m_flags & F_STOP_ON_DANGER_HEXES){
                    m_command = AC_STOP;
                    DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("act_danger_zone").c_str(), m_entity->GetInfo()->GetName().c_str()));
                }

                //~~~~~~~~~~~~ временное решение ~~~~~~~~~~~~~~

                unsigned dmg_flags = EntityDamager::DEFAULT_FLAGS;
                dmg_flags &= ~EntityDamager::FT_PLAY_ENTITY_HURT;

                //~~~~~~~~~~~~ временное решение ~~~~~~~~~~~~~~

                EntityDamager::GetInst()->Damage(human, risk, dmg_flags);
                m_activity->Notify(ActivityObserver::EV_DANGER_ZONE);
            }
        }

        if(risk == 0) m_flags |= F_STOP_ON_DANGER_HEXES;

        m_path.pop_back();
    }

    void Visit(VehicleEntity* vehicle)
    {
        pnt_vec_t track;
        
        PathUtils::GetInst()->CalcTrackField(m_entity, m_path.back(), &track);
        
        AreaManager area(track);
        AreaManager::iterator itor = area.begin();

        while(itor != area.end()){

            BaseEntity* entity = &*itor;

            ActivityObserver::meet_entity_s info(entity);
            m_activity->Notify(ActivityObserver::EV_MEET_ENTITY, &info);

            if(     entity->Cast2Vehicle()
                ||  vehicle->GetInfo()->IsRobot()
                || (m_flags & F_STOP_BEFORE_HUMAN && entity->Cast2Human())
                || (m_flags & F_STOP_BEFORE_ALLY && entity->GetPlayer() == vehicle->GetPlayer()))
                m_command = AC_STOP;
            else
                entity->GetEntityContext()->PlayDeath(vehicle);

            ++itor;
        }

        m_path.pop_back();
    }

private:
    
    activity_command m_command;

    pnt_vec_t   m_path;
    Activity*   m_activity;
    BaseEntity* m_entity;

    unsigned    m_flags;
};

//======================================================

//
// уменьшение точноcти при беге
//
class DecAccuracyDrv : public DecoratorDrv, private EntityObserver{
public:

	DecAccuracyDrv(HumanEntity* human, Driver* drv) : 
			m_human(human), DecoratorDrv(drv), m_hex_counter(0)
				{
        m_human->Attach(this, EV_PREPARE_DEATH);
				}
			~DecAccuracyDrv() 
				{
        if(m_human) m_human->Detach(this);
				}

    activity_command CalcStep(Activity* activity, activity_command user_cmd)
    {
        m_hex_counter++;
        return DecoratorDrv::CalcStep(activity, user_cmd);
    }

    bool Run(Activity* activity, bool result)
    {
        if(m_human && !result){
            
            HumanContext::Traits* traits = m_human->GetEntityContext()->GetTraits();
            traits->AddAccuracy( - m_hex_counter/2.0f);

            if(m_human) m_human->Detach(this);
            m_human = 0;
        }

        return DecoratorDrv::Run(activity, result);
    }
private:
    void Update(BaseEntity* entity, event_t event, info_t info)
    {
        m_human->Detach(this);
        m_human = 0;
    }

    int    m_hex_counter;
    HumanEntity* m_human;
};

//======================================================

//
// действие обладающее общей функциональностью
//
class BaseActivity : public Activity{
public:

    BaseActivity(Driver* driver = 0, Camerawork* camera = 0) : 
      m_driver(driver), m_camerawork(camera){}

    ~BaseActivity()
    { 
        delete m_driver;
        delete m_camerawork;
    }

protected:

    bool DrvRun(bool result)
    {
        result = m_driver ? m_driver->Run(this, result) : result;
        return m_camerawork ? m_camerawork->Film(result) : result;
    }

    activity_command DrvCalcStep(activity_command user_cmd)
    {
        return m_driver ? m_driver->CalcStep(this, user_cmd) : user_cmd;
    }

    activity_command DrvCalcMovepnts(activity_command user_cmd)
    {
        return m_driver ? m_driver->CalcMovepnts(this, user_cmd) : user_cmd;
    }

private:
    
    Driver*  m_driver;
    Camerawork* m_camerawork;
};

//======================================================

//
// поворот сущесва
//
class RotateActivity : public BaseActivity{
public:

    RotateActivity(BaseEntity* entity, float angle, Driver* drv = 0, Camerawork* cam = 0) :
        BaseActivity(drv, cam), m_entity(entity), m_step_delta(PI/18), m_angle(angle),
        m_step_last(0), m_mps_delta(PI/2), m_mps_last(0), m_end_time(-1), m_activity_sender(entity) {}

    bool Run(activity_command user_cmd)
    {
        //вызываемся в первый раз?
        if(m_end_time < 0){

            if(DrvCalcMovepnts(AC_TICK) == AC_STOP){
                m_end_time = 0;
                return DrvRun(m_end_time > Timer::GetSeconds());
            }
            
            m_mps_last = m_step_last = m_entity->GetGraph()->GetAngle();
            m_end_time = m_entity->GetGraph()->SetAngle(m_angle);                 
        }

        float current = m_entity->GetGraph()->GetAngle();

        if(FastAbs(current - m_step_last) > m_step_delta){
            m_step_last = current;
            user_cmd = DrvCalcStep(user_cmd); 
            if(user_cmd == AC_STOP) Stop();
        }

        if(FastAbs(current - m_mps_last) > m_mps_delta){
            m_mps_last = current;
            user_cmd = DrvCalcMovepnts(user_cmd);
            if(user_cmd == AC_STOP) Stop();
        }

        if(!DrvRun(m_end_time > Timer::GetSeconds())){  
            m_entity->GetGraph()->SetLoc(m_entity->GetGraph()->GetPos3(), m_angle);
            DrvCalcStep(AC_TICK);
            return false;
        }

        return true;
    }

private:

    void Stop()
    {
        m_end_time = 0;
        m_entity->GetGraph()->ChangeAction(GraphEntity::AT_STAY);
    }

private:

    float m_angle;
    float m_end_time;
    float m_mps_last;
    float m_step_last;

    const float m_mps_delta;
    const float m_step_delta;

    BaseEntity* m_entity;

    ActivityStatusSender m_activity_sender;
};

//======================================================

//
// перемещение существом
//
class MoveActivity : public BaseActivity, private MoveEvents, private EntityObserver{
public:

    MoveActivity(BaseEntity* entity, const pnt_vec_t& path, Driver* drv = 0, Camerawork* cam = 0) :
        m_entity(entity), m_first_time(true),  m_activity_sender(entity),
        m_fend(true), BaseActivity(drv, cam), m_usr_cmd(AC_TICK), m_path(path)
    {
        m_entity->Attach(this, EV_PREPARE_DEATH);
    }

    ~MoveActivity()
    {  
        if(m_entity) m_entity->Detach(this);
    }

    bool Run(activity_command cmd)
    {
        if(m_first_time){
            
            m_first_time = !m_first_time;
           
            PathUtils::GetInst()->UnlinkEntity(m_entity);

            if(DrvCalcStep(AC_TICK) == AC_STOP){
                {if(m_entity)PathUtils::GetInst()->LinkEntity(m_entity);}
                return DrvRun(!m_fend);
            }

            /*
            if(DrvCalcMovepnts(AC_TICK) == AC_STOP){
                PathUtils::GetInst()->LinkEntity(m_entity);
                return DrvRun(!m_fend);
            }
            */

            m_fend = m_entity?false:true;//grom
            if(m_entity) m_entity->GetGraph()->Move(m_path, this);            
        }

        //stop нельзя отменить
        if(m_usr_cmd != AC_STOP && cmd != AC_TICK)
            m_usr_cmd = cmd;

        return DrvRun(!m_fend && m_entity);
    }
  
    cmd_type OnStep(MoveEvents::event_type event)
    {
        if(m_fend = event == ET_END){
            PathUtils::GetInst()->LinkEntity(m_entity);
            return CT_MOVE;
        }

        if(DrvCalcMovepnts(m_usr_cmd) == AC_STOP)
            return CT_STOP;

        if(DrvCalcStep(m_usr_cmd) == AC_STOP)
            return CT_STOP;

        if(m_usr_cmd == AC_SPEED_UP)
            return CT_SPEED_UP;

        return CT_MOVE;
    }

private:

    void Update(BaseEntity* entity, event_t event, info_t info)
    {
        m_entity->Detach(this);
        m_entity = 0;
        m_fend = true;
    }

private:

    bool m_fend;
    bool m_first_time;

    pnt_vec_t   m_path;
    BaseEntity* m_entity;

    activity_command m_usr_cmd;

    ActivityStatusSender m_activity_sender;
};

//======================================================

//
// класс для проверки наличия линии огня (line of fire) между точками
//
class LOFCheck{
public:

    virtual ~LOFCheck(){}

    //проверка наличия линии огня
    virtual bool IsExist(BaseEntity* entity, const point3& from, const point3& to) = 0;
};

//======================================================

//
// класс кот производит трассировку для проверки линии видимости
//
class TracingLOFCheck : public LOFCheck{
public:

    TracingLOFCheck(eid_t ent, const rid_t& obj, bool fprint) : 
      m_fprint(fprint), m_victim(ent), m_object(obj) {}

    bool IsExist(BaseEntity* entity, const point3& from, const point3& to)
    { 
        //нет существа - нет проверки
        if(entity == 0) return false;
        
        LOSTracer los;
 
        los.SetFirst(entity->GetEID(), from);

        if(m_object.size())
            los.SetSecond(m_object, to);
        else 
            los.SetSecond(m_victim, to);

        los.CalcLOS();
        
        //вывод сообщения
        if(!los.HaveLOS() && m_fprint){

            shot_type shot = SHT_AIMSHOT;
            if(HumanEntity* human = entity->Cast2Human()) shot = human->GetEntityContext()->GetShotType();
            
            entity->Notify(EntityObserver::EV_NO_LOF);

            DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("no_lof").c_str(), entity->GetInfo()->GetName().c_str(), AIUtils::Shot2Str(shot)));
        }

        return los.HaveLOS();
    }

private:

    eid_t m_victim;
    rid_t m_object;
    bool  m_fprint;
};

//======================================================

//
// менеджер наведения на цель
//
class TargetManager{
public:

    virtual ~TargetManager(){}

    //узнать целeвую точку
    virtual point3 GetTarget() = 0;

    //информация о трассировке
    struct trace_info{
        point3 m_end;

        eid_t  m_entity;
        rid_t  m_object;
        
        ShotTracer::material_type m_material;
    };

    //провести первую трассировку
    virtual void MakeFirstTrace(BaseEntity* entity, const point3& from, trace_info* info) = 0;
    //провести последующую
    virtual void MakeNextTrace(BaseEntity* entity, const point3& from, const point3& dir, trace_info* info) = 0;
};

//
// реализация менеджера прицеливания
//
class TargetManagerImp : public  TargetManager,
                         private EntityObserver
{
public:

    TargetManagerImp(float accuracy, const point3& target, const rid_t& obj, eid_t victim) :
        m_victim(victim ? EntityPool::GetInst()->Get(victim) : 0),  
        m_accuracy(accuracy), m_target(target), m_object(obj), m_near_dist(2.0f)
    {
        if(m_victim) m_victim->Attach(this, EV_DEATH_PLAYED);
    }

    ~TargetManagerImp()
    {
        if(m_victim) m_victim->Detach(this);
    }
    
    point3 GetTarget() { return m_target; }

    void MakeFirstTrace(BaseEntity* entity, const point3& from, trace_info* info)
    {
        point3 ent_pos = entity->GetGraph()->GetPos3();
              
        //если мы рядом и есть препятствие мы всегда попадаем
        if(     (ent_pos - m_target).Length() < m_near_dist
            &&  (IsVictimExists(entity) || IsObjectExists())){
            
            if(IsVictimExists(entity)){
                
                info->m_object.clear();

                info->m_entity   = m_victim->GetEID();
                info->m_material = ShotTracer::MT_ENTITY;
                info->m_end      = m_victim->GetGraph()->GetShotPoint(from);
                return;
            }
            
            if(IsObjectExists()){
                info->m_entity   = 0;
                info->m_end      = m_target;
                info->m_object   = m_object;
                info->m_material = ShotTracer::MT_OBJECT;
                return;
            }
        }

        float  ent_ang = entity->GetGraph()->GetAngle() - PId2;
 
        //вектор направления сущ-ва
        point3 entity_dir(cosf(ent_ang), sinf(ent_ang), 0);
        
        //вектор направления оружия сущ-ва 
        point3 from_entpos_2_barrel(from - ent_pos);
        from_entpos_2_barrel.z = 0;
        from_entpos_2_barrel = Normalize(from_entpos_2_barrel);

        //направление трассировки
        point3 trace_dir = MakeScatter(m_target - from);

        //вектора направелния от оружия до цели
        point3 from_barrel_2_target = trace_dir;
        from_barrel_2_target.z = 0;
        from_barrel_2_target = Normalize(from_barrel_2_target);

        //если можем убить себя то корректируем направление трассировки
        if(     entity_dir.Dot(from_entpos_2_barrel) >= 0
            &&  from_barrel_2_target.Dot(from_entpos_2_barrel) < 0)
            trace_dir = entity_dir;


        //трассируем выстрел
        MakeNextTrace(entity, from, trace_dir, info);        
    }
    
    void MakeNextTrace(BaseEntity* entity, const point3& from, const point3& dir, trace_info* info)
    {
        //трассируем выстрел
        ShotTracer trace(entity ? entity->GetEID() : 0, from, dir);

        //вернем результаты трассировки        
        info->m_end      = trace.GetEnd(); 
        info->m_entity   = trace.GetEntity();
        info->m_object   = trace.GetObject();
        info->m_material = trace.GetMaterial();        
    }

private:

    void Update(subject_t subj, event_t event, info_t info)
    {
        m_victim->Detach(this);
        m_victim = 0;
    }

    bool IsVictimExists(BaseEntity* actor) const
    { return m_victim && m_victim->GetEID() != actor->GetEID(); }

    bool IsObjectExists() const
    { return m_object.size() && !GameObjectsMgr::GetInst()->IsDestroyed(m_object); }

    //сделать разбос по направлению
    point3 MakeScatter(point3 dir)
    {
        const float factor = 0.25;
        const float max_z  = 1.5;

        //если попали то выходим
        if(m_accuracy > NormRand())
            return dir;
                
        float angle = factor*100.0f*NormRand()*(1 - m_accuracy)*(NormRand() < 0.5 ? 1 : -1);
        
        Quaternion qt;
        qt.FromAngleAxis(TORAD(angle), AXISZ);
        
        float dz = NormRand() * max_z * (1.0f - m_accuracy); 
        dz = NormRand() < 0.5 ? dz : -dz;
        
        dir = qt * dir;
        dir.z += dz;

        return dir;        
    }

private:

    const float m_near_dist;

    rid_t       m_object;
    BaseEntity* m_victim;

    point3 m_target;
    float  m_accuracy;
};

//======================================================

//
// рассылка сообщений о стрельбе
//
class ShootNotifier{
public: 

    virtual ~ShootNotifier() {}

    //сообщение о промахе
    virtual void SendShoot(BaseEntity* actor, const point3& pos) = 0;
    //сообщение о попадании в сущетсво
    virtual void SendHit(BaseEntity* actor, const point3& pos, BaseEntity* victim, const rid_t& effect) = 0;    
};

//
// реализация уведомлений о стрельбе
//
class ShootNotifierImp : public ShootNotifier, private EntityObserver{
public:

    ShootNotifierImp(BaseEntity* victim, const rid_t& object) :
      m_object(object), m_victim(victim)
    {
        if(m_victim) m_victim->Attach(this, EV_PREPARE_DEATH);
    }

    ~ShootNotifierImp()
    {
        if(m_victim) m_victim->Detach(this);
    }

    void SendShoot(BaseEntity* actor, const point3& pos)
    {
        GameObserver::shoot_info info(pos, actor, m_victim, m_object);
        GameEvMessenger::GetInst()->Notify(GameObserver::EV_SHOOT, &info);
    }

    void SendHit(BaseEntity* actor, const point3& pos, BaseEntity* victim, const rid_t& effect)
    {
        SendShoot(actor, pos);

        GameObserver::hit_info info(actor, victim, pos, effect);
        GameEvMessenger::GetInst()->Notify(GameObserver::EV_HIT, &info);
    }

private:

    void Update(subject_t subject, event_t event, info_t info)
    {
        m_victim->Detach(this);
        m_victim = 0;
    }

private:

    rid_t       m_object;
    BaseEntity* m_victim;
};

//======================================================

//
// вынесеный наружу механизм стрельбы
//
class ShootManager{
public:

    virtual ~ShootManager(){}

    // инициализировать стрельбу
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // вернуть направление в кот надо развернуться для стрельбы
    //
    virtual void Init(Activity* activity, BaseEntity* entity) = 0;
    //остановить стрельбу
    virtual void Stop(Activity* activity, BaseEntity* entity) = 0;
    //выстрелить патрон
    virtual void Shoot(Activity* activity, BaseEntity* entity) = 0;
    //все выстрелили?
    virtual bool IsDone(Activity* activity, BaseEntity* entity) = 0;
    //проиграть полет пули
    virtual bool Run(Activity* activity, BaseEntity* entity) = 0; 
};

//
// реалиазация механизма стрельбы
//
class ShootManagerImp : public ShootManager{
public:

    ShootManagerImp(TargetManager* gunner, unsigned entity_dmgr,
                    ShootNotifier* notifier = 0, LOFCheck* lof = 0) :
        m_gunner(gunner), m_lof_check(lof), m_notifier(notifier),
        m_entity_damager(entity_dmgr) {}

    ~ShootManagerImp()
    {
        delete m_gunner;
        delete m_notifier;
        delete m_lof_check;
        delete m_effect_player;
    }

    void Init(Activity* activity, BaseEntity* entity)
    {
        Initializer initer;

        entity->Accept(initer);

        m_hit = initer.GetHit();
        m_barrel = initer.GetBarrel();
        m_shots_count = initer.GetShots();
        m_effect_player = initer.GetEffectPlayer();
    }

    void Shoot(Activity* activity, BaseEntity* entity)
    {
        if(m_shots_count == 0) return;
        
        entity->GetGraph()->GetBarrel(m_barrel, &m_hit.m_from);

        m_shots_count--;                

        MakeShot(entity, m_hit.m_from, shot_s(m_hit));

        entity->Accept(AmmoDec());
    }

    void Stop(Activity* activity, BaseEntity* entity)
    {
        m_shots_count = 0;
    }

    bool IsDone(Activity* activity, BaseEntity* entity)
    {
        //если можем проверить линию стрельбы - проверим
        if(m_lof_check){

            point3 from;

            entity->GetGraph()->GetBarrel(m_barrel, &from);

            //если нет линии стрельбы - не стреляем
            if(!m_lof_check->IsExist(entity, from, m_gunner->GetTarget())){
                m_shots_count = 0;
                activity->Notify(ActivityObserver::EV_NO_LOF, 0);
            }
            
            //проверка происходит однажды
            delete m_lof_check;
            m_lof_check = 0;
        }

        return m_shots_count == 0;
    }

    bool Run(Activity* activity, BaseEntity* entity)
    {
        shot_lst_t::iterator itor = m_shots.begin();

        while(itor != m_shots.end()){
            
            if(itor->m_end_time < Timer::GetSeconds()){
                
                //выстрел уже закончился
				if( !DirtyLinks::IsGoodEntity(entity) ) //Grom
					entity=NULL;

                if(IsTargetDestroyed(*itor))
				{if(entity) MakeShot(entity, itor->m_from, *itor, itor->m_to - itor->m_from);}
                else                    
                {if(entity) MakeHit(entity, *itor); }

				if( !DirtyLinks::IsGoodEntity(entity) ) //Grom
					entity=NULL;
                                                
                m_shots.erase(itor++);
                continue;
            }
            
            ++itor;
        }
        
        return !m_shots.empty();
    }

private:

    struct shot_s;
    struct effects_s;

    //
    // абстракция проигрывателя эффектов
    //
    class EffectPlayer {
    public:
        
        virtual ~EffectPlayer() {}

        //узнать эффекп при попадании
        virtual const rid_t GetHitEffect(ShotTracer::material_type material) const = 0;

        //поиграть эффект выстрела
        virtual void PlayFlash(BaseEntity* entity, const point3& to) = 0;
        //проиграть полет пули
        virtual float PlayTrace(const point3& from, const point3& to) = 0;
        //проиграть попадание
        virtual void PlayHit(ShotTracer::material_type material, const point3& from, const point3& to) = 0;
    };

    //
    // обычный проигрыватель эффектов
    //
    class UsualEffectPlayer : public EffectPlayer{
    public:

        UsualEffectPlayer(const rid_t& hit, const rid_t& trace) : 
          m_hit(hit), m_trace(trace) {}

        float PlayTrace(const point3& from, const point3& to)
        {
            return EffectMgr::GetInst()->MakeTracer(m_trace, from, to);
        }

        void PlayFlash(BaseEntity* entity, const point3& to)
        {
            if(VehicleEntity* vehicle = entity->Cast2Vehicle()){
 
                const VehicleInfo* info = vehicle->GetInfo();

                PlayEntityFlashEffect(vehicle, NULLVEC, to, info->GetShotSound(), info->GetFlashEffect());
                if(info->HaveShellOutlet()) PlayEntityShellsEffect(vehicle, info->GetShellOutlet(), to, info->GetShellsEffect());

            }else if(HumanEntity* human = entity->Cast2Human()){

                HumanContext* context = human->GetEntityContext();  
                WeaponThing*  weapon  = static_cast<WeaponThing*>(context->GetThingInHands(TT_WEAPON));
                
                PlayEntityFlashEffect(human, weapon->GetInfo()->GetBarrel(), to, weapon->GetInfo()->GetShotSound(context->GetShotType()), weapon->GetInfo()->GetFlashEffect());
                if(weapon->GetInfo()->HaveShellOutlet()) PlayEntityShellsEffect(human, weapon->GetInfo()->GetShellOutlet(), to, weapon->GetAmmo()->GetInfo()->GetShellsEffect());
            }
        }

        void PlayHit(ShotTracer::material_type material, const point3& from, const point3& to)
        {
            EffectMgr::GetInst()->MakeHit(GetHitEffect(material), from, to);
        }

        const rid_t GetHitEffect(ShotTracer::material_type material) const
        {
            switch(material){
            case ShotTracer::MT_AIR:
            case ShotTracer::MT_NONE:
                return "";
                
            case ShotTracer::MT_WALL:
            case ShotTracer::MT_ENTITY:
            case ShotTracer::MT_OBJECT:
                return m_hit;
                
            case ShotTracer::MT_SHIELD:
                return "hit_shield";
            }

            return "";
        }

    private:

        void PlayEntityFlashEffect( BaseEntity* entity, const point3& barrel,
            const point3& to, const rid_t& shot_sound, const rid_t& flash)
        {
            point3 from;

            //узнать расположение ствола
            entity->GetGraph()->GetBarrel(barrel, &from);
            
            //проиграть звук выстрела
            PlaySound(from, shot_sound);
            //проиграть вспышку выстрела
            EffectMgr::GetInst()->MakeFlash(flash, from, to);
        }

        void PlayEntityShellsEffect(BaseEntity* entity, const point3& outlet,
                                    const point3& to, const std::string& shells)
        {
				if(shells.empty() || shells=="-") return;//grom
            point3 from;            
            entity->GetGraph()->GetBarrel(outlet, &from);
            EffectMgr::GetInst()->MakeShellOutlet(entity, shells, from, to);
        }

    protected:

        virtual void PlaySound(const point3& from, const rid_t& snd)
        {
            SndPlayer::GetInst()->Play(from, snd);
        }

    private:

        rid_t  m_hit;
        rid_t  m_trace;
    };

    //
    // проигрывает единичный звук выстрела
    //
    class SingleShotEffectPlayer : public UsualEffectPlayer{
    public:

        SingleShotEffectPlayer(const rid_t& hit, const rid_t& trace) :
            UsualEffectPlayer(hit, trace), m_counter(0) {}

    protected:

        void PlaySound(const point3& from, const rid_t& rid)
        {
            if(m_counter++ == 0) UsualEffectPlayer::PlaySound(from, rid);
        }

    private:

        int m_counter;
    };

    //
    // инициализация в зависимости от типа существа
    //
    class Initializer : public EntityVisitor{
    public:

        Initializer() : m_shots(0), m_barrel(NULLVEC) {}

        void Visit(HumanEntity* human)
        {
            WeaponThing* weapon = static_cast<WeaponThing*>(human->GetEntityContext()->GetThingInHands(TT_WEAPON));
            
            shot_type sht_type = human->GetEntityContext()->GetShotType();
            m_shots = weapon->GetShotCount(sht_type);
            
            //запомнить повреждения
            m_hit.m_dmg[hit_t::BW_DMG].m_type = weapon->GetAmmo()->GetInfo()->GetBDmg().m_type;
            m_hit.m_dmg[hit_t::BW_DMG].m_val  = weapon->GetInfo()->GetBDmg().m_val + weapon->GetAmmo()->GetInfo()->GetBDmg().m_val;                
            
            //запомнить weapon additional damage
            m_hit.m_dmg[hit_t::AW_DMG] = weapon->GetInfo()->GetADmg();                
            
            //запомнить ammo aditional damage
            m_hit.m_dmg[hit_t::AA_DMG] = weapon->GetAmmo()->GetInfo()->GetADmg();
            
            //запомнить радиус повреждения
            m_hit.m_radius = weapon->GetAmmo()->GetInfo()->GetDmgRadius();
            
            rid_t hit   = weapon->GetAmmo()->GetInfo()->GetHitEffect(),
                  trace = weapon->GetAmmo()->GetInfo()->GetTraceEffect();

            if(sht_type == SHT_AUTOSHOT)
                m_player = new SingleShotEffectPlayer(hit, trace);
            else
                m_player = new UsualEffectPlayer(hit, trace);

            //узнать коорд стволов оружия
            m_barrel = weapon->GetInfo()->GetBarrel();
        }

        void Visit(VehicleEntity* vehicle)
        {
            m_shots = vehicle->GetInfo()->GetQuality();

            if(m_shots > vehicle->GetEntityContext()->GetAmmoCount())
                m_shots = vehicle->GetEntityContext()->GetAmmoCount();
            
            //запомнить basic damage
            m_hit.m_dmg[hit_t::BW_DMG] = vehicle->GetInfo()->GetBDmg();
            
            //запомнить weapon additional damage
            m_hit.m_dmg[hit_t::AW_DMG] = vehicle->GetInfo()->GetADmg();
            
            //запомнить ammo aditional damage
            m_hit.m_dmg[hit_t::AA_DMG].m_val   = 0;
            m_hit.m_dmg[hit_t::AA_DMG].m_type = DT_NONE;

            //запомнить радиус повреждения
            m_hit.m_radius = vehicle->GetInfo()->GetDmgRadius();
            
            //запомнить эффект попадания
            m_player = new UsualEffectPlayer( vehicle->GetInfo()->GetHitEffect(),
                                              vehicle->GetInfo()->GetTraceEffect());
        }

        int GetShots() const { return m_shots; }

        const hit_t& GetHit() const { return m_hit; }

        const point3& GetBarrel() const { return m_barrel; }

        EffectPlayer* GetEffectPlayer() const { return m_player; }
   
    private:

        hit_t  m_hit;
        int    m_shots; 
        point3 m_barrel;

        EffectPlayer* m_player;
    }; 

    //
    // уменьшить кол-во патронов в оружии существа
    //
    class AmmoDec : public EntityVisitor{
    public:

        AmmoDec(int count = 1) : m_count(count) {}

        void Visit(HumanEntity* human)
        {
            WeaponThing* weapon = static_cast<WeaponThing*>(human->GetEntityContext()->GetThingInHands(TT_WEAPON));

            weapon->GetAmmo()->SetCount(weapon->GetAmmo()->GetCount() - m_count);
            if(weapon->GetAmmo()->GetCount() == 0) delete weapon->Load(0);
        }

        void Visit(VehicleEntity* vehicle)
        {
            vehicle->GetEntityContext()->SetAmmoCount(vehicle->GetEntityContext()->GetAmmoCount() - m_count);
        }

    private:

        int m_count;
    };
        
    void MakeShot(BaseEntity* entity, point3& from, shot_s& shot, point3 dir = NULLVEC)
    {
        //посчитаем трассировки
        shot.m_count ++;

        TargetManager::trace_info trace_info;

        //отmрассируем выстрел
        if(shot.m_count == 1)
            m_gunner->MakeFirstTrace(entity, from, &trace_info);
        else
            m_gunner->MakeNextTrace(entity, from, dir, &trace_info);
        
        //проиграем эффект полета пули
        shot.m_from     = from;  
        shot.m_to       = trace_info.m_end; 
        shot.m_entity   = trace_info.m_entity;
        shot.m_object   = trace_info.m_object;
        shot.m_material = trace_info.m_material;        
        
        //разослать сообщение о выстреле
        if(m_notifier) m_notifier->SendShoot(entity, shot.m_from);
       
        shot.m_end_time = 0;

        //проиграем эффекты полета пули
        shot.m_end_time = m_effect_player->PlayTrace(shot.m_from, shot.m_to);        
        
        //проиграем звук выстрела и эффект вылета пули
        if(shot.m_count == 1) m_effect_player->PlayFlash(entity, shot.m_to);

        //поставим выстрел в список
        m_shots.push_back(shot);
    }
    
    void MakeHit(BaseEntity* entity, const shot_s& shot)
    {
        //проиграем эффект попадания
        m_effect_player->PlayHit(shot.m_material, shot.m_from, shot.m_to);

        //запустим механизм объемного взрыва
        switch(shot.m_material){
        case ShotTracer::MT_WALL:
            Damager::GetInst()->HandleLevelHit(entity, shot, m_entity_damager);
            break;

        case ShotTracer::MT_SHIELD:
            Damager::GetInst()->HandleShieldHit(entity,  shot, m_entity_damager);
            break;
        
        case ShotTracer::MT_OBJECT:
            Damager::GetInst()->HandleObjectHit(entity, shot.m_object, shot, m_entity_damager);
            break;

        case ShotTracer::MT_ENTITY:
            {
                BaseEntity* victim = EntityPool::GetInst()->Get(shot.m_entity);
                Damager::GetInst()->HandleEntityHit(entity, victim, shot, m_entity_damager);
            }
            break;

        case ShotTracer::MT_AIR:
        case ShotTracer::MT_NONE:
            Damager::GetInst()->HandleAirHit(entity,  shot, m_entity_damager);
            break;
        }

		if(!DirtyLinks::IsGoodEntity(entity))
			entity = NULL;//Grom

        //разослать уведомления о взрыве
        if(shot.m_material == ShotTracer::MT_ENTITY){
            BaseEntity* victim = EntityPool::GetInst()->Get(shot.m_entity);
            if(m_notifier) m_notifier->SendHit(entity, shot.m_to, victim, m_effect_player->GetHitEffect(shot.m_material));
        }else{
            if(m_notifier) m_notifier->SendHit(entity, shot.m_to, 0, m_effect_player->GetHitEffect(shot.m_material));
        }
    }

    bool IsTargetDestroyed(const shot_s& shot) const
    {
        return      (shot.m_material == ShotTracer::MT_ENTITY  && !EntityPool::GetInst()->Get(shot.m_entity))
               ||   (shot.m_material == ShotTracer::MT_OBJECT  && GameObjectsMgr::GetInst()->IsDestroyed(shot.m_object));
    }

private:


    hit_t  m_hit;
    point3 m_barrel;
    int    m_shots_count; //кол-во выстрелов
 
    LOFCheck*       m_lof_check;
    TargetManager*  m_gunner;
    ShootNotifier*  m_notifier;

    unsigned m_entity_damager;

    //информация о выстреле
    struct shot_s : public hit_t{

        int   m_count;    //количество трассировок выстрела

        eid_t m_entity;   //существо на кот закончилась трассировка    
        rid_t m_object;   //объект на кот закончилась трассировка
        float m_end_time; //время когда закончиться проигрывание эффекта стрельбы        

        ShotTracer::material_type m_material; //материал на кот окнчился эффект

        shot_s(const hit_t& hit) : m_count(0) { static_cast<hit_t&>(*this) = hit; }
    };

    typedef std::list<shot_s> shot_lst_t;
    shot_lst_t m_shots;

    EffectPlayer* m_effect_player;
};

//======================================================

//
// стрельба для существа
//
class ShootActivity : public BaseActivity, private EntityObserver, private EntityVisitor{
public:

    enum {
        
        F_CALC_STEP     = 1 << 0,
        F_CALC_MOVEPNTS = 1 << 1,

        F_WAS_SHOOTING  = 1 << 2,
    };

    ShootActivity(BaseEntity* entity, ShootManager* shoot, const point3& to, Driver* drv = 0, Camerawork* cam = 0) : 
        m_shoot(shoot), m_action(GraphEntity::AT_TURN), m_entity(entity),
        m_dir(to - entity->GetGraph()->GetPos3()), m_activity_sender(entity),
        BaseActivity(drv, cam), m_end_time(0), m_flags(0), m_pos(entity->GetGraph()->GetPos2())
    {
        m_shoot->Init(this, m_entity);
        m_entity->Attach(this, EntityObserver::EV_PREPARE_DEATH);
    }

    ~ShootActivity()
    {
        delete m_shoot;
        DetachObservers();
    }

    bool Run(activity_command cmd)
    {
        if(m_entity) m_entity->Accept(*this);        
        
        if(m_flags & F_CALC_STEP){
            cmd = DrvCalcStep(cmd);
            m_flags &= ~F_CALC_STEP;
        }
        
        if(m_flags & F_WAS_SHOOTING && m_flags & F_CALC_MOVEPNTS){
            cmd = DrvCalcMovepnts(cmd);
            m_flags &= ~F_CALC_MOVEPNTS;
        }
        
        if(cmd == AC_STOP) m_shoot->Stop(this, m_entity);
        
        return DrvRun(m_shoot->Run(this, m_entity) || (m_entity && m_action != GraphEntity::AT_STAY));
    }

private:

    void Visit(HumanEntity* human)
    {
        //еще не время?
        if(m_end_time > Timer::GetSeconds())
            return;
        
        //проиграем последовательность анимаций
        switch(m_action){
        case GraphEntity::AT_TURN:
            {
                if(human->GetGraph()->NeedRotate(m_dir))
                    m_end_time = human->GetGraph()->SetAngle(Dir2Angle(m_dir));
                else
                    m_action = GraphEntity::AT_SHOOT;                
            }
            break;

        case GraphEntity::AT_SHOOT:
            {
                GraphEntity::action_type action =       human->GetEntityContext()->GetShotType() == SHT_AIMSHOT 
                                                    ?   GraphEntity::AT_AIMSHOOT
                                                    :   GraphEntity::AT_SHOOT;

                m_end_time = human->GetGraph()->ChangeAction(action);
                m_action   = GraphEntity::AT_RECOIL;
            }
            break;
            
        case GraphEntity::AT_RECOIL:
            {
               m_flags |= F_CALC_STEP;

               if(m_shoot->IsDone(this, human)){

                    m_flags |= F_CALC_MOVEPNTS;
                    m_action = GraphEntity::AT_STAY;
                    
                    GraphEntity::action_type recoil = human->GetEntityContext()->GetShotType() == SHT_AIMSHOT 
                                                        ?   GraphEntity::AT_AIMRECOIL
                                                        :   GraphEntity::AT_RECOIL;
                    
                    if(m_flags & F_WAS_SHOOTING) m_end_time = human->GetGraph()->ChangeAction(recoil);                                
                    return;
                }

                m_flags |= F_WAS_SHOOTING;
                m_shoot->Shoot(this, human);
            }
            break;

        case GraphEntity::AT_STAY:
            return;
        }
    }

    void Visit(VehicleEntity* vehicle)
    {
        //еще не время?
        if(m_end_time > Timer::GetSeconds())
            return;
        
        switch(m_action){
        case GraphEntity::AT_TURN:
            {
                if(vehicle->GetGraph()->NeedRotate(m_dir))
                    m_end_time = vehicle->GetGraph()->SetAngle(Dir2Angle(m_dir));
                else
                    m_action = GraphEntity::AT_SHOOT;                
            }
            break;
            
        case GraphEntity::AT_SHOOT:
            {
                m_end_time = vehicle->GetGraph()->ChangeAction(GraphEntity::AT_SHOOT);
                m_action   = GraphEntity::AT_RECOIL;
            }
            break;

        case GraphEntity::AT_RECOIL:
            {
                m_flags |= F_CALC_STEP;

                //если нет патронов или неверная линия огня
                if(m_shoot->IsDone(this, vehicle)){
                    m_action = GraphEntity::AT_MOVE;
                    vehicle->GetGraph()->ChangeAction(GraphEntity::AT_MOVE);
                    vehicle->GetGraph()->MoveTo(m_pos);
                    m_flags |= F_CALC_MOVEPNTS;
                    return;
                }

                m_flags |= F_WAS_SHOOTING;
                m_shoot->Shoot(this, vehicle);
                m_end_time = vehicle->GetGraph()->ChangeAction(GraphEntity::AT_RECOIL);                 
            }
            break;
            
        case GraphEntity::AT_MOVE:
            if(GraphUtils::IsInVicinity(vehicle->GetGraph()->GetPos3(), m_pos, 0.2)){
                m_end_time = vehicle->GetGraph()->ChangeAction(GraphEntity::AT_STAY);
                m_action   = GraphEntity::AT_STAY;
            }
            return;
            
        case GraphEntity::AT_STAY:
            return;
        }
    }

    void Visit(TraderEntity* trader)
    {
        throw CASUS("ShootActivity: торговцы не умеют стрелять!");
    }

    void Update(BaseEntity* entity, event_t event, info_t info)
    {
        DetachObservers();
    }

    void DetachObservers()
    {
        if(m_entity){
            m_entity->Detach(this);
            m_entity = 0;
        }
    }

private:

    ipnt2_t  m_pos;
    point3   m_dir;
    
    unsigned m_flags;
    float    m_end_time;

    BaseEntity*   m_entity;   
    ShootManager* m_shoot;

    GraphEntity::action_type m_action;

    ActivityStatusSender m_activity_sender;
};

//======================================================
//
// посадка человека в технику
//
class ShipmentActivity : public BaseActivity{
public:

    ShipmentActivity(HumanEntity* human, VehicleEntity* vehicle, Driver* drv = 0, Camerawork* cam = 0) :
        m_human(human), m_vehicle(vehicle), BaseActivity(drv, cam),
        m_end_time(0), m_activity_sender1(human), m_activity_sender2(vehicle)
    {
        float time1 = human->GetGraph()->ChangeAction(GraphEntity::AT_SHIPMENT);
        float time2 = vehicle->GetGraph()->ChangeAction(GraphEntity::AT_SHIPMENT);

        m_end_time = std::max(time1, time2);
    }

    bool Run(activity_command cmd)
    {
        DrvCalcStep(cmd);
        
        if(m_end_time < Timer::GetSeconds() && !m_human->GetEntityContext()->GetCrew()){

            DrvCalcMovepnts(cmd);
            PathUtils::GetInst()->UnlinkEntity(m_human);
            m_vehicle->GetEntityContext()->IncCrew(m_human);
        }
        
        return DrvRun(m_end_time > Timer::GetSeconds());
    }

private:

    float m_end_time;  
   
    HumanEntity*   m_human;
    VehicleEntity* m_vehicle;

    ActivityStatusSender m_activity_sender1;
    ActivityStatusSender m_activity_sender2;
};

//======================================================

//
// use на объекте
// 
class UseActivity : public BaseActivity{
public:

    enum flag_type{
        F_PLAY_USE    = 1 << 0, 
        F_SWITCH_OBJ  = 1 << 1,
        F_SEND_NOTIFY = 1 << 2,
    };

    UseActivity(HumanEntity* human, const rid_t& rid, unsigned flags, Driver* drv = 0, Camerawork* cam = 0) :
        m_end_time(0), BaseActivity(drv, cam), m_info(human, rid), m_flags(flags) {}

    bool Run(activity_command cmd)
    {
        DrvCalcStep(cmd);
        
        while(m_end_time < Timer::GetSeconds()){
            
            //сначала играем анимацию USE
            if(m_flags & F_PLAY_USE){
                m_flags &= ~F_PLAY_USE;
                m_end_time = m_info.m_actor->GetGraph()->ChangeAction(GraphHuman::AT_USE);
                break;
            }

            if(m_flags & F_SWITCH_OBJ){
                
                //сбросить флаг
                m_flags &= ~F_SWITCH_OBJ;

                //уведомить о USE'е после проигрывания анимации
                m_flags |= F_SEND_NOTIFY;

                m_end_time = DirtyLinks::SwitchObjState(m_info.m_obj4use);                
                break;
            }
            
            if(m_flags & F_SEND_NOTIFY) GameEvMessenger::GetInst()->Notify(GameObserver::EV_USE, &m_info);
            break;
        }

        return DrvRun(m_end_time > Timer::GetSeconds());
    }

private:

    unsigned m_flags; 
    float    m_end_time;

    GameObserver::use_info m_info;
};

//======================================================

//
// Высадка человека из техники
//
class LandingActivity : public BaseActivity{
public:

    LandingActivity(HumanEntity* human, const ipnt2_t& to, Driver* drv = 0, Camerawork* cam = 0) : 
        m_activity_sender1(human), BaseActivity(drv, cam), m_end_time(0),
        m_human(human), m_activity_sender2(human->GetEntityContext()->GetCrew())
    {
        m_human->GetGraph()->SetLoc(to, m_human->GetGraph()->GetAngle());
            
        float   time1 = m_human->GetGraph()->ChangeAction(GraphEntity::AT_LANDING),
                time2 = m_human->GetEntityContext()->GetCrew()->GetGraph()->ChangeAction(GraphEntity::AT_LANDING);
            
        m_end_time = std::max(time1, time2);
    }

    bool Run(activity_command cmd)
    {
        DrvCalcStep(cmd);

        if(m_end_time < Timer::GetSeconds() && m_human->GetEntityContext()->GetCrew()){
            DrvCalcMovepnts(cmd);

            VehicleEntity* vehicle = m_human->GetEntityContext()->GetCrew();

            vehicle->GetEntityContext()->DecCrew(m_human);
            PathUtils::GetInst()->LinkEntity(m_human);
        }
        
        return DrvRun(m_end_time > Timer::GetSeconds());
    }

private:

    float        m_end_time;
    HumanEntity* m_human;

    ActivityStatusSender m_activity_sender1;
    ActivityStatusSender m_activity_sender2;
};

//======================================================

//
// менеджер бросания предметов
//
class ThrowManager {
public:

    virtual ~ThrowManager(){}

    //проиграть полет предмета
    virtual bool Run() = 0; 
    //бросить предмет на уровень
    virtual void Init(HumanEntity* human, const point3& to) = 0;    
};

//
// менеджер для броска гранаты
//
class GrenadeThrowManager : public ThrowManager{
public:

    GrenadeThrowManager(unsigned entity_damager, ShootNotifier* notifier = 0) :
        m_notifier(notifier), m_damager(entity_damager),
        m_grenade(0), m_end_time(0), m_entity(0) {}

    ~GrenadeThrowManager()
    { 
        delete m_grenade;
        delete m_notifier;
    }

    void Init(HumanEntity* human, const point3& to)
    {
        m_to = to;
        m_entity = human;

        m_grenade = static_cast<GrenadeThing*>(human->GetEntityContext()->GetThingInHands(TT_GRENADE));
        human->GetEntityContext()->RemoveThing(m_grenade);

        if(m_grenade->GetInfo()->IsPlasmaGrenade()) human->Notify(EntityObserver::EV_USE_PLASMA_GRENADE);
        
        human->GetGraph()->GetBarrel(NULLVEC, &m_from);
                
        point3 dir = m_to - m_from;
        
        //бросить гранату
        _GrenadeTracer tracer(human->GetGraph()->GetGID(), m_from, Normalize(dir), dir.Length(), 0);
        m_end_time = EffectMgr::GetInst()->MakeGrenadeFlight(tracer.GetTrace(), m_grenade->GetInfo()->GetItemModel(), &m_to);
        
        m_material = tracer.GetMaterial();
        m_eid      = tracer.GetEntity();
        m_obj      = tracer.GetObject();
        
        //разослать уведомления о броске гранаты
        if(m_notifier) m_notifier->SendShoot(m_entity, m_from);
    }

    bool Run()
    {
        if(m_grenade && m_end_time < Timer::GetSeconds()){

            damage_s damages[3];
            
            damages[0] = m_grenade->GetInfo()->GetBDmg();
            damages[1] = m_grenade->GetInfo()->GetADmg();
            
            //проиграть взрыв
            EffectMgr::GetInst()->MakeHit(m_grenade->GetInfo()->GetHitEffect(), m_from, m_to);

            hit_t hit(m_from, m_to, m_grenade->GetInfo()->GetDmgRadius(), damages);

            switch(m_material){
            case ShotTracer::MT_AIR:
            case ShotTracer::MT_NONE:
                Damager::GetInst()->HandleAirHit(m_entity, hit, m_damager);
                break;

            case ShotTracer::MT_SHIELD:
                Damager::GetInst()->HandleShieldHit(m_entity, hit, m_damager);
                break;

            case ShotTracer::MT_WALL:
                Damager::GetInst()->HandleLevelHit(m_entity, hit, m_damager);
                break;
                
            case ShotTracer::MT_OBJECT:
                Damager::GetInst()->HandleObjectHit(m_entity, m_obj, hit, m_damager);
                break;
                
            case ShotTracer::MT_ENTITY:
                Damager::GetInst()->HandleEntityHit(m_entity, EntityPool::GetInst()->Get(m_eid), hit, m_damager);
                break;
            }        

            //разослать уведомления о взрыве
            if(m_material == ShotTracer::MT_ENTITY){
                BaseEntity* victim = EntityPool::GetInst()->Get(m_eid);
                if(m_notifier) m_notifier->SendHit(m_entity, m_to, victim, m_grenade->GetInfo()->GetHitEffect());
            }else{
                if(m_notifier) m_notifier->SendHit(m_entity, m_to, 0, m_grenade->GetInfo()->GetHitEffect());
            }
            
            delete m_grenade;
            m_grenade = 0;
        }

        return m_grenade != 0;
    }

private:

    point3 m_to;
    point3 m_from;

    rid_t  m_obj;
    eid_t  m_eid;

    BaseEntity* m_entity;

    float    m_end_time;
    unsigned m_damager;

    GrenadeThing*  m_grenade;
    ShootNotifier* m_notifier;

    ShotTracer::material_type m_material;
};

//
// менеджер для броска камеры или shield'a
//
class CameraShieldThrowManager  : public ThrowManager{
public:

    CameraShieldThrowManager() : m_thing(0){}
    ~CameraShieldThrowManager() { delete m_thing; }

    void Init(HumanEntity* human, const point3& to)
    {
        m_to = to;
        m_player = human->GetPlayer();

        m_thing = human->GetEntityContext()->GetThingInHands(TT_CAMERA|TT_SHIELD);
        human->GetEntityContext()->RemoveThing(m_thing);        
        
        point3 from, dir;
        
        human->GetGraph()->GetBarrel(NULLVEC, &from);        

        dir = m_to - from;
        
        //бросить гранату
        _GrenadeTracer tracer(human->GetGraph()->GetGID(), from, Normalize(dir), dir.Length(), 0);
        m_end_time = EffectMgr::GetInst()->MakeGrenadeFlight(tracer.GetTrace(), m_thing->GetInfo()->GetItemModel(), &m_to);       
    }

    bool Run()
    {
        if(m_thing && m_end_time < Timer::GetSeconds()){

            if(m_thing->GetInfo()->IsShield())
                ShieldPool::GetInst()->Insert(static_cast<ShieldThing*>(m_thing), m_to);
            else if(m_thing->GetInfo()->IsCamera())
                CameraPool::GetInst()->Insert(m_player, static_cast<CameraThing*>(m_thing), m_to);

            delete m_thing;
            m_thing = 0;
        }
        
        return m_thing != 0;
    }

private:

    point3 m_to;
    float  m_end_time;

    BaseThing*  m_thing;
    player_type m_player;
};

//
// бросок гранаты
// 
class ThrowActivity : public BaseActivity{
public:

    enum {
        F_CALC_STEP     = 1 << 0 ,
        F_CALC_MOVEPNTS = 1 << 1,
    };

    ThrowActivity(HumanEntity* human, const point3& to, float accuracy, ThrowManager* mgr, Driver* drv = 0, Camerawork* cam = 0) :
      BaseActivity(drv, cam), m_end_time(0), m_human(human), m_accuracy(accuracy),
      m_flags(0), m_throw(mgr), m_to(to), m_action(GraphEntity::AT_TURN), m_activity_sender(human) {}

    ~ThrowActivity()
    {
        delete m_throw;
    }

    bool Run(activity_command cmd)
    {
        if(m_flags & F_CALC_STEP){
            cmd = DrvCalcStep(cmd);
            m_flags &= ~F_CALC_STEP;
        }

        if(m_flags & F_CALC_MOVEPNTS){
            cmd = DrvCalcMovepnts(cmd);
            m_flags &= ~F_CALC_MOVEPNTS;
        }

        PlayThrow();

        return DrvRun(m_throw->Run() || (m_action != GraphEntity::AT_STAY));
    }

private:

    //пориграть бросок гранаты
    void PlayThrow()
    {
        if(m_end_time > Timer::GetSeconds())       
            return;

        switch(m_action){
        case GraphEntity::AT_TURN:
            {
                //развернуть человека в сторону броска
                point3 dir = m_to - m_human->GetGraph()->GetPos3();
                m_end_time = m_human->GetGraph()->SetAngle(Dir2Angle(dir));
                m_action   = GraphEntity::AT_SHOOT;

                m_flags |= F_CALC_STEP;
            }
            break;

        case GraphEntity::AT_SHOOT:
            {
                m_end_time = m_human->GetGraph()->ChangeAction(GraphEntity::AT_SHOOT);
                m_action   = GraphEntity::AT_RECOIL;

                m_flags |= F_CALC_STEP;
            }
            break;

        case GraphEntity::AT_RECOIL:
            {
                m_flags |= F_CALC_STEP;
                m_flags |= F_CALC_MOVEPNTS;

                m_throw->Init(m_human, MakeCircleScatter(m_to, throw_scatter_radius * (1 - m_accuracy)));

                m_end_time = m_human->GetGraph()->ChangeAction(GraphEntity::AT_RECOIL);
                m_action   = GraphEntity::AT_STAY;
            }
            break;

        case GraphEntity::AT_STAY:
            return;
        }
    }

private:

    unsigned m_flags;

    point3 m_to;

    float m_accuracy;
    float m_end_time;

    HumanEntity*  m_human;
    ThrowManager* m_throw;

    GraphEntity::action_type m_action;
    ActivityStatusSender m_activity_sender;   
};

//======================================================

} // namespace

//======================================================

Activity* ActivityFactory::CreateMove(BaseEntity* entity, const pnt_vec_t& path, unsigned flags)
{
    Driver* drv = 0;

    if(flags & CT_STAY_CHANGE) drv = new StanceChangeDrv(entity, drv);
    if(flags & CT_PLAY_REACTION) drv = new ReactionDrv(entity, drv);    
    if(flags & CT_PLAY_FIRST_PHRASE) drv = new FirstPhraseDrv(entity,drv);

    if(flags & CT_ENEMY_STOP){

        unsigned drv_flags = 0;
       
        if(flags & CT_PRINT_MSGS) drv_flags |= OpponentViewDrv::F_PRINT_MSGS;

        drv = new OpponentViewDrv(entity, drv_flags, drv);
    }
 
    unsigned pass_flags = 0;

    if(flags & CT_BUS_PASS)   pass_flags |= PassDrv::F_STOP_BEFORE_HUMAN;
    if(flags & CT_USUAL_PASS) pass_flags |= PassDrv::F_STOP_BEFORE_ALLY|PassDrv::F_CALC_DANGER_HEXES;

    drv = new PassDrv(entity, path, pass_flags, drv);

    if(flags & CT_PATH_SHOW)  drv = new PathShowDrv(path,drv);
    
    if(flags & CT_UPDATE_MPS){

        int hexcost = 0;
        
        if(HumanEntity* human = entity->Cast2Human())
            hexcost = human->GetEntityContext()->GetHexCost();
        else if(VehicleEntity* vehicle = entity->Cast2Vehicle())
            hexcost = vehicle->GetEntityContext()->GetHexCost();
        
        drv = new UpdateMPDrv(entity, hexcost, drv);
    }

    if(flags & CT_UPDATE_VIS){

        unsigned flags = 0;
        
        flags |= UpdateVisMapDrv::F_UPDATE_ALWAYS;
        flags |= UpdateVisMapDrv::F_UPDATE_MARKER;
        flags |= UpdateVisMapDrv::F_UPDATE_SPECTATOR;
        
        drv = new UpdateVisMapDrv(entity, flags, drv);
    }

    if(flags & CT_MPS_LIMIT)  drv = new MPLimitDrv(entity, drv);
    
    if(flags & CT_STEP_LIMIT){

        unsigned drv_flags = 0;
        
        if(flags & CT_PRINT_MSGS) drv_flags |= StepLimitDrv::F_PRINT_MSGS;

        drv = new StepLimitDrv(entity, drv_flags, drv);
    }

    if(HumanEntity* human = entity->Cast2Human()){
        if(flags & CT_ACCURACY_DECREASE && human->GetGraph()->IsRunMove())
            drv = new DecAccuracyDrv(human, drv);
    }

    return new MoveActivity(entity, path, drv, Cameraman::GetInst()->FilmMove(entity));
}

Activity* ActivityFactory::CreateRotate(BaseEntity* entity, float angle, unsigned flags)
{
    Driver* drv = 0;

    if(flags & CT_ENEMY_STOP){

        unsigned drv_flags = 0;
       
        if(flags & CT_PRINT_MSGS) drv_flags |= OpponentViewDrv::F_PRINT_MSGS;

        drv = new OpponentViewDrv(entity, drv_flags, drv);
    }

    if(flags & CT_UPDATE_VIS){

        unsigned flags = 0;

        flags |= UpdateVisMapDrv::F_UPDATE_ALWAYS;
        flags |= UpdateVisMapDrv::F_UPDATE_SPECTATOR;

        drv = new UpdateVisMapDrv(entity, flags, drv);
    }

    if(flags & CT_MPS_LIMIT)   drv = new MPLimitDrv(entity, drv);
    if(flags & CT_UPDATE_MPS)  drv = new UpdateMPDrv(entity, MPS_FOR_ROTATE, drv);
    if(flags & CT_STAY_CHANGE) drv = new StanceChangeDrv(entity, drv);
    
    return new RotateActivity(entity, angle, drv, Cameraman::GetInst()->FilmRotate(entity));
}

Activity* ActivityFactory::CreateShoot(BaseEntity* entity, const point3& to, float accuracy, unsigned flags, const shoot_info_s& info)
{
    Driver* drv = 0;

    if(flags & CT_UPDATE_MPS){
       
        if(HumanEntity* human = entity->Cast2Human()){
            WeaponThing* weapon = static_cast<WeaponThing*>(human->GetEntityContext()->GetThingInHands(TT_WEAPON));
            drv = new UpdateMPDrv(entity, weapon->GetMovepnts(human->GetEntityContext()->GetShotType()), drv);
        }

        if(VehicleEntity* vehicle = entity->Cast2Vehicle()){
            drv = new UpdateMPDrv(vehicle, vehicle->GetInfo()->GetMp4Shot(), drv);
        }
    }
    
    if(flags & CT_UPDATE_VIS){

        unsigned flags = 0;

        flags |= UpdateVisMapDrv::F_UPDATE_ONCE;
        flags |= UpdateVisMapDrv::F_UPDATE_SPECTATOR;

        drv = new UpdateVisMapDrv(entity, flags, drv);
    }
    if(flags & CT_STAY_CHANGE) drv = new StanceChangeDrv(entity, drv);

    unsigned entity_damager = 0;

    entity_damager |= EntityDamager::FT_MAKE_EFFECTS;
    entity_damager |= EntityDamager::FT_SEND_GAME_EVENTS;
    entity_damager |= EntityDamager::FT_PLAY_ENTITY_HURT;

    if(flags & CT_HURT_SHOOT){
        entity_damager |= EntityDamager::FT_MAKE_DAMAGES;
        entity_damager |= EntityDamager::FT_PLAY_ENTITY_DEATH;
    }

    if(flags & CT_PRINT_DMGS) entity_damager |= EntityDamager::FT_PRINT_MESSAGES;
    
    LOFCheck* lof = 0;

    if(flags & CT_TRACE_LOF) lof = new TracingLOFCheck(info.m_entity, info.m_object, (flags & CT_PRINT_MSGS) != 0);

    BaseEntity* victim = info.m_entity ? EntityPool::GetInst()->Get(info.m_entity) : 0;
    Camerawork* cam = Cameraman::GetInst()->FilmShoot(entity, victim);

    TargetManager* target_mgr = new TargetManagerImp(accuracy, to, info.m_object, info.m_entity);
    ShootNotifier* notifier   = flags & CT_SEND_EVENTS ? new ShootNotifierImp(victim, info.m_object) : 0;
    ShootManager*  shoot_mgr  = new ShootManagerImp(target_mgr, entity_damager, notifier, lof);

    return new ShootActivity(entity, shoot_mgr, to, drv, cam);
}

Activity* ActivityFactory::CreateShipment(HumanEntity* human, VehicleEntity* vehicle, unsigned flags)
{
    Driver* drv = 0;

    if(flags & CT_STAY_CHANGE){
        drv = new StanceChangeDrv(human, drv);
        drv = new StanceChangeDrv(vehicle, drv);
    }

    if(flags & CT_UPDATE_MPS) drv = new UpdateMPDrv(human, human->GetEntityContext()->GetTraits()->GetMovepnts(), drv);

    return new ShipmentActivity(human, vehicle, drv, Cameraman::GetInst()->FilmShipment(human, vehicle));
}

Activity* ActivityFactory::CreateLanding(HumanEntity* human,const ipnt2_t& to, unsigned flags)
{
    Driver* drv = 0;

    if(flags & CT_UPDATE_MPS) drv = new UpdateMPDrv(human, MPS_FOR_LANDING, drv);

    if(flags & CT_STAY_CHANGE){
        drv = new StanceChangeDrv(human, drv);
        drv = new StanceChangeDrv(human->GetEntityContext()->GetCrew(), drv);
    }

    return new LandingActivity(human, to, drv, Cameraman::GetInst()->FilmLanding(human));
}

Activity* ActivityFactory::CreateUse(BaseEntity* entity, const rid_t& rid, unsigned flags)
{
    Driver* drv = 0;

    if(flags & CT_UPDATE_VIS){

        unsigned flags = 0;

        flags |= UpdateVisMapDrv::F_UPDATE_ALWAYS;        
        flags |= UpdateVisMapDrv::F_UPDATE_SPECTATOR;

        drv = new UpdateVisMapDrv(entity, flags, drv);
    }

    if(flags & CT_STAY_CHANGE) drv = new StanceChangeDrv(entity, drv);

    unsigned use_flags = 0;

    if(flags & CT_SWITCH_OBJECT) use_flags |= UseActivity::F_SWITCH_OBJ;
    if(!DirtyLinks::IsElevatorObj(rid)) use_flags |= UseActivity::F_PLAY_USE;

    return new UseActivity(entity->Cast2Human(), rid, use_flags, drv, Cameraman::GetInst()->FilmUse(entity, rid));
}

Activity* ActivityFactory::CreateThrow(HumanEntity* human, const point3& to, unsigned flags , const shoot_info_s& info, float accuracy)
{
    Driver*    drv = 0;
    BaseThing* thing = human->GetEntityContext()->GetThingInHands(TT_THROWABLE);

    if(flags & CT_UPDATE_MPS) drv = new UpdateMPDrv(human, AIUtils::CalcMps2Act(thing), drv);
       
    if(flags & CT_UPDATE_VIS){

        unsigned flags = 0;

        flags |= UpdateVisMapDrv::F_UPDATE_ONCE;
        flags |= UpdateVisMapDrv::F_UPDATE_SPECTATOR;

        drv = new UpdateVisMapDrv(human, flags, drv);
    }

    if(flags & CT_STAY_CHANGE) drv = new StanceChangeDrv(human, drv);

    ThrowManager* throw_mgr = 0;

    if(thing->GetInfo()->IsGrenade()){

        unsigned entity_damager = 0;
        
        entity_damager |= EntityDamager::FT_MAKE_EFFECTS;
        entity_damager |= EntityDamager::FT_SEND_GAME_EVENTS;
        entity_damager |= EntityDamager::FT_PLAY_ENTITY_HURT;
        
        if(flags & CT_HURT_SHOOT){
            entity_damager |= EntityDamager::FT_MAKE_DAMAGES;
            entity_damager |= EntityDamager::FT_PLAY_ENTITY_DEATH;
        }

        if(flags & CT_PRINT_DMGS) entity_damager |= EntityDamager::FT_PRINT_MESSAGES;

        ShootNotifier* notifier = 0;

        if(flags & CT_SEND_EVENTS){
            BaseEntity* victim = info.m_entity ? EntityPool::GetInst()->Get(info.m_entity) : 0;
            notifier = new ShootNotifierImp(victim, info.m_object);
        }

        throw_mgr = new GrenadeThrowManager(entity_damager, notifier);

    }else if(thing->GetInfo()->GetType() & (TT_CAMERA|TT_SHIELD)){

        throw_mgr = new CameraShieldThrowManager();
    }

    Camerawork* cam = Cameraman::GetInst()->FilmThrow(human, info.m_entity ? EntityPool::GetInst()->Get(info.m_entity) : 0);

    return new ThrowActivity(human, to, accuracy, throw_mgr, drv, cam);
}

ActivityFactory* ActivityFactory::GetInst()
{
    static ActivityFactory imp;
    return &imp;
}

//===========================================================================

namespace{

    //
    // фабрика скриптовых сцен
    //
    class ScriptSceneFactory{
    public:

        virtual ~ScriptSceneFactory(){}

        DCTOR_ABS_DEF(ScriptSceneFactory)

        //создать скриптовую сцену
        virtual Activity* Create() = 0;
        //сохранить/загрузить данные
        virtual void MakeSaveLoad(SavSlot& slot) = 0;
    };

    //
    // фабрика скриптовых сцен на условие победы
    //
    class KillEntsAndExitGame : public  ScriptSceneFactory,
                                private GameObserver,
                                private SpawnObserver
                                
    {
    public:

        class statistics{
        public:

            void MakeSaveLoad(SavSlot& slot)
            {
                if(slot.IsSaving())
                    slot << m_needed << m_killed;
                else
                    slot >> m_needed >> m_killed;
            }

            void IncKilled() { m_killed++; }

            void Init() { m_killed = 0; }

            void SetNeeded(int val) { m_needed = val; }

            bool IsDone() const { return m_killed >= m_needed; }

            statistics(int needed = 0) : m_needed(needed), m_killed(0){}

        private:

            int m_needed;
            int m_killed;
        };

        typedef std::map<rid_t, statistics> ent_map_t;

        KillEntsAndExitGame(const rid_t& bik = rid_t(), const ent_map_t& ents = ent_map_t()):
            m_activity(0), m_bik(bik), m_ents(ents)
        {
            Spawner::GetInst()->Attach(this, ET_ENTRY_LEVEL);
            GameEvMessenger::GetInst()->Attach(this, EV_KILL);

            InitEntsMap();
        }

        ~KillEntsAndExitGame()
        {
            delete m_activity;

            Spawner::GetInst()->Detach(this);
            GameEvMessenger::GetInst()->Detach(this);
        }

        DCTOR_DEF(KillEntsAndExitGame)
        
        Activity* Create()
        { 
            Activity* ret = m_activity;
            if(m_activity) m_activity = 0;
            return ret;
        }

        void MakeSaveLoad(SavSlot& slot)
        {
            if(slot.IsSaving()){

                slot << m_bik;
                slot << m_ents.size();

                ent_map_t::iterator itor = m_ents.begin();
                while(itor != m_ents.end()){

                    slot << itor->first;
                    itor->second.MakeSaveLoad(slot);

                    ++itor;
                }

            }else{

                slot >> m_bik;

                rid_t  rid;
                size_t size;
                
                slot >> size;

                while(size--){
                     slot >> rid;
                     m_ents[rid].MakeSaveLoad(slot);
                }
            }            
        }

    private:

        void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info)
        {
            kill_info* ptr = static_cast<kill_info*>(info);

            if(m_activity == 0 && ptr->IsKill() && ptr->m_victim){

                //ведем счет убитых сущетсв
                ent_map_t::iterator itor = m_ents.find(ptr->m_victim->GetInfo()->GetRID());
                if(itor != m_ents.end()) itor->second.IncKilled();

                //если убили всех кого надо
                if(AllKilled()){
                    InitEntsMap();
                    m_activity = new ShowBikAndExitGame(m_bik);
                }
            }
        }

        void Update(SpawnObserver::subject_t subj, SpawnObserver::event_t event, SpawnObserver::info_t info)
        {
            entry_info_s* ptr = static_cast<entry_info_s*>(info);

            //сбросить статистику в начале игры
            if(ptr->m_entry.empty())
				InitEntsMap();
        }

        bool AllKilled()
        {
            ent_map_t::iterator itor = m_ents.begin();
            while(itor != m_ents.end()){

                if(!itor->second.IsDone())
                    return false;

                ++itor;
            }

            return true;
        }

        void InitEntsMap()
        {
            ent_map_t::iterator itor = m_ents.begin();
            while(itor != m_ents.end()){
                itor->second.Init();
				Spawner *sp = Spawner::GetInst();
                ++itor;
            }
        }

        class ShowBikAndExitGame : public Activity{
        public:

            ShowBikAndExitGame(const rid_t& bik) : 
              m_bik(bik), m_end_time(0) {}

            bool Run(activity_command cmd)
            { 
                if(m_end_time == 0) m_end_time = Timer::GetSeconds() + 0.5f;
                    
                if(m_end_time < Timer::GetSeconds()){

                    {
                        SndUtils::EmptySoundSession empty_sound;
                        DirtyLinks::LoadLevel(std::string());
                        Forms::GetInst()->ShowBink(m_bik);
                    }

                    Forms::GetInst()->ShowMainMenuForm();
                }

                return true;                
            }

        private:

            rid_t m_bik;
            float m_end_time;
        };

    private:

        rid_t m_bik;

        ent_map_t  m_ents;
        Activity*  m_activity;
    };

    DCTOR_IMP(KillEntsAndExitGame)

    //
    // фабрика скриптовых сцен по убийству героя
    //
    class HeroKillGameExit : public  ScriptSceneFactory,
                             private GameObserver
    {
    public:

        HeroKillGameExit(const rid_t& hero = std::string(), player_type player = PT_NONE) :
            m_hero(hero), m_player(player), m_activity(0)
        { GameEvMessenger::GetInst()->Attach(this, EV_KILL); }

        DCTOR_DEF(HeroKillGameExit)

        ~HeroKillGameExit()
        {
            delete m_activity;
            GameEvMessenger::GetInst()->Detach(this);   
        }

        Activity* Create()
        { 
            Activity* ret = m_activity;
            m_activity = 0;
            return ret;
        }

        void MakeSaveLoad(SavSlot& slot)
        {  
            if(slot.IsSaving()){

                slot << m_hero;
                slot << static_cast<int>(m_player);

            }else{

                slot >> m_hero;

                int tmp; slot >> tmp;
                m_player = static_cast<player_type>(tmp);
            }
        }

    private:

        void Update(subject_t subj, event_t event, info_t info)
        {
            kill_info* ptr = static_cast<kill_info*>(info);

            if(     m_activity == 0
                &&  ptr->IsKill()
                &&  ptr->m_victim
                &&  ptr->m_victim->GetPlayer() == m_player
                &&  ptr->m_victim->GetInfo()->GetRID() == m_hero){
                m_activity = new ShowHeroDeathAndExitGame(ptr->m_victim->GetGraph()->GetPos3());            
            }
        }

        class ShowHeroDeathAndExitGame : public Activity{
        public:

            ShowHeroDeathAndExitGame(const point3& pnt) : 
                m_pnt(pnt), m_end_time(0) {}

            bool Run(activity_command cmd)
            {                
                if(m_end_time == 0){
                    
                    m_end_time = Timer::GetSeconds() + 5.0f;
                    
                    DirtyLinks::MoveCamera(m_pnt, 0.3f);
                    DirtyLinks::Print(DirtyLinks::GetStrRes("aiu_hacker_death"));
                    DirtyLinks::Print(DirtyLinks::GetStrRes("aiu_game_over"));
                }        

                if(m_end_time < Timer::GetSeconds() || IsAnyKeyPressed()){
                    Forms::GetInst()->ShowOptionsDialog(Forms::ODM_LOADMENU);
                    return false;
                }
 
                return true;
            }

        private:

            bool IsAnyKeyPressed()
            {
                for(int k = 0; k < 256; k++){
                    if(Input::KeyBack(k)) return true;
                }
                
                return false;
            }

        private:

            point3 m_pnt;
            float m_end_time;
        };

    private:

        rid_t       m_hero;
        Activity*   m_activity;
        player_type m_player;
    };

    DCTOR_IMP(HeroKillGameExit)

    //
    // фабрика обычных скриптовых сцен
    //
    class UsualScriptSceneFactory : public  ScriptSceneFactory,
                                    private GameObserver,
                                    private SpawnObserver
    {
    public:

        UsualScriptSceneFactory()
        {
            Spawner::GetInst()->Attach(this, ET_ENTRY_LEVEL);

            GameEvMessenger::GetInst()->Attach(this, EV_USE);
            GameEvMessenger::GetInst()->Attach(this, EV_KILL);
            GameEvMessenger::GetInst()->Attach(this, EV_QUEST_FINISHED);
            GameEvMessenger::GetInst()->Attach(this, EV_INIT_LEVEL_EXIT);            
        }

        DCTOR_DEF(UsualScriptSceneFactory)

        ~UsualScriptSceneFactory()
        {
            ClearScriptQueue();

            Spawner::GetInst()->Detach(this);   
            GameEvMessenger::GetInst()->Detach(this);
        }

        Activity* Create()
        {
            if(m_scenes.size()){
                ScriptScene* ss = m_scenes.back();
                m_scenes.pop_back();
                return new SSceneActivity(ss);
            }
            
            return 0;
        }

        void MakeSaveLoad(SavSlot& slot)
        { ScriptSceneManager::Instance()->MakeSaveLoad(slot); }

    private:

        void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info)
        {
            switch(event){
            case EV_USE:
                {
                     use_info* ptr = static_cast<use_info*>(info);

                     ScriptSceneManager::Params params;
                     
                     if(ptr->m_obj4use.size())
                         params.m_Object = ptr->m_obj4use;
                     else
                         params.m_Object = ptr->m_ent4use->GetInfo()->GetRID();

                     params.m_Level  = DirtyLinks::GetLevelSysName();
                     params.m_Phase  = Spawner::GetInst()->GetPhase();
                     
                     ScriptScene* ss = ScriptSceneManager::Instance()->CreateScene(ScriptSceneManager::E_USE, params);
                     if(ss) m_scenes.push_back(ss);
                }
                break;

            case EV_KILL:
                {
                    kill_info* ptr = static_cast<kill_info*>(info);

                    if(ptr->m_event == KEV_KILL && ptr->m_victim){
                        
                        ScriptSceneManager::Params params;
                        
                        params.m_Level  = DirtyLinks::GetLevelSysName();
                        params.m_Phase  = Spawner::GetInst()->GetPhase();
                        params.m_Object = ptr->m_victim->GetInfo()->GetRID();
                        
                        ScriptScene* ss = ScriptSceneManager::Instance()->CreateScene(ScriptSceneManager::E_KILL, params);
                        if(ss) m_scenes.push_back(ss);
                    }   
                }
                break;

            case EV_INIT_LEVEL_EXIT:
                {
                    exit_level_info* ptr = static_cast<exit_level_info*>(info);

                    ScriptSceneManager::Params params;
                    
                    params.m_Level   = DirtyLinks::GetLevelSysName();
                    params.m_ToLevel = ptr->m_2level;
                    params.m_Phase   = Spawner::GetInst()->GetPhase();
                    
                    ScriptScene* ss = ScriptSceneManager::Instance()->CreateScene(ScriptSceneManager::E_EXITTOLEVEL, params);
                    if(ss) m_scenes.push_back(ss);
                }
                break;

            case EV_QUEST_FINISHED:
                {
                    quest_info* ptr = static_cast<quest_info*>(info);

                    ScriptSceneManager::Params params;
                    
                    params.m_Quest = ptr->m_quest;
                    params.m_Level = DirtyLinks::GetLevelSysName();
                    params.m_Phase = Spawner::GetInst()->GetPhase();
                    
                    ScriptScene* ss = ScriptSceneManager::Instance()->CreateScene(ScriptSceneManager::E_QUESTFINISHED, params);
                    if(ss) m_scenes.push_back(ss);
                }
                break;
            }
        }

        void Update(SpawnObserver::subject_t subj, SpawnObserver::event_t event, SpawnObserver::info_t info)
        {
            ClearScriptQueue();

            entry_info_s* ptr = static_cast<entry_info_s*>(info);
            
            //если новая игра или новый эпизод
            if(ptr->m_entry.empty() || ptr->m_fnew_episode){
                ScriptSceneManager::NewGameParams params;

                params.m_Episode = Spawner::GetInst()->GetEpisode();
                ScriptSceneManager::Instance()->OnBegNewGame(params);
            }

            //обработка прихода на новй уровень
            ScriptSceneManager::Params params;
            
            params.m_Level = DirtyLinks::GetLevelSysName();
            params.m_Phase = Spawner::GetInst()->GetPhase();
            
            ScriptScene* ss = ScriptSceneManager::Instance()->CreateScene(ScriptSceneManager::E_STARTLEVEL, params);
            if(ss) m_scenes.push_back(ss);
        }

        void ClearScriptQueue()
        {
            while(m_scenes.size()){
                delete m_scenes.back();
                m_scenes.pop_back();
            }
        }

        //обертка для проигрывания скриптовой сцены
        class SSceneActivity : public Activity{
        public:
            
            SSceneActivity(ScriptScene* scene) : m_scene(scene) {}        
            ~SSceneActivity(){ delete m_scene; }
            
            //проигрывание действия нотация как while(act.Run())
            bool Run(activity_command cmd)
            {
				if( cmd!=AC_TICK && !m_scene->IsValid() )	   //Grom
					m_scene->Run(ScriptScene::TICK);
                return m_scene->Run(cmd == AC_TICK ? ScriptScene::TICK : ScriptScene::SKIP);
            }
            
        private:
            
            ScriptScene* m_scene;
        };

    private:

        typedef std::list<ScriptScene*> scene_queue_t;
        scene_queue_t m_scenes;
    };

    DCTOR_IMP(UsualScriptSceneFactory)

    //
    // класс для порождения и проигрывания скриптовых сцен
    //
    class SSceneMgrImp : public SSceneMgr{
    public:
                
        //породить скриптовую сцену для проигрывания
        Activity* CreateScriptScene()
        {
            for(size_t k = 0; k < m_factories.size(); k++){
                if(Activity* act = m_factories[k]->Create())
                    return act;
            }

            return 0;
        }

        //сохранить скриптовыве сцены
        void MakeSaveLoad(SavSlot& slot)
        { 
            if(slot.GetStore()->GetVersion() > 2)
                MakeSaveLoad3(slot);
            else
                MakeSaveLoad0(slot);
        }

        void Init()
        {
            m_factories.push_back(new UsualScriptSceneFactory());
            m_factories.push_back(new HeroKillGameExit("player", PT_PLAYER));

            KillEntsAndExitGame::ent_map_t ents;
			ents["gorn_4"].SetNeeded(2);
            ents["gorn_5"].SetNeeded(1);

            m_factories.push_back(new KillEntsAndExitGame("outro.bik", ents));
        }

        void Shut()
        {
				STACK_GUARD("SSceneMgr::Shut");
            for(size_t k = 0; k < m_factories.size(); delete m_factories[k++]);
            m_factories.clear();
        }

        bool IsScriptSceneFinished(const rid_t& rid) const
        {
            return ScriptSceneManager::Instance()->IsScriptSceneFinished(Spawner::GetInst()->GetEpisode(), rid.c_str());
        }

    private:

        void MakeSaveLoad0(SavSlot& slot)
        { 
            Shut();
            Init();

            ScriptSceneManager::Instance()->MakeSaveLoad(slot);
        }

        void MakeSaveLoad3(SavSlot& slot)
        {
            if(slot.IsSaving()){
                
                slot << m_factories.size();

                for(size_t k = 0; k < m_factories.size(); k++){
                    DynUtils::SaveObj(slot, m_factories[k]);
                    m_factories[k]->MakeSaveLoad(slot);
                }

            }else{

                Shut();

                size_t size;
                slot >> size;

                m_factories.resize(size);

                for(size_t k = 0; k < m_factories.size(); k++){
                    DynUtils::LoadObj(slot, m_factories[k]);
                    m_factories[k]->MakeSaveLoad(slot);
                }
            }
        }

    private:

        typedef std::vector<ScriptSceneFactory*> factory_vec_t;
        factory_vec_t m_factories;
    };
}

SSceneMgr* SSceneMgr::GetInst()
{
    static SSceneMgrImp imp;
    return &imp;
}

//===========================================================================

void Activity::Detach(ActivityObserver* observer)
{ 
    m_observers.Detach(observer);
}

void Activity::Attach(ActivityObserver* observer, ActivityObserver::event_t type)
{ 
    m_observers.Attach(observer, type);
}

void Activity::Notify(ActivityObserver::event_t event, ActivityObserver::info_t info)
{ 
    m_observers.Notify(this, event, info);
}