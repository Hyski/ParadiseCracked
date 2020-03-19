#pragma warning(disable:4786)

#include "logicdefs.h"

#include "../interface/screens.h"
#include "../interface/SlotsEngine.h" 
#include "../interface/inventoryscreen.h"

#include "form.h"
#include "spawn.h"
#include "thing.h"
#include "sscene.h"
#include "entity.h"
#include "aiutils.h"
#include "graphic.h"
#include "sndutils.h"
#include "entityaux.h"
#include "dirtylinks.h"
#include "thingfactory.h"
#include "phrasemanager.h"
#include "entityfactory.h"
#include "enemydetection.h"

#include "questserver.h"
#include "questengine.h"

namespace{

//
// разбор скрипта и выаод информации об ошибке
//
bool ParseThingScript(const rid_t& rid, ThingScriptParser& parser)
{
    int  line;
    std::string script;

    //получим описание скрипта item_set.xls
    if(!AIUtils::GetItemSetStr(rid, &script, &line)){
        std::ostringstream ostr;
        ostr << "GeneralQuestServer: нет набора предметов <" << rid << "> в item_set.xls";
        DirtyLinks::Print(ostr.str());
        return false;
    }

    //разбор скрита
    if(!parser.Parse(script)){
        
        std::ostringstream ostr;
        ostr << "GeneralQuestServer: ошибочный набор предметов <" << rid << ">";
        DirtyLinks::Print(ostr.str());

        ostr.str(""); ostr.clear();
        ostr << "строка в item_set.xls: " << line + 1;
        DirtyLinks::Print(ostr.str());

        ostr.str(""); ostr.clear();
        ostr << "позиция в строке: " << parser.GetLastPos();
        DirtyLinks::Print(ostr.str());
        return false;
    }

    return true;
}

//==================================================================

//
// обработка порождения предмета для quest'ов
//
class QuestThingFactory : public ThingFactory{
public:
        
    BaseThing* CreateKey(const rid_t& rid)
    {
        CreateThing(TT_KEY, rid);
        return 0;
    }
    
    BaseThing* CreateMoney(int sum)
    {
        CreateThing(TT_MONEY, "money");
        return 0;
    }
    
    BaseThing* CreateArmor(const rid_t& rid)
    {
        CreateThing(TT_ARMOUR, rid);
        return 0;
    }
    
    BaseThing* CreateShield(const rid_t& rid)
    {
        CreateThing(TT_SHIELD, rid);
        return 0;
    }
    
    BaseThing* CreateCamera(const rid_t& rid)
    {
        CreateThing(TT_CAMERA, rid);
        return 0;
    }
    
    BaseThing* CreateGrenade(const rid_t& rid)
    {
        CreateThing(TT_GRENADE, rid);
        return 0;
    }
    
    BaseThing* CreateScanner(const rid_t& rid)
    {
        CreateThing(TT_SCANNER, rid);
        return 0;
    }
    
    BaseThing* CreateImplant(const rid_t& rid)
    {
        CreateThing(TT_IMPLANT, rid);
        return 0;
    }
    
    BaseThing* CreateAmmo(const rid_t& rid, int cout)
    {
        CreateThing(TT_AMMO, rid);
        return 0;
    }
    
    BaseThing* CreateMedikit(const rid_t& rid, int charge)
    {
        CreateThing(TT_MEDIKIT, rid);
        return 0;
    }
    
    BaseThing* CreateWeapon(const rid_t& rid, const rid_t& ammo_rid, int ammo_count)
    {
        CreateThing(TT_WEAPON, rid);
        return 0;
    }
    
protected:
    
    virtual void CreateThing(thing_type type, const rid_t& rid) = 0;
};

//
// поиск предметов у команды
//
class ThingsFinder : public QuestThingFactory{
public:
    
    ThingsFinder(player_type player = PT_PLAYER) : 
      m_fall_found(true), m_player(player), m_money(0) {}     
     
    bool AllFound() const { return m_fall_found; }
      
private:
    
