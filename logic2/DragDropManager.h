//
// сервер для работы с механизмом drag & drop'a
//

#ifndef _PUNCH_DRAGDROPMANAGER_H_
#define _PUNCH_DRAGDROPMANAGER_H_

#include "entityaux.h"
#include "../interface/SlotsEngine.h"

class SlotEnv;
class HumanEntity;
class SlotsEngine;
class AbstractBin;
class AbstractGround;

typedef unsigned slid_t; //идентификатор слота

//
// стратегия на слот
//
class SlotStrategy{
public:

    virtual ~SlotStrategy();

    enum slot_type{
        ST_NONE,
        ST_HEAD,
        ST_BODY,
        ST_HANDS,
        ST_LKNEE,
        ST_RKNEE,
        ST_GROUND,
        ST_MARKET,
        ST_IMPLANTS,
        ST_BACKPACK,
    };

    //узнать тип слота
    virtual slot_type GetType() const = 0;

    //закрытие слота (необх. сохранить позицию предметов)
    virtual void Clear() = 0;
    //открытие слота (необх. заполнить слот)
    virtual void Fill(SESlot* slot, SlotEnv* env) = 0;
    //подсветить предметы в слоте 
    virtual void Illuminate(slot_type slot, BaseThing* thing) = 0;

    //обработка стаскивания предмета 
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:
    //          true  - стаскивание было
    //          false - стаскивания не было
    //
    virtual bool DoDrag(SEItem* item) = 0;

    //обработка бросания предмета
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:
    //          true  - бросание было
    //          false - бросания не было
    //
    virtual bool DoDrop(SEItem* item, const ipnt2_t& to) = 0;
};

//
// Декоратор для стратегии слота
//
class SlotDecorator : public SlotStrategy{
public:

    SlotDecorator(SlotStrategy* stratetgy);
    ~SlotDecorator();

    //узнать тип слота
    slot_type GetType() const;

    //узнать тип слота
    void Clear();
    //открытие слота (необх. заполнить слот)
    void Fill(SESlot* slot, SlotEnv* env);
    //подсветить предметы в слоте 
    void Illuminate(slot_type slot, BaseThing* thing);

    //обработка стаскивания предмета 
    bool DoDrag(SEItem* item);
    //обработка бросания предмета
    bool DoDrop(SEItem* item, const ipnt2_t& to);

private:

    SlotStrategy* m_strategy;
};

//
// декоратор разряжающий оружие в руках персонажа
//
class UnloadingDecorator : public SlotDecorator{
public:

    UnloadingDecorator(SlotStrategy* strategy);

    //открытие слота (необх. заполнить слот)
    void Fill(SESlot* slot, SlotEnv* env);
    //разрядить оружие 
    void Unload();

private:

    SlotEnv* m_env;
};

//
// контекст для стратегии слотов
//
class SlotEnv{
public:

    virtual ~SlotEnv() {}

    //получить ссылку на активного героя
    virtual HumanEntity* GetActor() const = 0;
    //получить доступ к контейнеру слотов
    virtual SlotsEngine* GetEngine() const = 0;
    //получить ссылку на землю
    virtual AbstractGround* GetGround() const = 0;

    //
    //утилиты для работы в слотах
    //

    //установить текст на item'e
    virtual void UpdateText(SEItem* item) = 0;
    //сценарий сбрасывания предмета
    virtual void DiscardItem(SEItem* item) = 0;
    //создать item для слота
    virtual SEItem* CreateItem(BaseThing* thing) = 0;
    //установить состояние предмета
    virtual void SetState(SEItem* item, BaseThing* thing = 0) = 0;

    //откуда пришел предмет
    virtual SlotStrategy::slot_type GetLastOwner(SEItem* item) const = 0;
};

//
// создание стратегий для слотов
//
class SlotFactory{
public:

    static SlotFactory* GetInst();

    //созать слот соответствующего типа
    SlotStrategy* Create(SlotStrategy::slot_type type);

private:

    SlotFactory(){}
};

//
// менеджер Drag & Drop'a
//
class DragDropManager : private SlotEnv,
                        private SEController,
                        private EntityObserver
{
public:

    DragDropManager(SlotsEngine* engine);
    ~DragDropManager();

    //установить нужный слот 
    void Insert(slid_t slot, SlotStrategy* strategy);

    //установить ссылку на абстракцию корзины
    void SetBin(AbstractBin* bin);
    //установить активного персонажа
    void SetActor(HumanEntity* human);
    //установить ссылку на абстракцию земли
    void SetGround(AbstractGround* ground);

    //отменить режим сбрасывания
    bool CancelDrop(); 

    //подсветить все
    void Illuminate(SlotStrategy::slot_type slot, BaseThing* thing);

private:

    //===============================================
    
    //получить ссылку на активного героя
    HumanEntity* GetActor() const;
    //получить доступ к контейнеру слотов
    SlotsEngine* GetEngine() const;
    //получить ссылку на землю
    AbstractGround* GetGround() const;
    
    //установить текст на item'e
    void UpdateText(SEItem* item);
    //сценарий сбрасывания предмета
    void DiscardItem(SEItem* item);
    //создать item для слота
    SEItem* CreateItem(BaseThing* thing);
    //установить состояние предмета
    void SetState(SEItem* item, BaseThing* thing);
    //откуда пришел предмет
    SlotStrategy::slot_type GetLastOwner(SEItem* item) const;

    //===============================================

    //обработка нажатия правой кнопки мыши
    void OnItemClick(SEItem* item);
    //обработка стаскивания
    void OnDrag(SESlot* slot, SEItem* item);
    //обоработка зависания
  	void OnLighting(SEItem* item,LIGHTING_MODE lm);
    //обработка бросания
	void OnDrop(SESlot* slot, SEItem* item, const ipnt2_t& pos);

    //===============================================

    //обработка изменения состояния существа
    void Update(BaseEntity* entity, event_t event, info_t info);

private:

    HumanEntity* m_actor;
    SlotsEngine* m_engine;

    AbstractBin*    m_bin;
    AbstractGround* m_ground;    

    typedef std::vector<SlotStrategy*> slot_vec_t;
    slot_vec_t m_slots;
};

#endif // _PUNCH_DRAGDROPMANAGER_H_

