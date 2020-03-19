#pragma warning(disable:4786)

#include "logicdefs.h"

#include "form.h"
#include "spawn.h"
#include "Thing.h"
#include "entity.h"
#include "Bureau.h"
#include "sscene.h"
#include "Player.h"    
#include "damager.h" 
#include "Graphic.h"
#include "HexGrid.h"
#include "AIUtils.h"
#include "Strategy.h"
#include "SndUtils.h"
#include "cameraman.h"
#include "pathutils.h"
#include "visutils3.h"
#include "GameLogic.h"
#include "DirtyLinks.h"
#include "QuestServer.h"
#include "phrasemanager.h"
#include "entityfactory.h"
#include "enemydetection.h"

#include "../options/options.h"
#include "../securom/securom.h"

/////////////////////////////////////////////////////////////////////////
//	Naughty: 14.05.02 для фиксирования бага с Helper'ом
#include "../interface/screens.h"
/////////////////////////////////////////////////////////////////////////

//~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~~~
#include "HexGrid.h"
#include "GameObserver.h"
//~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~~~

DynUtils::DynCtor*  DynUtils::DynCtor::m_first;
DynUtils::DynCtor*  DynUtils::DynCtor::m_cached;

//========================================================

namespace {

const char* level_file_name = "./saves/levels.dat";

const float lower_load_bar_bound = 0.5f;
const float upper_load_bar_bound = 1.0f;

typedef std::auto_ptr<AbstractProgressBar> bar_ptr_t;

//послать уведоление всем существам
void NotifyAllEntities(EntityObserver::event_type event, EntityObserver::info_t info = 0)
{
    EntityPool::iterator itor = EntityPool::GetInst()->begin();
    while(itor != EntityPool::GetInst()->end()){
        itor->Notify(event, info);
        ++itor;
    }
}

void EnableGraphDestroy(bool flag)
{
    GraphThing::EnableDestroy(flag);

    if(flag)
        GraphEntity::RaiseFlags(GraphEntity::FT_ENABLE_DESTROY);
    else
        GraphEntity::DropFlags(GraphEntity::FT_ENABLE_DESTROY);
}

//перелинковать сущетcва
void RelinkAllEntities()
{
    EntityBuilder builder;
    EntityPool::iterator itor = EntityPool::GetInst()->begin();

    while(itor != EntityPool::GetInst()->end()){
        builder.LinkEntity(&*itor, itor->GetGraph()->GetPos2(), itor->GetGraph()->GetAngle());
        ++ itor;
    }
}

//установить стойку для всех людей
void SetAllHumanStance()
{
    EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER|PT_ENEMY);
    while(itor != EntityPool::GetInst()->end()){
        AIUtils::ChangeHumanStance(itor->Cast2Human());
        ++itor;
    };
}

//удалить существа из массива существ
template <class Condition>
void DestroyEntities(Condition cond)
{
    EntityPool::iterator itor = EntityPool::GetInst()->begin();

    while(itor != EntityPool::GetInst()->end()){            

        if(cond(&*itor)){
            itor->GetGraph()->Destroy();
            EntityPool::GetInst()->Remove(&*itor);
        }

        itor++;
    }
}

//расчитать ходы для всех существо на начало игры
void CalcMovepoints4All()
{
    EntityPool::iterator itor = EntityPool::GetInst()->begin();
    while(itor != EntityPool::GetInst()->end()){
        AIUtils::CalcMovepnts4Turn(&*itor);
        ++itor;
    }
}

//
//условие для удаления по типу и игроку сущетва
//
class AttributeCondition{
public:

    AttributeCondition(unsigned type = ET_ALL_ENTS,
                       unsigned team = PT_ALL_PLAYERS,
                       unsigned attr = EA_ALL_ATTRIBUTES) :
        m_type(type), m_team(team), m_attr(attr){}
    
    bool operator()(BaseEntity* entity)
    {
        return      entity->GetType() & m_type                
                &&  entity->GetPlayer() & m_team
                &&  entity->GetAttributes() & m_attr;
    }           

private:

    unsigned m_type;
    unsigned m_team;
    unsigned m_attr;
};

//
// условие для удаления роботов
//
class RobotCondition {
public:

    bool operator()(BaseEntity* entity)
    {
        VehicleEntity* vehicle = entity->Cast2Vehicle();
        return vehicle && vehicle->GetInfo()->IsRobot();
    }
};

//========================================================

//
// абстракция progress_bar
//
class SaveLoadBar : public AbstractProgressBar{
public:

    SaveLoadBar(AbstractProgressBar* bar) : m_bar(bar){}
    ~SaveLoadBar(){ delete m_bar; }

    //установить движок + текстовую метку
    void SetPos(float value, const std::string& text = std::string())
    { if(m_bar) m_bar->SetPos(value, text); }

private:

    AbstractProgressBar* m_bar;
};

//========================================================

//
// класс для вывода сообщений об изменении соотношений сил
//
class PrintForceRelationChange : private RelationsObserver{
public:

    PrintForceRelationChange()
    { EnemyDetector::getInst()->attach(this, ET_RELATIONS_CHANGED); }

    ~PrintForceRelationChange()
    { EnemyDetector::getInst()->detach(this); }

private:

    void changed(event_type et, void* info)
    {        
        RelationDesc* ptr = static_cast<RelationDesc*>(info);

        if(ptr->isFirstPlayer()) return;

        std::string res_str = "unknown_relation";

        switch(ptr->m_relation_type){
        case RT_NEUTRAL:
            res_str = "lgc_neutrals";
            break;

        case RT_ENEMY:
            res_str = "lgc_enemies";
            break;

        case RT_FRIEND:
            res_str = "lgc_friends";
            break;

        } 

        DirtyLinks::Print(mlprintf(DirtyLinks::GetStrRes(res_str).c_str(),
            EnemyDetector::getInst()->getName(ptr->m_first_subteam).c_str(),
            EnemyDetector::getInst()->getName(ptr->m_second_subteam).c_str()));
    }
};

//========================================================

//
// Отложенная инициализация квестов
//
class InitQuestServer : private SpawnObserver{
public:

    InitQuestServer() : m_sheme(ST_NEXT_LEVEL)
    {
        Spawner::Attach(this, ET_ENTRY_LEVEL);
        Spawner::Attach(this, ET_FINISH_SPAWN);
    }

    ~InitQuestServer()
    {
        Spawner::GetInst()->Detach(this);
    }

private:

    void Update(subject_t subj, event_t event, info_t info)
    {
        switch(event){
        case ET_ENTRY_LEVEL:
            {
                entry_info_s* ptr = static_cast<entry_info_s*>(info);

                if(ptr->m_entry.empty())
                    m_sheme = ST_NEW_LEVEL;
                else if(ptr->m_fnew_episode)            
                    m_sheme = ST_NEW_EPISODE;
                else 
                    m_sheme = ST_NEXT_LEVEL;
            }
            break;

        case ET_FINISH_SPAWN:
            {
                switch(m_sheme){
                case ST_NEXT_LEVEL:
                    break;

                case ST_NEW_LEVEL:
                    QuestServer::GetInst()->Init(QuestServer::ST_START_NEW_GAME, Spawner::GetInst()->GetEpisode());
                    break;

                case ST_NEW_EPISODE:
                    QuestServer::GetInst()->Init(QuestServer::ST_CHANGE_EPISODE, Spawner::GetInst()->GetEpisode());
                    break;
                }
            }
            break;
        }
    }

private:

    enum sheme_type{
        ST_NEW_LEVEL,
        ST_NEXT_LEVEL,
        ST_NEW_EPISODE
    };

    sheme_type m_sheme;
};

//========================================================

//
// класс с обычным алгоритмом входа на уровень
//
class UsualLevelEnter : public SpawnObserver{
public:

    UsualLevelEnter()  { Spawner::Attach(this, ET_ENTRY_LEVEL); }
    ~UsualLevelEnter() { Spawner::Detach(this); }

private:

    void Update(Spawner* spawner, event_t type, info_t ptr)
    {
        entry_info_s* info = static_cast<entry_info_s*>(ptr);

        if(info->m_entry.empty())
            Init(ST_NEW_GAME);
        else if(info->m_fnew_episode)            
            Init(ST_NEW_EPISODE);
       
        GraphUtils::DestroyCorpses();

        if(info->m_entry.size()){

            DestroyEntities(RobotCondition());
            DestroyEntities(AttributeCondition(ET_ALL_ENTS, ~PT_PLAYER));
          
            //перелинкуем существа
            EntityPool::iterator itor = EntityPool::GetInst()->begin();
            while(itor != EntityPool::GetInst()->end()){

                ipnt2_t pos;

                if(!info->m_builder->GenerateSuitablePos(&*itor, info->m_entry, &pos))
                    throw CASUS("UsualLevelEnter: не хватет точек входа на уровень!");
                
                info->m_builder->LinkEntity(&*itor, pos, PIm2*NormRand());

                info->m_builder->SendSpawnEvent(&*itor);

                ++itor;
            }

            AIUtils::CalcAndShowActiveExits();
        }

        Bureau::GetInst()->HandleLevelEntry();
        Forms::GetInst()->Reset(Forms::RT_NEXT_LEVEL);
        EpisodeMapMgr::GetInst()->Insert(DirtyLinks::GetLevelSysName());
    }

    enum sheme_type{
        ST_NEW_GAME,
        ST_NEW_EPISODE,
    };

    void Init(sheme_type type)
    {
        if(type == ST_NEW_GAME){

            Forms::GetInst()->Reset(Forms::RT_SHUT);
            
            MoneyMgr::GetInst()->Reset();

            EnemyDetector::getInst()->init();
            ForceHandbook::GetInst()->Reset();
            ThingHandbook::GetInst()->Reset();
            EntityHandbook::GetInst()->Reset();

            DiaryManager::GetInst()->Delete(DiaryManager::ROOT_KEY);
            
            GameEvMessenger::GetInst()->Notify(GameObserver::EV_NEW_GAME);
			Spawner::GetInst()->SetDifficulty(Options::GetInt("game.type"));
        }
        
        EpisodeMapMgr::GetInst()->Reset();
               
        DestroyEntities(AttributeCondition(ET_ALL_ENTS, type == ST_NEW_GAME ? PT_ALL_PLAYERS: ~PT_PLAYER));
      
        AIUtils::CalcAndShowActiveExits();
    }  
};

//========================================================

//
// класс для отслеживания progress bar'а
//
class EntityLoadProgressBar : public GameObserver{
public:

    EntityLoadProgressBar(AbstractProgressBar* bar, float from, float to) :
        m_from(from), m_to(to), m_delta(0), m_bar(bar)
    {
        GameEvMessenger::GetInst()->Attach(this, EV_INIT_ENTLOAD);
        GameEvMessenger::GetInst()->Attach(this, EV_ENITITY_LOADED);

        if(m_bar) m_bar->SetPos(from, DirtyLinks::GetStrRes("lgc_prepare_load"));
    }

    ~EntityLoadProgressBar(){ GameEvMessenger::GetInst()->Detach(this); }

private:

    void Update(subject_t subj, event_t event, info_t info)
    {
        switch(event){
        case EV_ENITITY_LOADED:
            m_bar->SetPos(m_from += m_delta, mlprintf(
                      DirtyLinks::GetStrRes("lgc_load_entities").c_str(),
                      static_cast<entity_info*>(info)->m_entity->GetInfo()->GetName().c_str()));
            break;

        case EV_INIT_ENTLOAD:
            m_delta = (m_to - m_from)/static_cast<entload_info*>(info)->m_count;
            break;
        }
    }

private:

    float m_to;
    float m_from;
    float m_delta;

    AbstractProgressBar* m_bar;
};

//========================================================

//
// класс для отслеживания progress bar'а
//
class SpawnProgressBar : public SpawnObserver{
public:

    SpawnProgressBar(bar_ptr_t& bar, float from, float to) :
      m_from(from), m_to(to), m_delta(0), m_bar(bar)
    {
        Spawner::Attach(this, ET_ENTITY_SPAWN);
        Spawner::Attach(this, ET_PREPARE_SPAWN);

        m_bar->SetPos(from, DirtyLinks::GetStrRes("lgc_prepare_spawn"));
    }