    BaseThing* CreateMoney(int sum)
    {
        if(m_fall_found){
            m_money += sum;
            m_fall_found = m_money <= MoneyMgr::GetInst()->GetMoney();
        }
          
        return 0;
    }

    void CreateThing(thing_type type, const rid_t& rid)
    {
        if(m_fall_found){
            m_fall_found =      HaveThing(HPK_HEAD, type, rid)
                            ||  HaveThing(HPK_BODY, type, rid)
                            ||  HaveThing(HPK_HANDS, type, rid)
                            ||  HaveThing(HPK_LKNEE, type, rid)
                            ||  HaveThing(HPK_RKNEE, type, rid)
                            ||  HaveThing(HPK_BACKPACK, type, rid)
                            ||  (type != TT_IMPLANT && HaveThing(HPK_IMPLANTS, type, rid));
        }
    }        
    
    bool HaveThing(human_pack_type pack, thing_type thing, const rid_t& rid)
    {
        EntityPool::iterator eit = EntityPool::GetInst()->begin(ET_HUMAN, m_player);
        while(eit != EntityPool::GetInst()->end()){
            
            HumanContext* context = eit->Cast2Human()->GetEntityContext();
            HumanContext::iterator tit = context->begin(pack, thing);
            
            while(tit != context->end()){
                
                if(tit->GetInfo()->GetRID() == rid)
                    return true;
                
                ++tit;
            }
            
            ++eit;
        }
        
        return false;
    }
    
private:
    
    player_type m_player;
    
    int  m_money;
    bool m_fall_found;
};

//
// отобрать предметы у команды
//
class ThingsTaker : public QuestThingFactory{
public:
    
    ThingsTaker(player_type player = PT_PLAYER) : m_player(player) {}
   
private:

    BaseThing* CreateMoney(int sum)
    {
        MoneyMgr::GetInst()->SetMoney(MoneyMgr::GetInst()->GetMoney() - sum);
        return 0;
    }
    
    void CreateThing(thing_type type, const rid_t& rid)
    {
        if(     RemoveThing(HPK_HEAD, type, rid)
            ||  RemoveThing(HPK_BODY, type, rid)
            ||  RemoveThing(HPK_HANDS, type, rid)
            ||  RemoveThing(HPK_LKNEE, type, rid)
            ||  RemoveThing(HPK_RKNEE, type, rid)
            ||  RemoveThing(HPK_BACKPACK, type, rid)
            ||  (type != TT_IMPLANT && RemoveThing(HPK_IMPLANTS, type, rid)))
            return;
    }
    
    bool RemoveThing(human_pack_type pack, thing_type type, const rid_t& rid)
    {
        EntityPool::iterator eit = EntityPool::GetInst()->begin(ET_HUMAN, m_player);
        while(eit != EntityPool::GetInst()->end()){
            
            HumanContext* context = eit->Cast2Human()->GetEntityContext();
            HumanContext::iterator tit = context->begin(pack, type);
            
            while(tit != context->end()){
                
                if(tit->GetInfo()->GetRID() == rid){
                    
                    BaseThing* thing = &*tit;
                    
                    context->RemoveThing(thing);
                    delete thing;
                    
                    return true;
                }
                
                ++tit;
            }
            
            ++eit;
        }

        return false;
    }
    
private:
    
    player_type m_player;
};

//
// дать предметы персонажу
//
class ThingGiver : public ThingScriptParser::Acceptor{
public:

    ThingGiver(BaseEntity* actor) :
      m_actor(actor) { m_actor->Accept(init()); }

    ~ThingGiver() { m_actor->Accept(shut());}

    bool AcceptThing(BaseThing* thing)
    {
        m_actor->Accept(give_thing(thing));
        return true;
    }

    //принять информацию об организации
    void AcceptOrgInfo(const rid_t rid)
    {
        if(m_actor->GetPlayer() == PT_PLAYER) ForceHandbook::GetInst()->Push(0, rid, ForceHandbook::F_NEW_REC);
    }

