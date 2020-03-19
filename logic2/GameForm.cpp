//
// игровое меню iteration 2
//
#pragma warning(disable:4786)

#include "logicdefs.h"
#include "assert.h"

#include "../interface/screens.h"
#include "../interface/mousecursor.h"

#include "Thing.h"
#include "sscene.h"
#include "Entity.h"
#include "bureau.h"
#include "AIUtils.h"
#include "HexGrid.h"
#include "Graphic.h"
#include "strategy.h"
#include "sndutils.h"
#include "HexUtils.h"
#include "Activity.h"
#include "Cameraman.h"
#include "PathUtils.h"
#include "TraceUtils.h"
#include "DirtyLinks.h"
#include "GameFormImp.h"
#include "QuestServer.h"
#include "thingfactory.h"
#include "phrasemanager.h"

const float ground_scroll_radius       = 2.0f;
const float default_hero_scroll_radius = 5.0f;

const int   HumanIcons::m_dead_lvl  = 10;
const float HumanIcons::m_wound_lvl = 0.5f;

//==============================================================

void DirtyLinks::MakeQuickSaveLoad(Storage::storage_mode mode)
{
    Screens::Instance()->Game()->MakeSpecialSaveLoad(GameScreen::SST_QUICKSAVE, 
                mode == Storage::SM_LOAD ? GameScreen::SSM_LOAD : GameScreen::SSM_SAVE);
}

//==============================================================

BaseForm* FormFactory::CreateGameForm()
{
    return new GameFormImp();
}

//==============================================================

void DirtyLinks::Print(const std::string& str, message_type type)
{
    if(!Screens::Instance()->Game()->MsgWindow()->IsPulled())
        Screens::Instance()->Game()->MsgWindow()->IncMsgWindow();

    Screens::Instance()->Game()->MsgWindow()->AddText(str.c_str());

    if(type==MT_DENIAL) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_LOGIC_DENIED));
}

//==============================================================

namespace{

    //получить указатель на реализацию меню
    GameFormImp* GetGameFormImp()
    {
        return static_cast<GameFormImp*>(Forms::GetInst()->GetGameForm());
    }

    //переключить крыши на уровне
    void SwitchRoofs()
    {
        GameScreen::CHECKBOX_STATE state = Screens::Instance()->Game()->GetCBState(GameScreen::CB_ROOF);
        state = (state == GameScreen::CBS_ON) ? GameScreen::CBS_OFF : GameScreen::CBS_ON;
        Screens::Instance()->Game()->SetCBState(GameScreen::CB_ROOF, state);
        DirtyLinks::EnableRoofs(state == GameScreen::CBS_OFF);
    }

    bool IsEnemy4Player(BaseEntity* entity)
    {
        //RelationType relation = EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(entity);
        return EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(entity) == RT_ENEMY;
    }

    void SetShotCBs(GameScreen::CHECKBOX active_box)
    {
        Screens::Instance()->Game()->SetCBState(GameScreen::CB_AIMSHOT, active_box == GameScreen::CB_AIMSHOT  ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
        Screens::Instance()->Game()->SetCBState(GameScreen::CB_AUTOSHOT, active_box == GameScreen::CB_AUTOSHOT ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
        Screens::Instance()->Game()->SetCBState(GameScreen::CB_SNAPSHOT, active_box == GameScreen::CB_SNAPSHOT ? GameScreen::CBS_ON : GameScreen::CBS_OFF);   
    }

    void SetRunSitCBs(HumanEntity* human)
    {
        Screens::Instance()->Game()->SetCBState(GameScreen::CB_RUN, human->GetGraph()->IsRunMove() ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
        Screens::Instance()->Game()->SetCBState(GameScreen::CB_SIT, human->GetGraph()->IsSitPose() ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
    }

    bool IsLeftMouseButtonClick()
    {
        return     Input::MouseState().LButtonFront 
                && !Input::MouseState().RButtonState  
                && !Input::MouseState().MButtonState;
    }

    bool IsLeftMouseButtonDoubleClick(float click_time)
    {
        return      Input::MouseState().LDblClick
                &&  click_time + Input::Mouse()->DoubleClickTime() > Timer::GetSeconds();
    }

    template<class Icons, class Entity>
    Entity* FindNextEntity(Icons* icons, Entity* current)
    {
        int icon = current ? icons->Entity2Icon(current) : -1;

        //найдем следующее существо
        for(icon++; icon <  MAX_TEAMMATES; icon++){
            if(Entity* next = icons->Icon2Entity(icon))
                return next;
        }

        return 0;
    }

    bool CanChangeHumanPose(HumanEntity* human)
    {
        AIUtils::reason_type reason;

        if(AIUtils::CanChangePose(human, &reason))
            return true;
        
        DirtyLinks::Print( AIUtils::Reason2Str(reason), DirtyLinks::MT_DENIAL);
        return false;
    }

    bool IsChecked(GameScreen::CHECKBOX check_box)
    {
        return Screens::Instance()->Game()->GetCBState(check_box) == GameScreen::CBS_ON;
    }

    int GetPressedIndex()
    {
        if(Input::KeyBack(DIK_0)) return 9;
        if(Input::KeyBack(DIK_1)) return 0;
        if(Input::KeyBack(DIK_2)) return 1;
        if(Input::KeyBack(DIK_3)) return 2;
        if(Input::KeyBack(DIK_4)) return 3;
        if(Input::KeyBack(DIK_5)) return 4;
        if(Input::KeyBack(DIK_6)) return 5;
        if(Input::KeyBack(DIK_7)) return 6;
        if(Input::KeyBack(DIK_8)) return 7;
        if(Input::KeyBack(DIK_9)) return 8;

        return -1;
    }

    bool IsAnyNewJournalInfo()
    {
        return false;
    }

    void SetNextFastThing(HumanEntity* human, FastAccessStrategy::iter_type type)
    {        
        FastAccessStrategy* fast_access = human->GetEntityContext()->GetFastAccessStrategy();
        std::auto_ptr<FastAccessStrategy::Iterator> itor(fast_access->CreateIterator(type));
        
        //найдем следущий предмет
        itor->First(fast_access->GetThing());
        
        //если такого нет то уст. первый в посл-ти
        if(itor->IsDone()) itor->First(0);
        
        //установим предмет
        fast_access->SetThing(itor->Get());
    }
}

//==============================================================

namespace{

    class PanicForm : public BaseForm{
    public:
                
        void Show()
        {
            Screens::Instance()->Game()->ShowControls(false);
            Screens::Instance()->Activate(Screens::SID_PANIC);
        }
        
        void Init(const ini_s& ini){}
        void HandleInput(state_type* st){}
        void MakeSaveLoad(SavSlot& slot){}
    };
}

BaseForm* FormFactory::CreatePanicForm()
{
    return new PanicForm();
}
    
//==============================================================

namespace {

    class Switch2Aimshot : public EntityVisitor{
    public:
        
        void Visit(HumanEntity* human)
        {   
            HumanContext* context = human->GetEntityContext();

            if(!context->GetThingInHands(TT_WEAPON))
                return;
            
            context->SetShotType(SHT_AIMSHOT);
            SetShotCBs(GameScreen::CB_AIMSHOT);
        }
    };
}
    
//==============================================================

namespace {
    
    class Switch2Autoshot : public EntityVisitor{
    public:
        
        void Visit(HumanEntity* human)
        {
            HumanContext* context = human->GetEntityContext();
            
            WeaponThing* thing = static_cast<WeaponThing*>(context->GetThingInHands(TT_WEAPON));

            if(thing == 0 || !thing->GetInfo()->IsAuto())
                return;
                        
            context->SetShotType(SHT_AUTOSHOT);
            SetShotCBs(GameScreen::CB_AUTOSHOT);
        }        
    };
}
    
//==============================================================
    
namespace {

    class Switch2Snapshot : public EntityVisitor{
    public:
        
        void Visit(HumanEntity* human)
        {
            HumanContext* context = human->GetEntityContext();

            if(!context->GetThingInHands(TT_WEAPON)) return;
            
            context->SetShotType(SHT_SNAPSHOT);
            SetShotCBs(GameScreen::CB_SNAPSHOT);
        }
    };
}
    
//==============================================================

namespace {
    
    class Switch2Sit : public EntityVisitor{
    public:
        
        void Visit(HumanEntity* human)
        {
            if(human->GetGraph()->IsSitPose() || !CanChangeHumanPose(human))
                return;

            human->GetGraph()->Cast2Human()->ChangeMoveType(GraphHuman::MT_WALK);
            human->GetGraph()->Cast2Human()->ChangePose(GraphHuman::PT_SIT);
            
            SetRunSitCBs(human);
            AIUtils::CalcAndShowPassField(human);
        }
    };
}
    
//==============================================================
    
namespace {

    class Switch2Stand : public EntityVisitor{
    public:
        
        void Visit(HumanEntity* human)
        {
            if(human->GetGraph()->IsSitPose() && !CanChangeHumanPose(human))
                return;

            human->GetGraph()->Cast2Human()->ChangeMoveType(GraphHuman::MT_WALK);
            human->GetGraph()->Cast2Human()->ChangePose(GraphHuman::PT_STAND);
            
            SetRunSitCBs(human);
            AIUtils::CalcAndShowPassField(human);
        }
    };
}    

//==============================================================
    
namespace {

    class Switch2Run : public EntityVisitor{
    public:
        
        void Visit(HumanEntity* human)
        {
            if(human->GetGraph()->IsSitPose() && !CanChangeHumanPose(human))
                return;

            human->GetGraph()->Cast2Human()->ChangeMoveType(GraphHuman::MT_RUN);
            human->GetGraph()->Cast2Human()->ChangePose(GraphHuman::PT_STAND);
            
            SetRunSitCBs(human);
            AIUtils::CalcAndShowPassField(human);
        }
    };
}

//==============================================================

namespace{

    class ScriptScenePlayer : public ActionManager, private EntityObserver{
    public:

        static ScriptScenePlayer* GetInst()
        {
            static ScriptScenePlayer imp;
            return &imp;
        }

        void Init(Activity* activity) 
        {
            m_activity = activity;

            if(m_entity = GetGameFormImp()->GetCurEnt()){                
                m_entity->Attach(this, EV_QUIT_TEAM);
                m_entity->Attach(this, EV_DEATH_PLAYED);
            }
        }

        Activity* CreateAction()
        { 
            GetGameFormImp()->HandleSelectReq(0);
            
            //поставить интерфейс для скриптовых сцен
            Screens::Instance()->Game()->ShowControls(false);   
            
            return m_activity;
        }

        void Shut()
        {
            if(m_entity){
                m_entity->Detach(this);
                GetGameFormImp()->HandleSelectReq(m_entity);
            }

            Screens::Instance()->Game()->ShowControls(true);            

            m_entity   = 0;
            m_activity = 0;
        }

    private:

        ScriptScenePlayer(){}

        void Update(BaseEntity* entity, event_t event, info_t info)
        {
            m_entity->Detach(this);
            m_entity = 0;
        }

    private:

        Activity*   m_activity;
        BaseEntity* m_entity;
    };
}

//==============================================================

namespace{
    
    //
    // обычная стратегия перемотки персонажей через меню GameFormImp
    //
    class GameFormHeroScroller : public AbstractScroller{
    public:

        static GameFormHeroScroller* GetInst()
        {
            static GameFormHeroScroller imp;
            return &imp;
        }
    
        void Prev()
        {
            HumanIcons* icons = GetGameFormImp()->GetHumanIcons();

            for(--m_index; m_index >= 0; --m_index){

                if(IsSuitable(icons->Icon2Entity(m_index)))
                    return;
            }

            if(m_index < 0) m_index = -1;
        }

        bool IsREnd() const
        {
            return m_index == -1;
        }
        
        void Last(HumanEntity* human)
        {
            for(int k = MAX_TEAMMATES - 1; human && k >= 0; k--){

                HumanEntity* hero = GetGameFormImp()->GetHumanIcons()->Icon2Entity(k);

                if(IsSuitable(hero) && human->GetEID() == hero->GetEID()){
                    m_index = k;
                    return;
                } 
            }

            m_index = MAX_TEAMMATES;

            Prev();
        }
        
        void Next()
        {
            HumanIcons* icons = GetGameFormImp()->GetHumanIcons();

            for(++m_index; m_index < MAX_TEAMMATES; ++m_index){

                if(IsSuitable(icons->Icon2Entity(m_index)))
                    return;
            }

            if(m_index >= MAX_TEAMMATES) m_index = MAX_TEAMMATES;
        }

        bool IsEnd() const
        {
            return m_index == MAX_TEAMMATES;
        }
        
        void First(HumanEntity* human = 0)
        {
            for(int k = 0; human && k < MAX_TEAMMATES; k++){

                HumanEntity* hero = GetGameFormImp()->GetHumanIcons()->Icon2Entity(k);

                if(IsSuitable(hero) && human->GetEID() == hero->GetEID()){                    
                    m_index = k;
                    return;
                }
            }

            m_index = -1;

            Next();
        }
        
        HumanEntity* Get()
        {
            return (IsEnd() || IsREnd()) ? 0 :  GetGameFormImp()->GetHumanIcons()->Icon2Entity(m_index);
        }

    protected:

        virtual bool IsSuitable(HumanEntity* hero) const
        {
            return hero != 0;
        }

        GameFormHeroScroller(){}    

    private:

        int m_index;
    };
}

//==============================================================

namespace{

    //
    // скролинг по всем персонажам в нек. радиусе
    //
    class RadiusHeroScroller : public GameFormHeroScroller {
    public:

        static RadiusHeroScroller* GetInst()
        { 
            static RadiusHeroScroller imp;
            return &imp;
        }

        void Init(const point3& from, float radius = default_hero_scroll_radius)
        {
            m_from = from;
            m_radius = radius;
        }

    private:

        bool IsSuitable(HumanEntity* hero) const
        {
            return hero && (m_from - hero->GetGraph()->GetPos3()).Length() < m_radius;            
        }

        RadiusHeroScroller() : m_radius(default_hero_scroll_radius) {}

    private:

        point3 m_from;
        float  m_radius;
    };
}

//==============================================================

namespace{

    //
    //  скроллинг только по одному человеку
    //
    class SingleHumanScroller : public AbstractScroller{
    public:

        static SingleHumanScroller* GetInst()
        {
            static SingleHumanScroller imp;
            return &imp;
        }
            
        void Prev() {}

        void Next() {}

        bool IsEnd() const { return true; }

        bool IsREnd() const { return true; }
    
        void Last(HumanEntity* human) { }

        void First(HumanEntity* human) { }

        HumanEntity* Get() { return m_human; }

        void Init(HumanEntity* human) { m_human = human; }

    private:

        HumanEntity* m_human;
    };
}

//==============================================================

namespace{

    //узнать позицию целовека
    ipnt2_t GetHumanPos(HumanEntity* human)
    {
        if(VehicleEntity* vehicle = human->GetEntityContext()->GetCrew())
            return vehicle->GetGraph()->GetPos2();
        
        return human->GetGraph()->GetPos2();
    }

    //
    // брать предметы с обычной земли
    //
    class UsualGround : public AbstractGround{
    public:

        static UsualGround* GetInst()
        {
            static UsualGround imp;
            return &imp;
        }

        bool CanInsert(HumanEntity* human, BaseThing* thing, std::string* reason) const 
        {
            return true;
        }
        
        void Insert(HumanEntity* human, BaseThing* thing)
        {   
            Depot::GetInst()->Push(GetHumanPos(human), thing);
        }
        
        void Remove(HumanEntity* human, BaseThing* thing)
        {
            Depot::GetInst()->Remove(thing->GetTID());
        }

        AbstractGround::Iterator* CreateIterator(HumanEntity* human)        
        {
            return new UsualGroundItor(human);
        }

        void PlaySound(sound_event sound)
        {
            if(sound == SE_DROP) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_DROP_TO_GROUND));
            if(sound == SE_DRAG) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_DRAG_FROM_GROUND));
        }
    
    private:

        UsualGround(){}

        class UsualGroundItor : public AbstractGround::Iterator{
        public:

            UsualGroundItor(HumanEntity* human) : m_actor(human) { First(); }

            BaseThing* Get() { return &*m_first; }
            
            void Next() { m_first ++; }
           
            void First() { m_first = Depot::GetInst()->begin(GetHumanPos(m_actor), ground_scroll_radius); }
            
            bool IsNotDone() const { return m_first != Depot::GetInst()->end(); }
               
        private:

            HumanEntity*    m_actor;
            Depot::iterator m_first;            
        };
    };
}

//==============================================================

namespace{
    
    //
    // стратегия для сбрасывания предметов на землю
    //
    class GroundBin : public AbstractBin{
    public:

        static GroundBin* GetInst()
        {
            static GroundBin bin;
            return &bin;
        } 

        void Insert(HumanEntity* actor, BaseThing* thing)
        {
            Depot::GetInst()->Push(GetHumanPos(actor), thing);
        }

    private:

        GroundBin() {}
    };
}

//==============================================================

namespace{

    //
    // стратегия для вынимания предметов из ящика
    //
    class StoreBoxGround : public AbstractGround{
    public:

        static StoreBoxGround* GetInst()
        { 
            static StoreBoxGround imp;
            return &imp;
        }

        void SetStoreBox(StoreBox* box) { m_box = box; }

        bool CanInsert(HumanEntity* human, BaseThing* thing, std::string* reason) const 
        {
            //вычислим вес ящика с предметом thing
            float weight = thing->GetWeight();
            for(StoreBox::iterator itor = m_box->begin(); itor != m_box->end(); ++itor)
                weight += itor->GetWeight();

            if(weight > m_box->GetWeight()){
                if(reason) *reason = DirtyLinks::GetStrRes("box_full");
                return false;
            }

            return true;
        }
        
        void Insert(HumanEntity* human, BaseThing* thing)
        {   
            m_box->Insert(thing);
        }
        
        void Remove(HumanEntity* human, BaseThing* thing)
        {
            m_box->Remove(thing);
        }
    
        AbstractGround::Iterator* CreateIterator(HumanEntity* human)
        {
            return new StoreBoxItor(m_box);
        }

        void PlaySound(sound_event sound)
        {
            if(sound == SE_DROP) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_DROP_TO_BOX));
            if(sound == SE_DRAG) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_DRAG_FROM_BOX));
        }
    
    private:

        StoreBoxGround(){}

        class StoreBoxItor : public AbstractGround::Iterator{
        public:

            StoreBoxItor(StoreBox* box) : m_box(box) { First(); }
            
            void Next() { ++ m_first; }

            BaseThing* Get() { return &*m_first; }

            void First() { m_first = m_box->begin(); }

            bool IsNotDone() const { return m_first != m_box->end(); }

        private:

            StoreBox*          m_box;
            StoreBox::iterator m_first;        
        };

    private:

        StoreBox* m_box;
    };
};

//==============================================================

namespace{

    //
    // механизм подхода к существу
    //
    class Approach2EntityManager : public ActionManager{
    public:

        Activity* CreateMove()
        {
            pnt_vec_t path;
            PathUtils::GetInst()->CalcPath(m_actor, PathUtils::GetInst()->GetNearPnt(m_entity).m_pnt, path);
            
            return path.empty() ? 0 : ActivityFactory::GetInst()->CreateMove(m_actor, path, ActivityFactory::CT_PLAYER_MOVE);
        }
        
        Activity* CreateRotate()
        {
            return      m_rotate == RT_ACTOR2ENTITY
                    ?   CreateRotate(m_actor, m_entity)
                    :   CreateRotate(m_entity, m_actor);
        }
        
        bool NeedMove() const
        {
            if(m_actor->GetEID() == m_entity->GetEID())
                return false; 

            PathUtils::GetInst()->CalcPassField(m_actor);
            return PathUtils::GetInst()->GetNearPnt(m_entity).m_cost != 1;
        }
        
        bool NeedRotate() const
        {
            return      m_rotate == RT_ACTOR2ENTITY
                    ?   NeedRotate(m_actor, m_entity)
                    :   NeedRotate(m_entity, m_actor);
        }

    protected:

        enum rotate_type{
            RT_ACTOR2ENTITY, //actor поворачивается к существу
            RT_ENTITY2ACTOR, //существо повворачивается к actor
        };

        void Init(BaseEntity* actor, BaseEntity* entity, rotate_type type)
        {
            m_actor = actor;
            m_entity = entity;

            point3 pos1 = actor->GetGraph()->GetPos3(),
                   pos2 = entity->GetGraph()->GetPos3();
        }

    private:

        //должен ли первый повернуться ко второму
        bool NeedRotate(BaseEntity* first, BaseEntity* second) const
        {
            if(first->GetEID() == second->GetEID())
                return false; 

            point3 dir = second->GetGraph()->GetPos3() - first->GetGraph()->GetPos3();
            return first->GetGraph()->NeedRotate(dir);
        }