    ~SpawnProgressBar()
    {
        Spawner::Detach(this);
    }

private:

    void Update(Spawner* spawner, event_t event, info_t ptr)
    {
        switch(event){
        case ET_ENTITY_SPAWN:
            m_bar->SetPos(m_from += m_delta, mlprintf(
                          DirtyLinks::GetStrRes("lgc_entity_spawned").c_str(),
                          static_cast<spawn_info_s*>(ptr)->m_entity->GetInfo()->GetName().c_str()));
            break;

        case ET_PREPARE_SPAWN:
            {
                prepare_info_s* info = static_cast<prepare_info_s*>(ptr);
                m_delta = (m_to - m_from)/(info->m_humans + info->m_traders + info->m_vehicles + CountEntities(ET_ALL_ENTS));
            }            
            break;
        }
    }

    //подсчитать кол-во сущетсв опред типа
    int CountEntities(unsigned etype)
    {
        int count = 0;
        
        EntityPool::iterator itor = EntityPool::GetInst()->begin(etype);
        while(itor != EntityPool::GetInst()->end()){
            ++count;
            ++itor;
        }
        
        return count;
    }

private:

    float m_to;
    float m_from;
    float m_delta;

    bar_ptr_t& m_bar;
};

//========================================================

//
// класс для отслеживания данных зависимых от уровня
//
class UsualLevelData : public SpawnObserver{
public:
   
    UsualLevelData()
    { 
        Spawner::Attach(this, ET_EXIT_LEVEL);
        Spawner::Attach(this, ET_ENTRY_LEVEL);
    }

    ~UsualLevelData()
    {
        Spawner::Detach(this);
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        Win32File win_file(level_file_name, slot.IsSaving() ? OPEN_ALWAYS : CREATE_ALWAYS);        
        LevelFile lvl_file(&win_file, DirtyLinks::GetLevelSysName(), LevelFile::OM_READ);
        lvl_file.MakeSaveLoad(slot);
    }

private:

    void Update(Spawner* spawner, event_t type, info_t info)
    {
        switch(type){
        case ET_EXIT_LEVEL:
            SaveLoadData2LevelFile(Storage::SM_SAVE);
            break;

        case ET_ENTRY_LEVEL:
            if(     static_cast<entry_info_s*>(info)->m_entry.empty()
                ||  static_cast<entry_info_s*>(info)->m_fnew_episode){
                //уничтожим файл с данными об уровнях
                Win32File file_killer(level_file_name, CREATE_ALWAYS);
            }else{
                SaveLoadData2LevelFile(Storage::SM_LOAD);
            }
            break;
        }
    }

    void SaveLoadData2LevelFile(Storage::storage_mode save_load_mode)
    {
        try{
            
            Win32File win_file(level_file_name, OPEN_ALWAYS);
            
            LevelFile lvl_file(&win_file, DirtyLinks::GetLevelSysName(), 
                save_load_mode == Storage::SM_SAVE 
                ?   LevelFile::OM_WRITE
                :   LevelFile::OM_READ);
            
            Storage strg(&lvl_file, save_load_mode);

            Bureau::GetInst()->MakeSaveLoad(SavSlot(&strg, "level_bureau_data"));
            GameObjectsMgr::GetInst()->MakeSaveLoad(SavSlot(&strg, "level_objects_data"));
            DeadList::GetVehiclesList()->MakeSaveLoad(SavSlot(&strg, "level_vehicles_dead_list"));
            DeadList::GetRespawnerDeadList()->MakeSaveLoad(SavSlot(&strg, "level_all_dead_list"));
        }
        catch(CasusImprovisus& ){
        }
    }
};

//========================================================

//
// тестовое правило
//
class SpawnRuleEx : public Spawner::Rule{
public:

    //подходит ли данное правило
    virtual bool IsSuitable(const SpawnTag& tag) const = 0;
};

//
// запрещение расстановки убитых существ
//
class DoNotSpawnKilledEntityRule : public SpawnRuleEx{
public:

    DoNotSpawnKilledEntityRule(DeadList* lst, entity_type ent_type) :
      m_list(lst), m_entity(ent_type) {}

    bool IsSuitable(const SpawnTag& tag) const
    { 
        return     tag.GetEntityType() == m_entity
               &&  m_list->GetKilledCount(tag);
    }

    int CalcCount(const SpawnTag& tag, int xls_count){ return 0; }

private:

    DeadList*   m_list;
    entity_type m_entity;
};

//
// запрещение расстановки существ присутсв. в команде игрока
//
class DoNotSpawnTeammateRule : public SpawnRuleEx,
                               private SpawnObserver
{
public:

    DoNotSpawnTeammateRule(entity_type ent_type, player_type player) :
        m_ent_type(ent_type)
    {
        //составим список всех перешедших с нами на уровень
        EntityPool::iterator itor = EntityPool::GetInst()->begin(m_ent_type, player);
        while(itor != EntityPool::GetInst()->end()){
            m_peoples.insert(itor->GetInfo()->GetRID());
            ++itor;
        }

        Spawner::GetInst()->Attach(this, ET_ENTRY_LEVEL);
    }

    ~DoNotSpawnTeammateRule() { Spawner::GetInst()->Detach(this); }

    bool IsSuitable(const SpawnTag& tag) const
    { 
        return     tag.GetEntityType() == m_ent_type
               &&  m_peoples.count(tag.GetSysName());
    }

    int CalcCount(const SpawnTag& tag, int xls_count){ return 0; }

private:

    void Update(Spawner* spawner, event_t type, info_t info)
    { 
        if(static_cast<entry_info_s*>(info)->m_entry.empty())
            m_peoples.clear();
    }
    
private:

    rid_set_t   m_peoples;
    entity_type m_ent_type; 
};

//
// Запрещение расстановки техники
//
class DoNotSpawnKilledVehiclesRule : public SpawnRuleEx {
public:

    DoNotSpawnKilledVehiclesRule(DeadList* deads) : m_deads(deads) {}

    bool IsSuitable(const SpawnTag& tag) const
    { 
        return      tag.GetEntityType() == ET_VEHICLE
                &&  m_deads->GetKilledCount(tag);
    }

