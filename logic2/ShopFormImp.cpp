#pragma warning(disable:4786)

#include "logicdefs.h"

#include "../interface/screens.h"

#include "thing.h"
#include "entity.h"
#include "aiutils.h"
#include "sndutils.h"
#include "dirtylinks.h"
#include "shopformimp.h"
#include "dragdropmanager.h"

//===============================================================


BaseForm* FormFactory::CreateShopForm()
{
    return new ShopFormImp();
}

//===============================================================

namespace{

    //получить ссылку на реализацию
    ShopFormImp* GetShopFormImp()
    {
        return static_cast<ShopFormImp*>(Forms::GetInst()->GetShopForm());
    }
    
    //составить описание для торговца
    std::string GetTraderDesc(TraderEntity* trader)
    {
        std::ostringstream ostr;
        
        ostr << trader->GetInfo()->GetName() << ':' << std::endl;
        ostr << trader->GetInfo()->GetDesc() << std::endl;
        
        return ostr.str();
    }

    //перевод челочисленного значения в строку
    std::string Int2Str(int value)
    {
        std::ostringstream ostr;
        ostr << value;
        return ostr.str();
    }
}

//===============================================================

namespace{

    //
    // реализация вкладки товара
    //
    class UsualSheet : public CommoditySheet{
    public:

        UsualSheet(TradeScreen::SHEET sheet, unsigned mask) : 
            m_mask(mask), m_sheet(sheet) {}

        void Show(HumanEntity* hero, BaseThing* active)
        {
            TradeScreen::CommodityTraits traits;
            TraderEntity* trader = GetShopFormImp()->GetTrader();

            std::string tmp;

            if(m_commodity){

                traits.m_Cost = trader->GetInfo()->GetSellPrice(m_commodity);
                traits.m_Name = m_commodity->GetInfo()->GetName().c_str();
                traits.m_Quantity = trader->GetEntityContext()->GetCount(m_commodity);
                traits.m_SystemName = m_commodity->GetInfo()->GetShader().c_str();

                tmp = AIUtils::GetThingNumber(m_commodity);
                traits.m_Text = tmp.c_str();

                traits.m_State = 0;

                //человек может воспользоваться предметом?
                if(!ThingDelivery::GetInst()->IsSuitable(hero, m_commodity))                
                    traits.m_State |= TradeScreen::CS_GRAYED;
                
                //человек может использовать предмет совместно с другим?
                if(active && ThingDelivery::GetInst()->IsSuitable(m_commodity, active))
                    traits.m_State |= TradeScreen::CS_HIGHLIGHTED;

            }else{

                traits.m_Cost = 0;
                traits.m_State = 0;
                traits.m_Quantity = 0;
                traits.m_Name = "";
                traits.m_Text = "";
                traits.m_SystemName = "";
            }

            Screens::Instance()->Trade()->SetCommodityTraits(traits);
        }

        void Init()
        {
            m_commodity = 0;
            Next();
        }
        
        void Next()
        {
            TraderContext* context = GetShopFormImp()->GetTrader()->GetEntityContext();

            if(m_commodity){
    
                TraderContext::iterator itor = GetUpperBound(context->begin(m_mask), context->end(), m_commodity);

                if(itor != context->end()){
                    m_commodity = &*itor;
                    return;
                }
            }
            
            TraderContext::iterator itor = context->begin(m_mask);
            m_commodity = itor != context->end() ? &*itor : 0;

            Screens::Instance()->Trade()->EnableSheet(m_sheet, m_commodity != 0);
        }

        void Prev()
        {
            TraderContext* context = GetShopFormImp()->GetTrader()->GetEntityContext();

            if(m_commodity){

                TraderContext::iterator bound = GetLowerBound(context->begin(m_mask), context->end(), m_commodity),
                                        itor  = GetPrevious(context->begin(m_mask), bound);

                if(itor != bound){
                    m_commodity = &*itor;
                    return;
                }
            }

            TraderContext::iterator itor = GetPrevious(context->begin(m_mask), context->end());
            m_commodity = itor != context->end() ? &*itor : 0;

            Screens::Instance()->Trade()->EnableSheet(m_sheet, m_commodity != 0);
        }

        BaseThing* SellThing()
        { 
            BaseThing* thing4sale = m_commodity;

            if(thing4sale){

                TraderEntity* trader = GetShopFormImp()->GetTrader();
                TraderContext* context = trader->GetEntityContext();

                //защита от дурака: нельзя купить предмет если не хватает денег
                if(MoneyMgr::GetInst()->GetMoney() < trader->GetInfo()->GetSellPrice(thing4sale)){
                    DirtyLinks::Print( DirtyLinks::GetStrRes("no_money"));
                    SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_LOGIC_DENIED));
                    return 0;
                }

