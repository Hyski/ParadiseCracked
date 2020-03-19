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
    //вызывается перед запросом данных
    virtual void PrepareData(GraphPipe *Pipe)=0; 
    //получить количество используемых шейдеров
    virtual int GetNumShaders()=0; 
    //узнать имя данного шейдера
    virtual std::string GetShaderName(int ShaderNum)=0;
     //узнать количество вершин данного шейдера
    virtual int GetShaderVertCount(int ShaderNum)=0;
     //узнать тип содержимого данного шейдера
    virtual unsigned GetShaderContents(int ShaderNum)=0;
     //узнать тип примитива данного шейдера
    virtual Primi::PrimType GetShaderPrimType(int ShaderNum)=0;
    //заполнить вершины для шейдера (обязательно столько, сколько заявлено)
    virtual void FillShader(int ShaderNum, Primi *P)=0;
    //вызывается после окончания сеанса 
    virtual void FreeData()=0;
  public:
    PrimitiveContaner();
    virtual ~PrimitiveContaner();
  };

#endif // !defined(AFX_PRIMITIVECONTANER_H__FA828C60_3620_11D4_A0E0_525405F0AA60__INCLUDED_)
