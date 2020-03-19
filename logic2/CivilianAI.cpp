/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: интеллект мирных жителей
                                                                                
                                                                                
   Author: Alexander Garanin (Punch)
		   Mikhail L. Lepakhin (Flif)
***************************************************************/                
#pragma warning(disable:4786)
#include "logicdefs.h"
#include "PathUtils.h"
#include "HexUtils.h"
#include "HexGrid.h"
#include "Activity.h"

#include "CivilianAI.h"

#include "AIUtils.h"
#include "thing.h"
#include "strategy.h"
#include "Entity.h"
#include "player.h"
#include "VisUtils3.h"
#include "DirtyLinks.h"
#include "Graphic.h"
#include "ThingFactory.h"
#include "EnemyDetection.h"
#include "AIAPI.h"

#include "..\Interface\Console.h"

// пространство имен без имени - для функций-утилит этого файла
namespace
{
	// вывести в консоль
	void Print(eid_t entity, const std::string& str);
	// радиус для торгового бота
	const float trader_bot_radius = 10.0f;
};

/////////////////////////////////////////////////////////////////////////////
//
// узел на автобус
//
/////////////////////////////////////////////////////////////////////////////
BusNode::BusNode(eid_t ent_id, BusRoute* route, bus_type  type) :
    m_entity(ent_id), m_mode(BM_INIT), m_delay(0), m_route(route), m_activity(0)
{
    if(!ent_id) return;

    //переставить менеджер погрузки
    VehicleEntity*  vehicle = EntityPool::GetInst()->Get(m_entity)->Cast2Vehicle();
    VehicleContext* context = vehicle->GetEntityContext();
    
    if(type == BT_USUAL)
        context->SetCrewStrategy(StrategyFactory::GetInst()->CreateCrew(StrategyFactory::CT_BUS, vehicle));
    else
        context->SetCrewStrategy(StrategyFactory::GetInst()->CreateCrew(StrategyFactory::CT_BUSY, vehicle));
 
    Hide();
    Spawn();

    BusDispatcher::GetInst()->Insert(vehicle, m_route);
}

BusNode::~BusNode()
{
	VehicleEntity* vehicle = EntityPool::GetInst()->Get(m_entity)->Cast2Vehicle();
	BusDispatcher::GetInst()->Remove(vehicle, m_route);

    delete m_route;
    delete m_activity;
}

float BusNode::Think(state_type* st)
{
    if(st == 0) return 0.0f;

    *st = ST_INCOMPLETE;

    if(m_activity){

        if(!m_activity->Run(AC_TICK)){
            delete m_activity;
            m_activity = 0;
        }

        return 0.0f;
    }

    while(true){
        switch(m_mode){
        case BM_INIT:
            if(OnInit()) return 0.0f;
            break;

        case BM_DEST:
            if(OnDest()) return 0.0f;
            break;
            
        case BM_STOP:
            if(OnStop()) return 0.0f;
            break;
            
        case BM_SPAWN:
            if(OnRespawn()) return 0.0f;
            break;

        case BM_MOVE:
            if(OnMove()) return 0.0f;
            break;

        case BM_QUIT:
            *st = ST_FINISHED;
            m_mode = BM_INIT;
            return 1.0f;            
        }
    }
}

bool BusNode::OnInit()
{
    //перейдем к след. точке
    VehicleEntity* vehicle = EntityPool::GetInst()->Get(m_entity)->Cast2Vehicle();
    ipnt2_t        cur_pos = vehicle->GetGraph()->GetPos2(); 

    //обработка последней точки маршрута
    if(m_route->GetPntType(cur_pos) == BusRoute::PT_LAST){
        m_mode = BM_SPAWN;
        return false;
    }

    //если остановились посредине пути и нам мешает техника
    if(m_route->GetPntType(cur_pos) == BusRoute::PT_PATH && !CanMove()){
        m_mode = BM_QUIT;
        return false;
    }

    //пройдемся по маршруту
    pnt_vec_t path;
    m_route->GetPath(cur_pos, &path);

    //перейдем в режим ходьбы
    m_activity = ActivityFactory::GetInst()->CreateMove(vehicle, path, ActivityFactory::CT_BUS_MOVE);
    m_mode = BM_MOVE;
    return true;
}

bool BusNode::OnMove()
{
    VehicleEntity* vehicle = EntityPool::GetInst()->Get(m_entity)->Cast2Vehicle();
    ipnt2_t        cur_pos = vehicle->GetGraph()->GetPos2();
    
    //остановимся если не можем перейти на новую точку или это остановка
    if(m_route->GetPntType(cur_pos) == BusRoute::PT_STOP){
        m_mode = BM_STOP;
        return false;
    }

    //если конечная остановка
    if(m_route->GetPntType(cur_pos) == BusRoute::PT_DEST){
        m_mode = BM_DEST;
        return false;
    }

    //если конец пути
    if(m_route->GetPntType(cur_pos) == BusRoute::PT_LAST){
        Hide();
        m_delay = RangeRand(BUS_AND_CAR_STOP_DURATION);
        m_mode  = BM_STOP;
        return false;
    }

    m_mode = BM_STOP;
    return false;
}

bool BusNode::OnDest()
{
    VehicleEntity* veh = EntityPool::GetInst()->Get(m_entity)->Cast2Vehicle(); 
	veh->Cast2Vehicle()->GetEntityContext()->Select(false);
    //запретить посадку
    veh->GetEntityContext()->EnableShipment(false);

    //найти точки высадки
    pnt_vec_t lands;
    PathUtils::GetInst()->CalcLandField(veh, &lands);

    //найти всех и высадить
    EntityPool::iterator first = EntityPool::GetInst()->begin(ET_HUMAN, PT_ALL_PLAYERS, EA_INCREW);
    while(veh->GetEntityContext()->GetCrewSize() && first != EntityPool::GetInst()->end()){
        
        HumanEntity* human = first->Cast2Human();

        //человек из этого автобуса?
        if(     human->GetEntityContext()->GetCrew() 
            &&  human->GetEntityContext()->GetCrew()->GetEID() == m_entity){

            veh->GetEntityContext()->DecCrew(human);

            if(lands.empty()) throw CASUS("BusNode::OnDest: не хватет точек высадки!");
            
            int rnd_pnt = rand() % lands.size();
            human->GetGraph()->SetLoc(lands[rnd_pnt], human->GetGraph()->GetAngle());
            PathUtils::GetInst()->LinkEntity(human);

            lands.erase(lands.begin() + rnd_pnt);
        }

        ++first;
    }

    m_mode = BM_QUIT;
    return false;
}

bool BusNode::OnStop()
{
    VehicleEntity* veh = EntityPool::GetInst()->Get(m_entity)->Cast2Vehicle(); 


    //разрешить посадку
		veh->GetEntityContext()->EnableShipment(true);    
    m_mode = BM_QUIT;
    return false;        
}

bool BusNode::OnRespawn()
{
    //уменьшим задержку и выйдем
    if(m_delay){
        m_delay --;
        m_mode = BM_QUIT;
        return false;
    }

    BaseEntity* entity = EntityPool::GetInst()->Get(m_entity);

    BusRoute::pos_s pos;
    
    m_route->GenLoc(true, &pos);
    
    pnt_vec_t track;
    
    PathUtils::GetInst()->CalcTrackField(entity, pos.m_pos, &track);
    
    AreaManager area(track);

    //если место занято
    if(area.begin() != area.end()){
        m_mode = BM_QUIT;
        return false;
    }
        
    //присорединяем автобус
    entity->GetGraph()->SetLoc(pos.m_pos, pos.m_angle);
        
    VisMap::GetInst()->Insert(entity->GetEntityContext()->GetMarker());
    VisMap::GetInst()->Insert(entity->GetEntityContext()->GetSpectator());
    
    PathUtils::GetInst()->LinkEntity(entity);

    m_mode = BM_INIT;
    return false;
}

void BusNode::Hide()
{
    BaseEntity* entity = EntityPool::GetInst()->Get(m_entity);

    VisMap::GetInst()->Remove(entity->GetEntityContext()->GetMarker());
    VisMap::GetInst()->Remove(entity->GetEntityContext()->GetSpectator());

    PathUtils::GetInst()->UnlinkEntity(entity); 
	
	//entity->RaiseFlags(EA_INCREW);
    entity->GetGraph()->Visible(false);
}

