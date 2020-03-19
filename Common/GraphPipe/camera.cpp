// camera.cpp: implementation of the Camera class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include "camera.h"
#include "../../gamelevel/grid.h"
#include "../../logic2/hexutils.h"


/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description:Класс камеры. Отвечает за перемещение камеры,
               создание матриц видовой и проекционной.
               Также предоставляет дополнительные методы
               Например, для picking: (x,y) -> луч.

   Author: Grom 
   Creation: 12 апреля 2000
***************************************************************/  
#include "../precomp.h"   


#if 0&&defined(_HOME_VERSION)
CLog camera_log;
#define camlog	camera_log["camera.log"]
#else
#define camlog	/##/
#endif




static const float _UpAngle=(TORAD(35));
static const float _DownAngle=(TORAD(89.9));
static const float _NearDist=7;
static const float _FarDist=15;
/*********************************************************************/
void Camera::SetProjection(float _Fov, float _Near, float _Far, float _Aspect)
{
 Aspect=_Aspect;
 Fov=_Fov;
 Far=_Far;
 Near=_Near;
 D3DUtil_SetProjectionMatrix( Proj,Fov,Aspect,Near,Far);
 ProjMod=true;
}

void Camera::Apply(IDirect3DDevice7 *D3DDev)
  {
  if(ViewMod)
  D3DDev->SetTransform( D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)&View);

  if(ProjMod)
  D3DDev->SetTransform( D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)&Proj);

  ProjMod=ViewMod=false;
  }

void invmat(D3DMATRIX &a, const D3DMATRIX &b);
point3 Camera::Pick(float x,float y) const
  {// Compute the vector of the pick ray in screen space
  point3 PickRay;
  point3 v;
  v.x =  ((2.0f*x/D3DKernel::ResX())-1)/Proj._11;
  v.y = -((2.0f*y/D3DKernel::ResY())-1)/Proj._22;
  //v.z =  1.0f;
  
  // Get the inverse view matrix
  D3DMATRIX matView, m;
  invmat( m, View );
  
  // Transform the screen space pick ray into 3D space
  PickRay.x  = v.x*m._11 + v.y*m._21 + /*v.z**/m._31;
  PickRay.y  = v.x*m._12 + v.y*m._22 + /*v.z**/m._32;
  PickRay.z  = v.x*m._13 + v.y*m._23 + /*v.z**/m._33;
  return PickRay;
  }
float Camera::GetOrientAngle()
  {
  point3 X,Y;
  Y=AXISZ;
  X=Y.Cross(Right);
  float x,y;
  x=Dir.Dot(X);
  y=-Dir.Dot(Y);
  float ang=atan2(y,x);
  return ang;
  }
void Camera::Rotate(float Angle,unsigned Axis)
  {
  if(Moving!=MT_FREE) return;
  point3 rotvec,rotpnt;
  Quaternion q;
  Angle=-Angle;
  //дайте мне точку опоры, и я....
  if(Axis&Z) rotvec=AXISZ;//поворот вокруг оси Z
  else       rotvec=Right;//поворот вокруг оси right
  if(Axis&SELF)
    {
    rotpnt=Pos;    //поворот вокруг себя самой
    AimSpotDirty=true;
    }
  else          rotpnt=AimSpot;//поворот вокруг точки привязки
  if(!(Axis&Z))    
    {
    float ang=GetOrientAngle();
    if(ang-Angle<_UpAngle) 
      Angle=ang-_UpAngle;
    if(ang-Angle>_DownAngle) 
      Angle=ang-_DownAngle;
    }
  //теперь повернем все наше хозяйство
  q.FromAngleAxis(Angle,rotvec);
  Pos=q*(Pos-rotpnt)+rotpnt;
  Dir=Normalize(q*(Dir));
  Up=Normalize(q*(Up));
  Right=Dir.Cross(Up);
  Up=Right.Cross(Dir);
  // подправим матрицы
  ConstructView();
  }       
void Camera::SetLocation(const point3&p, const point3 &dir)
      {
	camlog("SetLocation: %f %f %f\n",p.x,p.y,p.z);
      Moving=MT_FREE;
      Pos=p;Dir=Normalize(dir);
      Right=Normalize(Dir.Cross(AXISZ));
      Up=Normalize(Right.Cross(Dir));
      ConstructView();
      }

