//
// —етка hex'ов экспортированна€ из Max'а
//

#ifndef _PUNCH_HEX_GRID_
#define _PUNCH_HEX_GRID_

class RiskSite;
class BaseEntity;

//
// Ќех'вое поле
//
class HexGrid{
public:

    //singleton
    static HexGrid* GetInst(){return m_instance;}

    //срез дл€ индексации сетки
    class hg_slice{
    public:

        size_t operator()(int x, int y) const;
        size_t operator()(const ipnt2_t pnt) const;
        const ipnt2_t& off2pnt(size_t off);

    private:
        friend class HexGrid;
        hg_slice(size_t x) : m_sizeX(x){}
        
        size_t  m_sizeX;
        ipnt2_t m_pnt;
    };

    //€чейка сетки
    class cell: public point3{
    public:

        cell();
        ~cell();

        enum cost_type{
            NONE  = -1,
            LAND  = -2,
        };
       
        //узнать св€занное свойство
        int GetProperty() const{ return m_prop; }

        //работа со стоимостью пути
        int  GetCost() const   { return m_cost; }
        void SetCost(int cost) { m_cost = cost; }

        //тип точки в сетке
        bool IsDefPnt() const  { return m_cost == NONE; }
        bool IsLandPnt() const { return m_cost == LAND; }
  
        //здесь есть кто нибудь из существ
        BaseEntity* GetEntity() const  { return m_entity; }

        //установить/сн€ть существо с €чейки пол€
        void UnlinkEntity() { m_entity = 0;}
        void LinkEntity(BaseEntity* ent){ m_entity = ent; }

        //проходима ли €чейка в напр dir
        bool IsPassable(int dir) const { return m_weight[dir] < 80; }
        //узнать стоимость прохода в опред направлении
        int GetWeight(int dir) const { return m_weight[dir]; }
        //установить стоимость прохода в опред направлении
        void SetWeight(int dir, int val) { m_weight[dir] = val; }

        //отметить точку
        void LandMark() { m_cost = LAND; }
        void Clear()    { m_cost = NONE; }

        //
        //операции с автобусными маршрутами
        //
        
        //точка входит в маршрут автобуса?
        bool IsBusRoute() const { return m_buses != 0; }

        //пометить/разметить маршрут
        void MarkBusRoute(unsigned route_id)   { m_buses |= (1 << route_id); }
        void UnmarkBusRoute(unsigned route_id) { m_buses &= ~(1 << route_id); }

        //
        // работа с земл€ными эффектами
        //

        //получить ссылку на RiskSite
        RiskSite* GetRisk() { return m_risk; }
        //установить ссылку на RiskSite
        void SetRisk(RiskSite* risk) { m_risk = risk; }

    private:

        friend class HexGrid;
      
        int m_cost;      // дл€ расчета стоимости переходов
        int m_prop;      // id свойств hex'са
        int m_weight[6]; // проходимость

        RiskSite*   m_risk;   //кусочек опасной зоны  
        BaseEntity* m_entity; //существо наход€щеес€ в этом hex'ce

        unsigned  m_buses;    //маршруты автобусов пролегающих через эту точку
    };

    //свойство €чеки
    struct prop {
        std::string  m_zone;    //название зоны
        unsigned     m_joints;  //битовые флаги зо переходов

        bool DynMan,DynHeight;
        std::string ManObj;//ќбъект, от которого зависит проходимость
        bool ManInv;       //проходимость зависит инверсно
        std::string HeightObj;//ќбъект, от которого зависит ¬ысота
        prop():m_joints(0),DynMan(false),DynHeight(false){}
    };

    virtual ~HexGrid();
    
    //попадает ли точка в поле
    bool IsOutOfRange(const ipnt2_t& pnt) const;

    //размерность сетки
    int GetSizeX() const;
    int GetSizeY() const;
    float GetHexSize() const;

    //получить средство дл€ индексации массива
    hg_slice GetSlice() const;

    //средства доступа к массиву €чеек
    typedef cell* cell_iterator;
    typedef prop* prop_iterator;