void BusNode::Spawn()
{
    BaseEntity* entity = EntityPool::GetInst()->Get(m_entity);
	//entity->DropFlags(EA_INCREW);

    while(true){

        BusRoute::pos_s pos;

        m_route->GenLoc(false, &pos);
        
        pnt_vec_t track;

        PathUtils::GetInst()->CalcTrackField(entity, pos.m_pos, &track);

        AreaManager area(track);

        if(area.begin() == area.end()){

            entity->GetGraph()->SetLoc(pos.m_pos, pos.m_angle);
            
            VisMap::GetInst()->Insert(entity->GetEntityContext()->GetMarker());
            VisMap::GetInst()->Insert(entity->GetEntityContext()->GetSpectator());
            
            PathUtils::GetInst()->LinkEntity(entity);

            return;
        }            
    }
}

void BusNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving()){

        st << m_entity;
        st << static_cast<int>(m_mode);
        st << m_delay;

        DynUtils::SaveObj(st, m_route);
        m_route->MakeSaveLoad(st);

    }else{

        st >> m_entity;
        int tmp; st >> tmp; m_mode = static_cast<bus_mode>(tmp);
        st >> m_delay;

        DynUtils::LoadObj(st, m_route);
        m_route->MakeSaveLoad(st);

        VehicleEntity* vehicle = EntityPool::GetInst()->Get(m_entity)->Cast2Vehicle();
        BusDispatcher::GetInst()->Insert(vehicle, m_route);
    }
}

bool BusNode::CanMove() const
{
    VehicleEntity* veh = EntityPool::GetInst()->Get(m_entity)->Cast2Vehicle();

    pnt_vec_t track;

    PathUtils::GetInst()->CalcTrackField(veh, veh->GetGraph()->GetPos2(), &track);

    AreaManager area(track);
    AreaManager::iterator itor = area.begin();

    while(itor != area.end()){

        if(itor->GetEID() != m_entity)
            return false;

        ++itor;
    }

    return true;
}

// умри!
bool BusNode::die()
{
	m_mode = BM_KILLED;
	return need2Delete();
}

// нужно ли удалять узел
bool BusNode::need2Delete() const
{
	if( (m_mode == BM_KILLED) && !m_activity) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// Узел для машин с нецивилизованным движением
//
/////////////////////////////////////////////////////////////////////////////
FeralCarNode::FeralCarNode(eid_t ent, CheckPoints* points) : 
    m_checks(points), m_mode(CM_INIT), m_delay(0), m_ent(ent), m_activity(0) 
{
    GameEvMessenger::GetInst()->Attach(this, EV_KILL);

    if(!m_ent) return;

    VehicleEntity* vehicle = EntityPool::GetInst()->Get(m_ent)->Cast2Vehicle();
    //переставим менеджер загрузки
    vehicle->GetEntityContext()->SetCrewStrategy(StrategyFactory::GetInst()->CreateCrew(StrategyFactory::CT_BUSY, vehicle));
    //переставим сценарий смерти
    vehicle->GetEntityContext()->SetDeathStrategy(StrategyFactory::GetInst()->CreateDeath(StrategyFactory::DT_FERALCAR, vehicle));

    Hide();

    m_checks->First();

    while(true){

        int skip = RangeRand(10);

        while(skip--) m_checks->Next();

        if(m_checks->IsEnd()) m_checks->First();

        CheckPoints::pos_s pos;
        m_checks->GetPos(&pos);

        pnt_vec_t track;
        
        PathUtils::GetInst()->CalcTrackField(vehicle, pos.m_pos, &track);
        
        AreaManager area(track);
        
        if(area.begin() == area.end()){
      
            vehicle->GetGraph()->SetLoc(pos.m_pos, vehicle->GetGraph()->GetAngle());
            
            VisMap::GetInst()->Insert(vehicle->GetEntityContext()->GetMarker());
            VisMap::GetInst()->Insert(vehicle->GetEntityContext()->GetSpectator());
            
            PathUtils::GetInst()->LinkEntity(vehicle);

            return;
        }
    }
}

FeralCarNode::~FeralCarNode()
{
    GameEvMessenger::GetInst()->Detach(this);
    delete m_checks;
    delete m_activity;
}

float FeralCarNode::Think(state_type* st)
{
    if(st == 0) return 0.0f;
    *st = ST_INCOMPLETE;

    if(m_activity){

        if(!m_activity->Run(AC_TICK)){
            delete m_activity;
            m_activity = 0;
        }

        return 0.0f;
    }

    while(true){
        switch(m_mode){
        case CM_QUIT:
            *st = ST_FINISHED;
            m_mode = CM_INIT;
            return 1.0f;
            
        case CM_SPAWN:
            if(OnSpawn()) return 0.0f;
            break;

        case CM_INIT:
            if(OnInit()) return 0.0f;
            break;

        case CM_DELAY:
            if(OnDelay()) return 0.0f;
            break;
        }
    }
}

void FeralCarNode::Update(subject_t subj, event_t event, info_t info)
{
    if(static_cast<kill_info*>(info)->m_victim->GetEID() == m_ent){
        Hide();
        m_delay = RangeRand(BUS_AND_CAR_STOP_DURATION);
    }
}

void FeralCarNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving()){

        st << m_ent;
        st << m_delay;
        st << static_cast<int>(m_mode);
        
        DynUtils::SaveObj(st, m_checks);
        m_checks->MakeSaveLoad(st);

    }else{

        st >> m_ent;
        st >> m_delay;
        int tmp; st >> tmp; m_mode =  static_cast<car_mode>(tmp);
        
        DynUtils::LoadObj(st, m_checks);
        m_checks->MakeSaveLoad(st);
    }
}

void FeralCarNode::Hide()
{
    VehicleEntity* vehicle = EntityPool::GetInst()->Get(m_ent)->Cast2Vehicle();    
    PathUtils::GetInst()->UnlinkEntity(vehicle);

    VisMap::GetInst()->Remove(vehicle->GetEntityContext()->GetMarker());
    VisMap::GetInst()->Remove(vehicle->GetEntityContext()->GetSpectator());

    vehicle->GetGraph()->Visible(false);
}

bool FeralCarNode::OnInit()
{
    //если есть задержка по концу пути
    if(m_delay > 0){
        m_mode = CM_DELAY;
        return false;
    }

    VehicleEntity* vehicle = EntityPool::GetInst()->Get(m_ent)->Cast2Vehicle();

    //если мало ходов выйдем
    if(vehicle->GetEntityContext()->GetMovepnts() < 3){
        m_mode = CM_QUIT;
        return false;
    }

    //расчитаем поле проходимости
    PathUtils::GetInst()->CalcPassField(vehicle, 0, 0, PathUtils::F_PASS_ALL_ENTITIES);

    CheckPoints::pos_s pos;
    m_checks->GetPos(&pos);
    
    while(true){     
        
        //расчитаем путь и будем по нему двигаться
        pnt_vec_t path;
        PathUtils::GetInst()->CalcPath(vehicle, pos.m_pos, path);

        if(path.size()){
            m_activity = ActivityFactory::GetInst()->CreateMove(vehicle, path, ActivityFactory::CT_CIVILIAN_CAR_MOVE);
            return true;
        }

        //последняя точка?
        if(m_checks->IsEnd()){
           
            if(vehicle->GetGraph()->GetPos2() == pos.m_pos){
                m_delay = 3;
                Hide();
            }

            m_mode = CM_QUIT;            
            return false;
        }

        //перейдем к следующей точке
        m_checks->Next();
        m_checks->GetPos(&pos); 
    }
}

bool FeralCarNode::OnSpawn()
{
    m_checks->First();

    CheckPoints::pos_s pos;
    m_checks->GetPos(&pos);

    VehicleEntity* vehicle = EntityPool::GetInst()->Get(m_ent)->Cast2Vehicle();

    pnt_vec_t track;
    PathUtils::GetInst()->CalcTrackField(vehicle, pos.m_pos, &track);
    
    AreaManager area(track);
    
    //если место занято 
    if(area.begin() != area.end()){
        m_mode = CM_QUIT;
        return false;
    }

    vehicle->GetGraph()->SetLoc(pos.m_pos, vehicle->GetGraph()->GetAngle());

    VisMap::GetInst()->Insert(vehicle->GetEntityContext()->GetMarker());
    VisMap::GetInst()->Insert(vehicle->GetEntityContext()->GetSpectator());
    
    PathUtils::GetInst()->LinkEntity(vehicle);
        
    m_mode = CM_INIT;
    return false;
}

