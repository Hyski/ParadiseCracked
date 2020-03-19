// GameLevel.cpp: implementation of the GameLevel class.
//
//////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include "../common/graphpipe/culling.h"
#include "../common/graphpipe/graphpipe.h"
//#include "../common/slib/sound.h"
#include "../sound/ISound.h"
#include "../common/texturemgr/texturemgr.h"
#include "GameLevel.h"
#include "bsp.h"

#include "../game.h"
#include "../Common/3DEffects/EffectManager.h"
#include "../common/saveload/saveload.h"
#include "../logic2/hexutils.h"
#include "LevelToLogic.h"
#include "../iworld.h"
#include "../skin/shadow.h"
#include "../interface/interface.h"
#include "objectsng.h"
#include "../logic2/logicdefs.h"
#include "../logic2/aiutils.h"
#include "explosionmanager.h"
#include "../common/utils/profiler.h"
#include "scattereditems.h"


// Chopping polygons
#include "chop.h"

//static std::vector< std::pair<BBox, unsigned > > test_boxes;

class GrantedAPI:public LevelAPI
  {
  protected:
    static GameLevel *Lev;
  public://API methods:
    virtual void EndTurn(unsigned Flag)
      {
      if(Lev) Lev->EndTurn(Flag);
      }
    virtual void EnableJoint(int Num,bool Flag)
      {
      unsigned p=HexGrid::GetInst()->GetActiveJoints();
      unsigned mask=1<<Num;
      if(Flag) HexGrid::GetInst()->SetActiveJoints(p|mask);
      else     HexGrid::GetInst()->SetActiveJoints(p&~mask);
      };
  public:
    void SetLevel(GameLevel *lev){Lev=lev;};
    virtual ~GrantedAPI(){};
    GrantedAPI(){SetAPI(this);};
  };
GameLevel *GrantedAPI::Lev=NULL;
GrantedAPI LevAPI;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
const std::string LEVELSDIR("models/");
bool _OldRender=true;
extern float AmbientVol;
extern float EffectsVol;

GameLevel::GameLevel()
{
STACK_GUARD("GameLevel::GameLevel");
	RoofVisible=true;
  LevAPI.SetLevel(this);
	LevelGrid=NULL;
	BspVB=NULL;
	DoMarks=DoLights=true;

}

GameLevel::~GameLevel()
{
STACK_GUARD("GameLevel::~GameLevel");
	Bsp::Close();
	if(LevelGrid) delete LevelGrid;
	if(BspVB) delete BspVB;
}

void GameLevel::Load(std::string FName)
{
STACK_GUARD("GameLevel::Load");
	MakeSaveLoad(FName,false);
	/*LevelName=FName;
	Unload();
	LoadBSP(LEVELSDIR+FName);
	LoadGrid(LEVELSDIR+FName);*/
}

void GameLevel::LoadGrid(std::string FName)
{
STACK_GUARD("GameLevel::LoadGrid");
if(LevelGrid) delete LevelGrid;
    LevelGrid=new Grid(FName+".grid");
}
void GameLevel::LoadBSP(std::string FName)
{
STACK_GUARD("GameLevel::LoadBSP");
	int i;
	FILE *in=fopen((FName+".bsp").c_str(),"rb");
	if(!in) throw CASUS(std::string("Impossible to open File!")+FName+".bsp");
	Bsp::Load(in);
	
	fread(&i,sizeof(int),1,in);
	LevelSounds.reserve(i);
	for(;i;i--)
    {
		SoundUtter snd;
		snd.Load(in);
		//?
		snd.Pos.y=-snd.Pos.y;
		LevelSounds.push_back(snd);
    }
	fread(&i,sizeof(int),1,in);
	LevelEffects.reserve(i);
	for(;i;i--)
    {
		NamedEffect Eff;
		Eff.Load(in);
		//?
		Eff.Position.y=-Eff.Position.y;
		Eff.Front.y=-Eff.Front.y;
		Eff.Up.y=-Eff.Up.y;
		Eff.Right.y=-Eff.Right.y;
		LevelEffects.push_back(Eff);
    }
	LevelObjects.Load(in);
	fread(&i,sizeof(int),1,in);//количество карт освещенности
	/*for(;i;i--)
    {
		int LMnumber;
		fread(&LMnumber,sizeof(int),1,in);//номер шейдера для этой карты высоты
		LMs[LMnumber].Load(in);
		static char d[10]="?";
		sprintf(d,"%d",LMnumber);
		TextureMgr::Instance()->CreateTexture(d,&LMs[LMnumber],0,0);
		//sprintf(d,"%d.bmp",LMnumber);
		//LMs[LMnumber].Save(d);
		//LMs[LMnumber].Save(d.c_str());
		if(feof(in)) break;
    }	*/
	fclose(in);
	
}
void GameLevel::Unload()
{
STACK_GUARD("GameLevel::Unload");
	//int i;
	Stop();

	Bsp::Close();
	if(BspVB) {delete BspVB;BspVB=NULL;}
	if(LevelGrid) delete LevelGrid;	LevelGrid=NULL;
	//LevelGrid.reset(NULL);
	LevelObjects.Clear();
	Marks.clear();
	for(LightmapSet::iterator it=LMs.begin();it!=LMs.end();it++)
    {
		char t[40];
				sprintf(t,"%d",*it);
		while(TextureMgr::Instance()->Release(t));
    }
	LMs.clear();

  EffectSet::iterator ei=LevelEffects.begin();
	for(;ei!=LevelEffects.end();ei++)
    			IWorld::Get()->GetEffects()->DestroyEffect((*ei).id);
	LevelEffects.clear();
	LevelSounds.clear();
	IWorld::Get()->GetPipe()->UnloadShaderFile(std::string("shaders/")+LevelName+".shader");
	ScatteredItem::Clear();

}

void GameLevel::Draw(GraphPipe *Pipe,bool Transparent)
{
STACK_GUARD("GameLevel::Draw");
	int i/* by Flif ,Summ=0*/;

	int ShdNum=Bsp::ShaderNames.size();
	PrepareData(Pipe);  
	/* by Flif LevelVertex *Verts=Bsp::Verts;*/
	for(i=0;i<ShdNum;i++)
	{
		if(!Bsp::VisNum[i]) continue;
		VShader *s=ShaderPtr[i];//Pipe->FindShader(ShaderNames[i]);

		if(!s||(s->Transparent!=Transparent)) continue;
		Pipe->PushBspTrianglesDyn(s,i,BspVB);
	}
//	for(i=0;i<test_boxes.size();i++)
//	{
//		Pipe->DrawBBox(test_boxes[i].first, test_boxes[i].second);
//	}
 /* BusPaths::iterator it,ite;
  it=BPaths.begin();
  ite=BPaths.end();
  for(;it!=ite;it++)
    {
    for(int i=0;i<it->second.Segments.size();i++)
      {
      for(int j=0;j<it->second.Segments[i].size();j++)
        {
        BBox b;
        b.Box(HexGrid::GetInst()->Get(it->second.Segments[i][j]),0.2);
        Pipe->DrawBBox(b,0xfefe34);
        }
      }
    }  */
}
//вызывается перед запросом данных
void GameLevel::PrepareData(GraphPipe *Pipe) 
{
STACK_GUARD("GameLevel::PrepareData");
	Frustum *fru=&Pipe->GetCam()->Cone;
	const Camera* Cam=Pipe->GetCam();
	if(Cam->IsViewChanged()||Cam->IsProjChanged())
    {
    Env.Update(Cam);
		static point3 p,f,r;
		Pipe->GetCamPos(&p,&f,&r);
		if(RoofVisible) Bsp::Traverse(0,p,fru,Bsp::ROOFS);
		else            Bsp::Traverse(0,p,fru,0);
    }
	
}