    //принять информацию о существах
    void AcceptEntInfo(entity_type type, const rid_t& rid)
    {
        if(m_actor->GetPlayer() == PT_PLAYER) EntityHandbook::GetInst()->Push(type, rid, EntityHandbook::F_NEW_REC);
    }

    //принять информацию о предметах
    void AcceptThingInfo(thing_type type, const rid_t& rid)
    {
        if(m_actor->GetPlayer() == PT_PLAYER) ThingHandbook::GetInst()->Push(type, rid, ThingHandbook::F_NEW_REC);
    }

private:

    class init : public EntityVisitor{
    public:

        void Visit(HumanEntity* human)
        {
            SlotsEngine*    engine  = Screens::Instance()->Inventory()->GetSlotsEngine();
            SESlot*         slot    = engine->SlotAt(InventoryScreen::S_BACKPACK);
            HumanContext*   context = human->GetEntityContext();

            slot->Clear();
            HumanContext::iterator thing_itor = context->begin(HPK_BACKPACK); 

            //заполним рюкзак
            while(thing_itor != context->end()){
                
                SEItem* item = engine->CreateItem(thing_itor->GetInfo()->GetShader().c_str(),
                                                  thing_itor->GetInfo()->GetSize(), &*thing_itor);

                item->SetPosition(thing_itor->GetPakPos());
                
                if(!human->IsRaised(EA_WAS_IN_INV) && !slot->CanInsert(item))
                    throw CASUS("ThingGiver: не хватает места в рюкзаке для передмета!");                    

                slot->Insert(item);
            
                ++thing_itor;
            }
        }
    };

    class shut : public EntityVisitor{
    public:

        void Visit(HumanEntity* human)
        {
            SlotsEngine* engine = Screens::Instance()->Inventory()->GetSlotsEngine();
            SESlot*      slot   = engine->SlotAt(InventoryScreen::S_BACKPACK);

            //сохраним коорд. предметов в рюкзаке
            SESlot::Iterator slot_itor = slot->Begin();
            while(slot_itor != slot->End()){
                slot_itor->GetBaseThing()->SetPakPos(slot_itor->Position());
                ++slot_itor;
            }

            slot->Clear();
        }
    };

    class give_thing : public EntityVisitor{
    public:

        give_thing(BaseThing* thing) : m_thing(thing) {}

        void Visit(HumanEntity* human)
        {
            SlotsEngine*  engine  = Screens::Instance()->Inventory()->GetSlotsEngine();
            SESlot*       slot    = engine->SlotAt(InventoryScreen::S_BACKPACK);
            HumanContext* context = human->GetEntityContext();

            SEItem* item = engine->CreateItem(m_thing->GetInfo()->GetShader().c_str(), m_thing->GetInfo()->GetSize(), m_thing);
            if(slot->CanInsert(item)){

                slot->Insert(item);
                context->InsertThing(HPK_BACKPACK, m_thing);

            }else{

                //бросим предмет на землю
                if(VehicleEntity* vehicle = context->GetCrew())
                    Depot::GetInst()->Push(vehicle->GetGraph()->GetPos2(), m_thing);
                else
                    Depot::GetInst()->Push(human->GetGraph()->GetPos2(), m_thing);

                delete item;
            }
        }

        void Visit(VehicleEntity* vehicle)
        {
           Depot::GetInst()->Push(vehicle->GetGraph()->GetPos2(), m_thing); 
        }

        void Visit(TraderEntity* trader)
        {
            trader->GetEntityContext()->InsertThing(m_thing);
        }

    private:

        BaseThing* m_thing;
    };

private:

    BaseEntity* m_actor;    
};

//============================================================

//
// служба построения отчета по законченному quest'y
//
class ReportBuilderImp : public  LogicServer1::ReportBuilder,
                         private ThingFactory
{
public:
    
    void Start()
    {
        m_money = 0;
        m_things = 0;
        m_experience = 0;
        m_event = ET_NONE;
    }

    void Finish()
    {
        switch(m_event){
        case ET_START:
            DirtyLinks::Print(DirtyLinks::GetStrRes("gqs_quest_started"));
            break;

        case ET_FINISH:
            DirtyLinks::Print(DirtyLinks::GetStrRes("gqs_quest_finished"));
            break;
        };

        if(m_money || m_things || m_experience){

            std::ostringstream ostr;

            ostr << DirtyLinks::GetStrRes("gqs_got_bonus");

            bool need_comma = false;

            if(m_money) ostr << ' ' << mlprintf( DirtyLinks::GetStrRes("gqs_got_money").c_str(), m_money);
            
            if(m_experience){
                if(m_money) ostr << ',';
                ostr << ' ' << mlprintf( DirtyLinks::GetStrRes("gqs_got_experience").c_str(), m_experience);
            }
            
            if(m_things){
                if(m_money || m_experience) ostr << ',';
                ostr << ' ' << DirtyLinks::GetStrRes("gqs_got_equipment");
            }

            DirtyLinks::Print(ostr.str());
        }
    }

    void AddExperience(int experience)
    { m_experience += experience; }

    void AddItems(const std::string& sys_name)
    { ParseThingScript(sys_name, ThingScriptParser(0, this)); }

    void AddEvent(event_type type)
    { m_event = type; }

public:
        
    BaseThing* CreateKey(const rid_t& rid)
    {  m_things++; return 0; }
    
    BaseThing* CreateMoney(int sum)
    { m_money += sum; return 0; }
    
    BaseThing* CreateArmor(const rid_t& rid)
    {  m_things++; return 0; }
    
    BaseThing* CreateShield(const rid_t& rid)
    {  m_things++; return 0; }
    
    BaseThing* CreateCamera(const rid_t& rid)
    {  m_things++; return 0; }
    
    BaseThing* CreateGrenade(const rid_t& rid)
    {  m_things++; return 0; }
    
    BaseThing* CreateScanner(const rid_t& rid)
    {  m_things++; return 0; }
    
    BaseThing* CreateImplant(const rid_t& rid)
    {  m_things++; return 0; }
    
    BaseThing* CreateAmmo(const rid_t& rid, int cout)
    {  m_things++; return 0; }
    
    BaseThing* CreateMedikit(const rid_t& rid, int charge)
    {  m_things++; return 0; }
    
    BaseThing* CreateWeapon(const rid_t& rid, const rid_t& ammo_rid, int ammo_count)
    {  m_things++; return 0; }

private:

    int m_money;
    int m_things;
    int m_experience;

    event_type m_event;
};

//============================================================

//
// класс предоставляющий средства для работы с существом
//
class EntityManipulatorImp : public LogicServer1::EntityManipulator{
public:

    EntityManipulatorImp(BaseEntity* actor = 0) : m_actor(actor) {}

    //установить текущее существо
    void SetActor(BaseEntity* actor) { m_actor = actor; }
    
    //узнать отношение существа к команде игрока 
    RelationType GetRelation2Player() const
    {
        if(m_actor){

            EntityPool::iterator itor = EntityPool::GetInst()->begin();
            
            while(itor != EntityPool::GetInst()->end()){
                
                if(itor->GetInfo()->GetRID() == m_actor->GetInfo()->GetRID())
                    return EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(&*itor);
                
                ++itor;
            }
        }
        
        return RT_NEUTRAL;
    }

    entity_type GetType() const
    { return m_actor ? m_actor->GetType() : ET_NONE; }


    const std::string& GetName() const
    { return m_actor ? m_actor->GetInfo()->GetName() : m_null_str; }
    
    //дать команде опыт
    void GiveExperience(int value)
    { if(m_actor) AIUtils::AddExp4Quest(value, m_actor); }

