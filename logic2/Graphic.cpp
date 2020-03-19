#pragma warning(disable:4786)
#include "logicdefs.h"

#include "../IWorld.h"
#include "../Skin/Person.h"
#include "../GameLevel/grid.h"
#include "../GameLevel/ScatteredItems.h"
#include "../Common/3Deffects/EffectManager.h"

#include "spawn.h"
#include "Thing.h"
#include "Entity.h"
#include "graphic.h"
#include "aiutils.h"
#include "sndutils.h"
#include "HexUtils.h"
#include "DirtyLinks.h"
#include "GraphHexGrid.h"
#include "EnemyDetection.h"

DCTOR_IMP(GraphThing)
DCTOR_IMP(GraphHuman)
DCTOR_IMP(GraphVehicle)

bool GraphThing::m_fcan_destroy = true;
float GraphEntity::m_update_time = 0.0f;
float GraphEntity::m_update_delta = 0.25f;

unsigned GraphEntity::m_flags = GraphEntity::FT_INIT;

GraphEntity* GraphEntity::m_first;
GraphGrid* GraphGrid::GetInst() 
{
	static GraphGrid m_obj;
	return &m_obj;
}

//================================================================

class CallbackBridge : public IMovingCallBack{
public:

    CallbackBridge() : m_events(0) {}

    void SetEvents(MoveEvents* events)
    {
        m_events = events;
    }

    COMMAND VisitHex(EVENT event)
    {
        switch(m_events->OnStep(event == EV_STEP ? MoveEvents::ET_STEP : MoveEvents::ET_END)){
        case MoveEvents::CT_MOVE:     return CMD_CONTINUE;
        case MoveEvents::CT_STOP:     return CMD_STOP;
        case MoveEvents::CT_SPEED_UP: return CMD_SPEED_UP;
        }

        throw CASUS("CallbackBridge: неивестная команда!!!");
    }

private:

    MoveEvents* m_events;
};

//================================================================

namespace{

    unsigned GetEntityBannerColor(BaseEntity* entity)
    {
        //цвета надписей над существами разных команд
        const unsigned ally_color    = 0x00ff00;
        const unsigned enemy_color   = 0xff0000;
        const unsigned neutral_color = 0xffff00;

        if(entity->GetPlayer() == PT_PLAYER)
            return ally_color;
        else if(EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(entity) == RT_ENEMY)
            return enemy_color;

        return neutral_color;
    }

    IEntity::EFFECT_TYPE GetEffectType(GraphEntity::effect_type type)
    {
        switch(type){
        case GraphEntity::ET_FLAME: return IEntity::ET_FLAME;
        case GraphEntity::ET_SHOCK: return IEntity::ET_SHOCK;
        case GraphEntity::ET_ELECTRIC: return IEntity::ET_ELECTRIC;
        case GraphEntity::ET_SELECTION: return IEntity::ET_SELECTION;
        }

        throw CASUS("GetEffectType: неизвестный тип эффекта!!!");
    }

    IEntity::SHOW_MODE GetShowMode(GraphEntity::show_mode mode)
    {
        switch(mode){
        case GraphEntity::SM_SHOW: return IEntity::SM_SHOW;
        case GraphEntity::SM_HIDE: return IEntity::SM_HIDE;
        case GraphEntity::SM_SLOW_HIDE: return IEntity::SM_SLOW_HIDE;
        }
 
        throw CASUS("GetShowMode: неизвестный тип отображения!!!");
    }