bool GameLevel::TraceRay(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm, bool Sight)
{
STACK_GUARD("GameLevel::TraceRay");
	float f;

	bool r=Bsp::TraceRay(ray(From,Dir),&f,Norm,(RoofVisible||!Sight)?Bsp::ROOFS:0);
	if(r)
		*Res=From+f*Dir;
	return r;
}


void GameLevel::DoMark(const point3 &Where, const float Rad, const std::string& Shader)
{
STACK_GUARD("GameLevel::DoMark");
	Mark *d=new Mark(Where, Rad, Shader);
	CollectPlanes(d->Pos,Rad, *d);
	Marks.Add(d);
}

void GameLevel::DrawMarks()
{
STACK_GUARD("GameLevel::DrawMarks");
	Marks.Draw();
}
void GameLevel::Start()
{
STACK_GUARD("GameLevel::Start");
HexGrid::GetInst()->SetActiveJoints(0xffffffff);
SoundSet::iterator si=LevelSounds.begin();
while(si!=LevelSounds.end())
	{
	if(si->Type&NamedSound::NS_CYCLED)
		{
		bool Static=(si->Type&NamedSound::NS_STATIC)?true:false;
		std::string b(std::string("sounds/")+si->GetNextName()+".wav");
		
		std::string a;
		if(Static)        a="lodinamiccycled2d";
		else  			a ="lodinamiccycled";
		const ISndScript *snd_script = ISound::instance()->getScript(a.c_str());
		si->m_Emitter = ISound::instance()->createEmitter(snd_script,b.c_str());
		if (!Static) si->m_Emitter->setPosition(si->GetPos());
		si->m_Emitter->play();
		}
	si->LastTimePlayed=Timer::GetSeconds()+2*frand()*((NamedSound*)&*si)->Freq;
	si++;
	}
  EffectSet::iterator ei=LevelEffects.begin();
	while(ei!=LevelEffects.end())
    {
    (*ei).id = IWorld::Get()->GetEffects()->CreateAmbientEffect((*ei).Name,
			(*ei).Position,
			(*ei).Color/255.0);
		
		ei++;
    }
	LevelObjects.Start();
  LevelObjects.UpdateHGFull();
            
}

void GameLevel::Stop()
{
STACK_GUARD("GameLevel::Stop");
	SoundSet::iterator si=LevelSounds.begin();
  while(si!=LevelSounds.end())
    {
    if(si->Type&NamedSound::NS_CYCLED)
      {
		/*	bool Static=(si->Type&NamedSound::NS_STATIC)?true:false;
      if(Static)
        {
        if(si->tr)
          {
          sound::dev->Stop(*si->tr);
          delete si->tr;
          si->tr=NULL;
          }
        }
      else*/
        {
				if(si->m_Emitter)
					{
					si->m_Emitter->stop();
					si->m_Emitter->Release();
					si->m_Emitter = 0;
					}
        }
      }
    si++;
    }
  if(IWorld::Get()->GetEffects())
  {
	  EffectSet::iterator ei=LevelEffects.begin();
	  for(;ei!=LevelEffects.end();ei++)
	  {
		  IWorld::Get()->GetEffects()->DestroyEffect((*ei).id);
	  }
  }

	LevelObjects.Stop();
  ISound::instance()->manage();
}

void GameLevel::CreateVB()
	{
		if(!_OldRender&&Bsp::VertNum)
		{
			unsigned FVFflags=D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1);
			if(BspVB) delete BspVB;
			BspVB = new DynamicVB( D3DKernel::GetD3D(),FVFflags,Bsp::VertNum,sizeof(point3)*2+2*sizeof(texcoord));
			unsigned int startv;
			char *data=(char*)BspVB->Lock(Bsp::VertNum,&startv);
			for(int v=0;v<Bsp::VertNum;v++)
				{
				*(point3*)data            =Bsp::Verts[v].Pos;
				*(point3*)(data+12)       =Bsp::Verts[v].Normal;
				*(texcoord*)(data+12+12)  =Bsp::Verts[v].uv;
				*(texcoord*)(data+12+12+8)=Bsp::Verts[v].lightmapuv;
				data+=12+12+8+8;
				}
			BspVB->Unlock();
		//	BspVB->GetInterface()->Optimize(D3DKernel::GetD3DDevice(),0);
		}
	}

void GameLevel::LinkShaders(GraphPipe *Pipe)
{
STACK_GUARD("GameLevel::LinkShaders");
	ShaderPtr.clear();
	ShaderPtr.reserve(Bsp::ShaderNames.size());
	for(int i=0;i<Bsp::ShaderNames.size();i++)
    {
		VShader *s=Pipe->FindShader(Bsp::ShaderNames[i]);
		ShaderPtr.push_back(s);
    }
	CreateLMtextures();
	LevelObjects.GetShapePool()->CreateVB();
	CreateVB();
}

