//
// реализация меню торговли
//

#ifndef _PUNCH_SHOPFORMIMP_H_
#define _PUNCH_SHOPFORMIMP_H_

#include "form.h"
#include "entityaux.h"

#include "../interface/tradescreen.h"

class BaseThing;
class HumanEntity;
class TraderEntity;
class DragDropManager;
class UnloadingDecorator;
class MarketSlotDecorator;

//
// абстракция вкладки на предмет в витрине торговца
//
class CommoditySheet{
public:

    virtual ~CommoditySheet(){}

    //инициализация перед входом в меню
    virtual void Init() = 0;
    //показать текущее состояние вкладки
    virtual void Show(HumanEntity* hero, BaseThing* active = 0) = 0;

    //перейти к следущему элементу 
    virtual void Next() = 0;
    //перейти к предидущему элементу
    virtual void Prev() = 0;  
    
    //продать товар клиенту
    virtual BaseThing* SellThing() = 0;
    //получит ссылку на предмет
    virtual BaseThing* GetThing() = 0;
};

//
// реализация меню торговли
//
class ShopFormImp : public  BaseForm,
                    private EntityObserver,
                    private TradeScreenController
{
public:

    ShopFormImp();
    ~ShopFormImp();

    //узнать текущего торговца
    TraderEntity* GetTrader();

    //показать меню
    void Show();
    //инициализация меню
    void Init(const ini_s& ini);  
    //сохранить/загрузить настройки меню
    void MakeSaveLoad(SavSlot& slot);
    //обработка клавиатурного ввода
    void HandleInput(state_type* state);
    //уведомление о необх подсветки предмета
    void HandleIlluminateSheetReq(BaseThing* active);

private:

    //обработка покупки предмета
    void HandleBuyReq();
    //обработка продажи предметов
    void HandleSellReq();
    //обработка закрытия меню
    void HandleExitFormReq();
    //перейти к следущему герою
    void HandleNextHeroReq();
    //перейти к предидущему герою
    void HandlePrevHeroReq();
    //обработка перехода к предидущему предмету
    void HandlePrevCommodityReq();
    //обработка перехода к следущему предмету
    void HandleNextCommodityReq();
    //обработка выделения персонажа
    void HandleSelectHeroReq(HumanEntity* hero);

    //обработка нажатия правой кнопки мыши на предмете
	void OnCommodityClick();
    //обработка зависания мышки над предметом торговца
    void OnCommodityHanging(HANGING_MODE hm);
    //обработка перехода на другой таб
    void OnSheetClick(TradeScreen::SHEET id);
    //обработка нажатия кнопки
	void OnButtonClick(TradeScreen::BUTTON id);

    //отобразить первую активную вкладку
    void ShowFirstActiveSheet();

    //обновление параметров персонажа
    void Update(BaseEntity* entity, event_t event, info_t);
private:

    HumanEntity*  m_hero;
    TraderEntity* m_trader;

    AbstractGround*   m_ground;
    AbstractScroller* m_scroller;
    DragDropManager*  m_drag_drop; 

    UnloadingDecorator*  m_unloader; 
    MarketSlotDecorator* m_market_slot;

    TradeScreen::SHEET m_cur_sheet;
    CommoditySheet*    m_sheet[TradeScreen::MAX_SHEET];
};

#endif // _PUNCH_SHOPFORMIMP_H_
