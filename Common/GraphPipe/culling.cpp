//#include "stdafx.h"
#include "precomp.h"
#include <float.h>
#include "culling.h"
int Frustum::TestBBox(const BBox &box) const //0=снаружи 1=внутри -1=частично внутри
  {
  float D1,D2;//d1 - проекция на нормаль меньше чем у d2
  static point3 dot1,dot2;
  unsigned flags=0;
  for(int i=0;i<6;i++)
    {
    switch (Planes[i].Octant)
      {
      case 0:  // x>0 y>0 z>0
        dot1.x=box.minx;dot1.y=box.miny;dot1.z=box.minz;
        dot2.x=box.maxx;dot2.y=box.maxy;dot2.z=box.maxz;
        break;
      case 1:  // x<0 y>0 z>0
        dot1.x=box.maxx;dot1.y=box.miny;dot1.z=box.minz;
        dot2.x=box.minx;dot2.y=box.maxy;dot2.z=box.maxz;
        break;
      case 2:  // x>0 y<0 z>0
        dot1.x=box.minx;dot1.y=box.maxy;dot1.z=box.minz;
        dot2.x=box.maxx;dot2.y=box.miny;dot2.z=box.maxz;
        break;
      case 3:  // x<0 y<0 z>0
        dot1.x=box.maxx;dot1.y=box.maxy;dot1.z=box.minz;
        dot2.x=box.minx;dot2.y=box.miny;dot2.z=box.maxz;
        break;
      case 4:  // x>0 y>0 z<0
        dot1.x=box.minx;dot1.y=box.miny;dot1.z=box.maxz;
        dot2.x=box.maxx;dot2.y=box.maxy;dot2.z=box.minz;
        break;
      case 5:  // x<0 y>0 z<0
        dot1.x=box.maxx;dot1.y=box.miny;dot1.z=box.maxz;
        dot2.x=box.minx;dot2.y=box.maxy;dot2.z=box.minz;
        break;
      case 6:  // x>0 y<0 z<0
        dot1.x=box.minx;dot1.y=box.maxy;dot1.z=box.maxz;
        dot2.x=box.maxx;dot2.y=box.miny;dot2.z=box.minz;
        break;
      case 7:  // x<0 y<0 z<0
        dot1.x=box.maxx;dot1.y=box.maxy;dot1.z=box.maxz;
        dot2.x=box.minx;dot2.y=box.miny;dot2.z=box.minz;
        break;
      }
        D2=Planes[i].TestPoint(dot2);
        D1=Planes[i].TestPoint(dot1);
        if(D2<0)
          return NOTVISIBLE;
        if(D1>=0) flags|=(1<<i);//полностью на положительной стороне
    }
  if(flags==0x3f) return VISIBLE;
  return PARTVISIBLE;
  }
int Frustum::TestBSphere(const BSphere &sphere) const //0=снаружи 1=внутри -1=частично внутри
  {
  float r1,r2,r3,r4,r5,r0;
  r0=Planes[0].TestPoint(sphere.Center);
  r1=Planes[1].TestPoint(sphere.Center);
  r2=Planes[2].TestPoint(sphere.Center);
  r3=Planes[3].TestPoint(sphere.Center);
  r4=Planes[4].TestPoint(sphere.Center);
  r5=Planes[5].TestPoint(sphere.Center);
  float rad=sphere.Radius,mrad=-rad;
  if((r0<mrad)||(r1<mrad)||(r2<mrad)||
     (r3<mrad)||(r4<mrad)||(r5<mrad))
    return NOTVISIBLE;
  if((FastAbs(r0)<rad)||(FastAbs(r1)<rad)||(FastAbs(r2)<rad)||
     (FastAbs(r3)<rad)||(FastAbs(r4)<rad)||(FastAbs(r5)<rad))
    return PARTVISIBLE;
  return VISIBLE;
  }
int Frustum::TestPoint(const point3 &dot) const //0=снаружи 1=внутри
  {
  if(Planes[0].TestPoint(dot)<0.f) return NOTVISIBLE;
  if(Planes[1].TestPoint(dot)<0.f) return NOTVISIBLE;
  if(Planes[2].TestPoint(dot)<0.f) return NOTVISIBLE;
  if(Planes[3].TestPoint(dot)<0.f) return NOTVISIBLE;
  if(Planes[4].TestPoint(dot)<0.f) return NOTVISIBLE;
  if(Planes[5].TestPoint(dot)<0.f) return NOTVISIBLE;
  return VISIBLE;
  }

