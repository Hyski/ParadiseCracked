#pragma warning(disable:4786)

#include "logicdefs.h"

#include "form.h"
#include "thing.h"
#include "entity.h"
#include "aiutils.h"
#include "sndutils.h"
#include "dirtylinks.h"
#include "thingfactory.h"
#include "DragDropManager.h"

//=============================================================

SlotStrategy::~SlotStrategy()
{
}

//=============================================================

SlotDecorator::SlotDecorator(SlotStrategy* strategy) :
    m_strategy(strategy)    
{
}

SlotDecorator::~SlotDecorator()
{
    delete m_strategy;
}

SlotStrategy::slot_type SlotDecorator::GetType() const
{
    return m_strategy->GetType();
}

void SlotDecorator::Clear()
{
    m_strategy->Clear();
}

void SlotDecorator::Illuminate(slot_type slot, BaseThing* thing)
{
    m_strategy->Illuminate(slot, thing);
}

void SlotDecorator::Fill(SESlot* slot, SlotEnv* env)
{
    m_strategy->Fill(slot, env);
}

bool SlotDecorator::DoDrag(SEItem* item)
{
    return m_strategy->DoDrag(item);
}

bool SlotDecorator::DoDrop(SEItem* item, const ipnt2_t& to)
{
    return m_strategy->DoDrop(item, to);
}

//=============================================================

UnloadingDecorator::UnloadingDecorator(SlotStrategy* strategy) :
    SlotDecorator(strategy)
{
}

void UnloadingDecorator::Fill(SESlot* slot, SlotEnv* env)
{
    SlotDecorator::Fill(slot, m_env = env);
}

void UnloadingDecorator::Unload()
{
    HumanContext::iterator itor = m_env->GetActor()->GetEntityContext()->begin(HPK_HANDS, TT_WEAPON);

    WeaponThing* weapon =       itor != m_env->GetActor()->GetEntityContext()->end()
                            ?   static_cast<WeaponThing*>(&*itor)
                            :   0;
    
    if(m_env->GetEngine()->MigrationMgr()->GetItem() || weapon == 0)
        return;

    HumanContext::reload_notifier notifier(m_env->GetActor(), weapon);
   
    if(weapon->GetAmmo()){
        //созадим новый предмет
        SEItem* item = m_env->CreateItem(weapon->Load(0));
        //повесим предмет на мышь
        m_env->GetEngine()->MigrationMgr()->Insert(item);
    }
}

//=============================================================

namespace{

    //
    // Декоратор для прогрывания звука при drag & drop'е в слоте
    //
    class UsualSoundDecorator : public SlotDecorator{
    public:
        
        UsualSoundDecorator(SlotStrategy* strategy, SndUtils::sound_type snd4drag, SndUtils::sound_type snd4drop) :
          SlotDecorator(strategy), m_snd4drag(snd4drag), m_snd4drop(snd4drop) {}
        
        bool DoDrag(SEItem* item)
        {
            bool ret = SlotDecorator::DoDrag(item);
            if(ret) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(m_snd4drag));
            return ret;
        }

        bool DoDrop(SEItem* item, const ipnt2_t& to)
        {
            bool ret = SlotDecorator::DoDrop(item, to);
            if(ret) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(m_snd4drop));
            return ret;
        }
        
    private:
        
        SndUtils::sound_type m_snd4drag;
        SndUtils::sound_type m_snd4drop;
    };

    //
    // Декоратор для прогрывания звука при drag & drop'е на земле
    //
    class GroundSoundDecorator : public SlotDecorator{
    public:
        
        GroundSoundDecorator(SlotStrategy* strategy) : SlotDecorator(strategy), m_ground(0) {}

        void Fill(SESlot* slot, SlotEnv* env)
        {
            m_ground = env->GetGround();
            SlotDecorator::Fill(slot, env);            
        }

        void Clear() { m_ground = 0; }
        
        bool DoDrag(SEItem* item)
        {
            bool ret = SlotDecorator::DoDrag(item);
            if(ret && m_ground) m_ground->PlaySound(AbstractGround::SE_DRAG);
            return ret;
        }

        bool DoDrop(SEItem* item, const ipnt2_t& to)
        {
            bool ret = SlotDecorator::DoDrop(item, to);
            if(ret && m_ground) m_ground->PlaySound(AbstractGround::SE_DROP);
            return ret;
        }
        
    private:

        AbstractGround* m_ground;
    };
}
//=============================================================

namespace{
    
    //
    // общая стратегия для слотов
    //
    class CommonSlotStrategy : public SlotStrategy{
    public:

        CommonSlotStrategy(slot_type type) : m_slot_type(type) {}

        void Fill(SESlot* slot, SlotEnv* env)
        {
            m_env = env;
            m_slot = slot;
        }

