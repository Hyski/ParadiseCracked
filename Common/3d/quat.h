// MAGIC Software
// http://www.cs.unc.edu/~eberly
//
// This free software is supplied under the following terms:
// 1. You may distribute the original source code to others at no charge.
// 2. You may modify the original source code and distribute it to others at
//    no charge.  The modified code must be documented to indicate that it is
//    not part of the original package.
// 3. You may use this code for non-commercial purposes.  You may also
//    incorporate this code into commercial packages.  However, you may not
//    sell any of your source code which contains my original and/or modified
//    source code (see items 1 and 2).  In such a case, you would need to
//    factor out my code and freely distribute it.
// 4. The original code comes with absolutely no warranty and no guarantee is
//    made that the code is bug-free.

#ifndef QUAT_H
#define QUAT_H
#include <math.h>

// уберем ненужные Warnings
#pragma warning (disable:4244)
#pragma warning (disable:4305)

class point3
{
public:
    // point or vector in 3-space
    float x, y, z;

    point3 () {}

    point3 (float _x, float _y, float _z)
    {         x = _x; y = _y; z = _z;    }

    point3 (const point3& rkPoint)
    {        x = rkPoint.x; y = rkPoint.y; z = rkPoint.z;    }

    point3& operator= (const point3& rkPoint)
    {  x = rkPoint.x;   y = rkPoint.y;    z = rkPoint.z;  return *this;    }

    bool operator== (const point3& rkPoint) const
    { return (x == rkPoint.x && y == rkPoint.y && z == rkPoint.z);  }

    bool operator!= (const point3& rkPoint) const
    { return (x != rkPoint.x || y != rkPoint.y || z != rkPoint.z);  }

    point3 operator+ (const point3& rkPoint) const
    {   return point3(x+rkPoint.x,y+rkPoint.y,z+rkPoint.z);   }
    point3 &operator+= (const point3& rkPoint)
    {  x+=rkPoint.x;y+=rkPoint.y;z+=rkPoint.z;return *this;  }
    point3 &operator-= (const point3& rkPoint)
    {   x-=rkPoint.x;y-=rkPoint.y;z-=rkPoint.z;return *this;  }

    point3 operator- (const point3& rkPoint) const
    {   return point3(x-rkPoint.x,y-rkPoint.y,z-rkPoint.z);}
    point3 operator- () const
    {   return point3(-x,-y,-z);}

    point3 operator* (float dCoeff) {   return point3(dCoeff*x,dCoeff*y,dCoeff*z);}
    point3& operator*= (float dCoeff) {  x*=dCoeff;y*=dCoeff;z*=dCoeff;return *this;}
    point3 operator/ (float dCoeff) const
    {  
    float r=1.f/dCoeff;
    return point3(x*r,y*r,z*r);}

    friend point3 operator* (float dCoeff, const point3& rkPoint)
    {  return point3(dCoeff*rkPoint.x,dCoeff*rkPoint.y,dCoeff*rkPoint.z);}

    float Dot (const point3& rkPoint) const
    {  return x*rkPoint.x + y*rkPoint.y + z*rkPoint.z;}

    point3 Cross (const point3& rkPoint) const
    {
        return point3(
            y*rkPoint.z - z*rkPoint.y,
            z*rkPoint.x - x*rkPoint.z,
            x*rkPoint.y - y*rkPoint.x );
    }

    float Length () const { return sqrt(x*x+y*y+z*z); }
    float SqrLength () const{ return (x*x+y*y+z*z); }
    friend point3 Normalize(point3 a);
    friend bool operator <(const point3 &a,const point3&b);
    friend void Interpolate(point3 *ret, const point3 &From, const point3 &To,float alpha);
};
inline void Interpolate(point3 *ret, const point3 &From, const point3 &To,float alpha)
  {
  register float ialpha=1-alpha;
  ret->x=From.x*alpha+To.x*ialpha;
  ret->y=From.y*alpha+To.y*ialpha;
  ret->z=From.z*alpha+To.z*ialpha;
  }
inline bool operator <(const point3 &a,const point3&b)
  {
  if(a.x<b.x) return true;
  else if(a.x>b.x) return false;
  if(a.y<b.y) return true;
  else if(a.y>b.y) return false;
  if(a.z<b.z) return true;
  else return false;
  }
inline point3 Normalize(point3 a)
  {
  float rlen=1.f/sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
  return point3(a.x*rlen,a.y*rlen,a.z*rlen);
  }

class Quaternion
{
public:
    float w, x, y, z;

    // construction and destruction
    Quaternion () {}
    Quaternion (float _w, float _x, float _y, float _z);
    Quaternion (const Quaternion& q);

    // conversion between quaternions, matrices, and angle-axes
    void FromRotationMatrix (const float R[3][3]);
    void ToRotationMatrix (float R[3][3]) const;
    void ToRotationMatrix (float R[4][4]) const;
    void FromAngleAxis (const float& angle, const float& ax,
        const float& ay, const float& az);
    void FromAngleAxis (const float& rdAngle, const point3& rkPoint);
    void ToAngleAxis (float& angle, float& ax, float& ay, float& az) const;
    void ToAngleAxis (float& rkAngle, point3& rkPoint) const;

    // arithmetic operations
    Quaternion& operator= (const Quaternion& q);
    Quaternion operator+ (const Quaternion& q) const;
    Quaternion operator- (const Quaternion& q) const;
    Quaternion operator* (const Quaternion& q) const;
    Quaternion operator* (float c) const;
    friend Quaternion operator* (float c, const Quaternion& q);
    Quaternion operator- () const;

    // functions of a quaternion
    float Dot (const Quaternion& q) const;  // dot product of quaternions
    float Norm () const;  // squared-length of quaternion
    Quaternion Inverse () const;  // apply to non-zero quaternion
    Quaternion UnitInverse () const;  // apply to unit-length quaternion
    Quaternion Exp () const;
    Quaternion Log () const;

    // rotation of a point by a quaternion
    point3 operator* (const point3& pt) const;

    // spherical linear interpolation
    static Quaternion Slerp (float t, const Quaternion& p,
        const Quaternion& q);

    static Quaternion SlerpExtraSpins (float t, const Quaternion& p,
        const Quaternion& q, int iExtraSpins);

    // setup for spherical quadratic interpolation
    static void Intermediate (const Quaternion& q0, const Quaternion& q1,
        const Quaternion& q2, Quaternion& a, Quaternion& b);

    // spherical quadratic interpolation
    static Quaternion Squad (float t, const Quaternion& p,
        const Quaternion& a, const Quaternion& b, const Quaternion& q);

    static Quaternion ZERO;
    static Quaternion IDENTITY;
};

#endif