bool FeralCarNode::OnDelay()
{
    m_delay --;
    m_mode = m_delay > 0 ? CM_QUIT : CM_SPAWN;
    return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// узел для одного существа
//
//
/////////////////////////////////////////////////////////////////////////////
CivilianNode::CivilianNode(eid_t ent_id, CheckPoints* checks) :
    m_entity(ent_id), m_mode(CM_INIT), m_delay(0), m_checks(checks),m_run(0),
    m_time(0), m_anim(AT_NONE), m_duration(0), m_sav_anim(AT_NONE),
    m_enemy(0), m_was_hurt(false), m_activity(0)
{
    GameEvMessenger::GetInst()->Attach(this, EV_HURT);
    GameEvMessenger::GetInst()->Attach(this, EV_SHOOT);
        
    if(m_entity) OnSpawnMsg();
}

CivilianNode::~CivilianNode()
{
	GameEvMessenger::GetInst()->Detach(this);
    delete m_checks;
    delete m_activity;
}

float CivilianNode::Think(state_type* st)
{
    bool flag = true;
	float complexity = 0.0f;

    //проиграть действие
    if(m_activity){

        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK)){
            delete m_activity;
            m_activity = 0;
        }

        return complexity;
    }

    while(flag){
        switch(m_mode){
        case CM_INIT:
            flag = OnInitMode(st);
            break;

        case CM_QUIT:
            flag = OnQuitMode(st);
			complexity = 1.0;
            break;

        case CM_MOVE:
            flag = OnMoveMode(st);
            break;

        case CM_ENDMOVE:
            flag = OnEndMoveMode(st);
            break;

        case CM_RUNAWAY:
            flag = OnRunAwayMode(st);
            break;

        case CM_STOP:
            flag = OnStopMode(st);
            break;

        case CM_THING_SEARCH:
            flag = OnThingSearchMode(st);
            break;

        case CM_THING_TAKE:
            flag = OnThingTakeMode(st);
            break;

        case CM_ATTACK:
            flag = OnAttackMode(st);
            break;

        case CM_WEAPON2HANDS:
            flag = OnWeapon2Hands(st);
            break;

        case CM_WEAPON_LOAD:
            flag = OnLoadWeaponMode(st);
            break;
        }
    }

    PlayAnim(st);
	return complexity;
}

void CivilianNode::PlayAnim(state_type* st)
{
    if(m_anim == AT_NONE 
        ||  m_duration <= 0 
        ||  m_time > Timer::GetSeconds())
        return;

    //посчитем ходы
    if(st) m_duration--;

    HumanEntity* human = EntityPool::GetInst()->Get(m_entity)->Cast2Human();

    switch(m_anim){
    case AT_FEAR:
        m_time = human->GetGraph()->ChangeAction(GraphEntity::AT_FEAR);
        break;

    case AT_SPECIAL:
        m_time = human->GetGraph()->ChangeAction(GraphEntity::AT_SPECIAL);
        break;
    }
}

bool CivilianNode::OnLoadWeaponMode(state_type* st)
{
    if(st == 0) return false;

    if(m_time < Timer::GetSeconds()){
        m_mode = CM_WEAPON2HANDS;
        return true;
    }

    return false;
}

bool CivilianNode::OnWeapon2Hands(state_type* st)
{
    if(st == 0) return false;

    HumanEntity*  human   = EntityPool::GetInst()->Get(m_entity)->Cast2Human();
    HumanContext* context = human->GetEntityContext();

    //у нас в руках оружие?
    if(WeaponThing* weapon = static_cast<WeaponThing*>(context->GetThingInHands(TT_WEAPON))){
        
        //если оно заряжено выход
        if(weapon->GetAmmo()){
            m_mode = CM_INIT;
            return true;
        }
        
        //найдем патроны
        HumanContext::iterator itor = context->begin(HPK_ALL, TT_AMMO);
        while(itor != context->end()){

            AmmoThing* ammo = static_cast<AmmoThing*>(&*itor);
            
            //если они подходят зарядим
            if(weapon->GetInfo()->GetCaliber() == ammo->GetInfo()->GetCaliber()){
                context->RemoveThing(itor);
                weapon->Load(ammo);
                
                m_mode = CM_WEAPON_LOAD;
                m_time = human->GetGraph()->ChangeAction(GraphEntity::AT_RELOAD);
                return false;
            }

            ++itor;
        }

        //поищем патроны
        m_mode = CM_THING_SEARCH;
        return true;
    }

    //если руки заняты
    if(BaseThing* thing = context->GetThingInHands()){
        context->RemoveThing(thing);
        context->InsertThing(HPK_BACKPACK, thing);
    }
    
    // поищем у себя подходящую пушку
    HumanContext::iterator itor = context->begin(HPK_ALL, TT_WEAPON);
    while(itor != context->end())
	{
        BaseThing* thing = &*itor;
		// проверим, можем ли мы ее использовать
		if(ThingDelivery::GetInst()->CanInsert(ThingDelivery::ST_USUAL, human, thing, HPK_HANDS))
		{
			// можем использовать
			context->RemoveThing(itor);
			context->InsertThing(HPK_HANDS, thing);
			
			//зарядим оружие
			m_mode = CM_WEAPON2HANDS;
			return true;
		}
		// использовать не сможем
		itor++;
    }
	// подходящей пушки с собой нет

    //поищем пушку
    m_mode = CM_THING_SEARCH;
    return true;
}

bool CivilianNode::OnThingSearchMode(state_type* st)
{
    if(st == 0) return false;
 
    HumanEntity* human = EntityPool::GetInst()->Get(m_entity)->Cast2Human();
    
    //осмотримся, найдем ближайший предмет, который видим
    ipnt2_t pos;
	if(!AIAPI::getInst()->getWeaponLocation(m_entity, &pos))
	{
		// оружия и патронов с гранатами не видно
        m_mode = CM_MOVE;
        return true;
	}

    // юнит захочет подобрать оружие с вероятностью 40%
	if(NormRand() > 0.4f)
	{
		// не будем подбирать
        m_mode = CM_MOVE;
        return true;
    }

    pnt_vec_t path;

    // подберем его
    PathUtils::GetInst()->CalcPassField(human, 0, 0, PathUtils::F_PASS_ALL_ENTITIES);
    PathUtils::GetInst()->CalcPath(human, pos, path);

    if(path.size() && (path.front() == pos) &&
		(!HexGrid::GetInst()->Get(pos).IsBusRoute()))
	{
        m_activity = ActivityFactory::GetInst()->CreateMove(human, path, ActivityFactory::CT_CIVILIAN_MOVE);
        m_mode = CM_THING_TAKE;
        return false;
    }

    m_mode = CM_MOVE;
    return true;
}

bool CivilianNode::OnThingTakeMode(state_type* st)
{
    if(st == 0) return false;

    HumanEntity* human = EntityPool::GetInst()->Get(m_entity)->Cast2Human();

	int weapons_num = AIAPI::getInst()->getThingCount(m_entity, TT_WEAPON);
	int ammo_num = AIAPI::getInst()->getThingCount(m_entity, TT_AMMO);
    
    //возьмем ближайший предмет
    Depot::iterator itor = Depot::GetInst()->begin(human->GetGraph()->GetPos2(), 1.0f, TT_WEAPON|TT_AMMO);
    while(itor != Depot::GetInst()->end())
	{
        BaseThing* thing = &*itor;
		switch(thing->GetInfo()->GetType())
		{
		case TT_WEAPON :
			{
				if(weapons_num >= 2)
				{
					// больше оружие не поднимаем
					itor++;
					continue;
				}
				break;
			}
		case TT_AMMO :
			{
				if(ammo_num >= 10)
				{
					// больше патроны не поднимаем
					itor++;
					continue;
				}
				break;
			}
		}

        Depot::GetInst()->Remove(itor++);
        human->GetEntityContext()->InsertThing(HPK_BACKPACK, thing);
    }

    m_mode = CM_INIT;
    return true;
}