        //закрытие слота (необх. сохранить позицию предметов)
        void Clear()
        {
            SESlot::Iterator itor = GetSlot()->Begin();

            while(itor != GetSlot()->End()){
                itor->GetBaseThing()->SetPakPos(itor->Position());
                ++itor;
            }

            GetSlot()->Clear();
        }
        
        //подсветить предметы в слоте (eсли thing == 0 => нет предмета для подсветки)
        void Illuminate(slot_type slot, BaseThing* thing)
        {             
            //подсветим каждый предмет в слоте
            SESlot::Iterator itor = GetSlot()->Begin();

            while(itor != GetSlot()->End()){                
                m_env->SetState(&*itor, thing);
                ++itor;
            }
        }

        bool DoDrag(SEItem* item)
        {
            if(item->GetBaseThing()->GetInfo()->IsMoney()){
                RemoveMoney(item);
                return true;
            }

            return false;
        }

        slot_type GetType() const { return m_slot_type; }

    protected:

        SEItem* GetFirstItem()
        {
            return GetSlot()->Begin() != GetSlot()->End() ? &*GetSlot()->Begin() : 0;
        }

        void Refill()
        {
            typedef std::list<SEItem*> item_lst_t;
            item_lst_t items;
            
            SESlot::Iterator slot_itor = GetSlot()->Begin();
            while(slot_itor != GetSlot()->End()){                
                items.push_back(&*(slot_itor++));
                GetSlot()->Remove(items.back());
            }
            
            item_lst_t::iterator item_itor = items.begin();
            while(item_itor != items.end()){
                GetSlot()->CanInsert(*item_itor);
                Insert(*item_itor);
                ++item_itor;
            }
        }

        void Insert(SEItem* item)
        {
            m_slot->Insert(item);
            
            SEItem* mouse = GetEnv()->GetEngine()->MigrationMgr()->GetItem();
            m_env->SetState(item, mouse ? mouse->GetBaseThing() : 0);
        }

        void Swap(SEItem* item_bottom, SEItem* item_top)
        {
            m_slot->Swap(item_bottom, item_top);

            SEItem* mouse = GetEnv()->GetEngine()->MigrationMgr()->GetItem();
            m_env->SetState(item_top, mouse ? mouse->GetBaseThing() : 0);
        }

        virtual void RemoveMoney(SEItem* item)
        {
            MoneyThing* money_thing = static_cast<MoneyThing*>(item->GetBaseThing());

            //увеличить наш счет
            MoneyMgr::GetInst()->SetMoney(MoneyMgr::GetInst()->GetMoney() + money_thing->GetSum());                
            
            //проиграем звук зачисления денег на счет
            SndPlayer::GetInst()->Play(money_thing->GetInfo()->GetUseSound());

            //удалить деньги из слота
            GetSlot()->Remove(item);
            //удалиить сам Item
            delete item;

            //уведомить кого следует
            GetEnv()->GetActor()->Notify(EntityObserver::EV_TAKE_MONEY);
        }

        //получить ссылку на
        SESlot* GetSlot() { return m_slot; }
        //получить ссылку на окружение
        SlotEnv* GetEnv() {return m_env; }

    private: 

        SESlot*   m_slot;
        SlotEnv*  m_env;
        slot_type m_slot_type;        
    };
}

//=============================================================

namespace{

    //
    // слот для земли
    //
    class GroundSlotStrategy : public CommonSlotStrategy{
    public:

        GroundSlotStrategy() : CommonSlotStrategy(ST_GROUND){}
        
        //открытие слота (необх. заполнить слот)
        void Fill(SESlot* slot, SlotEnv* env)
        {
            CommonSlotStrategy::Fill(slot, env);

            GetSlot()->Clear();
            GetSlot()->UnScroll();

            std::auto_ptr<AbstractGround::Iterator> itor(GetEnv()->GetGround()->CreateIterator(GetEnv()->GetActor()));

            while(itor->IsNotDone()){
                
                SEItem* item = GetEnv()->CreateItem(itor->Get());

                if(!GetSlot()->CanInsert(item))
                    throw CASUS("GroundSlotStrategy::Fill: не хватает места для передмета!");

                Insert(item);

                itor->Next();
            }
        }

        //подсветить предметы в слоте (eсли thing == 0 => нет предмета для подсветки)
        void Illuminate(slot_type slot, BaseThing* thing)
        {
            //подсветим сам слот
            GetSlot()->SetState(   (thing && !thing->GetInfo()->IsMoney()) 
                                ?  SESlot::ST_SELECTED : SESlot::ST_NORMAL);           

            //подсветим предметы в слоте
            CommonSlotStrategy::Illuminate(slot, thing);
        }
        
        //обработка стаскивания предмета
        bool DoDrag(SEItem* item)
        {
            if(!CommonSlotStrategy::DoDrag(item)){
                //вытащить предмет с земли
                GetEnv()->GetGround()->Remove(GetEnv()->GetActor(), item->GetBaseThing());
                //вытащить предмет из слота
                GetSlot()->Remove(item);
                //повесить на мышь
                GetEnv()->GetEngine()->MigrationMgr()->Insert(item);
            }

            //заполнить слот заново
            Refill();

            return true;   
        }

