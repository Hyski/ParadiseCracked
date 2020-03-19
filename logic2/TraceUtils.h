//
// класс для трассировки луча
//

#ifndef _PUNCH_TRACEUTILS_H_
#define _PUNCH_TRACEUTILS_H_
#include "../skin/keyanimation.h"

//ВНИМАНИЕ: это необходимо т.к.этим кодом пользуются эффекты
typedef unsigned eid_t;

//базовый класс для всех трассировщиков
class BaseTracer{
};

//
// трассировка для выстрела
//
class ShotTracer: public BaseTracer{
public:
  //разнообразные флаги
  enum flags
    {
    F_VISROOFS            = 0x01, //крыши прозрачны если отключены
    F_SKIP_INVISIBLE_ENTS = 0x02, //невидимые существа не участвуют в трассировке
    F_SKIP_SHIELDS        = 0x04, //энергетические щиты не участвуют в трассировке 
    F_ENTITIES_AS_BOX     = 0x08, //энергетические щиты не участвуют в трассировке 
    F_SIGHT               = 0x10, //трассируем взор
    };

    //тип материала в кот. может "упереться" луч
    enum material_type{
        MT_NONE,
        MT_AIR,
        MT_WALL,
        MT_ENTITY,
        MT_OBJECT,
        MT_SHIELD,
    };

    //конструктор по умолчанию, им лучше не пользоваться
    ShotTracer(){}    

    //конструктор, кот должен использоваться почти всегда,
    //вызывает Calc явно
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // NOTE: radius == 0 означает, что радиус бесконечен
    //
    ShotTracer(eid_t ent, const point3& org, const point3& dir, float radius = 0, unsigned flags=0) : 
        Ray(org,Normalize(dir)),Radius(radius),VisRoofs((flags & F_VISROOFS) != 0),
        id(ent),m_fskip_invisible_ents((flags & F_SKIP_INVISIBLE_ENTS) != 0),
        m_skip_shields((flags&F_SKIP_SHIELDS) != 0),m_ent_as_box((flags&F_ENTITIES_AS_BOX) != 0),
        m_sight((flags&F_SIGHT) != 0)         
          {Calc();}

    ~ShotTracer(){}

    //установить все ручками
    void SetEnt(eid_t ent) {id=ent;}
    void SetDir(const point3& dir) {Ray.Direction=dir;Ray.Update();}
    void SetOrg(const point3& org) {Ray.Origin=org;Ray.Update();}
    void SetRadius(float radius) {Radius=radius;}

    //произвести расчет явно
    virtual void Calc();

    //
    //получить результаты последнего расчета
    //

    //точка окончания луча
    point3 GetEnd() const {return Touch.TouchPoint;}      
    point3 GetEndNorm() const {return Touch.TouchNorm;}      
    //id существа, если существа нет, то 0
    unsigned GetEntity() const {return Touch.Ent;}
    //тип материала последнего расчета
    material_type GetMaterial() const {return Touch.Mat;}     
    //навание объекта, если объекта нет, пустая строка
    std::string GetObject() const {return Touch.ObjName;}

protected:
  struct TouchData
    {
    point3   TouchPoint;
    point3   TouchNorm;
    material_type Mat;
    std::string ObjName;
    eid_t Ent;
    };
  
   eid_t    id;    //это нужно пропустить
   ray      Ray;    //трассируемый луч
   float    Radius; //дальность трасстровки
   TouchData Touch; //куда мы попали
   bool VisRoofs;    //если да, то крыши прозрачны если отключены
   bool m_fskip_invisible_ents;
   bool m_skip_shields;
   bool m_ent_as_box;
   bool m_sight;
};
class _GrenadeTracer:public ShotTracer
{
protected:
	KeyAnimation ka;
	unsigned SkipEnt;
	float m_Time;
public:
	_GrenadeTracer(unsigned _SkipEnt, const point3& org, const point3& dir, float radius = 0, unsigned flags=0,float max_time=10.f)
		:ShotTracer(_SkipEnt,org,dir,radius,flags),SkipEnt(_SkipEnt),m_Time(max_time)
	{Calc();}
	void Calc();
	const KeyAnimation& GetTrace(){return ka;};
	
};

//
// трассировка линии видимости
//
class LOSTracer: public BaseTracer{
public:

    //конструктор по умолчанию, им лучше не пользоваться
  LOSTracer():SightClear(false),sid(0),did(0){}    

    //конструктор, кот должен использоваться почти всегда
    //вызывает CalcLOS сразу, по созданию объекта
	//если проверяется возможность попадания в уровень - д.б. ent2==0
    LOSTracer(  eid_t ent1, const point3& pos1, 
                eid_t ent2, const point3& pos2)
		{
			SetFirst(ent1,pos1);
			SetSecond(ent2,pos2);
			CalcLOS();
		}
		//для проверки возможности попадания в объект
    LOSTracer(  eid_t ent1, const point3& pos1, 
			const std::string &ObjName, const point3& pos2)
		{
			SetFirst(ent1,pos1);
			SetSecond(ObjName,pos2);
			CalcLOS();
		}

    ~LOSTracer(){}
    
    //установить все ручками
    void SetFirst(eid_t ent_id, const point3& pos)
      {sid=ent_id; Source=pos;}
    void SetSecond(eid_t ent_id, const point3& pos)
      {did=ent_id; Destination=pos;}
    void SetSecond(const std::string &ObjName, const point3& pos)
      {m_ObjName=ObjName; Destination=pos;}

    //явно пересчитать линию видимости
    bool CalcLOS();

    //вернуть результат последнего расчета
    //или false если расчета не было
    bool HaveLOS() const {return SightClear;}
    operator bool(){return HaveLOS();}
    
private:
    point3 Source,Destination; //точки, образующие луч
    eid_t  sid,did;          //номера источника и назначения
		std::string m_ObjName;
    bool   SightClear;       //
};

#endif // _PUNCH_TRACEUTILS_H_