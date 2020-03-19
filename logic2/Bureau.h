//
// ������ ��� ������ � ������� �� ������
//

#ifndef _PUNCH_BUREAU_H_
#define _PUNCH_BUREAU_H_

class BoxInfo;
class DataBox;
class StoreBox;
class BaseThing;

//
// ���������� ��� �����
//
class BoxVisitor{
public:

    virtual ~BoxVisitor(){}

    //�������� ���� � �������
    virtual void Visit(DataBox* box) = 0;
    //�������� ���� � ����������
    virtual void Visit(StoreBox* box) = 0;
};

//
// ������� ���� �� ������
//
class BaseBox{
public:

    BaseBox(const rid_t& rid);
    virtual ~BaseBox();

    DCTOR_ABS_DEF(BaseBox)

    //�������� ��������� id �����
    const rid_t& GetRID() const;

    //�������� ��������� �� �����. ��������
    virtual DataBox* Cast2Info();
    virtual StoreBox* Cast2Store();
    
    //������� ���������� ��� ����� �����
    virtual void Accept(BoxVisitor& visitor) = 0;   
    //����������/��������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

protected:

    const BoxInfo* GetInfo() const;
    
private:

    BoxInfo* m_info;
};

//
// ���� - ��������� ����������
//
class DataBox : public BaseBox{
public:

    DataBox(const rid_t& rid = rid_t());
    ~DataBox();
    
    DCTOR_DEF(DataBox)

    //������� ��������� �� ���������
    DataBox* Cast2Info();
    //������� ���������� ��� ����� �����
    void Accept(BoxVisitor& visitor);   
    //����������/��������
    void MakeSaveLoad(SavSlot& slot);

    int GetExperience() const;

    const rid_t& GetScriptRID() const;
};

//
// ��������� ����� ��� �������� ���������
//
class StoreBox : public BaseBox{
public:

    StoreBox(const rid_t& rid = rid_t());
    ~StoreBox();

    DCTOR_DEF(StoreBox)

    //������� ��������� �� ���������
    StoreBox* Cast2Store();
    //������� ���������� ��� ����� �����
    void Accept(BoxVisitor& visitor);   
    //����������/��������
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
// �������� ��� ������� �� ��������� � �����
//
class StoreBox::Iterator{
public:

    Iterator(StoreBox::pack_t* pack = 0, int first = 0, unsigned mask = 0);

    //��������
    Iterator& operator ++();
    Iterator  operator ++(int)
    { Iterator tmp = *this; operator++(); return tmp;}
    
    //��������� ��� �������� ������
    BaseThing* operator ->(){return (*m_pack)[m_first];}
    BaseThing& operator * (){return *(*m_pack)[m_first];}
    
    //��������� �� !=
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
//  ���� - � ��� ��������� ��� ����� �� ������
//
class Bureau{
public:

    ~Bureau();

    //singleton
    static Bureau* GetInst();

    //�������� ��������� �� ���� ��� 0
    BaseBox* Get(const rid_t& rid);
  
    //���������� ���������� ���������
    void Reset();
    //���������� / ��������
    void MakeSaveLoad(SavSlot& slot);

    //�������� ���� � ������
    void Insert(BaseBox* box);
    //���������� �������� ������
    void HandleLevelEntry();
    //���������� ����������� �����
    void HandleObjectDamage(const rid_t& rid);

protected:

    Bureau();
    
private:

    typedef std::vector<BaseBox*> box_vec_t;
    box_vec_t m_boxes;

    bool m_fcan_spawn;
};

#endif // _PUNCH_BUREAU_H_