                context->RemoveThing(thing4sale);

                //разберемся с деньгами
                int sum = trader->GetInfo()->GetSellPrice(thing4sale);
                MoneyMgr::GetInst()->SetMoney(MoneyMgr::GetInst()->GetMoney() - sum);                

                TraderContext::iterator itor = GetLowerBound(context->begin(m_mask), context->end(), thing4sale);

                if(itor != context->end())
                    m_commodity = &*itor;
                else
                    Next();
            }
         
            return thing4sale;
        }

        BaseThing* GetThing()
        {
            return m_commodity;
        }

    private:

        TraderContext::iterator GetLowerBound(TraderContext::iterator first, TraderContext::iterator last, BaseThing* thing)
        {
            TraderContext::less_thing less;

            while(first != last){

                if(!less(&*first, thing) && !less(thing, &*first))
                    return first;

                ++first;
            }

            return first;
        }

        TraderContext::iterator GetUpperBound(TraderContext::iterator first, TraderContext::iterator last, BaseThing* thing)
        {
            TraderContext::less_thing less;

            while(first != last){

                if(less(thing, &*first))
                    return first;

                ++first;
            }

            return first;
        }

        TraderContext::iterator GetPrevious(TraderContext::iterator first, TraderContext::iterator last)
        {
            TraderContext::iterator prev = last;
            while(first != last) prev = first++;
            return prev;
        }
        
    private: 

        unsigned    m_mask;
        BaseThing*  m_commodity;

        TradeScreen::SHEET m_sheet;
    };
}

//===============================================================

class MarketSlotDecorator : public SlotDecorator{
public:
    
    MarketSlotDecorator(SlotStrategy* strategy) :
      m_user(0), SlotDecorator(strategy) {}      
      
    void Fill(SESlot* slot, SlotEnv* env)
    {
        SlotDecorator::Fill(m_slot = slot, m_env = env);

        SEItem* mouse = m_env->GetEngine()->MigrationMgr()->GetItem();
        Illuminate(SlotStrategy::ST_NONE, mouse ? mouse->GetBaseThing() : 0);

        if(m_user == 0) m_user = m_env->GetActor();
    }

    void Clear() { }

    //инициализация при входе в меню
    void Init()
    {
      m_slot->UnScroll();
    }

    //очистка при выходе из меню
    void Shut()
    {
		STACK_GUARD("MarkerSlotDec::Shut");
        SESlot::Iterator itor = m_slot->Begin();

        while(itor != m_slot->End()){
            m_env->GetGround()->Insert(m_user, itor->GetBaseThing());
            ++itor;
        }

        m_slot->Clear();
        m_user = 0;
    }

    void Illuminate(SlotStrategy::slot_type slot, BaseThing* thing)
    {
        SlotDecorator::Illuminate(slot, thing);
        GetShopFormImp()->HandleIlluminateSheetReq(thing);
    }
      
    bool DoDrag(SEItem* item)
    {
        bool ret = SlotDecorator::DoDrag(item);
        UpdateTotalSum();
        return ret;
    }
      
    bool DoDrop(SEItem* item, const ipnt2_t& to)
    {
        bool ret = SlotDecorator::DoDrop(item, to);
        UpdateTotalSum();
        return ret;
    }
      
    void SellThings(TraderEntity* trader)
    {
        SESlot::Iterator itor = m_slot->Begin();
        
        //разберемся с деньгами
        MoneyMgr::GetInst()->SetMoney(MoneyMgr::GetInst()->GetMoney() + CalcTotalSum());
        
        //проиграть звук продажи предметов
        if(itor != m_slot->End()) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_SELL_TO_TRADER));

        //вставим предметы в торговца
        while(itor != m_slot->End()){
            
            SEItem* item = &*(itor++);
            
            m_slot->Remove(item);
            
            trader->GetEntityContext()->InsertThing(item->GetBaseThing());
            
            delete item;              
        } 
        
        UpdateTotalSum();
    }
      
    void BuyThing(BaseThing* thing)
    {
        //надо вставить предмет в слот
        SEItem* item = m_env->CreateItem(thing);
        
        if(!m_slot->CanInsert(item)) throw CASUS("MarketSlotDecorator: А слот-то не бесконечен!");
        
        //раскрасить предмет
        m_env->SetState(item);          
        //вставить в слот
        m_slot->Insert(item);
        
        UpdateTotalSum();
    }
    