bool CivilianNode::OnStopMode(state_type* st)
{
    if(st == 0) return false;
    
    m_delay--;
    if(m_delay <= 0){
        m_mode = CM_INIT;
        m_sav_anim = AT_NONE;
        return true;
    }

    m_mode = CM_QUIT;
    return true;
}

void CivilianNode::Update(subject_t subj, event_t event, info_t ptr)
{
    switch(event)
	{
    case EV_SHOOT:
        {
            shoot_info* info = static_cast<shoot_info*>(ptr);
            OnShootEv(info->m_actor, info->m_point);
        }
        break;

    case EV_HURT:
        {
            hurt_info* info = static_cast<hurt_info*>(ptr);
            OnHurtEv(info->m_killer, info->m_victim);
        }
        break;
    }
}

void CivilianNode::OnShootEv(BaseEntity* who, const point3& to)
{
    BaseEntity* ent = EntityPool::GetInst()->Get(m_entity);
    
    point3 vec   = HexGrid::GetInst()->Get(ent->GetGraph()->GetPos2()) - to;   
    float  dist2 = vec.x*vec.x + vec.y * vec.y + vec.z * vec.z;
    
    if(dist2 < 10.0f*10.0f){
        m_duration = 3;
        m_anim = AT_FEAR;
    }
}

void CivilianNode::OnHurtEv(BaseEntity* killer, BaseEntity* victim)
{    
    //нас ранили?
    if(victim->GetEID() == m_entity){
        HumanEntity* human = victim->Cast2Human();

        m_run  = 10;
        human->GetGraph()->Cast2Human()->ChangeMoveType(GraphHuman::MT_RUN);

        //поставить на проирывание анимацию страха
        m_anim = AT_FEAR;
        m_duration = 3;

        if(killer){            
            //запомним врага
            m_enemy |= killer->GetPlayer();

            //убежать
            if(!HaveWeapon()) m_was_hurt = true;
            m_dir = human->GetGraph()->GetPos3() - killer->GetGraph()->GetPos3();
        }
    }
}

bool CivilianNode::OnInitMode(state_type* st)
{ 
    if(!st) return false;

    //сохраним анимацию
    m_sav_anim = m_anim;
    m_anim  = AT_NONE;

    if(m_was_hurt){
        m_mode = HaveWeapon() ? CM_ATTACK : CM_RUNAWAY;
        m_was_hurt = false;
        return true;
    }

    if(m_delay > 0){
        m_mode = CM_STOP;
        return true;
    }

    HumanEntity* human = EntityPool::GetInst()->Get(m_entity)->Cast2Human();

    //нужно перейти на шаг?
    if(m_run && --m_run <= 0) human->GetGraph()->Cast2Human()->ChangeMoveType(GraphHuman::MT_WALK);
    
    //если у unit'а есть ходы
    if(human->GetEntityContext()->GetTraits()->GetMovepnts() > 3){
        m_mode = m_enemy ? CM_ATTACK : CM_THING_SEARCH;
        return true;
    }

    m_mode = CM_QUIT;
    return false;
}

bool CivilianNode::OnMoveMode(state_type* st)
{
    HumanEntity* human = EntityPool::GetInst()->Get(m_entity)->Cast2Human();

    PathUtils::GetInst()->CalcPassField(human, 0, 0, PathUtils::F_PASS_ALL_ENTITIES);

    //защита от зацикливания
    int iterations = 5*NormRand()+1;

    //опред точку к кот нужно идти
    while(iterations--){

        CheckPoints::pos_s pos;
        m_checks->GetPos(&pos);

        pnt_vec_t path;
        PathUtils::GetInst()->CalcPath(human, pos.m_pos, path);

        int size = path.size();

        if( (path.size()) &&
			(!HexGrid::GetInst()->Get(pos.m_pos).IsBusRoute())
			)
		{
            m_mode = CM_ENDMOVE;
            m_activity = ActivityFactory::GetInst()->CreateMove(human, path, ActivityFactory::CT_CIVILIAN_MOVE);
            return false;
        }

        //пойдем к другой точке
        int rnd_num = 10*NormRand();
        while(rnd_num--){
            m_checks->Next();
            if(m_checks->IsEnd()) m_checks->First();
        }
    } 
    
    m_mode = CM_QUIT;
    return false;
}

bool CivilianNode::OnQuitMode(state_type* st)
{
    if(st){
        *st = ST_FINISHED;
        m_mode = CM_INIT;
        m_anim = m_sav_anim;
    }

    return false;
}

bool CivilianNode::OnEndMoveMode(state_type* st)
{
    HumanEntity* human = EntityPool::GetInst()->Get(m_entity)->Cast2Human();

    CheckPoints::pos_s pos;
    m_checks->GetPos(&pos);

    //дошли до точки
    if(human->GetGraph()->GetPos2() == pos.m_pos){
        
        //рaзвернем существо
        m_activity = ActivityFactory::GetInst()->CreateRotate(human, pos.m_angle, ActivityFactory::CT_CIVILIAN_ROTATE);
        m_mode = CM_QUIT;

        if(pos.m_fspecial){
            m_delay = 4*NormRand()+1;
            m_sav_anim = AT_SPECIAL;
            m_duration = m_delay;
        }

        return false;        
    }
    
    //дошли, но там нечего делать
    m_mode = CM_INIT;
    return true;
}

bool CivilianNode::OnRunAwayMode(state_type* st)
{
    if(st == 0) return false;

    HumanEntity*  human   = EntityPool::GetInst()->Get(m_entity)->Cast2Human();
    HumanContext* context = human->GetEntityContext();

    PathUtils::GetInst()->CalcPassField(human, 0, 0, PathUtils::F_PASS_ALL_ENTITIES);

    //если мало movepoints
    if(context->GetStepsCount() < 2){
        m_mode = CM_QUIT;
        return true;
    }

    point3    pos3  = HexGrid::GetInst()->Get(human->GetGraph()->GetPos2());
    pnt_vec_t path;

    m_dir = Normalize(m_dir);

    pos3.x += context->GetStepsCount()*m_dir.x;
    pos3.y += context->GetStepsCount()*m_dir.y;

    ipnt2_t pos2 = HexUtils::scr2hex(pos3);
    if( (!HexGrid::GetInst()->IsOutOfRange(pos2)) &&
		(!HexGrid::GetInst()->Get(pos2).IsBusRoute())
		)
	{
        PathUtils::GetInst()->CalcPath(human, pos2, path);

        if(path.size()){
            m_activity = ActivityFactory::GetInst()->CreateMove(human, path, ActivityFactory::CT_CIVILIAN_MOVE);
            m_mode = CM_QUIT;
            return false;
        }
    }
    
    int iterations = 5*NormRand() + 1;

    //попробуем убежать в случайном направлении
    while(iterations--){

        int dx = context->GetStepsCount() + NormRand()*10;
        int dy = context->GetStepsCount() + NormRand()*10;

        pos2.x = human->GetGraph()->GetPos2().x + (NormRand() > 0.5f ? -dx : dx);
        pos2.y = human->GetGraph()->GetPos2().y + (NormRand() > 0.5f ? -dy : dy);

        if( (HexGrid::GetInst()->IsOutOfRange(pos2)) ||
			(HexGrid::GetInst()->Get(pos2).IsBusRoute())
			)
            continue;

        path.clear();
        PathUtils::GetInst()->CalcPath(human, pos2, path);
        
        if(path.size()){
            m_activity = ActivityFactory::GetInst()->CreateMove(human, path, ActivityFactory::CT_CIVILIAN_MOVE);
            m_mode = CM_QUIT;
            return false;
        }
    }

    m_mode = CM_QUIT;
    return true;
}