    unsigned GHSound2SPSound(GraphHuman::sound_type sound)
    {
        switch(sound){
        case GraphHuman::SND_NO_LOF: return SoundPerson::ET_NO_LOF;
        case GraphHuman::SND_CRITICAL_HURT: return SoundPerson::ET_CHURT;
        case GraphHuman::SND_LUCKY_STRIKE: return SoundPerson::ET_LUCKYSTRIKE;
        case GraphHuman::SND_DEATH: return SoundPerson::ET_DEATH;
        case GraphHuman::SND_ENEMY_SIGHTED: return SoundPerson::ET_ENEMYSIGHTED;
        case GraphHuman::SND_PRETTY_GIRL: return SoundPerson::ET_GIRL;
        case GraphHuman::SND_SELECT_COMMAND: return SoundPerson::ET_SELECTION;
        case GraphHuman::SND_MOVE_COMMAND: return SoundPerson::ET_MOVE;
        case GraphHuman::SND_ATTACK_COMMAND: return SoundPerson::ET_ATTACK;
        case GraphHuman::SND_CRITICAL_MISS: return SoundPerson::ET_MISS;
        case GraphHuman::SND_UNDER_FIRE: return SoundPerson::ET_UNDERFIRE;
        case GraphHuman::SND_TREATMENT_LOW: return SoundPerson::ET_TREATMENT_LOW;
        case GraphHuman::SND_TREATMENT_MEDIUM: return SoundPerson::ET_TREATMENT_MED;
        case GraphHuman::SND_TREATMENT_HIGH: return SoundPerson::ET_TREATMENT_HIGH;
        case GraphHuman::SND_TEAMMATE_NEW: return SoundPerson::ET_TEAM_NEW;
        case GraphHuman::SND_TEAMMATE_COME: return SoundPerson::ET_TEAM_COME;
        case GraphHuman::SND_TEAMMATE_LEAVE: return SoundPerson::ET_TEAM_LEAVE;
        case GraphHuman::SND_TEAMMATE_DISMISS: return SoundPerson::ET_TEAM_DISMISS;
        case GraphHuman::SND_ENEMY_DIED: return SoundPerson::ET_ENEMYDIED;
        case GraphHuman::SND_ENEMY_EXPLODED: return SoundPerson::ET_ENEMY_EXPLODED;
        case GraphHuman::SND_OPEN_SUCCEED: return SoundPerson::ET_OPEN_OK;
        case GraphHuman::SND_OPEN_FAILED: return SoundPerson::ET_OPEN_FAILED;
        case GraphHuman::SND_CAR_DRIVE_FAILED: return SoundPerson::ET_CAR_FAILED;
        case GraphHuman::SND_LEVELUP: return SoundPerson::ET_LEVELUP;
        case GraphHuman::SND_GOT_SHOTGUN: return SoundPerson::ET_GOT_SHOTGUN;
        case GraphHuman::SND_GOT_ROCKET_LAUNCHER: return SoundPerson::ET_GOT_RLAUNCHER;
        case GraphHuman::SND_GOT_AUTOCANNON: return SoundPerson::ET_GOT_AUTOCANNON;
        case GraphHuman::SND_GOT_ENERGY_GUN: return SoundPerson::ET_GOT_ENERGYGUN;
        case GraphHuman::SND_USE_PLASMA_GRENADE: return SoundPerson::ET_USE_PLASMA_GREN;
        case GraphHuman::SND_TERMOPLASMA_EXPLOSION: return SoundPerson::ET_SAW_BIGBANG;
        case GraphHuman::SND_INSERT_SCANNER: return SoundPerson::ET_SCANNER;
        case GraphHuman::SND_REMOVE_SCANNER: return SoundPerson::ET_SCANNER|SoundPerson::ET_STOPIT;
        }

        return SoundPerson::ET_RANDOM3D;
    }
}

//================================================================

bool GraphUtils::IsInVicinity(const point3& pos, const ipnt2_t& pnt, float eps)
{
    point3 test = HexGrid::GetInst()->Get(pnt);
    
    float dx = pos.x - test.x,
          dy = pos.y - test.y;
    
    return dx*dx + dy*dy < eps*eps;
}

void GraphUtils::DestroyCorpses()
{
    IWorld::Get()->GetCharacterPool()->CleanByAction(DEAD);
}

//================================================================

void GraphGrid::Draw()
{
    m_graph->Draw();
}

void GraphGrid::Show(hex_type type, bool flag)
{
    m_fshow[type] = flag;

    switch(type){
    case HT_JOINTS:
        m_graph->Show(GraphHexGrid::joint, DirtyLinks::GetIntOpt(DirtyLinks::OT_SHOW_JOINTS) && flag);        
        break;

    case HT_FRONT:
        m_graph->Show(GraphHexGrid::front, DirtyLinks::GetIntOpt(DirtyLinks::OT_SHOW_FRONT) && flag);
        break;

    case HT_PATH:
        m_graph->Show(GraphHexGrid::path, DirtyLinks::GetIntOpt(DirtyLinks::OT_SHOW_PATH) && flag);
        break;

    case HT_LAND:
        m_graph->Show(GraphHexGrid::land,DirtyLinks::GetIntOpt(DirtyLinks::OT_SHOW_LANDS) && flag);
        break;
    }
}

void GraphGrid::SetHexes(hex_type type, const pnt_vec_t& vec)
{
    switch(type){
    case HT_JOINTS:
        m_graph->SetHexes(GraphHexGrid::joint, vec);
        break;

    case HT_FRONT:
        m_graph->SetHexes(GraphHexGrid::front, vec);
        break;

    case HT_PATH:
        m_graph->SetHexes(GraphHexGrid::path, vec);
        break;

    case HT_LAND:
        m_graph->SetHexes(GraphHexGrid::land, vec);
        break;
    }
}

GraphGrid::GraphGrid() : m_graph(new GraphHexGrid()) 
{    
    for(int k = 0; k < HT_MAX; m_fshow[k++] = false);
}

GraphGrid::~GraphGrid()
{
    delete m_graph;
}