void GameLevel::CollectPlanes(point3 &Pos, float Rad, BaseMark &m,int CurIndex)
{
STACK_GUARD("GameLevel::CollectPlanes");
	Bsp *node=&Bsp::Nodes[CurIndex];
	Plane *p=Bsp::Planes+node->PlaneIdx;
	
	//LevelBSP *bsp=&BspBSP[CurIndex];
	
	float d=p->TestPoint(Pos);
	if(d>=Rad) //точка в положительном полупространстве и далеко
    {
		if(node->Front)
			CollectPlanes(Pos,Rad,m,node->Front);
    }
	else
		if(d<=-Rad) //точка в отрицательном полупространстве и далеко
		{
			if(node->Back)
				CollectPlanes(Pos,Rad,m,node->Back);
		}
		else
		{
			if(node->Front)
				CollectPlanes(Pos,Rad,m,node->Front);
			if(node->Back)
				CollectPlanes(Pos,Rad,m,node->Back);
			
			
			float _r=(m.Col>>16)&0xff;  //рассчитаем цвет на удалении
			float _g=(m.Col>>8)&0xff;
			float _b=(m.Col)&0xff;
			float alpha=1.f-FastAbs(d/Rad);
			_r*=alpha;_g*=alpha;_b*=alpha;
			unsigned long nCol=RGB_MAKE((int)_r,(int)_g,(int)_b);
			//unsigned long nCol=RGB_MAKE(255,255,255);
			if(!nCol) return; //цвет-то черный - пропустим
			
			point3 ax,ay; //оси в плоскости
			point3 v[3],Center;//вершины треугольника и проекция точки
			float tu[3],tv[3]; //текстурные координаты
			float vmin,vmax,umin,umax; //для отсечения далеких треугольников
			float r=0.5/sqrt(Rad*Rad-d*d); //обратный радиус пятна на этой плоскости (0,5 для масштабирования uv в [-0.5 0.5])
			float sx,sy; //для оптимизации при просчитывании текстурных координат
			
			static point3 axis(0.1,0.124314,0.1); //абстрактная ось
			ax=r*Normalize(p->Normal.Cross(axis));
			ay=r*Normalize(p->Normal.Cross(ax));
			
			Center=Pos-p->Normal*d;
			sx=0.5-Center.Dot(ax);
			sy=0.5-Center.Dot(ay);
			
			
			int i=node->StartFace;
			int cycleend=node->StartFace+node->AllFacesNum;
			/*if(node->flags&LevelBSP::OBSERVERFRONT) i+=node->PosNum;
			else  cycleend=bsp->FirstTri+bsp->PosNum;
			*/
			
			point3 endPoly[10];
			point3 tmpPoly[10];
			point3 bias = p->Normal*0.005;
			int endVertCount, tmpVertCount;
			
			for(;i<cycleend;i++)
			{
        if(Bsp::IsRoof[i]) continue;
				if (m.TriNum>=BaseMark::MAXTRIS-4)  return;
				v[0]=Bsp::Verts[Bsp::Faces[i*3+0]].Pos + bias;
				v[1]=Bsp::Verts[Bsp::Faces[i*3+1]].Pos + bias;
				v[2]=Bsp::Verts[Bsp::Faces[i*3+2]].Pos + bias;

				// Обрежем треугольник кубом, ограничивающим отметину
				if (!Chopper::Chop(v,3,
					point3(1.0f,0.0f,0.0f), Pos+point3(-Rad,0.0f,0.0f),
					tmpPoly,tmpVertCount)) continue;
				if (!Chopper::Chop(tmpPoly,tmpVertCount,
					point3(-1.0f,0.0f,0.0f), Pos+point3(Rad,0.0f,0.0f),
					endPoly,endVertCount)) continue;

				if (!Chopper::Chop(endPoly,endVertCount,
					point3(0.0f,1.0f,0.0f), Pos+point3(0.0f,-Rad,0.0f),
					tmpPoly,tmpVertCount)) continue;
				if (!Chopper::Chop(tmpPoly,tmpVertCount,
					point3(0.0f,-1.0f,0.0f), Pos+point3(0.0f,Rad,0.0f),
					endPoly,endVertCount)) continue;

				if (!Chopper::Chop(endPoly,endVertCount,
					point3(0.0f,0.0f,1.0f), Pos+point3(0.0f,0.0f,-Rad),
					tmpPoly,tmpVertCount)) continue;
				if (!Chopper::Chop(tmpPoly,tmpVertCount,
					point3(0.0f,0.0f,-1.0f), Pos+point3(0.0f,0.0f,Rad),
					endPoly,endVertCount)) continue;

				// Обрезали - остаток добавляем в массив
				v[0] = endPoly[0];
				for (int trs = 0; trs < endVertCount-2; trs++)
				{
					// Надо казус сюда добавить на всякий случай
					if (m.TriNum>=BaseMark::MAXTRIS-4) return;

					v[1] = endPoly[trs+1];
					v[2] = endPoly[trs+2];

					tu[0]=v[0].Dot(ax)+sx;   tv[0]=v[0].Dot(ay)+sy;
					tu[1]=v[1].Dot(ax)+sx;   tv[1]=v[1].Dot(ay)+sy;
					tu[2]=v[2].Dot(ax)+sx;   tv[2]=v[2].Dot(ay)+sy;
					vmin=tv[0]<tv[1]?tv[0]:tv[1];  vmin=vmin<tv[2]?vmin:tv[2];
					umin=tu[0]<tu[1]?tu[0]:tu[1];  umin=umin<tu[2]?umin:tu[2];
					vmax=tv[0]<tv[1]?tv[1]:tv[0];  vmax=vmax>tv[2]?vmax:tv[2];
					umax=tu[0]<tu[1]?tu[1]:tu[0];  umax=umax>tu[2]?umax:tu[2];
					if(vmin>1||vmax<0||umax<0||umin>1) continue;
					
					m.UVs[m.TriNum*3+0]=texcoord(tu[0],tv[0]);
					m.UVs[m.TriNum*3+1]=texcoord(tu[1],tv[1]);
					m.UVs[m.TriNum*3+2]=texcoord(tu[2],tv[2]);
					memcpy(&m.Tris[m.TriNum*3],v,sizeof(point3)*3);
					m.Color[m.TriNum*3+0]=nCol;
					m.Color[m.TriNum*3+1]=nCol;
					m.Color[m.TriNum*3+2]=nCol;
					m.TriNum++;
				}

				// Старый код наложения клякс на геометрию
/*				tu[0]=v[0].Dot(ax)+sx;   tv[0]=v[0].Dot(ay)+sy;
				tu[1]=v[1].Dot(ax)+sx;   tv[1]=v[1].Dot(ay)+sy;
				tu[2]=v[2].Dot(ax)+sx;   tv[2]=v[2].Dot(ay)+sy;
				vmin=tv[0]<tv[1]?tv[0]:tv[1];  vmin=vmin<tv[2]?vmin:tv[2];
				umin=tu[0]<tu[1]?tu[0]:tu[1];  umin=umin<tu[2]?umin:tu[2];
				vmax=tv[0]<tv[1]?tv[1]:tv[0];  vmax=vmax>tv[2]?vmax:tv[2];
				umax=tu[0]<tu[1]?tu[1]:tu[0];  umax=umax>tu[2]?umax:tu[2];
				if(vmin>1||vmax<0||umax<0||umin>1) continue;
				
				m.UVs[m.TriNum*3+0]=texcoord(tu[0],tv[0]);
				m.UVs[m.TriNum*3+1]=texcoord(tu[1],tv[1]);
				m.UVs[m.TriNum*3+2]=texcoord(tu[2],tv[2]);
				memcpy(&m.Tris[m.TriNum*3],v,sizeof(point3)*3);
				m.Color[m.TriNum*3+0]=nCol;
				m.Color[m.TriNum*3+1]=nCol;
				m.Color[m.TriNum*3+2]=nCol;
				m.TriNum++;*/
			}
		}
}

void GameLevel::UpdateMarks(float Time)
{
STACK_GUARD("GameLevel::UpdateMarks");
	Marks.Update(Time);
}

void GameLevel::AddMark(BaseMark *L)
  {
STACK_GUARD("GameLevel::AddMark");

  L->TriNum=0;
	if(DoMarks) CollectPlanes(L->Pos,L->Rad, *L);
  Marks.Add(L);
  }
//обновить состояния объектов ... может и не надо
void GameLevel::UpdateObjects(float Time)
	{
	LevelObjects.Update(Time);
	SoundSet::iterator si=LevelSounds.begin();
  while(si!=LevelSounds.end())
    {
    if(si->Type&NamedSound::NS_RANDOM)
      {
			float ltime=si->LastTimePlayed;
			float interval=Time-ltime;
			float maxinterval=((NamedSound*)&*si)->Freq;
			if(sqrt(frand())*maxinterval<interval)
				{//пора проиграть звук
				std::string a((si->Type&NamedSound::NS_STATIC)?"lodinamic2d":"lodinamic");
				std::string b(std::string("sounds/")+si->GetNextName()+".wav");
				si->LastTimePlayed=si->LastTimePlayed+((NamedSound*)&*si)->Freq;
				const ISndScript *snd_script = ISound::instance()->getScript(a.c_str());
				ISndEmitter *emitter = ISound::instance()->createEmitter(snd_script,b.c_str());
				emitter->setPosition(si->Pos);
				emitter->play();
				emitter->Release();
				}
      }
    si++;
    }
	
	}
//сообщение о конце тура
void GameLevel::EndTurn(unsigned Smth)
  {
STACK_GUARD("GameLevel::EndTurn");
	LevelObjects.EndTurn(Smth);
  }
void GameLevel::LoadEffects(SavSlot &lvleff)
  {
STACK_GUARD("GameLevel::LoadEffects");
  int i;
  Shadows::ClearLights();
		lvleff>>i;
		LevelEffects.reserve(i);
		for(;i;i--)
		{
			NamedEffect Eff;
			Eff.MakeSaveLoad(lvleff);
      if(Eff.Name.find("light(")==0)
        {
        std::istringstream strm(Eff.Name);
        float Power,Dist;
        strm.ignore(strlen("light("));
        strm>>Power;
        strm.ignore(100,',');
        //strm.getline(&t,1);
        strm>>Dist;
        Shadows::AddLight(Eff.Position,Power,Dist);
        }
      else
        {
			LevelEffects.push_back(Eff);
        }
		}
  }

class ProgrBar
	{
	public:
		static void SetTitle(const std::string &form,char *n)
		{
    char buff[1024];
    sprintf(buff, form.c_str(), n);
    Interface::Instance()->ProBar()->SetTitle(buff);
		};
	static void SetMofN(int m,int n)
		{
     Interface::Instance()->ProBar()->SetMofN(m,n);
		}
		static void SetPercent(float p)
			{
  Interface::Instance()->ProBar()->SetPercent(p);
			}
		static void SetRange(float a,float b)
			{
   	Interface::Instance()->ProBar()->SetRange(a,b);
			}
	};

namespace DirtyLinks
	{
	std::string GetStrRes(const std::string& rid); 
	}

