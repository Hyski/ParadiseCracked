#ifndef _MyCulling
#define _MyCulling

#include "../3d/quat.h"

struct Plane
  {
  enum {X=0x01,Y=0x02,Z=0x04};
  point3 Normal; //нормаль к плоскости
  float D;       //параметр (расстояние от 0,0,0 до плоскости)
  unsigned flag; //флаги плоскости
  int Octant;    //октант, в котором находится нормаль
  Plane(){};
  void Update()
    {
    flag=X|Y|Z;
    if(FastAbs(Normal.x)<epsilon)
      flag&=~X;
    if(FastAbs(Normal.y)<epsilon)
      flag&=~Y;
    if(FastAbs(Normal.z)<epsilon)
      flag&=~Z;

    Octant=0;
    if(Normal.x<0) Octant|=0x01;
    if(Normal.y<0) Octant|=0x02;
    if(Normal.z<0) Octant|=0x04;
    }
  Plane(const point3 &norm)
    {
    Normal=Normalize(norm);D=0;
    Update();
    }
  Plane(const point3 &norm,const point3 &dot)
    {
    Normal=Normalize(norm);D=-Normal.Dot(dot);
    Update();
    }
  Plane(const point3 &a1,const point3 &a2,const point3 &a3)
    {
    Normal=Normalize((a3-a1).Cross(a2-a1));   D=-Normal.Dot(a1);
    Update();
    }
  float TestPoint(const point3 &dot) const
    { 
    float ret=D;
    if(flag&X)
      ret+=Normal.x*dot.x;
    if(flag&Y)
      ret+=Normal.y*dot.y;
    if(flag&Z)
      ret+=Normal.z*dot.z;
    return ret;
    }
  float TestPointEps(const point3 &dot) const
    { 
    float ret;
    ret=Normal.Dot(dot)+D;
    if(FastAbs(ret)<epsilon) ret=0.f;
        return ret;
    }

  bool CutSegment(const point3 &A, const point3 &B, point3 *ret) const
    {
    float a,b;
    a=TestPoint(A);
    if(a==0.f) {return false;}
    b=TestPoint(B);
    if(b==0.f){return false;}
    float length=fabs(a)+fabs(b);
    *ret=(fabs(a)/length)*B + (fabs(b)/length)*A;
    return true;
    }
  bool operator ==(const Plane &p) const
    {
    if(FastAbs(D-p.D)>epsilon)return false;
    if((Normal-p.Normal).SqrLength()>epsilon) return false;
    return true;
    }
  Plane operator -() const
    {
     Plane p;
     p.Normal=-Normal; p.D=-D;
     return p;
    }
  bool RayIntersect(const ray &r,float *Dist)
    {
    float e=r.Direction.Dot(Normal);
    if(e==0) return false;
    *Dist=(-D-Normal.Dot(r.Origin))/e;
    return true;
    }

  };
//////////////////////////////////////////////////////////////////////////////
struct Triangle
  {
  point3 V[3]; //вершины
  point3 N[3]; //нормали вершин
  point3 Normal; //нормаль к плоскости, образованной треугольником
  bool Roof;
  texcoord UV[3];
  int MatID;//номер в списке шейдеров
  Triangle(const Triangle &a);
  Triangle &operator =(const Triangle &a);
  Triangle(){};
  bool RayTriangle(const point3 &orig, const point3 &dir,float *t, float *u, float *v);

  float Area();
  bool Degenerate()
    {
    //if(Area()<0.01) return true;
    //return false;
    if((V[0]-V[1]).SqrLength()<epsilon)return true;
    if((V[0]-V[2]).SqrLength()<epsilon)return true;
    if((V[2]-V[1]).SqrLength()<epsilon)return true;
    return false;
    }

  };
inline float Triangle::Area()
  {
  float a=(V[0]-V[1]).Length();
  float b=(V[0]-V[2]).Length();
  float c=(V[2]-V[1]).Length();
  float p=a+b+c;
  return sqrt(p*(p-a)*(p-b)*(p-c));
  }
inline Triangle &Triangle::operator =(const Triangle &a)
    {
    //memcpy(this,&a,sizeof(Triangle));
    static const int _size=sizeof(Triangle)/4;
    _asm{
      mov edi,this
        mov esi,a
        mov ecx, _size
        rep movsd
      }
    return *this;
    };
inline Triangle::Triangle(const Triangle &a)
    {
    //memcpy(this,&a,sizeof(Triangle));
    *this=a;
    };
//////////////////////////////////////////////////////////////////////////////
struct BSphere
  {
  point3 Center;
  float Radius;
  };
class Frustum
  {
  public:
    enum  {VISIBLE,NOTVISIBLE,PARTVISIBLE};
    Plane Planes[6]; //far,right,left,bottom,up,near;
    point3 Points[5]; //вершины неусеченной пирамиды
                      //позиция камеры, lu,ru,rd,ld
    Frustum(){};
    int TestBBox(const BBox &box) const ;//0=снаружи 1=внутри -1=частично внутри
    int TestBSphere(const BSphere &sphere)const ;//0=снаружи 1=внутри -1=частично внутри
    int TestPoint(const point3 &dot)const ;//0=снаружи 1=внутри -1=частично внутри
    int WhichSide(const Plane *p)const 
      {
      unsigned flags=0;
      unsigned mask=0;
      for(int i=0;i<5;i++)
        {
        mask<<=1;
        mask|=1;
        if(p->TestPoint(Points[i])>=0)
          flags|=(1<<i);
        if(flags!=0&&flags!=mask) return PARTVISIBLE;
        }
      if(flags==mask) return VISIBLE;
      return NOTVISIBLE;
      };
  };
class Cylinder//класс для цилиндра, параллельного оси Z
  {
  public:
    Cylinder(const Cylinder &a){*this=a;};
    Cylinder(const point3 &Base, float Height, float Radius):m_Base(Base),m_Height(Height), m_Radius(Radius){};
  public:
    bool TraceRay(const ray &r, float *Dist, point3 *Norm);
    bool TraceSegment(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm);


  protected:
    point3 m_Base; //координаты центра нижнего основания
    float m_Height;//высота
    float m_Radius;//радиус
  };



LevelVertex Slerp(const LevelVertex *a,const LevelVertex *b,float alpha);

#endif