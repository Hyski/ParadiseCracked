// LongShot.cpp: implementation of the LongShot class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include "../common/Graphpipe/graphpipe.h"
#include "LongShot.h"
#include "../common/datamgr/txtfile.h"
#include "../Options/Options.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define LONGSHOTXLSNAME "scripts/levels.txt"
LongShot::LongShot()
  {
	Enabled=true;
	int i;
  static const std::string Names[6]={"env/down","env/up","env/right","env/left","env/front","env/back"};
  
  PartNum=6;
  for(i=0;i<PartNum;i++)
    {
    Parts[i]=new STObject;
    Parts[i]->MaterialName=Names[i];
    Parts[i]->Alloc(6,0);
    }
  CurrentLevelName="";
  ParseXLS();

  static texcoord t[12*3]={
                     texcoord(1,0),texcoord(1,1),texcoord(0,1),
                     texcoord(1,0),texcoord(0,1),texcoord(0,0),
                     texcoord(1,0),texcoord(0,1),texcoord(0,0),
                     texcoord(1,0),texcoord(1,1),texcoord(0,1),

                     texcoord(1,1),texcoord(0,0),texcoord(1,0),
                     texcoord(1,1),texcoord(0,1),texcoord(0,0),
                     texcoord(0,1),texcoord(0,0),texcoord(1,0),
                     texcoord(0,1),texcoord(1,0),texcoord(1,1),

                     texcoord(0,1),texcoord(1,0),texcoord(1,1),
                     texcoord(0,1),texcoord(0,0),texcoord(1,0),
                     texcoord(1,1),texcoord(0,1),texcoord(0,0),
                     texcoord(1,1),texcoord(0,0),texcoord(1,0),
    };
  for(i=0;i<PartNum;i++)
      {
      memcpy(Parts[i]->uv,t+2*3*i,sizeof(texcoord)*3*2);
      }
	bool value;
  value=Options::GetInt("game.environment");

	Enable(value);
  }
void LongShot::ParseXLS()
  {
  TxtFile xls(DataMgr::Load(LONGSHOTXLSNAME));
  DataMgr::Release(LONGSHOTXLSNAME);

	const int EnvColumn=18;

  std::string levname,shdname;
  for(int i=1;i<xls.SizeY();i++)
	{
		xls.GetCell(i,0,levname);
		xls.GetCell(i,EnvColumn,shdname);
    LevelToShader[levname]=shdname;
		if(!levname.size()) break;
	}
}

void LongShot::LevelChanged(const std::string &NewName, GraphPipe *Pipe)
  {
  //1. выгрузить старые шейдеры
  if(CurrentLevelName.size())
    Pipe->UnloadShaderFile(LevelToShader[CurrentLevelName]+".shader");

  //2. загрузить новые
 Pipe->RegisterShaderFile(LevelToShader[NewName]+".shader");

  CurrentLevelName=NewName;
  }

LongShot::~LongShot()
{
PartNum=6;//реально их действительно столько
Release();
}
void LongShot::Enable(bool Flag)
	{
  if(Flag)PartNum=6;
	else PartNum=0;
	}
void LongShot::Update(const Camera *Cam)
{
if(!Enabled) return;
  CamOffset=Cam->GetPos();
	float r=185;
  point3 v[12*3]={
		point3(-r,-r,-r),point3(-r,r,-r),point3(r,r,-r),//bottom
			point3(-r,-r,-r),point3(r,r,-r),point3(r,-r,-r),
			point3(-r,-r,r),point3(r,r,r),point3(-r,r,r),  //top
			point3(-r,-r,r),point3(r,-r,r),point3(r,r,r),
			
			point3(-r,-r,-r),point3(r,-r,r),point3(-r,-r,r), //right
			point3(-r,-r,-r),point3(r,-r,-r),point3(r,-r,r),
			point3(-r,r,-r),point3(-r,r,r),point3(r,r,r),    //left
			point3(-r,r,-r),point3(r,r,r),point3(r,r,-r),
			
			point3(-r,-r,-r),point3(-r,r,r),point3(-r,r,-r), //front
			point3(-r,-r,-r),point3(-r,-r,r),point3(-r,r,r),
			point3(r,-r,-r),point3(r,r,-r),point3(r,r,r),    //back
			point3(r,-r,-r),point3(r,r,r),point3(r,-r,r),
	};
	{
		for(int t=0;t<12*3;t++)
			v[t]+=CamOffset;
	}
  for(int i=0;i<PartNum;i++)
	{
		memcpy(Parts[i]->Points,v+2*3*i,sizeof(point3)*3*2);
	}
	
}