void Camera::MoveBack(float time)
	{
camlog("MoveBack<: %f %f %f\n", Pos.x, Pos.y, Pos.z);
	point3 DirY(Dir);
	point3 DirX(Right);
	HexGrid *g=HexGrid::GetInst();
	if(g)
		{
		float deltax,deltay;
		float MaxX=g->GetSizeX(),MaxY=g->GetSizeY();
		float cosa=sin(TORAD(60));
		if(AimSpot.x>MaxX*1) deltax=MaxX-AimSpot.x;
		else if(AimSpot.x<0) deltax=-AimSpot.x;
			 else deltax=0;
		if(AimSpot.y>MaxY*cosa) deltay=MaxY*cosa-AimSpot.y;
		 else
			 if(AimSpot.y<0)   deltay=-AimSpot.y;
			 else deltay=0;

		 float c = time* powf(2.f, hypot(deltax,deltay)*0.2f );

		 c = c>1.f? 1.f : c;
		deltax*=c;
		deltay*=c;
		if(deltax||deltay)
				 {
				 point3 D(deltax,deltay,0);
				 point3 MoveVec(DirX.Dot(D),DirY.Dot(D),0);
				 Move(MoveVec);
				 }
		}
camlog("MoveBack>: %f %f %f\n", Pos.x, Pos.y, Pos.z);
	}
void Camera::CalcAimSpot()
  {
  if(AimSpotDirty)
    {
    float step=0;
    point3 p;
    Grid *g=(Grid*)HexGrid::GetInst();
    float MaxX=0,MaxY=0;
    if(g)
      {
      MaxX=g->GetSizeX();
      MaxY=g->GetSizeY();
      }
    float h=0;
    for(step=0;step<40.f;step+=0.3)
      {
      p=Pos+step*Dir;
      if(g)
        {
        bool OutOfField=((p.x>MaxX)||(p.x<0)||(p.y>MaxY*1)||(p.y>MaxY));
        if((p.z>0) && OutOfField) continue;
        if(!OutOfField)        h=g->Height(p);
        else 
			{
				h=0;
			}
        }
      else
       h=0;
      if(h>p.z) break;
      }
    AimSpot=p;
    AimSpotDirty=false;
    }
  }
void Camera::ConstructView()
  {
  D3DUtil_SetIdentityMatrix(View);
  View._11=Right.x;    View._21=Right.y;    View._31=Right.z;
  View._12=Up.x;       View._22=Up.y;       View._32=Up.z;
  View._13=Dir.x;      View._23=Dir.y;      View._33=Dir.z;
  View._41=-Pos.Dot(Right);
  View._42=-Pos.Dot(Up);
  View._43=-Pos.Dot(Dir);
  UpdateFrustum();
  CalcAimSpot();
  ViewMod=true;
  }
void Camera::Zoom(float val)
  {
  float len=Pos.z-AimSpot.z;
  if(len+val*Dir.z<_NearDist) return;//val=-(len-_NearDist)/Dir.z;
  if(len+val*Dir.z>_FarDist) return;//val=-(len-_FarDist)/Dir.z;
  Pos+=Dir*val;
  ConstructView();
  }
void Camera::UpdateFrustum()
  {
  //far,right,left,bottom,up,near;
  Cone.Planes[0]=Plane(-Dir,Pos+Dir*Far);//Far
  Cone.Planes[5]=Plane(Dir,Pos+Dir*Near);//Near
  Quaternion q;
  float FovW=atan(1.f/Proj._11)*2;
  float FovH=atan(1.f/Proj._22)*2;
  q.FromAngleAxis(PId2-FovW/2,Up);
  Cone.Planes[1]=Plane(q*Dir,Pos);//Left
  
  q.FromAngleAxis(-PId2+FovW/2,Up);
  Cone.Planes[2]=Plane(q*Dir,Pos);//Right
                 
  q.FromAngleAxis(PId2-FovH/2,Right);
  Cone.Planes[3]=Plane(q*Dir,Pos);//up

  q.FromAngleAxis(-PId2+FovH/2,Right);
  Cone.Planes[4]=Plane(q*Dir,Pos);//bottom

  Cone.Points[0]=Pos; //вершина пирамиды
  Cone.Points[1]=Pos+Dir*Far-Right*Far*tan(Fov/2)+Up*Far*tan(Fov/2);
  Cone.Points[2]=Pos+Dir*Far+Right*Far*tan(Fov/2)+Up*Far*tan(Fov/2);
  Cone.Points[3]=Pos+Dir*Far-Right*Far*tan(Fov/2)-Up*Far*tan(Fov/2);
  Cone.Points[4]=Pos+Dir*Far+Right*Far*tan(Fov/2)-Up*Far*tan(Fov/2);
  }
void Camera::Move(const point3 &Delta)
  {
camlog("Move: %f %f %f\n",Delta.x,Delta.y,Delta.z);
  if(Moving!=MT_FREE) return;
  point3 dir_y=Normalize(point3(Dir.x,Dir.y,0));
  point3 dir_x=Normalize(point3(Right.x,Right.y,0));
  Pos+=Delta.x*dir_x+Delta.y*dir_y;
  Zoom(Delta.z);
  if(Delta.x!=0.f||Delta.y!=0.f)  AimSpotDirty=true;

  ConstructView();
  }