void GraphGrid::HandleOptionsChange()
{
    for(int k = 0; k < HT_MAX; k++){
        Show(static_cast<hex_type>(k), m_fshow[k]);
    }
}

//================================================================


EffectMgr* EffectMgr::GetInst()
{
    static EffectMgr imp;
    return &imp;
}

EffectMgr::EffectMgr()
{
}

EffectMgr::~EffectMgr()
{
}

void EffectMgr::MakeHit(const rid_t& name, const point3& from, const point3& to)
{
    point3 ground = to;
    ground.z = static_cast<Grid*>(HexGrid::GetInst())->Height(ground);

    IWorld::Get()->GetEffects()->CreateHitEffect(name, from, to, ground, 1.7);
}

void EffectMgr::MakeFlash(const rid_t& name, const point3& from, const point3& to)
{
    IWorld::Get()->GetEffects()->CreateFlashEffect(name, from, to);
}

void EffectMgr::MakeShellOutlet(BaseEntity* entity, const rid_t& name, const point3& from, const point3& to)
{
	IWorld::Get()->GetEffects()->CreateShellEffect(entity->GetEID(), name, from, to);
}

float EffectMgr::MakeTracer(const rid_t& name, const point3& from, const point3& to)
{
    return IWorld::Get()->GetEffects()->CreateTracerEffect(name, from, to);
}

void EffectMgr::MakeHumanTreatment(const GraphHuman* graph)
{
    point3 pos = graph->GetPos3();
    IWorld::Get()->GetEffects()->CreateHitEffect("hit_medikit", pos, pos, pos, 1.7f); 
}

void EffectMgr::MakeHumanBlood(const point3& from, const point3& to)
{
#if !defined(NO_BLOOD)
    IWorld::Get()->GetEffects()->CreateHitEffect("hit_blood", from, to, to, 1.0);
    IWorld::Get()->GetEffects()->CreateHitEffect("hit_blood", 2*to - from, to, to, 1.0);
#endif
}

void EffectMgr::MakeBloodSpot(const point3& where)
{
#if !defined(NO_BLOOD)
    point3 ground = where;
    ground.z = DirtyLinks::GetLandHeight(where);
    IWorld::Get()->GetEffects()->CreateHitEffect("hit_blood_spot", ground, ground, ground, 1.0f);
#endif
}

void EffectMgr::MakeVehicleExplosion(const GraphVehicle* graph)
{
    point3 pnt = graph->GetPos3();
    IWorld::Get()->GetEffects()->CreateHitEffect("hit_huge_g_explosion", pnt, pnt, pnt, 1.0); 
}

float EffectMgr::MakeGrenadeFlight(const KeyAnimation& ka, const std::string& skin, point3* end_pnt)
{
    IWorld::Get()->GetEffects()->CreateGrenadeEffect(ka, skin);
    float end_time = ka.GetLastTime();
    if(end_pnt) ka.GetTrans(end_pnt, end_time);
    return Timer::GetSeconds() + end_time;
}

unsigned EffectMgr::MakeConst(const rid_t& rid, const point3& to)
{
    return IWorld::Get()->GetEffects()->CreateMovableEffect(rid, to);
}

void EffectMgr::DestroyConst(unsigned id)
{
    IWorld::Get()->GetEffects()->DestroyEffect(id);
}

void EffectMgr::MakeMeat(const GraphHuman* human, const point3& from)
{
    point3 pos = human->GetPos3();
    pos.z = DirtyLinks::GetLandHeight(pos) + 1.0f;
    IWorld::Get()->GetEffects()->CreateMeatEffect(human->GetGID(), from, pos);
}

//================================================================

GraphThing::GraphThing(gid_t gid, BaseThing* thing) :
    m_gid(gid), m_thing(thing)
{
}

GraphThing::~GraphThing()
{
    if(m_fcan_destroy) IWorld::Get()->GetItemsPool()->Delete(m_gid);
}

void GraphThing::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving()){
     
        st << m_gid;
        st << GetParent()->GetTID();

    }else{

        st >> m_gid;

        tid_t tid;
        st >> tid;

        m_thing = Depot::GetInst()->Get(tid);
    }
}

point3 GraphThing::GetPos3() const
{    
    return IWorld::Get()->GetItemsPool()->Get(m_gid)->GetPos();
}

ipnt2_t GraphThing::GetPos2() const
{
    return HexUtils::scr2hex(GetPos3());
}

void GraphThing::EnableDestroy(bool flag)
{
    m_fcan_destroy = flag;
}

//================================================================

