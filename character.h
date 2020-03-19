#ifndef __IENTITY_H_
#define __IENTITY_H_
//абстрактный интерфейс для персонажей (людей и техники)
//этот интерфейс реализует графическую оболочку этих сущностей
class Person;
class Vehicle;
class GraphPipe;
#include "world.h"
enum ANIMATION{LAZY=0x01, STAY=0x02, HURT=0x04,WALK=0x08, TURN=0x10,
               SHOOT=0x20,SHOOTRECOIL=0x40, DEAD=0x80, RUN=0x100,
              FALL=0x200, USE=0x400, KNOCKOUT=0x800, RELOAD=0x1000,
              AIMSHOOT=0x2000, AIMSHOOTRECOIL=0x4000,PANIC=0x8000,
              SPECANIM=0x10000,};
class IMovingCallBack;
class IEntity
  { 
  public:
		enum OWNERSHIP {OS_PLAYER, OS_CIVILIAN, OS_OTHER};
		typedef std::vector<point3> points_t;

    virtual void SetLocation(const point3 &Pos, float Angle)=0;
    virtual void Draw()=0;
    virtual float SetAngle(float CurTime, float Angle)=0;
    virtual point3 GetLocation() const =0;
    virtual float SetAction(unsigned Action,float CurTime)=0;
    virtual void GetBarrel(const point3 &Offs, point3 *pos)=0;
    virtual void SetDestination(const point3  &d)=0;
    virtual void StartMoving(ANIMATION Type, const std::vector<ipnt2_t> &hexes, IMovingCallBack *MoveCB)=0;
    virtual void UpdateOnTime(float Time)=0;
    virtual float GetAngle()=0;
    virtual bool TraceRay(const ray &r, float *Dist, point3 *Norm,bool AsBox=false)=0;
    virtual bool TraceSegment(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm)=0;
    virtual point3 GetShotPoint(point3 suppose_point=NULLVEC)=0;
    virtual void GetLOSPoints(points_t *points) const=0;//ВНИМАНИЕ: массив не очищается внутри функции
		virtual void SetOwnership(OWNERSHIP ownby) =0;
    virtual ~IEntity(){};
  public:
    virtual void Enable(bool)=0;
    virtual void SetBanner(const std::string &banner,unsigned int Color)=0;
    virtual void DrawBanner()=0;
    virtual void DrawFOS()=0;
		virtual bool NeedRotation(const point3& Dir)=0;
    enum EFFECT_TYPE{
        ET_FLAME, 
        ET_SHOCK, 
        ET_ELECTRIC,
        ET_SELECTION,
        ET_SIZE
    };

    enum SHOW_MODE{
        SM_SHOW,      //показать эффект
        SM_HIDE,      //спрятать эффект
        SM_SLOW_HIDE, //медленно спрятать эффект
    };

    virtual void ShowEffect(EFFECT_TYPE type, SHOW_MODE mode)=0;
    virtual void SetFOS(float front_radius, float back_radius, float sight_angle)=0;
  public://Преобразование интерфейсов
    virtual Person  *GetHuman() {return NULL;};
    virtual Vehicle *GetVehicle() {return NULL;};
		virtual void MakeSaveLoad(SavSlot &sl)=0;
  };
struct hit_s;
class LogicDamageStrategy;
class CharacterPool
  {
  public:
    typedef unsigned Handle; //так и только так можно ссылаться на элементы
  public:
    enum DECOR_TYPE {DT_FOS,DT_BANNER};
    void Delete(Handle h);
    void Clear();
    IEntity *Get(Handle Which){return Pool[Which];};
    CharacterPool();
    ~CharacterPool();
    Handle CreateHuman();
    Handle CreateVehicle();
    void Draw();
    void Update(float Time);
    void ShowDecor(bool Show, DECOR_TYPE dt);
    void MakeSaveLoad(Storage &St);
		void CleanByAction(ANIMATION Animation); 
    bool TraceSegment(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm,Handle *Who);
		void DoExplosion(const point3& Pos,float Radius,const point3 &Dir,
		               const IExplosionAPI::Damage dam[3],
									 std::string ObjName,
									 IExplosionAPI *API);
    void DoExplosion(int  entity, const hit_s& hit, LogicDamageStrategy *LogicDamage);

  private: 
		enum entity_type{ET_NONE,ET_PERSON,ET_VEHICLE};
    std::vector<IEntity *> Pool;
    bool BannerVis;
    bool FOSVis;
  };
class ScatteredItem;
class ItemsPool
  {
  public:
    typedef unsigned  Handle; //так и только так можно ссылаться на элементы
  public:
    void Clear();
    void Delete(Handle h);
    Handle Create(const std::string &Name, const point3 &Pos);
    ScatteredItem *Get(Handle Which){return Pool[Which];};
    ItemsPool(){};
    ~ItemsPool();
    void Draw();
    void MakeSaveLoad(SavSlot &Slot);
  private:
    std::vector<ScatteredItem *> Pool;
  };

/*
void RealEntity::StartMoving(RUN||WALK, vector[hex], IMovingCallBack *MoveCB);
*/
class IMovingCallBack
  {
  public:

    enum COMMAND {CMD_STOP, CMD_CONTINUE, CMD_SPEED_UP};
    enum EVENT   {EV_STEP, EV_END};

    virtual COMMAND VisitHex(EVENT event)=0;
  };



#endif