void GameLevel::CreateLMtextures(Storage *store)
	{
		int LMnumber;
		if(!Options::GetInt("system.video.lightmaps")) return;
	//выгрузим старые варианты
	LightmapSet::iterator it=LMs.begin();
	for(;it!=LMs.end();it++)
    {
		static char d[10]="?";
		sprintf(d,"%d",*it);
		TextureMgr::Instance()->Release(d);
    }
	LMs.clear();
	 //загрузим новые
	SavSlot *sl=NULL;
	Storage *_st=NULL;
	if(!store)
		{
		std::string nname=(LEVELSDIR+LevelName+".bsp");
		PackageFile  file(nname.c_str());
		_st = new Storage(&file, Storage::SM_LOAD);
		sl = new SavSlot(_st, "LVLLMS");
		}
	else
		{
		sl = new SavSlot(store,"LVLLMS");
		}
	SavSlot &lvllms = *sl;
		lvllms>>LMnumber;
		DIBData LM;
		for(int i=0;i<LMnumber;i++)
			{
			if(store)
				ProgrBar::SetMofN(i+1,LMnumber);
			int lmnum,x,y,bpp,size;
			lvllms>>lmnum;
			lvllms>>x>>y>>bpp>>size;

			LM.Create(x,y,bpp);
			lvllms.Load(LM.Image(),size);
			
			char d[10]="?";
			sprintf(d,"%d",lmnum);
			TextureMgr::Instance()->CreateTexture(d,&LM,0,0);
			LMs.insert(lmnum);

			LM.Release();
			}
		delete sl;
		if(_st) delete _st;
	}

bool GameLevel::MakeSaveLoad(std::string FName,bool IsSaving)
{
STACK_GUARD("GameLevel::MakeSaveLoad(file)");
//Interface::Instance()->UpdateLoadingProgressBar(float);
std::string oldname=LevelName;

char LevRealName[2048];
strcpy(LevRealName,IWorld::Get()->GetLevelName(FName).c_str());


	static const float GEOM_S=0,   GEOM_F=0.05;
	static const float  SND_S=0.05, SND_F=0.075;
	static const float  EFF_S=0.075, EFF_F=0.1;
	static const float  OBJ_S=0.1, OBJ_F=0.35;
	static const float  LGH_S=0.35, LGH_F=0.4;
	static const float  MOV_S=0.4, MOV_F=0.5;
  ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevel"),LevRealName);


	Unload();
	LevelName=FName;

		if(oldname!=LevelName)
			IWorld::Get()->GetPipe()->UnloadShaderFile(std::string("shaders/")+oldname+".shader");


	std::string nname=(LEVELSDIR+FName+".bsp");
	PackageFile  file(nname.c_str());
	//StdFile  file(nname.c_str(), IsSaving?"wb":"rb");
	Storage  st(&file, IsSaving?Storage::SM_SAVE:Storage::SM_LOAD);
	SavSlot bspcell(&st, "LVLBSP");
	SavSlot lvlsnd(&st, "LVLSND");
	SavSlot lvleff(&st, "LVLEFF");
	SavSlot lvlobj(&st, "LVLOBJ");



  
  ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevelgeom"),LevRealName);
	ProgrBar::SetRange(GEOM_S,GEOM_F);
  ProgrBar::SetPercent(0);
	{
		STACK_GUARD("Bsp::MakeSaveLoad");
   	Bsp::MakeSaveLoad(bspcell);
	}
  ProgrBar::SetPercent(1);


	int i;
	if(IsSaving)
    {
		lvlsnd<<LevelSounds.size();
		for(i=0;i<LevelSounds.size();i++)
			LevelSounds[i].MakeSaveLoad(lvlsnd);
		lvleff<<LevelEffects.size();
		for(i=0;i<LevelEffects.size();i++)
			LevelEffects[i].MakeSaveLoad(lvleff);
		
		LevelObjects.MakeSaveLoad(lvlobj);  
			/*lvllms<<LMnumber;
			for(i=;i<LMnumber;i--)
			{
			lvllms<<LMs[i].Width()<<LMs[i].Height()<<LMs[i].Bpp()<<LMs[i].ImageSize();
			lvllms.Save(LMs[i].Image(),LMs[i].ImageSize());
	}*/
    }
	else
    {
		int j;
		lvlsnd>>i;
		LevelSounds.reserve(i);
    ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevelsound"),LevRealName);
  	ProgrBar::SetRange(SND_S,SND_F);
    ProgrBar::SetPercent(0);
		for(j=0;j<i;j++)
		{
     ProgrBar::SetMofN(j+1,i);

		 SoundUtter snd;
		 snd.MakeSaveLoad(lvlsnd);
		LevelSounds.push_back(snd);
		}

    ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadleveleff"),LevRealName);
  	ProgrBar::SetRange(EFF_S,EFF_F);
    ProgrBar::SetPercent(0);
    LoadEffects(lvleff);
    ProgrBar::SetPercent(1);


    ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevelobj"),LevRealName);
  	ProgrBar::SetRange(OBJ_S,OBJ_F);
    ProgrBar::SetPercent(0);
		LevelObjects.MakeSaveLoad(lvlobj);
    ProgrBar::SetPercent(1);


    ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevellght"),LevRealName);
  	ProgrBar::SetRange(LGH_S,LGH_F);
    ProgrBar::SetPercent(0);

		CreateLMtextures(&st);
    ProgrBar::SetPercent(1);



    ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevelpass"),LevRealName);
  	ProgrBar::SetRange(MOV_S,MOV_F);
    ProgrBar::SetPercent(0);

		LoadGrid(LEVELSDIR+FName);
 	  LevelAPI::GetAPI()->EnableJoint(0,true); LevelAPI::GetAPI()->EnableJoint(1,true);
 	  LevelAPI::GetAPI()->EnableJoint(2,true); LevelAPI::GetAPI()->EnableJoint(3,true);
 	  LevelAPI::GetAPI()->EnableJoint(4,true); LevelAPI::GetAPI()->EnableJoint(5,true);
 	  LevelAPI::GetAPI()->EnableJoint(6,true); LevelAPI::GetAPI()->EnableJoint(7,true);
    ProgrBar::SetPercent(1);

    try
      {
	 SavSlot lvlspl(&st, "LVLSPL"); 
   LoadSplines(lvlspl);
      }catch(CasusImprovisus &)
      {;}


      LoadCameras();

			CreateVB();
			
    }
    ProgrBar::SetTitle(DirtyLinks::GetStrRes("ld_loadlevelend"),LevRealName);
	LevelObjects.UpdateHGFull();
  Env.LevelChanged(FName,IWorld::Get()->GetPipe());
	return true;
}
void GameLevel::LoadCameras()
  {
STACK_GUARD("GameLevel::LoadCameras");
  std::string name="models/"+LevelName+".cameras";
	PackageFile  file(name.c_str());
  CamPaths.clear();
  if(file.IsOk())
    {
		 file.Seek(0,SEEK_END);int size=file.GetPos(); file.Seek(0,SEEK_SET);
     char *t=new char[size+1];
		 t[size]=0;
     file.Read(t,size);
     std::istringstream stream(t);
		 delete t;

     int camnum;
     (stream.ignore(strlen("Cameras Number:"))>>camnum).ignore(strlen("\n"));
     while(camnum--)
       {
       std::string CamName;
       KeyAnimation ka;
       (stream>>CamName).ignore(strlen("\n"));
       ka.Load(stream);
       CamPaths[CamName]=ka;
       }
    }
  }