        //обработка бросания предмета
        bool DoDrop(SEItem* item, const ipnt2_t& to)
        {
            SEItem* item4drop = GetEnv()->GetEngine()->MigrationMgr()->GetItem();

            //снять предмет с мыши
            GetEnv()->GetEngine()->MigrationMgr()->Remove();            
            //положить на землю
            GetEnv()->GetGround()->Insert(GetEnv()->GetActor(), item4drop->GetBaseThing());
            
            //положить в слот
            if(!GetSlot()->CanInsert(item4drop))
                throw CASUS("GroundSlotStrategy::DoDrop нет места в слоте!");            
            
            Insert(item4drop);

            return true;   
        }

    private:

        void RemoveMoney(SEItem* item)
        {
            //удалить деньги с земли
            GetEnv()->GetGround()->Remove(GetEnv()->GetActor(), item->GetBaseThing());
            //положить деньги на счет
            CommonSlotStrategy::RemoveMoney(item);
        }
    };  
}

//=============================================================
#include "graphic.h"
namespace{

    //
    // базовый слот человека
    //
    class HumanSlotStrategy : public CommonSlotStrategy{
    public:

        HumanSlotStrategy(human_pack_type pack, slot_type slot) :
          CommonSlotStrategy(slot), m_pack(pack) {}

        //открытие слота (необх. заполнить слот)
        void Fill(SESlot* slot, SlotEnv* env)
        {
            CommonSlotStrategy::Fill(slot, env);

            HumanContext* human = GetEnv()->GetActor()->GetEntityContext(); 
            HumanContext::iterator itor = human->begin(m_pack);

            GetSlot()->Clear();

			for(int cnt=0; cnt<4; cnt++) //цикл по разным типам снаряжения
			{
            HumanContext::iterator itor = human->begin(m_pack);
            while(itor != human->end()){

				if(cnt==0 && !itor->GetInfo()->IsWeapon()) {itor++;continue;}
				if(cnt==1 && !itor->GetInfo()->IsGrenade()) {itor++;continue;}
				if(cnt==2 && !itor->GetInfo()->IsMedikit()) {itor++;continue;}
				if(cnt==3 && (itor->GetInfo()->IsWeapon()
								||itor->GetInfo()->IsGrenade()
								||itor->GetInfo()->IsMedikit()))
															{itor++;continue;}

                SEItem* item = GetEnv()->CreateItem(&*itor);
                
                if(    !GetEnv()->GetActor()->IsRaised(EA_WAS_IN_INV)
                    && !GetSlot()->CanInsert(item))
				{
					//Grom
					BaseThing* thing = &*itor;
					itor++;
					human->RemoveThing(thing);
					Depot::GetInst()->Push(GetEnv()->GetActor()->GetGraph()->GetPos2(), thing);
					delete item;
					continue;
                    //throw CASUS("HumanSlotStrategy::Fill: не хватает места для передмета!");                    
				}
				else
				{
					Insert(item);
				}
				
                ++itor;
            }
			}
        }

        //подсветить предметы в слоте (eсли item == 0 => нет предмета для подсветки)
        void Illuminate(slot_type slot, BaseThing* thing)
        {
            ThingDelivery::scheme_type scheme = slot == ST_GROUND ? ThingDelivery::ST_GROUND : ThingDelivery::ST_USUAL;

            bool fcan_deliver = thing && CanDeliver(scheme, thing);
            bool fcan_place   = thing && GetSlot()->CanPlace(thing->GetInfo()->GetSize());
            
            //подсветим сам слот
            GetSlot()->SetState((fcan_place && fcan_deliver) ? SESlot::ST_SELECTED : SESlot::ST_NORMAL);
            //подсветим предметы в слоте
            CommonSlotStrategy::Illuminate(slot, thing);
        }
        
        //обработка стаскивания предмета
        bool DoDrag(SEItem* item)
        {
            if(!CommonSlotStrategy::DoDrag(item)){
                //вытащим предмет у человека
                GetEnv()->GetActor()->GetEntityContext()->RemoveThing(item->GetBaseThing());
                //вытащим предмет из слота
                GetSlot()->Remove(item);
                //повесим предмет на мышь
                GetEnv()->GetEngine()->MigrationMgr()->Insert(item);
            }

            return true;
        }