    //сказать фразу
    void SayPhrase(phrase_type type)
    {
        if(m_actor == 0) return;

        switch(type){
        case PT_USE:
            if(PhraseManager::GetUsePhrases()->IsExists(m_actor)){                        
                SayPhrase(PhraseManager::GetUsePhrases()->GetPhrase(m_actor));
            }
            break;

        case PT_FIRST:
            if(PhraseManager::GetFirstPhrases()->CanSay(m_actor)){ 

                PhraseManager::GetFirstPhrases()->MarkAsSaid(m_actor);                

                if(!PhraseManager::GetFirstPhrases()->IsExists(m_actor)){
                    SayPhrase(PT_USE);
                    return;
                }

                SayPhrase(PhraseManager::GetFirstPhrases()->GetPhrase(m_actor));
            }
            break;
        }
    }

    //сказать фразу
    void SayPhrase(const phrase_s& phrase)
    { if(m_actor) Forms::GetInst()->ShowTalkDialog(m_actor, phrase); }

    //узнать системное имя персонажа
    const std::string& GetSysName() const
    { return m_actor ? m_actor->GetInfo()->GetRID() : m_null_str; }
    
    //узнать игрока к кот. принадлежит существо
    player_type GetPlayer() const
    { return m_actor ? m_actor->GetPlayer() : PT_NONE; }

    //дать предметы команде
    void GiveItems(const std::string &itemset)
    {
        if(m_actor == 0) return;

        ThingGiver giver(m_actor);
        ParseThingScript(itemset, ThingScriptParser(&giver, ThingFactory::GetInst()));
    }
    
    //перевести существо в другую команду
    void ChangePlayer(player_type new_player, const std::string& ai_model)
    {
        if(m_actor == 0) return;

        //если присоединение к команде игрока, проверим в ней кол-во народу
        if(m_actor->Cast2Human() && new_player == PT_PLAYER){

            int number_of_humans = 0;

            //посчитаем кол-во народу
            EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);
            while(itor != EntityPool::GetInst()->end()){
                ++ itor;
                ++ number_of_humans;
            }

            //если кол-во народу больше, чем нужно выход
            if(number_of_humans >= MAX_TEAMMATES){
                Forms::GetInst()->ShowMessageBox( mlprintf(                        
                        DirtyLinks::GetStrRes("gqs_can_not_add_hero").c_str(),
                        m_actor->GetInfo()->GetName().c_str()));
                return;
            }
        }

        if(ai_model.size()){
            EntityBuilder builder;
            builder.SetAIModel(m_actor, ai_model);
        }

        AIUtils::ChangeEntityPlayer(m_actor, new_player);
    }

private:

    BaseEntity* m_actor;
    std::string m_null_str;
};

//============================================================

//
// сервис для итерации по масииву существ
//
class EntityIteratorImp : public LogicServer1::EntityIterator{
public:
    
    //установить параметры итерирования, перейти к первому сущетсву
    void First(const std::string& sys_name, player_type player)
    {  
        m_rid  = sys_name;
        m_itor = Find(EntityPool::GetInst()->begin(ET_ALL_ENTS, player), EntityPool::GetInst()->end());
    }
    
    //перейти к следущему существу
    void Next()
    {
        m_itor = Find(++m_itor, EntityPool::GetInst()->end());
    }

    //закончена ли итерация
    bool IsDone() const { return m_itor == EntityPool::GetInst()->end(); }
    
    //получить сылку на манипулятор существа
    LogicServer1::EntityManipulator* Get()
    {
        if(IsDone()) return 0;

        m_manip.SetActor(&(*m_itor));
        return &m_manip;
    }

private:

    bool IsSuitable(const BaseEntity* entity) const
    { return entity->GetInfo()->GetRID() == m_rid; }

    EntityPool::iterator Find(EntityPool::iterator first, EntityPool::iterator last)
    {
        while(first != last){

            if(IsSuitable(&(*first)))
                return first;

            ++first;
        }

        return last;
    }

private:

    rid_t m_rid;

    EntityPool::Iterator m_itor;
    EntityManipulatorImp m_manip; 
};

//======================================================================

//
// реализация сервера квестов
//
class LogicServer1Imp : public LogicServer1{
public:

    //получить доступ к службе составления отчета
    ReportBuilder* GetReportBuilder()
    { return &m_report_builder;  }
    
