// Grid.cpp: implementation of the Grid class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include <stdio.h>
#include "../common/graphpipe/culling.h"
#include "../logic2/hexutils.h"

#include "Grid.h"

struct HexStep {int x,y;};
const  HexStep Steps[2][6]=
{
    { { 1, 0},{ 0, 1},{-1, 1},{-1, 0},{-1,-1},{ 0,-1}},
    { { 1, 0},{ 1, 1},{ 0, 1},{-1, 0},{ 0,-1},{ 1,-1}}
};

Grid::Grid(const std::string& name):
    HexGrid(name.c_str())
{
}

Grid::~Grid()
{
}
float dist_to_weight(float d)
  {
  float dist;
  if(d<0.6) dist=(0.6-d)/0.6+0.01;
  else dist=0.01;
  dist*=dist;
  saturate(dist,0.f,1.f);
  return dist;
  }
float Grid::Height(const point3 &Pos) const
  {
  float d;
  int i;
  float h=0;
  float summ=0;
  float w;
  ipnt2_t hpnt=HexUtils::scr2hex(Pos);
  if(HexGrid::GetInst()->IsOutOfRange(hpnt)) return 0;
  
  point3 r=HexGrid::GetInst()->Get(hpnt);

  d=hypot(Pos.x-r.x,Pos.y-r.y);
  w=dist_to_weight(d);
  h+=r.z*w;
  summ+=w;

  for(i=0;i<6;i++)
    {
    HexUtils::GetFrontPnts0(hpnt,i, &hpnt);
    if(HexGrid::GetInst()->IsOutOfRange(hpnt)) continue;
    r=HexGrid::GetInst()->Get(hpnt);
    d=hypot(Pos.x-r.x,Pos.y-r.y);
    w=dist_to_weight(d);
    h+=r.z*w;
    summ+=w;
    }
  h/=summ;
  return h;
  }
/*
float Grid::Height(const point3 &_Pos) const
{
    float HEXSize = GetHexSize();
    float HEXSizeY=HEXSize*cos(3.14159265*30/180);

    int y1,y2,x1,x2,x3,x4,x5;
    point3 Pos(_Pos);
 //   Pos.y=-Pos.y;

    y1=floor(Pos.y/HEXSizeY);
    y2=y1+1;

    x1=(Pos.x-((y1&1)?HEXSize/2:0))/HEXSize;   x2=x1+1;
    x3=x1+Steps[y1&1][4].x;
    x4=x3+1;
    x5=x4+1;
    Triangle a;
    float t,u,v;
    static point3 dir(0,0,-1);
    point3  Origin(Pos+point3(0,0,10000));

    int SizeX = GetSizeX(), 
        SizeY = GetSizeY();

    HexGrid::const_cell_iterator cells = first_cell();
    HexGrid::hg_slice hs = GetSlice();

    if(y1>=0&&y1<SizeY&&y2>=0&&y2<SizeY)
    {
        if(x1>=0&&x1<SizeX&&x2>=0&&x2<SizeX&& x4>=0&&x4<SizeX)
        {
            a.V[0]= *(cells + hs(x1, y1));   //a.V[0].y = -a.V[0].y;
            a.V[1]= *(cells + hs(x2, y1));   //a.V[1].y = -a.V[1].y;
            a.V[2]= *(cells + hs(x4, y2));   //a.V[2].y = -a.V[2].y;

            if(a.RayTriangle(Origin,dir,&t,&u,&v))
            {
                return Origin.z-t;
            }
        }

        if(x1>=0&&x1<SizeX&&x4>=0&&x4<SizeX&& x3>=0&&x3<SizeX)
        {
            a.V[0]= *(cells + hs(x1, y1));   //a.V[0].y = -a.V[0].y;
            a.V[1]= *(cells + hs(x4, y2));   //a.V[1].y = -a.V[1].y;
            a.V[2]= *(cells + hs(x3, y2));   //a.V[2].y = -a.V[2].y;

            if(a.RayTriangle(Origin,dir,&t,&u,&v))
            {
                return Origin.z-t;
            }
        }
        
        if(x5>=0&&x5<SizeX&&x2>=0&&x2<SizeX&& x4>=0&&x4<SizeX)
        {
            a.V[0]= *(cells + hs(x2, y1));   //a.V[0].y = -a.V[0].y;
            a.V[1]= *(cells + hs(x5, y2));   //a.V[1].y = -a.V[1].y;
            a.V[2]= *(cells + hs(x4, y2));   //a.V[2].y = -a.V[2].y;

            if(a.RayTriangle(Origin,dir,&t,&u,&v))
            {
                return Origin.z-t;
            }
        }
    }
    return 0;
}
*/