        //обработка бросания предмета
        bool DoDrop(SEItem* item, const ipnt2_t& to)
        {
            SEItem* item4drop = GetEnv()->GetEngine()->MigrationMgr()->GetItem();
            ThingDelivery::scheme_type scheme = GetDeliveryScheme(item4drop);

            //сделаем drop
            if(     CanDeliver(scheme, item4drop->GetBaseThing(), DCT_PRINT)
                &&  ((item && DoRawSwap(item)) || (!item && DoRawDrop(to))))
			{
				bool i=try_fill_stack(item, item4drop);
				if(i)
				{
					//очистить мышь
					GetEnv()->GetEngine()->MigrationMgr()->Remove();
                    GetEnv()->GetActor()->GetEntityContext()->RemoveThing(item->GetBaseThing());
					//махнуть местами 
					//Swap(item, item4drop);
					
					/*GetEnv()->GetEngine()->MigrationMgr()->Insert(item);
					GetEnv()->GetEngine()->MigrationMgr()->Remove();
					*/
            //вставим ноаый предмет в слот человека
            GetEnv()->GetActor()->GetEntityContext()->InsertThing(m_pack, item4drop->GetBaseThing());
            //активируем предмет в слоте
            ThingDelivery::GetInst()->Activate(scheme, GetEnv()->GetActor(), item4drop->GetBaseThing(), m_pack);

					
					delete item;
					return true;
				}

                
                ActiveNewAndRemoveOld(scheme, item4drop, item);

                return true;
            }

            return false;
        }

    protected:

        enum deliver_check_type{
            DCT_PRINT,
            DCT_SILENT,
        };

        virtual bool CanDeliver(ThingDelivery::scheme_type scheme, BaseThing* thing, deliver_check_type check = DCT_SILENT)
        {
            ThingDelivery::reason_type reason;

            //можем ли бросить предмет с точки зрения логики?
            if(thing == 0 || !ThingDelivery::GetInst()->CanInsert(scheme, GetEnv()->GetActor(), thing, m_pack, &reason)){
                if(check == DCT_PRINT)PrintDeliveryFailtureReason(reason);
                return false;
            }
            
            return true;
        }

        void PrintDeliveryFailtureReason(ThingDelivery::reason_type reason)
        {
            SndUtils::sound_type sound = SndUtils::SND_UNSUITABLE_PARAMETER;
            
            if(     reason == ThingDelivery::RT_SPACESUIT_WITH_NO_CANNON
                ||  reason == ThingDelivery::RT_CANNON_WITHOUT_SPACESUIT)
                sound = SndUtils::SND_INCOMPATIBLE_SUIT_AND_WEAPON;
            
            SndPlayer::GetInst()->Play(SndUtils::Snd2Str(sound));
            
            DirtyLinks::Print(ThingDelivery::GetReason(reason), DirtyLinks::MT_DENIAL);
        }

        void ActiveNewAndRemoveOld(ThingDelivery::scheme_type scheme, SEItem* new_item, SEItem* old_item)
        {
            //снять старый предмет
            if(old_item) GetEnv()->GetActor()->GetEntityContext()->RemoveThing(old_item->GetBaseThing());                
            //вставим ноаый предмет в слот человека
            GetEnv()->GetActor()->GetEntityContext()->InsertThing(m_pack, new_item->GetBaseThing());
            //активируем предмет в слоте
            ThingDelivery::GetInst()->Activate(scheme, GetEnv()->GetActor(), new_item->GetBaseThing(), m_pack);
        }

        ThingDelivery::scheme_type GetDeliveryScheme(SEItem* item)
        {
            return (item && GetEnv()->GetLastOwner(item) == ST_GROUND) ? ThingDelivery::ST_GROUND : ThingDelivery::ST_USUAL;
        }

    private:

        //поменять предметы местами в меню
        bool DoRawSwap(SEItem* item_on)
        {
            SEItem* item4drop = GetEnv()->GetEngine()->MigrationMgr()->GetItem();

            if(GetSlot()->CanSwap(item_on, item4drop)){ 
                //очистить мышь
                GetEnv()->GetEngine()->MigrationMgr()->Remove();
                //махнуть местами 
                Swap(item_on, item4drop);
                //повесть на мышь другой                
				GetEnv()->GetEngine()->MigrationMgr()->Insert(item_on);

                return true;
            }

            return false;
        }
		bool try_fill_stack(SEItem* item_on, SEItem* item4drop)
		{
			if(!item_on || !item4drop) return false;
			BaseThing *on=item_on->GetBaseThing();
			BaseThing *off=item4drop->GetBaseThing();
			if(!on->GetInfo()->IsAmmo()) return false;
			if(!off->GetInfo()->IsAmmo()) return false;
			if(on->GetInfo()->GetRID()!=off->GetInfo()->GetRID()) return false;

			AmmoThing *am1=static_cast<AmmoThing*>(on);
			AmmoThing *am2=static_cast<AmmoThing*>(off);
			int stack_size = am1->GetInfo()->GetQuality();
			if (am2->GetCount() >= stack_size) return false;

			int delta = stack_size-am2->GetCount();
			if(delta >= am1->GetCount())
			{
				//полностью входит
				am2->SetCount(am2->GetCount()+am1->GetCount());
				GetEnv()->UpdateText(item4drop);
			//GetEnv()->GetActor()->GetEntityContext()->RemoveThing(on);
			}
			else
			{
				am2->SetCount(stack_size);
				am1->SetCount(am1->GetCount()-delta);
				GetEnv()->UpdateText(item4drop);
				GetEnv()->UpdateText(item_on);
				return false;
			}
			return true;

		}