void Camera::LinkToSpline(const KeyAnimation &spl,float Time)       //камера встает на рельсы сплайна
  {
  const float focustime=0.1;
  Moving=MT_SPLINE;
  starttime=Time+focustime;
  path=spl;
  //в начале  добавим текущее положение
  float r[3][3];
  r[0][0]=Right.x; r[1][0]=Right.y; r[2][0]=Right.z;
  r[0][1]=Up.x;    r[1][1]=Up.y;    r[2][1]=Up.z;
  r[0][2]=-Dir.x;  r[1][2]=-Dir.y;  r[2][2]=-Dir.z;
  Quaternion q;
  q.FromRotationMatrix(r);
  q.w=-q.w;
  path.SetKey(-focustime,q,Pos);

  //в конце выправим матрицу по вертикали
  point3 NRight,NUp,NDir,NPos;
  path.GetTrans(&NPos,path.GetLastTime()+1);
  path.GetAngle(&q,path.GetLastTime()+1);
  q.w=-q.w;
  NDir=Normalize(q*(-AXISZ));
  NRight=Normalize(NDir.Cross(AXISZ));
  NUp=Normalize(NRight.Cross(NDir));
  r[0][0]=NRight.x; r[1][0]=NRight.y; r[2][0]=NRight.z;
  r[0][1]=NUp.x;    r[1][1]=NUp.y;    r[2][1]=NUp.z;
  r[0][2]=-NDir.x;  r[1][2]=-NDir.y;  r[2][2]=-NDir.z;
  q.FromRotationMatrix(r);
  q.w=-q.w;
  path.SetKey(path.GetLastTime()+focustime/2,q,NPos); 
  }

void Camera::LinkToGrid(const GeneralGrid *, const point3 &)         //указание для камеры сетки привязки
  {
  }

void Camera::Update(float Time) //при необходимости обновляется положение камеры
  {
camlog("Update: %f %f %f\n", Pos.x, Pos.y, Pos.z);
  static float LTime=0;
  if(Moving == MT_SPLINE)
    {
    if(Time-starttime>path.GetLastTime()) 
      {Moving=MT_FREE;AimSpotDirty=true;}
    path.GetTrans(&Pos,Time-starttime);
    Quaternion q;
    path.GetAngle(&q,Time-starttime);
    q.w=-q.w;
    Dir=Normalize(q*(-AXISZ));
    Up=Normalize(q*(AXISY));
    Right=Normalize(q*AXISX);
    ConstructView();
    }
  if(Moving==MT_FREE)
    {       
    float timedelta=Time-LTime;
    float len=Pos.z-AimSpot.z;
    float val=0;
    if(len<_NearDist) val=-20*timedelta;
    if(len>_FarDist) val=20*timedelta;

    /*DebugInfo::Add(350,50,"len=%f, val=%f",len,val);
    DebugInfo::Add(350,80,"AimSpot(%f %f %f)",AimSpot.x,AimSpot.y,AimSpot.z);
    DebugInfo::Add(350,110,"AimSpotDirty %s",AimSpotDirty?"true":"false");
    DebugInfo::Add(350,140,"Pos(%f %f %f)",Pos.x,Pos.y,Pos.z); */
    if(val>5) val=5;

	if(Dir.z*val>_FarDist)
		val = (_FarDist-Pos.z)/Dir.z;
	if(Pos.z+Dir.z*val<_NearDist)
		val = (_NearDist-Pos.z)/Dir.z;

    if(val!=0.f)
      {
      Pos+=Dir*val;
      ConstructView();
      }

		MoveBack(Time-LTime);//FIXME:
    }
  LTime=Time;
camlog("Update>: %f %f %f\n", Pos.x, Pos.y, Pos.z);
  }
void Camera::FocusOn(const point3 &spot,float FocusTime)   //смена фокуса камеры не указанную точку
  {
camlog("FocusOn: %f %f %f\n", spot.x, spot.y, spot.z);
  KeyAnimation ka;
  Quaternion q;
  point3 NewDir,NewPos,NewUp,NewRight;
  NewDir=Normalize(spot-Pos);
  NewDir.z=-hypot(NewDir.x,NewDir.y);
  NewDir=Normalize(NewDir);
  NewRight=Normalize(NewDir.Cross(AXISZ));
  NewUp=Normalize(NewRight.Cross(NewDir));
  NewPos=spot-11*NewDir;
  float r[3][3];
  r[0][0]=NewRight.x; r[1][0]=NewRight.y; r[2][0]=NewRight.z;
  r[0][1]=NewUp.x;    r[1][1]=NewUp.y;    r[2][1]=NewUp.z;
  r[0][2]=-NewDir.x;  r[1][2]=-NewDir.y;  r[2][2]=-NewDir.z;
  q.FromRotationMatrix(r);
  q.w=-q.w;
  ka.SetKey(FocusTime,q,NewPos);  
  LinkToSpline(ka,Timer::GetSeconds());
  }
void Camera::SetDest(const point3 &, const point3 &Dest)
  {
  FocusOn(Dest);
  }