    int CalcCount(const SpawnTag& tag, int xls_count)
    {
        rec_s* record = FindRec(tag);
        
        //если такой записи еще нет создадим ее
        if(record == 0){
            m_records.push_back(rec_s(tag));
            record = &m_records.back();
        }

        record->m_req4spawn += xls_count;
        int ret = record->m_req4spawn - m_deads->GetKilledCount(tag);

        //нормировка
        if(ret < 0) ret = 0;

        return ret > xls_count ? xls_count : ret;
    }

private:

    struct rec_s;
    rec_s* FindRec(const SpawnTag& tag)
    {
        rec_lst_t::iterator itor = m_records.begin();
        while(itor != m_records.end()){

            if(     itor->m_sys_name == tag.GetSysName()
                &&  itor->m_ai_model == tag.GetAIModel()
                &&  itor->m_spawn_zone == tag.GetSpawnZone())
                return &(*itor);

            ++itor;
        }

        return 0;
    }

private:

    struct rec_s{
        std::string m_sys_name;
        std::string m_ai_model;
        std::string m_spawn_zone;

        int m_req4spawn;

        rec_s() {}

        rec_s(const SpawnTag& tag, int count = 0) : 
            m_req4spawn(count), m_spawn_zone(tag.GetSpawnZone()),
            m_ai_model(tag.GetAIModel()), m_sys_name(tag.GetSysName()){}
    };

    typedef std::list<rec_s> rec_lst_t;
    rec_lst_t m_records;

    DeadList* m_deads;
};
 
//
// управление количеством людей для расстановки в зависимости от сложности игры
// Grom
class DifficultyRespawnerRule : public SpawnRuleEx{
public:

    DifficultyRespawnerRule(DeadList* lst) :
      m_list(lst) {}

    bool IsSuitable(const SpawnTag& tag) const
    { 
        return   true||m_list->GetKilledCount(tag);
    }

    int CalcCount(const SpawnTag& tag, int xls_count)
	{
		float cA,cB;
		switch(Spawner::GetInst()->GetDifficulty())
		{
		case Spawner::D_EASIEST:
			cA = 0.4f;
			cB = 0.0f;
			break;
		case Spawner::D_EASY:
			cA = 0.6f;
			cB = 0.3f;
			break;
		case Spawner::D_NORMAL:
			cA = 0.8f;
			cB = 0.5f;
			break;
		default:
		case Spawner::D_HARD:
			cA = 1.f;
			cB = 1.f;
			break;
		}
	float to_burn = ceil(cA*xls_count);
	float killed = m_list->GetKilledCount(tag);
	return  std::max(static_cast<int>(to_burn - killed),
					 static_cast<int>(floor(to_burn * cB))
		);		
	}

private:

    DeadList*   m_list;
};
 
//
// массив правил расстановки
//
class SpawnRules : public Spawner::RuleSet {
public:

    SpawnRules(){}
    ~SpawnRules(){}

    //найти нужное правило
    Spawner::Rule* FindRule(const SpawnTag& tag)
    {
        rule_lst_t::iterator itor = m_rules.begin();

        while(itor != m_rules.end()){

            if((*itor)->IsSuitable(tag))
                return *itor;

            ++itor;
        }

        return 0;
    }

    //поместить правило в массив
    void Insert(SpawnRuleEx* rule) { m_rules.push_back(rule); }

private:

    typedef std::list<SpawnRuleEx*> rule_lst_t;
    rule_lst_t m_rules;
};

//========================================================

//
// системная обработка игровых событий
//
class UsualGameObserver : public GameObserver{
public:

    UsualGameObserver()
    {
        GameEvMessenger::GetInst()->Attach(this, EV_TURN);
        GameEvMessenger::GetInst()->Attach(this, EV_KILL);
    }

    ~UsualGameObserver()
    {  
        GameEvMessenger::GetInst()->Detach(this);
    }

    void Update(subject_t subj, event_t event, info_t info)
    {
        if(event == EV_TURN) OnEvent(static_cast<turn_info*>(info));
        if(event == EV_KILL) OnEvent(static_cast<kill_info*>(info));
    }

private:

    void OnEvent(turn_info* info)
    {
        DirtyLinks::SendTurnEvent(info->m_player, info->IsBegining());
    }

    void OnEvent(kill_info* info)
    {
        if(info->m_event != KEV_KILL) return;

        //уведомить квесты
        QuestServer::GetInst()->HandleKillEntity(info->m_killer, info->m_victim);

        std::ostringstream ostr;

        //выыедем сообщение об умирании
        DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("lgc_death").c_str() , info->m_victim->GetInfo()->GetName().c_str()));

        //изменим стойку людей кот на нас смотрели
        VisMap::spectator_itor itor = VisMap::GetInst()->spectator_begin(info->m_victim->GetEntityContext()->GetMarker(), VT_VISIBLE);
        while(itor != VisMap::GetInst()->spectator_end()){
            
            if(HumanEntity* human = itor->GetEntity() ? itor->GetEntity()->Cast2Human() : 0)
                AIUtils::ChangeHumanStance(human, info->m_victim);
            
            ++itor;
        }

        //обработаем смерть союзника
        if(HumanEntity* human = info->m_victim->Cast2Human()){        
            EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN|ET_VEHICLE, human->GetPlayer());
            while(itor != EntityPool::GetInst()->end()){
                AIUtils::CalcHumanAllyDeath(&*itor);
                ++itor;
            }
        }

        int experience = 0;

        if(HumanEntity* human = info->m_victim->Cast2Human())
            experience = human->GetInfo()->GetExp4Kill();
        else if(VehicleEntity* vehicle = info->m_victim->Cast2Vehicle())
            experience = vehicle->GetInfo()->GetExp4Kill();
        
        //обработаем смерть врага
        if(info->m_killer && info->m_killer->GetPlayer() == PT_PLAYER)
            AIUtils::AddExp4Kill(experience, info->m_killer);
    }
};

//========================================================