        //создать поворот первого ко второму
        Activity* CreateRotate(BaseEntity* first, BaseEntity* second)
        {
            point3 dir = second->GetGraph()->GetPos3() - first->GetGraph()->GetPos3();
            return ActivityFactory::GetInst()->CreateRotate(first, Dir2Angle(dir), ActivityFactory::CT_CIVILIAN_ROTATE);
        }

    private:

        rotate_type m_rotate;

        BaseEntity* m_actor;
        BaseEntity* m_entity;
    };
}

//==============================================================

namespace{

    //
    // класс для Use на существе
    //
    class EntityUseManager : public Approach2EntityManager, private EntityVisitor{
    public:

        static EntityUseManager* GetInst()
        {
            static EntityUseManager imp;
            return &imp;
        }

        void Init(HumanEntity* actor, BaseEntity* ent4use)
        {
            m_actor = actor;
            m_ent4use = ent4use;
            
            Approach2EntityManager::Init(actor, ent4use, RT_ENTITY2ACTOR);
        }

        void Shut()
        {
            m_ent4use->Accept(*this);
        }

    private:

        EntityUseManager(){}

        void Visit(HumanEntity* human)
        {
            UseBaseEntity(human);
        }

        void Visit(TraderEntity* trader)
        {
            UseBaseEntity(trader);

            RadiusHeroScroller::GetInst()->Init(trader->GetGraph()->GetPos3());

            if(EnemyDetector::getInst()->isHeEnemy4Me(m_actor, trader)){

                Forms::GetInst()->ShowTalkDialog(trader, phrase_s(DirtyLinks::GetStrRes("gf_cant_trade"), ""));

            }else{

                GetGameFormImp()->HandleShowShopReq(m_actor->Cast2Human(), trader,
                                                UsualGround::GetInst(),
                                                RadiusHeroScroller::GetInst(),
                                                GroundBin::GetInst());
            }
        }

        void Visit(VehicleEntity* vehicle)
        {
            UseBaseEntity(vehicle);
        }

        void UseBaseEntity(BaseEntity* entity)
        {
            QuestServer::GetInst()->HandleUseEntity(m_actor, entity);   
        }

    private:

        HumanEntity* m_actor;
        BaseEntity*  m_ent4use;     
    };
}

//==============================================================

namespace{

    //
    // положить предмет в ящик
    //
    class StoreBoxAcceptor : public ThingScriptParser::Acceptor{
    public:

        StoreBoxAcceptor(StoreBox* box) : m_box(box) {}

        bool AcceptThing(BaseThing* thing)
        {
            m_box->Insert(thing);
            return true;
        }

        //принять информцию об организации
        void AcceptOrgInfo(const rid_t rid) {}
        //принять информацию о существах
        void AcceptEntInfo(entity_type type, const rid_t& rid) {}
        //принять информацию о предметах
        void AcceptThingInfo(thing_type type, const rid_t& rid) {}

    private:

        StoreBox* m_box;
    };

    //
    // принять информацию с консоли
    //
    class HandbookAcceptor : public ThingScriptParser::Acceptor{
    public:

        bool AcceptThing(BaseThing* thing) { return false; }

        void AcceptOrgInfo(const rid_t rid)
        {
            ForceHandbook::GetInst()->Push(0, rid, ForceHandbook::F_NEW_REC);
        }

        void AcceptThingInfo(thing_type type, const rid_t& rid)
        {
            ThingHandbook::GetInst()->Push(type, rid, ThingHandbook::F_NEW_REC);
        }

        void AcceptEntInfo(entity_type type, const rid_t& rid)
        {
            EntityHandbook::GetInst()->Push(type, rid, EntityHandbook::F_NEW_REC);
        }
    };

    //
    // класс для use на объекте
    //
    class ObjectUseManager : public ActionManager, private BoxVisitor{
    public:

        static ObjectUseManager* GetInst()
        {
            static ObjectUseManager imp;
            return &imp;
        }

        void Init(HumanEntity* actor, const rid_t& rid)
        {
            m_object = rid;
            m_actor = actor;            
            m_obj_hexes.clear();
            DirtyLinks::CalcObjectHexes(m_object, &m_obj_hexes);
        }

        Activity* CreateAction()
        {
			if(!DirtyLinks::IsGoodEntity(m_actor))
			{
				m_object.clear();
				m_actor=NULL;
				return NULL;
			}
			else
			{
            if(!CanUseObject(m_object)) m_object.clear();
			}
                                    
            return ActivityFactory::GetInst()->CreateUse(m_actor, m_object,                    
                                                    m_object.empty()
                                                ?   ActivityFactory::CT_PLAYER_USE_FAILED
                                                :   ActivityFactory::CT_PLAYER_USE_SUCCEED);            
        }

        Activity* CreateMove()
        {
            PathUtils::near_pnt_t near_pnt = PathUtils::GetInst()->GetNearPnt(m_obj_hexes);

            if(!near_pnt.IsDefPnt()){
                pnt_vec_t path;
                PathUtils::GetInst()->CalcPath(m_actor, near_pnt.m_pnt, path);
                return ActivityFactory::GetInst()->CreateMove(m_actor, path, ActivityFactory::CT_PLAYER_MOVE);
            }
            
            return 0;
        }

        Activity* CreateRotate()
        {
            point3 dir = DirtyLinks::GetObjCenter(m_object) - m_actor->GetGraph()->GetPos3();
            return ActivityFactory::GetInst()->CreateRotate(m_actor, Dir2Angle(dir), ActivityFactory::CT_CIVILIAN_ROTATE);
        }

        bool NeedMove() const
        {
            PathUtils::GetInst()->CalcPassField(m_actor);

            PathUtils::near_pnt_t near_pnt = PathUtils::GetInst()->GetNearPnt(m_obj_hexes);
            return near_pnt.m_cost != 0;
        }

        bool NeedRotate() const
        {
            point3 dir = DirtyLinks::GetObjCenter(m_object) - m_actor->GetGraph()->GetPos3();
            return m_actor->GetGraph()->NeedRotate(dir);
        }

        void Shut()
        {
            //если объект есть
            if(m_object.size()){

                GameObserver::use_info info(m_actor, m_object);
                GameEvMessenger::GetInst()->Notify(GameObserver::EV_USE, &info);

                if(BaseBox* box = Bureau::GetInst()->Get(m_object))
                    box->Accept(*this);
                
                //пометить как использованный
                GameObjectsMgr::GetInst()->MarkAsUsed(m_object);
            }
        }

    private:

        ObjectUseManager(){}

        bool CanUseObject(const rid_t& obj_rid)
        {
            if(GameObjectsMgr::GetInst()->IsUsed(obj_rid))
                return true;

            HumanContext* context = m_actor->GetEntityContext();
            rid_t         key_rid = DirtyLinks::GetKey4UseObj(obj_rid);
            
            //найдем ключ у человека
            if(key_rid.size()){
                
                HumanContext::iterator itor = context->begin(HPK_ALL, TT_KEY);
                
                while(itor != context->end()){

                    if(itor->GetInfo()->GetRID() == key_rid){   
                        
                        KeyThing* thing = static_cast<KeyThing*>(&*itor);

                        m_actor->Notify(EntityObserver::EV_OPEN_SUCCEED);

                        DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("gf_key_used").c_str(),
                                           m_actor->GetInfo()->GetName().c_str(),
                                           thing->GetInfo()->GetName().c_str()));

                        DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("open_success").c_str(),
                                           m_actor->GetInfo()->GetName().c_str()));

                        //проиграть звук на используемый ключ
                        SndPlayer::GetInst()->Play(thing->GetInfo()->GetUseSound());

                        context->RemoveThing(itor);
                        delete thing;
                        return true;
                    }

                    ++itor;
                }
            }

            int wisdom = DirtyLinks::GetWisdom4UseObj(obj_rid);

            if(wisdom >= 0 && context->GetTraits()->GetWisdom() > wisdom){

                if(wisdom){
                    m_actor->Notify(EntityObserver::EV_OPEN_SUCCEED);
                    DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("open_success").c_str(),
                                       m_actor->GetInfo()->GetName().c_str()));
                }

                return true;
            }

            m_actor->Notify(EntityObserver::EV_OPEN_FAILED);

            if(key_rid.size()){

                DirtyLinks::Print( mlprintf(DirtyLinks::GetStrRes("gf_need_key").c_str(), 
                                   m_actor->GetInfo()->GetName().c_str()));

            }else if(wisdom >= 0){

                DirtyLinks::Print( mlprintf(DirtyLinks::GetStrRes("gf_no_wisdom").c_str(),
                                   m_actor->GetInfo()->GetName().c_str())); 
            }
                
            DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("open_fail").c_str(), m_actor->GetInfo()->GetName().c_str()), DirtyLinks::MT_DENIAL);
            return false;            
        }

        void Visit(StoreBox* store)
        {
            //если ящик до этого не открывался заполним его предметами
            if(!GameObjectsMgr::GetInst()->IsUsed(store->GetRID())){
               
                if(store->GetScriptRID().size()){

                    StoreBoxAcceptor  acceptor(store);
                    ThingScriptParser parser(&acceptor, ThingFactory::GetInst());
                
                    int line;
                    std::string script;
                
                    if(!AIUtils::GetItemSetStr(store->GetScriptRID(), &script, &line))
                        ThrowCantReadScript(store->GetRID(), store->GetScriptRID());
                
                    if(!parser.Parse(script))
                      ThrowScriptParseError(store->GetRID(), store->GetScriptRID(), line, parser.GetLastPos());
                }

                AIUtils::AddExp4Hack(store->GetExperience(), m_actor);
            }
                        
            StoreBoxGround::GetInst()->SetStoreBox(store);

            SingleHumanScroller::GetInst()->Init(m_actor);

            GetGameFormImp()->HandleShowInventoryReq(
                        GetGameFormImp()->GetCurEnt()->Cast2Human(), 
                        StoreBoxGround::GetInst(),
                        SingleHumanScroller::GetInst(),
                        GroundBin::GetInst());
        }

        void Visit(DataBox* console)
        {
            //если ящик еще не открывался
            if(!GameObjectsMgr::GetInst()->IsUsed(console->GetRID())){
                
                HandbookAcceptor acceptor;
                ThingScriptParser parser(&acceptor);
                
                int line;
                std::string script;
                
                if(!AIUtils::GetItemSetStr(console->GetScriptRID(), &script, &line))
                    ThrowCantReadScript(console->GetRID(), console->GetScriptRID());
                
                if(!parser.Parse(script))
                    ThrowScriptParseError(console->GetRID(), console->GetScriptRID(), line, parser.GetLastPos());

                AIUtils::AddExp4Hack(console->GetExperience(), m_actor);
                return;
            }

            DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("no_data").c_str(), m_actor->GetInfo()->GetName().c_str()));                        
        }

        void ThrowCantReadScript(const rid_t& box_rid, const rid_t& script_rid)
        {
            std::ostringstream ostr;
            
            ostr << "GetItemSetStr: нет набора <" << script_rid << ">" << std::endl;
            ostr << "ящик:\t" << box_rid << std::endl;
            ostr << "уровень:\t" << DirtyLinks::GetLevelSysName() << std::endl;
            
            throw CASUS(ostr.str());
        }

        void ThrowScriptParseError(const rid_t& box_rid, const rid_t& script_rid, int line, int pos)
        {
            std::ostringstream ostr;
            
            ostr << "ScriptParser: ошибка разбора в наборе <" << script_rid << ">" << std::endl;
            ostr << "ящик:\t" << box_rid << std::endl;
            ostr << "уровень:\t " << DirtyLinks::GetLevelSysName() << std::endl;
            ostr << "item_sets.xls: line = " << line + 1 << " pos = " << pos << std::endl;
            
            throw CASUS(ostr.str());
        }

    private:

        rid_t m_object;
      
        pnt_vec_t    m_obj_hexes;
        HumanEntity* m_actor;
    };
};

//==============================================================

namespace{

    //
    // менеджер use'a для лечения
    //
    class TreatmentManager : public Approach2EntityManager{
    public:

        static TreatmentManager* GetInst()
        {
            static TreatmentManager imp;
            return &imp;
        }

        void Init(HumanEntity* doctor, HumanEntity* sick)
        {
            m_sick = sick;
            m_doctor = doctor;

            Approach2EntityManager::Init(doctor, sick, RT_ACTOR2ENTITY);

            m_medikit = static_cast<MedikitThing*>(doctor->GetEntityContext()->GetThingInHands(TT_MEDIKIT));
        }

        Activity* CreateAction()
        {
            AIUtils::MakeTreatment(m_doctor, m_sick, m_medikit);
            return 0;
        }

        void Shut()
        {
            if(m_medikit->GetCharge() == 0){
                m_doctor->GetEntityContext()->RemoveThing(m_medikit);
                delete m_medikit;
            }
        }

    private:

        TreatmentManager(){}

    private:

        HumanEntity*  m_sick;
        HumanEntity*  m_doctor;
        MedikitThing* m_medikit;
    };
}

//==============================================================

namespace{

    //
    // менеджер взлома техники
    //
    class EntityHackManager : public Approach2EntityManager{
    public:

        static EntityHackManager* GetInst()
        {
            static EntityHackManager imp;
            return &imp;
        }

        void Init(HumanEntity* hacker, VehicleEntity* vehicle, float probability)
        {
            m_hacker = hacker;
            m_vehicle = vehicle;

            m_probability = probability;

            Approach2EntityManager::Init(hacker, vehicle, RT_ACTOR2ENTITY);
        }

        void Shut()
        {
            if(NormRand() < m_probability){ 

                CommonScenario("gf_hack_succeed", EntityObserver::EV_OPEN_SUCCEED);

                AIUtils::ChangeEntityPlayer(m_vehicle, PT_PLAYER);
                AIUtils::AddExp4Hack(m_vehicle->GetInfo()->GetExp4Hack(), m_hacker);

                GetGameFormImp()->HandleSelectReq(m_vehicle);
                return;
            }

            CommonScenario("gf_hack_failed", EntityObserver::EV_OPEN_FAILED);
        }

    private:

        void CommonScenario(const rid_t& res_str, EntityObserver::event_t event)
        {
            m_hacker->Notify(event);
            m_hacker->GetEntityContext()->GetTraits()->AddMovepnts(- MPS_FOR_HACK);
            
            DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes(res_str).c_str(),
                m_hacker->GetInfo()->GetName().c_str(),
                m_vehicle->GetInfo()->GetName().c_str()));
        }


    private:

        float m_probability;

        HumanEntity*   m_hacker;
        VehicleEntity* m_vehicle;
    };
}

//==============================================================

namespace{

    //
    // менеджер для перемещения существа
    //
    class EntityMoveManager : public ActionManager{
    public:

        enum flag_type{
            F_NEED_WALK       = 1 << 0, //перейти в режим дохождения если не достигли точки назначения
            F_CALC_PASS_FIELD = 1 << 1, //расчитать поле проходимости перед перемещением

            F_INS_2_WALKERS   = 1 << 2,

            F_USUAL_MOVE   = F_NEED_WALK,
            F_WALKERS_MOVE = F_NEED_WALK|F_CALC_PASS_FIELD,
        };
		virtual void Shut()
		{
			m_actor = 0;
			m_flags=0;
		}
        static EntityMoveManager* GetInst()
        {
            static EntityMoveManager imp;
            return &imp;
        }

        void Init(BaseEntity* actor, const ipnt2_t& hex_pnt, unsigned flags = F_USUAL_MOVE)
        {
            m_hex = hex_pnt;
            m_actor = actor;

            m_flags = flags;
          
            VehicleEntity* vehicle = actor->Cast2Vehicle();
            if(vehicle && !CanVehicleMove(vehicle)){
                
                m_actor = 0;
                
                if(IsNoDriver4Vehicle(vehicle)){

                    EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);
                    while(itor != EntityPool::GetInst()->end()){

                        VehicleEntity* crew = itor->Cast2Human()->GetEntityContext()->GetCrew();

                        if(crew && crew->GetEID() == vehicle->GetEID()){
                            itor->Notify(EntityObserver::EV_CAR_DRIVE_FAILED);
                            break;
                        }

                        ++itor;
                    }

                    DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("gf_no_driver").c_str(),
                                vehicle->GetInfo()->GetName().c_str()));
                }
            }
            
            HumanEntity* human = actor->Cast2Human();
            if(human && !CanHumanMove(human)){
                
                m_actor = 0;
                
                //если не можем сделать ни шагу из-за перегрузки
                if(IsHumanOverloaded(human)){
                    DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("weight_overload").c_str(),
                                human->GetInfo()->GetName().c_str()));        
                }
            }

            if(m_actor) m_actor->Notify(EntityObserver::EV_MOVE_COMMAND);
        }

        bool NeedMove() const
        {
            if(m_flags & F_INS_2_WALKERS && m_flags & F_NEED_WALK) GetGameFormImp()->GetWalkers()->Insert(m_actor, m_hex);
            return m_actor != 0;
        }

        Activity* CreateMove()
        { 
            if(m_flags & F_CALC_PASS_FIELD) PathUtils::GetInst()->CalcPassField(m_actor);

            pnt_vec_t   path;                            

            //вычислить путь и породить действие
            PathUtils::GetInst()->CalcPath(m_actor, m_hex, path);

            if(m_flags & F_NEED_WALK){
            
                //если путь длинный => занесемся в ходоки
                if(HumanEntity* human = m_actor->Cast2Human()){                

                    if(path.size() > human->GetEntityContext()->GetStepsCount())
                        m_flags |= F_INS_2_WALKERS;                  

                }else if(VehicleEntity* vehicle = m_actor->Cast2Vehicle()){

                    if(path.size() > vehicle->GetEntityContext()->GetStepsCount())
                        m_flags |= F_INS_2_WALKERS;

                }
            }
                        
            return ActivityFactory::GetInst()->CreateMove(m_actor, path, ActivityFactory::CT_PLAYER_MOVE);;
        }

    private:

        EntityMoveManager(){}

        bool CanVehicleMove(VehicleEntity* vehicle) const
        {
            return      vehicle->GetInfo()->IsRobot()
                    ||  (       vehicle->GetInfo()->IsTech()
                            &&  vehicle->GetEntityContext()->GetDriver());
        }

        bool CanHumanMove(HumanEntity* human) const { return true; }

        bool IsHumanOverloaded(HumanEntity* human) const
        {
            HumanContext* context = human->GetEntityContext();
            return context->GetTraits()->GetWeight() > context->GetLimits()->GetWeight();                  
        }

        bool IsNoDriver4Vehicle(VehicleEntity* vehicle) const
        {
            return      vehicle->GetInfo()->IsTech()
                    &&  vehicle->GetEntityContext()->GetDriver() == 0;
        }

    private:

        unsigned    m_flags;

        ipnt2_t     m_hex;
        BaseEntity* m_actor;
    };
}
    
//==============================================================

namespace{

    class EntityRotateManager : public ActionManager{
    public:

        static EntityRotateManager* GetInst()
        {
            static EntityRotateManager imp;
            return &imp;
        }

        void Init(BaseEntity* actor, const point3& dir)
        {
            m_dir = dir;
            m_actor = actor;
        }

        Activity* CreateRotate()
        { 
            return ActivityFactory::GetInst()->CreateRotate(m_actor, Dir2Angle(m_dir), ActivityFactory::CT_PLAYER_ROTATE);
        }

        bool NeedRotate() const { return true; }

    private:

        EntityRotateManager() {}

    private:

        point3      m_dir;        
        BaseEntity* m_actor;
    };
}

//==============================================================

namespace{

    class EntityShootManager : public ActionManager{
    public:

        static EntityShootManager* GetInst()
        {
            static EntityShootManager imp;
            return &imp;
        }

        void Init(BaseEntity* entity, float accuracy, const point3& to, eid_t victim, const rid_t& obj)
        {
            m_entity = entity;

            m_to = to;
            m_accuracy = accuracy;
            m_victim = victim;
            m_object = obj;

            m_entity->Notify(EntityObserver::EV_ATTACK_COMMAND);
        }

        Activity* CreateAction()
        {
            if(m_object.size()) return ActivityFactory::GetInst()->CreateShoot(m_entity, m_to, m_accuracy, 
                                                  ActivityFactory::CT_PLAYER_SHOOT,
                                                  ActivityFactory::shoot_info_s(m_object));

            return ActivityFactory::GetInst()->CreateShoot(m_entity, m_to, m_accuracy,
                ActivityFactory::CT_PLAYER_SHOOT,
                ActivityFactory::shoot_info_s(m_victim));
        }

    private:

        EntityShootManager(){}

    private:

        BaseEntity* m_entity;