        //бросить предмет в слот меню
        bool DoRawDrop(const ipnt2_t& to)
        {
            SEItem* item4drop = GetEnv()->GetEngine()->MigrationMgr()->GetItem();

            if(GetSlot()->CanInsert(item4drop, &to)){
                //очистить мышь
                GetEnv()->GetEngine()->MigrationMgr()->Remove();
                //вставить предмет
                Insert(item4drop);
                return true;
            }

            return false;
        }

        void RemoveMoney(SEItem* item)
        {
            //удалить деньги с земли
            GetEnv()->GetActor()->GetEntityContext()->RemoveThing(item->GetBaseThing());
            //положить деньги на счет
            CommonSlotStrategy::RemoveMoney(item);
        }

    private:

        human_pack_type m_pack;
    };
}

//=============================================================

namespace{

    //
    // поле имплантов
    //
    class ImplantSlotStrategy : public HumanSlotStrategy{
    public:

        ImplantSlotStrategy() : HumanSlotStrategy(HPK_IMPLANTS, ST_IMPLANTS) {}

        //обработка стаскивания предмета
        bool DoDrag(SEItem* item)
        {
            return CommonSlotStrategy::DoDrag(item);
        }

        //обработка бросания предмета
        bool DoDrop(SEItem* item, const ipnt2_t& to)
        {
            SEItem* item4drop = GetEnv()->GetEngine()->MigrationMgr()->GetItem();
            ThingDelivery::scheme_type scheme = GetDeliveryScheme(item4drop);

            if(     GetSlot()->CanInsert(item4drop)
                &&  CanDeliver(scheme, item4drop->GetBaseThing(), DCT_PRINT)){

                //очистить мышь
                GetEnv()->GetEngine()->MigrationMgr()->Remove();
                //положить в интерфейс
                Insert(item4drop);

                //активировать предмет
                ActiveNewAndRemoveOld(scheme, item4drop, 0);

                return true;
            }

            return false;
        }
    };
}

//=============================================================

namespace{

    //
    // слот для рук челоека
    //
    class HandsSlotStrategy : public HumanSlotStrategy, private EntityObserver{
    public:

        HandsSlotStrategy() : HumanSlotStrategy(HPK_HANDS, ST_HANDS) {}

        //открытие слота (необх. заполнить слот)
        void Fill(SESlot* slot, SlotEnv* env)
        {
            HumanSlotStrategy::Fill(slot, env);
            GetEnv()->GetActor()->Attach(this, EV_BODY_PACK_CHANGE);
            GetEnv()->GetActor()->Attach(this, EV_WEAPON_STATE_CHANGE);
        }

        //закрытие слота (необх. сохранить позицию предметов)
        void Clear()
        {
            HumanSlotStrategy::Clear();
            GetEnv()->GetActor()->Detach(this);
        }

        //обработка бросания предмета
        bool DoDrop(SEItem* item, const ipnt2_t& to)
        {
            SEItem*  slot_item  = GetFirstItem(),
                  *  mouse_item = GetEnv()->GetEngine()->MigrationMgr()->GetItem(); 

            WeaponThing* weapon =    (slot_item && slot_item->GetBaseThing()->GetInfo()->IsWeapon())
                                 ?   static_cast<WeaponThing*>(slot_item->GetBaseThing()) : 0;

            AmmoThing* ammo =   (mouse_item->GetBaseThing()->GetInfo()->IsAmmo()) 
                             ?  static_cast<AmmoThing*>(mouse_item->GetBaseThing()) : 0;

            if(weapon && ammo && ThingDelivery::GetInst()->IsSuitable(weapon, ammo))
			{
				//Grom
				HumanContext *con= GetEnv()->GetActor()->GetEntityContext();

				int mps=con->GetTraits()->GetMovepnts();
				int mps4reload = weapon->GetInfo()->GetMps2Reload();
				if(mps<mps4reload) 
				{
					ThingDelivery::reason_type reason=ThingDelivery::RT_NOT_ENOUGH_MOVEPNTS;
                    DirtyLinks::Print(ThingDelivery::GetReason(reason), DirtyLinks::MT_DENIAL);
					return false;//ситуация такая не должнв встречаться
				}

				con->GetTraits()->AddMovepnts(-mps4reload);
				  
                DoLoad();
                return false;
            }

            return HumanSlotStrategy::DoDrop(GetFirstItem(), to);
        }
        
    private:

        void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
        {
            if(event == EV_BODY_PACK_CHANGE) CheckArmor();
            if(event == EV_WEAPON_STATE_CHANGE) RefreshItemText();
        }

        //совместим ли предмет с броником?
        void CheckArmor()
        {
            SEItem* item = GetFirstItem();

            if(item && !CanDeliver(ThingDelivery::ST_USUAL, item->GetBaseThing())){
                GetSlot()->Remove(item);
                GetEnv()->GetActor()->GetEntityContext()->RemoveThing(item->GetBaseThing());
                GetEnv()->DiscardItem(item);
            }
        }

        //перерисовать текст на предмете
        void RefreshItemText()
        {
            if(SEItem* item = GetFirstItem()) GetEnv()->UpdateText(item);
        }     

        //зарядить пушку
        bool DoLoad()
        {
            SEItem* weapon_item = GetFirstItem(),
                  * ammo_item   = GetEnv()->GetEngine()->MigrationMgr()->GetItem(); 

            AmmoThing*   ammo   = static_cast<AmmoThing*>(ammo_item->GetBaseThing());
            WeaponThing* weapon = static_cast<WeaponThing*>(weapon_item->GetBaseThing());

            HumanContext::reload_notifier notifier(GetEnv()->GetActor(), weapon);

            //1. В оружии нет патронов
            if(weapon->GetAmmo() == 0){

                GetEnv()->GetEngine()->MigrationMgr()->Remove();
                
                //снимем излишек патронов
                if(ammo->GetCount() > weapon->GetInfo()->GetQuality()){
                    SEItem* new_ammo = GetEnv()->CreateItem(ThingFactory::GetInst()->CreateAmmo(ammo->GetInfo()->GetRID(), ammo->GetCount() - weapon->GetInfo()->GetQuality()));
                    GetEnv()->GetEngine()->MigrationMgr()->Insert(new_ammo);
                    ammo->SetCount(weapon->GetInfo()->GetQuality());
                }

                delete ammo_item;
                weapon->Load(ammo);
                return true;
            }
            
            //2. В оружии и ammo одинаковые патроны
            if(weapon->GetAmmo()->GetInfo()->GetRID() == ammo->GetInfo()->GetRID()){
                
                //повесим излишек на мышь
                if(weapon->GetAmmo()->GetCount() + ammo->GetCount() > weapon->GetInfo()->GetQuality()){
                    
                    ammo->SetCount(weapon->GetAmmo()->GetCount() + ammo->GetCount() - weapon->GetInfo()->GetQuality());
                    weapon->GetAmmo()->SetCount(weapon->GetInfo()->GetQuality());
                    
                    GetEnv()->UpdateText(ammo_item);
                    return true;
                }
                                
                //добавим патроны, уничтожим item
                weapon->GetAmmo()->SetCount(weapon->GetAmmo()->GetCount() + ammo->GetCount());

                GetEnv()->GetEngine()->MigrationMgr()->Remove();

                delete ammo;
                delete ammo_item;
                return true;
            }
            
            //3. Разные патроны на мышке и в оружии, патронов достаточно.
            if(weapon->GetInfo()->GetQuality() >= ammo->GetCount()){

                SEItem* new_ammo = GetEnv()->CreateItem(weapon->Load(ammo));

                GetEnv()->GetEngine()->MigrationMgr()->Remove();
                GetEnv()->GetEngine()->MigrationMgr()->Insert(new_ammo);

                delete ammo_item;
                return true;
            }
            
            return false;
        }
    };
};

//=============================================================

namespace{

    //
    // слот тела человека
    //
    class BodySlotStrategy : public HumanSlotStrategy{
    public:

        BodySlotStrategy() : HumanSlotStrategy(HPK_BODY, ST_IMPLANTS) {}
       
        bool DoDrop(SEItem* item, const ipnt2_t& to)
        {
            return HumanSlotStrategy::DoDrop(GetFirstItem(), to);
        }

    private:

        virtual bool CanDeliver(ThingDelivery::scheme_type scheme, BaseThing* thing, deliver_check_type check = DCT_SILENT)
        {
            ThingDelivery::reason_type reason;
            
            //можем ли бросить предмет с точки зрения логики?
            if(     thing == 0
                ||  (       !ThingDelivery::GetInst()->CanInsert(scheme, GetEnv()->GetActor(), thing, HPK_BODY, &reason)
                      &&    (       reason != ThingDelivery::RT_SPACESUIT_WITH_NO_CANNON
                                &&  reason != ThingDelivery::RT_CANNON_WITHOUT_SPACESUIT))){

                if(check == DCT_PRINT)PrintDeliveryFailtureReason(reason);
                return false;
            }
            
            return true;
        }
    };
}

//=============================================================

namespace{

    //
    // слот для торговли
    //
    class MarketSlotStrategy : public CommonSlotStrategy{
    public:

        MarketSlotStrategy() : CommonSlotStrategy(ST_MARKET) {}