GraphEntity::GraphEntity(gid_t gid, BaseEntity* entity) :
    m_update_flags(UT_ALL), m_gid(gid), m_entity(entity), m_visible(true), m_next(m_first)
{
    //присоед. к списку существ
    m_first = this;

    for(int k = 0; k < ET_MAX; m_show_modes[k++] = SM_HIDE);
    
    if(entity){
        LinkObserver();
        Visible(m_visible);  
    }
}

GraphEntity::~GraphEntity()
{
    //снять наблюдателя и удалить
    m_entity->Detach(this);

    GraphEntity* prev = 0,
               * cur  = m_first;

    //отсоед от списка существ
    while(cur){
        
        if(cur == this){ 

            if(prev)
                prev->m_next = m_next;
            else
                m_first = m_next;

            break;
        }    
        
        prev = cur;
        cur  = cur->m_next;
    }
}

void GraphEntity::Update()
{
		STACK_GUARD("GraphEntity::Update");
    if(m_update_time < Timer::GetSeconds()){

        m_update_time = Timer::GetSeconds() + m_update_delta;

        GraphEntity* first = m_first;
        
        //перемотаем весь список 
        while(first){
            
            //обновим всех кого нужно
            if(first->m_update_flags){
                first->UpdateEntity();
                first->m_update_flags = 0;
            }
            
            first = first->m_next;
        }
    }
}

void GraphEntity::LinkObserver()
{
    //отсоед наблюдателя
    m_entity->Detach(this);

    //присоед все заново
    m_entity->Attach(this, EV_QUIT_TEAM);
    m_entity->Attach(this, EV_JOIN_TEAM);
    m_entity->Attach(this, EV_FOS_CHANGE);
    m_entity->Attach(this, EV_DEATH_PLAYED);
    m_entity->Attach(this, EV_WEAPON_RELOAD);
    m_entity->Attach(this, EV_RELATION_CHANGE);
    m_entity->Attach(this, EV_SELECTION_CHANGE);

    //только для людей
    m_entity->Attach(this, EV_BODY_PACK_CHANGE);
    m_entity->Attach(this, EV_HANDS_PACK_CHANGE);
    m_entity->Attach(this, EV_LONG_DAMAGE_CHANGE);
}

void GraphEntity::Update(BaseEntity* entity, event_t event, info_t info)
{
    switch(event){
    case EV_QUIT_TEAM:
    case EV_JOIN_TEAM:
        m_update_flags |= UT_TEAM_CHANGE;
		
    case EV_RELATION_CHANGE:
        m_update_flags |= UT_BANNER;
        break;

    case EV_WEAPON_RELOAD:
        ChangeAction(AT_RELOAD);
        break;

    case EV_FOS_CHANGE:
        m_update_flags |= UT_FOS;
        break;

    case EV_DEATH_PLAYED:
        m_update_flags |= UT_ALL;
        UpdateEntity();
        break;

    case EV_SELECTION_CHANGE:
        m_update_flags |= UT_SELECTION;
        break;

    case EV_BODY_PACK_CHANGE:
        m_update_flags |= UT_ARMOR;
        break;

    case EV_HANDS_PACK_CHANGE:
        m_update_flags |= UT_HANDS;
        break;

    case EV_LONG_DAMAGE_CHANGE:
        m_update_flags |= UT_DMG_EFFECT;
        break;
    }
}

void GraphEntity::UpdateEntity()
{
    IEntity* graph = IWorld::Get()->GetCharacterPool()->Get(m_gid);

    if(GetUpdateFlags() & UT_TEAM_CHANGE){

        switch(m_entity->GetPlayer()){
        case PT_NONE:
        case PT_ENEMY:
            graph->SetOwnership(IEntity::OS_OTHER);
            break;

        case PT_PLAYER:
            graph->SetOwnership(IEntity::OS_PLAYER);
            break;

        case PT_CIVILIAN:            
            graph->SetOwnership(IEntity::OS_CIVILIAN);
            break;
        }
    }

    if(GetUpdateFlags() & UT_BANNER) graph->SetBanner(GetParent()->GetInfo()->GetName(), GetEntityBannerColor(GetParent()));

    if(GetUpdateFlags() & UT_FOS){

        AIUtils::fos_s fos;
        AIUtils::GetEntityFOS(GetParent(), &fos);
        
        graph->SetFOS(fos.m_front, fos.m_back, fos.m_angle);
    }

    if(GetUpdateFlags() & UT_BASE_FLAGS) Visible(IsVisible());
}

void GraphEntity::Destroy()
{
    if(IsRaised(FT_ENABLE_DESTROY)){
        m_entity->Detach(this);
        IWorld::Get()->GetCharacterPool()->Delete(m_gid);    
    }
}

void GraphEntity::ShowDecor(decor_type type, bool flag)
{
    switch(type){
    case DT_FOS:
        IWorld::Get()->GetCharacterPool()->ShowDecor(flag, CharacterPool::DT_FOS);
        break;

    case DT_BANNER:
        IWorld::Get()->GetCharacterPool()->ShowDecor(flag, CharacterPool::DT_BANNER);
        break;
    };
}
   
