//
// ����� ��� ����������� ����
//

#ifndef _PUNCH_TRACEUTILS_H_
#define _PUNCH_TRACEUTILS_H_
#include "../skin/keyanimation.h"

//��������: ��� ���������� �.�.���� ����� ���������� �������
typedef unsigned eid_t;

//������� ����� ��� ���� ��������������
class BaseTracer{
};

//
// ����������� ��� ��������
//
class ShotTracer: public BaseTracer{
public:
  //������������� �����
  enum flags
    {
    F_VISROOFS            = 0x01, //����� ��������� ���� ���������
    F_SKIP_INVISIBLE_ENTS = 0x02, //��������� �������� �� ��������� � �����������
    F_SKIP_SHIELDS        = 0x04, //�������������� ���� �� ��������� � ����������� 
    F_ENTITIES_AS_BOX     = 0x08, //�������������� ���� �� ��������� � ����������� 
    F_SIGHT               = 0x10, //���������� ����
    };

    //��� ��������� � ���. ����� "���������" ���
    enum material_type{
        MT_NONE,
        MT_AIR,
        MT_WALL,
        MT_ENTITY,
        MT_OBJECT,
        MT_SHIELD,
    };

    //����������� �� ���������, �� ����� �� ������������
    ShotTracer(){}    

    //�����������, ��� ������ �������������� ����� ������,
    //�������� Calc ����
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // NOTE: radius == 0 ��������, ��� ������ ����������
    //
    ShotTracer(eid_t ent, const point3& org, const point3& dir, float radius = 0, unsigned flags=0) : 
        Ray(org,Normalize(dir)),Radius(radius),VisRoofs((flags & F_VISROOFS) != 0),
        id(ent),m_fskip_invisible_ents((flags & F_SKIP_INVISIBLE_ENTS) != 0),
        m_skip_shields((flags&F_SKIP_SHIELDS) != 0),m_ent_as_box((flags&F_ENTITIES_AS_BOX) != 0),
        m_sight((flags&F_SIGHT) != 0)         
          {Calc();}

    ~ShotTracer(){}

    //���������� ��� �������
    void SetEnt(eid_t ent) {id=ent;}
    void SetDir(const point3& dir) {Ray.Direction=dir;Ray.Update();}
    void SetOrg(const point3& org) {Ray.Origin=org;Ray.Update();}
    void SetRadius(float radius) {Radius=radius;}

    //���������� ������ ����
    virtual void Calc();

    //
    //�������� ���������� ���������� �������
    //

    //����� ��������� ����
    point3 GetEnd() const {return Touch.TouchPoint;}      
    point3 GetEndNorm() const {return Touch.TouchNorm;}      
    //id ��������, ���� �������� ���, �� 0
    unsigned GetEntity() const {return Touch.Ent;}
    //��� ��������� ���������� �������
    material_type GetMaterial() const {return Touch.Mat;}     
    //������� �������, ���� ������� ���, ������ ������
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
  
   eid_t    id;    //��� ����� ����������
   ray      Ray;    //������������ ���
   float    Radius; //��������� �����������
   TouchData Touch; //���� �� ������
   bool VisRoofs;    //���� ��, �� ����� ��������� ���� ���������
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
// ����������� ����� ���������
//
class LOSTracer: public BaseTracer{
public:

    //����������� �� ���������, �� ����� �� ������������
  LOSTracer():SightClear(false),sid(0),did(0){}    

    //�����������, ��� ������ �������������� ����� ������
    //�������� CalcLOS �����, �� �������� �������
	//���� ����������� ����������� ��������� � ������� - �.�. ent2==0
    LOSTracer(  eid_t ent1, const point3& pos1, 
                eid_t ent2, const point3& pos2)
		{
			SetFirst(ent1,pos1);
			SetSecond(ent2,pos2);
			CalcLOS();
		}
		//��� �������� ����������� ��������� � ������
    LOSTracer(  eid_t ent1, const point3& pos1, 
			const std::string &ObjName, const point3& pos2)
		{
			SetFirst(ent1,pos1);
			SetSecond(ObjName,pos2);
			CalcLOS();
		}

    ~LOSTracer(){}
    
    //���������� ��� �������
    void SetFirst(eid_t ent_id, const point3& pos)
      {sid=ent_id; Source=pos;}
    void SetSecond(eid_t ent_id, const point3& pos)
      {did=ent_id; Destination=pos;}
    void SetSecond(const std::string &ObjName, const point3& pos)
      {m_ObjName=ObjName; Destination=pos;}

    //���� ����������� ����� ���������
    bool CalcLOS();

    //������� ��������� ���������� �������
    //��� false ���� ������� �� ����
    bool HaveLOS() const {return SightClear;}
    operator bool(){return HaveLOS();}
    
private:
    point3 Source,Destination; //�����, ���������� ���
    eid_t  sid,did;          //������ ��������� � ����������
		std::string m_ObjName;
    bool   SightClear;       //
};

#endif // _PUNCH_TRACEUTILS_H_