void GameLevel::LoadSplines(SavSlot &lvlspl)
  {
STACK_GUARD("GameLevel::LoadSplines");
  int num;
  lvlspl>>num;
  
  for(int i=0;i<num;i++)
    {
    std::string Name;
    unsigned Type;
    int PntNum;
    
    lvlspl>>Name;
    lvlspl>>Type;
    lvlspl>>PntNum;
    if(Type==0)//грузим как автобусный путь
      {
      BusPath Bus;
      Bus.Name=Name;
      int n=0;
        std::vector<point3> path;
        int startpos=0,endpos=0;
      while(n<PntNum)
        {
        for(;n<PntNum;n++,endpos++) //накопим точки до остановки
          {
          point3 Pos,Dir;
          bool Stop;
          lvlspl>>Pos;
          lvlspl>>Dir;
          lvlspl>>Stop;
          path.push_back(Pos);
          if(Stop) {endpos++;n++;break;}
          }
         //превратим точки в путь по хексам
        BusPath::PathSeg seg;
        point3 dot;
        ipnt2_t hpnt=HexUtils::scr2hex(path[0]);
        //seg.push_back(hpnt);
        
        for(int i=startpos;i<endpos-1;i++)
          {
          float len=4*(path[i+1]-path[i]).Length();
          for(float f=0;f<1;f+=1.f/len)
            {
            dot=f*path[i+1]+(1-f)*path[i];
            hpnt=HexUtils::scr2hex(dot);
            if(!seg.size()||seg.back()!=hpnt)
              seg.push_back(hpnt);
            }
          }
        startpos=endpos-1;
        Bus.Segments.push_back(seg);
        }
      BPaths[Bus.Name]=Bus;
      }
    else
      {
      DestPoints Points;
      Points.Name=Name;
      for(int n=0;n<PntNum;n++)
        {
        DestPoints::DestPoint p;
        bool Stop;
        lvlspl>>p.Pos;
        lvlspl>>p.Dir;
        lvlspl>>Stop;
        p.Dir=Normalize(p.Dir);
        p.Special=Stop?1:0;
        Points.Points.push_back(p);
        }
       PPoints[Points.Name]=Points;
      }
    
    }
  }

bool GameLevel::MakeSaveLoad(Storage &st)
{
STACK_GUARD("GameLevel::MakeSaveLoad(storage)");
	SavSlot cell(&st, "Lvdta");
	SavSlot lvlspots(&st, "LVLSPOTS");
	if(cell.IsSaving())
    {//Save
		cell<<LevelName;
		
		LevelObjects.MakeSaveLoad(cell);
		
		Marks.MakeSaveLoad(cell);
		//...
		//cell.Save((void*)punch, strlen(punch) + 1); 
    }
	else
    {//Load
		std::string oldname=LevelName;
		cell>>LevelName;

		if(oldname!=LevelName)
			IWorld::Get()->GetPipe()->UnloadShaderFile(std::string("shaders/")+oldname+".shader");

		Unload();

		Load(LevelName);
		LevelObjects.ClearObjectInfluence();
		LevelObjects.Clear();
		
		IWorld::Get()->GetPipe()->BindToGrid(LevelGrid,point3(0,0,0));
		IWorld::Get()->GetPipe()->GetCam()->Move(NULLVEC);
		
		std::string name=std::string("shaders/")+LevelName+".shader";
		IWorld::Get()->GetPipe()->RegisterShaderFile(name.c_str());
		LinkShaders(IWorld::Get()->GetPipe());
		
		ProgrBar::SetRange(0.5,0.5);
		ProgrBar::SetPercent(0);
		LevelObjects.MakeSaveLoad(cell);
		Marks.MakeSaveLoad(cell);
   	LevelObjects.UpdateHGFull();
    Env.LevelChanged(LevelName,IWorld::Get()->GetPipe());
    }
  IWorld::Get()->GetEffects()->MakeSaveLoad(lvlspots);
	return true;
}


void GameLevel::EnableMarks(MARK_TYPE type, bool enable)
	{
STACK_GUARD("GameLevel::EnableMarks");
	switch(type)
		{
		case MT_STATIC: DoMarks=enable;break;
		case MT_DYNAMIC: DoLights=enable;break;
		}
	}
bool GameLevel::MarksEnabled(MARK_TYPE type)
	{
STACK_GUARD("GameLevel::MarksEnabled");
	switch(type)
		{
		case MT_STATIC: return DoMarks;
		case MT_DYNAMIC: return DoLights;
		}
	return false;
	}



bool TraceSegment(const point3 &Pos, const point3 &Dir, point3 *Pnt, point3 *Norm, float *TimePart,int CurIndex=0)
	{
STACK_GUARD("TraceSegment");
	Bsp *node=&Bsp::Nodes[CurIndex];
	Plane *p=Bsp::Planes+node->PlaneIdx;

	float dist=p->Normal.Dot(Dir);
	float d=p->TestPoint(Pos);
	if((d>0&&dist>0)||(d>fabs(dist))) //точка в положительном полупространстве и далеко
    {
		if(node->Front)
			return TraceSegment(Pos,Dir,Pnt,Norm,TimePart,node->Front);
    }
	else
		if((d<0&&dist<0)||(d<-fabs(dist))) //точка в отрицательном полупространстве и далеко
		{
			if(node->Back)
			return TraceSegment(Pos,Dir,Pnt,Norm,TimePart,node->Back);
		}
		else
			{
			unsigned n1=d<0?node->Back:node->Front;
			unsigned n2=d<0?node->Front:node->Back;
			if(n1&&TraceSegment(Pos,Dir,Pnt,Norm,TimePart,n1)) 	return true;
			//а здесь проверим на попадание в нашу плоскость
			int i=node->StartFace;
			int cycleend=node->StartFace+node->AllFacesNum;
			float t,u,v;
			for(;i<cycleend;i++)
				{
				static Triangle a;
				a.V[0]=Bsp::Verts[Bsp::Faces[i*3+0]].Pos;	a.V[1]=Bsp::Verts[Bsp::Faces[i*3+1]].Pos;	a.V[2]=Bsp::Verts[Bsp::Faces[i*3+2]].Pos;
				if(a.RayTriangle(Pos,Dir,&t,&u,&v))
					{
					*Norm=p->Normal;
					*Pnt=Pos+t*Dir;
					*TimePart=t;
					return true;
					}
				}
			if(n2) return TraceSegment(Pos,Dir,Pnt,Norm,TimePart,n2);
			}
		return false;
	}