bool CivilianNode::OnAttackMode(state_type* st)
{ 
    if(st == 0) return false;
    
    HumanEntity* human = EntityPool::GetInst()->Get(m_entity)->Cast2Human();
    WeaponThing* weapon = static_cast<WeaponThing*>(human->GetEntityContext()->GetThingInHands(TT_WEAPON));

    //если оружие не в руках взять его в руки
    if(!weapon || !weapon->GetAmmo()){
        m_mode = CM_WEAPON2HANDS;
        return true;
    }

    //найдем всех перцев в зоне видимости
    VisMap::marker_itor itor = VisMap::GetInst()->marker_begin(human->GetEntityContext()->GetSpectator(), VT_VISIBLE);
    while(itor != VisMap::GetInst()->marker_end()){

        BaseEntity* enemy = itor->GetEntity();
                
        //если это враг
        if(enemy && enemy->GetPlayer() & m_enemy){

            point3 enemy_pos = enemy->GetGraph()->GetShotPoint();

            //прицелимся
            if(AIUtils::CanShoot(human, weapon, enemy_pos)){
                
                m_mode = CM_RUNAWAY;
                m_dir  = human->GetGraph()->GetPos3() - enemy_pos;
                
                //стреляем
                m_activity = ActivityFactory::GetInst()->CreateShoot(human, enemy_pos, AIUtils::NormalizeAccuracy(human, AIUtils::CalcShootAccuracy(human, weapon, enemy_pos)), ActivityFactory::CT_CIVILIAN_SHOOT);
                return false;
            }
        }

        ++itor;
    }
    
    m_mode = CM_THING_SEARCH;
    return false;
}

void CivilianNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving()){

        st << m_entity;
        st << m_run;
        st << m_enemy;
        st << m_delay;
        st << m_was_hurt;
        st << m_duration;

        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_anim);
        st << static_cast<int>(m_sav_anim);

        DynUtils::SaveObj(st, m_checks);
        m_checks->MakeSaveLoad(st);

    }else{

        st >> m_entity;

		BaseEntity* bes = EntityPool::GetInst()->Get(m_entity);

        st >> m_run;
        st >> m_enemy;
        st >> m_delay;
        st >> m_was_hurt;
        st >> m_duration;

        int tmp;

        st >> tmp; m_mode = static_cast<civilian_mode>(tmp);
        st >> tmp; m_anim = static_cast<anim_type>(tmp);
        st >> tmp; m_sav_anim = static_cast<anim_type>(tmp);

        DynUtils::LoadObj(st, m_checks);
        m_checks->MakeSaveLoad(st);
    }
}

void CivilianNode::OnSpawnMsg()
{
    HumanEntity* human = EntityPool::GetInst()->Get(m_entity)->Cast2Human();

    PathUtils::GetInst()->UnlinkEntity(human);
    
    //встанем на начало
    m_checks->First(CheckPoints::FT_SPAWN);

    //установим Unit'а
    CheckPoints::pos_s pos;
    m_checks->GetPos(&pos);
    human->GetGraph()->SetLoc(pos.m_pos, pos.m_angle);
    
    VisMap::GetInst()->Update(human->GetEntityContext()->GetMarker(), VisMap::UE_POS_CHANGE);
    VisMap::GetInst()->Update(human->GetEntityContext()->GetSpectator(), VisMap::UE_POS_CHANGE);

    PathUtils::GetInst()->LinkEntity(human);

    //если есть спец анимация поставить ее на проигрывание
    if(pos.m_fspecial){
        m_mode = CM_STOP;
        m_delay = 3*NormRand();

        m_anim = AT_SPECIAL;
        m_duration = m_delay;
    }
}

bool CivilianNode::HaveWeapon() const
{
    HumanEntity* human = EntityPool::GetInst()->Get(m_entity)->Cast2Human();
    HumanContext::iterator itor = human->GetEntityContext()->begin(HPK_ALL, TT_WEAPON);
    return itor != human->GetEntityContext()->end();
}

// умри!
bool CivilianNode::die()
{
	m_mode = CM_KILLED;
	return need2Delete();
}

// нужно ли удалять узел
bool CivilianNode::need2Delete() const
{
	if( (m_mode == CM_KILLED) && !m_activity) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// узел для человека, который стоит и играет анимацию
//
/////////////////////////////////////////////////////////////////////////////
FixedNPCNode::FixedNPCNode(eid_t ent) :
    m_entity(ent), m_time(0)
{
}

FixedNPCNode::~FixedNPCNode()
{
}

float FixedNPCNode::Think(state_type* st)
{
	float complexity = 0.0f;
    if(st)
	{
		*st = ST_FINISHED;
		complexity = 1.0f;
	}

    if(m_time > Timer::GetSeconds())
        return complexity;

    if(TraderEntity* trader = EntityPool::GetInst()->Get(m_entity)->Cast2Trader())
        m_time = trader->GetGraph()->ChangeAction(GraphEntity::AT_SPECIAL);
    
    if(HumanEntity* human = EntityPool::GetInst()->Get(m_entity)->Cast2Human())
        m_time = human->GetGraph()->ChangeAction(GraphEntity::AT_SPECIAL);

	return complexity;
}

void FixedNPCNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving()){
        
        st << m_entity;

    }else{

        st >> m_entity;
   }
}

/////////////////////////////////////////////////////////////////////////////
//
// узел для рекламного бота
//
/////////////////////////////////////////////////////////////////////////////

// конструктор - id существа
AdvertisingBotNode::AdvertisingBotNode(eid_t id) : m_mode(ABM_SEARCHING),
m_turn_state(TS_NONE), m_entity(id), m_activity(0), m_enemy_id(0),
m_target_pnt(0, 0), m_initialising(false), m_rest_turn(0), m_attack_turn(0)
{
}

AdvertisingBotNode::~AdvertisingBotNode()
{
	delete m_activity;
}

DCTOR_IMP(AdvertisingBotNode)

float AdvertisingBotNode::Think(state_type* st)
{
	float complexity = 0.0f;
    //проиграть действие
    if(m_activity){

        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK)){
			m_activity->Detach(&m_activity_observer);
            delete m_activity;
            m_activity = 0;
        }
        return complexity;
    }

	// если не наш ход - на выход
	if(!st)
	{
		m_turn_state = TS_NONE;
		return complexity;
	}

	// если m_turn_state == TS_NONE, то это начало нашего хода
	if(m_turn_state == TS_NONE)
	{
		m_turn_state = TS_START;
		// начнем инициализацию
		PanicPlayer::GetInst()->Init(EntityPool::GetInst()->Get(m_entity));
		m_initialising = true;
	}

	// проверим флаг инициализации
	if(m_initialising)
	{
		if(PanicPlayer::GetInst()->Execute())
		{
			// нужно продолжать процесс инициализации
			*st = ST_INCOMPLETE;
			return complexity;
		}
		// инициализация закончена
		m_initialising = false;
	}

    // юнит свободен - можно подумать
	bool flag = true;
    while(flag)
	{
        switch(m_mode)
		{
        case ABM_SEARCHING:
            flag = thinkSearching(st);
            break;
        
		case ABM_ATTACK:
            flag = thinkAttack(st);
            break;
        
		case ABM_REST:
            flag = thinkRest(st);
            break;
        }
    }

	if(m_turn_state == TS_END)
	{
		*st = ST_FINISHED;
		m_turn_state = TS_START;
		complexity = 1.0f;
	}
	else *st = ST_INCOMPLETE;
	return complexity;
}

void AdvertisingBotNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());
		st << m_enemy_id;
		st << m_target_pnt.x;
		st << m_target_pnt.y;
		st << m_rest_turn;
		st << m_attack_turn;

		return;
    }
	// читаемся
	int tmp;
	st >> tmp; m_mode = static_cast<AdvertisingBotMode>(tmp);
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> m_entity;
	st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));
	st >> m_enemy_id;
	st >> m_target_pnt.x;
	st >> m_target_pnt.y;
	st >> m_rest_turn;
	st >> m_attack_turn;
	
	// то, что восстанавливается без чтения
	m_activity = 0;
	m_initialising = false;
}