        eid_t  m_victim;
        rid_t  m_object;

        point3 m_to;
        float  m_accuracy;
    };
}

//==============================================================

namespace {

    class ThingThrowManager : public ActionManager{
    public:

        static ThingThrowManager* GetInst()
        {
            static ThingThrowManager imp;
            return &imp;
        }

        void Init(HumanEntity* human, const point3& pnt)
        {
            m_to = pnt;
            m_human = human;
        }

        Activity* CreateAction()
        {
            return ActivityFactory::GetInst()->CreateThrow(m_human, m_to, ActivityFactory::CT_PLAYER_GRENADE_THROW);
        }

    private:

        ThingThrowManager(){}

    private:
        
        point3 m_to;
        HumanEntity* m_human;
    };
};

//==============================================================

namespace{

    //
    // пустой обработчик ввода
    //
    class EmptyInputManager : public InputManager {
    public:
        
        void HandleInput(){}
    };
}

//==============================================================

namespace{

    //
    // обычный обработчик ввода
    //
    class UsualInputManager : public InputManager {
    public:

        void HandleInput()
        {
            GameFormImp* game_form = GetGameFormImp();

            GraphEntity::ShowDecor(GraphEntity::DT_FOS, (GetGameFormImp()->CanShowFOS() || Input::KeyState(DIK_L)) != 0);
            GraphEntity::ShowDecor(GraphEntity::DT_BANNER, Input::KeyState(DIK_TAB) != 0);
            
            if(Input::KeyBack(DIK_RETURN) || Input::KeyBack(DIK_NUMPADENTER)){
                GetGameFormImp()->HandleEndOfTurnReq();    
                return;
            }
            
            if(Input::KeyBack(DIK_J)) game_form->HandleShowJournalReq();

            if(Input::KeyBack(DIK_HOME)) SwitchRoofs();
            
            if(Input::KeyBack(DIK_ADD))
                Screens::Instance()->Game()->MsgWindow()->IncMsgWindow();
            
            if(Input::KeyBack(DIK_SUBTRACT))
                Screens::Instance()->Game()->MsgWindow()->DecMsgWindow();

            if(Input::KeyBack(DIK_M)) game_form->HandleShowOptionsReq();

            if(Input::KeyBack(DIK_B)) game_form->HandleSwitchHandsMode();

            if(Input::KeyBack(DIK_LBRACKET)) game_form->HandlePrevFastAccessThing();

            if(Input::KeyBack(DIK_RBRACKET)) game_form->HandleNextFastAccessThing();

            if(Input::KeyBack(DIK_C) && game_form->GetCurEnt()){
                if(ActionManager* action = game_form->GetWalkers()->CreateAction(game_form->GetCurEnt()))
                    game_form->HandlePlayActionReq(action);
            }

            int index = GetPressedIndex();

            if(index >= 0){

                if(Input::KeyState(DIK_LCONTROL) || Input::KeyState(DIK_RCONTROL)){
                    
                    if(BaseEntity* entity = game_form->GetEnemyIcons()->Icon2Entity(index))
                        Cameraman::GetInst()->FocusEntity(entity);                 
                    
                }else if(Input::KeyState(DIK_RALT) || Input::KeyState(DIK_LALT)){

                    if(VehicleEntity* vehicle = game_form->GetRobotIcons()->Icon2Entity(index))
                        game_form->HandleSelectReq(vehicle, GameFormImp::SEL_COMMAND);
                    
                }else if(index < 6){
                    
                    if(HumanEntity* human = game_form->GetHumanIcons()->Icon2Entity(index))
                        game_form->HandleSelectReq(human, GameFormImp::SEL_COMMAND);                
                }
            }
            
            if(Input::KeyBack(DIK_ESCAPE)){

                if(game_form->CanMakeHack())
                    game_form->HandleSwitchHackReq();
                else if(game_form->CanAttack())
                    game_form->HandleSwitchAttackReq();
                else if(game_form->CanMakeUse())
                    game_form->HandleSwitchUseReq();
                else if(game_form->GetCurEnt())
                    game_form->HandleSelectReq(0);
                else
                    game_form->HandleShowOptionsReq();
            }
            
            if(game_form->GetCurEnt() && Input::KeyBack(DIK_SPACE))
                Cameraman::GetInst()->FocusEntity(game_form->GetCurEnt());            
            
            if(game_form->GetCurEnt() && game_form->GetCurEnt()->Cast2Human() && Input::KeyBack(DIK_I))
                game_form->HandleShowInventoryReq(game_form->GetCurEnt()->Cast2Human(), UsualGround::GetInst(), GameFormHeroScroller::GetInst(), GroundBin::GetInst());
            
            if(game_form->GetCurEnt() && Input::KeyBack(DIK_SLASH))
                game_form->HandleSwitch2Aimshot();
            
            if(game_form->GetCurEnt() && Input::KeyBack(DIK_PERIOD))
                game_form->HandleSwitch2Snapshot();    
            
            if(game_form->GetCurEnt() && Input::KeyBack(DIK_COMMA))
                game_form->HandleSwitch2Autoshot();    
            
            if(game_form->GetCurEnt() && Input::KeyBack(DIK_S)){
                if(IsChecked(GameScreen::CB_SIT))
                    game_form->HandleSwitch2Stand();    
                else
                    game_form->HandleSwitch2Sit();
            }
            
            if(game_form->GetCurEnt() && Input::KeyBack(DIK_R)){
                if(IsChecked(GameScreen::CB_RUN))
                    game_form->HandleSwitch2Stand();    
                else
                    game_form->HandleSwitch2Run();
            }
            
            if(game_form->GetCurEnt() && Input::KeyBack(DIK_W))
                game_form->HandleSwitch2Stand();    
            
            if(Input::KeyBack(DIK_N)){
                
                BaseEntity*           current  = game_form->GetCurEnt();
                GameFormHeroScroller* scroller = GameFormHeroScroller::GetInst();
                
                //на все про все 2 - ва шага
                int iterations = 2;
                
                while(iterations--){
                    
                    //если никто не выделен, выделим человека
                    if(current == 0){                        
                        
                        scroller->First();                        

                        if(!scroller->IsEnd()){
                            current = scroller->Get();
                            break;
                        }
                    }
                    
                    //если выделен человек, выделим след человека
                    if(current && current->Cast2Human()){

                        scroller->First(current->Cast2Human());                        
                        scroller->Next();
                                                 
                        if(current = scroller->IsEnd() ? 0 : scroller->Get())
                            break;
                    }
                    
                    //если человек на выделяется выделим первую технику
                    if(current == 0 && (current = FindNextEntity(game_form->GetTechIcons(),static_cast<VehicleEntity*>(0))))
                        break;
                    
                    //если выделена технику выделим след. технику
                    if(current && current->Cast2Vehicle() && (current = FindNextEntity(game_form->GetTechIcons(), current->Cast2Vehicle())))
                        break;
                }
                
                if(current){
                    game_form->HandleSelectReq(current);
                    Cameraman::GetInst()->FocusEntity(current);
                    current->Notify(EntityObserver::EV_SELECT_COMMAND);
                }
            }

            if(Input::KeyBack(DIK_E)) game_form->HandleReloadWeapon();

            if(Input::KeyBack(DIK_F5)) DirtyLinks::MakeQuickSaveLoad(Storage::SM_SAVE);
            if(Input::KeyBack(DIK_F9)) DirtyLinks::MakeQuickSaveLoad(Storage::SM_LOAD);

            //перемотка оружия в колесиком мышки
            if(Input::MouseState().dz && Screens::Instance()->Game()->IsCursorOnWeaponIcon()){
                if(Input::MouseState().dz > 0)
                    game_form->HandleNextFastAccessThing();
                else
                    game_form->HandlePrevFastAccessThing();
            }
            
            //~~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~~

#ifdef _HOME_VERSION
       
            if(game_form->GetCurEnt() && Input::KeyBack(DIK_Z)){

                if(HumanEntity* human = game_form->GetCurEnt()->Cast2Human()){
                    HumanContext::Traits* traits = human->GetEntityContext()->GetTraits();
                    traits->AddMovepnts(traits->GetMovepnts() + 1);
                }else if(VehicleEntity* vehicle = game_form->GetCurEnt()->Cast2Vehicle()){
                    VehicleContext* context = vehicle->GetEntityContext();
                    context->SetMovepnts(context->GetMovepnts()*2+1);
                }
            }       
            
            if(game_form->GetCurEnt() && game_form->GetCurEnt()->Cast2Human() && Input::KeyBack(DIK_X)){
                
                EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_TRADER);
                
                if(itor != EntityPool::GetInst()->end()){  

                    game_form->HandleShowShopReq(game_form->GetCurEnt()->Cast2Human(),
                                                 itor->Cast2Trader(),
                                                 UsualGround::GetInst(),
                                                 GameFormHeroScroller::GetInst(),
                                                 GroundBin::GetInst());                
                    return;
                }            
            }

#endif            
            //~~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~~
            
            //обработка мышиного курсора
            game_form->GetCursor()->Update();       
        }
    };
}

//==============================================================

namespace{

    //
    // обработчик ввода для действия 
    //
    class ActivityInputManager : public InputManager{
    public:
        
        void HandleInput()
        {
            activity_command user_cmd = AC_TICK;
            
            if(IsLeftMouseButtonDoubleClick(GetGameFormImp()->GetCursor()->GetLastClickTime()))
                user_cmd = AC_SPEED_UP;
            else if(IsLeftMouseButtonClick())
                user_cmd = AC_STOP;
            
            GetGameFormImp()->RunActivity(user_cmd);
        }
    };
}

//==============================================================

namespace{

    //
    // класс установливющий иконку человека при выборе существа
    //
    class BodySelector : public EntityVisitor{
    public:

        void Visit(HumanEntity* human)
        {
            HumanContext* context = human->GetEntityContext();

            GameScreen::BodyIconTraits icon_traits;
            
            icon_traits.m_Damages = 0;
            if(context->HaveLongDamage(::DT_SHOCK)) icon_traits.m_Damages |= GameScreen::BodyIconTraits::DT_SHOCK;
            if(context->HaveLongDamage(::DT_FLAME)) icon_traits.m_Damages |= GameScreen::BodyIconTraits::DT_FLAME;
            if(context->HaveLongDamage(::DT_ELECTRIC)) icon_traits.m_Damages |= GameScreen::BodyIconTraits::DT_ELECTRIC;
            
            icon_traits.m_Body  = Dmg2BodyPartState(context->GetBodyPartDamage(BPT_BODY));
            icon_traits.m_Hands = Dmg2BodyPartState(context->GetBodyPartDamage(BPT_HANDS)); 
            icon_traits.m_Head  = Dmg2BodyPartState(context->GetBodyPartDamage(BPT_HEAD));
            icon_traits.m_Legs  = Dmg2BodyPartState(context->GetBodyPartDamage(BPT_LEGS));
            
            Screens::Instance()->Game()->SetBodyIcon(icon_traits);
        }

    private:

        GameScreen::BodyIconTraits::BODY_PARTS_STATE Dmg2BodyPartState(int val)
        {
            const int m_low_dmg = 1;
            const int m_medium_dmg = 5;
            const int m_huge_dmg = 15;
            
            if( val <= m_low_dmg ) return GameScreen::BodyIconTraits::BPS_GREEN;
            if( val <= m_medium_dmg) return GameScreen::BodyIconTraits::BPS_BLUE;
            if( val <= m_huge_dmg) return GameScreen::BodyIconTraits::BPS_YELLOW;
            
            return GameScreen::BodyIconTraits::BPS_RED;
        }
    };

    //
    // класс устанавливающий иконку с оружием
    //
    class WeaponSelector : public EntityVisitor{
    private:

        class ShotTypeMpsAdapter : public GameScreen::ShootMovePoints{
        public:

            ShotTypeMpsAdapter() { Init(); }

            ShotTypeMpsAdapter(HumanEntity* human, WeaponThing* thing)
            {
                int hum_mps = human->GetEntityContext()->GetTraits()->GetMovepnts();

                m_aim_count = Int2Str(thing->GetMovepnts(SHT_AIMSHOT));
                m_AimColor  = Bool2Color(hum_mps >= thing->GetMovepnts(SHT_AIMSHOT));
                
                m_snap_count = Int2Str(thing->GetMovepnts(SHT_SNAPSHOT));
                m_SnapColor  = Bool2Color(hum_mps >= thing->GetMovepnts(SHT_SNAPSHOT));

                if(thing->GetInfo()->IsAuto()){
                    m_auto_count = Int2Str(thing->GetMovepnts(SHT_AUTOSHOT));
                    m_AutoColor  = Bool2Color(hum_mps >= thing->GetMovepnts(SHT_AUTOSHOT));
                }

                Init();
            }

            ShotTypeMpsAdapter(HumanEntity* human, BaseThing* thing)
            {
                int thing_mps = AIUtils::CalcMps2Act(thing);
                int hum_mps   = human->GetEntityContext()->GetTraits()->GetMovepnts();

                m_aim_count = Int2Str(thing_mps);
                m_AimColor  = Bool2Color(hum_mps >= thing_mps);

                Init();
            }

            ShotTypeMpsAdapter(VehicleEntity* vehicle)
            {
                if(vehicle->GetInfo()->GetAmmoCount()){
                    
                    int mps4sht = vehicle->GetInfo()->GetMp4Shot();
                    int veh_mps = vehicle->GetEntityContext()->GetMovepnts();

                    m_aim_count = Int2Str(mps4sht);
                    m_AimColor  = Bool2Color(veh_mps >= mps4sht);                    
                }

                Init();
            }

        private:

            void Init()
            {
                m_AimMP = m_aim_count.c_str();
                m_SnapMP = m_snap_count.c_str();
		        m_AutoMP = m_auto_count.c_str();
            }

            unsigned Bool2Color(bool flag) { return flag ? 0x00ff00 : 0xff0000; }

            std::string Int2Str(int value)
            {
                std::ostringstream ostr;
                ostr << value;
                return ostr.str();
            }
           
        private:

            std::string m_aim_count;
            std::string m_snap_count;
            std::string m_auto_count;
        };

        class WeaponIconAdapter : public GameScreen::WeaponIconTraits{
        public:

            WeaponIconAdapter(BaseThing* thing)
            {
                Init();
                SetThing(thing);
            }

            WeaponIconAdapter(WeaponThing* thing)
            {
                Init(); 
                SetThing(thing);
            
                if(AmmoThing* ammo = thing->GetAmmo()){
                    SetCount(ammo->GetCount());
                    SetInitials(ammo->GetInfo()->GetInitials());
                }
            }

            WeaponIconAdapter(MedikitThing* med)
            {
                Init(); 
                SetThing(med);
                SetCount(med->GetCharge());
            }

            WeaponIconAdapter(VehicleEntity* vehicle)
            {
                Init();

                if(vehicle->GetInfo()->GetAmmoCount()){
                    SetShader(vehicle->GetInfo()->GetWeaponIcon());
                    SetInitials(vehicle->GetInfo()->GetAmmoInfo());
                    SetCount(vehicle->GetEntityContext()->GetAmmoCount());
                }
            }

        private:

            void SetThing(BaseThing* thing)
            {
                SetShader(thing->GetInfo()->GetShader());
                SetInitials(thing->GetInfo()->GetInitials());                
            }

            void SetShader(const std::string& shader)
            {
                m_icon_name = shader;
                m_ImageName = m_icon_name.c_str();
            }

            void SetInitials(const std::string& initials)
            {
                m_ammo_info = initials;
                m_AmmoDesc = m_ammo_info.c_str();
            }

            void SetCount(int count)
            {
                std::ostringstream ostr;

                ostr << count;

                m_ammo_count = ostr.str();
		        m_AmmoQuantity = m_ammo_count.c_str();
            }

            void Init()
            {
                m_AmmoDesc = m_ammo_info.c_str();
                m_ImageName = m_icon_name.c_str();
                m_AmmoQuantity = m_ammo_count.c_str();                
            }

        private:
            
            std::string m_icon_name;
            std::string m_ammo_info;
            std::string m_ammo_count;
        };

    public:

        void Visit(HumanEntity* human)
        {
            //точность стрельбы
            if(BaseThing* thing = human->GetEntityContext()->GetThingInHands()){

                //установим картинку оружия
                if(thing->GetInfo()->IsWeapon())
                    SetThing(human, static_cast<WeaponThing*>(thing));
                else if(thing->GetInfo()->IsMedikit())
                    SetThing(human, static_cast<MedikitThing*>(thing));
                else if(thing->GetInfo()->IsGrenade())
                    SetThing(human, static_cast<GrenadeThing*>(thing));
                else if(thing->GetInfo()->IsCamera())
                    SetThing(human, static_cast<CameraThing*>(thing));
                else if(thing->GetInfo()->IsShield())
                    SetThing(human, static_cast<ShieldThing*>(thing));
                else if(thing->GetInfo()->IsScanner())
                    SetThing(human, static_cast<ScannerThing*>(thing));
                else
                    SetNullThing();

                return;
            }
            
            SetNullThing();
        }

        void Visit(VehicleEntity* vehicle)
        {
            GameScreen* screen = Screens::Instance()->Game();

            DisableShotTypeButtons();

            WeaponIconAdapter traits(vehicle);
            screen->SetWeaponIcon(&traits);

            ShotTypeMpsAdapter shoot_traits(vehicle);
            screen->SetShootMovePoints(shoot_traits);
        }

    private:

        void SetThing(HumanEntity* human, WeaponThing* weapon)
        {
            GameScreen* screen = Screens::Instance()->Game();
            shot_type type = human->GetEntityContext()->GetShotType();

            //разрешить кнопки управления оружием
            screen->EnableCBState(GameScreen::CB_AIMSHOT, true);
            screen->EnableCBState(GameScreen::CB_SNAPSHOT, true);
            screen->EnableCBState(GameScreen::CB_AUTOSHOT, weapon->GetInfo()->IsAuto());

            //установить кнопки в соотв сост       
            screen->SetCBState(GameScreen::CB_AIMSHOT, type == SHT_AIMSHOT ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
            screen->SetCBState(GameScreen::CB_AUTOSHOT, type == SHT_AUTOSHOT ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
            screen->SetCBState(GameScreen::CB_SNAPSHOT, type == SHT_SNAPSHOT ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
          
            WeaponIconAdapter thing_traits(weapon);
            screen->SetWeaponIcon(&thing_traits);

            ShotTypeMpsAdapter shoot_traits(human, weapon);
            screen->SetShootMovePoints(shoot_traits);
        }

        void SetThing(HumanEntity* human, BaseThing* thing)
        {
            DisableShotTypeButtons();

            GameScreen* screen = Screens::Instance()->Game();

            WeaponIconAdapter thing_traits(thing);
            screen->SetWeaponIcon(&thing_traits);
            
            ShotTypeMpsAdapter shoot_traits(human, thing);
            screen->SetShootMovePoints(shoot_traits);
        }

        void SetThing(HumanEntity* human, MedikitThing* medikit)
        {
            DisableShotTypeButtons();

            GameScreen* screen = Screens::Instance()->Game();

            WeaponIconAdapter thing_traits(medikit);
            screen->SetWeaponIcon(&thing_traits);

            ShotTypeMpsAdapter shoot_traits(human, medikit);
            screen->SetShootMovePoints(shoot_traits);
        }

        void SetNullThing()
        {
            DisableShotTypeButtons();

            GameScreen* screen = Screens::Instance()->Game();

            screen->SetShootMovePoints(ShotTypeMpsAdapter());
            screen->SetWeaponIcon(static_cast<GameScreen::WeaponIconTraits*>(0));
        }
        
        void DisableShotTypeButtons()
        {
            GameScreen* screen = Screens::Instance()->Game();
 
            screen->EnableCBState(GameScreen::CB_AIMSHOT, false);
            screen->EnableCBState(GameScreen::CB_SNAPSHOT, false);            
            screen->EnableCBState(GameScreen::CB_AUTOSHOT, false);
        }
    };

    //
    // класс для установки совтояния QuickSlot'a
    //
    class QuickSlotSelector : public EntityVisitor{
    public:

        void Visit(HumanEntity* human)
        {
            HumanContext* context = human->GetEntityContext();

            if(!context->GetFastAccessStrategy()->CanMakeFastAccess()){
                DisableQuickSlotButtons();                
                return;
            }
            
            GameScreen* screen  = Screens::Instance()->Game();

            HumanContext::hands_mode mode = context->GetHandsMode();
            
            //установить установить кнопки быстрого доступа к предмету
            screen->EnableCBState(GameScreen::CB_BACKPACK, true);
            screen->EnableButtonState(GameScreen::B_UP_ARROW, mode == HumanContext::HM_FAST_ACCESS);
            screen->EnableButtonState(GameScreen::B_DOWN_ARROW, mode == HumanContext::HM_FAST_ACCESS);
            screen->SetCBState(GameScreen::CB_BACKPACK, mode == HumanContext::HM_HANDS ? GameScreen::CBS_OFF : GameScreen::CBS_ON);
        }
        
        void Visit(TraderEntity* trader)
        { DisableQuickSlotButtons(); }

        void Visit(VehicleEntity* vehicle)
        { DisableQuickSlotButtons(); }

    private:

        void DisableQuickSlotButtons()
        {
            GameScreen* screen = Screens::Instance()->Game();

            screen->EnableCBState(GameScreen::CB_BACKPACK, false);
            screen->EnableButtonState(GameScreen::B_UP_ARROW, false);
            screen->EnableButtonState(GameScreen::B_DOWN_ARROW, false);
        }
    };

    //
    // класс для установки состояния кнопки перезарядки
    //
    class ReloadButtonSelector : public EntityVisitor{
    public:

        void Visit(HumanEntity* human)
        {
            Screens::Instance()->Game()->EnableButtonState(GameScreen::B_RELOAD, GetGameFormImp()->GetReloader()->CanReload(human));
        }

        void Visit(VehicleEntity* vehicle)
        {
            Screens::Instance()->Game()->EnableButtonState(GameScreen::B_RELOAD, false);
        }
    };

    //
    // класс для работы с movepnts при выборе существа
    //
    class MovepntsSelector : public EntityVisitor{
    public:

        void Visit(HumanEntity* human)
        {
            if(VehicleEntity* vehicle = human->GetEntityContext()->GetCrew()){
                AIUtils::CalcAndShowLandField(vehicle);
                return;
            }
            
            AIUtils::CalcAndShowPassField(human);
        }

        void Visit(VehicleEntity* vehicle)
        {
            AIUtils::CalcAndShowPassField(vehicle);
        }
    };

    //
    // кнопки для человека
    //
    class ButtonStateSelector : public EntityVisitor{
    public:

        void Visit(HumanEntity* human)
        {
            GameScreen* game_scr = Screens::Instance()->Game();

            HumanContext* context = human->GetEntityContext();
            GraphHuman*   graph   = human->GetGraph()->Cast2Human();
            
            game_scr->EnableCBState(GameScreen::CB_USE, true);
            game_scr->EnableCBState(GameScreen::CB_SPECIAL, true);
            game_scr->EnableCBState(GameScreen::CB_SIT, context->CanSit());
            game_scr->EnableCBState(GameScreen::CB_RUN, context->CanRun());            

            //отрисовать правильные кнопки
            game_scr->SetCBState(GameScreen::CB_SIT, graph->IsSitPose() ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
            game_scr->SetCBState(GameScreen::CB_RUN, graph->IsRunMove() ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
            game_scr->SetCBState(GameScreen::CB_USE, GetGameFormImp()->CanMakeUse() ? GameScreen::CBS_ON : GameScreen::CBS_OFF);            
            game_scr->SetCBState(GameScreen::CB_SPECIAL, GetGameFormImp()->CanMakeHack() ? GameScreen::CBS_ON : GameScreen::CBS_OFF);

						/*Grom*/
						 static bool rec=false; //защита от повторного входа
						 if(!rec)
							 {
							 if(!context->CanSit())
								 {
								 rec=true;
								 context->SetHandsMode(HumanContext::HM_HANDS);
								 game_scr->EnableCBState(GameScreen::CB_BACKPACK, false);
								 //game_scr->SetCBState(GameScreen::CB_BACKPACK, GameScreen::CBS_OFF);
								 rec=false;
								 }
							 else
								 {
								 game_scr->EnableCBState(GameScreen::CB_BACKPACK, true);
								 if(context->GetHandsMode()==HumanContext::HM_HANDS)
								 game_scr->SetCBState(GameScreen::CB_BACKPACK, GameScreen::CBS_OFF);
								 else
								 game_scr->SetCBState(GameScreen::CB_BACKPACK, GameScreen::CBS_ON);
								 }
							 }
						

        }

        void Visit(VehicleEntity* vehicle)
        {
            GameScreen* game_scr = Screens::Instance()->Game();
         
            game_scr->EnableCBState(GameScreen::CB_SIT, false);
            game_scr->EnableCBState(GameScreen::CB_RUN, false);
            game_scr->EnableCBState(GameScreen::CB_USE, false);
            game_scr->EnableCBState(GameScreen::CB_SPECIAL, false);
        }
    };

    //
    // класс для выделение существа
    //
    class Selector : public EntityVisitor{
    public:
        
        void Visit(TraderEntity* trader)
        {
            throw CASUS("Selecter: попытка выделения торговца!!!");
        }
        
        void Visit(HumanEntity* human)
        {   
            HumanContext* context = human->GetEntityContext(); 

            context->Select(true);

            human->Accept(m_body_selector);
            human->Accept(m_button_selector);
            human->Accept(m_reload_selector);
            human->Accept(m_movepnts_selector);
            human->Accept(m_quick_slot_selector);

            GetGameFormImp()->GetEnemyIcons()->HandleSelectReq(human);
            
            //человек в технике?
            if(context->GetCrew()){
                context->Select(true);
                context->GetCrew()->GetEntityContext()->Select(true);
                return;            
            }
           
            human->Accept(m_weapon_selector); 
        }
        
        void Visit(VehicleEntity* vehicle)
        {
            VehicleContext* context = vehicle->GetEntityContext();

            vehicle->Accept(m_body_selector);
            vehicle->Accept(m_button_selector);
            vehicle->Accept(m_weapon_selector);           
            vehicle->Accept(m_reload_selector);
            vehicle->Accept(m_movepnts_selector);
            vehicle->Accept(m_quick_slot_selector);

            GetGameFormImp()->GetEnemyIcons()->HandleSelectReq(vehicle);
            
            context->Select(true);
        }
        
    private:
        
        BodySelector      m_body_selector;
        WeaponSelector    m_weapon_selector;
        MovepntsSelector  m_movepnts_selector;
        QuickSlotSelector m_quick_slot_selector;

        ReloadButtonSelector m_reload_selector;
        ButtonStateSelector  m_button_selector;
   };
}

//==============================================================

namespace{
    
    class Deselector : public EntityVisitor{
    public:
        
        void Visit(HumanEntity* human)
        {
            HumanContext* contect = human->GetEntityContext();

            //развыделить существо
            if(contect->GetCrew()) contect->GetCrew()->GetEntityContext()->Select(false);
                       
            contect->Select(false);
           
            //очистить человека
            GameScreen::BodyIconTraits icon_traits;
            
            icon_traits.m_Damages = 0;
            
            icon_traits.m_Body  = GameScreen::BodyIconTraits::BPS_NONE;
            icon_traits.m_Hands = GameScreen::BodyIconTraits::BPS_NONE;
            icon_traits.m_Head  = GameScreen::BodyIconTraits::BPS_NONE;
            icon_traits.m_Legs  = GameScreen::BodyIconTraits::BPS_NONE;
            
            Screens::Instance()->Game()->SetBodyIcon(icon_traits);
            
            CommonPart();
        }
        
        void Visit(VehicleEntity* vehicle)
        {
            vehicle->GetEntityContext()->Select(false);
            CommonPart();
        }
        
        void Visit(TraderEntity* trader)
        {
            throw CASUS("Deselecter: попытка развыделить торговца!!!");
        }
        
    private:
        
        void CommonPart()
        {
            AIUtils::HideLandField();
            AIUtils::HidePassField();

            GameScreen* game_scr = Screens::Instance()->Game();
            
            game_scr->SetWeaponIcon(static_cast<GameScreen::WeaponIconTraits*>(0));
            
            game_scr->EnableCBState(GameScreen::CB_BACKPACK, false);
            game_scr->EnableCBState(GameScreen::CB_AIMSHOT, false);
            game_scr->EnableCBState(GameScreen::CB_SNAPSHOT, false);
            game_scr->EnableCBState(GameScreen::CB_AUTOSHOT, false);
            game_scr->EnableCBState(GameScreen::CB_SPECIAL, false);
            game_scr->EnableCBState(GameScreen::CB_SIT,false);
            game_scr->EnableCBState(GameScreen::CB_RUN, false);
            game_scr->EnableCBState(GameScreen::CB_USE, false);
            
            game_scr->EnableButtonState(GameScreen::B_RELOAD, false);
            game_scr->EnableButtonState(GameScreen::B_UP_ARROW, false);
            game_scr->EnableButtonState(GameScreen::B_DOWN_ARROW, false);

            GameScreen::ShootMovePoints shoot_traits;

            shoot_traits.m_AimMP = shoot_traits.m_AutoMP = shoot_traits.m_SnapMP = "";
            game_scr->SetShootMovePoints(shoot_traits);
            
            GetGameFormImp()->GetEnemyIcons()->HandleSelectReq(0);
        }        
    };
}

//==============================================================

namespace{

    class HumanLandingManager : public ActionManager{
    public:

        static HumanLandingManager* GetInst()
        {
            static HumanLandingManager imp;
            return &imp;
        }

        void Init(HumanEntity* hum, const ipnt2_t& hex_pnt)
        {
            m_hex = hex_pnt;
            m_human = hum;
            m_vehicle = m_human->GetEntityContext()->GetCrew();
        }

        Activity* CreateAction()
        { 
            Deselector desel;
            m_human->Accept(desel);

            return ActivityFactory::GetInst()->CreateLanding(m_human, m_hex, ActivityFactory::CT_PLAYER_LANDING); 
        }
        
    private:
        
        HumanLandingManager(){}

    private:

        ipnt2_t    m_hex;
        
        HumanEntity*   m_human;
        VehicleEntity* m_vehicle;
    };
}

//==============================================================

namespace{

    class HumanShipmentManager : public Approach2EntityManager{
    public:

        static HumanShipmentManager* GetInst() 
        {
            static HumanShipmentManager imp;
            return &imp;
        }

        void Init(HumanEntity* hum, VehicleEntity* veh)
        {
            m_human = hum;
            m_vehicle = veh;

            Approach2EntityManager::Init(hum, veh, RT_ACTOR2ENTITY);
        }

        Activity* CreateAction()
        {            
            return ActivityFactory::GetInst()->CreateShipment(m_human, m_vehicle, ActivityFactory::CT_PLAYER_SHIPMENT);
        }

        void Shut()
        {   
            if(m_vehicle->GetPlayer() == PT_PLAYER){
                GetGameFormImp()->HandleSelectReq(m_vehicle);
                GetGameFormImp()->GetTechIcons()->Insert(m_vehicle);
            }else{                       
                GetGameFormImp()->HandleSelectReq(0);         
            }
        }

    private:

        HumanShipmentManager() {}

    private:

        HumanEntity*   m_human;
        VehicleEntity* m_vehicle;
    };
}

//==============================================================

namespace{

    class NullCursorMgr : public CursorManager{
    public:

        void Execute() {}

        bool CanExecute(CursorDevice* device)
        { 
            //SetCursorText("null cursor");
            return false;
        }
    };

    class MenuCursorMgr : public CursorManager{
    public:

        void Execute() {}

        bool CanExecute(CursorDevice* device)
        { 
            CursorDevice::menu_part menu_part = CursorDevice::MP_NONE;
            
            HumanEntity* human = GetGameFormImp()->GetCurEnt() ? GetGameFormImp()->GetCurEnt()->Cast2Human() : 0;

            if(     human == 0 
                ||  !GetGameFormImp()->CanAttack() 
                ||  !device->GetMenuPart(&menu_part)
                ||  menu_part != CursorDevice::MP_WEAPON)
                return false;

            //нужен курсор на лечение?
            if(human->GetEntityContext()->GetThingInHands(TT_MEDIKIT)){
                device->SetCursor(CursorDevice::CT_MEDIC);
                return true;
            }
            
            device->SetCursor(CursorDevice::CT_ATTACK_ENABLE);
            return true;
        }
    };

    class UseCursorMgr : public CursorManager{
    public:

        bool CanExecute(CursorDevice* device)
        { 
            m_human = 0;
            m_ent4use = 0;
            m_object.clear();

            if(GetGameFormImp()->GetCurEnt()) m_human = GetGameFormImp()->GetCurEnt()->Cast2Human();

            if(m_human == 0){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_ent"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            if(device->GetObject(&m_object)){
                return      CanUseObject(m_object, device)
                        &&  CanAccessObject(m_object, device);
            }

            if(m_ent4use = device->GetEntity()) return CanUseEntity(m_ent4use, device);

            device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
            return false;
        }

        void Execute()
        {
            GameFormImp* game_form = GetGameFormImp();
            if(game_form->CanMakeUse()) game_form->HandleSwitchUseReq();

            if(m_ent4use){
                EntityUseManager::GetInst()->Init(m_human, m_ent4use);
                GetGameFormImp()->HandlePlayActionReq(EntityUseManager::GetInst());            
                return;
            }

            if(m_object.size()){
                ObjectUseManager::GetInst()->Init(m_human, m_object);
                GetGameFormImp()->HandlePlayActionReq(ObjectUseManager::GetInst());
                return;
            }
        }

    private:

        bool CanAccessObject(const rid_t& obj, CursorDevice* device)
        {
            pnt_vec_t   obj_hexes;

            DirtyLinks::CalcObjectHexes(m_object, &obj_hexes);
            PathUtils::near_pnt_t near_pnt = PathUtils::GetInst()->GetNearPnt(obj_hexes);
            
            if(near_pnt.IsDefPnt() /*|| near_pnt.m_cost > m_human->GetEntityContext()->GetStepsCount()*/){
                device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_rng"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            return true;
        }

        bool CanUseObject(const rid_t& obj_rid, CursorDevice* device)
        {
            std::string hint;

            if(DirtyLinks::CanUseObject(obj_rid, &hint)){
                device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(DirtyLinks::GetStrRes(hint)));
                return true;
            }           

            if(BaseBox* box = Bureau::GetInst()->Get(obj_rid)){

                if(box->Cast2Info())
                    device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(DirtyLinks::GetStrRes("gf_console")));
                else if(box->Cast2Store())
                    device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(DirtyLinks::GetStrRes("gf_store")));
                
                return true;
            }

            return false;
        }

        bool CanUseEntity(BaseEntity* ent4use, CursorDevice* device)
        {
            if(m_human->GetEID() == ent4use->GetEID())
                return false;

            if(!AIUtils::IsAccessibleEntity(m_human, ent4use)){
                device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_rng"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(ent4use->GetInfo()->GetName()));
            return true;
        }

    private:

        std::string  m_object;
        HumanEntity* m_human;
        BaseEntity*  m_ent4use;
    };

    class MoveCursorMgr : public CursorManager, private EntityVisitor{
    public:

        bool CanExecute(CursorDevice* device)
        { 
            m_device = device;
            m_entity = GetGameFormImp()->GetCurEnt();

            //предположим худшее
            m_device->SetCursor(CursorDevice::CT_MOVE_DISABLE);

            //если нет текущего существа
            if(m_entity == 0){
                m_device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_ent"), CursorDevice::DENIAL_COLOR));
                return false;
            }
               
            //если есть выход за пределы поля
            if(!m_device->GetLevelHex(&m_pnt)){
                m_device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_lvl"), CursorDevice::DENIAL_COLOR));
                return false;            
            }

            //если не проходимая зона
            if(HexGrid::GetInst()->Get(m_pnt).IsDefPnt()){
                m_device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
                return false;
            }

            //проверка специфичная для существа
            m_cost = 0;
            m_entity->Accept(*this);

            if(m_cost){
                m_device->SetCursor(CursorDevice::CT_MOVE_ENABLE, CursorDevice::text(m_cost, m_cost > 0 ? CursorDevice::DEFAULT_COLOR : CursorDevice::DENIAL_COLOR));
                return true;
            }

            return false;
        }

        void Execute()
        {
            EntityMoveManager::GetInst()->Init(GetGameFormImp()->GetCurEnt(), m_pnt);
            GetGameFormImp()->HandlePlayActionReq(EntityMoveManager::GetInst());
        }

    private:

        void Visit(HumanEntity* human)
        {
            HumanContext* context = human->GetEntityContext();

            if(context->GetTraits()->GetWeight() > context->GetLimits()->GetWeight()){
                m_device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_weight_overload"),  CursorDevice::DENIAL_COLOR));
                return;
            }
           
            m_cost = HexGrid::GetInst()->Get(m_pnt).GetCost() * context->GetHexCost();            
            if(m_cost > context->GetTraits()->GetMovepnts()) m_cost = context->GetTraits()->GetMovepnts() - m_cost;
        }

        void Visit(VehicleEntity* vehicle)
        {
            VehicleContext* context = vehicle->GetEntityContext();
            
            m_cost = HexGrid::GetInst()->Get(m_pnt).GetCost() * context->GetHexCost();
            if(m_cost > context->GetMovepnts()) m_cost = context->GetMovepnts() - m_cost;
        }

    private:
        
        ipnt2_t m_pnt;
        int     m_cost;     

        BaseEntity*   m_entity;
        CursorDevice* m_device;
    };

    class SelectCursorMgr : public CursorManager{
    public:

        bool CanExecute(CursorDevice* device)
        { 
            if((m_entity = device->GetEntity()) && m_entity->GetPlayer() == PT_PLAYER){      
                device->SetCursor(CursorDevice::CT_SELECT, CursorDevice::text(m_entity->GetInfo()->GetName()));
                return true;
            }

            return false;
        }

        void Execute() { GetGameFormImp()->HandleSelectReq(m_entity, GameFormImp::SEL_COMMAND); }

    private:

        BaseEntity* m_entity;
    };

    class RotateCursorMgr : public CursorManager, private EntityVisitor{
    public:

        bool CanExecute(CursorDevice* device)
        { 
            m_entity = GetGameFormImp()->GetCurEnt();

            if(m_entity == 0){      
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_ent"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            point3 pnt;

            if(!device->GetTracePnt(&pnt))
                return false;

            m_dir    = pnt - m_entity->GetGraph()->GetPos3(); 
            int cost = AIUtils::CalcMps4EntityRotate(m_entity, m_dir);

            //узнаем кол-во ходов существа
            m_entity->Accept(*this);

            //если не хватает на поворот
            if(cost > m_moves){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_mps"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            device->SetCursor(CursorDevice::CT_MOVE_ENABLE, CursorDevice::text(cost));
            return true;
        }

        void Execute()
        {
            EntityRotateManager::GetInst()->Init(m_entity, m_dir);
            GetGameFormImp()->HandlePlayActionReq(EntityRotateManager::GetInst());
        }

    private:

        void Visit(HumanEntity* human)
        {
            m_moves = human->GetEntityContext()->GetTraits()->GetMovepnts();
        }

        void Visit(VehicleEntity* vehicle)
        {
            m_moves = vehicle->GetEntityContext()->GetMovepnts();
        }

    private:

        int     m_moves;   
        point3  m_dir;

        BaseEntity*   m_entity;
    };

    class LandingCursorMgr : public CursorManager{
    public:

        bool CanExecute(CursorDevice* device)
        { 
            m_human = 0;

            if(GetGameFormImp()->GetCurEnt()) m_human = GetGameFormImp()->GetCurEnt()->Cast2Human();

            if(m_human == 0){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_hum"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            if(m_human->GetEntityContext()->GetCrew() == 0){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_not_in_crew"), CursorDevice::DENIAL_COLOR));
                return false;
            }
           
            if(!device->GetLevelHex(&m_pnt)){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_lvl"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            if(!HexGrid::GetInst()->Get(m_pnt).IsLandPnt()){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_land_zone"), CursorDevice::DENIAL_COLOR));
                return false;
            }
                
            device->SetCursor(CursorDevice::CT_UNLOAD);
            return true;
        }

        void Execute()
        {
            HumanLandingManager::GetInst()->Init(m_human, m_pnt);
            GetGameFormImp()->HandlePlayActionReq(HumanLandingManager::GetInst());
        }

    private:

        ipnt2_t      m_pnt;
        HumanEntity* m_human;
    };

    class ShipmentCursorMgr : public CursorManager{
    public:

        bool CanExecute(CursorDevice* device)
        { 
            m_human = 0;
            m_vehicle = 0;

            if(GetGameFormImp()->GetCurEnt()) m_human = GetGameFormImp()->GetCurEnt()->Cast2Human();

            if(m_human == 0){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_hum"), CursorDevice::DENIAL_COLOR));
                return false;
            }
            
            if(!(m_vehicle = device->GetEntity() ? device->GetEntity()->Cast2Vehicle() : 0 )){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_veh"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            if(!AIUtils::IsAccessibleEntity(m_human, m_vehicle)){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_rng"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            if(!m_vehicle->GetEntityContext()->CanJoinCrew(m_human)){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_full_veh"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            device->SetCursor(CursorDevice::CT_SHIPMENT, CursorDevice::text(m_vehicle->GetInfo()->GetName()));
            return true;
        }

        void Execute()
        {
            HumanShipmentManager::GetInst()->Init(m_human, m_vehicle);
            GetGameFormImp()->HandlePlayActionReq(HumanShipmentManager::GetInst());
        }

    private:

        HumanEntity*   m_human;
        VehicleEntity* m_vehicle;
    };

    class TreatmentCursorMgr : public CursorManager{
    public:

        bool CanExecute(CursorDevice* device)
        { 
            m_sick = 0;
            m_doctor = 0;
             
            if(GetGameFormImp()->GetCurEnt()) m_doctor = GetGameFormImp()->GetCurEnt()->Cast2Human();
            
            if(m_doctor == 0){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_hum"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            MedikitThing* medikit = static_cast<MedikitThing*>(m_doctor->GetEntityContext()->GetThingInHands(TT_MEDIKIT));
            if(medikit == 0){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_medikit"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            if(!(m_sick = device->GetEntity() ? device->GetEntity()->Cast2Human() : 0)){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_ent_4_treat"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            if(m_sick->GetEID() != m_doctor->GetEID() && !AIUtils::IsAccessibleEntity(m_doctor, m_sick)){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_rng"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            HumanContext* sick_context   = m_sick->GetEntityContext();
            HumanContext* doctor_context = m_doctor->GetEntityContext();

            if(sick_context->GetTraits()->GetHealth() >= sick_context->GetLimits()->GetHealth()){
                std::string msg = mlprintf( DirtyLinks::GetStrRes("aiu_not_ill").c_str(), m_sick->GetInfo()->GetName().c_str());
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(msg, CursorDevice::DENIAL_COLOR));
                return false;
            }

            //int mps4treatment = PathUtils::GetInst()->GetNearPnt(m_sick).m_cost*doctor_context->GetHexCost() + medikit->GetInfo()->GetMp2Act();
			//Grom
            int mps4treatment =  medikit->GetInfo()->GetMp2Act();
			if(sick_context!=doctor_context)
				mps4treatment+=PathUtils::GetInst()->GetNearPnt(m_sick).m_cost*doctor_context->GetHexCost();

            if(doctor_context->GetTraits()->GetMovepnts() < mps4treatment){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("aiu_no_mps_for_treat"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            device->SetCursor(CursorDevice::CT_MEDIC,
                CursorDevice::text(m_sick->GetInfo()->GetName()),
                CursorDevice::text(),
                CursorDevice::text(mps4treatment));

            return true;
        }

        void Execute()
        {
            TreatmentManager::GetInst()->Init(m_doctor, m_sick);
            GetGameFormImp()->HandlePlayActionReq(TreatmentManager::GetInst());
        }

    private:

        HumanEntity* m_sick;
        HumanEntity* m_doctor;
    };

    class AttackCursorMgr : public CursorManager, private EntityVisitor{
    public:

        bool CanExecute(CursorDevice* device)
        { 
            m_victim = 0;
            m_entity = 0;
            m_accuracy = 0;
            m_fexecute = false;
            m_object.clear();
            m_device = device;

            //поставить запрещающий курсор
            m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE);

            m_entity = GetGameFormImp()->GetCurEnt();
            if(m_entity == 0){
                m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_ent"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            device->GetTracePnt(&m_to);
            device->GetObject(&m_object);

            if(m_victim = device->GetEntity()){

                if(m_victim->GetEID() == m_entity->GetEID())
                    return false;

                m_to = m_victim->GetGraph()->GetShotPoint(m_to);
            }

            //проверки специфич. для существа
            m_entity->Accept(*this);
            return m_fexecute;
        }

        void Execute()
        {
            GameFormImp* game_form = GetGameFormImp();

            if(game_form->CanAttack()) game_form->HandleSwitchAttackReq();

            if(HumanEntity* human = m_entity->Cast2Human()){

                //проверка: это бросок предмета?
                if(human->GetEntityContext()->GetThingInHands(TT_THROWABLE)){
                    ThingThrowManager::GetInst()->Init(human, m_to);
                    GetGameFormImp()->HandlePlayActionReq(ThingThrowManager::GetInst());        
                    return;
                }
            }

            EntityShootManager::GetInst()->Init(m_entity, m_accuracy, m_to, m_victim ? m_victim->GetEID() : 0, m_object);
            GetGameFormImp()->HandlePlayActionReq(EntityShootManager::GetInst());        
        }

    private:

        void Visit(HumanEntity* human)
        {
            HumanContext* context = human->GetEntityContext();
            AIUtils::reason_type reason = AIUtils::RT_NONE;

            //проверка на оружие
            if(WeaponThing* weapon = static_cast<WeaponThing*>(context->GetThingInHands(TT_WEAPON))){

                if(AIUtils::CanShoot(human, weapon, m_to, &reason)){
                    m_fexecute = true;
                    
                    int accuracy = AIUtils::CalcShootAccuracy(human, weapon, m_to);                    
                    SetShootCursor(accuracy, weapon->GetMovepnts(context->GetShotType()));

                    m_accuracy = AIUtils::NormalizeAccuracy(m_entity, accuracy);
                    return;
                }

                m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE, CursorDevice::text(AIUtils::Reason2Str(reason), CursorDevice::DENIAL_COLOR));
                return;
            }

            //проверка на бросаемый предмет
            if(BaseThing* thing = context->GetThingInHands(TT_THROWABLE)){

                if(AIUtils::CanThrow(human, thing, m_to, &reason)){
                    m_fexecute = true;
                    m_accuracy = 0.0f;
                    SetShootCursor(0, AIUtils::CalcMps2Act(thing));
                    return;
                }

                m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE, CursorDevice::text(AIUtils::Reason2Str(reason), CursorDevice::DENIAL_COLOR));
                return;
            }

            //если в руках есть предмет вывести надпись что им нельзя атаковать
            if(BaseThing* thing = context->GetThingInHands()){
                std::string text = mlprintf(DirtyLinks::GetStrRes("gf_not_for_attack").c_str(), thing->GetInfo()->GetName().c_str());
                m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE, CursorDevice::text(text, CursorDevice::DENIAL_COLOR));
                return;
            }

            m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_weap"), CursorDevice::DENIAL_COLOR));
        }

        void Visit(VehicleEntity* vehicle)
        {
            AIUtils::reason_type reason = AIUtils::RT_NONE;

            if(AIUtils::CanShoot(vehicle, m_to, &reason)){
                m_fexecute = true;

                int accuracy = vehicle->GetInfo()->GetAccuracy();
                SetShootCursor(accuracy, vehicle->GetInfo()->GetMp4Shot());

                m_accuracy = AIUtils::NormalizeAccuracy(m_entity, accuracy);
                return;
            }

            m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE, CursorDevice::text(AIUtils::Reason2Str(reason), CursorDevice::DENIAL_COLOR));
        }

        void SetShootCursor(int accuracy, int movepnts)
        {
            m_device->SetCursor(CursorDevice::CT_ATTACK_ENABLE,
                      CursorDevice::text(m_victim ? m_victim->GetInfo()->GetName() : ""),
                      CursorDevice::text(accuracy), CursorDevice::text(movepnts));
        }

    private:

        point3 m_to;
        rid_t  m_object;
        float  m_accuracy;        

        bool   m_fexecute;
        
        BaseEntity* m_victim;
        BaseEntity* m_entity;

        CursorDevice* m_device;
    };

    class HackCursorMgr : public CursorManager{
    public:

        bool CanExecute(CursorDevice* device)
        {
            m_human = 0;
            m_vehicle = 0;
            m_probability = 0;

            BaseEntity* cur_ent = GetGameFormImp()->GetCurEnt();

            m_human = cur_ent ? cur_ent->Cast2Human() : 0;

            device->SetCursor(CursorDevice::CT_MOVE_DISABLE);

            if(m_human == 0){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_hum"), CursorDevice::DENIAL_COLOR));                
                return false;
            }
            
            m_vehicle = device->GetEntity() ? device->GetEntity()->Cast2Vehicle() : 0;
            if(m_vehicle == 0 || !m_vehicle->GetInfo()->IsRobot()){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_ent_4_hack"), CursorDevice::DENIAL_COLOR));                
                return false;
            }

            if(m_vehicle->GetPlayer() == m_human->GetPlayer()){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_can_not_hack_teammate"), CursorDevice::DENIAL_COLOR));                
                return false;
            }

            if(!AIUtils::IsAccessibleEntity(m_human, m_vehicle)){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_rng"), CursorDevice::DENIAL_COLOR));
                return false;
            }

            HumanContext* human_context = m_human->GetEntityContext();
            int mps4hack = PathUtils::GetInst()->GetNearPnt(m_vehicle).m_cost*human_context->GetHexCost() + MPS_FOR_HACK;

            if(human_context->GetTraits()->GetMovepnts() < mps4hack){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_mps"), CursorDevice::DENIAL_COLOR));                
                return false;
            }
            
            if(human_context->GetTraits()->GetWisdom() < m_vehicle->GetInfo()->GetWisdom()){
                device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_wisdom_4_hack"), CursorDevice::DENIAL_COLOR));                
                return false;
            }

            m_probability = AIUtils::CalcHackProbability(m_human, m_vehicle);

            device->SetCursor(CursorDevice::CT_SPECIAL,
                    CursorDevice::text(m_vehicle->GetInfo()->GetName()),
                    CursorDevice::text(100.0f * m_probability),
                    CursorDevice::text(mps4hack));
            
            return true;
        }

        void Execute()
        {
            GameFormImp* game_form = GetGameFormImp();
            if(game_form->CanMakeHack()) game_form->HandleSwitchHackReq();

            EntityHackManager::GetInst()->Init(m_human, m_vehicle, m_probability);
            GetGameFormImp()->HandlePlayActionReq(EntityHackManager::GetInst());        
        }

    private:

        float  m_probability;

        HumanEntity*   m_human;
        VehicleEntity* m_vehicle;
    };
}

//==============================================================

namespace{

    class UsualCursorAdviser : public CursorAdviser{
    public:

        bool NeedUse()
        { 
            return      Input::KeyState(DIK_LALT)
                    ||  Input::KeyState(DIK_RALT)
                    ||  GetGameFormImp()->CanMakeUse();
        }

        bool NeedHack() 
        {
            return Input::KeyState(DIK_H) || GetGameFormImp()->CanMakeHack();
        }

        bool NeedAttack()
        { 
            return      Input::KeyState(DIK_LCONTROL)
                    ||  Input::KeyState(DIK_RCONTROL)
                    ||  GetGameFormImp()->CanAttack();
        }

        bool NeedRotate()
        { 
            return Input::KeyState(DIK_LSHIFT) || Input::KeyState(DIK_RSHIFT);
        }
    };
}

//==============================================================

namespace{

    class UsualCursorSelector : public CursorSelector, private EntityVisitor{
    public:

        cursor_type Select(CursorDevice* device, CursorAdviser* adviser)
        {  
            //если курсор над интерфейсом
            if(device->GetMenuPart(0)) return CUR_MENU;

            BaseEntity* entity = GetGameFormImp()->GetCurEnt();

            //если в интерфейсе никто не выделен
            if(entity == 0) return CUR_SELECT;

            //итак существо есть => анализ для существа
            if(adviser->NeedRotate()) return CUR_ROTATE;

            m_result  = CUR_NULL;
            m_device  = device;
            m_adviser = adviser;

            entity->Accept(*this);
            return m_result;
        }

    private:

        void Visit(HumanEntity* human)
        {
            HumanContext* context = human->GetEntityContext();


            //если человек в технике курсор на высадку
            if(context->GetCrew()){

                BaseEntity* entity = m_device->GetEntity();

                if(entity && entity->GetPlayer() == PT_PLAYER)
                    m_result = CUR_SELECT;
                else
                    m_result = CUR_LANDING;

                return;
            }

            //обработка курсора атаки
            if(m_adviser->NeedAttack()){
                m_result =      context->GetThingInHands(TT_MEDIKIT) 
                            ?   CUR_TREATMENT  //курсор лечения если в руках аптечка
                            :   CUR_ATTACK;    //курсор атаки в любом другом случае            
                return;
            }

            //если нужен use их есть у нас
            if(m_adviser->NeedUse()){
                m_result = CUR_USE;
                return;
            }

            //если есть курсор взлома
            if(m_adviser->NeedHack()){
                m_result = CUR_HACK;
                return;
            }
            //если курсор указывает на существо
            if(BaseEntity* entity = m_device->GetEntity()){

                //если это мы и есть => выход
                if(entity->GetEID() == human->GetEID())
                    return;
                
                //если враг - атакуем
                if(EnemyDetector::getInst()->isHeEnemy4Me(human, entity)){
                    m_result = CUR_ATTACK;
                    return;
                }
                
                if(VehicleEntity* vehicle = entity->Cast2Vehicle()){
                    
                    //если наш робот, мы можем его выделить
                    if(vehicle->GetInfo()->IsRobot() && vehicle->GetPlayer() == PT_PLAYER){
                        m_result = CUR_SELECT;
                        return;
                    }
                    
                    //если наша или ничейная техника попробуем в нее залезть
                    if(vehicle->GetInfo()->IsTech() && vehicle->GetPlayer() & (PT_NONE|PT_PLAYER|PT_CIVILIAN)){                        
                        m_result = CUR_SHIPMENT;
                        return;
                    }
                }
                
                //если наш => выделение
                if(entity->GetPlayer() == PT_PLAYER){
                    m_result = CUR_SELECT;
                    return;
                }
                
                //на всех остальныx USE
                m_result = CUR_USE;
                return;
            }

            //если курсор указывает на HexGrid
            if(m_device->GetLevelHex(0)){
                m_result = CUR_MOVE;
                return;
            }

            //если курсор указывает на объект
            std::string object;
            if(m_device->GetObject(&object)){
                m_result = DirtyLinks::IsElevatorObj(object) ? CUR_MOVE : CUR_USE;
                return;
            }
        }

        void Visit(VehicleEntity* vehicle)
        {
            if(m_adviser->NeedAttack()){
                m_result = CUR_ATTACK;
                return;
            }

            //если курсор указывет на сущетсво
            if(BaseEntity* entity = m_device->GetEntity()){

                //если это мы и есть выход
                if(entity->GetEID() == vehicle->GetEID())
                    return;
                
                //если враг - атакуем
                if(EnemyDetector::getInst()->isHeEnemy4Me(vehicle, entity)){
                    m_result = CUR_ATTACK;
                    return;
                }
                
                //eсли наш - выделяем
                if(entity->GetPlayer() == PT_PLAYER){
                    m_result = CUR_SELECT;
                    return;
                }

                return;
            }

            //если курсор указывает на поле
            if(m_device->GetLevelHex(0)){
                m_result = CUR_MOVE;
                return;
            }
        }

        void Visit(TraderEntity* trader)
        {
        }

    private:

        cursor_type    m_result;
        CursorDevice*  m_device;
        CursorAdviser* m_adviser;
    };
}

//==============================================================

namespace{

    //
    // реализация мышиного устройства
    //
    class CursorDeviceImp : public CursorDevice{
    public:

        CursorDeviceImp() : m_flags(0), m_cursor(CT_NORMAL), m_entity(0), m_menu_part(MP_NONE)
        {
            //если курсор над интерфейсом выход
            if(Screens::Instance()->Game()->FromPoint(Input::MouseState().x, Input::MouseState().y)){
                m_flags |= F_MENU_PART;
                if(Screens::Instance()->Game()->IsCursorOnWeaponIcon()) m_menu_part = MP_WEAPON;
                return;            
            }
                  
            unsigned trace_flags =  ShotTracer::F_VISROOFS|ShotTracer::F_SKIP_INVISIBLE_ENTS
                                   |ShotTracer::F_SKIP_SHIELDS|ShotTracer::F_ENTITIES_AS_BOX; 
         
            ShotTracer tracer(0, DirtyLinks::GetCamPos(), DirtyLinks::PickCam(Input::MouseState().x, Input::MouseState().y), 0, trace_flags);

            m_flags |= F_TRACE_PNT;
            m_trace_pnt = tracer.GetEnd();

            switch(tracer.GetMaterial()){
            case ShotTracer::MT_AIR:
            case ShotTracer::MT_SHIELD:
                break;

            case ShotTracer::MT_WALL:
                if(CalcLevelHex(m_trace_pnt, &m_hex_pnt))
                    m_flags |= F_LEVEL_HEX;
                break;
            
            case ShotTracer::MT_ENTITY:
                m_flags |= F_ENTITY;
                m_entity = EntityPool::GetInst()->Get(tracer.GetEntity());
                break;
            
            case ShotTracer::MT_OBJECT:
                m_flags |= F_OBJECT;
                m_object = tracer.GetObject();

                if(     DirtyLinks::IsElevatorObj(m_object)
                    &&  CalcLevelHex(m_trace_pnt, &m_hex_pnt))
                    m_flags |= F_LEVEL_HEX;
                break;
            }
        }

        ~CursorDeviceImp()
        {
            MouseCursor::CURSOR_TYPE type = MouseCursor::CT_NORMAL;

            switch(m_cursor){
            case CT_USE: type = MouseCursor::CT_CANUSE; break; 
            case CT_MEDIC: type = MouseCursor::CT_MEDIC; break; 
            case CT_SELECT: type = MouseCursor::CT_SELECT; break;
            case CT_UNLOAD: type = MouseCursor::CT_UNLOAD; break;
            case CT_NORMAL: type = MouseCursor::CT_NORMAL; break;
            case CT_PICKUP: type = MouseCursor::CT_PICKUP; break;
            case CT_SPECIAL: type = MouseCursor::CT_SPECIAL; break;
            case CT_SHIPMENT: type = MouseCursor::CT_SHIPMENT; break;
            case CT_MOVE_ENABLE: type = MouseCursor::CT_CANMOVE; break;
            case CT_MOVE_DISABLE: type = MouseCursor::CT_CANNOTMOVE; break;
            case CT_ATTACK_ENABLE: type = MouseCursor::CT_CANATTACK; break;
            case CT_ATTACK_DISABLE: type = MouseCursor::CT_CANNOTATTACK; break;
            }

            MouseCursor::SetCursor(type, m_text[0].IsEmpty() ? 0 : &m_text[0],
                                         m_text[1].IsEmpty() ? 0 : &m_text[1],
                                         m_text[2].IsEmpty() ? 0 : &m_text[2]);
        }

        BaseEntity* GetEntity() const
        { 
            return m_flags & F_ENTITY ? m_entity : 0; 
        }

        bool GetObject(rid_t* str) const
        {
            if(m_flags & F_OBJECT){
                if(str) *str = m_object;
                return true;
            }

            return false;
        }

        bool GetTracePnt(point3* pnt) const 
        {
            if(m_flags & F_TRACE_PNT){
                if(pnt) *pnt = m_trace_pnt;
                return true;
            }

            return false;
        }

        bool GetLevelHex(ipnt2_t* pnt) const
        {
            if(m_flags & F_LEVEL_HEX){
                if(pnt) *pnt = m_hex_pnt;
                return true;
            }

            return false;
        }

        bool GetMenuPart(menu_part* part) const
        {
            if(m_flags & F_MENU_PART){
                if(part) *part = m_menu_part;
                return true;
            }

            return false;
        }

        void SetCursor(cursor_type type, const text& t1, const text& t2, const text& t3)
        {
            m_cursor = type;

            m_text[0].Adapt(t1);
            m_text[1].Adapt(t2);
            m_text[2].Adapt(t3);
        }

    private:

        bool CalcLevelHex(const point3& pnt3, ipnt2_t* pnt2)
        {
            ipnt2_t ret;
            
            if(!HexGrid::GetInst()->IsOutOfRange(ret = HexUtils::scr2hex(pnt3))){
                *pnt2 = ret;
                return true;
            }

            return false;
        }

    private:

        struct TxtFldAdapter : public MouseCursor::TextField{

            bool IsEmpty() const { return m_text.empty(); }

            void Adapt(const text& text)
            {
                m_Color = text.m_color;
                m_text = text.m_text;
                m_Text = m_text.c_str();
            }

        private:

            std::string m_text;
        };
        
        TxtFldAdapter m_text[3];   
        cursor_type   m_cursor;

        rid_t       m_object;
        BaseEntity* m_entity;
        ipnt2_t     m_hex_pnt;
        menu_part   m_menu_part;
        point3      m_trace_pnt;

        enum flag_type{
            F_OBJECT    = 1 << 0,
            F_ENTITY    = 1 << 1,
            F_LEVEL_HEX = 1 << 2,
            F_MENU_PART = 1 << 3,
            F_TRACE_PNT = 1 << 4,
        };
        