bool Triangle::RayTriangle(const point3 &orig, const point3 &dir, 
                           float *t, float *u, float *v)
  {
  point3 edge1, edge2, tvec, pvec, qvec;
  float det,inv_det;
  
  /* find vectors for two edges sharing vert0 */
  edge1=V[1]-V[0];
  edge2=V[2]-V[0];
  
  /* begin calculating determinant - also used to calculate U parameter */
  pvec=dir.Cross(edge2);
  /* if determinant is near zero, ray lies in plane of triangle */
  det = edge1.Dot(pvec);
  
  if (det > -epsilon && det < epsilon)
    return false;
  inv_det = 1.0 / det;
  
  /* calculate distance from vert0 to ray origin */
  tvec=orig-V[0];
  
  /* calculate U parameter and test bounds */
  *u = tvec.Dot(pvec) * inv_det;
  if (*u < 0.0 || *u > 1.0)
    return false;
  
  /* prepare to test V parameter */
  qvec=tvec.Cross(edge1);
  
  /* calculate V parameter and test bounds */
  *v = dir.Dot(qvec) * inv_det;
  if (*v < 0.0 || *u + *v > 1.0)
    return false;
  
  /* calculate t, ray intersects triangle */
  *t = edge2.Dot(qvec) * inv_det;
  return true;
  }
LevelVertex Slerp(const LevelVertex *a,const LevelVertex *b,float alpha)
  {
  static Quaternion q1,q2,qr;
  LevelVertex vert;
  vert.Pos=(1.f-alpha)*a->Pos + (alpha)*b->Pos;
  vert.uv.u=(1.f-alpha)*a->uv.u + (alpha)*b->uv.u;
  vert.uv.v=(1.f-alpha)*a->uv.v + (alpha)*b->uv.v;
  vert.lightmapuv.u=(1.f-alpha)*a->lightmapuv.u + (alpha)*b->lightmapuv.u;
  vert.lightmapuv.v=(1.f-alpha)*a->lightmapuv.v + (alpha)*b->lightmapuv.v;
  
  

  q1.FromAngleAxis(0,a->Normal);
  q2.FromAngleAxis(0,b->Normal);
  qr=q1.Slerp(alpha,q1,q2);
  qr.ToAngleAxis(alpha,vert.Normal);
  vert.Normal=Normalize(vert.Normal);
  return vert;
  }
bool Cylinder::TraceRay(const ray &Ray, float *Dist, point3 *Norm)
  {
    point3 DirN=Normalize(Ray.Direction); 
    point3 D=DirN;D.z=0;
    point3 P0=Ray.Origin;P0.z=0;
    float Dlen=D.Length();

      point3 pos1=m_Base;pos1.z=0;
      float tPer=D.Dot(pos1-P0);
      float dist=(pos1-(P0+D*tPer)).Length();
      if(dist>m_Radius) return false;
      
      float q=sqrt(m_Radius*m_Radius-dist*dist);
      point3 p1;
      
      p1=Ray.Origin+DirN*(tPer/Dlen-q);
      if(tPer/Dlen-q>0&&p1.z>m_Base.z&&p1.z<m_Base.z+m_Height)
        {
        *Dist=(tPer/Dlen-q)/Ray.Direction.Length();
        *Norm=Normalize(p1-m_Base);
        return true;
      }
      p1=Ray.Origin+DirN*(tPer/Dlen+q);
      if(tPer/Dlen+q>0&&p1.z>m_Base.z&&p1.z<m_Base.z+m_Height)
        {
        *Dist=(tPer/Dlen+q)/Ray.Direction.Length();
        *Norm=Normalize(p1-m_Base);
        return true;
      }
      return false;
  }
bool Cylinder::TraceSegment(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm)
{
	static const float BOGUS=100000;
	point3 norm,pnt;
	float d;

	point3 mid_pnt=From+0.5*Dir;
	float Len=Dir.Length()/2; //половина длины сегмента
	
  if(hypot(mid_pnt.x-m_Base.x,mid_pnt.y-m_Base.y)>Len+m_Radius) return false;

	if(TraceRay(ray(From,Dir),&d,&norm))
		{
    if(d<=1.f)
      {
				*Norm=norm;
				*Res=From+d*Dir;
        return true;
      }
		}

	return false;
}