private:
    
    void UpdateTotalSum()
    {
        Screens::Instance()->Trade()->SetText(TradeScreen::T_SALE_COST, Int2Str(CalcTotalSum()).c_str());
    }
    
    int CalcTotalSum()
    {
        int sum = 0;
        
        SESlot::Iterator itor = m_slot->Begin();
        
        while(itor != m_slot->End()){
            sum += GetShopFormImp()->GetTrader()->GetInfo()->GetBuyPrice(itor->GetBaseThing());
            ++itor;
        } 
        
        return sum;
    }
    
private:

    SlotEnv* m_env;
    SESlot*  m_slot;

    HumanEntity* m_user;
};

//===============================================================

ShopFormImp::ShopFormImp() :
    m_hero(0), m_cur_sheet(TradeScreen::S_GUNS), m_drag_drop(new DragDropManager(Screens::Instance()->Trade()->GetSlotsEngine()))
{
    Screens::Instance()->Trade()->SetController(this);

    memset(m_sheet, 0, sizeof(m_sheet));

    //установим вкладки
    m_sheet[TradeScreen::S_AMMO] = new UsualSheet(TradeScreen::S_AMMO, TT_AMMO);
    m_sheet[TradeScreen::S_GUNS] = new UsualSheet(TradeScreen::S_GUNS, TT_WEAPON);
    m_sheet[TradeScreen::S_GREN] = new UsualSheet(TradeScreen::S_GREN, TT_GRENADE);
    m_sheet[TradeScreen::S_ARMOR] = new UsualSheet(TradeScreen::S_ARMOR, TT_ARMOUR);
    m_sheet[TradeScreen::S_EQUIP] = new UsualSheet(TradeScreen::S_EQUIP, TT_IMPLANT|TT_MEDIKIT|TT_MONEY|TT_KEY|TT_CAMERA|TT_SHIELD|TT_SCANNER);

    //вставим слоты в механизм Drag&Drop'a

    m_market_slot = new MarketSlotDecorator(SlotFactory::GetInst()->Create(SlotStrategy::ST_MARKET));
    m_drag_drop->Insert(TradeScreen::S_TRADE, m_market_slot);

    m_unloader = new UnloadingDecorator(SlotFactory::GetInst()->Create(SlotStrategy::ST_HANDS));
	m_drag_drop->Insert(TradeScreen::S_HANDS, m_unloader);

    m_drag_drop->Insert(TradeScreen::S_BODY, SlotFactory::GetInst()->Create(SlotStrategy::ST_BODY));
	m_drag_drop->Insert(TradeScreen::S_HEAD, SlotFactory::GetInst()->Create(SlotStrategy::ST_HEAD));
    m_drag_drop->Insert(TradeScreen::S_GROUND, SlotFactory::GetInst()->Create(SlotStrategy::ST_GROUND));
	m_drag_drop->Insert(TradeScreen::S_IMPLANT, SlotFactory::GetInst()->Create(SlotStrategy::ST_IMPLANTS)); 
	m_drag_drop->Insert(TradeScreen::S_BACKPACK, SlotFactory::GetInst()->Create(SlotStrategy::ST_BACKPACK));
	m_drag_drop->Insert(TradeScreen::S_KNEE_LEFT, SlotFactory::GetInst()->Create(SlotStrategy::ST_LKNEE));
	m_drag_drop->Insert(TradeScreen::S_KNEE_RIGHT, SlotFactory::GetInst()->Create(SlotStrategy::ST_RKNEE));
}

ShopFormImp::~ShopFormImp()
{
    delete m_drag_drop;

    for(int k = 0; k < TradeScreen::MAX_SHEET; delete m_sheet[k++]);

    //Screens::Instance()->Trade()->SetController(0);
}

void ShopFormImp::Show()
{
    Screens::Instance()->Activate(Screens::SID_TRADE);
}

void ShopFormImp::ShowFirstActiveSheet()
{
    TradeScreen* screen = Screens::Instance()->Trade();

    //выберем текущую вкладку
    for(int k = 0; k < TradeScreen::MAX_SHEET; k++){        

        if(m_sheet[k]->GetThing()){
            
            screen->EnableButton(TradeScreen::B_BUY, true);
            screen->EnableButton(TradeScreen::B_PREV_THING, true);
            screen->EnableButton(TradeScreen::B_NEXT_THING, true);

            OnSheetClick(static_cast<TradeScreen::SHEET>(k));
            return;
        }
    }

    //все равно что показывать, они все пустые
    m_sheet[0]->Show(m_hero);

    m_cur_sheet = TradeScreen::MAX_SHEET;

    screen->EnableButton(TradeScreen::B_BUY,false);
    screen->EnableButton(TradeScreen::B_PREV_THING,false);
    screen->EnableButton(TradeScreen::B_NEXT_THING,false);
}