void TraceGrenade(const point3 &_pos, const point3 &_dir, KeyAnimation *trace, CharacterPool::Handle Skip, float MaxTime=10.f)
{
STACK_GUARD("TraceGrenade");
//test_boxes.clear();
	const float DAMPING=0.8;
	//на входе точка и направление, направление подразумевает и силу вылета
	static const float step=0.5;
	int i=0;
	point3 dir(_dir),pos(_pos);

	GameLevel *lev=IWorld::Get()->GetLevel();

	while(i++<1000)
	{
		
		//1.найдем отрезок пути/времени
		float vel=dir.Length();
		float steptime=step/vel;
		float timepart = 0.0f;
		bool stop=false;
		//2. проверим нет ли пересечения с миром
		point3 stepdir(dir/vel*step);
		static point3 newpos,ColNorm,newdir;
		//FIXME: нужно использовать TraceSegment
		//bool wascollision=TraceSegment(pos,stepdir,&newpos,&ColNorm,&timepart);
		bool wascollision=lev->TraceRay(pos,stepdir, &newpos, &ColNorm);
		if(wascollision)
			{
			timepart=0.5*(newpos-pos).Length()/stepdir.Length();
			if(timepart>1.f)
				wascollision=false;
			newpos=pos+stepdir*timepart;
			}
		
		
		std::string ObjName;
		point3 objpos,objnorm;
		bool wascol=lev->LevelObjects.TraceSegment(pos,stepdir,&objpos,&objnorm,&ObjName);
		if(wascol)
		{
			float opart=(objpos-pos).Length()/stepdir.Length();
			if(!wascollision||opart<timepart)
			{
				newpos=objpos;
				ColNorm=objnorm;
				timepart=opart;
				wascollision=true;
			}
		}
		point3 cpos,cnorm;
		CharacterPool::Handle Who;
		bool poolcol=IWorld::Get()->GetCharacterPool()->TraceSegment(pos,stepdir,&cpos,&cnorm,&Who);
		if(poolcol&&Who!=Skip)
		{
			float opart=(cpos-pos).Length()/stepdir.Length();
			if(!wascollision||opart<timepart)
			{
				newpos=cpos;
				ColNorm=cnorm;
				timepart=opart;
				wascollision=true;
			}
		}
		
		//отскакивание от силовых полей
		ShieldPool::iterator s1=ShieldPool::GetInst()->begin(),se=ShieldPool::GetInst()->end();
		point3 N,D;
		for(;s1!=se;s1++)
		{
			Cylinder c(s1->GetPos(),3, s1->GetRadius());
			if(c.TraceSegment(pos,stepdir,&D,&N))
			{
				float opart=(D-pos).Length()/stepdir.Length();
				if(!wascollision||opart<timepart)
				{
					newpos=D;
					ColNorm=N;
					timepart=opart;
					wascollision=true;
				}
			}
		}
		
		if(wascollision)	//было столкновение
		{
			point3 Nsh=ColNorm*ColNorm.Dot(-stepdir);
			newdir=2*Nsh+stepdir;
			float cosa=fabs(Normalize(ColNorm).Dot(Normalize(stepdir)));
			newpos+=Normalize(newdir)*0.01;
			newdir*=(1.f-cosa)*DAMPING+(1.f-DAMPING);
			if(ColNorm.z>0)
			 newdir*=0.8+fabs(0.2*(1.f-ColNorm.z*ColNorm.z));
		}
		else			//полет свободный
		{
			newpos=pos+stepdir;
			newdir=stepdir;
			timepart=1.f;
		}
		{
			BBox bo;bo.Box(newpos,0.02);
			unsigned col = wascollision?0xff8080:0x80ff80;
//			test_boxes.push_back( std::pair<BBox,unsigned>( bo,col)   );
		}
		//3. занесем позицию
		trace->SetKey(trace->GetLastTime()+steptime*timepart,newpos);
		//4. если нужно продолжим
		dir=0.999*(newdir*vel/step);
		vel=dir.Length();
		if(wascollision)
		{
		/*if(fabs(ColNorm.z)>0.8)
		if(vel<1.0f)
		stop=true;
		else
		{
		dir*=0.6;//чтобы не прыгала, сволочь
		dir.z*=0.8;
		}
			if(dir.z/hypot(dir.x,dir.y)>4) dir*=0.8;*/
			
			if(vel<fabs(ColNorm.z)/2) stop=true;
			else if(vel<2.1) stop=true;
			
		}
		
		if(stop||trace->GetLastTime()>MaxTime) break;
		//if(vel<0.3f) break;
		//5. применим силу тяготения
		dir-=9.8*AXISZ*steptime*timepart;
		pos=newpos;
		}
	}
	
	//
//реализация DynObject
//
void DynObjectPool::MakeSaveLoad(SavSlot &s)
{
STACK_GUARD("DynObjectPool::MakeSaveLoad");
	int i,j;
	if(s.IsSaving())
		{
		s<<m_Objects.size();  
		ObjectsIt it=m_Objects.begin();
		for(;it!=m_Objects.end();it++)
			it->second.MakeSaveLoad(s);
		}
	else
		{
		s>>i;  
		GetShapePool()->Clear();
		for(j=0;j<i;j++)
		{
		ProgrBar::SetMofN(j,i);
			DynObject Obj;
			Obj.MakeSaveLoad(s);
			if(Obj.PartNum&&Obj.Parts[0]->PntNum)
			{
				//fixme: таких объектов не должно быть - записать их в лог.
  		Obj.m_MyShape=GetShapePool()->AddShape(&Obj);
			m_Objects.insert(ObjContaner::value_type(Obj.Name,Obj));
			}
		}
		GetShapePool()->CreateVB();
		Update(0);
	}
}
ShapePool *DynObjectPool::m_Shapes=NULL;
DynObjectPool::~DynObjectPool()
	{
STACK_GUARD("DynObjectPool::~DynObjectPool");
  if(m_Shapes) {delete m_Shapes;m_Shapes=NULL;}
	}

ShapePool* DynObjectPool::GetShapePool()
		{
		if(!m_Shapes)
			m_Shapes=new ShapePool;
		return m_Shapes;
		}

void DynObjectPool::Update(float Time)
{
STACK_GUARD("DynObjectPool::Update");
	ObjectsIt it=m_Objects.begin(),ite=m_Objects.end();
	while(it!=ite)
	{
    DynObject &obj=it->second;
    bool needupdate=obj.Animation;
    obj.Update(Time);
    if(needupdate)
      UpdateHG(&obj);
		it++;
	}
}
float DynObjectPool::SwitchState(const std::string& Name,float Time,SWITCH_STATE State)
	{
	STACK_GUARD("DynObjectPool::SwitchState");
	std::pair<ObjectsIt,ObjectsIt> range=m_Objects.equal_range(Name);
	float st=-1;
	float time=0;
	ObjectsIt it=range.first,ite=range.second;
	
	bool Locked=false; //Объекты однажды открытые не должны закрываться.
	if(Grid *LevelGrid=(Grid*)HexGrid::GetInst())
		{
		if(LevelGrid->ManDirect[Name].size()) Locked=true;
		if(LevelGrid->ManInvert[Name].size()) Locked=true;
		if(LevelGrid->HeightDirect[Name].size()) Locked=false;
		}
	
	for(;it!=ite;it++)
		{
		DynObject &obj=it->second;
		
		switch(State)
			{
				case SS_SWITCH: if(st==-1) st=obj.State<0.5?1:0;break;
				case SS_OFF: st=0;break;
				case SS_ON: st=1;break;
				}
			if(Locked && st==1 && obj.State<0.5) 
				obj.UseOptions.Usable=false;

			obj.ChangeState(st,Time);
			float otime=obj.GetLastTime();
			if(otime>time) time=otime;
			UpdateHG(&obj);
			}
		return Time+time+0.2;
	}

class rec_handler
	{
	private: int &c;
	public:
		rec_handler(int &counter):c(counter){c++;};
	~rec_handler(){c--;}

	};
