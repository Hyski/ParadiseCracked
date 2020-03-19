//
// набор игровых меню 
//

#ifndef _PUNCH_FORM_H_
#define _PUNCH_FORM_H_

struct phrase_s;
class BaseThing;

class BaseEntity;
class HumanEntity;
class TraderEntity;

class HMForm;
class BaseDialog;
class ConsoleForm;

//
// абстракция земли для заполнения "земляного" слота
//
class AbstractGround{
public:

    virtual ~AbstractGround(){}

    //можно ли положить предмет в контейнер
    virtual bool CanInsert(HumanEntity* actor, BaseThing* thing, std::string* reason) const = 0;

    //положить предмет в контейнер
    virtual void Insert(HumanEntity* actor, BaseThing* thing) = 0;
    //удалить предмет из контейнера
    virtual void Remove(HumanEntity* actor, BaseThing* thing) = 0;

    enum sound_event{
        SE_DRAG,    
        SE_DROP,
    };

    //обработка проигрывания звука
    virtual void PlaySound(sound_event sound) = 0;

    //
    // полиморфный итератор по предметам
    // 
    class Iterator{
    public:
        
        virtual ~Iterator(){}

        //перейти на следущий элемент 
        virtual void Next() = 0;
        //перейти на первый элемент
        virtual void First() = 0;

        //получить доступ к предмету
        virtual BaseThing* Get() = 0;
        //итерирование еще не закончено?
        virtual bool IsNotDone() const = 0;
    };

    //породить полиморфный итератор
    virtual Iterator* CreateIterator(HumanEntity* human) = 0;
};

//
// абстракция скроллера по комманде
//
class AbstractScroller{
public:

    virtual ~AbstractScroller(){}

    //==========================================================
    
    //
    // функции для итерации в обратном направлении
    // 

    //перейти к предид персонажу
    virtual void Prev() = 0;
    //это конец последовательности?
    virtual bool IsREnd() const = 0;
    
    //установить итератор на конец последовательности
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // если:
    //      human == 0 итератор устанавл. в конец последоват.
    //      human != 0 итератор устанавл. на human c конца посл
    //
    virtual void Last(HumanEntity* human = 0) = 0;

    //==========================================================

    //
    // функции для итерации в прямом направлении
    // 
    
    //перейти к следущему персонажу
    virtual void Next() = 0;
    //это начало последовательности?
    virtual bool IsEnd() const = 0;

    //установить итратор в начало последовательности
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // если:
    //      human == 0 итератор устанавл. в начало последоват.
    //      human != 0 итератор устанавл. на human c начала посл
    //
    virtual void First(HumanEntity* human = 0) = 0;

    //==========================================================

    //в последовательности только один элемент?
    bool CanScroll() const;

    //получить ссылку на текущий эл-т последовательности
    virtual HumanEntity* Get() = 0;
};

//
// абстракция корзины - в нее попадают не нужные предметы
//
class AbstractBin{
public:

    virtual ~AbstractBin(){}

    //добавить предмет в корзину
    virtual void Insert(HumanEntity* actor, BaseThing* thing) = 0;
};

//
// абстракция progress_bar
//
class AbstractProgressBar{
public:

    virtual ~AbstractProgressBar(){}

    //установить движок + текстовую метку
    virtual void SetPos(float value, const std::string& text = std::string()) = 0;
};

//
// базовый класс для всех меню в игре
//
class BaseForm{
public:

    virtual ~BaseForm(){}

    struct ini_s {};

    //для инициализации магазина
    struct shop_ini_s : public ini_s{
      HumanEntity*      m_human;
      TraderEntity*     m_trader;
      AbstractBin*      m_bin;
      AbstractGround*   m_ground;
      AbstractScroller* m_scroller;

      shop_ini_s(HumanEntity* human, TraderEntity* trader, AbstractBin* bin,
                 AbstractGround* ground, AbstractScroller* scroller) :
        m_human(human), m_ground(ground), m_trader(trader), m_bin(bin), m_scroller(scroller){}
    };

    //инициаализация меню одевания
    struct inv_ini_s : public ini_s{       
       HumanEntity*      m_human;
       AbstractBin*      m_bin;
       AbstractGround*   m_ground;
       AbstractScroller* m_scroller;

       inv_ini_s(HumanEntity* human, AbstractScroller* scroller,
                 AbstractGround* ground, AbstractBin* bin): 
         m_human(human), m_scroller(scroller), m_ground(ground), m_bin(bin){}
    };