void ShopFormImp::HandleInput(state_type* state)
{
    if(Input::KeyBack(DIK_ESCAPE)) HandleExitFormReq();

    /*
    if(Input::KeyBack(DIK_P)) HandlePrevHeroReq();
    if(Input::KeyBack(DIK_N)) HandleNextHeroReq();
    */
}

void ShopFormImp::Init(const ini_s& ini)
{
    const shop_ini_s& sini = static_cast<const shop_ini_s&>(ini);

    m_trader = sini.m_trader;
    m_ground = sini.m_ground;
    m_scroller = sini.m_scroller;

    //установим параметры торговца
    TradeScreen::TraderTraits traits;

    std::string desc = GetTraderDesc(m_trader);

    traits.m_Description = desc.c_str();
    traits.m_Portrait    = m_trader->GetInfo()->GetRID().c_str();

    TradeScreen* screen = Screens::Instance()->Trade();

    screen->SetTraderTraits(traits);
    
    //установим кнопки в первоначальное состояние
    screen->SetSheetState(TradeScreen::S_AMMO, TradeScreen::SS_NORMAL);
    screen->SetSheetState(TradeScreen::S_GUNS, TradeScreen::SS_NORMAL);
    screen->SetSheetState(TradeScreen::S_GREN, TradeScreen::SS_NORMAL);
    screen->SetSheetState(TradeScreen::S_ARMOR, TradeScreen::SS_NORMAL);
    screen->SetSheetState(TradeScreen::S_EQUIP, TradeScreen::SS_NORMAL);

    //проинициализируем вкладки
    for(int k = 0; k < TradeScreen::MAX_SHEET; m_sheet[k++]->Init());

    //установим поля денег
    screen->SetText(TradeScreen::T_SALE_COST, Int2Str(0).c_str());
    screen->SetText(TradeScreen::T_HERO_MONEY, Int2Str(MoneyMgr::GetInst()->GetMoney()).c_str());

    //если скролл не возможен запретим кнопки скролла
    screen->EnableButton(TradeScreen::B_PREV_HERO, m_scroller->CanScroll());    
    screen->EnableButton(TradeScreen::B_NEXT_HERO, m_scroller->CanScroll());    

    m_drag_drop->SetBin(sini.m_bin);
    m_drag_drop->SetGround(sini.m_ground);

    //выделим нашего перца
    m_scroller->First(sini.m_human);
    HandleSelectHeroReq(m_scroller->Get());

    m_market_slot->Init();
    
    //выберем текущую вкладку
    ShowFirstActiveSheet();   
}

void ShopFormImp::OnButtonClick(TradeScreen::BUTTON button)
{
    switch(button){
    case TradeScreen::B_PREV_HERO:
        HandlePrevHeroReq();
        break;

    case TradeScreen::B_NEXT_HERO:
        HandleNextHeroReq();
        break;

    case TradeScreen::B_UNLOAD:
        m_unloader->Unload();
        break;        

    case TradeScreen::B_BACK:
        HandleExitFormReq();
        break;

    case TradeScreen::B_PREV_THING:
        HandlePrevCommodityReq();
        break;

    case TradeScreen::B_NEXT_THING:
        HandleNextCommodityReq();
        break;

    case TradeScreen::B_BUY:
        HandleBuyReq();
        break;

    case TradeScreen::B_SELL:
        HandleSellReq();
        break;
    }
}

void ShopFormImp::OnCommodityClick()
{
    if(BaseThing* thing = m_sheet[m_cur_sheet]->GetThing())
        Forms::GetInst()->ShowDescDialog(thing);
}

void ShopFormImp::OnSheetClick(TradeScreen::SHEET sheet)
{
    m_cur_sheet = sheet;

    m_sheet[m_cur_sheet]->Show(m_hero);
    Screens::Instance()->Trade()->SetSheetState(m_cur_sheet, TradeScreen::SS_PUSHED);
}

void ShopFormImp::HandlePrevCommodityReq()
{
    m_sheet[m_cur_sheet]->Prev();
    m_sheet[m_cur_sheet]->Show(m_hero);
}