//
// смена музыки в зависимости от врагов
//
class EnemyMusicChanger : public  MusicChanger,
                          private GameObserver,
                          private SpawnObserver,
                          private SpectatorObserver,
                          private RelationsObserver
{
public:

    EnemyMusicChanger() :
        m_flags(F_TURN_ON|F_UPDATE), m_danger(0), m_duration(0)
    {
        Spawner::Attach(this, ET_EXIT_LEVEL);
        Spawner::Attach(this, ET_ENTRY_LEVEL);
    }

    ~EnemyMusicChanger()
    {
        Shut();
        Spawner::Detach(this);
    }

    //нужно ли сменить музыку?
    bool NeedUpdate() const { return m_flags & F_UPDATE; }

    //функция анализа игровой ситуации
    music_type GetMusic()
    {
        if(m_flags & F_TURN_ON && m_flags & F_UPDATE){

            int danger_points = 0;

            VisMap::marker_itor itor = VisMap::GetInst()->marker_begin(PlayerSpectator::GetInst()->GetSpectator(), VT_VISIBLE);
            while(itor != VisMap::GetInst()->marker_end()){
                
                BaseEntity* entity = itor->GetEntity();
                
                if(entity &&  EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(entity) == RT_ENEMY){
                    
                    if(HumanEntity* human = entity->Cast2Human())
                        danger_points += human->GetInfo()->GetDangerPoints();
                    else if(VehicleEntity* vehicle = entity->Cast2Vehicle())
                        danger_points += vehicle->GetInfo()->GetDangerPoints();
                    
                }
                
                ++itor;
            }     

            if(m_danger < danger_points){
                m_danger = danger_points;
                m_duration = THEME_DURATION;
            }
            
            m_flags &= ~F_UPDATE;
        }

        if(m_danger > STRAINED_LIM) return MT_STRAINED;
        if(m_danger > UNEASY_LIM) return MT_UNEASY;

        return MT_QUIET;
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving()){
            slot << m_flags;
            slot << m_danger;
            slot << m_duration;
        }else{
            slot >> m_flags;
            slot >> m_danger;
            slot >> m_duration;

            Init();
        }
    }

private:

    void Update(Spectator* spectator, SpectatorObserver::event_t event, SpectatorObserver::info_t info)
    { 
        m_flags |= F_UPDATE;
    }
    
    void changed(RelationsObserver::event_type et, void* info) 
    { 
        EntityDesc* desc = static_cast<EntityDesc*>(info);

        BaseEntity* entity = EntityPool::GetInst()->Get(static_cast<EntityDesc*>(info)->m_id);
        unsigned flags = VisMap::GetInst()->GetVisFlags(PlayerSpectator::GetInst()->GetSpectator(), entity->GetEntityContext()->GetMarker());
        if(flags & VT_VISIBLE) m_flags |= F_UPDATE;
    }

    void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t ptr)
    {
        turn_info* info = static_cast<turn_info*>(ptr);

        if(info->m_player != PT_PLAYER)
            return;

        m_flags &= ~(F_TURN_OFF|F_TURN_ON);
        m_flags |= F_UPDATE;

        if(info->IsBegining()){            
            if(--m_duration <= 0) m_danger = 0;
            m_flags |= F_TURN_ON;
        }

        if(info->IsEnd()) m_flags |= F_TURN_OFF;               
    }

    void Update(SpawnObserver::subject_t subj, SpawnObserver::event_t event, SpawnObserver::info_t ptr)
    {
        if(event == ET_EXIT_LEVEL) Shut();
        if(event == ET_ENTRY_LEVEL) Init();
    }

    void Init()
    {
        GameEvMessenger::GetInst()->Attach(this, EV_TURN);
        EnemyDetector::getInst()->attach(this, ET_RELATION2PLAYER_CHANGED);
        
        PlayerSpectator::GetInst()->GetSpectator()->Attach(this, EV_VISIBLE_MARKER);
        PlayerSpectator::GetInst()->GetSpectator()->Attach(this, EV_INVISIBLE_MARKER);

        JukeBox::GetInst()->Reset();    
        JukeBox::GetInst()->SetChanger(this);
    }

    void Shut()
    {
				STACK_GUARD("EnemyMusicChanger::Shut");

        JukeBox::GetInst()->Reset();        
        
        EnemyDetector::getInst()->detach(this);
        GameEvMessenger::GetInst()->Detach(this);        
        PlayerSpectator::GetInst()->GetSpectator()->Detach(this);
    }
  
private:

    enum {

        F_UPDATE   = 1 << 0,
        F_TURN_ON  = 1 << 1,
        F_TURN_OFF = 1 << 2,

        UNEASY_LIM   = 50,
        STRAINED_LIM = 100,

        THEME_DURATION = 3,
    };

    int m_danger;
    int m_duration; 

    unsigned m_flags;
};

//========================================================

//
// класс для конфигурирования экрана HiddenMovement
//
class HiddenMovementConfigurator : private GameObserver{
public:

    HiddenMovementConfigurator()
    {
        GameEvMessenger::GetInst()->Attach(this, EV_TURN);
        GameEvMessenger::GetInst()->Attach(this, EV_START_GAME);
    }

    ~HiddenMovementConfigurator() { GameEvMessenger::GetInst()->Detach(this); }

private:

    void Update(subject_t subj, event_t event, info_t info)
    {
        if(event == EV_START_GAME){
            Cameraman::GetInst()->Configure(Cameraman::F_SHOW_NOTHING);
            return;
        }

        if(event == EV_TURN){

            turn_info* ptr = static_cast<turn_info*>(info);
            if(ptr->IsBegining()) Cameraman::GetInst()->Configure(Config4Player(ptr->m_player));
            return;
        }
    }

    unsigned Config4Player(player_type player)
    {
        if(player == PT_ENEMY || player == PT_CIVILIAN){
            
            unsigned result = Cameraman::F_SHOW_ALL & (~Cameraman::F_SHOW_ALL_OPTIONS);
            
            if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_ENEMY_RELATED_MOVE))
                result |= Cameraman::F_SHOW_ENEMY_RELATED_MOVE;
            