        unsigned m_flags;
    };
}

//==============================================================

SmartCursor::SmartCursor() :
    m_requests(0), m_current(0), m_upd_time(0), m_click_time(0),
    m_adviser(new UsualCursorAdviser()), m_selector(new UsualCursorSelector())
{
    m_cursors[CursorSelector::CUR_USE] = new UseCursorMgr();
    m_cursors[CursorSelector::CUR_HACK] = new HackCursorMgr();
    m_cursors[CursorSelector::CUR_NULL] = new NullCursorMgr();
    m_cursors[CursorSelector::CUR_MOVE] = new MoveCursorMgr();
    m_cursors[CursorSelector::CUR_MENU] = new MenuCursorMgr();
    m_cursors[CursorSelector::CUR_SELECT] = new SelectCursorMgr();
    m_cursors[CursorSelector::CUR_ROTATE] = new RotateCursorMgr();
    m_cursors[CursorSelector::CUR_ATTACK] = new AttackCursorMgr();
    m_cursors[CursorSelector::CUR_LANDING] = new LandingCursorMgr();
    m_cursors[CursorSelector::CUR_SHIPMENT] = new ShipmentCursorMgr();
    m_cursors[CursorSelector::CUR_TREATMENT] = new TreatmentCursorMgr();
}

SmartCursor::~SmartCursor()
{
    delete m_adviser;
    delete m_selector;

    for(int k = 0; k < CursorSelector::MAX_CURSORS; delete m_cursors[k++]);
}

void SmartCursor::SetUsual()
{
    MouseCursor::SetCursor(MouseCursor::CT_NORMAL);
}

void SmartCursor::Update()
{    
    if(m_requests || m_upd_time < Timer::GetSeconds()){

        m_current  = 0;
        m_requests = 0;
        m_upd_time = Timer::GetSeconds() + DirtyLinks::GetFloatOpt(DirtyLinks::OT_MOUSE_UPDATE_TIME);

        CursorDeviceImp cursor_device;

        m_current = m_cursors[m_selector->Select(&cursor_device, m_adviser)];
        if(!m_current->CanExecute(&cursor_device)) m_current = 0;
    }

    //нажатие левой кнопки мыши
    if(m_current && IsLeftMouseButtonClick()){
        //запомним время последнего click'а
        m_click_time = Timer::GetSeconds();
        //выполним комманду курсора
        m_current->Execute();
    }
}

//==============================================================

void Walkers::info_s::MakeSaveLoad(SavSlot& slot)
{
    if(slot.IsSaving()){

        if(m_entity){
            
            slot << m_entity->GetEID();
            slot << m_point.x << m_point.y;

        }else{

            slot << static_cast<eid_t>(0);
        }

    }else{

        eid_t eid;
        slot >> eid;

        if(eid){
            m_entity = EntityPool::GetInst()->Get(eid);
            slot >> m_point.x >> m_point.y;
        }
    }
}

bool Walkers::info_s::IsSame(BaseEntity* entity) const
{
    return m_entity && m_entity->GetEID() == entity->GetEID();
}

bool Walkers::info_s::NeedMove() const
{
    return m_entity && m_point != m_entity->GetGraph()->GetPos2(); 
}

Walkers::Walkers()
{
    Spawner::GetInst()->Attach(this, ET_PREPARE_SPAWN);
}

Walkers::~Walkers()
{
    Spawner::GetInst()->Detach(this);
    for(int k = 0; k < MAX_TEAMMATES; Remove(m_infos[k++]));
}

int Walkers::Ent2Index(BaseEntity* entity) const
{
    for(int k = 0; k < MAX_TEAMMATES; k++){
        if(m_infos[k].IsSame(entity))
            return k;
    }

    return -1;
}

bool Walkers::IsGoing(BaseEntity* ent) const
{
    return Ent2Index(ent) >= 0;
}

void Walkers::Insert(BaseEntity* entity, const ipnt2_t& to)
{
    int index = Ent2Index(entity);

    //попробуем найти такое же существо и подправить точку
    if(index >= 0){
        m_infos[index].m_point = to;
        return;
    }

    //вставим это сущетво в списки
    for(int k = 0; k < MAX_TEAMMATES; k++){

        if(m_infos[k].IsEmpty()){

            m_infos[k].m_point  = to;
            m_infos[k].m_entity = entity;

            //поставить наблюдателя
            LinkObserver(entity);

            entity->Notify(EntityObserver::EV_INGOING_CHANGE);
            return;
        }
    }
}

void Walkers::LinkObserver(BaseEntity* entity)
{
    if(entity == 0) return;

    entity->Attach(this, EV_DEATH_PLAYED);
    entity->Attach(this, EV_ACTIVITY_INFO);
}

void Walkers::Update(Spawner* spawn, SpawnObserver::event_t event, SpawnObserver::info_t info)
{
    for(int k = 0; k < MAX_TEAMMATES; Remove(m_infos[k++]));
}

void Walkers::Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
{
    //сообщения о конце действия на не интересны
    if(event == EV_ACTIVITY_INFO && static_cast<activity_info_s*>(info)->IsEnd())
        return;

    int index = Ent2Index(entity);

    if(index < 0) throw CASUS("Walkers: работа с существом за которым не наблюдаем!");

    Remove(m_infos[index]);    
}

void Walkers::Remove(info_s& info)
{
    if(BaseEntity* entity = info.m_entity){
        
        info.m_entity = 0;
        
        entity->Detach(this);    
        entity->Notify(EntityObserver::EV_INGOING_CHANGE);
    }
}

void Walkers::MakeSaveLoad(SavSlot& slot)
{
   for(int k = 0; k < MAX_TEAMMATES; m_infos[k++].MakeSaveLoad(slot));
   
   if(!slot.IsSaving()){       

       //восстановить наблюдателей
       for(int k = 0; k < MAX_TEAMMATES; LinkObserver(m_infos[k++].m_entity));
   }  
}

ActionManager* Walkers::CreateAction(BaseEntity* entity)
{
    if(entity){
        int index = Ent2Index(entity);
        return index >= 0 ? CreateAction(m_infos[index]) : 0;
    }

    for(int k = 0; k < MAX_TEAMMATES; k++){

        if(m_infos[k].IsEmpty())
            continue;

        if(ActionManager* action = CreateAction(m_infos[k]))
            return action;        
    }

    return 0;
}

ActionManager* Walkers::CreateAction(info_s& info)
{
    if(HumanEntity* human = info.m_entity->Cast2Human()){

        if(human->GetEntityContext()->GetStepsCount() == 0)
            return 0;    
    }

    if(VehicleEntity* vehicle = info.m_entity->Cast2Vehicle()){

        if(vehicle->GetEntityContext()->GetStepsCount() == 0)
            return 0;    
    }

    //инициализируем менеджер перемещения
    EntityMoveManager::GetInst()->Init(info.m_entity, info.m_point, EntityMoveManager::F_WALKERS_MOVE);

    //удаляемся из списков
    Remove(info);

    //идем
    return EntityMoveManager::GetInst();
}

//==============================================================

namespace{

    GameScreen::PersonIconMgr* GetPersonIcons()
    {
        return Screens::Instance()->Game()->GetPersonIconMgr();
    }
}

typedef GameScreen::PersonIconMgr::Icon human_icon_t;
#define hi_prefix GameScreen::PersonIconMgr::MonitorTraits

HumanIcons::HumanIcons()
{
    GetPersonIcons()->RemoveAll();

    Spawner::GetInst()->Attach(this, ET_PREPARE_SPAWN);
    Spawner::GetInst()->Attach(this, ET_ENTITY_SPAWN);
}

HumanIcons::~HumanIcons()
{
    Spawner::GetInst()->Detach(this);

    EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);

    while(itor != EntityPool::GetInst()->end()){
        itor->Detach(this);
        ++itor;
    }
}

HumanEntity* HumanIcons::Icon2Entity(int index)
{
    human_icon_t* icon = GetPersonIcons()->IconAt(index);
    return icon ? icon->GetHumanEntity() : 0;
}

void HumanIcons::Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info)
{
    switch(type){
    case ET_PREPARE_SPAWN:
        GetPersonIcons()->RemoveAll();
        break;

    case ET_ENTITY_SPAWN:
        {
            
            HumanEntity* human =    static_cast<spawn_info_s*>(info)->m_entity
                                ?   static_cast<spawn_info_s*>(info)->m_entity->Cast2Human()
                                :   0;
            
            if(human && human->GetPlayer() == PT_PLAYER) Insert(human, human->GetInfo()->IsHacker());
        }
        break;
    }
}

void HumanIcons::Insert(HumanEntity* human, bool insert2front)
{
    //если вставка в начало списка
    if(insert2front){

        typedef std::list<HumanEntity*> human_list_t;
        human_list_t hlst;

        hlst.push_back(human);

        for(int k = 0; k < MAX_TEAMMATES; k++){
            if(HumanEntity* hum = Icon2Entity(k))
                hlst.push_back(hum);
        }

        //чистим список
        GetPersonIcons()->RemoveAll();

        //вставляем в список все по новой
        human_list_t::iterator itor = hlst.begin();
        while(itor != hlst.end()) Insert(*(itor++), false);     
        
        return;
    }

    human_icon_t  icon(human);
    HumanContext* context = human->GetEntityContext();

    //зарегистировать наблюдателя
    LinkObserver(human);

    //установим свойства иконки
    icon.SetStateFlags(GetHumanFlags(human));
    icon.SetHealth(context->GetTraits()->GetHealth());
    icon.SetMovePoints(context->GetTraits()->GetMovepnts());
    icon.SetHeartbeat(static_cast<hi_prefix::HEARTBEAT>(GetHumanHeartbeat(human)));
    icon.SetPortrait(human->GetInfo()->GetRID().c_str(), context->IsSelected() ? hi_prefix::ST_NORMAL : hi_prefix::ST_NONE);

    //вставим иконку
    GetPersonIcons()->InsertIcon(icon);
}

int HumanIcons::GetHumanHeartbeat(HumanEntity* human)
{
    HumanContext* context = human->GetEntityContext();

    if(context->GetTraits()->GetHealth() < m_dead_lvl)
        return hi_prefix::HB_DEAD;
    
    if(static_cast<float>(context->GetTraits()->GetHealth())/static_cast<float>(context->GetLimits()->GetHealth()) < m_wound_lvl)
        return hi_prefix::HB_WOUND;

    return hi_prefix::HB_CALM;
}
   
unsigned HumanIcons::GetHumanFlags(HumanEntity* human)
{
    HumanContext* context = human->GetEntityContext();

    unsigned flags = 0;

    if(context->GetCrew()) flags |= hi_prefix::IF_INVEHICLE;
    if(context->GetPanicType() != HPT_NONE) flags |= hi_prefix::IF_INPANIC;
    if(context->GetTraits()->GetLevelupPoints()) flags |= hi_prefix::IF_LEVELUP;
    if(GetGameFormImp()->GetWalkers()->IsGoing(human)) flags |= hi_prefix::IF_INMOVEMENT;

    return flags;
}

void HumanIcons::LinkObserver(HumanEntity* human)
{
    human->Attach(this, EV_QUIT_TEAM);
    human->Attach(this, EV_CREW_CHANGE);
    human->Attach(this, EV_PANIC_CHANGE);
    human->Attach(this, EV_PREPARE_DEATH);
    human->Attach(this, EV_HEALTH_CHANGE);
    human->Attach(this, EV_INGOING_CHANGE);
    human->Attach(this, EV_LEVELUP_CHANGE);
    human->Attach(this, EV_MOVEPNTS_CHANGE);
    human->Attach(this, EV_SELECTION_CHANGE);
}

void HumanIcons::Remove(HumanEntity* human, remove_type type)
{
    human->Detach(this);

    GameScreen::PersonIconMgr::REMOVE_TYPE rt =     type == RT_DEATH
                                                ?   GameScreen::PersonIconMgr::RT_SLOW
                                                :   GameScreen::PersonIconMgr::RT_QUICK;

    human_icon_t* icon = GetPersonIcons()->IconAt(human);
    if(icon && type == RT_DEATH) icon->SetPortrait(human->GetInfo()->GetRID().c_str(), hi_prefix::ST_DEAD);
    
    GetPersonIcons()->RemoveIcon(human, rt);
}

void HumanIcons::Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
{
    HumanEntity* human = entity->Cast2Human();

    switch(event){
    case EV_QUIT_TEAM:
        if(human->GetPlayer() != PT_PLAYER) Remove(human);
        break;

    case EV_CREW_CHANGE:
    case EV_PANIC_CHANGE:
    case EV_INGOING_CHANGE:
    case EV_LEVELUP_CHANGE:
        if(human_icon_t* icon = GetPersonIcons()->IconAt(human))
            icon->SetStateFlags(GetHumanFlags(human));
        break;

    case EV_PREPARE_DEATH:
        Remove(human, RT_DEATH);
        break;

    case EV_HEALTH_CHANGE:
        if(human_icon_t* icon = GetPersonIcons()->IconAt(human)){
            icon->SetHealth(human->GetEntityContext()->GetTraits()->GetHealth());        
            icon->SetHeartbeat(static_cast<hi_prefix::HEARTBEAT>(GetHumanHeartbeat(human)));
        }
        break;

    case EV_MOVEPNTS_CHANGE:
        if(human_icon_t* icon = GetPersonIcons()->IconAt(human))
            icon->SetMovePoints(human->GetEntityContext()->GetTraits()->GetMovepnts());
        break;

    case EV_SELECTION_CHANGE:
        if(human_icon_t* icon = GetPersonIcons()->IconAt(human)){

            hi_prefix::SEL_TYPE selection =     human->GetEntityContext()->IsSelected()
                                            ?   hi_prefix::ST_NORMAL : hi_prefix::ST_NONE;        

            icon->SetPortrait(human->GetInfo()->GetRID().c_str(), selection);
        }
        break;
    }
}

void HumanIcons::MakeSaveLoad(SavSlot& slot)
{
    if(slot.IsSaving()) return;

    GetPersonIcons()->RemoveAll();
    
    EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);
    
    while(itor != EntityPool::GetInst()->end()){
        Insert(itor->Cast2Human(), itor->Cast2Human()->GetInfo()->IsHacker());
        ++itor;
    }   
}

int HumanIcons::Entity2Icon(HumanEntity* human)
{
    HumanIcons* icons = GetGameFormImp()->GetHumanIcons();
    
    for(int k = 0; k < MAX_TEAMMATES && human; k++){
        if(human == icons->Icon2Entity(k))
            return k;
    }

    return -1;
}

#undef hi_prefix

//==============================================================

TechIcons::TechIcons()
{
    Spawner::GetInst()->Attach(this, ET_PREPARE_SPAWN);
    Spawner::GetInst()->Attach(this, ET_ENTITY_SPAWN);
}

TechIcons::~TechIcons()
{
    Spawner::GetInst()->Detach(this);

    for(int k = 0; k < MAX_TEAMMATES; k++){
        if(m_icons[k].m_vehicle) m_icons[k].m_vehicle->Detach(this);
    }    
}

void TechIcons::Update()
{
    for(int k = 0; k < MAX_TEAMMATES; k++){
        if(m_icons[k].m_requests){
            DrawIcon(k, m_icons[k].m_vehicle);
            m_icons[k].m_requests = 0;
        }
    }
}

void TechIcons::Insert(VehicleEntity* vehicle)
{
    //найдем первый свободный слот и вставим технику
    for(int k = 0; k < MAX_TEAMMATES; k++){

        if(!m_icons[k].m_vehicle){
            
            //пропишемся в массив иконок
            m_icons[k].m_vehicle  = vehicle;
            m_icons[k].m_requests++;

            //установим наблюдателя
            LinkObserver(vehicle);
            break;
        }

        if(m_icons[k].m_vehicle->GetEID() == vehicle->GetEID()){
            m_icons[k].m_requests++;
            return;
        }
    }
}

void TechIcons::LinkObserver(BaseEntity* vehicle)
{
    vehicle->Attach(this, EV_QUIT_TEAM);
    vehicle->Attach(this, EV_CREW_CHANGE);
    vehicle->Attach(this, EV_DEATH_PLAYED);
    vehicle->Attach(this, EV_HEALTH_CHANGE);
    vehicle->Attach(this, EV_INGOING_CHANGE);
    vehicle->Attach(this, EV_MOVEPNTS_CHANGE);
    vehicle->Attach(this, EV_SELECTION_CHANGE);   
}

void TechIcons::Remove(VehicleEntity* vehicle)
{
    //удалим технику
    for(int k = 0; k < MAX_TEAMMATES; k++){
        if(m_icons[k].m_vehicle->GetEID() == vehicle->GetEID()){

            //снимем набюдателя
            vehicle->Detach(this);

            //передвинем все вверх
            for(int z = k+1; z < MAX_TEAMMATES; z++){
                m_icons[z-1] = m_icons[z];
                m_icons[z-1].m_requests++;
            }

            m_icons[MAX_TEAMMATES - 1].m_vehicle  = 0;
            m_icons[MAX_TEAMMATES - 1].m_requests ++;
            break;
        }
    }
}

VehicleEntity* TechIcons::Icon2Entity(unsigned icon)
{
    if(icon >= MAX_TEAMMATES) return 0;
    return m_icons[icon].m_vehicle;
}

int TechIcons::Entity2Icon(VehicleEntity* vehicle)
{
    for(int k = 0; k < MAX_TEAMMATES; k++){
        if(     m_icons[k].m_vehicle
            &&  m_icons[k].m_vehicle->GetEID() == vehicle->GetEID())
            return k;
    }

    return -1;
}

void TechIcons::DrawIcon(int icon_id, VehicleEntity* vehicle)
{
    //если все нормально - выход
    GameScreen::VehicleIconTraits icon_traits;
    
    icon_traits.m_Visible = vehicle != 0;
    
    if(vehicle){
        
        VehicleContext* context  = vehicle->GetEntityContext();

        icon_traits.m_HitPoints  = context->GetHealth();
        icon_traits.m_MaxCrew    = vehicle->GetInfo()->GetCapacity();
        icon_traits.m_NumCrew    = context->GetCrewSize();
        icon_traits.m_MovePoints = context->GetMovepnts();
        icon_traits.m_Selection  = context->IsSelected() ? GameScreen::VehicleIconTraits::ST_NORMAL : GameScreen::VehicleIconTraits::ST_NONE;

        icon_traits.m_Flags      = 0; 
        if(GetGameFormImp()->GetWalkers()->IsGoing(vehicle)) icon_traits.m_Flags |= GameScreen::VehicleIconTraits::VIF_INMOVEMENT;
    }

    Screens::Instance()->Game()->SetVehicleIcon(icon_id, icon_traits);
}

void TechIcons::Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info)
{
    switch(type){
    case ET_PREPARE_SPAWN:
        {
            for(int k = 0; k < MAX_TEAMMATES; k++){
                m_icons[k].m_vehicle  = 0;
                m_icons[k].m_requests ++;
            }
        }
        break;

    case ET_ENTITY_SPAWN:
        {
            VehicleEntity* vehicle =    static_cast<spawn_info_s*>(info)->m_entity
                ?  static_cast<spawn_info_s*>(info)->m_entity->Cast2Vehicle()
                :  0;
            
            if(     vehicle
                &&  vehicle->GetInfo()->IsTech()
                &&  vehicle->GetPlayer() == PT_PLAYER)
                Insert(vehicle);
        }
        break;
    }

}

void TechIcons::Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
{
    switch(event){
    case EV_QUIT_TEAM:
        if(entity->GetPlayer() != PT_PLAYER)
            Remove(entity->Cast2Vehicle());
        break;

    case EV_DEATH_PLAYED:
        Remove(entity->Cast2Vehicle());
        break;

    default:
        {
            int icon = Entity2Icon(entity->Cast2Vehicle());
            if(icon >= 0) m_icons[icon].m_requests++;
        }
    };
}

void TechIcons::MakeSaveLoad(SavSlot& slot)
{
    if(slot.IsSaving()){
        
        for(int k = 0; k < MAX_TEAMMATES; k++){
            slot << (m_icons[k].m_vehicle ? m_icons[k].m_vehicle->GetEID() : 0);
        }

    }else{

        eid_t eid;

        for(int k = 0; k < MAX_TEAMMATES; k++){
            slot >> eid;

            if(eid){
                m_icons[k].m_vehicle = EntityPool::GetInst()->Get(eid)->Cast2Vehicle();
                LinkObserver(m_icons[k].m_vehicle);
            }

            m_icons[k].m_requests++;
        }
    }
}

//==============================================================

EnemyIcons::EnemyIcons() : m_current(0)
{
    AttachObservers();

    //заполним массив иконок с врагами
    Screens::Instance()->Game()->GetEnemyIconMgr()->Clear();
    VisMap::marker_itor itor = VisMap::GetInst()->marker_begin(PlayerSpectator::GetInst()->GetSpectator(), VT_VISIBLE);

    while(itor != VisMap::GetInst()->marker_end()){
        Insert(&*itor);
        ++itor;
    }
}