        void Illuminate(slot_type slot, BaseThing* thing)
        {
            GetSlot()->SetState(thing ? SESlot::ST_SELECTED : SESlot::ST_NORMAL);

            CommonSlotStrategy::Illuminate(slot, thing);
        }
                
        bool DoDrag(SEItem* item)
        {
            if(!CommonSlotStrategy::DoDrag(item)){

                //вытащить предмет из слота
                GetSlot()->Remove(item);
                //прицепить к мышке
                GetEnv()->GetEngine()->MigrationMgr()->Insert(item);

                Refill();

                return true;
            }

            return false;
        }
                
        bool DoDrop(SEItem* item, const ipnt2_t& to)
        {
            SEItem* item4drop = GetEnv()->GetEngine()->MigrationMgr()->GetItem();

            //бросить предмет в слот
            if(GetSlot()->CanInsert(item4drop)){

                //отсоед от мышки
                GetEnv()->GetEngine()->MigrationMgr()->Remove();
                //бросаем в слот
                Insert(item4drop);

                return true;
            }

            return false;
        }
    };
}

//=============================================================

DragDropManager::DragDropManager(SlotsEngine* engine) :
    m_actor(0), m_engine(engine), m_bin(0), m_ground(0)    
{
    m_engine->SetController(this);
}

DragDropManager::~DragDropManager()
{
    //m_engine->SetController(0);
    for(size_t k = 0; k < m_slots.size(); delete m_slots[k++]);
}

void DragDropManager::Insert(slid_t slot, SlotStrategy* strategy)
{
    if(m_slots.size() <= slot) m_slots.resize(slot + 1);
    m_slots[slot] = strategy;
}

void DragDropManager::SetActor(HumanEntity* human)
{
    //уведомим слоты об уходе старого персонажа
    if(m_actor){

        m_actor->Detach(this);
        m_actor->RaiseFlags(EA_WAS_IN_INV);

        for(size_t k = 0; k < m_slots.size(); k++){            
            if(m_slots[k]){
                m_slots[k]->Clear();            
                m_slots[k]->Illuminate(SlotStrategy::ST_NONE, 0);
            }
        }
    }

    //уведомим слоты о появлении нового
    if(m_actor = human){

        m_actor->Attach(this, EV_WISDOM_CHANGE);
        m_actor->Attach(this, EV_STRENGTH_CHANGE);

        for(size_t k = 0; k < m_slots.size(); k++){
            if(m_slots[k]) m_slots[k]->Fill(m_engine->SlotAt(k), this);
        }
    }
}

void DragDropManager::OnDrag(SESlot* slot, SEItem* item)
{
    if(m_slots[slot->Id()]) m_slots[slot->Id()]->DoDrag(item);
}

void DragDropManager::OnDrop(SESlot* slot, SEItem* item, const ipnt2_t& pos)
{
    if(m_slots[slot->Id()]) m_slots[slot->Id()]->DoDrop(item, pos);
}

void DragDropManager::OnLighting(SEItem* item,LIGHTING_MODE lm)
{
    SEItem* light_item = 0;

    //зажечь слоты и предметы в слотах
    if(item && lm == LM_START){

        if(m_engine->MigrationMgr()->GetItem() == item){
            
            item->SetState(0);
            light_item = item;

        } else if(GetLastOwner(item) != SlotStrategy::ST_IMPLANTS){

            light_item = item;
        }
    }

    if(light_item)
        Illuminate(GetLastOwner(light_item), light_item->GetBaseThing());
    else
        Illuminate(SlotStrategy::ST_NONE, 0);
}

void DragDropManager::Illuminate(SlotStrategy::slot_type slot, BaseThing* thing)
{
       for(size_t k = 0; k < m_slots.size(); k++){
        if(m_slots[k]) m_slots[k]->Illuminate(slot, thing);
    }
}

void DragDropManager::SetBin(AbstractBin* bin)
{
    m_bin = bin;
}

HumanEntity* DragDropManager::GetActor() const
{
    return m_actor;
}

SlotsEngine* DragDropManager::GetEngine() const
{
    return m_engine;
}

AbstractGround* DragDropManager::GetGround() const
{
    return m_ground;
}

void DragDropManager::UpdateText(SEItem* item)
{
    item->SetText(AIUtils::GetThingNumber(item->GetBaseThing()).c_str());
}

SlotStrategy::slot_type DragDropManager::GetLastOwner(SEItem* item) const
{
    if(item == 0 || item->SlotId() == SEItem::m_SlotIdIsAbsent)
        return SlotStrategy::ST_NONE;

    return m_slots[item->SlotId()]->GetType();
}

SEItem* DragDropManager::CreateItem(BaseThing* thing)
{
    SEItem* item = m_engine->CreateItem(thing->GetInfo()->GetShader().c_str(), thing->GetInfo()->GetSize(), thing);
    
    item->SetState(0);
    item->SetPosition(thing->GetPakPos());
    
    UpdateText(item);

    return item;
}