// обработка режима Searching
bool AdvertisingBotNode::thinkSearching(state_type* st)
{
	// 1. найдем всех видимых людей - если кого-нибудь вижу - в атаку
	// 2. никого нет - если дошел до нужной точки выбираем новую
	// 3. если кончились мувпоинты - передаем ход
	// 4. идем в заданную точку

	// 1. найдем всех видимых людей - если кого-нибудь вижу - в атаку
	AIAPI::entity_vector_t entities;
	if(AIAPI::getInst()->getEntitiesVisibleByEntity(m_entity,
		&entities, ET_HUMAN))
	{
		// есть цели
		Print(m_entity, "|ABOT|thinkSearching: вижу цель - в атаку");
		m_enemy_id = AIAPI::getInst()->getRandomEntity(entities);
		m_mode = ABM_ATTACK;
		m_turn_state = TS_INPROGRESS;
		m_attack_turn = 0;
		return false;
	}
	// 2. никого нет - если дошел до нужной точки выбираем новую
	if(m_target_pnt == EntityPool::GetInst()->Get(m_entity)->GetGraph()->GetPos2())
	{
		Print(m_entity, "|ABOT|thinkSearching: дошел до точки - выбираю новую");
		// рассчитаем поле проходимости
		pnt_vec_t allhexes;
		PathUtils::GetInst()->CalcPassField(EntityPool::GetInst()->Get(m_entity),
			0, &allhexes, PathUtils::F_PASS_ALL_ENTITIES);
		if(allhexes.empty())
		{
			// передаем ход - идти некуда
			//Print(m_entity, "|P|thinkPatrol: мне некуда ходить!!!");
			m_turn_state = TS_END;
			return false;
		}
		// получим случайную точку
		m_target_pnt = AIAPI::getInst()->getRandomPoint(allhexes);
	}

	// 3. если кончились мувпоинты - передаем ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		Print(m_entity, "|ABOT|thinkSearching: кончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}
	// 4. идем в заданную точку
	Print(m_entity, "|ABOT|thinkSearching: иду в заданную точку");
	PathUtils::GetInst()->CalcPassField(EntityPool::GetInst()->Get(m_entity),
		0, 0, PathUtils::F_PASS_ALL_ENTITIES);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(EntityPool::GetInst()->Get(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		Print(m_entity, "|ABOT|thinkSearching: заданная точка не достижима");
		m_target_pnt = EntityPool::GetInst()->Get(m_entity)->GetGraph()->GetPos2();
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(EntityPool::GetInst()->Get(m_entity),
		path,
		ActivityFactory::CT_CIVILIAN_CAR_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Attack
bool AdvertisingBotNode::thinkAttack(state_type* st)
{
	// 1. если в состоянии атаки прошло три тура - переход в состояние отдыха
	// 2. если цель умерла или вышла из зоны видимости - в состояние отдыха
	// 3. если кончились мувпоинты - предаем ход
	// 4. выберем точку рядом с целью и пойдем  в нее

	if(m_turn_state == TS_START) m_attack_turn++;
	// 1. если в состоянии атаки прошло три тура - переход в состояние отдыха
	if(m_attack_turn > 3)
	{
		Print(m_entity, "|ABOT|thinkAttack: прошло три тура - на отдых");
		m_mode = ABM_REST;
		m_rest_turn = 0;
		m_turn_state = TS_END;
		return false;
	}
	// 2. если цель умерла или вышла из зоны видимости - в состояние отдыха
	if(!AIAPI::getInst()->isEntityVisible(m_entity, m_enemy_id))
	{
		Print(m_entity, "|ABOT|thinkAttack: цель ушла или погибла - на отдых");
		m_mode = ABM_REST;
		m_rest_turn = 0;
		m_turn_state = TS_END;
		return false;
	}

	// 3. если кончились мувпоинты - предаем ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		Print(m_entity, "|ABOT|thinkAttack: закончились мувпониты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. выберем точку рядом с целью и пойдем  в нее
	Print(m_entity, "|ABOT|thinkAttack: идем к цели");
	// рассчитаем поле проходимости
	PathUtils::GetInst()->CalcPassField(EntityPool::GetInst()->Get(m_entity),
		0, 0, PathUtils::F_CALC_NEAR_PNTS|PathUtils::F_PASS_ALL_ENTITIES);
	// проверим, существуют ли точки рядом с существом
	if(!PathUtils::GetInst()->IsNear(EntityPool::GetInst()->Get(m_enemy_id)))
	{
		// к существу нельзя подойти
		Print(m_entity, "|ABOT|thinkAttack: к цели нельзя подойти - на отдых");
		m_mode = ABM_REST;
		m_rest_turn = 0;
		m_turn_state = TS_END;
		return false;
	}
	m_target_pnt = PathUtils::GetInst()->GetNearPnt(EntityPool::GetInst()->Get(m_enemy_id)).m_pnt;
	if(m_target_pnt == EntityPool::GetInst()->Get(m_entity)->GetGraph()->GetPos2())
	{
		// подошли вплотную к цели - передаем ход
		m_turn_state = TS_END;
		return false;
	}
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(EntityPool::GetInst()->Get(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// к существу нельзя подойти
		Print(m_entity, "|ABOT|thinkAttack: к цели нельзя подойти - на отдых");
		m_mode = ABM_REST;
		m_rest_turn = 0;
		m_turn_state = TS_END;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(EntityPool::GetInst()->Get(m_entity),
		path,
		ActivityFactory::CT_CIVILIAN_CAR_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Rest
bool AdvertisingBotNode::thinkRest(state_type* st)
{
	m_rest_turn++;
	if(m_rest_turn > 1) 
	{
		m_mode = ABM_SEARCHING;
		Print(m_entity, "|ABOT|thinkRest: на поиск новой цели!");
	}
	else
	{
		Print(m_entity, "|ABOT|thinkRest: отдыхаем");
	}
	m_turn_state = TS_END;
	return false;
}

// умри!
bool AdvertisingBotNode::die()
{
	m_mode = ABM_KILLED;
	return need2Delete();
}

// нужно ли удалять узел
bool AdvertisingBotNode::need2Delete() const
{
	if( (m_mode == ABM_KILLED) && !m_activity) return true;
	return false;
}



/////////////////////////////////////////////////////////////////////////////
//
// узел для торгового бота
//
/////////////////////////////////////////////////////////////////////////////

// конструктор - id существа
TradingBotNode::TradingBotNode(eid_t id) : m_mode(TBM_SEARCHING),
m_turn_state(TS_NONE), m_entity(id), m_activity(0), m_enemy_id(0), m_base_pnt(0, 0),
m_target_pnt(0, 0), m_initialising(false), m_rest_turn(0), m_attack_turn(0)
{
	if(m_entity) OnSpawnMsg();
}

TradingBotNode::~TradingBotNode()
{
	delete m_activity;
}

DCTOR_IMP(TradingBotNode)

float TradingBotNode::Think(state_type* st)
{
	float complexity = 0.0f;
    //проиграть действие
    if(m_activity){

        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK)){
			m_activity->Detach(&m_activity_observer);
            delete m_activity;
            m_activity = 0;
        }
        return complexity;
    }

	// если не наш ход - на выход
	if(!st)
	{
		m_turn_state = TS_NONE;
		return complexity;
	}

	// если m_turn_state == TS_NONE, то это начало нашего хода
	if(m_turn_state == TS_NONE)
	{
		m_turn_state = TS_START;
		// начнем инициализацию
		PanicPlayer::GetInst()->Init(EntityPool::GetInst()->Get(m_entity));
		m_initialising = true;
	}

	// проверим флаг инициализации
	if(m_initialising)
	{
		if(PanicPlayer::GetInst()->Execute())
		{
			// нужно продолжать процесс инициализации
			*st = ST_INCOMPLETE;
			return complexity;
		}
		// инициализация закончена
		m_initialising = false;
	}

    // юнит свободен - можно подумать
	bool flag = true;
    while(flag)
	{
        switch(m_mode)
		{
        case TBM_SEARCHING:
            flag = thinkSearching(st);
            break;
        
		case TBM_ATTACK:
            flag = thinkAttack(st);
            break;
        
		case TBM_REST:
            flag = thinkRest(st);
            break;
        }
    }

	if(m_turn_state == TS_END)
	{
		*st = ST_FINISHED;
		m_turn_state = TS_START;
		complexity = 1.0f;
	}
	else *st = ST_INCOMPLETE;
	return complexity;
}

void TradingBotNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());
		st << m_enemy_id;
		st << m_base_pnt.x;
		st << m_base_pnt.y;
		st << m_target_pnt.x;
		st << m_target_pnt.y;
		st << m_rest_turn;
		st << m_attack_turn;

		return;
    }
	// читаемся
	int tmp;
	st >> tmp; m_mode = static_cast<TradingBotMode>(tmp);
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> m_entity;
	st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));
	st >> m_enemy_id;
	st >> m_base_pnt.x;
	st >> m_base_pnt.y;
	st >> m_target_pnt.x;
	st >> m_target_pnt.y;
	st >> m_rest_turn;
	st >> m_attack_turn;
	
	// то, что восстанавливается без чтения
	m_activity = 0;
	AIAPI::getInst()->getBaseField(m_base_pnt, static_cast<int>(trader_bot_radius), &m_base_field);
	m_initialising = false;
}

//обработка сообщения о расстановке
void TradingBotNode::OnSpawnMsg()
{
	m_base_pnt = EntityPool::GetInst()->Get(m_entity)->GetGraph()->GetPos2();
	AIAPI::getInst()->getBaseField(m_base_pnt, static_cast<int>(trader_bot_radius), &m_base_field);
}

// обработка режима Searching
bool TradingBotNode::thinkSearching(state_type* st)
{
	// 1. найдем всех видимых людей - если кого-нибудь вижу - в атаку
	// 2. никого нет - если дошел до нужной точки выбираем новую
	// 3. если кончились мувпоинты - передаем ход
	// 4. идем в заданную точку

	// 1. найдем всех видимых людей - если кого-нибудь вижу - в атаку
	AIAPI::entity_vector_t entities;
	if(AIAPI::getInst()->getEntitiesVisibleByEntity(m_entity,
		&entities, ET_HUMAN, m_base_field))
	{
		// есть цели
		Print(m_entity, "|TBOT|thinkSearching: вижу цель - в атаку");
		m_enemy_id = AIAPI::getInst()->getRandomEntity(entities);
		m_mode = TBM_ATTACK;
		m_turn_state = TS_INPROGRESS;
		m_attack_turn = 0;
		return false;
	}
	// 2. никого нет - если дошел до нужной точки выбираем новую
	if(m_target_pnt == EntityPool::GetInst()->Get(m_entity)->GetGraph()->GetPos2())
	{
		Print(m_entity, "|TBOT|thinkSearching: дошел до точки - выбираю новую");
		// выберем лучший хекс в радиусе 10 хексов от базовой точки:
		// рассчитаем поле проходимости
		PathUtils::GetInst()->CalcPassField(EntityPool::GetInst()->Get(m_entity),
			0, 0, PathUtils::F_PASS_ALL_ENTITIES);
		// получим вектор доступных хексов базового поля
		pnt_vec_t vec;
		AIAPI::getInst()->getReachableField(m_base_field, &vec, true);
		if(vec.empty())
		{
			// передаем ход - идти некуда
			Print(m_entity, "|TBOT|thinkSearching: мне некуда ходить!!!");
			m_turn_state = TS_END;
			return false;
		}
		// получим случайную точку
		m_target_pnt = AIAPI::getInst()->getRandomPoint(vec);
	}

	// 3. если кончились мувпоинты - передаем ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		Print(m_entity, "|TBOT|thinkSearching: кончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}
	// 4. идем в заданную точку
	Print(m_entity, "|TBOT|thinkSearching: иду в заданную точку");
	PathUtils::GetInst()->CalcPassField(EntityPool::GetInst()->Get(m_entity),
		0, 0, PathUtils::F_PASS_ALL_ENTITIES);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(EntityPool::GetInst()->Get(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		Print(m_entity, "|TBOT|thinkSearching: заданная точка не достижима");
		m_target_pnt = EntityPool::GetInst()->Get(m_entity)->GetGraph()->GetPos2();
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(EntityPool::GetInst()->Get(m_entity),
		path,
		ActivityFactory::CT_CIVILIAN_CAR_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Attack
bool TradingBotNode::thinkAttack(state_type* st)
{
	// 1. если в состоянии атаки прошло три тура - переход в состояние отдыха
	// 2. если цель умерла или вышла из зоны видимости,
	//    или вышла из базового поля - в состояние отдыха
	// 3. если кончились мувпоинты - предаем ход
	// 4. выберем точку рядом с целью и пойдем  в нее

	if(m_turn_state == TS_START) m_attack_turn++;
	// 1. если в состоянии атаки прошло три тура - переход в состояние отдыха
	if(m_attack_turn > 3)
	{
		Print(m_entity, "|TBOT|thinkAttack: прошло три тура - на отдых");
		m_mode = TBM_REST;
		m_rest_turn = 0;
		m_turn_state = TS_END;
		return false;
	}
	// 2. если цель умерла или вышла из зоны видимости - в состояние отдыха
	//    или вышла из базового поля - в состояние отдыха
	if( (!AIAPI::getInst()->isEntityVisible(m_entity, m_enemy_id)) ||
		(!AIAPI::getInst()->isEntityInBaseField(m_enemy_id, m_base_field))
		)
	{
		Print(m_entity, "|TBOT|thinkAttack: цель ушла или погибла - на отдых");
		m_mode = TBM_REST;
		m_rest_turn = 0;
		m_turn_state = TS_END;
		return false;
	}

	// 3. если кончились мувпоинты - предаем ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		Print(m_entity, "|TBOT|thinkAttack: закончились мувпониты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. выберем точку рядом с целью и пойдем  в нее
	Print(m_entity, "|TBOT|thinkAttack: идем к цели");
	// рассчитаем поле проходимости
	PathUtils::GetInst()->CalcPassField(EntityPool::GetInst()->Get(m_entity),
		0, 0, PathUtils::F_CALC_NEAR_PNTS|PathUtils::F_PASS_ALL_ENTITIES);
	// проверим, существуют ли точки рядом с существом
	if(!PathUtils::GetInst()->IsNear(EntityPool::GetInst()->Get(m_enemy_id)))
	{
		// к существу нельзя подойти
		Print(m_entity, "|TBOT|thinkAttack: к цели нельзя подойти - на отдых");
		m_mode = TBM_REST;
		m_rest_turn = 0;
		m_turn_state = TS_END;
		return false;
	}
	m_target_pnt = PathUtils::GetInst()->GetNearPnt(EntityPool::GetInst()->Get(m_enemy_id)).m_pnt;
	if(m_target_pnt == EntityPool::GetInst()->Get(m_entity)->GetGraph()->GetPos2())
	{
		// подошли вплотную к цели - передаем ход
		m_turn_state = TS_END;
		return false;
	}
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(EntityPool::GetInst()->Get(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// к существу нельзя подойти
		Print(m_entity, "|TBOT|thinkAttack: к цели нельзя подойти - на отдых");
		m_mode = TBM_REST;
		m_rest_turn = 0;
		m_turn_state = TS_END;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(EntityPool::GetInst()->Get(m_entity),
		path,
		ActivityFactory::CT_CIVILIAN_CAR_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Rest
bool TradingBotNode::thinkRest(state_type* st)
{
	m_rest_turn++;
	if(m_rest_turn > 1) 
	{
		m_mode = TBM_SEARCHING;
		Print(m_entity, "|TBOT|thinkRest: на поиск новой цели!");
	}
	else
	{
		Print(m_entity, "|TBOT|thinkRest: отдыхаем");
	}
	m_turn_state = TS_END;
	return false;
}

// умри!
bool TradingBotNode::die()
{
	m_mode = TBM_KILLED;
	return need2Delete();
}

// нужно ли удалять узел
bool TradingBotNode::need2Delete() const
{
	if( (m_mode == TBM_KILLED) && !m_activity) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// узел для ремонтного бота
//
/////////////////////////////////////////////////////////////////////////////

// конструктор - id существа
RepairBotNode::RepairBotNode(eid_t id) : m_mode(RBM_GOING),
m_turn_state(TS_NONE), m_entity(id), m_activity(0), m_target_pnt(0, 0), m_initialising(false),
m_attack_turn(0)
{
}

RepairBotNode::~RepairBotNode()
{
	delete m_activity;
}

DCTOR_IMP(RepairBotNode)

float RepairBotNode::Think(state_type* st)
{
	float complexity = 0.0f;
    //проиграть действие
    if(m_activity){

        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK)){
			m_activity->Detach(&m_activity_observer);
            delete m_activity;
            m_activity = 0;
        }
        return complexity;
    }

	// если не наш ход - на выход
	if(!st)
	{
		m_turn_state = TS_NONE;
		return complexity;
	}

	// если m_turn_state == TS_NONE, то это начало нашего хода
	if(m_turn_state == TS_NONE)
	{
		m_turn_state = TS_START;
		// начнем инициализацию
		PanicPlayer::GetInst()->Init(EntityPool::GetInst()->Get(m_entity));
		m_initialising = true;
	}

	// проверим флаг инициализации
	if(m_initialising)
	{
		if(PanicPlayer::GetInst()->Execute())
		{
			// нужно продолжать процесс инициализации
			*st = ST_INCOMPLETE;
			return complexity;
		}
		// инициализация закончена
		m_initialising = false;
	}

    // юнит свободен - можно подумать
	bool flag = true;
    while(flag)
	{
        switch(m_mode)
		{
        case RBM_GOING:
            flag = thinkGoing(st);
            break;
        
		case RBM_ATTACK:
            flag = thinkAttack(st);
            break;
        }
    }

	if(m_turn_state == TS_END)
	{
		*st = ST_FINISHED;
		m_turn_state = TS_START;
		complexity = 1.0f;
	}
	else *st = ST_INCOMPLETE;
	return complexity;
}

void RepairBotNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());
		st << m_target_pnt.x;
		st << m_target_pnt.y;
		st << m_attack_turn;

		return;
    }
	// читаемся
	int tmp;
	st >> tmp; m_mode = static_cast<RepairBotMode>(tmp);
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> m_entity;
	st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));
	st >> m_target_pnt.x;
	st >> m_target_pnt.y;
	st >> m_attack_turn;
	
	// то, что восстанавливается без чтения
	m_activity = 0;
	m_initialising = false;
}

// обработка режима Going
bool RepairBotNode::thinkGoing(state_type* st)
{
	// 1. Если дошел до заданной точки - выбирает новую и переходит в атаку
	// 2. Если кончились мувпоинты - передает ход
	// 3. Идет в заданную точку

	// 1. Если дошел до заданной точки - выбирает новую и переходит в атаку
	if(m_target_pnt == EntityPool::GetInst()->Get(m_entity)->GetGraph()->GetPos2())
	{
		Print(m_entity, "|RBOT|thinkGoing: дошел до точки - выбираю новую");
		// рассчитаем поле проходимости
		pnt_vec_t allhexes;
		PathUtils::GetInst()->CalcPassField(EntityPool::GetInst()->Get(m_entity),
			0, &allhexes, PathUtils::F_PASS_ALL_ENTITIES);
		if(allhexes.empty())
		{
			// передаем ход - идти некуда
			Print(m_entity, "|RBOT|thinkGoing: мне некуда ходить!!!");
			m_turn_state = TS_END;
			return false;
		}
		// получим случайную точку
		m_target_pnt = AIAPI::getInst()->getRandomPoint(allhexes);
		// и переходим в атаку
		m_mode = RBM_ATTACK;
		m_turn_state = TS_END;
		m_attack_turn = 0;
		return false;
	}

	// 2. если кончились мувпоинты - передаем ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		Print(m_entity, "|RBOT|thinkGoing: кончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. Идет в заданную точку
	Print(m_entity, "|RBOT|thinkGoing: иду в заданную точку");
	PathUtils::GetInst()->CalcPassField(EntityPool::GetInst()->Get(m_entity),
		0, 0, PathUtils::F_PASS_ALL_ENTITIES);
	pnt_vec_t path;
	PathUtils::GetInst()->CalcPath(EntityPool::GetInst()->Get(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		Print(m_entity, "|RBOT|thinkGoing: заданная точка не достижима");
		m_target_pnt = EntityPool::GetInst()->Get(m_entity)->GetGraph()->GetPos2();
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::GetInst()->CreateMove(EntityPool::GetInst()->Get(m_entity),
		path,
		ActivityFactory::CT_CIVILIAN_CAR_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

// обработка режима Attack
bool RepairBotNode::thinkAttack(state_type* st)
{
	// 1. Если в состоянии атаки прошло три тура - переход в состояние "переход"
	// 2. Если кончились мувпоинты - передает ход
	// 3. Атакует точку перед собой

	if(m_turn_state == TS_START) m_attack_turn++;
	// 1. Если в состоянии атаки прошло три тура - переход в состояние "переход"
	if(m_attack_turn > 3)
	{
		Print(m_entity, "|RBOT|thinkAttack: прошло три тура - переход");
		m_mode = RBM_GOING;
		m_turn_state = TS_END;
		return false;
	}
	// 2. Если кончились мувпоинты - передает ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		Print(m_entity, "|RBOT|thinkAttack: закончились мувпониты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. Атакует точку перед собой
	point3 target;
	target = EntityPool::GetInst()->Get(m_entity)->GetGraph()->GetPos3();
	target.x += 1.0f;
	target.y += 1.0f;
	target.z += 1.0f;
	std::string reason;
	Print(m_entity, "|RBOT|thinkAttack:  будем стрелять");
	if(AIAPI::getInst()->shootByVehicle(m_entity, target, &m_activity, &reason))
	{
		Print(m_entity, "|RBOT|thinkAttack:  выстрел пошел!!!");
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	}
	else
	{
		std::string str = "|RBOT|thinkAttack: стрелять нельзя: ";
		str += reason;
		Print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

// умри!
bool RepairBotNode::die()
{
	m_mode = RBM_KILLED;
	return need2Delete();
}

// нужно ли удалять узел
bool RepairBotNode::need2Delete() const
{
	if( (m_mode == RBM_KILLED) && !m_activity) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// узел для пофигиста
//
/////////////////////////////////////////////////////////////////////////////

// конструктор - id существа
IndifferentNode::IndifferentNode(eid_t id) : 
m_turn_state(TS_NONE), m_entity(id), m_initialising(false)
{
	if(m_entity) OnSpawnMsg();
}

IndifferentNode::~IndifferentNode()
{
}

DCTOR_IMP(IndifferentNode)

float IndifferentNode::Think(state_type* st)
{
	float complexity = 0.0f;

	// если не наш ход - на выход
	if(!st)
	{
		m_turn_state = TS_NONE;
		return complexity;
	}

	// если m_turn_state == TS_NONE, то это начало нашего хода
	if(m_turn_state == TS_NONE)
	{
		m_turn_state = TS_START;
		// начнем инициализацию
		PanicPlayer::GetInst()->Init(EntityPool::GetInst()->Get(m_entity));
		m_initialising = true;
	}

	// проверим флаг инициализации
	if(m_initialising)
	{
		if(PanicPlayer::GetInst()->Execute())
		{
			// нужно продолжать процесс инициализации
			*st = ST_INCOMPLETE;
			return complexity;
		}
		// инициализация закончена
		m_initialising = false;
	}

    // юнит свободен - можно подумать
	// это пофигист - ему думать нечего
	m_turn_state = TS_END;

	if(m_turn_state == TS_END)
	{
		*st = ST_FINISHED;
		m_turn_state = TS_START;
		complexity = 1.0f;
	}
	else *st = ST_INCOMPLETE;
	return complexity;
}

void IndifferentNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_turn_state);
		st << m_entity;

		return;
    }
	// читаемся
	int tmp;
	st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
	st >> m_entity;
	
	// то, что восстанавливается без чтения
	m_initialising = false;
}

//обработка сообщения о расстановке
void IndifferentNode::OnSpawnMsg()
{
    // сделать, чтобы юнит не паниковал
    if(HumanEntity*  human = EntityPool::GetInst()->Get(m_entity)->Cast2Human())
	{
		HumanContext* context = human->GetEntityContext();
		context->SetPanicSelector(0);
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// пространство имен без имени - для функций-утилит этого файла
//
//////////////////////////////////////////////////////////////////////////////
namespace
{
	//
	// вывести в консоль
	//
	void Print(eid_t entity, const std::string& str)
	{
		BaseEntity* be = EntityPool::GetInst()->Get(entity);
		std::ostringstream os;
		os << be->GetInfo()->GetName();
		os << "(" << entity << ")";
		os << str;
		Console::AddString(os.str().c_str());
	}
};