    //уведомление о событии в квестах
    void Notify(quest_event_type event, void* info)
    {
        if(event == QET_NEW_QUEST_START){
            GameEvMessenger::GetInst()->Notify(GameObserver::EV_NEW_QUEST);
            SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_NEW_QUEST));
        }

        if(event == QET_QUEST_FINISHED){
            GameObserver::quest_info qinfo(static_cast<quest_info*>(info)->m_quest);
            GameEvMessenger::GetInst()->Notify(GameObserver::EV_QUEST_FINISHED, &qinfo);
        }
    }

    //получить ссылку на объект итератор
    EntityIterator* GetEntityIterator()
    { return &m_entity_iterator; }
    
    //показать MessageBox 
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //возврат:
    //         true  -  была нажата кнопка OK(Yes)
    //         false -  была нажата кнопка Cancel(No) 
    //
    bool ShowMessageBox(const std::string& text)
    { return Forms::GetInst()->ShowMessageBox(text); }
    
    //отобрать предметы у команды
    void TakeItems(const std::string &itemset)
    {
        ThingsTaker taker;
        ParseThingScript(itemset, ThingScriptParser(0, &taker));
    }

    //можно ли забрать набор предметов у команды
    bool CanGetItems(const std::string &itemset) const
    {
        ThingsFinder finder;
        
        return      ParseThingScript(itemset, ThingScriptParser(0, &finder))
                &&  finder.AllFound();
    }

    bool IsHeroDead(const std::string &sys_name) const
    {
        SpawnTag tag;

        tag.SetSysName(sys_name);

        tag.SetEntityType(ET_HUMAN);
        if(DeadList::GetHeroesList()->GetKilledCount(tag))
            return true;

        tag.SetEntityType(ET_TRADER);
        if(DeadList::GetTradersList()->GetKilledCount(tag))
            return true;

        return false;
    }

    //закончилась ли скриптовая сцена
    bool IsSceneFinished(const std::string &scene_name) const
    { return SSceneMgr::GetInst()->IsScriptSceneFinished(scene_name); }

    //изменить игровую фазу
    void ChangePhase(int phase)
    { Spawner::GetInst()->SetPhase(phase);  }

    //возвращает текущую игровую фазу
    int GetCurrentPhase() const
    { return Spawner::GetInst()->GetPhase(); }

    //разрешить/запретить выход на уровень
    void EnableLevel(const std::string &lev_name, bool enable)
    {  Spawner::GetInst()->EnableExit(lev_name, enable); }

    // изменить отношение сторон друг к другу
    void ChangeAlignment(const std::string &side_a, const std::string &side_b, RelationType type)
    { EnemyDetector::getInst()->setRelation(side_a, side_b, type); }
    
    //добавляет новость в журнал
    void AddNews(const std::string& sys_name)
    { NewsPool::GetInst()->Push(sys_name); }

    //полностью стирает запись из журнала
    void RemoveJournalTheme(const std::string& theme)
    {
        std::auto_ptr<DiaryManager::Iterator> ptr(DiaryManager::GetInst()->CreateIterator());
        
        //ищем запись на тему
        ptr->First(DiaryManager::ROOT_KEY);
        while(ptr->IsNotDone()){
            
            if(ptr->Get()->GetInfo() == theme){
                DiaryManager::GetInst()->Delete(ptr->Get()->GetKey());
                return;
            }
            
            ptr->Next();
        }
    }

    //Добавляет запись в журнал
    void AddJournalRecord(const std::string& theme, const std::string& text)
    {
        std::auto_ptr<DiaryManager::Iterator> ptr(DiaryManager::GetInst()->CreateIterator());

        DiaryManager::key_t theme_key = DiaryManager::NULL_KEY;

        //ищем запись на тему
        ptr->First(DiaryManager::ROOT_KEY);
        while(ptr->IsNotDone()){

            if(ptr->Get()->GetInfo() == theme){
                theme_key = ptr->Get()->GetKey();
                break;
            }

            ptr->Next();
        }

        //если записи на тему нет создадим ее
        if(theme_key == DiaryManager::NULL_KEY){

             theme_key = DiaryManager::GetInst()->Insert(
                                            DiaryManager::Record(theme,
                                            DiaryManager::ROOT_KEY,
                                            DiaryManager::Record::F_NEW));
        }

        //добавим запись в тему
        DiaryManager::GetInst()->Insert(DiaryManager::Record(text, theme_key, DiaryManager::Record::F_NEW));
    }

