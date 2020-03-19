//
// ������� ��� ����������� ���������/����������� ���������
//

#ifndef _PUNCH_VISUTILS3_H_
#define _PUNCH_VISUTILS3_H_

class BaseThing;
class BaseEntity;

class Marker;
class Spectator;
class MSRelation;

enum vis_flags{
    VT_NONE      = 0,
    VT_INVISIBLE = 1 << 0,  //��������
    VT_VISIBLE   = 1 << 1,  //�������
    VT_HIGHLIGHT = 1 << 2,  //���������    
};

typedef unsigned mid_t; //��������: ���� ������ �� ����� �� mid_t != 0
typedef unsigned sid_t; //��������: ���� ����������� �� ����� �� sid_t != 0

//
// ����������� �� ��������
//
class MarkerObserver : public Observer<Marker*>{
public:

    enum event_type{
        EV_NONE,
        EV_VISIBLE_FOR_SPECTATOR,   //��� ������� �����������
        EV_INVISIBLE_FOR_SPECTATOR, //�� ����� �� ���� ������ �����������
    };

    struct spectator_info{
        Spectator* m_spectator;
        spectator_info(Spectator* sp) : m_spectator(sp) {}
    };
};

//
// ������ - ���������� ������� ��������
//
class Marker{
public:

    Marker() : m_mid(0), m_counter(0){}
    virtual ~Marker() {}

    DCTOR_ABS_DEF(Marker)

    //��������� / ��������� �����������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //�������� ������ �� ��������
    virtual BaseEntity* GetEntity() {return 0;}

    //�������� ������ ����� ����������� ������
    virtual void GetPoints(pnt3_vec_t* points) = 0;

    //��������/�������� ������
    virtual void Highlight(bool flag);
    //������� �� ������ 
    virtual bool IsHighlighted() const = 0;

    //�������� ��� ���������� ��������� �������
    void ZeroRequests();
    bool HaveRequests() const;
    void AddRequest(unsigned type);

    //��������/���������� �������������
    void  SetMID(mid_t mid) { m_mid = mid;  }
    mid_t GetMID() const    { return m_mid; }

    //������ � �������������
    void Detach(MarkerObserver* observer);
    void Attach(MarkerObserver* observer, MarkerObserver::event_t event);
    void Notify(MarkerObserver::event_t event, MarkerObserver::info_t info = 0);

private:

    mid_t m_mid;
    int   m_counter;

    typedef ObserverManager<MarkerObserver> obs_mgr_t;
    obs_mgr_t m_observers;
};

//
// ����������� �� Spectator'��
//
class SpectatorObserver : public Observer<Spectator*>{
public:

    enum event_type{
        EV_NONE,
        EV_VISIBLE_MARKER,   //������ ����� � ���� ���������
        EV_INVISIBLE_MARKER, //������ ����� �� ���� ���������
    };

    //���������� � ��������/�������� ��������
    struct marker_info{
        Marker* m_marker;
        marker_info(Marker* mk) : m_marker(mk) {}
    };
    
    //void Update(Spectator* spectator, event_type event, info_s* info) = 0;
};

//
// ����������� - ���������� �������� - �����������
//
class Spectator{
public:

    Spectator() : m_sid(0) {}
    virtual ~Spectator(){}

    DCTOR_ABS_DEF(Spectator)

    //���������� ��� �������./������. �����������
    virtual void OnInsert() {}
    virtual void OnRemove() {}

    //��������� / ��������� �����������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //��������� ��������� ��� �������
    virtual unsigned CalcVisibility(Marker* marker) = 0;
    //��������� ��������� ��� �����
    virtual unsigned CalcVisibility(const point3& pnt) = 0;
    //������� ����� ����� �������� � ������������
    virtual MSRelation* CreateRelation(Marker* marker) = 0;

    //�������� ������ �� ��������
    virtual BaseEntity* GetEntity() { return 0; }
    //������ ��� ������ - ����������� (����� ������ ��� �����!)
    virtual player_type GetPlayer() const { return PT_NONE; }

    //��������/���������� �������������
    void  SetSID(sid_t sid) { m_sid = sid; }
    sid_t GetSID() const { return m_sid; }

    //������ � �������������
    void Detach(SpectatorObserver* observer);
    void Attach(SpectatorObserver* observer, SpectatorObserver::event_t event);
    void Notify(SpectatorObserver::event_t event, SpectatorObserver::info_t info = 0);

private:

    sid_t m_sid;

    typedef ObserverManager<SpectatorObserver> obs_mgr_t;
    obs_mgr_t m_observers;
};

//
// ��������� ����� �������� � ������������
//
class MSRelation{
public:

    MSRelation() : m_flags(VT_NONE){}
    virtual ~MSRelation(){}

    //������ ��������� ������ ������� ���������
    unsigned GetFlags() const { return m_flags; }

    //�������� ��������� 
    virtual void Update(Marker* marker, Spectator* spect, int event) = 0;

protected: 

    //���������� ����� � ����������� ������
    void SwitchMarker(unsigned flags, Marker* marker, Spectator* spect);

private:

    unsigned m_flags;
};

// 
// ����� ���������
//
class VisMap{
public:

    static VisMap* GetInst();

    ~VisMap();

    //����� �����
    void Reset();

    //��������/������� ������
    void Insert(Marker* marker);
    void Remove(Marker* marker);

    //��������/������� �����������
    void Insert(Spectator* spectator);
    void Remove(Spectator* spectator);

    enum update_event{
        UE_NONE,
        
        UE_POS_CHANGE,    //��������� �������
        UE_FOS_CHANGE,    //��������� ���� ���������
        
        UE_INS_MARKER,    //���������� ������ �������
        UE_DEL_MARKER,    //�������� �������
        
        UE_INS_SPECTATOR, //���������� �����������
        UE_DEL_SPECTATOR, //�������� �����������
        
        UE_MARKER_ON,     //������ ���������
        UE_MARKER_OFF,    //������ ����������
    };

    //�������� ��� ��������� ����� ������������ � ��������
    unsigned GetVisFlags(Spectator* spectator, Marker* marker);

    //����������� ��������� ��� ��������� �������
    void Update(Marker* marker, update_event event);
    //����������� ��������� ��� ��������� �����������
    void Update(Spectator* spectator, update_event event);

    //�������� ��������� ������������ �� ��������
    class SpectatorItor;
    typedef SpectatorItor spectator_itor;

    spectator_itor spectator_begin(Marker* marker, unsigned type);
    spectator_itor spectator_end();

    //�������� ��������� �������� ��� �����������
    class MarkerItor;
    typedef MarkerItor marker_itor;

    marker_itor marker_begin(Spectator* spectator, unsigned type);
    marker_itor marker_end();

private:

    VisMap();

    //����������� ������ � ���� ����� ������� ���������
    void SwitchMarker(Marker* marker);
    //��������� ������� ��������� �� �����. �������� 
    void ExpandRelationsTbl(const ipnt2_t& old_size, const ipnt2_t& new_size);

private:

    friend class MarkerItor;
    friend class SpectatorItor;

    typedef std::vector<Marker*> mark_vec_t;
    typedef std::vector<Spectator*> spect_vec_t;

    mark_vec_t   m_markers;
    spect_vec_t  m_spectators;

    MSRelation** m_relations;
};

//
// �������� �� �������� ��� �������
//
class VisMap::MarkerItor{
public:

    MarkerItor(sid_t sid = 0, unsigned type = VT_NONE, size_t first = 0) : 
        m_sid(sid), m_vis(type), m_vis_map(VisMap::GetInst()), m_first(first)
    {  if(sid && !IsSuitable(m_vis_map->m_markers[m_first])) operator++(); }

    MarkerItor& operator ++();
    MarkerItor operator ++(int)
    { MarkerItor tmp = *this; operator++(); return tmp; }
    
    //��������� ��� �������� ������
    Marker* operator ->(){ return m_vis_map->m_markers[m_first]; }
    Marker& operator * (){ return *(m_vis_map->m_markers[m_first]); }
    
    //��������� �� !=
    friend bool operator != (const MarkerItor& i1, const MarkerItor& i2)
    { return i1.m_first != i2.m_first; } 

private:

    bool IsSuitable(Marker* marker)
    {
        if(marker == 0) return false;
        
        size_t index = marker->GetMID() + m_vis_map->m_markers.size() * m_sid;
        return m_vis_map->m_relations[index] && m_vis_map->m_relations[index]->GetFlags() & m_vis;
    }

private:

    sid_t    m_sid;
    unsigned m_vis;

    size_t   m_first;
    VisMap*  m_vis_map;
};

//
// �������� �� ������������ ��� �������
//
class VisMap::SpectatorItor{
public:

    SpectatorItor(mid_t mid = 0, unsigned type = VT_NONE, size_t first = 0) : 
        m_mid(mid), m_vis(type), m_vis_map(VisMap::GetInst()), m_first(first)
    {  if(mid && !IsSuitable(m_vis_map->m_spectators[m_first])) operator++(); }

    SpectatorItor& operator ++();
    SpectatorItor  operator ++(int)
    { SpectatorItor tmp = *this; operator++(); return tmp; }
    
    //��������� ��� �������� ������
    Spectator* operator ->(){ return m_vis_map->m_spectators[m_first]; }
    Spectator& operator * (){ return *(m_vis_map->m_spectators[m_first]); }
    
    //��������� �� !=
    friend bool operator != (const SpectatorItor& i1, const SpectatorItor& i2)
    { return i1.m_first != i2.m_first; } 

private:

    bool IsSuitable(Spectator* spectator)
    {
        if(spectator == 0) return false;
        
        size_t index = m_mid + m_vis_map->m_markers.size() * spectator->GetSID();
        return m_vis_map->m_relations[index] && m_vis_map->m_relations[index]->GetFlags() & m_vis;
    }

private:

    mid_t    m_mid;
    unsigned m_vis;

    size_t   m_first;
    VisMap*  m_vis_map;
};

#endif // _PUNCH_VISUTILS3_H_
