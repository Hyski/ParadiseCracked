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

#include "precomp.h"
#include "quat.h"

static float g_dEpsilon = 1e-03;  // cutoff for sin(angle) near zero
static float g_dPi = 4.0*atan(1.0);

Quaternion Quaternion::ZERO(0,0,0,0);
Quaternion Quaternion::IDENTITY(1,0,0,0);

//---------------------------------------------------------------------------
Quaternion::Quaternion (float _w, float _x, float _y, float _z)
{
    w = _w;
    x = _x;
    y = _y;
    z = _z;
}
//---------------------------------------------------------------------------
Quaternion::Quaternion (const Quaternion& q)
{
    w = q.w;
    x = q.x;
    y = q.y;
    z = q.z;
}
//---------------------------------------------------------------------------
void Quaternion::FromRotationMatrix (const float R[3][3])
{
    // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternion Calculus and Fast Animation".

    float trace = R[0][0]+R[1][1]+R[2][2];
    float root;

    if ( trace > 0.0 )
    {
        // |w| > 1/2, may as well choose w > 1/2
        root = sqrt(trace+1.0);  // 2w
        w = 0.5*root;
        root = 0.5/root;  // 1/(4w)
        x = (R[2][1]-R[1][2])*root;
        y = (R[0][2]-R[2][0])*root;
        z = (R[1][0]-R[0][1])*root;
    }
    else
    {
        // |w| <= 1/2
        static int next[3] = { 1, 2, 0 };
        int i = 0;
        if ( R[1][1] > R[0][0] )
            i = 1;
        if ( R[2][2] > R[i][i] )
            i = 2;
        int j = next[i];
        int k = next[j];

        root = sqrt(R[i][i]-R[j][j]-R[k][k]+1.0);
        float* quat[3] = { &x, &y, &z };
        *quat[i] = 0.5*root;
        root = 0.5/root;
        w = (R[k][j]-R[j][k])*root;
        *quat[j] = (R[j][i]+R[i][j])*root;
        *quat[k] = (R[k][i]+R[i][k])*root;
    }
}
//---------------------------------------------------------------------------
void Quaternion::ToRotationMatrix (float R[4][4]) const
{
    float tx  = 2.0*x;
    float ty  = 2.0*y;
    float tz  = 2.0*z;
    float twx = tx*w;
    float twy = ty*w;
    float twz = tz*w;
    float txx = tx*x;
    float txy = ty*x;
    float txz = tz*x;
    float tyy = ty*y;
    float tyz = tz*y;
    float tzz = tz*z;

    R[0][0] = 1.0-(tyy+tzz);
    R[0][1] = txy-twz;
    R[0][2] = txz+twy;
    R[1][0] = txy+twz;
    R[1][1] = 1.0-(txx+tzz);
    R[1][2] = tyz-twx;
    R[2][0] = txz-twy;
    R[2][1] = tyz+twx;
    R[2][2] = 1.0-(txx+tyy);
}
void Quaternion::ToRotationMatrix (float R[3][3]) const
{
    float tx  = 2.0*x;
    float ty  = 2.0*y;
    float tz  = 2.0*z;
    float twx = tx*w;
    float twy = ty*w;
    float twz = tz*w;
    float txx = tx*x;
    float txy = ty*x;
    float txz = tz*x;
    float tyy = ty*y;
    float tyz = tz*y;
    float tzz = tz*z;

    R[0][0] = 1.0-(tyy+tzz);
    R[0][1] = txy-twz;
    R[0][2] = txz+twy;
    R[1][0] = txy+twz;
    R[1][1] = 1.0-(txx+tzz);
    R[1][2] = tyz-twx;
    R[2][0] = txz-twy;
    R[2][1] = tyz+twx;
    R[2][2] = 1.0-(txx+tyy);
}
//---------------------------------------------------------------------------
void Quaternion::FromAngleAxis (const float& angle, const float& ax,
                                const float& ay, const float& az)
{
    // assert:  axis[] is unit length
    //
	// The quaternion representing the rotation is
	//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

    float halfAngle = 0.5*angle;
    float sn = sin(halfAngle);
    w = cos(halfAngle);
    x = sn*ax;
    y = sn*ay;
    z = sn*az;
}
//---------------------------------------------------------------------------
void Quaternion::FromAngleAxis (const float& rdAngle, const point3& rkPoint)
{
    FromAngleAxis(rdAngle,rkPoint.x,rkPoint.y,rkPoint.z);
}
//---------------------------------------------------------------------------
void Quaternion::ToAngleAxis (float& angle, float& ax, float& ay,
                              float& az) const
{
	// The quaternion representing the rotation is
	//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

    float length2 = x*x+y*y+z*z;
    if ( length2 > 0.0 )
    {
        angle = 2.0*acos(w);
        float invlen = 1.0/sqrt(length2);
        ax = x*invlen;
        ay = y*invlen;
        az = z*invlen;
    }
    else
    {
        // angle is 0 (mod 2*pi), so any axis will do
        angle = 0;
        ax = 1.0;
        ay = 0.0;
        az = 0.0;
    }
}
//---------------------------------------------------------------------------
void Quaternion::ToAngleAxis (float& rdAngle, point3& rkPoint) const
{
    ToAngleAxis(rdAngle,rkPoint.x,rkPoint.y,rkPoint.z);
}
//---------------------------------------------------------------------------
Quaternion& Quaternion::operator= (const Quaternion& q)
{
    w = q.w;
    x = q.x;
    y = q.y;
    z = q.z;

    return *this;
}
//---------------------------------------------------------------------------
Quaternion Quaternion::operator+ (const Quaternion& q) const
{
    return Quaternion(w+q.w,x+q.x,y+q.y,z+q.z);
}
//---------------------------------------------------------------------------
Quaternion Quaternion::operator- (const Quaternion& q) const
{
    return Quaternion(w-q.w,x-q.x,y-q.y,z-q.z);
}
//---------------------------------------------------------------------------
Quaternion Quaternion::operator* (const Quaternion& q) const
{
    // NOTE:  Multiplication is not generally commutative, so in most
    // cases p*q != q*p.

#if 1
    return Quaternion
    (
		w*q.w-x*q.x-y*q.y-z*q.z,
		w*q.x+x*q.w+y*q.z-z*q.y,
		w*q.y+y*q.w+z*q.x-x*q.z,
		w*q.z+z*q.w+x*q.y-y*q.x
    );
#else
    // The above code uses 16 multiplications and 12 additions.  The code
    // below uses 12 multiplications (you might be able to avoid the
    // divisions by 2 and manipulate the floating point exponent directly)
    // and 28 additions.  For an architecture where multiplications are more
    // expensive than additions, you should do the cycle count and consider
    // using the code below.  On an Intel Pentium, multiplications and
    // additions cost the same, so the code below is slower than the code
    // above.

    float A = (w+x)*(q.w+q.x);
    float B = (z-y)*(q.y-q.z);
    float C = (x-w)*(q.y+q.z);
    float D = (y+z)*(q.x-q.w);
    float E = (x+z)*(q.x+q.y);
    float F = (x-z)*(q.x-q.y);
    float G = (w+y)*(q.w-q.z);
    float H = (w-y)*(q.w+q.z);

    float EpF = E+F, EmF = E-F;
    float GpH = G+H, GmH = G-H;

    return Quaternion
    (
         B+0.5*(GpH-EpF),
         A-0.5*(GpH+EpF),
        -C+0.5*(EmF+GmH),
        -D+0.5*(EmF-GmH)
    );
#endif
}
//---------------------------------------------------------------------------
Quaternion Quaternion::operator* (float c) const
{
    return Quaternion(c*w,c*x,c*y,c*z);
}
//---------------------------------------------------------------------------
Quaternion operator* (float c, const Quaternion& q)
{
    return Quaternion(c*q.w,c*q.x,c*q.y,c*q.z);
}
//---------------------------------------------------------------------------
Quaternion Quaternion::operator- () const
{
    return Quaternion(-w,-x,-y,-z);
}
//---------------------------------------------------------------------------
float Quaternion::Dot (const Quaternion& q) const
{
    return w*q.w+x*q.x+y*q.y+z*q.z;
}
//---------------------------------------------------------------------------
float Quaternion::Norm () const
{
    return w*w+x*x+y*y+z*z;
}
//---------------------------------------------------------------------------
Quaternion Quaternion::Inverse () const
{
    float norm = w*w+x*x+y*y+z*z;
    if ( norm > 0.0 )
    {
        norm = 1.0/norm;
        return Quaternion(w*norm,-x*norm,-y*norm,-z*norm);
    }
    else
    {
        // return an invalid result to flag the error
        return ZERO;
    }
}
//---------------------------------------------------------------------------
Quaternion Quaternion::UnitInverse () const
{
    // assert:  'this' is unit length
    return Quaternion(w,-x,-y,-z);
}
//---------------------------------------------------------------------------
Quaternion Quaternion::Exp () const
{
    // If q = A*(x*i+y*j+z*k) where (x,y,z) is unit length, then
    // exp(q) = cos(A)+sin(A)*(x*i+y*j+z*k).  If sin(A) is near zero,
    // use exp(q) = cos(A)+A*(x*i+y*j+z*k) since A/sin(A) has limit 1.

    float angle = sqrt(x*x+y*y+z*z);
    float sn = sin(angle);

    Quaternion result;
    result.w = cos(angle);

    if ( fabs(sn) >= g_dEpsilon )
    {
        float coeff = sn/angle;
        result.x = coeff*x;
        result.y = coeff*y;
        result.z = coeff*z;
    }
    else
    {
        result.x = x;
        result.y = y;
        result.z = z;
    }

    return result;
}
//---------------------------------------------------------------------------
Quaternion Quaternion::Log () const
{
    // If q = cos(A)+sin(A)*(x*i+y*j+z*k) where (x,y,z) is unit length, then
    // log(q) = A*(x*i+y*j+z*k).  If sin(A) is near zero, use log(q) =
    // sin(A)*(x*i+y*j+z*k) since sin(A)/A has limit 1.

    Quaternion result;
    result.w = 0.0;

    if ( fabs(w) < 1.0 )
    {
        float angle = acos(w);
        float sn = sin(angle);
        if ( fabs(sn) >= g_dEpsilon )
        {
            float coeff = angle/sn;
            result.x = coeff*x;
            result.y = coeff*y;
            result.z = coeff*z;
            return result;
        }
    }

    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}
