//
// фабрика предметов
//

#ifndef _PUNCH_THINGFACTORY_H_
#define _PUNCH_THINGFACTORY_H_

class BaseThing;

//
// фабрика предметов
//
class ThingFactory{
public:

    //получить текущую фабрику предметов
    static ThingFactory* GetInst();

    //создать ключ
    virtual BaseThing* CreateKey(const rid_t& rid) = 0;
    //создать деньги (если sum == 0 ==> сумма берется из поля стоимости)
    virtual BaseThing* CreateMoney(int sum) = 0;
    //создать броник
    virtual BaseThing* CreateArmor(const rid_t& rid) = 0;
    //создать силовое поле
    virtual BaseThing* CreateShield(const rid_t& rid) = 0;
    //создать камеру
    virtual BaseThing* CreateCamera(const rid_t& rid) = 0;
    //создать гранату
    virtual BaseThing* CreateGrenade(const rid_t& rid) = 0;
    //создать сканнер
    virtual BaseThing* CreateScanner(const rid_t& rid) = 0;
    //cоздать имплант
    virtual BaseThing* CreateImplant(const rid_t& rid) = 0;
    //создать патроны (если count == 0 ==> это полная обойма)
    virtual BaseThing* CreateAmmo(const rid_t& rid, int cout) = 0;
    //создать аптечку (count == 0 ==> в аптечке обычный заряд)
    virtual BaseThing* CreateMedikit(const rid_t& rid, int charge) = 0;
    //создать оружие (ammo_count != 0 ==> в оружие кладуться патроны)
    virtual BaseThing* CreateWeapon(const rid_t& rid, const rid_t& ammo_rid, int ammo_count) = 0;

protected:

    //объект не создается, но его можно наследовать
    ThingFactory(){}
};

//
// класс кот анализирует строку и создает предметы
//
class ThingScriptParser{
public:

    //
    // класс - получатель предметов 
    //
    class Acceptor{
    public:
        
        virtual ~Acceptor(){}
        
        // принять предмет
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // возврат:
        //            true  - предмет принят
        //            false - предмет не принят
        //
        virtual bool AcceptThing(BaseThing* thing) = 0;

        //принять информцию об организации
        virtual void AcceptOrgInfo(const rid_t rid) = 0;
        //принять информацию о существах
        virtual void AcceptEntInfo(entity_type type, const rid_t& rid) = 0;
        //принять информацию о предметах
        virtual void AcceptThingInfo(thing_type type, const rid_t& rid) = 0;
    };

    ThingScriptParser(Acceptor* acceptor = 0, ThingFactory* factory = 0);
    ~ThingScriptParser();

    //узнать последнего символа
    int GetLastPos();
    //установить фабрику предметов
    void SetFactory(ThingFactory* factory);
    //установить получателя предметов
    void SetAcceptor(Acceptor* acceptor);

    // разобрать строку, передать предметы получателю
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:        
    //          true  - не было ошибок
    //          false - были ошибки 
    // NOTE:
    //       если указатель на текущий Acceptor == 0,  
    //       то происходит только проверка правильности 
    //       скрипта без порождения предметов
    //
    bool Parse(const std::string& script);

private:

    enum token_type{
        T_KEY,          // key
        T_MONEY,        // money
        T_IMPLANT,      // implant
        T_CAMERA,       // camera
        T_SHIELD,       // shield
        T_WEAPON,       // weapon
        T_AMMO,         // ammo
        T_ARMOR,        // armor
        T_SCANNER,      // scanner
        T_GRENADE,      // grenade
        T_MEDIKIT,      // medic

        T_INFO,         // info
        T_HUMAN,        // human
        T_TRADER,       // trader
        T_VEHICLE,      // vehicle
        T_ORGANIZATION, // organization

        T_LBRACKET,     // (
        T_RBRACKET,     // )
        T_COMMA,        // ,

        T_NUMBER,
        T_IDENT,
        T_END, 
    };

    token_type GetToken(std::string* str, int* num); 

    BaseThing* ParseKey();
    BaseThing* ParseAmmo();
    BaseThing* ParseMoney();
    BaseThing* ParseArmor();
    BaseThing* ParseWeapon();
    BaseThing* ParseCamera();
    BaseThing* ParseShield();
    BaseThing* ParseScanner();
    BaseThing* ParseImplant();
    BaseThing* ParseGrenade();
    BaseThing* ParseMedikit();

    void ParseInfo();

private:

    class LexerError{};
    class ParseError{};

    Acceptor*     m_acceptor;
    ThingFactory* m_factory; 

    std::istringstream m_input; 
};

//
// разбор скрипта для генерации набора предметов
//
class ItemSetParser{
public:

    //
    // класс - получатель предметов 
    //
    class Acceptor{
    public:
        
        virtual ~Acceptor(){}
        
        // принять предмет
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // возврат:
        //            true  - предмет принят
        //            false - предмет не принят
        //
        virtual void Accept(const rid_t& item_set) = 0;
    };
    
    ItemSetParser(Acceptor* acceptor = 0);

    // разобрать строку, передать набор в парсер набора
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:        
    //          true  - не было ошибок
    //          false - были ошибки 
    // NOTE:
    //       если указатель на текущий ThingScriptParser == 0,  
    //       то происходит только проверка правильности скрипта 
    //       без порождения предметов
    //
    bool Parse(const std::string& script);
    
    //узнать позицию последнего разобранного символа
    int GetLastPos();
    //установить парсер набора предметов
    void SetAcceptor(Acceptor* acceptor);
    
private:
    
    enum token_type{
        
        T_LBRACKET, //(
        T_RBRACKET, //)
            
        T_RND,      //~  выбрать случайным образом набор
        T_ADD,      //+  сложить наборы в один 
            
        T_NULL,     //none нулевой набор
            
        T_NUMBER,
        T_IDENT,
        T_END,
    };
    
    typedef std::vector<rid_t> rid_vec_t;
    
    struct info_s{
        std::string m_ident;
        int         m_number;
    };
    
    token_type GetToken(info_s* info);
    
    struct node_s{
        token_type m_type;
        rid_vec_t  m_rids;
        
        std::vector<node_s> m_nodes;
    };
    
    void ParseBlock(node_s* node);
    void SpawnThings(node_s& node);
    void SpawnThings(const rid_t& rid);

private:
        
    class LexerErr{};
    class ParseErr{};
        
    std::istringstream m_input;
    Acceptor*          m_acceptor; 
};

//
// приемщик информации
//
class InfoAcceptor{
public:

    virtual ~InfoAcceptor(){}

    //принять информцию об организации
    virtual void AcceptOrganization(const rid_t rid) = 0;
    //принять информацию о предметах
    virtual void AcceptThing(thing_type type, const rid_t& rid) = 0;
    //принять информацию о существах
    virtual void AcceptEntity(entity_type type, const rid_t& rid) = 0;
};

namespace AIUtils{

    //прочитать строку из item_sets.xls   
    bool GetItemSetStr(const rid_t& rid, std::string* set_str, int* line);
}

#endif // _PUNCH_THINGFACTORY_H_