void GraphEntity::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving()){

        st << m_gid;
        st << m_entity->GetEID();
        st << m_visible;
        st.Save(m_show_modes, sizeof(m_show_modes));

    }else{
        
        st >> m_gid;

        eid_t eid; st >> eid;
        m_entity = EntityPool::GetInst()->Get(eid);

        st >> m_visible;

        st.Load(m_show_modes, sizeof(m_show_modes));

        IEntity* graph = IWorld::Get()->GetCharacterPool()->Get(m_gid);
        
        for(int k = 0; k < ET_MAX; k++){
            effect_type effect = static_cast<effect_type>(k);
            graph->ShowEffect(GetEffectType(effect), GetShowMode(m_show_modes[effect])); 
        }

        //присоед наблюдателя
        LinkObserver();
    }
}

bool GraphEntity::TraceRay(const ray &r, float *dist, point3 *norm, bool AsBox)
{
    return IWorld::Get()->GetCharacterPool()->Get(m_gid)->TraceRay(r, dist, norm, AsBox);
}
    
void GraphEntity::SetLoc(const ipnt2_t& pnt, float angle)
{
    SetLoc(HexGrid::GetInst()->Get(pnt), angle);
}

void GraphEntity::SetLoc(const point3& pos, float angle)
{
    IWorld::Get()->GetCharacterPool()->Get(m_gid)->SetLocation(pos, angle);
}
    
float GraphEntity::SetAngle(float angle)
{
    return IWorld::Get()->GetCharacterPool()->Get(m_gid)->SetAngle(Timer::GetSeconds(), angle);
}

float GraphEntity::GetAngle() const
{
    return IWorld::Get()->GetCharacterPool()->Get(m_gid)->GetAngle();
}

point3 GraphEntity::GetPos3() const
{
    return IWorld::Get()->GetCharacterPool()->Get(m_gid)->GetLocation();
}

ipnt2_t GraphEntity::GetPos2() const
{
    return HexUtils::scr2hex(GetPos3());
}

void GraphEntity::GetBarrel(const point3& barrel, point3* from)
{
    IWorld::Get()->GetCharacterPool()->Get(m_gid)->GetBarrel(barrel, from);
}

void GraphEntity::GetShellOutlet(const point3& offset, point3* from)
{
    IWorld::Get()->GetCharacterPool()->Get(m_gid)->GetBarrel(offset, from);
}

void GraphEntity::Visible(bool flag)
{
    m_visible = flag;
    IWorld::Get()->GetCharacterPool()->Get(m_gid)->Enable(m_visible || m_flags & FT_ALWAYS_VISIBLE);
}

void GraphEntity::MoveTo(const ipnt2_t& to)
{
    IWorld::Get()->GetCharacterPool()->Get(m_gid)->SetDestination(HexGrid::GetInst()->Get(to));
}

float GraphEntity::ChangeAction(action_type action)
{
    IEntity* graph = IWorld::Get()->GetCharacterPool()->Get(m_gid);

    switch(action){
    case AT_TURN:
        return graph->SetAction(TURN, Timer::GetSeconds());

    case AT_MOVE:
        return graph->SetAction(WALK, Timer::GetSeconds());

    case AT_STAY:
        return graph->SetAction(STAY, Timer::GetSeconds());

    case AT_DEAD:
        return graph->SetAction(DEAD, Timer::GetSeconds());

    case AT_HURT:
        return graph->SetAction(HURT, Timer::GetSeconds());

    case AT_FALL:
        return graph->SetAction(FALL, Timer::GetSeconds());

    case AT_FEAR:
        return graph->SetAction(PANIC, Timer::GetSeconds());

    case AT_RELOAD:
        return graph->SetAction(RELOAD, Timer::GetSeconds());

    case AT_SPECIAL:
        return graph->SetAction(SPECANIM, Timer::GetSeconds());

    case AT_USE:
        return graph->SetAction(USE, Timer::GetSeconds()); 

    case AT_LANDING:
    case AT_SHIPMENT:
        return 0;

    case AT_SHOOT:
        return graph->SetAction(SHOOT ,Timer::GetSeconds());

    case AT_RECOIL:
        return graph->SetAction(SHOOTRECOIL ,Timer::GetSeconds());

    case AT_AIMSHOOT:
        return graph->SetAction(AIMSHOOT ,Timer::GetSeconds());

    case AT_AIMRECOIL:
        return graph->SetAction(AIMSHOOTRECOIL ,Timer::GetSeconds());
    }

    return 0;
}