            if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_ENEMY_RELATED_SHOOT))
                result |= Cameraman::F_SHOW_ENEMY_RELATED_SHOOT;
            
            if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_FRIEND_RELATED_MOVE))
                result |= Cameraman::F_SHOW_FRIEND_RELATED_MOVE;
            
            if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_FRIEND_RELATED_SHOOT))
                result |= Cameraman::F_SHOW_FRIEND_RELATED_SHOOT;
            
            if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_NEUTRAL_RELATED_MOVE))
                result |= Cameraman::F_SHOW_NEUTRAL_RELATED_MOVE;
            
            if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_NEUTRAL_RELATED_SHOOT))
                result |= Cameraman::F_SHOW_NEUTRAL_RELATED_SHOOT;
            
            if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_CIVILIAN_VEHICLES_MOVE))
                result |= Cameraman::F_SHOW_CIVILIAN_VEHICLES_MOVE;
            
            if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_CIVILIAN_VEHICLES_SHOOT))
                result |= Cameraman::F_SHOW_CIVILIAN_VEHICLES_SHOOT;
            
            if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_CIVILIAN_HUMANS_TRADERS_MOVE))
                result |= Cameraman::F_SHOW_CIVILIAN_HUMANS_TRADERS_MOVE;
            
            if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_CIVILIAN_HUMANS_TRADERS_SHOOT))
                result |= Cameraman::F_SHOW_CIVILIAN_HUMANS_TRADERS_SHOOT;
            
            return result;
        }

        return Cameraman::F_SHOW_NOTHING;
    }
};

//========================================================

//
// интерфейсный класс инкапсулирующий модуль логики
//
class LogicFrame{
public:

    virtual ~LogicFrame(){}

    DCTOR_ABS_DEF(LogicFrame)

    //здесь можно рисовать
    virtual void Draw() = 0;
    //здесь можно думать
    virtual void Think() = 0;
    //сохранение/загрузка
    virtual void MakeSaveLoad(SavSlot& slot) = 0;  
};

//========================================================

//
// модуль обычной игры
//
class UsualLogicFrame : public LogicFrame{
public:

    //создание новой игры
    UsualLogicFrame(int)
    {
        EnableGraphDestroy(true);
        
        bar_ptr_t bar_ptr(FormFactory::GetInst()->CreateLoadBar(lower_load_bar_bound, upper_load_bar_bound));

        bar_ptr->SetPos(0);

        InitLogic();
                
        //заполнить массив игроками
        PlayerPool::GetInst()->Insert(PlayerFactory::GetInst()->CreateHumanPlayer());
        PlayerPool::GetInst()->Insert(PlayerFactory::GetInst()->CreateEnemyPlayer());
        PlayerPool::GetInst()->Insert(PlayerFactory::GetInst()->CreateCivilianPlayer());   
        
        bar_ptr->SetPos(0.1f);
        
        UsualLevelEnter  usual_enter;
        SpawnProgressBar spawn_bar(bar_ptr, 0.1f, 0.9f);        
        EntityBuilder    spawn_api;
        SpawnRules       spawn_rules;

        DoNotSpawnTeammateRule          rule1(ET_HUMAN, PT_PLAYER);
        DoNotSpawnKilledEntityRule      rule2(DeadList::GetHeroesList(), ET_HUMAN);
        DoNotSpawnKilledEntityRule      rule3(DeadList::GetTradersList(), ET_TRADER);
        DoNotSpawnKilledVehiclesRule    rule4(DeadList::GetVehiclesList());
		DifficultyRespawnerRule         rule5(DeadList::GetRespawnerDeadList());

        spawn_rules.Insert(&rule1);
        spawn_rules.Insert(&rule2);
        spawn_rules.Insert(&rule3);
        spawn_rules.Insert(&rule4);
        spawn_rules.Insert(&rule5);

        //расставить существ на уровне
        Spawner::GetInst()->Spawn(&spawn_api, &spawn_rules);    

        bar_ptr->SetPos(0.91f, DirtyLinks::GetStrRes("lgc_init_entities"));

        SetAllHumanStance();
        CalcMovepoints4All();

        //пусть все существа обновят данные о команде
        NotifyAllEntities(EntityObserver::EV_JOIN_TEAM);

        GameEvMessenger::GetInst()->Notify(GameObserver::EV_START_GAME);

        bar_ptr->SetPos(1.0f);
    }

    DCTOR_DEF(UsualLogicFrame)

    //здесь можно рисовать
    void Draw()
    {
        GraphGrid::GetInst()->Draw();
        //RiskArea::GetInst()->TestSmth();
        return;

#ifdef _HOME_VERSION
        
        // ~~~~~~~~~~~~~ test ~~~~~~~~~~~~~
        HexGrid::cell_iterator  first_cell = HexGrid::GetInst()->first_cell(),
                                last_cell  = HexGrid::GetInst()->last_cell();
        
        HexGrid::const_prop_iterator props = HexGrid::GetInst()->first_prop();
        point3   pnt3;
        BBox     piece;

        int counter = 0;
        
        while(first_cell != last_cell){

            pnt3 = *first_cell;
            piece.Box(pnt3, 0.15);

            //if((counter++ % 50) == 0) first_cell->SetFireDuration(2);
           
            if( first_cell->IsBusRoute()) DirtyLinks::DrawBBox(piece, 0xff00ff);
            
            //if(!first_cell->IsDefPnt()) DirtyLinks::DrawBBox(piece, 0xffffff);
            if( first_cell->GetEntity()) DirtyLinks::DrawBBox(piece, 0x00ff00);
            //if( first_cell->IsLandPnt()) DirtyLinks::DrawBBox(piece, 0xff00ff);
            //if(first_cell->IsPassable) DirtyLinks::DrawBBox(piece, 0x00ffff);
            ++first_cell;
        }
        
#endif

        // ~~~~~~~~~~~~~ test ~~~~~~~~~~~~~    
    }

