// Grid.h: interface for the Grid class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRID_H__E9F3F420_3CA0_11D4_A0E0_525405F0AA60__INCLUDED_)
#define AFX_GRID_H__E9F3F420_3CA0_11D4_A0E0_525405F0AA60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//включить описание HexGrid
#include "../logic2/HexGrid.h"

extern float HEXSize;
extern float HEXSizeY;
class GeneralGrid
  {
  public:
    GeneralGrid(){};
  virtual ~GeneralGrid(){};
  virtual float Height(const point3 &Pos)const=0;
  virtual point3 ValidCoord(const point3 &Pos)const=0;
  };

class Grid:public GeneralGrid , public HexGrid {
public:       
    Grid(const std::string& name);
    ~Grid();

    float Height(const point3 &Pos) const ;
    virtual point3 ValidCoord(const point3 &Pos) const {return Pos;};
};

#endif // !defined(AFX_GRID_H__E9F3F420_3CA0_11D4_A0E0_525405F0AA60__INCLUDED_)