    typedef const cell* const_cell_iterator;
    typedef const prop* const_prop_iterator;

    //дл€ организации быстрого доступа к €чейкам
    cell_iterator first_cell() const;
    cell_iterator last_cell() const;
    prop_iterator first_prop() const;
    prop_iterator last_prop() const;

    //медленный, но удобный доступ к €чейке
    cell& Get(const ipnt2_t& pnt);    
    const prop& GetProp(const ipnt2_t& pnt);

    //управление зонами выхода
    void SetActiveJoints(unsigned val);
    unsigned GetActiveJoints() const;

protected:

    HexGrid(const char* name);

private:

    void ReadStr(std::istringstream &stream, std::string& str);
    
private:

    int      m_SizeX, m_SizeY;   
    int      m_props_num;
    float    m_HexSize;
    unsigned m_active_joints;

    cell*    m_grid;
    prop*    m_props;  

    static HexGrid* m_instance;
    
public: //Grom
  //вспомогательный данные дл€ быстрого изменени€ проходимости..
  struct heightfront
    {  //точка и направление на/с объета с динамически измен€емой высотой
    cell *from;
    cell *to;
    int dir;
    heightfront(cell *a,cell *b, int d):from(a),to(b), dir(d){;}
    heightfront():from(NULL),to(NULL),dir(0){;}
    };
  typedef std::vector<ipnt2_t> CellVec;
  typedef std::vector<heightfront> HFrontVec;
  std::map<std::string,CellVec> ManDirect;    //Objstate==0 - непроходимо
  std::map<std::string,CellVec> ManInvert;    //Objstate==1 - непроходимо
  std::map<std::string,CellVec> HeightDirect; //объекты, от которых зависит высота
  std::map<std::string,HFrontVec>  HeightFront; //объекты, от которых зависит высота
};

//
//~~~~~~~~~~~~~~~~~~~~~~~~inlines~~~~~~~~~~~~~~~~~~~~~~~~
//

inline unsigned HexGrid::GetActiveJoints() const
{
    return m_active_joints;
}

inline HexGrid::prop_iterator HexGrid::first_prop() const
{
    return m_props;
}

inline HexGrid::prop_iterator HexGrid::last_prop() const
{
    return m_props + m_props_num;
}

inline HexGrid::cell_iterator HexGrid::first_cell() const 
{
    return m_grid;
}

inline HexGrid::cell_iterator HexGrid::last_cell() const 
{
    return m_grid + m_SizeX*m_SizeY;
}

inline int HexGrid::GetSizeX() const
{
    return m_SizeX;
}

inline int HexGrid::GetSizeY() const
{
    return m_SizeY;
}

inline float HexGrid::GetHexSize() const
{
    return m_HexSize;
}

inline HexGrid::cell& HexGrid::Get(const ipnt2_t& pnt)
{
    return *(first_cell() + GetSlice()(pnt));
}

inline const HexGrid::prop& HexGrid::GetProp(const ipnt2_t& pnt)
{
    return *(first_prop() + Get(pnt).m_prop);
}

inline bool HexGrid::IsOutOfRange(const ipnt2_t& pnt) const
{   
    return pnt.x < 0 || pnt.x > (m_SizeX - 1) || pnt.y < 0 || pnt.y > (m_SizeY - 1);
}

inline size_t HexGrid::hg_slice::operator()(const ipnt2_t pnt) const
{
    return pnt.x + pnt.y*m_sizeX; 
}

inline size_t HexGrid::hg_slice::operator()(int x, int y) const
{
    return x + y*m_sizeX; 
}

inline const ipnt2_t& HexGrid::hg_slice::off2pnt(size_t off)
{
    m_pnt.x = off%m_sizeX;
    m_pnt.y = off/m_sizeX;
    return m_pnt;
}

inline HexGrid::hg_slice HexGrid::GetSlice() const
{
    return hg_slice(m_SizeX);
}

#endif //_PUNCH_HEX_GRID_