//---------------------------------------------------------------------------
point3 Quaternion::operator* (const point3& pt) const
{
    // Given a vector u = (x0,y0,z0) and a unit length quaternion
    // q = <w,x,y,z>, the vector v = (x1,y1,z1) which represents the
    // rotation of u by q is v = q*u*q^{-1} where * indicates quaternion
    // multiplication and where u is treated as the quaternion <0,x0,y0,z0>.
    // Note that q^{-1} = <w,-x,-y,-z>, so no real work is required to
    // invert q.  Now
    //
    //   q*u*q^{-1} = q*<0,x0,y0,z0>*q^{-1}
    //     = q*(x0*i+y0*j+z0*k)*q^{-1}
    //     = x0*(q*i*q^{-1})+y0*(q*j*q^{-1})+z0*(q*k*q^{-1})
    //
    // As 3-vectors, q*i*q^{-1}, q*j*q^{-1}, and 2*k*q^{-1} are the columns
    // of the rotation matrix computed in Quaternion::ToRotationMatrix.  The
    // vector v is obtained as the product of that rotation matrix with
    // vector u.  As such, the quaternion representation of a rotation
    // matrix requires less space than the matrix and more time to compute
    // the rotated vector.  Typical space-time tradeoff...

    float R[3][3];
    ToRotationMatrix(R);

    point3 result;
    result.x = R[0][0]*pt.x+R[0][1]*pt.y+R[0][2]*pt.z;
    result.y = R[1][0]*pt.x+R[1][1]*pt.y+R[1][2]*pt.z;
    result.z = R[2][0]*pt.x+R[2][1]*pt.y+R[2][2]*pt.z;

    return result;
}
//---------------------------------------------------------------------------
Quaternion Quaternion::Slerp (float t, const Quaternion& p,
    const Quaternion& q)
{
    // Compute dot product (equal to cosine of the angle between quaternions)
    float fCosTheta = p.Dot(q);

    // Check angle to see if quaternions are in opposite hemispheres
    static Quaternion tmp;
    if( fCosTheta < 0.0f ) 
    {
        // If so, flip one of the quaterions
        fCosTheta = -fCosTheta;
        tmp=-q;
    }
    else
      tmp=q;

    // Set factors to do linear interpolation, as a special case where the
    // quaternions are close together.
    float fBeta = 1.0f - t;
    
    // If the quaternions aren't close, proceed with spherical interpolation
    if( 1.0f - fCosTheta > 0.001f ) 
    {   
        float fTheta = acosf( fCosTheta );
        
        fBeta  = sinf( fTheta*fBeta ) / sinf( fTheta);
        t = sinf( fTheta*t ) / sinf( fTheta);
    }

    // Do the interpolation
    return fBeta*p+t*tmp;
}
//---------------------------------------------------------------------------
Quaternion Quaternion::SlerpExtraSpins (float t, const Quaternion& p,
    const Quaternion& q, int iExtraSpins)
{
    // assert:  p.Dot(q) >= 0 (guaranteed in RotationKey::Preprocess)
    float dCos = p.Dot(q);

    // numerical round-off error could create problems in call to acos
    if ( dCos < -1.0 )
        dCos = -1.0;
    else if ( dCos > 1.0 )
        dCos = 1.0;

    float dAngle = acos(dCos);
    float dSin = sin(dAngle);  // fSin >= 0 since fCos >= 0

    if ( dSin < g_dEpsilon )
    {
        return p;
    }
    else
    {
        float dPhase = g_dPi*iExtraSpins*t;
        float dInvSin = 1.0/dSin;
        float dCoeff0 = sin((1.0-t)*dAngle - dPhase)*dInvSin;
        float dCoeff1 = sin(t*dAngle + dPhase)*dInvSin;
        return dCoeff0*p + dCoeff1*q;
    }
}
//---------------------------------------------------------------------------
void Quaternion::Intermediate (const Quaternion& q0, const Quaternion& q1,
    const Quaternion& q2, Quaternion& a, Quaternion& b)
{
    // assert:  q0, q1, q2 are unit quaternions

    Quaternion q0inv = q0.UnitInverse();
    Quaternion q1inv = q1.UnitInverse();
    Quaternion p0 = q0inv*q1;
    Quaternion p1 = q1inv*q2;
    Quaternion arg = 0.25*(p0.Log()-p1.Log());
    Quaternion marg = -arg;

    a = q1*arg.Exp();
    b = q1*marg.Exp();
}
//---------------------------------------------------------------------------
Quaternion Quaternion::Squad (float t, const Quaternion& p,
    const Quaternion& a, const Quaternion& b, const Quaternion& q)
{
    return Slerp(2*t*(1-t),Slerp(t,p,q),Slerp(t,a,b));
}