    //здесь можно думать
    void Think()
    {
		STACK_GUARD("UsualLogicFrame::Think");
        PlayerPool::GetInst()->Think(); 

		/////////////////////////////////////////////////////////////////////////
		//	Naughty: 14.05.02 для фиксирования бага с Helper'ом
		if(!Screens::Instance()->IsOptionsPresent())
			HelperManager::GetInst()->Tick();
		/////////////////////////////////////////////////////////////////////////

        GraphEntity::Update();
        JukeBox::GetInst()->Tick();

#ifdef _HOME_VERSION

        //~~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~

        if(Input::KeyBack(DIK_V)){

            static int counter;

            if(++counter % 2)
                GraphEntity::DropFlags(GraphEntity::FT_ALWAYS_VISIBLE);
            else
                GraphEntity::RaiseFlags(GraphEntity::FT_ALWAYS_VISIBLE);
        }

        if(Input::KeyBack(DIK_T)) QuestServer::GetInst()->TestSmth(); 

 /*      
        if(Input::KeyBack(DIK_T)){

            point3 center = HexGrid::GetInst()->Get(ipnt2_t(
                                HexGrid::GetInst()->GetSizeX() * NormRand(),
                                HexGrid::GetInst()->GetSizeY() * NormRand()));

            float radius = 7.0f * NormRand() + 3.0f;

            RiskArea::GetInst()->InsertFlame(center, radius, 100);
        }

        if(Input::KeyBack(DIK_T)){

            ForceHandbook::GetInst()->Push(0, "htpd", ForceHandbook::F_NEW_REC);           
            ForceHandbook::GetInst()->Push(0, "crusaders", ForceHandbook::F_NEW_REC);           
            
            ThingHandbook::GetInst()->Push(TT_WEAPON, "atomic_flower", ThingHandbook::F_NEW_REC);
            ThingHandbook::GetInst()->Push(TT_ARMOUR, "kerk_butterfly", ThingHandbook::F_NEW_REC);
            ThingHandbook::GetInst()->Push(TT_CAMERA, "handycam", ThingHandbook::F_NEW_REC);

            EntityHandbook::GetInst()->Push(ET_HUMAN, "dinara", EntityHandbook::F_NEW_REC);
            EntityHandbook::GetInst()->Push(ET_VEHICLE, "atheist", EntityHandbook::F_NEW_REC);           

            DiaryManager::GetInst()->Insert(DiaryManager::Record("hello punch",
                                            DiaryManager::ROOT_KEY,
                                            DiaryManager::Record::F_NEW));

            DiaryManager::GetInst()->Insert(DiaryManager::Record("hello punch1",
                                            DiaryManager::ROOT_KEY,
                                            DiaryManager::Record::F_NEW));

            DiaryManager::key_t key = DiaryManager::GetInst()->Insert(
                                            DiaryManager::Record("hello punch2",
                                            DiaryManager::ROOT_KEY,
                                            DiaryManager::Record::F_NEW));

            DiaryManager::GetInst()->Insert(DiaryManager::Record("hello punch3",
                                            key,
                                            DiaryManager::Record::F_NEW));

            DiaryManager::GetInst()->Insert(DiaryManager::Record("hello punch4",
                                            key,
                                            DiaryManager::Record::F_NEW));

            DiaryManager::GetInst()->Insert(DiaryManager::Record("hello punch5",
                                            key,
                                            DiaryManager::Record::F_NEW));
            
        }   

        if(Input::KeyBack(DIK_NUMPADMINUS)){

            std::auto_ptr<DiaryManager::Iterator> itor(DiaryManager::GetInst()->CreateIterator());

            itor->First();
            while(itor->IsNotDone()){

                if(itor->Get()->GetInfo() == "hello punch2"){
                    DiaryManager::GetInst()->Delete(itor->Get()->GetKey());
                    break;
                }

                itor->Next();
            }
        } 

        if(Input::KeyBack(DIK_NUMPADPLUS)){

            EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);

            if(itor != EntityPool::GetInst()->end()){
                itor->Cast2Human()->GetEntityContext()->GetTraits()->AddExperience(10000);
            }
        }

        if(Input::KeyBack(DIK_NUMPADMINUS)){

            EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);

            if(itor != EntityPool::GetInst()->end()){
                itor->Cast2Human()->GetEntityContext()->GetTraits()->AddExperience(- 100);
            }
        } */

        //~~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~

#endif // _HOME_VERSION
    }
    
    struct load_notifier{
        
        load_notifier(SavSlot& slot) : m_fload(!slot.IsSaving())
        { if(m_fload) GameEvMessenger::GetInst()->Notify(GameObserver::EV_INIT_GAME_LOAD); }

        ~load_notifier()
        { if(m_fload) GameEvMessenger::GetInst()->Notify(GameObserver::EV_START_GAME); }

    private:

        bool m_fload;
    };

