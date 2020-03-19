//
//  Утилиты для работы с путем
//

#ifndef _PUNCH_PATHUTILS_H_
#define _PUNCH_PATHUTILS_H_

class BusRoute;

class BaseEntity;
class VehicleEntity;

//
// класс утилит для расчета пути
//
class PathUtils{
public:

    ~PathUtils();

    //singleton
    static PathUtils* GetInst();

    //поставить существо на сетку
    void LinkEntity(BaseEntity* entity);
    //снять существо с сетки
    void UnlinkEntity(BaseEntity* entity);

    enum flag_type{
        F_CALC_NEAR_PNTS      = 1 << 0, //считать точки подхода к существам
        F_CALC_OVER_INVISIBLE = 1 << 1, //считать путь поверх невидимых существ
        F_PASS_ALL_ENTITIES   = 1 << 2, //обсчитывать путь вокруг всех существ   

        //наборы флагов для разных расчетов
        F_PLAYER_CALC  = F_CALC_NEAR_PNTS|F_CALC_OVER_INVISIBLE,                 
        F_DEFAULT_CALC = F_PLAYER_CALC,
    };

    // расчитать поле проходимости для существа
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // front - помещается граница до кот. можно дойти
    // pass  - помещаются все точки в кот можно пройти 
    //
    void CalcPassField(BaseEntity* entity, pnt_vec_t* front = 0, pnt_vec_t* pass = 0, unsigned flags = F_DEFAULT_CALC);

    // расчитать поле проходимости под взрыв гранаты
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // pass  - помещаются все точки в кот можно пройти 
    //
    void CalcPassField(const ipnt2_t& center, float radius, pnt_vec_t* pass = 0);

    //расчитать поле высадки для людей
    void CalcLandField(VehicleEntity* vehicle, pnt_vec_t* front = 0);
    //расчитать массив hex'ов занимаемых существом
    void CalcTrackField(BaseEntity* entity, ipnt2_t pos, pnt_vec_t* vec);

    //найти путь
    void CalcPath(BaseEntity* entity, ipnt2_t to, pnt_vec_t& pnts);
  
    //можно ли поставить существо в эту позицию
    bool IsEmptyPlace(BaseEntity* entity, const ipnt2_t& pos);
    
    //структура для расчета точки подхода
    struct near_pnt_t{
        int     m_cost;
        ipnt2_t m_pnt;

        bool IsDefPnt() const;
    };

    //узнать существует ли точка подхода к существу
    bool IsNear(BaseEntity* entity) const; 
    //получить параметры точки подхода
    const near_pnt_t& GetNearPnt(BaseEntity* entity) const;

    //узнать параметры точки подхода к нек. множеству hex'ов
    const near_pnt_t& GetNearPnt(const pnt_vec_t& vec) const;

protected:

    PathUtils();

private:

    //пометить поля выхода
    void FindLandPnts(ipnt2_t* first, ipnt2_t* last, pnt_vec_t* front);

private:

    //массив точек подхода
    near_pnt_t  m_near_pnts[MAX_ENTITIES];
};

//
// класс - поддержка автобусных маршрутов на уровне
//
class BusDispatcher{
public:
        
    static BusDispatcher* GetInst();

    //уведомление о переходе на уровень
    void Reset();

    //зарегистрировать маршрут
    void Insert(VehicleEntity* bus, BusRoute* route);
    //снять маршрут с уровня
    void Remove(VehicleEntity* bus, BusRoute* route);

private:

    BusDispatcher();

    //обойти весь маршрут автобуса и применить операцию
    template<class Entity, class Route, class Operation>
    void BusRouteWalker(Entity* entity, Route* route, Operation oper)
    {
        BusRoute::pos_s pos;
        
        route->GenLoc(true, &pos);

        pnt_vec_t path;

        while(route->GetPntType(pos.m_pos) != Route::PT_LAST){
            
            path.clear();
            
            route->GetPath(pos.m_pos, &path);

            if(path.empty()) return;
            
            pos.m_pos = path[0];
            
            for(size_t k = 0; k < path.size(); k++){
                oper.SetPos(path[k]);
                entity->Accept(oper);
            }
        }
    }

private:

    eid_t m_buses[MAX_BUSES];
};

//
// механизм просмотра существ в нек области
//
class AreaManager{
public:

    AreaManager(const pnt_vec_t& vec);
    ~AreaManager();

    // 
    // итератор по существам в области
    // 
    class Iterator{
    public:

        Iterator(size_t first = 0, unsigned etype = 0, unsigned ptype = 0,
                 unsigned attrs = 0, pnt_vec_t* area = 0);
        
        Iterator& operator ++();
        Iterator  operator ++(int) 
        { Iterator tmp = *this;  operator++(); return tmp;}
        
        //операторы доступа к существам
        BaseEntity* operator ->();
        BaseEntity& operator * (){ return *operator->();}
        
        //сравнения на !=
        friend bool operator != (const Iterator& i1, const Iterator& i2)
        { return i1.m_first != i2.m_first; }
        
        friend bool operator == (const Iterator& i1, const Iterator& i2)
        { return i1.m_first == i2.m_first; }

    private:

        bool IsSuitable(const ipnt2_t hex_pnt);

    private:
        
        typedef std::set<eid_t> eid_set_t;

        unsigned m_etype;
        unsigned m_ptype;
        unsigned m_attrs;

        size_t m_first;

        eid_set_t  m_eids;
        pnt_vec_t* m_area;
    }

    typedef iterator;

    iterator begin(unsigned entity = ET_ALL_ENTS, unsigned player = PT_ALL_PLAYERS, unsigned attrs = EA_ALL_ATTRIBUTES);
    iterator end();

private:

    pnt_vec_t m_area;
};

#endif // _PUNCH_PATHUTILS_H_