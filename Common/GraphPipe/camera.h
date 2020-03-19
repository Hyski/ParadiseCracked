#if !defined(AFX_CAMERA_H__A2C89401_104C_11D4_81EA_0060520A50A9__INCLUDED_)
#define AFX_CAMERA_H__A2C89401_104C_11D4_81EA_0060520A50A9__INCLUDED_

/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description:����� ������. �������� �� ����������� ������,
               �������� ������ ������� � ������������.
               ����� ������������� �������������� ������
               ��������, ��� picking: (x,y) -> ���.

   Author: Grom 
   Creation: 12 ������ 2000
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
    point3 Pos,Dir,Right,Up; //���������� ������
    Quaternion  Orient;      // ���������� ������
    D3DMATRIX View,Proj;     //������� ����������� � ����������� �������
    
    float Near,Far,Fov,Aspect;
    
    bool ViewMod,ProjMod;

    point3 AimSpot;          //����� �� �����, ���� ������� ������
    bool AimSpotDirty;
    MOVE_TYPE Moving;
  private: //��, ��� �������� ����������� �� �������
    float      starttime;//
    KeyAnimation  path;//
  public:
    Frustum Cone;
  protected:
    void ConstructView();
    void CalcAimSpot();
    float GetOrientAngle();
    void UpdateFrustum();//�������� Frustum
    void Zoom(float val);   //(+)�����������/(-)�������� ������
		void MoveBack(float time);
  public:
    enum RotateAxis{SELF=0x10,AIMSPOT=0x20,RIGHT=0x01,Z=0x02};
    
    Camera():Moving(MT_FREE){ViewMod=ProjMod=true;AimSpotDirty=true;};

    void SetProjection(float Fov, float Near, float Far, float Aspect);
    void Apply(IDirect3DDevice7 *D3DDev);    //��������� ������� ������ � ����������
    point3 Pick(float x,float y) const; //������� ����� �� ������ � �����������
    
    void Rotate(float Angle,unsigned Axis); //������� ������ ��������� ���
    void Move(const point3 &Delta); //�������� �� ��������� ����������	(z - ���������� ������������)
    void LinkToSpline(const KeyAnimation &spl, float Time);       //������ ������ �� ������ �������
    void LinkToGrid(const GeneralGrid *Grid, const point3&p);         //�������� ��� ������ ����� ��������
    void Update(float Time); //��� ������������� ����������� ��������� ������
    
    void SetDest(const point3 &Dir, const point3 &Dest);
    void SetLocation(const point3&p, const point3 &dir);
    bool  IsLinked(){return Moving!=MT_FREE;};
    
    void FocusOn(const point3 &spot,float FocusTime=0.2);   //����� ������ ������ �� ��������� �����
    //�������� ��������� ������
    const point3& GetPos() const {return Pos;};
    const point3& GetFront() const {return Dir;};
    const point3& GetUp() const {return Up;};
	const point3& GetRight() const {return Right;};
    bool IsViewChanged() const {return ViewMod;};//���������� �� ������� ������ � �������� �����
    bool IsProjChanged() const {return ProjMod;};//���������� �� �������� ������ � �������� �����
    
    void GetProjOptions(float *_Far, float *_Near, float *_Fov) const {*_Far=Far;*_Near=Near;*_Fov=Fov;};//�������� ���������� � ������ � �������� ������
    const D3DMATRIX* GetView() const {return &View;};//�������� ������� ������� ������
    const D3DMATRIX* GetProj() const {return &Proj;};//�������� ������� �������� ������
  };
#endif // !defined(AFX_CAMERA_H__A2C89401_104C_11D4_81EA_0060520A50A9__INCLUDED_)