private:

    ReportBuilderImp   m_report_builder;
    EntityIteratorImp  m_entity_iterator;
};

//============================================================

class QuestServerImp : public QuestServer{
public:

    void Init(sheme_type sheme, int episode)
    {
        if(sheme == ST_START_NEW_GAME){
            QuestEngine::Close();
            QuestEngine::Init();
            QuestEngine::ChangeEpisode(episode, &m_server1);
        }

        if(sheme == ST_CHANGE_EPISODE){
            QuestEngine::ChangeEpisode(episode, &m_server1);
        }
    }

    void Shut()
    {
		STACK_GUARD("QuestServer::Shut");
        QuestEngine::Close();
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(!slot.IsSaving()) QuestEngine::Init();
        QuestEngine::MakeSaveLoad(slot);
    }

    void HandleUseEntity(BaseEntity* user, BaseEntity* used)
	{
        GameObserver::use_info info(user, used);
        GameEvMessenger::GetInst()->Notify(GameObserver::EV_USE, &info);

        EntityManipulatorImp user_manip(user),
                             used_manip(used);

        QuestEngine::HandleMessage(MakeUseMsg(&user_manip, &used_manip));
	}

    void HandleKillEntity(BaseEntity* killer, BaseEntity* victim)
	{
        EntityManipulatorImp killer_manip(killer),
                             victim_manip(victim);

        QuestEngine::HandleMessage(MakeKillMsg(&killer_manip, &victim_manip));
	}

    void HandleFirstPhrase(BaseEntity* actor, BaseEntity* entity)    
    {  
        GameObserver::use_info info(actor, entity);
        GameEvMessenger::GetInst()->Notify(GameObserver::EV_USE, &info);

        EntityManipulatorImp actor_manip(actor),
                             entity_manip(entity);

        QuestEngine::HandleMessage(MakeFirstPhraseMsg(&actor_manip, &entity_manip));

        //первая фраза сказана
        PhraseManager::GetFirstPhrases()->MarkAsSaid(entity);
    }

    void TestSmth()
    {   
        LogicServer1::quest_info info("1_11_quest");
        m_server1.Notify(LogicServer1::QET_QUEST_FINISHED, &info);
    }

private:

    typedef LogicServer1::EntityManipulator ent_manip_t;

    QuestMessage1 MakeUseMsg(ent_manip_t* actor, ent_manip_t* used)
    {
        QuestMessage1 qm;

        qm.serv = &m_server1;        
        qm.Type = QuestMessage1::MT_USE;

        qm.Actor = actor;
        qm.Subject = used;

        return qm;
    }
    
    QuestMessage1 MakeKillMsg(ent_manip_t* killer, ent_manip_t* victim)
    {
        QuestMessage1 qm;
        
        qm.serv = &m_server1;
        qm.Type = QuestMessage1::MT_KILL;

        qm.Actor = killer;
        qm.Subject = victim;
       
        return qm;
    }
    
    QuestMessage1 MakeFirstPhraseMsg(ent_manip_t* actor, ent_manip_t* entity)
    {
        QuestMessage1 qm;
        
        qm.serv = &m_server1;
        qm.Type = QuestMessage1::MT_FIRSTUSE;

        qm.Actor   = actor;
        qm.Subject = entity;

        return qm;
    }

private:

    LogicServer1Imp m_server1;
};

//============================================================

} //namespace

//============================================================

QuestServer* QuestServer::GetInst()
{
    static QuestServerImp instance;
    return &instance;
}

//============================================================