void DragDropManager::OnItemClick(SEItem* item)
{
    Forms::GetInst()->ShowDescDialog(item->GetBaseThing());
}

void DragDropManager::SetGround(AbstractGround* ground)
{
    m_ground = ground;
}

void DragDropManager::DiscardItem(SEItem* real_item)
{
    std::auto_ptr<SEItem> item(real_item);

    //можем сбросить на землю?
    if(m_ground->CanInsert(m_actor, item->GetBaseThing(), 0)){

        //вставим в землю
        m_ground->Insert(m_actor, item->GetBaseThing());

        //перечитаем слоты
        for(size_t k = 0; k < m_slots.size(); k++){
            if(m_slots[k] && m_slots[k]->GetType() == SlotStrategy::ST_GROUND)
                m_slots[k]->Fill(m_engine->SlotAt(k), this);            
        }

        return;
    }

    //на землю не вышло, значит в урну
    m_bin->Insert(m_actor, item->GetBaseThing());
}

bool DragDropManager::CancelDrop()
{
    if(SEItem* item = m_engine->MigrationMgr()->GetItem()){
        m_engine->MigrationMgr()->Remove();
        DiscardItem(item);
        return true;
    }

    return false;
}

void DragDropManager::SetState(SEItem* item, BaseThing* thing)
{
    unsigned flags = 0;
    
    //человек может воспользоваться предметом?
    if(!ThingDelivery::GetInst()->IsSuitable(m_actor, item->GetBaseThing()))                
        flags |= SEItem::ST_GRAYED;
    
    //человек может использовать предмет совместно с другим?
    if(thing && ThingDelivery::GetInst()->IsSuitable(item->GetBaseThing(), thing))
        flags |= SEItem::ST_HIGHLIGHTED;
    
    item->SetState(flags);
}

void DragDropManager::Update(BaseEntity* entity, event_t event, info_t info)
{
    if(SEItem* light_item = m_engine->MigrationMgr()->GetItem())
        Illuminate(GetLastOwner(light_item), light_item->GetBaseThing());
    else
        Illuminate(SlotStrategy::ST_NONE, 0);
}

//=============================================================

SlotFactory* SlotFactory::GetInst()
{
    static SlotFactory factory;
    return &factory;
}

SlotStrategy* SlotFactory::Create(SlotStrategy::slot_type type)
{
   switch(type){
    case SlotStrategy::ST_BODY:
        return new UsualSoundDecorator( new BodySlotStrategy(),
                                        SndUtils::SND_DRAG_FROM_BODY,
                                        SndUtils::SND_DROP_TO_BODY);

    case SlotStrategy::ST_GROUND:
        return new GroundSoundDecorator(new GroundSlotStrategy());

    case SlotStrategy::ST_IMPLANTS:
        return new UsualSoundDecorator( new ImplantSlotStrategy(),
                                        SndUtils::SND_DRAG_FROM_IMPLANTS,
                                        SndUtils::SND_DROP_TO_IMPLANTS);

    case SlotStrategy::ST_HEAD:
        return new UsualSoundDecorator( new HumanSlotStrategy(HPK_HEAD, SlotStrategy::ST_HEAD),
                                        SndUtils::SND_DRAG_FROM_HEAD,
                                        SndUtils::SND_DROP_TO_HEAD);

    case SlotStrategy::ST_HANDS:
        return new UsualSoundDecorator( new HandsSlotStrategy(),
                                        SndUtils::SND_DRAG_FROM_HANDS,
                                        SndUtils::SND_DROP_TO_HANDS);

    case SlotStrategy::ST_LKNEE:
        return new UsualSoundDecorator( new HumanSlotStrategy(HPK_LKNEE, SlotStrategy::ST_LKNEE),
                                        SndUtils::SND_DRAG_FROM_KNEES,
                                        SndUtils::SND_DROP_TO_KNEES);

    case SlotStrategy::ST_RKNEE:
        return new UsualSoundDecorator( new HumanSlotStrategy(HPK_RKNEE, SlotStrategy::ST_RKNEE),
                                        SndUtils::SND_DRAG_FROM_KNEES,
                                        SndUtils::SND_DROP_TO_KNEES);

    case SlotStrategy::ST_BACKPACK:
        return new UsualSoundDecorator( new HumanSlotStrategy(HPK_BACKPACK, SlotStrategy::ST_BACKPACK),
                                        SndUtils::SND_DRAG_FROM_BACKPACK,
                                        SndUtils::SND_DROP_TO_BACKPACK);

    case SlotStrategy::ST_MARKET:
        return new UsualSoundDecorator( new MarketSlotStrategy(),
                                        SndUtils::SND_DRAG_FROM_MARKET,
                                        SndUtils::SND_DROP_TO_MARKET);
    }

    return 0;
}
