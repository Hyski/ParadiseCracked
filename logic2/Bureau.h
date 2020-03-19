//
// классы для работы с ящиками на уровне
//

#ifndef _PUNCH_BUREAU_H_
#define _PUNCH_BUREAU_H_

class BoxInfo;
class DataBox;
class StoreBox;
class BaseThing;

//
// посетитель для ящика
//
class BoxVisitor{
public:

    virtual ~BoxVisitor(){}

    //посетить ящик с данными
    virtual void Visit(DataBox* box) = 0;
    //посетить ящик с предметами
    virtual void Visit(StoreBox* box) = 0;
};

//
// базовый ящик на уровне
//
class BaseBox{
public:

    BaseBox(const rid_t& rid);
    virtual ~BaseBox();

    DCTOR_ABS_DEF(BaseBox)

    //получить строковое id ящика
    const rid_t& GetRID() const;

    //получить указатель на соотв. инерфейс
    virtual DataBox* Cast2Info();
    virtual StoreBox* Cast2Store();
    
    //принять посетителя для соотв ящика
    virtual void Accept(BoxVisitor& visitor) = 0;   
    //сохранение/загрузка
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

protected:

    const BoxInfo* GetInfo() const;
    
private:

    BoxInfo* m_info;
};

//
// ящик - хранилище информации
//
class DataBox : public BaseBox{
public:

    DataBox(const rid_t& rid = rid_t());
    ~DataBox();
    
    DCTOR_DEF(DataBox)

    //вернуть указатель на интерфейс
    DataBox* Cast2Info();
    //принять посетителя для соотв ящика
    void Accept(BoxVisitor& visitor);   
    //сохранение/загрузка
    void MakeSaveLoad(SavSlot& slot);

    int GetExperience() const;

    const rid_t& GetScriptRID() const;
};

//
// интерфейс ящика для хранения предметов
//
class StoreBox : public BaseBox{
public:

    StoreBox(const rid_t& rid = rid_t());
    ~StoreBox();

    DCTOR_DEF(StoreBox)

    //вернуть указатель на интерфейс
    StoreBox* Cast2Store();
    //принять посетителя для соотв ящика
    void Accept(BoxVisitor& visitor);   
    //сохранение/загрузка
    void MakeSaveLoad(SavSlot& slot);

    class Iterator;
    typedef Iterator iterator;
    
    iterator end();
    iterator begin(unsigned type = TT_ALL_ITEMS); 

    tid_t Insert(BaseThing* thing);
    void Remove(iterator&  itor);
    void Remove(BaseThing* thing);

    int GetRespawn() const;   
    int GetExperience() const;

    float GetWeight() const;

    const rid_t& GetScriptRID() const;

private:

    friend class Iterator;

    typedef std::vector<BaseThing*> pack_t;
    pack_t  m_pack;
};

//
// итератор для прохода по предметам в ящике
//
class StoreBox::Iterator{
public:

    Iterator(StoreBox::pack_t* pack = 0, int first = 0, unsigned mask = 0);

    //итерация
    Iterator& operator ++();
    Iterator  operator ++(int)
    { Iterator tmp = *this; operator++(); return tmp;}
    
    //операторы для удобства работы
    BaseThing* operator ->(){return (*m_pack)[m_first];}
    BaseThing& operator * (){return *(*m_pack)[m_first];}
    
    //сравнение на !=
    friend bool operator != (const Iterator& i1, const Iterator& i2)
    { return i1.m_first != i2.m_first; }

private:

    bool IsSuitable(const BaseThing* thing) const;

private:

    size_t    m_first;       
    unsigned  m_mask; 
    
    StoreBox::pack_t* m_pack;
};

//
//  бюро - в нем находятся все ящики на уровне
//
class Bureau{
public:

    ~Bureau();

    //singleton
    static Bureau* GetInst();

    //получить указатель на ящик или 0
    BaseBox* Get(const rid_t& rid);
  
    //уничтожить внутренние структуры
    void Reset();
    //сохранение / загрузка
    void MakeSaveLoad(SavSlot& slot);

    //добавить ящик в список
    void Insert(BaseBox* box);
    //обработать загрузку уровня
    void HandleLevelEntry();
    //обработать уничтожение ящика
    void HandleObjectDamage(const rid_t& rid);

protected:

    Bureau();
    
private:

    typedef std::vector<BaseBox*> box_vec_t;
    box_vec_t m_boxes;

    bool m_fcan_spawn;
};

#endif // _PUNCH_BUREAU_H_