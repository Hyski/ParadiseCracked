#if !defined(AFX_CAMERA_H__A2C89401_104C_11D4_81EA_0060520A50A9__INCLUDED_)
#define AFX_CAMERA_H__A2C89401_104C_11D4_81EA_0060520A50A9__INCLUDED_

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
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class GeneralGrid;
#include "culling.h"
#include "../../skin/keyanimation.h"
class KeyAnimation;
class Camera
  {
  private:
    enum MOVE_TYPE{MT_SPLINE, MT_FREE};
    point3 Pos,Dir,Right,Up; //ориентация камеры
    Quaternion  Orient;      // ориентация камеры
    D3DMATRIX View,Proj;     //матрицы отгруженные в графическую систему
    
    float Near,Far,Fov,Aspect;
    
    bool ViewMod,ProjMod;

    point3 AimSpot;          //точка на сетке, куда смотрит камера
    bool AimSpotDirty;
    MOVE_TYPE Moving;
  private: //то, что касается перемещения по сплайну
    float      starttime;//
    KeyAnimation  path;//
  public:
    Frustum Cone;
  protected:
    void ConstructView();
    void CalcAimSpot();
    float GetOrientAngle();
    void UpdateFrustum();//обновить Frustum
    void Zoom(float val);   //(+)приближение/(-)удаление камеры
		void MoveBack(float time);
  public:
    enum RotateAxis{SELF=0x10,AIMSPOT=0x20,RIGHT=0x01,Z=0x02};
    
    Camera():Moving(MT_FREE){ViewMod=ProjMod=true;AimSpotDirty=true;};

    void SetProjection(float Fov, float Near, float Far, float Aspect);
    void Apply(IDirect3DDevice7 *D3DDev);    //применить матрицы камеры к устройству
    point3 Pick(float x,float y) const; //перевод точки на экране в направление
    
    void Rotate(float Angle,unsigned Axis); //поворот вокруг указанной оси
    void Move(const point3 &Delta); //смещение на указанное расстояние	(z - управление приближением)
    void LinkToSpline(const KeyAnimation &spl, float Time);       //камера встает на рельсы сплайна
    void LinkToGrid(const GeneralGrid *Grid, const point3&p);         //указание для камеры сетки привязки
    void Update(float Time); //при необходимости обновляется положение камеры
    
    void SetDest(const point3 &Dir, const point3 &Dest);
    void SetLocation(const point3&p, const point3 &dir);
    bool  IsLinked(){return Moving!=MT_FREE;};
    
    void FocusOn(const point3 &spot,float FocusTime=0.2);   //смена фокуса камеры не указанную точку
    //получить положение камеры
    const point3& GetPos() const {return Pos;};
    const point3& GetFront() const {return Dir;};
    const point3& GetUp() const {return Up;};
	const point3& GetRight() const {return Right;};
    bool IsViewChanged() const {return ViewMod;};//изменилась ли позиция камеры с прошлого кадра
    bool IsProjChanged() const {return ProjMod;};//изменилась ли проекция камеры с прошлого кадра
    
    void GetProjOptions(float *_Far, float *_Near, float *_Fov) const {*_Far=Far;*_Near=Near;*_Fov=Fov;};//получить информацию о планах и раскрыве камеры
    const D3DMATRIX* GetView() const {return &View;};//получить видовую матрицу камеры
    const D3DMATRIX* GetProj() const {return &Proj;};//получить матрицу проекции камеры
  };
#endif // !defined(AFX_CAMERA_H__A2C89401_104C_11D4_81EA_0060520A50A9__INCLUDED_)
