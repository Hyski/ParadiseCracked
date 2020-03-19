// PrimitiveContaner.h: interface for the PrimitiveContaner class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRIMITIVECONTANER_H__FA828C60_3620_11D4_A0E0_525405F0AA60__INCLUDED_)
#define AFX_PRIMITIVECONTANER_H__FA828C60_3620_11D4_A0E0_525405F0AA60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../3d/geometry.h"
#include <string>
class GraphPipe;
class PrimitiveContaner  
  {
  public:
    //���������� ����� �������� ������
    virtual void PrepareData(GraphPipe *Pipe)=0; 
    //�������� ���������� ������������ ��������
    virtual int GetNumShaders()=0; 
    //������ ��� ������� �������
    virtual std::string GetShaderName(int ShaderNum)=0;
     //������ ���������� ������ ������� �������
    virtual int GetShaderVertCount(int ShaderNum)=0;
     //������ ��� ����������� ������� �������
    virtual unsigned GetShaderContents(int ShaderNum)=0;
     //������ ��� ��������� ������� �������
    virtual Primi::PrimType GetShaderPrimType(int ShaderNum)=0;
    //��������� ������� ��� ������� (����������� �������, ������� ��������)
    virtual void FillShader(int ShaderNum, Primi *P)=0;
    //���������� ����� ��������� ������ 
    virtual void FreeData()=0;
  public:
    PrimitiveContaner();
    virtual ~PrimitiveContaner();
  };

#endif // !defined(AFX_PRIMITIVECONTANER_H__FA828C60_3620_11D4_A0E0_525405F0AA60__INCLUDED_)