void ShopFormImp::HandleNextCommodityReq()
{
    m_sheet[m_cur_sheet]->Next();
    m_sheet[m_cur_sheet]->Show(m_hero);
}

void ShopFormImp::HandleExitFormReq()
{
    if(m_drag_drop->CancelDrop())
        return;

    m_market_slot->Shut();

    HandleSelectHeroReq(0);
    Forms::GetInst()->ShowGameForm();
}

void ShopFormImp::HandleSelectHeroReq(HumanEntity* hero)
{   
    if(m_hero) m_hero->Detach(this);

    m_drag_drop->SetActor(m_hero = hero);

    if(m_hero == 0) return;

    m_hero->Attach(this, EV_TAKE_MONEY);
    m_hero->Attach(this, EV_WEIGHT_CHANGE);
    m_hero->Attach(this, EV_WEAPON_STATE_CHANGE);

    //имя героя
    Screens::Instance()->Trade()->SetText(TradeScreen::T_HERO_NAME, m_hero->GetInfo()->GetName().c_str());

    Update(m_hero, 0, 0);
}

void ShopFormImp::Update(BaseEntity* entity, event_t event, info_t)
{
    TradeScreen::HeroTraits hero_traits;
    HumanContext* context = m_hero->GetEntityContext();
    
    //заполним свойства героя
    hero_traits.m_WeightMax     = context->GetLimits()->GetWeight(); 
    hero_traits.m_WeightCurrent = context->GetTraits()->GetWeight();

    TradeScreen* screen = Screens::Instance()->Trade();

    //установим свойства героя
    screen->SetHeroTraits(hero_traits);

    //кнопка разрядки оружия
    HumanContext::iterator itor = context->begin(HPK_HANDS, TT_WEAPON);
    screen->EnableButton(TradeScreen::B_UNLOAD, itor != context->end() && static_cast<WeaponThing*>(&*itor)->GetAmmo());    
}

TraderEntity* ShopFormImp::GetTrader()
{
    return m_trader;
}

void ShopFormImp::HandleSellReq()
{
    m_market_slot->SellThings(m_trader);

    for(int k = 0;  k < TradeScreen::MAX_SHEET; k++){
        m_sheet[k]->Next();    
        m_sheet[k]->Prev();
    }

    if(m_cur_sheet == TradeScreen::MAX_SHEET)
        ShowFirstActiveSheet();
    else
        m_sheet[m_cur_sheet]->Show(m_hero);
    
    Screens::Instance()->Trade()->SetText(TradeScreen::T_HERO_MONEY, Int2Str(MoneyMgr::GetInst()->GetMoney()).c_str());
}

void ShopFormImp::HandleBuyReq()
{
    if(BaseThing* thing = m_sheet[m_cur_sheet]->SellThing()){

        m_market_slot->BuyThing(thing);

        if(m_sheet[m_cur_sheet]->GetThing())
            m_sheet[m_cur_sheet]->Show(m_hero);
        else
            ShowFirstActiveSheet();

        //проиграть звук успешной покупки предмета
        SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_BUY_FROM_TRADER_SUCCEEDED));
        Screens::Instance()->Trade()->SetText(TradeScreen::T_HERO_MONEY, Int2Str(MoneyMgr::GetInst()->GetMoney()).c_str());
        return;
    }

    //проиграть звук не успешной покупки предмета
    SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_BUY_FROM_TRADER_FAILED));
}

void ShopFormImp::HandleIlluminateSheetReq(BaseThing* active)
{
    if(m_hero && m_cur_sheet != TradeScreen::MAX_SHEET)
        m_sheet[m_cur_sheet]->Show(m_hero, active);
}

void ShopFormImp::OnCommodityHanging(HANGING_MODE hm)
{
    m_drag_drop->Illuminate(SlotStrategy::ST_NONE, hm == HM_START ? m_sheet[m_cur_sheet]->GetThing() : 0);
}

void ShopFormImp::HandleNextHeroReq()
{
    m_scroller->Next();
    
    if(m_scroller->IsEnd()) m_scroller->First();
    
    HandleSelectHeroReq(m_scroller->Get());
}

void ShopFormImp::HandlePrevHeroReq()
{
    m_scroller->Prev();

    if(m_scroller->IsREnd()) m_scroller->Last();
    
    HandleSelectHeroReq(m_scroller->Get());
}

void ShopFormImp::MakeSaveLoad(SavSlot& slot)
{
}