void GraphEntity::ShowEffect(effect_type type, show_mode mode)
{   
    //если эффект уже находися в этом режиме
    if(m_show_modes[type] == mode) return;

    m_show_modes[type] = mode;
    IWorld::Get()->GetCharacterPool()->Get(m_gid)->ShowEffect(GetEffectType(type), GetShowMode(mode)); 
}

bool GraphEntity::NeedRotate(const point3& dir)
{
    return IWorld::Get()->GetCharacterPool()->Get(m_gid)->NeedRotation(dir);
}

bool GraphEntity::IsRaised(unsigned flag)
{
    return m_flags & flag;
}

void GraphEntity::DropFlags(unsigned flag)
{
    m_flags &= ~flag;

    for(GraphEntity* ptr = m_first; ptr; ptr = ptr->m_next)
        ptr->m_update_flags |= UT_BASE_FLAGS; 
}

void GraphEntity::RaiseFlags(unsigned flag)
{
    m_flags |= flag;

    for(GraphEntity* ptr = m_first; ptr; ptr = ptr->m_next)
        ptr->m_update_flags |= UT_BASE_FLAGS; 
}

point3 GraphEntity::GetShotPoint(const point3& hint) const
{
    return IWorld::Get()->GetCharacterPool()->Get(m_gid)->GetShotPoint(hint);
}

void GraphEntity::GetVisPoints(pnt3_vec_t* pnts) const
{
    IWorld::Get()->GetCharacterPool()->Get(m_gid)->GetLOSPoints(pnts);
}

//================================================================

GraphHuman::GraphHuman(gid_t gid, BaseEntity* entity):
    GraphEntity(gid, entity), m_move(MT_NONE), m_pose(PT_NONE),
    m_bridge(new CallbackBridge()), m_stance(ST_USUAL), m_hold(HT_FREEHANDS)
{       
    if(!entity) return;
    
    Person* person = IWorld::Get()->GetCharacterPool()->Get(GetGID())->GetHuman();

    if(HumanEntity* human = entity->Cast2Human()){
        person->Load(human->GetInfo()->GetModel(), human->GetInfo()->GetSkins(), human->GetInfo()->GetRID());

        m_move = MT_WALK;
        m_pose = PT_STAND;
        m_stance = ST_USUAL;
    }
        
    if(TraderEntity* trader = entity->Cast2Trader()){
        std::string graph_aux[6];
        
        graph_aux[0] = trader->GetInfo()->GetSkin();    
        graph_aux[1] = "-";
        graph_aux[2] = "-";
        graph_aux[3] = "-";
        graph_aux[4] = "-";
        graph_aux[5] = "-";
        
        person->Load(trader->GetInfo()->GetModel(), graph_aux, trader->GetInfo()->GetRID());
    }
}

GraphHuman::~GraphHuman()
{  
    delete m_bridge;
}

void GraphHuman::Move(const pnt_vec_t& path, MoveEvents* events)
{
    m_bridge->SetEvents(events);
    IWorld::Get()->GetCharacterPool()->Get(GetGID())->StartMoving(m_move == MT_RUN ? RUN : WALK, path, m_bridge);
}

void GraphHuman::UpdateEntity()
{
    GraphEntity::UpdateEntity();

    Person* person = IWorld::Get()->GetCharacterPool()->Get(GetGID())->GetHuman();

    if(HumanEntity* human = GetParent()->Cast2Human()){

        HumanContext* context = human->GetEntityContext();

        if(GetUpdateFlags() & UT_HANDS){

            BaseThing* thing = context->GetThingInHands(TT_WEAPON|TT_THROWABLE);
            const ThingInfo* info = thing ? thing->GetInfo() : 0;

            if(info == 0)
                SetWeapon(HT_FREEHANDS, rid_t());
            else if(info->IsGrenade())
                SetWeapon(HT_GRENADE, static_cast<const GrenadeInfo*>(info)->GetHoldModel());
            else if(info->IsCamera())
                SetWeapon(HT_GRENADE, static_cast<const CameraInfo*>(info)->GetHoldModel());
            else if(info->IsShield())
                SetWeapon(HT_GRENADE, static_cast<const ShieldInfo*>(info)->GetHoldModel());
            else if(info->IsScanner())
                SetWeapon(HT_FREEHANDS, static_cast<const ScannerInfo*>(info)->GetHoldModel());
            else if(info->IsWeapon())
                SetWeapon(static_cast<const WeaponInfo*>(info)->GetHoldType(), static_cast<const WeaponInfo*>(info)->GetHoldModel());
        }

        if(GetUpdateFlags() & UT_ARMOR){        
            HumanContext::iterator itor = context->begin(HPK_BODY, TT_ARMOUR);
            SetSuit(itor != context->end() ? static_cast<ArmorThing*>(&*itor)->GetInfo()->GetSuit() : ARM_NORM);
        }
        
        if(GetUpdateFlags() & UT_DMG_EFFECT){
            ShowEffect(GraphEntity::ET_FLAME, context->HaveLongDamage(DT_FLAME) ? GraphEntity::SM_SHOW : GraphEntity::SM_SLOW_HIDE);
            // ---- нет эффекта ---- ShowEffect(GraphEntity::ET_SHOCK, context->HaveLongDamage(DT_SHOCK) ? GraphEntity::SM_SHOW : GraphEntity::SM_SLOW_HIDE);
            // ---- нет эффекта ---- ShowEffect(GraphEntity::ET_ELECTRIC, context->HaveLongDamage(DT_ELECTRIC) ? GraphEntity::SM_SHOW : GraphEntity::SM_SLOW_HIDE);
        }

        if(GetUpdateFlags() & UT_SELECTION) ShowEffect(GraphEntity::ET_SELECTION, context->IsSelected() ? GraphEntity::SM_SHOW : GraphEntity::SM_HIDE);

    }else{

        SetWeapon(HT_FREEHANDS, rid_t());
        SetSuit(ARM_NORM);    
    }
}
    