void DynObjectPool::DoExplosion(std::string ObjName,const hit_s& hit, LogicDamageStrategy *LogicStrategy)//При взрыве уничтожаются объекты...
  {
STACK_GUARD("DynObjectPool::DoExplosion");
static int recursion_level=0;
rec_handler rh(recursion_level);
if(recursion_level>3) return;




	typedef std::set<std::string> ObjectNames;
	ObjectNames ObjNames;
	ObjectsIt it=m_Objects.begin();

	hit_s NewHit=hit;
  for(int n=0;n<hit_s::MAX_DMSGS;n++)
	{
		if(NewHit.m_dmg[n].Type!=hit_s::DT_STRIKE&&NewHit.m_dmg[n].Type!=hit_s::DT_FLAME&&NewHit.m_dmg[n].Type!=hit_s::DT_EXPLOSIVE)
      NewHit.m_dmg[n].Type=hit_s::DT_NONE;
    if(LogicStrategy&&NewHit.m_dmg[n].Type==hit_s::DT_FLAME)
      LogicStrategy->LightFire(NewHit.m_to, NewHit.m_radius, NewHit.m_dmg[n].Value);

	}
	
	
	for(;it!=m_Objects.end();it++)
	{
		DynObject &obj=it->second;
		if(!obj.Destruct.Destructable) continue;
		
		float falloff=1;
		bool thisobj=(obj.Name==ObjName);
		if(!thisobj)
		{
			if(hit.m_radius==0.f) continue;
			float dist=obj.Bound.DistToPoint(hit.m_to);
			if(dist>hit.m_radius) continue;
			falloff=dist>1.f?1.f/(dist*dist*dist):1.f;
		}
		int HP=obj.Destruct.HitPoints;
		//!FIXME:
		if((NewHit.m_dmg[0].Type==hit_s::DT_NONE||HP>=NewHit.m_dmg[0].Value*falloff)&&
			(NewHit.m_dmg[1].Type==hit_s::DT_NONE||HP>=NewHit.m_dmg[1].Value*falloff)&&
			(NewHit.m_dmg[2].Type==hit_s::DT_NONE||HP>=NewHit.m_dmg[2].Value*falloff))
			continue;
		ObjNames.insert(obj.Name);
		obj.Sounder->Stop();
		//Это чтобы правильно изменить зоны проходимости
		ChangeHG(obj.Name,1,obj.World._43);
	}
	
	//инициируем взрыв всех объектов
  std::vector<hit_s> Hits;
  for(ObjectNames::iterator i = ObjNames.begin(); i != ObjNames.end(); i++)
    {
    std::pair<ObjectsIt,ObjectsIt> range=m_Objects.equal_range(*i);
    if(range.first!=m_Objects.end())
      {
      hit_s Hit=range.first->second.Destruct.Damage;
      Hit.m_from=Hit.m_to=range.first->second.Bound.GetCenter();
      Hits.push_back(Hit);
      }
    if(LogicStrategy) LogicStrategy->DestroyObject(*i);
    IWorld::Get()->GetEffects()->DestroyObjectByExplosion(hit.m_to, (*i));
    //if(LogicStrategy) LogicStrategy->LightFire(hit.m_to, hit.m_radius, 15/*?*/);
    }
  //а теперь обработка взрыва всех собранных объектов
  return;//fixme:
  for(int j=0;j<Hits.size();j++)
    {
    ExplosionManager::Get()->OnAirHit(-1,Hits[j]);
    }
    
  }


  bool DynObjectPool::TraceRay(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm,std::string *Name, DynObjectPool::TRACE_TYPE trace_type)
{
STACK_GUARD("DynObjectPool::TraceRay");
	static const float BOGUS=100000;
	float dist=BOGUS;
	point3 norm,pnt;
	float d;

	ObjectsIt it=m_Objects.begin(),ite=m_Objects.end();
	for(;it!=ite;it++)
    {
		DynObject &obj=it->second;
    if(trace_type==TT_SIGHT){ if(obj.Transparent) continue;}
    else    if(trace_type==TT_PHYSIC) {if(obj.Ghost) continue;}
		if(obj.TraceRay(ray(From,Dir),&d,&norm))
		{
			if(d<dist)
			{
				*Norm=norm;
				*Res=From+d*Dir;
				dist=d;
				*Name=obj.Name;
			}
		}
    }

	return dist==BOGUS?false:true;
}
bool DynObjectPool::TraceSegment(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm,std::string *Name)
{
STACK_GUARD("DynObjectPool::TraceSegment");
	static const float BOGUS=100000;
	float dist=BOGUS;
	point3 norm,pnt;
	float d;

	point3 start_pnt=From,end_pnt=From+Dir,mid_pnt=From+0.5*Dir;
	float Len=Dir.Length()/2; //половина длины сегмента
	ObjectsIt it=m_Objects.begin(),ite=m_Objects.end();
	for(;it!=ite;it++)
    {
		DynObject &obj=it->second;
		if(obj.Ghost) continue;//FIXME: нужен флаг
		if(obj.Bound.DistToPoint(mid_pnt)>Len)
			continue;

		if(obj.TraceRay(ray(From,Dir),&d,&norm))
		{
			if(d<dist)
			{
				*Norm=norm;
				*Res=From+d*Dir;
				dist=d;
				*Name=obj.Name;
			}
		}
    }

	return dist==BOGUS?false:true;
}
void DynObjectPool::Clear()
{
STACK_GUARD("DynObjectPool::Clear");
	m_Objects.clear();
	UpdateHGFull();
	if(m_Shapes)m_Shapes->Clear();
}
void DynObjectPool::UpdateHG(DynObject *Obj)
{
STACK_GUARD("DynObjectPool::UpdateHG");
	float state=Obj->State;//<0.5?0.f:1.f;
	ChangeHG(Obj->Name,state,Obj->World._43);
}
void DynObjectPool::UpdateHGFull()
	{
STACK_GUARD("DynObjectPool::UpdateHGFull");
	ObjectsIt it=m_Objects.begin(),ite=m_Objects.end();
	Grid *LevelGrid=(Grid*)HexGrid::GetInst();
	if(!LevelGrid) return;

	for(;it!=ite;it++)
    {
		DynObject &obj=it->second;
   	//--------------------------------------------
		HexGrid::CellVec *vect=&LevelGrid->HeightDirect[obj.Name]; //fixme: здесь идет заполнение поля "лифт" у объекта - очень коряво
		if(vect->size())  
			obj.CaseInfo.Elevator=true;
    //--------------------------------------------
		UpdateHG(&obj);
    }
	}
bool CanPass(const ipnt2_t &np)
	{
STACK_GUARD("CanPass");
	for(int p=0;p<6;p++)
		{
		if(HexGrid::GetInst()->Get(np).GetWeight(p)<50) return true;
				}
	return false;
	}
void DynObjectPool::ClearObjectInfluence()
	{
STACK_GUARD("DynObjectPool::ClearObjectInfluence");
	ObjectsIt it=m_Objects.begin(),ite=m_Objects.end();
	Grid *LevelGrid=(Grid*)HexGrid::GetInst();
	if(!LevelGrid) return;
	for(;it!=ite;it++)
    {
		DynObject &obj=it->second;
		ChangeHG(obj.Name,1,obj.World._43);
    }
	
	}
void DynObjectPool::ChangeHG(const std::string &ObjName,float _State, float z)
{
STACK_GUARD("DynObjectPool::ChangeHG");
bool State=_State>0.5?true:false;
	Grid *LevelGrid=(Grid*)HexGrid::GetInst();
	if(!LevelGrid) return;
	ipnt2_t np;
	HexGrid::CellVec *vec;
	vec=&LevelGrid->ManDirect[ObjName];
	if(vec->size())
	{
		for(int i=0;i<vec->size();i++)
		{
			for(int p=0;p<6;p++)
			{
				HexUtils::GetFrontPnts0((*vec)[i], p, &np);
				if(LevelGrid->IsOutOfRange(np)) 
					{
  				LevelGrid->Get((*vec)[i]).SetWeight(p,100);//?FIXME:
					continue;
					}
				if(State&&!CanPass(np))
					if(LevelGrid->Get((*vec)[i]).GetProperty()!=LevelGrid->Get(np).GetProperty())
					continue;
				LevelGrid->Get(np).SetWeight(HexUtils::GetReverseDir(p), State?0:100);
				LevelGrid->Get((*vec)[i]).SetWeight(p, State?0:100);
				
			}
		}
	}
	vec=&LevelGrid->ManInvert[ObjName];
	if(vec->size())
	{
		for(int i=0;i<vec->size();i++)
		{
			for(int p=0;p<6;p++)
			{
				HexUtils::GetFrontPnts0((*vec)[i], p, &np);
				if(LevelGrid->IsOutOfRange(np)){LevelGrid->Get((*vec)[i]).SetWeight(p, 100); continue;}
				if(!State&&!CanPass(np))
					if(LevelGrid->Get((*vec)[i]).GetProperty()!=LevelGrid->Get(np).GetProperty())
					continue;
				LevelGrid->Get((*vec)[i]).SetWeight(p,State?100:0);
				LevelGrid->Get(np).SetWeight(HexUtils::GetReverseDir(p), State?100:0);
			}
		}
	} 
	vec=&LevelGrid->HeightDirect[ObjName];
	if(vec->size())
	{
		for(int i=0;i<vec->size();i++)
		{
			LevelGrid->Get((*vec)[i]).z=z;
		}
		HexGrid::HFrontVec *frontvec=&LevelGrid->HeightFront[ObjName];
		HexGrid::HFrontVec::iterator it=frontvec->begin(),ite=frontvec->end();
		for(;it!=ite;it++)
		{
			ipnt2_t np_f(HexUtils::scr2hex(*it->from));
			ipnt2_t np_t(HexUtils::scr2hex(*it->to));
			bool dh_f,dh_t;
			dh_f=LevelGrid->GetProp(np_f).DynHeight;
			dh_t=LevelGrid->GetProp(np_t).DynHeight;
			if( (dh_f||CanPass(np_f)) && (dh_t||CanPass(np_t)) )
			{
				if(FastAbs(it->from->z - it->to->z)>0.6)
				{
					it->from->SetWeight(it->dir, 100);
					it->to->SetWeight(HexUtils::GetReverseDir(it->dir), 100);
				}
				else
				{
					it->from->SetWeight(it->dir, 0);
					it->to->SetWeight(HexUtils::GetReverseDir(it->dir), 0);
				}
			}
		}
	}
}

