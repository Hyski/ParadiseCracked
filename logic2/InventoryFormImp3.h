//
// реализация меню одевания iteration 3
//

#ifndef _PUNCH_INVENTORYFORM3_H_
#define _PUNCH_INVENTORYFORM3_H_

#include "form.h"
#include "entityaux.h"
#include "../interface/inventoryscreen.h"

class HumanEntity;
class DragDropManager;
class UnloadingDecorator;

//
// абстракция механизма LevelUp'a
//
class LevelupManager{
public:

    virtual ~LevelupManager(){}

    //установить героя
    virtual void SetActor(HumanEntity* human) = 0;

    //узнать кол-во очков levelup'a
    virtual int GetPoints() const = 0;

    //узнать значение параметра
    virtual int GetTrait(InventoryScreen::LEVEL_UP trait) const = 0;
    virtual int GetLimit(InventoryScreen::LEVEL_UP trait) const = 0;

    //собственно увеличение/уменьшение параметров
    virtual void Inc(InventoryScreen::LEVEL_UP trait) = 0;
    virtual void Dec(InventoryScreen::LEVEL_UP trait) = 0;
};

//
// реализация меню
//
class InventoryFormImp3 : public  BaseForm,
                          private EntityObserver, 
                          private InventoryScreenController
{
public:

    InventoryFormImp3();
    ~InventoryFormImp3();

    //показать меню
    void Show();
    //инициализация перед показом меню
    void Init(const ini_s& ini);
    //сохранить/загрузить настройки меню
    void MakeSaveLoad(SavSlot& slot);
    //обработка клавитаурного ввода
    void HandleInput(state_type* state);

private:

    //удалить героя 
    void HandleDismissHero();
    //выход из меню
    void HandleExitFormReq();
    //перейти к следущему герою
    void HandleNextHeroReq();
    //перейти к предид герою
    void HandlePrevHeroReq();
    //выделение персонажа в меню
    void HandleSelectHeroReq(HumanEntity* hero);

    //enable/disable кнопок уволнения/перемотки персонажей
    void EnableDismissAndScrollHero();

    //обработка нажатия кнопок
    void OnButtonClick(InventoryScreen::BUTTON id);
    //обработка описания levelup'a
    void OnLevelUpDescription(InventoryScreen::LEVEL_UP id);
    //обработка levelup'a
	void OnLevelUpClick(InventoryScreen::LEVEL_UP id, InventoryScreen::LEVEL_UP_SIGN sign);

    //уведомление об изменении параметров существа
    void Update(BaseEntity* entity, event_t event, info_t info);

private:

    HumanEntity*      m_hero;
    AbstractGround*   m_ground;
    AbstractScroller* m_scroller;
    DragDropManager*  m_drag_drop; 
    LevelupManager*   m_leveluper;

    UnloadingDecorator* m_unloader;
};

#endif //_PUNCH_INVENTORYFORM3_H_