EnemyIcons::~EnemyIcons()
{
   DetachObservers();
}

void EnemyIcons::AttachObservers()
{
    //предотвращаем двойное подключение
    DetachObservers();

    Spawner::GetInst()->Attach(this, ET_PREPARE_SPAWN);

    //присоеденимся к наблюдателю за командой
    Spectator* spectator = PlayerSpectator::GetInst()->GetSpectator();
    
    spectator->Attach(this, EV_VISIBLE_MARKER);
    spectator->Attach(this, EV_INVISIBLE_MARKER);    
    
    EnemyDetector::getInst()->attach(this, ET_RELATION2PLAYER_CHANGED);
}

void EnemyIcons::DetachObservers()
{
    Spawner::GetInst()->Detach(this);
    EnemyDetector::getInst()->detach(this);
    PlayerSpectator::GetInst()->GetSpectator()->Detach(this);
    
    if(m_current) m_current->GetEntityContext()->GetSpectator()->Detach(this);
}

void EnemyIcons::HandleSelectReq(BaseEntity* entity)
{
    if(m_current) m_current->GetEntityContext()->GetSpectator()->Detach(this);

    //сбросим старые флаги
    Screens::Instance()->Game()->GetEnemyIconMgr()->SetFlags(0);
    
    if(m_current = entity){

        Spectator* spectator = m_current->GetEntityContext()->GetSpectator();
        
        spectator->Attach(this, EV_VISIBLE_MARKER);
        spectator->Attach(this, EV_INVISIBLE_MARKER);
        
        //установим новые
        VisMap::marker_itor itor = VisMap::GetInst()->marker_begin(entity->GetEntityContext()->GetSpectator(), VT_VISIBLE);            
        
        while(itor != VisMap::GetInst()->marker_end()){
            SetFlags(m_current, &*itor);
            ++itor;
        }            
    }
}

void EnemyIcons::Update(Spawner* spawner, SpawnObserver::event_t event, SpawnObserver::info_t info)
{
    AttachObservers();
    Screens::Instance()->Game()->GetEnemyIconMgr()->Clear();
}

void EnemyIcons::Update(Spectator* spectator, SpectatorObserver::event_t event, SpectatorObserver::info_t info)
{
    BaseEntity* entity = spectator->GetEntity();
    Marker* marker = static_cast<marker_info*>(info)->m_marker;

    if(entity && m_current && entity->GetEID() == m_current->GetEID()){

        SetFlags(entity, marker);        

    }else{

        if(event == EV_VISIBLE_MARKER) Insert(marker);
        if(event == EV_INVISIBLE_MARKER) Remove(marker);    
    }
}

void EnemyIcons::changed(RelationsObserver::event_type et, void* info)
{
    BaseEntity* entity = EntityPool::GetInst()->Get(static_cast<EntityDesc*>(info)->m_id);

    unsigned flags = VisMap::GetInst()->GetVisFlags(PlayerSpectator::GetInst()->GetSpectator(),
                                                    entity->GetEntityContext()->GetMarker());
    
    if(flags & VT_VISIBLE){

        if(IsEnemy4Player(entity))
            Insert(entity->GetEntityContext()->GetMarker());
        else
            Remove(entity->GetEntityContext()->GetMarker());
    }
}

void EnemyIcons::Insert(Marker* marker)
{
    BaseEntity* entity = marker->GetEntity();
    GameScreen::EnemyIconMgr* icon_mgr = Screens::Instance()->Game()->GetEnemyIconMgr();
    
    if(entity && icon_mgr->ItemAt(marker) == 0 && IsEnemy4Player(entity)){            
        icon_mgr->Insert(GameScreen::EnemyIconMgr::EnemyIcon(entity->GetInfo()->GetName(), marker));        
        SetFlags(m_current, marker);
    }
}

void EnemyIcons::Remove(Marker* marker)
{
    Screens::Instance()->Game()->GetEnemyIconMgr()->Remove(marker);
}

void EnemyIcons::SetFlags(BaseEntity* entity, Marker* marker)
{
    unsigned visibility = 0;
    if(entity) visibility = VisMap::GetInst()->GetVisFlags(entity->GetEntityContext()->GetSpectator(), marker) ;
    
    unsigned flags = 0;
    if(visibility & VT_VISIBLE) flags |= GameScreen::EnemyIconMgr::EnemyIcon::F_SELECTED;
    
    GameScreen::EnemyIconMgr::EnemyIcon* icon = 0; 
    GameScreen::EnemyIconMgr* icon_mgr = Screens::Instance()->Game()->GetEnemyIconMgr();
    
    if(icon = icon_mgr->ItemAt(marker)) icon->SetFlags(flags);
}

BaseEntity* EnemyIcons::Icon2Entity(unsigned index)
{
    GameScreen::EnemyIconMgr::EnemyIcon* icon = 0;

    if(icon = Screens::Instance()->Game()->GetEnemyIconMgr()->ItemAt(index))
        if(icon->GetMarker()) return icon->GetMarker()->GetEntity();           
        
    return 0;
}

void EnemyIcons::MakeSaveLoad(SavSlot& slot)
{
    GameScreen::EnemyIconMgr* mgr = Screens::Instance()->Game()->GetEnemyIconMgr();

    if(slot.IsSaving()){
        
        size_t index = 0;
        GameScreen::EnemyIconMgr::EnemyIcon* icon = 0;

        while(icon = mgr->ItemAt(index++)){
            slot << icon->GetMarker()->GetEntity()->GetEID();
        }

        slot << static_cast<eid_t>(0);

    }else{

        eid_t eid;
        slot >> eid;

        mgr->Clear();

        while(eid){
            Insert(EntityPool::GetInst()->Get(eid)->GetEntityContext()->GetMarker());
            slot >> eid;
        }

        AttachObservers();
    }
}

//==============================================================

RobotIcons::RobotIcons()
{
    Spawner::Attach(this, ET_PREPARE_SPAWN);
    Spawner::Attach(this, ET_ENTITY_SPAWN);

    Screens::Instance()->Game()->GetHackVehicleMgr()->Clear();
}

RobotIcons::~RobotIcons()
{
    Spawner::Detach(this);
}

void RobotIcons::Insert(VehicleEntity* robot)
{
    Screens::Instance()->Game()->GetHackVehicleMgr()->Insert(GameScreen::HackVehicleMgr::Icon(robot->GetInfo()->GetName(), robot));        

    robot->Attach(this, EV_PREPARE_DEATH);
    robot->Attach(this, EV_SELECTION_CHANGE);

    SetRobotState(robot);
}

VehicleEntity* RobotIcons::Icon2Entity(unsigned index)
{
    GameScreen::HackVehicleMgr::Icon* icon = 0;

    if(icon = Screens::Instance()->Game()->GetHackVehicleMgr()->ItemAt(index))
        if(icon->GetVehicleEntity()) return icon->GetVehicleEntity()->Cast2Vehicle();           

    return 0;
}

void RobotIcons::Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info)
{
    switch(type){
    case ET_PREPARE_SPAWN:
        Screens::Instance()->Game()->GetHackVehicleMgr()->Clear();
        break;

    case ET_ENTITY_SPAWN:
        {
            VehicleEntity* vehicle =    static_cast<spawn_info_s*>(info)->m_entity
                ?  static_cast<spawn_info_s*>(info)->m_entity->Cast2Vehicle()
                :  0;
            if(     vehicle
                &&  vehicle->GetInfo()->IsRobot()
                &&  vehicle->GetPlayer() == PT_PLAYER)
                Insert(vehicle);
        }
        break;
    }

}

void RobotIcons::Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
{
    if(event == EV_PREPARE_DEATH) Remove(entity->Cast2Vehicle());
    if(event == EV_SELECTION_CHANGE) SetRobotState(entity->Cast2Vehicle());
}

void RobotIcons::SetRobotState(VehicleEntity* robot)
{    
    unsigned flags = 0;
    if(robot->GetEntityContext()->IsSelected()) flags |= GameScreen::HackVehicleMgr::Icon::F_SELECTED;
    
    GameScreen::HackVehicleMgr::Icon* icon = 0; 
    
    if(icon = Screens::Instance()->Game()->GetHackVehicleMgr()->ItemAt(robot))
        icon->SetFlags(flags);
}

void RobotIcons::Remove(VehicleEntity* robot)
{
    robot->Detach(this);
    Screens::Instance()->Game()->GetHackVehicleMgr()->Remove(robot);    
}

void RobotIcons::MakeSaveLoad(SavSlot& slot)
{
    GameScreen::HackVehicleMgr* mgr = Screens::Instance()->Game()->GetHackVehicleMgr();

    if(slot.IsSaving()){

        size_t index = 0;
        GameScreen::HackVehicleMgr::Icon* icon = 0;

        while(icon = mgr->ItemAt(index++)){
            slot << icon->GetVehicleEntity()->GetEID();
        }

        //признак конца
        slot << static_cast<eid_t>(0);

    }else{

        eid_t eid;
        slot >> eid;

        mgr->Clear();

        while(eid){
            Insert(EntityPool::GetInst()->Get(eid)->Cast2Vehicle());
            slot >> eid;
        }
    }
}

//==============================================================

Reloader::Reloader()
{
}

Reloader::~Reloader()
{
}

void Reloader::DoReload(HumanEntity* human)
{       
    HumanContext* context = human->GetEntityContext();
    WeaponThing*  weapon  = static_cast<WeaponThing*>(context->GetThingInHands(TT_WEAPON));

int mps=context->GetTraits()->GetMovepnts();
int mps4reload = weapon->GetInfo()->GetMps2Reload();
if(mps<mps4reload) return;
context->GetTraits()->AddMovepnts(-mps4reload);

    HumanContext::reload_notifier notifier(human, weapon);

    HumanContext::iterator itor = context->begin(HPK_ALL, TT_AMMO);
    
    //ищем подходящие патроны
    while(itor != context->end()){

        AmmoThing* ammo4load = static_cast<AmmoThing*>(&(*itor++));
               
        //если патроны не подходят перейти к следущим
        if(!ThingDelivery::GetInst()->IsSuitable(weapon, ammo4load))
            continue;

        if(AmmoThing* weapon_ammo = weapon->GetAmmo()){
            
            if(weapon_ammo->GetInfo()->GetRID() == ammo4load->GetInfo()->GetRID()){
                
                //пополним обойму
                weapon_ammo->SetCount(ammo4load->GetCount() + weapon_ammo->GetCount());
                ammo4load->SetCount(0);
                
                //обрежем заряженное по магазину
                if(weapon_ammo->GetCount() > weapon->GetInfo()->GetQuality()){
                    ammo4load->SetCount(weapon_ammo->GetCount() - weapon->GetInfo()->GetQuality());
                    weapon_ammo->SetCount(weapon->GetInfo()->GetQuality());
                }
                
                //если обойма пуста удалить ее
                if(ammo4load->GetCount() == 0){
                    context->RemoveThing(ammo4load);
                    delete ammo4load;
                }
            }
            
        }else{
            
            //оружие пустое заряжаем первые попавшиеся патроны
            if(ammo4load->GetCount() > weapon->GetInfo()->GetQuality()){
                
                AmmoThing* new_ammo = static_cast<AmmoThing*>(ThingFactory::GetInst()->CreateAmmo(
                                                                        ammo4load->GetInfo()->GetRID(),
                                                                        weapon->GetInfo()->GetQuality()));
                //создаим новые патроны в оружии
                weapon->Load(new_ammo);
                
                //уберем заряженное из обоймы
                ammo4load->SetCount(ammo4load->GetCount() - weapon->GetInfo()->GetQuality());
                
            }else{
                
                context->RemoveThing(ammo4load);
                weapon->Load(ammo4load);
            }
        } 

        //если оружие больше не заряжается выход
        if(!CanLoadAmmo(weapon)) break;
    }
}

bool Reloader::CanReload(HumanEntity* human) const
{
    HumanContext* context = human->GetEntityContext();
    
    //если у человека нет оружия  
    if(WeaponThing* weapon = static_cast<WeaponThing*>(context->GetThingInHands(TT_WEAPON))){
        
        //если оружие не заряжается
        if(!CanLoadAmmo(weapon)) return false;

        //получим ссылку на патроны в оружии
        AmmoThing* weapon_ammo = weapon->GetAmmo();
        
        HumanContext::iterator itor = context->begin(HPK_ALL, TT_AMMO);
        
        //ищем подходящие патроны
        while(itor != context->end()){
                        
            if(ThingDelivery::GetInst()->IsSuitable(weapon, &(*itor))){

                //если в оружие нет патронов, то подойдут любые
                if(weapon_ammo == 0)
                    return true;

                //если в оружии есть патроны то нужны такие же
                if(weapon_ammo->GetInfo()->GetRID() == itor->GetInfo()->GetRID())
                    return true;
            }
            
            ++itor;
        }
    }

    return false;
}

bool Reloader::CanLoadAmmo(const WeaponThing* weapon) const
{
    return      weapon->GetAmmo() == 0
            ||  weapon->GetAmmo()->GetCount() < weapon->GetInfo()->GetQuality();
}

//==============================================================

GameFormImp::GameFormImp() : 
    m_last_move(0), m_activity(0), m_auto_turn(0), m_action_mgr(0), m_update_delta(0.35f),
    m_cur_ent(0), m_flags(0), m_turn_state(TS_START_TURN), m_activity_type(AT_NONE),  m_update_time(0)
{
    Spawner::GetInst()->Attach(this, ET_ENTRY_LEVEL);
    Spawner::GetInst()->Attach(this, ET_FINISH_SPAWN);    

    //проиниц массив драйверов ввода
    m_input_mgr[IT_EMPTY]     = new EmptyInputManager();
    m_input_mgr[IT_USUAL]     = new UsualInputManager();
    m_input_mgr[IT_ACTIVITY]  = new ActivityInputManager();

    Init();
}

GameFormImp::~GameFormImp()
{
    Spawner::GetInst()->Detach(this);

    if(m_cur_ent) m_cur_ent->Detach(this);

    for(int k = 0; k < MAX_INPUT_MGRS; delete m_input_mgr[k++]);

    delete m_activity;
}

void GameFormImp::Show()
{
    GameScreen* screen = Screens::Instance()->Game();

    screen->ShowControls(true);
    screen->SetController(this);
    screen->SetCBState(GameScreen::CB_MENU, GameScreen::CBS_OFF);

    m_auto_turn = 0;
    m_last_move = Timer::GetSeconds();
    
    std::string str;
    
    if(DirtyLinks::GetCmdLineArg("-autoturn", &str)){
        std::istringstream istr(str);
        istr >> m_auto_turn;
    }

    Screens::Instance()->Activate(Screens::SID_GAME);
}

void GameFormImp::HandleInput(state_type* st)
{
    if(!st) return;
    *st = ST_INCOMPLETE;

    m_vehicles.Update();

    //установить кнопку журнала квестов
    if(m_update_time < Timer::GetSeconds()){        

        m_update_time = Timer::GetSeconds() + m_update_delta;

        bool fupdated = Forms::GetInst()->GetJournalForm()->IsUpdated();

        if(!(m_flags & FT_UPDATE_JOURNAL) && fupdated){
            DirtyLinks::Print(DirtyLinks::GetStrRes("gf_journal_updated"));
            SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_JOURNAL_WAS_UPDATED));
        }

        m_flags = fupdated ? (m_flags | FT_UPDATE_JOURNAL) : (m_flags & ~FT_UPDATE_JOURNAL);

        Screens::Instance()->Game()->SetJournalButtonState( 
                        fupdated
                    ?   GameScreen::JBS_BLINK
                    :   GameScreen::JBS_NORMAL);     
    }

    //проигрывание действия
    if(m_activity_type != AT_NONE){

        if(m_turn_state == TS_INCOMPLETE)
            m_input_mgr[IT_ACTIVITY]->HandleInput();
        else
            RunActivity(AC_TICK);

        return;
    }    

    if(Input::MouseState().dx || Input::MouseState().dy || Input::MouseState().dz)
        m_last_move = Timer::GetSeconds();

    if(m_auto_turn && m_auto_turn < Timer::GetSeconds() - m_last_move){
        m_auto_turn = 0;
        HandleEndOfTurnReq();
    }

    //проигрывание скриптовых сцен
    if(Activity* activity = SSceneMgr::GetInst()->CreateScriptScene()){
        ScriptScenePlayer::GetInst()->Init(activity);
        HandlePlayActionReq(ScriptScenePlayer::GetInst());
        return;        
    }
    
    switch(m_turn_state){
    case TS_INCOMPLETE:
        m_input_mgr[IT_USUAL]->HandleInput();
        break;

    case TS_START_TURN:
        m_turn_state = TS_INCOMPLETE;
        if(m_cur_ent){
            HandleSelectReq(m_cur_ent);
            Cameraman::GetInst()->FocusEntity(m_cur_ent);
        }
        break;

    case TS_FINISHED:
        *st = ST_FINISHED;
        m_turn_state = TS_START_TURN;
        break;

    case TS_RUN_WALKERS:
        if(ActionManager* action = m_walkers.CreateAction())
            HandlePlayActionReq(action);
        else
            m_turn_state = TS_FINISHED;
        break;

    case TS_TEST_LEVEL_EXIT:
        {
            Screens::Instance()->Game()->ShowControls(false);
            Cameraman::GetInst()->Configure(Cameraman::F_SHOW_ALL);

            ShowUsualCursor();
            
            AIUtils::HideLandField();
            AIUtils::HidePassField();
        
            m_turn_state = TS_RUN_WALKERS;

            std::string  sys_name, full_name;
            
            if(!CanExitLevel(&sys_name))
                break;
            
            full_name = DirtyLinks::GetLevelFullName(sys_name);
            
            if(Forms::GetInst()->ShowMessageBox( mlprintf( DirtyLinks::GetStrRes("gf_can_exit_level").c_str(), full_name.c_str()))){
                
                m_turn_state = TS_EXIT_LEVEL;
                
                //сохрание по выходе с уровня
                Screens::Instance()->Game()->MakeSpecialSaveLoad(GameScreen::SST_AUTOSAVE, GameScreen::SSM_SAVE);

                GameObserver::exit_level_info info(sys_name);
                GameEvMessenger::GetInst()->Notify(GameObserver::EV_INIT_LEVEL_EXIT, &info);
            }            
        }
        break;

    case TS_EXIT_LEVEL:
        {
            m_turn_state = TS_RUN_WALKERS;

            std::string level;
            if(CanExitLevel(&level)){
                m_turn_state = TS_WAIT_LEVEL;
                
                //уничтожить ссылку на текущего перца
                if(m_cur_ent) HandleSelectReq(0);
                
                //выход с уровня
                Spawner::GetInst()->ExitLevel(level);
            }
        }
        break;

    case TS_WAIT_LEVEL:
        break;
    }
}

void GameFormImp::MakeSaveLoad(SavSlot& slot)
{
    m_robots.MakeSaveLoad(slot);
    m_humans.MakeSaveLoad(slot);
    m_enemies.MakeSaveLoad(slot);
    m_walkers.MakeSaveLoad(slot);
    m_vehicles.MakeSaveLoad(slot);

//	Naughty: переставляю эту строчку только для загрузки
//  Screens::Instance()->Game()->MsgWindow()->Clear();

    if(slot.IsSaving()){
        
        slot << m_flags;
        slot << (m_cur_ent != 0);

        if(m_cur_ent) slot << m_cur_ent->GetEID();

    }else{

        slot >> m_flags;

        bool flag;
        slot >> flag;

        if(flag){
            eid_t eid; slot >> eid;
            HandleSelectReq(EntityPool::GetInst()->Get(eid));
        }

        RestoreSelection();
        ResetCursorModeButtons();

        m_turn_state = TS_INCOMPLETE;

		//	Naughty: очищаем консоль
		Screens::Instance()->Game()->MsgWindow()->Clear();
    }
}

void GameFormImp::HandleSwitch2Run()
{
    Switch2Run switcher;
    m_cur_ent->Accept(switcher);
}

void GameFormImp::HandleSwitch2Sit()
{
    Switch2Sit switcher;
    m_cur_ent->Accept(switcher);
}

void GameFormImp::HandleSwitch2Stand()
{
    Switch2Stand switcher;
    m_cur_ent->Accept(switcher);
}

void GameFormImp::HandleSwitch2Aimshot()
{
    Switch2Aimshot switcher;
    m_cur_ent->Accept(switcher);
}

void GameFormImp::HandleSwitch2Autoshot()
{
    Switch2Autoshot switcher;
    m_cur_ent->Accept(switcher);
}