    //сохранение/загрузка
    void MakeSaveLoad(SavSlot& slot)
    {
        load_notifier notifier(slot);

        SaveLoadBar load_bar(slot.IsSaving() ? 0 : FormFactory::GetInst()->CreateLoadBar(0.5f, 1.0f));

        //запретиь удаление графич оболочек, чтобы не убить уже загруженные
        EnableGraphDestroy(false);

        load_bar.SetPos(0);

        {
            EntityLoadProgressBar ent_load_bar(&load_bar, 0, 0.4f);
        
            //если загрузка обычной игры созданим каркас заново
            if(!slot.IsSaving()){
                
                Forms::GetInst()->Reset(Forms::RT_SHUT);
                
                InitLogic();
                
                DestroyEntities(AttributeCondition());
            }
                        
            EntityPool::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_entities"));
            if(!slot.IsSaving()) RelinkAllEntities();  
        }

        load_bar.SetPos(0.40f, DirtyLinks::GetStrRes("lgc_load_level_data"));
        
        //разберемся с данными об уровнях
        m_level_data.MakeSaveLoad(SavSlot(slot.GetStore(), "logic_level_data"));
        Bureau::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_bureau"));

        load_bar.SetPos(0.46f);

		NewsPool::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_news"));
        MoneyMgr::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_money"));
        DiaryManager::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_diary"));

        load_bar.SetPos(0.52f);

        EpisodeMapMgr::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_levels"));        
        CameraPool::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_camera_pool"));
        ShieldPool::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_shield_pool"));

        load_bar.SetPos(0.58f);

        GameObjectsMgr::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_objects_mgr"));
        PhraseManager::GetFirstPhrases()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_first_phrase"));
        EnemyDetector::getInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_enemy_detector"));

        load_bar.SetPos(0.64f, DirtyLinks::GetStrRes("lgc_load_players_data"));

        PlayerSpectator::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_player_spectator"));
        Depot::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_depot"));
        PlayerPool::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_players"));

        load_bar.SetPos(0.70f);

        if(slot.GetStore()->GetVersion() > 3)
            ForceHandbook::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_force_handbook"));
        else
            ForceHandbook::GetInst()->Reset();

        ThingHandbook::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_thing_handbook"));
        EntityHandbook::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_entity_handbook"));
        DeadList::GetHeroesList()->MakeSaveLoad(SavSlot(slot.GetStore(), "level_heroes_dead_list"));

        load_bar.SetPos(0.76f);

        DeadList::GetTradersList()->MakeSaveLoad(SavSlot(slot.GetStore(), "level_traders_dead_list"));
        DeadList::GetVehiclesList()->MakeSaveLoad(SavSlot(slot.GetStore(), "level_vehicles_dead_list"));
        DeadList::GetRespawnerDeadList()->MakeSaveLoad(SavSlot(slot.GetStore(), "level_all_dead_list"));
		

        //fixme::grom
        if(!slot.IsSaving()){            
            GraphGrid::GetInst()->Show(GraphGrid::HT_JOINTS, true);
            AIUtils::CalcAndShowActiveExits();
        }

        load_bar.SetPos(0.84f);

        QuestServer::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_quests"));       
        HelperManager::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_helper"));

        load_bar.SetPos(0.90f, DirtyLinks::GetStrRes("lgc_load_menu_data"));
        
        SSceneMgr::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_scr_scenes"));
        Spawner::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_spawner"));
        Forms::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_menus"));

        load_bar.SetPos(0.96f);

        RiskArea::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_risk_area"));
        m_changer.MakeSaveLoad(SavSlot(slot.GetStore(), "logic_music_changer"));

        //разрешить удаление графич оболочек
        EnableGraphDestroy(true); 

        load_bar.SetPos(1.0f, DirtyLinks::GetStrRes("lgc_load_complete"));
   }

private:

    //загрузка новой игры из Save'a
    UsualLogicFrame() {}

    //создать каркас игры
    void InitLogic()
    {
        GraphGrid::GetInst()->Show(GraphGrid::HT_JOINTS, true);

        BusDispatcher::GetInst()->Reset();
                  
        Depot::GetInst()->Reset();
        Bureau::GetInst()->Reset();
        VisMap::GetInst()->Reset();
        PlayerPool::GetInst()->Reset();
        CameraPool::GetInst()->Reset();
        ShieldPool::GetInst()->Reset();

        GameObjectsMgr::GetInst()->Reset();
        PlayerSpectator::GetInst()->Reset();

        VisMap::GetInst()->Insert(PlayerSpectator::GetInst()->GetSpectator());

        //создать обычную
        Spawner::CreateUsual();
    }

private:
    
    EnemyMusicChanger m_changer;
    UsualGameObserver m_game_obs;
    UsualLevelData    m_level_data;

    InitQuestServer          m_quests;
    PrintForceRelationChange m_relations;

    HiddenMovementConfigurator m_hidden_movent_config;
};

DCTOR_IMP(UsualLogicFrame)

//========================================================

class GameLogicImp : public GameLogic{
public:

    void Init()
    {
        JukeBox::GetInst()->Init();
        NewsPool::GetInst()->Init();        
        SSceneMgr::GetInst()->Init();
        SndPlayer::GetInst()->Init();
        DeadList::GetHeroesList()->Init();
        DeadList::GetRespawnerDeadList()->Init();
        DeadList::GetTradersList()->Init();
        DeadList::GetVehiclesList()->Init();
        HelperManager::GetInst()->Init();

        PhraseManager::GetUsePhrases()->Init();
        PhraseManager::GetFirstPhrases()->Init();
    }

    void Shut()
    {
		STACK_GUARD("GameLogic::Shut");
        delete m_frame.release();

        Forms::GetInst()->Reset(Forms::RT_SHUT);

        Depot::GetInst()->Reset();
        Bureau::GetInst()->Reset();
        VisMap::GetInst()->Reset();
        JukeBox::GetInst()->Reset();
        CameraPool::GetInst()->Reset();
        ShieldPool::GetInst()->Reset();

        ForceHandbook::GetInst()->Reset();
        ThingHandbook::GetInst()->Reset();
        EntityHandbook::GetInst()->Reset();

        NewsPool::GetInst()->Shut();
        SSceneMgr::GetInst()->Shut();
        SndPlayer::GetInst()->Shut();
        QuestServer::GetInst()->Shut();
        HelperManager::GetInst()->Shut();
        DeadList::GetHeroesList()->Shut();
        DeadList::GetTradersList()->Shut();
        DeadList::GetVehiclesList()->Shut();
        DeadList::GetRespawnerDeadList()->Shut();

        PhraseManager::GetUsePhrases()->Shut();
        PhraseManager::GetFirstPhrases()->Shut();
                
        //уничтожаем игроков в конце
        PlayerPool::GetInst()->Reset();

        DestroyEntities(AttributeCondition());
    }

    //AI думает в этом tick'е
    void Think() { m_frame->Think(); }

    //необходимо для отрисовки игровых существ
    void Draw()  { m_frame->Draw(); }

    //загрузка/сохранение игры
    void MakeSaveLoad(Storage& st)
    {
        SavSlot slot(&st, "game_logic");
        
        if(slot.IsSaving()){        
            
            DynUtils::SaveObj(slot, &(*m_frame));

        }else{

            delete m_frame.release();

            LogicFrame* frame = 0;
            DynUtils::LoadObj(slot, frame);

            m_frame = frame_ptr_t(frame);
        }
        
        m_frame->MakeSaveLoad(slot);
    }

    //начало обычной игры
    void BegNewGame()
    {
        delete m_frame.release();
        m_frame = frame_ptr_t(new UsualLogicFrame(0));
    }

    //начало сетевой игры
    void BegNetGame() { delete m_frame.release(); }

private:

    typedef std::auto_ptr<LogicFrame> frame_ptr_t;
    frame_ptr_t m_frame;
};

} // namespace

//========================================================

GameLogic* GameLogic::GetInst()
{
    static GameLogicImp imp;
    return &imp;
}