void DynObjectPool::CollectHexesForObject(const std::string &ObjName,std::vector<ipnt2_t> *vec)
{
STACK_GUARD("DynObjectPool::CollectHexesForObject");
//три варианта:
//1.объект с динамической проходимостью
//2.объект с динамической высотой
//3.произвольный объект
	ipnt2_t np;
	Grid *LevelGrid=(Grid*)HexGrid::GetInst();
	HexGrid::CellVec *vect;
	vect=&LevelGrid->ManDirect[ObjName];
	if(vect->size())
	{
		for(int i=0;i<vect->size();i++)
		{
      vec->push_back((*vect)[i]);
			for(int p=0;p<6;p++)
			{
				HexUtils::GetFrontPnts0((*vect)[i], p, &np);
				if(LevelGrid->IsOutOfRange(np)) 	continue;
				if(LevelGrid->Get((*vect)[i]).GetProperty()==LevelGrid->Get(np).GetProperty()) continue;
        vec->push_back(np);
			}
		}
    return;
	}
	vect=&LevelGrid->HeightDirect[ObjName];
	if(vect->size())
	{
		for(int i=0;i<vect->size();i++)
		{
      vec->push_back((*vect)[i]);
		}
    return;
	}




	int cx,cy;
  std::pair<ObjectsIt,ObjectsIt> range=m_Objects.equal_range(ObjName);
	ObjectsIt st,en=range.second;
	for(st=range.first;st!=en;st++)
	{
	//1. найдем центральную точку
		DynObject &obj=st->second;
		point3 pnt=obj.LocalBound.GetCenter(),tpnt;
		PointMatrixMultiply(*(D3DVECTOR*)&tpnt,*(D3DVECTOR*)&pnt,obj.World);
    ipnt2_t hpnt=HexUtils::scr2hex(tpnt);
	//2. отступим немного
		float rad=obj.LocalBound.GetBigRadius()+2;
		int stepx=ceil(rad);
		int stepy=1.f/sin(TORAD(60))*ceil(rad);

	//3. пройдемся по полученному множеству и пометим близкие к объекту хексы
		for(cx=hpnt.x-stepx;cx<hpnt.x+stepx;cx++)
		for(cy=hpnt.y-stepy;cy<hpnt.y+stepy;cy++)
		{
			if(cx<0||cy<0||cx>=HexGrid::GetInst()->GetSizeX()||cy>=HexGrid::GetInst()->GetSizeY())
				continue;
			HexGrid::cell *c=&HexGrid::GetInst()->Get(ipnt2_t(cx,cy));

			point3 hexpos=*c;
	   	point3 pnt;
  		PointMatrixMultiply(*(D3DVECTOR*)&pnt,*(D3DVECTOR*)&hexpos,obj.InvWorld);
			float dist=obj.LocalBound.DistToPoint(pnt);
			if(dist>2) continue;
			vec->push_back(ipnt2_t(cx,cy));
		}

	}	
}
void DynObjectPool::Load(FILE *in)
{
STACK_GUARD("DynObjectPool::Load");
	int i;
	fread(&i,sizeof(int),1,in);//Объекты
	for(;i;i--)
    {
		DynObject Obj;
		Obj.Load(in);
		m_Objects.insert(ObjContaner::value_type(Obj.Name,Obj));
    }
}
void DynObjectPool::Start()
{
STACK_GUARD("DynObjectPool::Start");
	ObjectsIt it=m_Objects.begin(),ite=m_Objects.end();
	for(;it!=ite;it++)
		{
		/*DynObject &o=it->second;
		if(o.Name=="ffield")
			int t=0;*/
    if(it->second.Sounder)it->second.Sounder->Start();
		}
}
void DynObjectPool::Stop()
{
STACK_GUARD("DynObjectPool::Stop");
	ObjectsIt it=m_Objects.begin(),ite=m_Objects.end();
	for(;it!=ite;it++)
    if(it->second.Sounder)it->second.Sounder->Stop();
}
void DynObjectPool::EndTurn(unsigned Smth)
{
		ObjectsIt it=m_Objects.begin();
		while(it!=m_Objects.end())
    {
			DynObject &obj=it->second;
			/* by Flif bool needupdate=obj.Animation;*/
			if(obj.StationInfo.HasStation)
      {
        obj.EndTurn(Smth);
      }
			it++;
    }
}
void DynObjectPool::SetStationInfo(const std::string &Name,const Station& st)
{
STACK_GUARD("DynObjectPool::SetStationInfo");
	std::pair<ObjectsIt,ObjectsIt> range=m_Objects.equal_range(Name);
         ObjectsIt it=range.first;
         for(;it!=range.second;it++)
           it->second.StationInfo=st;
}

void DynObjectPool::Draw(GraphPipe *Pipe, bool /*trans*/)
{
STACK_GUARD("DynObjectPool::Draw");
	int oc=0;
	ObjectsIt it=m_Objects.begin(),ite=m_Objects.end();
	
	const Frustum *f=&Pipe->GetCam()->Cone;
	while(it!=ite)
	{
		DynObject &obj=it->second;
		//FIXME: необходимо делать это гораздо реже
		if(f->TestBBox(obj.Bound)==Frustum::NOTVISIBLE) {it++;continue;}
		//D3DKernel::GetD3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD,&obj.World);
		//Pipe->Chop(&obj);
		GetShapePool()->PushToDraw(obj.m_MyShape,obj.World);
		oc++;
		it++;
	}
	GetShapePool()->DrawAll();
	
	StatesManager::SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX*)IDENTITYMAT);
}
void DynObjectPool::EraseByName(const std::string& Name)
  {
STACK_GUARD("DynObjectPool::EraseByName");
  std::pair<ObjectsIt,ObjectsIt> range=m_Objects.equal_range(Name);
  ObjectsIt it=range.first,it1=range.second;
  while(it!=it1)
		{
		DynObject &obj=it->second;
		ChangeHG(obj.Name,1,obj.World._43);
    m_Objects.erase(it++);
		}
  }
void DynObjectPool::CollectObjectsForPoints(const std::vector<point3> &pnts, std::set<std::string> *objects)
  {
STACK_GUARD("DynObjectPool::CollectObjectsForPoints");
  if(!objects) return;
  int num_pnts=pnts.size();

	ObjectsIt it=m_Objects.begin(),ite=m_Objects.end();
  for(;it!=ite;it++)
    {
    DynObject &obj=it->second;
    if(objects->find(obj.Name)!=objects->end()) continue;
    for(int i=0;i<num_pnts;i++)
      if(obj.Bound.DistToPoint(pnts[i])<1.f) {objects->insert(obj.Name);break;}
    }
  
  }


const LevelCase* DynObjectPool::GetCase(const std::string& Name)
{
STACK_GUARD("DynObjectPool::GetCase");
	std::pair<ObjectsIt,ObjectsIt> range=m_Objects.equal_range(Name);
	ObjectsIt it=range.first,it1=range.second;
	while(it!=it1)
		return &(it->second.CaseInfo);
	return NULL;
}