    //инициализация меню скрытых перем
    struct hm_ini_s : public ini_s{
        player_type m_player;
        hm_ini_s(player_type player) : m_player(player) {}
    };

    //сделать меню текущим
    virtual void Show() = 0;
    //обработка ввода
    virtual void HandleInput(state_type* st) = 0;
    //сохранение/загрузка
    virtual void MakeSaveLoad(SavSlot& slot) = 0;
    //инициализация меню
    virtual void Init(const ini_s& ptr = ini_s()) = 0;
};

//
// меню журнала
//
class JournalForm : public BaseForm{
public:

    //есть ли новая информация в меню
    virtual bool IsUpdated() const = 0;
};

//
// фабрика меню
//
class FormFactory{
public:

    static FormFactory* GetInst();

    //создать меню торговли
    BaseForm* CreateShopForm();
    //создать меню паники
    BaseForm* CreatePanicForm();
    //создать меню одевания персонажа
    BaseForm* CreateInventoryForm();

    //создать игровое меню 
    BaseForm* CreateGameForm();
    //создать меню журнала
    JournalForm* CreateJournalForm();

    //создать progress bar
    AbstractProgressBar* CreateLoadBar(float from, float to);

private:

    FormFactory(){}
};

//
// система игровых меню 
//
class Forms{
public:

    ~Forms();

    //singleton
    static Forms* GetInst();

    enum reset_type {
        RT_SHUT,
        RT_NEXT_LEVEL
    };
   
    //переинициализация меню
    void Reset(reset_type type);
    //сохранить/восстановить сист. меню
    void MakeSaveLoad(SavSlot& st);

    //получить ссылку на соотв меню
    HMForm* GetHMForm();
    BaseForm* GetShopForm();
    BaseForm* GetGameForm();
    BaseForm* GetPanicForm();
    BaseForm* GetInventoryForm();
    JournalForm* GetJournalForm();

    //обработать клавиатурный ввод в текущем меню
    void HandleInput(state_type* st);

    //перейти в журнал
    void ShowJournalForm();
    //перейти в основное игровое меню
    void ShowMainMenuForm();
    //показать экран паники
    void ShowPanicForm(HumanEntity* panicer);
    //показать экран скрытых перемещений
    void ShowHMForm(player_type player);
    //отобразить информкацию о персонаже
    void ShowHeroInfo(HumanEntity* hero);
    //перейти в игровое меню
    void ShowGameForm(bool focus_camera = false);
    //перейти в меню одевания
    void ShowInventoryForm(HumanEntity* human, AbstractGround* ground, AbstractScroller* scroller, AbstractBin* bin);
    //перейти в магазин
    void ShowShopForm(HumanEntity* hero, TraderEntity* trader, AbstractGround* ground, AbstractScroller* scroller, AbstractBin* bin);

    enum options_mode{
        ODM_GAMEMENU,    //полноценное меню опций
        ODM_LOADMENU,    //экран загрузки
    };
    
    //перейти в диалог игровых опций
    void ShowOptionsDialog(options_mode mode);

    //перейти в диалог описания предмета
    void ShowDescDialog(BaseThing* thing);
    //перейти в диалог описания
    void ShowDescDialog(const std::string& desc);

    //показать диалог разговора
    void ShowTalkDialog(BaseEntity* entity, const phrase_s& phrase);

    struct helper_s{
        std::string m_image;
        std::string m_message;

        bool m_fenable_cur;
        bool m_fenable_all;

        helper_s(std::string im, std::string msg, bool fcur, bool fall) : 
            m_image(im), m_message(msg), m_fenable_cur(fcur), m_fenable_all(fall){}
    };

    //показать диалог помощника
    void ShowHelper(helper_s& helper);

    //показать bink
    void ShowBink(const rid_t& rid);

    //показать message box
    bool ShowMessageBox(const std::string& text);
    //установить progress bar в экране hidden movement
    void UpdateHMProress(float val);
    //установить вимость экрана HiddenMovement
    void SetHMVisibility(bool flag);
   
protected:

    Forms();

private:

    BaseForm*    m_current;
    BaseDialog*  m_dialog;

    HMForm*   m_hm_form;
    BaseForm* m_shop_form;
    BaseForm* m_game_form;
    BaseForm* m_panic_form;
    BaseForm* m_inventory_form; 

    ConsoleForm* m_console;
    JournalForm* m_journal_form;
};

#endif // _PUNCH_FORM_H_