void GraphHuman::ChangePose(pose_type type)
{
    Person*      person = IWorld::Get()->GetCharacterPool()->Get(GetGID())->GetHuman();
    HumanEntity* human  = GetParent()->Cast2Human();

    pose_type old_pose = m_pose;

    switch(type){
    case PT_STAND:
        {
            m_pose = PT_STAND;        

            HumanContext*          context = human->GetEntityContext();
            HumanContext::iterator itor = context->begin(HPK_BODY, TT_ARMOUR);
            
            if(itor != context->end() && static_cast<ArmorThing*>(&*itor)->GetInfo()->IsSpaceSuit())
                person->SetStanding(SUITSTAY, Timer::GetSeconds());
            else
                person->SetStanding(m_stance == ST_USUAL ? PEACESTAY : WARSTAY, Timer::GetSeconds());   
        }
        break;

    case PT_SIT:
        if(human == 0 || human->GetEntityContext()->CanSit()){
            m_pose = PT_SIT;
            person->SetStanding(SIT, Timer::GetSeconds());
        }
        break;
    }

    if(human){    

        //подсчет ходов для человека при смене позы
        if((m_pose == PT_SIT && old_pose == PT_STAND) || (m_pose == PT_STAND && old_pose == PT_SIT))
            human->GetEntityContext()->GetTraits()->AddMovepnts(- MPS_FOR_POSE_CHANGE);
        
        if(m_pose != old_pose) human->Notify(EntityObserver::EV_POSE_TYPE_CHANGE);
    }
}

void GraphHuman::ChangeStance(stance_type type)
{
    m_stance = type;
    ChangePose(m_pose);
}
    
void GraphHuman::ChangeMoveType(move_type type)
{
    move_type old_move = m_move;

    HumanEntity* human = GetParent()->Cast2Human();

    m_move = (human && human->GetEntityContext()->CanRun()) ? type : MT_WALK;

    if(human && old_move != m_move) human->Notify(EntityObserver::EV_MOVE_TYPE_CHANGE);
}

float GraphHuman::ChangeAction(action_type action)
{
    if(action == AT_MOVE){
        Person* person = IWorld::Get()->GetCharacterPool()->Get(GetGID())->GetHuman();
        return m_move == MT_RUN ? person->SetAction(RUN, Timer::GetSeconds()) : person->SetAction(WALK, Timer::GetSeconds());
    }

    return GraphEntity::ChangeAction(action);
}

void GraphHuman::MakeSaveLoad(SavSlot& st)
{
    GraphEntity::MakeSaveLoad(st);

    if(st.IsSaving()){

        st << static_cast<int>(m_pose);
        st << static_cast<int>(m_move);
        st << static_cast<int>(m_stance);

    }else{

        int tmp;

        st >> tmp; ChangePose(static_cast<pose_type>(tmp));
        st >> tmp; ChangeMoveType(static_cast<move_type>(tmp));
        st >> tmp; ChangeStance(static_cast<stance_type>(tmp));
    }
}

void GraphHuman::SetSuit(armor_type type)
{
    Person* person = IWorld::Get()->GetCharacterPool()->Get(GetGID())->GetHuman();

    switch(type){
    case ARM_NORM:
        person->SetArmour(Person::ARM_NORM);
        break;

    case ARM_SUIT1:
        person->SetArmour(Person::ARM_SUIT1);
        break;

    case ARM_SUIT2:
        person->SetArmour(Person::ARM_SUIT2);
        break;

    case ARM_SUIT3:
        person->SetArmour(Person::ARM_SUIT3);
        break;

    case ARM_SUIT4:
        person->SetArmour(Person::ARM_SUIT4);
        break;

    case ARM_SCUBA:
        person->SetArmour(Person::ARM_SCUBA);
        break;
    }

    //некоторые видиды броников имеют специфическую стойку
    ChangeStance(m_stance);
}

