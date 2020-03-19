/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ���� ����� ��������� ������� ������ ���� ������, �����...
       ������ ������ ���� ��������� � ������� ����
     Author: Grom 
   Creation: 31 ���� 2000
***************************************************************/                

#if !defined(AFX_DOBJECT_H__F16DCF19_563D_4860_A425_BF836A6F7898__INCLUDED_)
#define AFX_DOBJECT_H__F16DCF19_563D_4860_A425_BF836A6F7898__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../skin/skin.h"

class DObject  
{
//���������� ���������
  protected:
    std::map<float,point3> Trans;       //����� ��� �������� �������
    std::map<float,Quaternion> Angles;  //����� ��� �������� �������
    float LastTime;                //����� ���������� �����
    point3 Translate;
    Quaternion Angle;
  public:
    BBox Bound;     //BBox � ������� �����������
    BBox LocalBound;//BBox � ��������� ����������� �������  (���������)
    SimpleTexturedObject *Parts[500];
    std::vector<SimpleTexturedObject> PartsVec;
    int PartsNum;
    std::string Name;
    D3DMATRIX World;

//����� ������� ���������
    float State;//����������� ��������� �������� 0 - ������ 1 - �����
    bool Animation;//���� - � ������ ������ ���� ��������
    float EndState; //����������� - � �������� ��������� ��������
    float StartTime;//�����, ����� ������� ������� ����� �����������
  public:
    void ChangeState(float State, float Time);
  public:
	  void UpdateBound();
	  void Update(float Time);
    float GetLastTime()
      {
      std::map<float,Quaternion>::iterator ang=Angles.end();
      std::map<float,point3>::iterator trans=Trans.end();
      ang--;
      trans--;
      return ang->first>trans->first?ang->first:trans->first;
      }
	  void Load(FILE *f);
	  void UpdateOnTime(float Time);
    DObject& operator=(const DObject &a)
      {
      Angle=a.Angle;Translate=a.Translate;
      Trans=a.Trans;Angles=a.Angles;
      Name=a.Name;
      int i;
      PartsVec.clear();
      PartsNum=a.PartsNum;
      PartsVec=a.PartsVec;
      for(i=0;i<PartsNum;i++)
        Parts[i]=&PartsVec[i];
      LocalBound=a.LocalBound;
      Bound=a.Bound;
      LastTime=a.LastTime;
      State=a.State;
      Animation=a.Animation;
      World=a.World;
      return *this;
      }
	DObject();
  DObject(const DObject &a)
    {    
    PartsNum=0;
    LastTime=0;
    *this=a;
    }
	virtual ~DObject();






};

#endif // !defined(AFX_DOBJECT_H__F16DCF19_563D_4860_A425_BF836A6F7898__INCLUDED_)