void GameFormImp::HandleSwitch2Snapshot()
{
    Switch2Snapshot switcher;
    m_cur_ent->Accept(switcher);
}

void GameFormImp::HandleSelectReq(BaseEntity* entity, selection_type type)
{
    if(m_cur_ent){

        //снять наблюдателя
        m_cur_ent->Detach(this);
        m_cur_ent->Accept(Deselector());
		//RunActivity(AC_STOP);//Grom
    }

    //уберем кнопки принудительной атаки, use, hack
    if(     entity == 0 
        ||  (m_cur_ent && m_cur_ent->GetEID() != entity->GetEID()))
        ResetCursorModeButtons();

    if(m_cur_ent = entity){
        
        //поставить наблюдателя
        m_cur_ent->Attach(this, EV_DESTROY);
        m_cur_ent->Attach(this, EV_QUIT_TEAM);
        m_cur_ent->Attach(this, EV_INSERT_THING);
        m_cur_ent->Attach(this, EV_REMOVE_THING);
        m_cur_ent->Attach(this, EV_PREPARE_DEATH);
        m_cur_ent->Attach(this, EV_HEX_COST_CHANGE);
        m_cur_ent->Attach(this, EV_MOVEPNTS_CHANGE);
        m_cur_ent->Attach(this, EV_BODY_PACK_CHANGE);
        m_cur_ent->Attach(this, EV_BODY_PART_CHANGE);
        m_cur_ent->Attach(this, EV_HANDS_PACK_CHANGE);
        m_cur_ent->Attach(this, EV_LONG_DAMAGE_CHANGE);
        m_cur_ent->Attach(this, EV_WEAPON_STATE_CHANGE);
        m_cur_ent->Attach(this, EV_CAN_MAKE_FAST_ACCESS);

        if(type == SEL_COMMAND) m_cur_ent->Notify(EntityObserver::EV_SELECT_COMMAND);

        Selector sel;
        m_cur_ent->Accept(sel);
    }

    m_cursor.Invalidate();
}

void GameFormImp::HandlePlayActionReq(ActionManager* mgr)
{
    ShowUsualCursor();

    AIUtils::HidePassField();
    AIUtils::HideLandField();

    m_action_mgr = mgr;     
    Screens::Instance()->Game()->SetController(0);

    m_activity_type = AT_ACTION_MOVE;
    if(m_action_mgr->NeedMove()) m_activity = m_action_mgr->CreateMove();       
}

void GameFormImp::Init()
{
    GameScreen* screen = Screens::Instance()->Game();

    screen->SetJournalButtonState(GameScreen::JBS_NORMAL);
    
    screen->SetCBState(GameScreen::CB_RUN, GameScreen::CBS_OFF);
    screen->SetCBState(GameScreen::CB_SIT, GameScreen::CBS_OFF); 
    screen->SetCBState(GameScreen::CB_USE, GameScreen::CBS_OFF);
    screen->SetCBState(GameScreen::CB_SIT, GameScreen::CBS_OFF);
    screen->SetCBState(GameScreen::CB_RUN, GameScreen::CBS_OFF);
    screen->SetCBState(GameScreen::CB_MENU, GameScreen::CBS_OFF);
    screen->SetCBState(GameScreen::CB_ROOF, GameScreen::CBS_OFF);
    screen->SetCBState(GameScreen::CB_ENDTURN, GameScreen::CBS_OFF);

    screen->SetCBState(GameScreen::CB_FOS, (m_flags & FT_SHOW_FOS) ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
    
    screen->EnableCBState(GameScreen::CB_USE, false);
    screen->EnableCBState(GameScreen::CB_SIT, false);
    screen->EnableCBState(GameScreen::CB_RUN, false);
    screen->EnableCBState(GameScreen::CB_AIMSHOT, false);
    screen->EnableCBState(GameScreen::CB_SNAPSHOT, false);
    screen->EnableCBState(GameScreen::CB_AUTOSHOT, false);

    GameScreen::BodyIconTraits icon_traits;

    icon_traits.m_Body  = GameScreen::BodyIconTraits::BPS_NONE;
    icon_traits.m_Hands = GameScreen::BodyIconTraits::BPS_NONE;
    icon_traits.m_Head  = GameScreen::BodyIconTraits::BPS_NONE;
    icon_traits.m_Legs  = GameScreen::BodyIconTraits::BPS_NONE;
    icon_traits.m_Damages = 0;

    screen->SetBodyIcon(icon_traits);

    SwitchRoofs();
}

void GameFormImp::RunActivity(int command)
{
    if(m_activity && m_activity->Run(static_cast<activity_command>(command)))
	{
        return;
	}

    delete m_activity;
    m_activity = 0;
	
		switch(m_activity_type){
		case AT_ACTION_MOVE:
			if(TrapActionMoveEnd()) return;
			break;
			
		case AT_ACTION_ROTATE:
			if(TrapActionRotateEnd()) return;
			break;
			
		case AT_ACTION:
			if(TrapActionEnd()) return;
			break;
		}
    if(m_cur_ent){
        Selector sel;
        m_cur_ent->Accept(sel);
    }
    
    m_activity_type = AT_NONE;
    Screens::Instance()->Game()->SetController(this);
}

void GameFormImp::HandleSwitchFOSReq()
{
    m_flags ^= FT_SHOW_FOS;
    Screens::Instance()->Game()->SetCBState(GameScreen::CB_FOS, (m_flags & FT_SHOW_FOS) ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
}

void GameFormImp::HandleEndOfTurnReq()
{
    m_turn_state = TS_TEST_LEVEL_EXIT; 
}

void GameFormImp::HandleShowJournalReq()
{
    ShowUsualCursor();
    Forms::GetInst()->ShowJournalForm();
    Screens::Instance()->Game()->SetJournalButtonState(GameScreen::JBS_SELECTED);
}

void GameFormImp::HandleShowInventoryReq(HumanEntity* human, AbstractGround* ground, AbstractScroller* scroller, AbstractBin* bin)
{
    ShowUsualCursor();
    Forms::GetInst()->ShowInventoryForm(human, ground, scroller, bin);
}

void GameFormImp::Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
{
	assert(!IsBadWritePtr(entity,sizeof(BaseEntity)));

    switch(event){
    case EV_DESTROY:
		m_cur_ent = NULL;
    case EV_PREPARE_DEATH:
		//if(m_action_mgr) m_action_mgr->Shut();//Grom
		//Внимание! сквозной switch
    case EV_QUIT_TEAM:
        GetGameFormImp()->HandleSelectReq(0);
        break;
        
    case EV_HEX_COST_CHANGE:
    case EV_MOVEPNTS_CHANGE: 
        
        m_cur_ent->Accept(WeaponSelector());

        if(m_activity_type == AT_NONE && m_turn_state != TS_RUN_WALKERS)
            m_cur_ent->Accept(MovepntsSelector());
        break;
        
    case EV_BODY_PART_CHANGE:
    case EV_LONG_DAMAGE_CHANGE:
        m_cur_ent->Accept(BodySelector());
        break;

    case EV_BODY_PACK_CHANGE:
        m_cur_ent->Accept(ButtonStateSelector());
        break;

    case EV_CAN_MAKE_FAST_ACCESS:
        m_cur_ent->Accept(QuickSlotSelector());
        break;

    case EV_REMOVE_THING:
    case EV_INSERT_THING:
        if(static_cast<thing_info_s*>(info)->m_thing->GetInfo()->IsAmmo())
            m_cur_ent->Accept(ReloadButtonSelector());
        break;
        
    case EV_HANDS_PACK_CHANGE:
    case EV_WEAPON_STATE_CHANGE:
        m_cur_ent->Accept(WeaponSelector());
        m_cur_ent->Accept(QuickSlotSelector());
        m_cur_ent->Accept(ButtonStateSelector());
        m_cur_ent->Accept(ReloadButtonSelector());
        if(CanAttack()) HandleSwitchAttackReq();
        break;
    }
}

bool GameFormImp::TrapActionMoveEnd()
{
	if(!m_cur_ent && (m_activity_type==AT_ACTION_MOVE||m_activity_type==AT_ACTION_ROTATE) )
	{}
	else
	{
	
    if(m_action_mgr->NeedMove())
        return false;
//    m_activity_type = AT_ACTION_ROTATE;
    if(m_action_mgr->NeedRotate()) m_activity = m_action_mgr->CreateRotate();   
	}
    m_activity_type = AT_ACTION_ROTATE;
    return true;
}

bool GameFormImp::TrapActionRotateEnd()
{ 
	if(!m_cur_ent && (m_activity_type==AT_ACTION_MOVE||m_activity_type==AT_ACTION_ROTATE) )
	{
	}
	else
	{
    if(m_action_mgr->NeedRotate())
        return false;
	}
	
    m_activity_type = AT_ACTION;
    m_activity = m_action_mgr->CreateAction();
    return true;
}

bool GameFormImp::TrapActionEnd()
{
	/*if(!m_cur_ent && (m_activity_type==AT_ACTION_MOVE||m_activity_type==AT_ACTION_ROTATE) )
		return false;
	  */
    m_action_mgr->Shut();
    return false;
}

bool GameFormImp::CanExitLevel(std::string* level)
{
    EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_ALL_ENTS, PT_PLAYER);

    //если наших нет - выход
    if(itor == EntityPool::GetInst()->end())
        return false;

    itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER, EA_INCREW);
    while(itor != EntityPool::GetInst()->end()){
		HumanEntity *ent = itor->Cast2Human();
		if(ent)
		{
			VehicleEntity *v=ent->GetEntityContext()->GetCrew();
			if(v->GetPlayer()!=PT_PLAYER) return false;
		}
		++itor;
    }


    const unsigned none_joints = 0xffffffff;

    unsigned joints = none_joints;

    //проверка для людей
    itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER, EA_NOT_INCREW);
    while(itor != EntityPool::GetInst()->end()){
        joints &= HexGrid::GetInst()->GetProp(itor->GetGraph()->GetPos2()).m_joints;
        ++ itor;
    }

    //проверка техники
    itor = EntityPool::GetInst()->begin(ET_VEHICLE, PT_PLAYER);
    while(itor != EntityPool::GetInst()->end()){

        if(itor->Cast2Vehicle()->GetInfo()->IsTech()){
            joints &= HexGrid::GetInst()->GetProp(itor->GetGraph()->GetPos2()).m_joints;
        }

        ++itor;
    }
    
    return joints && joints != none_joints && Spawner::GetInst()->CanExit(joints, level);
}

void GameFormImp::Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info)
{
	assert(!IsBadWritePtr(spawner,sizeof(Spawner)));

    switch(type){
    case ET_ENTRY_LEVEL:
        if(static_cast<entry_info_s*>(info)->m_entry.empty())
            ResetCursorModeButtons();
        break;

    case ET_FINISH_SPAWN:
        RestoreSelection();
        m_turn_state = TS_START_TURN;
        Screens::Instance()->Game()->MsgWindow()->Clear();
        break;
    }
}

void GameFormImp::HandleShowShopReq(HumanEntity* hero, TraderEntity* trader, AbstractGround* ground, AbstractScroller* scroller, AbstractBin* bin)
{
    Forms::GetInst()->ShowShopForm(hero, trader, ground, scroller, bin);
    ShowUsualCursor();
}

void GameFormImp::HandleShowOptionsReq()
{
    ShowUsualCursor();

    Forms::GetInst()->ShowOptionsDialog(Forms::ODM_GAMEMENU);
    Screens::Instance()->Game()->SetCBState(GameScreen::CB_MENU, GameScreen::CBS_ON);
}

void GameFormImp::HandleSwitchUseReq()
{
    if(m_cur_ent && m_cur_ent->Cast2Human()){

        //переключить состояние флага
        if((m_flags ^= FT_CAN_USE) & FT_CAN_USE){
            if(CanAttack()) HandleSwitchAttackReq();
            if(CanMakeHack()) HandleSwitchHackReq();
        }

        //обновить курсор немедленно
        m_cursor.Invalidate();

        //изменить вид кнопки
        Screens::Instance()->Game()->SetCBState(GameScreen::CB_USE, (m_flags & FT_CAN_USE) ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
    }
}

void GameFormImp::HandleSwitchAttackReq()
{
    if(m_cur_ent && m_cur_ent->Cast2Human()){

        //переключим флаг
        if((m_flags ^= FT_CAN_ATTACK) & FT_CAN_ATTACK){
            if(CanMakeUse()) HandleSwitchUseReq();
            if(CanMakeHack()) HandleSwitchHackReq();
        }

        //обновим курсор
        m_cursor.Invalidate();
    }
}

bool GameFormImp::CanMakeUse() const
{
    return m_cur_ent && (m_flags & FT_CAN_USE) && m_cur_ent->Cast2Human();
}

bool GameFormImp::CanAttack() const
{
    return m_cur_ent && (m_flags & FT_CAN_ATTACK) && m_cur_ent->Cast2Human();
}

void GameFormImp::RestoreSelection()
{
    EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);
    while(itor != EntityPool::GetInst()->end()){
        
        if(itor->Cast2Human()->GetEntityContext()->IsSelected()){
            Cameraman::GetInst()->FocusEntity(&*itor, 0);
            HandleSelectReq(&*itor);
            return;
        }
        
        ++itor;
    }
    
    //просмотрим технику
    itor = EntityPool::GetInst()->begin(ET_VEHICLE);
    while(itor != EntityPool::GetInst()->end()){
        
        if(itor->Cast2Vehicle()->GetEntityContext()->IsSelected()){
            Cameraman::GetInst()->FocusEntity(&*itor, 0);
            HandleSelectReq(&*itor);
            return;
        }
        
        ++itor;
    }

    //выделим первого в иконках
    if(BaseEntity* ent4sel = m_humans.Icon2Entity(0)){
        Cameraman::GetInst()->FocusEntity(ent4sel, 0);
        HandleSelectReq(ent4sel);
    }
}

void GameFormImp::ShowControls(bool flag)
{
    Screens::Instance()->Game()->ShowControls(flag);
}

bool GameFormImp::CanMakeHack() const
{
    return m_cur_ent && (m_flags & FT_CAN_HACK) && m_cur_ent->Cast2Human();
}

void GameFormImp::HandleSwitchHackReq()
{
   if(m_cur_ent && m_cur_ent->Cast2Human()){

        //переключить состояние флага
        if((m_flags ^= FT_CAN_HACK) & FT_CAN_HACK){         
            if(CanMakeUse()) HandleSwitchUseReq();
            if(CanAttack()) HandleSwitchAttackReq();
        }        

        //обновить курсор немедленно
        m_cursor.Invalidate();
        //изменить вид кнопки
        Screens::Instance()->Game()->SetCBState(GameScreen::CB_SPECIAL, (m_flags & FT_CAN_HACK) ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
    }
}

void GameFormImp::ShowUsualCursor()
{
    m_cursor.SetUsual();
}

void GameFormImp::OnCheckBoxClick(GameScreen::CHECKBOX id)
{
    switch(id){
    case GameScreen::CB_USE:
        HandleSwitchUseReq();
        break;
        
    case GameScreen::CB_FOS:
        HandleSwitchFOSReq();
        break;
        
    case GameScreen::CB_SPECIAL:
        HandleSwitchHackReq();
        break;             
        
    case GameScreen::CB_ROOF:
        SwitchRoofs();
        break;   
        
    case GameScreen::CB_ENDTURN:
        HandleEndOfTurnReq();
        break;
        
    case GameScreen::CB_MENU:
        HandleShowOptionsReq();
        break;
        
    case GameScreen::CB_JOURNAL:
        HandleShowJournalReq();
        break;             
    }
    
    if(m_cur_ent == 0) return;
    
    switch(id){
    case GameScreen::CB_AIMSHOT:
        HandleSwitch2Aimshot();
        break;
        
    case GameScreen::CB_AUTOSHOT:
        HandleSwitch2Autoshot();
        break;
        
    case GameScreen::CB_SNAPSHOT:
        HandleSwitch2Snapshot();
        break;
        
    case GameScreen::CB_SIT:
        if(IsChecked(GameScreen::CB_SIT))
            HandleSwitch2Stand();
        else
            HandleSwitch2Sit();
        break;
        
    case GameScreen::CB_RUN:
        if(IsChecked(GameScreen::CB_RUN))
            HandleSwitch2Stand();
        else
            HandleSwitch2Run();
        break;   

    case GameScreen::CB_BACKPACK:
        HandleSwitchHandsMode();
        break;
    }
}

void GameFormImp::OnIconClick(unsigned int id,GameScreen::BN_CLICK click_type)
{
    HumanEntity* human = m_humans.Icon2Entity(id);
    
    if(!human) return;
    
    switch(click_type){
    case GameScreen::BNC_LCLICK:
        HandleSelectReq(human, SEL_COMMAND);
        break;
        
    case GameScreen::BNC_RCLICK:        
        HandleShowInventoryReq(human, UsualGround::GetInst(), GameFormHeroScroller::GetInst(), GroundBin::GetInst());
        break;
        
    case GameScreen::BNC_LDBLCLICK:
        Cameraman::GetInst()->FocusEntity(human);
        break;
    }
}

void GameFormImp::OnWeaponIconClick() 
{  
    if(Input::MouseState().LButtonFront){
        HandleSwitchAttackReq();
        return;
    }
    
    if(Input::MouseState().RButtonFront){
        
        if(m_cur_ent == 0 || m_cur_ent->Cast2Human() == 0)
            return;
        
        BaseThing* thing = m_cur_ent->Cast2Human()->GetEntityContext()->GetThingInHands();
        if(thing) Forms::GetInst()->ShowDescDialog(thing);                
    }
}

void GameFormImp::OnVehicleIconClick(unsigned int icon_number, GameScreen::BN_CLICK click_type)
{
    VehicleEntity* vehicle = GetGameFormImp()->GetTechIcons()->Icon2Entity(icon_number);
    
    if(!vehicle) return;
    
    switch(click_type){
    case GameScreen::BNC_LCLICK:
        HandleSelectReq(vehicle);
        break;
        
    case GameScreen::BNC_LDBLCLICK:
        Cameraman::GetInst()->FocusEntity(vehicle);
        break;
    }
}

void GameFormImp::OnEnemyIconClick(Marker* marker)
{   
    if(BaseEntity* entity = marker->GetEntity()) Cameraman::GetInst()->FocusEntity(entity);
}

void GameFormImp::OnHackVehicleClick(VehicleEntity* vehicle, GameScreen::BN_CLICK click_type)
{
    switch(click_type){
    case GameScreen::BNC_LCLICK:
        HandleSelectReq(vehicle);
        break;
        
    case GameScreen::BNC_LDBLCLICK:
        Cameraman::GetInst()->FocusEntity(vehicle);
        break;
    }
}

void GameFormImp::OnButtonClick(GameScreen::BUTTON button)
{
    switch(button){
    case GameScreen::B_RELOAD:
        HandleReloadWeapon();
        break;  
        
    case GameScreen::B_UP_ARROW:
        HandleNextFastAccessThing();
        break;

    case GameScreen::B_DOWN_ARROW:
        HandlePrevFastAccessThing();
        break;
    }
}

void GameFormImp::Init(const ini_s& ini)
{
}

void GameFormImp::HandleReloadWeapon()
{
    HumanEntity* human = m_cur_ent ? m_cur_ent->Cast2Human() : 0;

    if(human && m_reloader.CanReload(human))
        m_reloader.DoReload(human);
}

void GameFormImp::HandleSwitchHandsMode()
{
    if(HumanEntity* human = m_cur_ent ? m_cur_ent->Cast2Human() : 0){

        HumanContext* context = human->GetEntityContext();

        context->SetHandsMode(      context->GetHandsMode() == HumanContext::HM_HANDS
                                ?   HumanContext::HM_FAST_ACCESS
                                :   HumanContext::HM_HANDS);
    }
}

void GameFormImp::HandleNextFastAccessThing()
{
    HumanEntity* human = m_cur_ent ? m_cur_ent->Cast2Human() : 0;

    if(human && human->GetEntityContext()->GetHandsMode() == HumanContext::HM_FAST_ACCESS)
        SetNextFastThing(human, FastAccessStrategy::IT_FORWARD);    
}

void GameFormImp::HandlePrevFastAccessThing()
{
    HumanEntity* human = m_cur_ent ? m_cur_ent->Cast2Human() : 0;

    if(human && human->GetEntityContext()->GetHandsMode() == HumanContext::HM_FAST_ACCESS)
        SetNextFastThing(human, FastAccessStrategy::IT_BACKWARD);
}

void GameFormImp::ResetCursorModeButtons()
{
    if(CanMakeUse()) HandleSwitchUseReq();
    if(CanAttack()) HandleSwitchAttackReq();
    if(CanMakeHack()) HandleSwitchHackReq();
}