void GraphHuman::SetWeapon(hold_type type, const std::string& name)
{
    m_hold = type;

    Person* person = IWorld::Get()->GetCharacterPool()->Get(GetGID())->GetHuman();

    switch(type){
    case HT_RIFLE:
        person->SetWeapon(RIFLE, name, Timer::GetSeconds());
        break;

    case HT_PISTOL:
        person->SetWeapon(PISTOL, name, Timer::GetSeconds());
        break;

    case HT_CANNON:
        person->SetWeapon(CANNON, name, Timer::GetSeconds());
        break;
    
    case HT_AUTOCANNON:
        person->SetWeapon(AUTOCANNON, name, Timer::GetSeconds());
        break;
    
    case HT_ROCKET:
        person->SetWeapon(ROCKET, name, Timer::GetSeconds());
        break;
    
    case HT_SPECIAL:
        person->SetWeapon(SPECIAL, name, Timer::GetSeconds());
        break;

    case HT_GRENADE:
        person->SetWeapon(GRENADE, name, Timer::GetSeconds());
        break;

    case HT_FREEHANDS:
        person->SetWeapon(FREEHANDS, name, Timer::GetSeconds());
        break;
    }

    //c некоторыми видами оружия unit не бегает
    ChangeMoveType(m_move);
}

void GraphHuman::PlaySound(sound_type sound)
{
//    theLog("SndPlayer::GetInst()->CanPlay(): returns == %i\n", static_cast<int>(SndPlayer::GetInst()->CanPlay()));
    if(SndPlayer::GetInst()->CanPlay()) IWorld::Get()->GetCharacterPool()->Get(GetGID())->GetHuman()->HandleSoundEvent(GHSound2SPSound(sound));
}

//================================================================

GraphVehicle::GraphVehicle(gid_t gid, BaseEntity* entity):
    GraphEntity(gid, entity), m_bridge(new CallbackBridge())
{
    if(entity){

        const VehicleInfo* info =  entity->Cast2Vehicle()->GetInfo();

        Vehicle::VehSounds sounds;

        sounds.Walk = SndUtils::MakeSoundFileName(info->GetMoveSound());
        sounds.Hurt = SndUtils::MakeSoundFileName(info->GetDamageSound());
        sounds.Death = SndUtils::MakeSoundFileName(info->GetExplodeSound());
        sounds.Engine = SndUtils::MakeSoundFileName(info->GetEngineSound());

        IWorld::Get()->GetCharacterPool()->Get(GetGID())->GetVehicle()->Load(info->GetModel(), sounds);
    }
}

GraphVehicle::~GraphVehicle()
{
    delete m_bridge;
}

void GraphVehicle::Move(const pnt_vec_t& path, MoveEvents* events)
{
    m_bridge->SetEvents(events);
    IWorld::Get()->GetCharacterPool()->Get(GetGID())->StartMoving(WALK, path, m_bridge);
}

void GraphVehicle::UpdateEntity()
{
    GraphEntity::UpdateEntity();

    if(GetUpdateFlags() & UT_SELECTION){        
        VehicleContext* context = GetParent()->Cast2Vehicle()->GetEntityContext();
        ShowEffect(GraphEntity::ET_SELECTION, context->IsSelected() ? GraphEntity::SM_SHOW : GraphEntity::SM_HIDE);
    }
}

//================================================================

GraphFactory* GraphFactory::GetInst()
{
    static GraphFactory imp;
    return &imp;
}

GraphHuman* GraphFactory::CreateGraphHuman(BaseEntity* human)
{
    gid_t gid = IWorld::Get()->GetCharacterPool()->CreateHuman();
    return new GraphHuman(gid, human);
}

GraphVehicle* GraphFactory::CreateGraphVehicle(BaseEntity* vehicle)
{
    gid_t gid = IWorld::Get()->GetCharacterPool()->CreateVehicle();
    return new GraphVehicle(gid, vehicle);
}

GraphThing* GraphFactory::CreateGraphThing(BaseThing* thing, const ipnt2_t& pos)
{
    point3 pos3 = HexGrid::GetInst()->Get(pos);

    float dx = 0.45*NormRand(),
          dy = 0.45*NormRand();

    pos3.x += NormRand() < 0.5 ? -dx : dx;
    pos3.y += NormRand() < 0.5 ? -dy : dy; 

    gid_t gid = IWorld::Get()->GetItemsPool()->Create(thing->GetInfo()->GetItemModel(), pos3);
    return new GraphThing(gid, thing);
}


