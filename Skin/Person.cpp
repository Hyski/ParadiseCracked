// Person.cpp: implementation of the Person class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4503)
#include "precomp.h"
#include "../common/graphpipe/graphpipe.h"
#include "skin.h"
#include "skanim.h"
#include "../Gamelevel/Gamelevel.h"
#include "skeleton.h"
#include "AnimaLibrary.h"
#include "../gamelevel/grid.h"

#include "Person.h" 
#include "../options/options.h"
#include "Shadow.h"
#include "../iworld.h"
#include "../interface/banner.h"
#include "../interface/interface.h"
#include "../common/3deffects/effectmanager.h"
#include "../logic2/hexutils.h"
#include "../common/utils/profiler.h"


#include "../game.h"

#include <algorithm>



//#define CHECK_SOUNDS
#ifdef CHECK_SOUNDS
  static char *snd_type2name[]={"RANDOM2D", "RANDOM3D", "LAZY", "WALK", "RUN",
    "HURT", "CHURT", "LUCKYSTRIKE", "DEATH", "ENEMYSIGHTED", "GIRL",
    "SELECTION", "MOVE", "ATTACK", "MISS", "UNDERFIRE",
    "TREATMENT_LOW", "TREATMENT_MED", "TREATMENT_HIGH",
    "TEAM_COME", "TEAM_DISMISS", "TEAM_NEW", "TEAM_LEAVE",
    "ENEMYDIED", "ENEMY_EXPLODED", "OPEN_OK", "OPEN_FAILED",
    "CAR_FAILED", "LEVELUP",
    "GOT_RLAUNCHER", "GOT_AUTOCANNON", "GOT_SHOTGUN", "GOT_ENERGYGUN",
    "USE_PLASMA_GREN", "SAW_BIGBANG", "VEH_AMBIENT", "NOLOF", "SCANNER"};
#endif



static const std::string PERSONSKEL("animations/anims/characters/");
static const std::string PERSONSKIN("animations/skins/characters/");
static const std::string WEAPONSKIN("animations/skins/items/weapons/");
static const std::string VEHICLESKEL("animations/anims/vehicles/");
static const std::string VEHICLESKIN("animations/skins/vehicles/");
static const std::string VEHXLS("scripts/species/vehicle.txt");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
float SpeedMul;
const float DESTFAULT=0.1;
static const float TURNINGSPEED=TORAD(240);//поворот рад/сек на месте
static const float TURNINGFAULT=TORAD(3); //ошибка угла поворот на месте
static const float WTURNINGSPEED=TORAD(90);//поворот рад/сек при ходьбе
static const float WTURNINGFAULT=TORAD(25);//ошибка угла поворот при ходьбе


class spline
  {
  private:
  int power;
  std::vector<point3> v;
  float Sp(float,float,float);
  public:
    void Clear(){v.clear();}
    point3 GetPoint(float);
    spline(float _power=3)
      {
      power=_power=3;
      }
    void AddPoint(const point3 &p){v.push_back(p);if(v.size()==1)v.push_back(p);};
    void End()
      {
      if(v.empty())return;
      v.push_back(v[v.size()-1]);
      };
    void ShowIt();
  };

/**********************************************************************************
классы, служащие для озвучки персонажей
***********************************************************************************/
extern float VoicesVol;
extern float EffectsVol;

std::queue<SoundEventsQueue::_TypeNameRef> SoundEventsQueue::m_TeamPhrases;

class SoundPlayer
  {
  ISndEmitter *m_Emitter;
  public:
    SoundPlayer() : m_Emitter(0) {Registered=false;};
    SoundPlayer(const SoundPlayer &a) : m_Emitter(0) {*this=a;};
    ~SoundPlayer(){Stop();};
    SoundPlayer& operator=(const SoundPlayer &a)
      {
      m_Pos=a.m_Pos;
      m_Vel=a.m_Vel;
      Registered=a.Registered;
      return *this;
      };
  public: //поддержка интерфейсов sound:sdemit
    point3& GetPos(){return m_Pos;};
    point3& GetVel(){return m_Vel;};
  public:
    void Play2d(const std::string &Name,bool cycled=false)
      {       
			Play_(cycled?"charactercycled2d":"character2d",Name,true);  
			};
    void Play3d(const std::string &Name,bool cycled=false)
      {       Play_(cycled?"charactercycled":"character",Name,false); 
      };
    void SetPosition(const point3 &Pos)
      {
      if (m_Emitter)
        {
        m_Emitter->setPosition(Pos);
        }
      m_Pos=Pos;
      };
    void Stop()
      {
      if (m_Emitter)
        {
        m_Emitter->stop();
        m_Emitter->Release();
        m_Emitter = 0;
        Unreg();
        }
      }
    bool IsPlaying() {
			bool p=m_Emitter?m_Emitter->isPlaying():false;
      if (m_Emitter&&!p)
				{
				Stop();
				}
			return p;
			};
  private:
    void Reg(){if(!Registered){Registered=true;}};
    void Unreg(){if(Registered){Registered=false;}};
    void Play_(const std::string &script, const std::string &name,bool /*is2d*/)
      {
      Stop();
      Reg();
      const ISndScript *snd_script = ISound::instance()->getScript(script.c_str());
      m_Emitter = ISound::instance()->createEmitter(snd_script,name.c_str());
      m_Emitter->setPosition(m_Pos);
      m_Emitter->play();
      }
  protected:
    point3 m_Pos,m_Vel;
    bool Registered;
  };  

class SoundPerson::EventHandler // класс для обработки звукового события.
  {
  friend class SoundsParser;
	friend class SoundEventsQueue;
	friend class SoundPerson;
  public:
    void Go();
    void Stop(){m_Player.Stop();};
    void UpdatePos(const point3 &pos){m_Player.SetPosition(pos);};
    void Tick(float Time,bool ClearWathdog=false);     //регулярно вызываемая функция для возможности подумать
    bool IsPlaying() {return m_Player.IsPlaying();};
    const SoundPlayer* GetPlayer(){return &m_Player;};
		bool is2d() const {return m_Is2d;};
  private:
    enum TYPE {T_BYPERCENT, T_BYFREQ};
    TYPE m_PlayingType;
    bool m_Is2d;
    bool m_Cycled;
    float m_Percent;      //с какой вероятностью при событии играется звук. [0..1]
    float m_Frequency;    //раз в m_Frequency секунд играется звук 
		int m_LastPlayed;
    std::vector<std::string> m_SndNames;
    SoundPlayer m_Player;
		int m_Priority;
  private:
    float m_LastBeat;
  public:
    EventHandler():m_Cycled(false),m_LastBeat(0),m_Frequency(1),m_Percent(0),m_Is2d(true),m_PlayingType(T_BYPERCENT),m_LastPlayed(-1){};
  };

void SoundEventsQueue::PersonDied(const std::string& sys_name)
	{
	if(m_TeamPhrases.size())
		{
		std::queue<_TypeNameRef> p=m_TeamPhrases;
		while (m_TeamPhrases.size()) m_TeamPhrases.pop();
		while(p.size())
			{
			 if(p.front().SysName!=sys_name)
				 {
					m_TeamPhrases.push(p.front());
				 }
				p.pop();
			}
		if(m_TeamPhrases.size() && !m_TeamPhrases.front().Ev->IsPlaying()) 
			{
			m_TeamPhrases.front().Ev->Go();
			}
		}


	}

void SoundEventsQueue::SoundStarted(const std::string& sys_name, SoundPerson::EVENTTYPE Type,SoundPerson::EventHandler *Ev)
  {
	STACK_GUARD("SoundEventsQueue::SoundStarted");
	//
	_TypeNameRef dt;
	dt.Ev=Ev;
	dt.Type=Type;
	dt.SysName=sys_name;
	m_TeamPhrases.push(dt);
	if(m_TeamPhrases.size()==1)
		{
		Ev->Go();
		}
  }
void SoundEventsQueue::Clear()
	{
	STACK_GUARD("SoundEventsQueue::Clear");
	while(m_TeamPhrases.size())
	m_TeamPhrases.pop();
	}
void SoundEventsQueue::Tick(float Time)
  {
	STACK_GUARD("SoundEventsQueue::Tick");
	if(m_TeamPhrases.size())
		{
		_TypeNameRef p=m_TeamPhrases.front();
		if(!p.Ev->m_Player.IsPlaying())
			{
			m_TeamPhrases.pop();
			if(m_TeamPhrases.size()) 	m_TeamPhrases.front().Ev->Go();
			}
		}
  }


//статические переменные
bool                       SoundsParser::CacheLoaded=false;
SoundsParser::SysNameMap_t SoundsParser::EventCache;
SoundsParser::StrMap_t     SoundsParser::LevelSteps; //отображение уровень->анимации ходьбы и бега
std::string                SoundsParser::CurLevel="h_house"; 

RealEntity::RealEntity():
    BillBoard(NULL),LastPose(NULL),
      CurAnima(NULL),Normal(NULL),Weapons(NULL),Angle(0),Location(NULLVEC),LastVisUpdate(0),
      SoundScheme(NULL)
  {
  memset(Effects,0,sizeof(Effects));
  memset(EffectsState,0,sizeof(EffectsState));
  MoveSpline=new spline;
  Enabled=true;
  Visible=true;
  SkinBox.Degenerate();
  }
RealEntity::~RealEntity()
  {
STACK_GUARD("RealEntity::~RealEntity");
  delete BillBoard;
  delete LastPose;
  if(Weapons) delete Weapons;
  delete MoveSpline;
  if(SoundScheme) delete SoundScheme;
  EffectManager *em=IWorld::Get()->GetEffects();
  for(int type=0;type<ET_SIZE;type++)
    {
    if(Effects[type]) em->DestroyEffect(Effects[type]);
    }

  }
void RealEntity::HandleSoundEvent(unsigned type)//SoundPerson::EVENTTYPE type;
  {
STACK_GUARD("RealEntity::HandleSoundEvent");
	#ifdef CHECK_SOUNDS
    logFile["person_sounds.log"]("Событие:%x (%p %d) у %s (%p)\n",type,SoundScheme,(int)Enabled,SysName.c_str(),this);
  #endif 

  if(!SoundScheme) return;
  if(!Enabled && type!=SoundPerson::ET_CAR_FAILED) return;

  SoundScheme->Event(type);
  }
void RealEntity::SetSoundScheme(SoundPerson *Scheme)
  {
STACK_GUARD("RealEntity::SetSoundScheme");
  if(SoundScheme) delete SoundScheme;
  SoundScheme=Scheme;
  }

void RealEntity::Enable(bool flag)
  {
STACK_GUARD("RealEntity::Enable");
  Enabled=flag;
  /* by Flif EffectManager *em=IWorld::Get()->GetEffects();*/

	for(int en=0;en<ET_SIZE;en++)
      {
      if(EffectsState[en])
				{
        ShowEffect((EFFECT_TYPE)en, flag?SM_SHOW:SM_HIDE);
				if(!flag)EffectsState[en]=true;
				}
      }
  if(SoundScheme)
    {
    if(!flag) SoundScheme->Stop();
    }
  }

void RealEntity::DrawBanner()
  {
STACK_GUARD("RealEntity::DrawBanner");
  if(BillBoard&&Enabled&&Visible&&CurState!=DEAD)
    {
    point3 pnt(GetLocation()+point3(0,0,1));
    BillBoard->SetPoint(pnt);
    }
  }
void RealEntity::SetBanner(const std::string &banner,unsigned int Color)
  {
STACK_GUARD("RealEntity::SetBanner");
   if(!BillBoard) BillBoard=new Banner;
   BillBoard->SetText(banner.c_str());
	 BillBoard->SetColor(Color);
  }
void RealEntity::Draw()
  {
STACK_GUARD("RealEntity::Draw");
  if(!Visible || !Normal || !Enabled || !Game::RenderEnabled()) return;
  GraphPipe *Pipe=IWorld::Get()->GetPipe();

  StatesManager::SetTransform(D3DTRANSFORMSTATE_WORLD,&World);

  Pipe->Chop(Normal->GetMesh());
  if(Weapons) Pipe->Chop(Weapons->GetMesh());
  //MoveSpline->ShowIt();
  }
void RealEntity::CorrectDirWalking(float deltaTime)
  {
STACK_GUARD("RealEntity::CorrectDirWalking");
  //ходячий товарищ поворачивается в сторону точки,
  //которую ему укажут. Туда и идет
  Quaternion q;
  Transformation World;
  
  float DELTANG=WTURNINGSPEED*deltaTime*SpeedMul;
  float adelta;
  if(DestLinked)
    {
    point3 dist(Dest.x,Dest.y,0);
    dist-=Location;dist.z=0;
    if(dist.Length()<DESTFAULT) 
      {DestLinked=false;return;}
    
    //коррекция направления
    if(Angle>PIm2)Angle-=PIm2;
    if(Angle<0)   Angle+=PIm2;
    float DesiredAngle;
    
    DesiredAngle=atan2(dist.x,-dist.y);
    if(DesiredAngle<0) DesiredAngle=PIm2+DesiredAngle;
    if(!Visible||!Enabled || !Game::RenderEnabled())
      {
      Angle=DesiredAngle;
      SetLocation(Dest,Angle);
      }
    else
      {
      
      float delta=(DesiredAngle-Angle);
      adelta=fabs(delta);
      if(adelta>WTURNINGFAULT)
        {
        float er=1.f,ad=adelta>PI?PIm2-adelta:adelta;
        if(ad>TORAD(29)) er=7*(ad-TORAD(29))+1;
        DELTANG*=er;
        if(FastAbs(DELTANG)>adelta) 
          {
          if(DELTANG>0.f) DELTANG=adelta;
          else            DELTANG=-adelta;
          }
        int sign=delta<0?-1:1;
        //if(adelta>PI){ adelta=PIm2-adelta;Angle-=adelta*0.4*sign;}
        //else Angle+=adelta*0.4*sign;
        if(adelta>PI){ Angle-=DELTANG*sign;}
        else Angle+=DELTANG*sign;
        //FIXME:
        if(GetVehicle())
          {
          SetLocation(Dest,Angle);
          }
        SetLocation(Location,Angle);
        }
      }
    }
  }
void RealEntity::CorrectDirTurning(float deltaTime)
  {
STACK_GUARD("RealEntity::CorrectDirTurning");

  if(DestLinked)
    {
    
		if(!Visible||!Enabled || !Game::RenderEnabled())
      { //если поворота не видно - пропустим
      Angle=DesAngle;
      SetLocation(Location,Angle);
			DestLinked=false;
			return;
      }

    point3 _dir = point3(cos(Angle),sin(Angle),0);
    point3 _des_dir = point3(cos(DesAngle),sin(DesAngle),0);
		float cosa=_dir.Dot(_des_dir);
		cosa=std::min(std::max(-1.f,cosa),1.f);
		float ang = acos(cosa);
		if(ang < TORAD(5))
			{
      SetLocation(Location,DesAngle);
			DestLinked=false;
			return;
			}
		point3 cross=_dir.Cross(_des_dir);
		
    float DELTANG=TURNINGSPEED*deltaTime*SpeedMul;
		float add=std::min(DELTANG,ang);
		if(cross.z<0) add=-add;
		Angle+=add;
		SetLocation(Location,Angle);

    }
  else
    {
    SetAction(STAY,LastUpdate+deltaTime);
    }
  }




static const unsigned STICKCASE=LAZY|STAY|HURT
    |DEAD
    |FALL|USE|KNOCKOUT
    |AIMSHOOT|SHOOT|AIMSHOOTRECOIL
    |SHOOTRECOIL; //случай отсутствия смещения
  static const unsigned TURNINGCASE=WALK|RUN;//случай изменения угла поворота

  static const unsigned GOSTAYCASE=LAZY|STAY|HURT|FALL|  //в этой ситуации возврат к стойке
                      USE|KNOCKOUT|RELOAD/*|AIMSHOOTRECOIL|SHOOTRECOIL*/;
static const unsigned CYCLEDCASE=WALK|RUN|TURN;
void RealEntity::AnimaOver(float Time)
  {
STACK_GUARD("RealEntity::AnimaOver");
  Quaternion q;
  Transformation World;
  static point3 pos;
    if(CurState&GOSTAYCASE)
      {
      //выберем одну из анимаций стойки
      if(CurState==STAY&&rand()<2000)
        SetAction(LAZY,Time);
      else
      SetAction(STAY,Time);
      //если уже стояли, то вероятно захотим сыграть lazy
      }

    if(CurState&CYCLEDCASE) 
      {
      SwitchAnimation(rand(),Time,0.01);
      }
  }
void RealEntity::UpdateVisibility(float Time,GraphPipe *Pipe)
  {
STACK_GUARD("RealEntity::UpdateVisibility");
  static const float VISUPD=1/20.f; //20 раз/сек
  static const float INVISUPD=1/5.f; //20 раз/сек
  float timestep=Visible?VISUPD:INVISUPD;
  if(Time-LastVisUpdate>timestep)
    {
    LastVisUpdate=Time+(float)rand()/32768*INVISUPD-INVISUPD/2;
    Frustum *f=&Pipe->GetCam()->Cone;
    Visible=(Frustum::NOTVISIBLE!=f->TestBBox(SkinBox));
    }
  }
void invmat(D3DMATRIX &a, const D3DMATRIX &b)
  {
 STACK_GUARD("invmat");
 point3 Translate(b._41,b._42,b._43);
  point3 right(b._11,b._12,b._13);
  point3    up(b._21,b._22,b._23);
  point3   dir(b._31,b._32,b._33);
  D3DUtil_SetIdentityMatrix(a);
  a._41=-Translate.Dot(right);
  a._42=-Translate.Dot(up);
  a._43=-Translate.Dot(dir);
  a._11=b._11; a._12=b._21; a._13=b._31;
  a._21=b._12; a._22=b._22; a._23=b._32;
  a._31=b._13; a._32=b._23; a._33=b._33;
  }
void RealEntity::UpdateSkinParms()
  {
STACK_GUARD("RealEntity::UpdateSkinParms");
  //Обновим матрицы
  invmat(InvWorld,World);
  //обновим BBox
  LocalSkinBox=Normal->GetBBox();
  point3 points[8]={
                    point3(LocalSkinBox.minx,LocalSkinBox.miny,LocalSkinBox.minz),
                    point3(LocalSkinBox.minx,LocalSkinBox.miny,LocalSkinBox.maxz),
                    point3(LocalSkinBox.minx,LocalSkinBox.maxy,LocalSkinBox.minz),
                    point3(LocalSkinBox.minx,LocalSkinBox.maxy,LocalSkinBox.maxz),
                    point3(LocalSkinBox.maxx,LocalSkinBox.miny,LocalSkinBox.minz),
                    point3(LocalSkinBox.maxx,LocalSkinBox.miny,LocalSkinBox.maxz),
                    point3(LocalSkinBox.maxx,LocalSkinBox.maxy,LocalSkinBox.minz),
                    point3(LocalSkinBox.maxx,LocalSkinBox.maxy,LocalSkinBox.maxz)
    };
  point3 tpoints[8];
  PointMatrixMultiply(TODXVECTOR(tpoints[0]),TODXVECTOR(points[0]),World);
  PointMatrixMultiply(TODXVECTOR(tpoints[1]),TODXVECTOR(points[1]),World);
  PointMatrixMultiply(TODXVECTOR(tpoints[2]),TODXVECTOR(points[2]),World);
  PointMatrixMultiply(TODXVECTOR(tpoints[3]),TODXVECTOR(points[3]),World);
  PointMatrixMultiply(TODXVECTOR(tpoints[4]),TODXVECTOR(points[4]),World);
  PointMatrixMultiply(TODXVECTOR(tpoints[5]),TODXVECTOR(points[5]),World);
  PointMatrixMultiply(TODXVECTOR(tpoints[6]),TODXVECTOR(points[6]),World);
  PointMatrixMultiply(TODXVECTOR(tpoints[7]),TODXVECTOR(points[7]),World);

  SkinBox.Degenerate();
  SkinBox.Enlarge(tpoints[0]);  SkinBox.Enlarge(tpoints[1]);
  SkinBox.Enlarge(tpoints[2]);  SkinBox.Enlarge(tpoints[3]);
  SkinBox.Enlarge(tpoints[4]);  SkinBox.Enlarge(tpoints[5]);
  SkinBox.Enlarge(tpoints[6]);  SkinBox.Enlarge(tpoints[7]);
  }
bool RealEntity::NotifyHex(IMovingCallBack::EVENT Ev)
  {
STACK_GUARD("RealEntity::NotifyHex");
  if(Ev==IMovingCallBack::EV_END)
    {
          MoveCB->VisitHex(IMovingCallBack::EV_END);
          MovePoints.clear();
          SetAction(STAY,Timer::GetSeconds());
          return true;
    }
  else
    {
    IMovingCallBack::COMMAND com;
    com=MoveCB->VisitHex(Ev);
    if(CurState==DEAD)
      {
      MovePoints.clear();
      return true;
      }
    switch(com)
      {
      case IMovingCallBack::CMD_STOP:
        SetLocation(HexGrid::GetInst()->Get(MovePoints[LastPoint+1]),Angle);
        MovePoints.clear();
        SetAction(STAY,Timer::GetSeconds());
        MoveCB->VisitHex(IMovingCallBack::EV_END);
        return true;
        break;
      case IMovingCallBack::CMD_CONTINUE:
        //fixme:что-то все непонятно очень
        if(!Enabled/*||!Visible*/|| !Game::RenderEnabled()){
          for(int i=LastPoint+1;i<MovePoints.size();i++)
            {
            if(i) Angle=atan2(HexGrid::GetInst()->Get(MovePoints[i]).x-HexGrid::GetInst()->Get(MovePoints[i-1]).x,-HexGrid::GetInst()->Get(MovePoints[i]).y+HexGrid::GetInst()->Get(MovePoints[i-1]).y);
            SetLocation(HexGrid::GetInst()->Get(MovePoints[i]),Angle);
            if(MoveCB->VisitHex(IMovingCallBack::EV_STEP)==IMovingCallBack::CMD_STOP)
              break;
            if(CurState==DEAD)
              {
              MovePoints.clear();
              return true;
              }
            }
          MovePoints.clear();
          if(CurState==DEAD)
            return true;
          
          SetAction(STAY,Timer::GetSeconds());
          MoveCB->VisitHex(IMovingCallBack::EV_END);
          return true;
          }
        break;
      case IMovingCallBack::CMD_SPEED_UP:
        {
        for(int i=LastPoint+1;i<MovePoints.size();i++)
          {
          if(i) Angle=atan2(HexGrid::GetInst()->Get(MovePoints[i]).x-HexGrid::GetInst()->Get(MovePoints[i-1]).x,-HexGrid::GetInst()->Get(MovePoints[i]).y+HexGrid::GetInst()->Get(MovePoints[i-1]).y);
          SetLocation(HexGrid::GetInst()->Get(MovePoints[i]),Angle);
          if(MoveCB->VisitHex(IMovingCallBack::EV_STEP)==IMovingCallBack::CMD_STOP)
            break;
          if(CurState==DEAD)
            {
            MovePoints.clear();
            return true;
            }
          
          }
        MovePoints.clear();
        if(CurState==DEAD)
          return true;
        SetAction(STAY,Timer::GetSeconds());
        MoveCB->VisitHex(IMovingCallBack::EV_END);
        return true;
        }
        break;
      }
    }
  return false;
  }
void RealEntity::PlaceOnSpline(float val)
  {
STACK_GUARD("RealEntity::PlaceOnSpline");
  point3 p=MoveSpline->GetPoint(val);
  point3 nnewp;
  float step;
  if(GetHuman())
    step=0.02;
  else if(!MovePoints.size())
        step=0.2;
  else   
    step=3.f/MovePoints.size();
  float v1,v2;
  v1=std::max(0.f,val-step);
  v2=std::min(1.f,val+step);
  point3 pp=MoveSpline->GetPoint(v1);
  nnewp=MoveSpline->GetPoint(v2);
  Angle=atan2(nnewp.x-pp.x,-(nnewp.y-pp.y));
  
  SetLocation(p,Angle);
  }

void RealEntity::MoveBySpline(float Time)
  {
STACK_GUARD("RealEntity::MoveBySpline");
  //1. подсчитать пройденный путь.
	point3 p1,p2;
	p1=CurAnima->GetOffset(LastPose,LastPose->StartTime);
	p2=CurAnima->GetOffset(LastPose,LastPose->EndTime);
  float Speed = (p2-p1).Length()/(LastPose->EndTime-LastPose->StartTime);
  extern float SkSpeed;

  float dist=Speed*(Time-LastUpdate);//hypot(p1.x-p2.x,p1.y-p2.y);

  float spline_dist=dist/MovePoints.size();
  if(spline_dist+SplinePos>1) spline_dist=1-SplinePos;
  //2. собрать хексы, по которым прошлись.
  std::vector<ipnt2_t> passed_hexes;
  std::vector<float> passed_hexes_pos;
  float t,step=0.1/MovePoints.size();
  ipnt2_t pnt;
  for(t=0;t<=spline_dist;t+=step)
    {
    pnt=HexUtils::scr2hex(MoveSpline->GetPoint(SplinePos+t));
    bool newpnt=passed_hexes.size()&&passed_hexes.back()!=pnt;
    bool lastpnt=MovePoints.back()==pnt;
    if(!passed_hexes.size()||
      (newpnt&&!lastpnt))
      {
      passed_hexes.push_back(pnt);
      passed_hexes_pos.push_back(t);
      }     
    }
  //3. послать оповещение.
  ipnt2_t starth=MovePoints[LastPoint+1];
  int i;
  for(i=0;i<passed_hexes.size();i++)  if(passed_hexes[i]==starth)break;
  float ultimate_offset=spline_dist;
  for(;i<passed_hexes.size();i++)
    {
    PlaceOnSpline(SplinePos+passed_hexes_pos[i]);
    if(NotifyHex(IMovingCallBack::EV_STEP))
      {//приказ - остановиться
      //ultimate_offset=passed_hexes_pos[i];
      return;
      }
    LastPoint++;
    if(LastPoint==MovePoints.size()-1)
      {
			SetLocation(HexGrid::GetInst()->Get(MovePoints.back()),Angle);
      NotifyHex(IMovingCallBack::EV_END);
      return;
      }
    }
  //4. установить человека.
  PlaceOnSpline(SplinePos+ultimate_offset);
  SplinePos+=ultimate_offset;
  }
void RealEntity::UpdateOnTime(float Time)
  { 
STACK_GUARD("RealEntity::UpdateOnTime");
	CodeProfiler CF("person.update");

  GraphPipe *Pipe=IWorld::Get()->GetPipe();  
  if(!CurAnima) return;


  UpdateVisibility(Time,Pipe);
  float DesUpdFPS=(Visible&&Enabled)?(1.f/(AnimaQual*31)):(1.f/3);
  if(CurState==DEAD&&LastPose&&Time>LastPose->EndTime) DesUpdFPS=0.3f;
  if(Time-LastUpdate<=DesUpdFPS) return;//ограничиваем обновление в зависимости от видимости

  if(LastPose&&Time>LastPose->EndTime) //кончилась анимация
    {
    AnimaOver(Time);
    }

  if(CurState&TURNINGCASE)//парень поворачивается
    {
  if(MovePoints.size())    MoveBySpline(Time);
  else
		CorrectDirWalking(Time-LastUpdate);
    }

  if(CurState&TURN)//парень поворачивается
    CorrectDirTurning(Time-LastUpdate);

	Location.z=World._43=IWorld::Get()->GetLevel()->LevelGrid->Height(Location);
	SkState m_LastState=CurAnima->Get(LastPose,Time);
  if(m_LastState.m_BoneNum==0)
    {
#if 0
    struct {char *Name;int State;}
    anims[]={"LAZY",0x01, "STAY",0x02, "HURT",0x04,"WALK",0x08, "TURN",0x10,
               "SHOOT",0x20,"SHOOTRECOIL",0x40, "DEAD",0x80, "RUN",0x100,
              "FALL",0x200, "USE",0x400, "KNOCKOUT",0x800, "RELOAD",0x1000,
              "AIMSHOOT",0x2000, "AIMSHOOTRECOIL",0x4000, "PANIC",0x8000,
              "SPECANIM",0x10000,"",0};
    char *anim="fatal error";
    for(int i=0;anims[i].Name[0];i++)
      {
      if(CurState==anims[i].State)
        anim=anims[i].Name;
      }
//    logFile["failed_skels.log"]("нет анимации. %s, animation=%s\n", Name.c_str(),anim);
#endif
      throw CASUS(Name+" - проблемы с анимацией!");
    }

  if(Normal)    Normal->Update(&m_LastState);
  if(Weapons)   Weapons->Update(&m_LastState);

  UpdateSkinParms();
  LastUpdate=Time+(1.f/(2*(AnimaQual*31)))*((float)rand()/32768-0.5);
// ~~~~~~~~~~~~~ ShadowCaster
  if (shadow) shadow->Update (Time);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
  EffectManager *em=IWorld::Get()->GetEffects();
    for(int en=0;en<ET_SIZE;en++)
    {
    point3 pos=GetLocation();
    if(Effects[en])
      em->SetPosition(Effects[en],en==ET_SELECTION?pos:(pos+AXISZ));
    }
  //поддержка звуковой схемы
    if(SoundScheme)
      {
      SoundScheme->UpdatePos(GetLocation());
	  if(CurState==DEAD)	SoundScheme->Tick(0,Enabled);
	  else					SoundScheme->Tick(Time,Enabled);
      }



  }
float RealEntity::SetAngle(float CurTime, float angle)
{
STACK_GUARD("RealEntity::SetAngle");
	if(!Enabled||!Visible|| !Game::RenderEnabled())
		{
		Angle=angle;
    SetLocation(Location,Angle);
		return CurTime;
	}
	else
		{
    point3 _dir = point3(cos(Angle),sin(Angle),0);
    point3 _des_dir = point3(cos(angle),sin(angle),0);
		float cosa=_dir.Dot(_des_dir);
		cosa=std::min(std::max(-1.f,cosa),1.f);
		float ang = acos(cosa);
		if(ang < TORAD(5))
			{
			Angle = angle;
      SetLocation(Location,Angle);
			DestLinked=false;
			return CurTime;
			}

		DesAngle = angle;
		DestLinked=true;
		SetAction(TURN, CurTime);

		return CurTime+ang/TURNINGSPEED/SpeedMul;
	}
}
void RealEntity::UpdateWorldMat(float Angle, const point3 &pos)
  {
STACK_GUARD("RealEntity::UpdateWorldMat");
  D3DUtil_SetRotateZMatrix( World, Angle);
  World._41 = pos.x;
  World._42 = pos.y;
  World._43 = pos.z;
  }
void RealEntity::SetLocation(const point3 &_Pos, float angle)
  {
STACK_GUARD("RealEntity::SetLocation");

  Angle=fmod(angle,TORAD(360));
  if(!CurAnima) return;
  UpdateWorldMat(Angle,_Pos);

	if(Location!=_Pos)
		{
		Location=_Pos;
		Location.z=World._43=IWorld::Get()->GetLevel()->LevelGrid->Height(Location);
		}
  }
void RealEntity::SetDestination(const point3 &d)
  {
STACK_GUARD("RealEntity::SetDestination");
  DestLinked=true;
  Dest=d;
  }

bool RealEntity::TraceRay(const ray &r, float *Dist, point3 *Norm, bool AsBox)
  {
STACK_GUARD("RealEntity::TraceRay");
  if(!Normal) return false;
  point3 p,norm;
  if(SkinBox.IntersectRay(r,&p))
    {
    /*
    старый вариант: считалось попадание только в бокс в мировых координатах
    if(AsBox)
      {
      *Dist=(p-r.Origin).Length();
      *Norm=AXISZ;
      return true;
      }*/
    ray localray;
    VectorMatrixMultiply(TODXVECTOR(localray.Direction),TODXVECTOR(r.Direction),InvWorld);
    PointMatrixMultiply(TODXVECTOR(localray.Origin),TODXVECTOR(r.Origin),InvWorld);
    localray.Update();
    if(LocalSkinBox.IntersectRay(localray,&p))
      {
      if(AsBox)
        {
        *Dist=(p-localray.Origin).Length();
        *Norm=AXISZ;
        return true;
        }
      
      if(Normal->GetMesh()->TraceRay(localray,Dist,&norm))
        {
        VectorMatrixMultiply(*(D3DVECTOR*)Norm,*(D3DVECTOR*)&norm,World);
        return true;
        }
      }
    }
  return false;
  }
bool RealEntity::TraceSegment(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm)
	{
STACK_GUARD("RealEntity::TraceSegment");
  if(!Normal) return false;
  point3 p,norm;
	float dist;
	point3 start_pnt=From,end_pnt=From+Dir,mid_pnt=From+0.5*Dir;
		float Len=Dir.Length()/2; //половина длины сегмента
		if(SkinBox.DistToPoint(mid_pnt)>Len)return false;
    ray localray;
    VectorMatrixMultiply(TODXVECTOR(localray.Direction),
      TODXVECTOR(Dir),InvWorld);
    PointMatrixMultiply(TODXVECTOR(localray.Origin),
      TODXVECTOR(From),InvWorld);
    localray.Update();
    if(Normal->GetMesh()->TraceRay(localray,&dist,&norm))
      {
      VectorMatrixMultiply(*(D3DVECTOR*)Norm,*(D3DVECTOR*)&norm,World);
			*Res=From+dist*Dir;
      return true;
			}
		return false;
	}

float RealEntity::SetAction(unsigned /*ANIMATION*/ Action,float CurTime)
  {
STACK_GUARD("RealEntity::SetAction");
  const unsigned REPENABLE=(TURN|SPECANIM|PANIC|STAY|SHOOTRECOIL|AIMSHOOTRECOIL);
  const unsigned SKIPABLE=(LAZY|HURT|SHOOT|SHOOTRECOIL|FALL|DEAD|USE|RELOAD|AIMSHOOT|AIMSHOOTRECOIL|SPECANIM);
  const unsigned SOUNDHANDLED=(TURN|WALK|RUN|LAZY|DEAD|HURT);


	bool Changed=  (CurState!=Action);

  if(Action==HURT && CurState==FALL)  return CurTime;

  float ret,delay;
  //FIXME:
  if(Action==DEAD) 
  for(int en=0;en<ET_SIZE;en++)
      if(Effects[en])
        ShowEffect((EFFECT_TYPE)en, SM_HIDE);


  if(!Changed &&!(CurState&REPENABLE)) return CurTime;
  if(CurState==TURN&&DestLinked && !Changed)
    {
		SetLocation(Location,DesAngle);
		DestLinked=false;
    }

  switch(Action)
    {
    case SHOOTRECOIL: delay=0.01;break;
    case AIMSHOOTRECOIL: delay=0.01;break;
    case LAZY: case STAY: delay=0.9;break;
    case WALK: case RUN: delay=0.5;break;
    case TURN:           delay=0.5;break;
    default: delay=1.5;
    }
	bool CanSkip=false;
	CanSkip=(!Enabled||!Visible|| !Game::RenderEnabled())&&(Action&SKIPABLE);
	CanSkip=CanSkip||( (Action&(SHOOT|AIMSHOOT))&&(CurState&(SHOOTRECOIL|AIMSHOOTRECOIL)) );

  CurState=Action;

	if(CanSkip)  {SwitchAnimation(rand(),0,0);ret=CurTime;}
	else				 {ret = SwitchAnimation(rand(),CurTime,delay);}
  
  if(SoundScheme && Changed)
    {
    if(CurState&SOUNDHANDLED)
      {
      if(CurState&WALK)HandleSoundEvent(SoundPerson::ET_WALK);
      else if(CurState&TURN)HandleSoundEvent(SoundPerson::ET_WALK);
      else if(CurState&RUN)HandleSoundEvent(SoundPerson::ET_RUN);
      else if(CurState&LAZY)HandleSoundEvent(SoundPerson::ET_LAZY);
      else if(CurState&DEAD)
        {  //если умер - остановить все звуки и проиграть звук смерти
        HandleSoundEvent(SoundPerson::ET_WALK|SoundPerson::ET_STOPIT);
        HandleSoundEvent(SoundPerson::ET_RUN|SoundPerson::ET_STOPIT);
        HandleSoundEvent(SoundPerson::ET_DEATH);
        }
      else if(CurState&HURT)
        {  //если умер - остановить все звуки и проиграть звук смерти
        HandleSoundEvent(SoundPerson::ET_WALK|SoundPerson::ET_STOPIT);
        HandleSoundEvent(SoundPerson::ET_RUN|SoundPerson::ET_STOPIT);
        HandleSoundEvent(SoundPerson::ET_HURT);
        }
      if(!(CurState&LAZY)) HandleSoundEvent(SoundPerson::ET_VEH_AMBIENT|SoundPerson::ET_STOPIT);
      }
    else 
      {
      HandleSoundEvent(SoundPerson::ET_WALK|SoundPerson::ET_STOPIT);
      HandleSoundEvent(SoundPerson::ET_RUN|SoundPerson::ET_STOPIT);
      HandleSoundEvent(SoundPerson::ET_VEH_AMBIENT);
      //HandleSoundEvent(SoundPerson::ET_LAZY|SoundPerson::ET_STOPIT);
      }
    }
  
  if(CurState & (AIMSHOOT|SHOOT))//чтобы успел сыграть анимацию прицеливания
		ret+=1.f/5;		 
  return ret;
  }
void RealEntity::StartMoving(ANIMATION Type, const std::vector<ipnt2_t> &hexes, IMovingCallBack *_MoveCB)
  {
STACK_GUARD("RealEntity::StartMoving");
  MoveCB=_MoveCB;
  /* by Flif int t=hexes.size();*/
  MoveSpline->Clear();
  MovePoints=hexes;
  std::reverse(MovePoints.begin(),MovePoints.end());
  LastPoint=-1;
  MoveSpline->AddPoint(GetLocation());    
  ipnt2_t newhex=HexUtils::scr2hex(GetLocation());
  SplinePos=0;
  for(int i=hexes.size()-1;i>=0;i--)
    {
    MoveSpline->AddPoint(HexGrid::GetInst()->Get(hexes[i])+AXISZ);    
    }
  // MoveSpline->AddPoint(HexGrid::GetInst()->Get(hexes[0])+AXISZ);    
  MoveSpline->End();
  if(hexes.size())    SetAction(Type,Timer::GetSeconds());
  else _MoveCB->VisitHex(IMovingCallBack::EV_END);
  }
void RealEntity::ShowEffect(EFFECT_TYPE type, SHOW_MODE mode)
  {
STACK_GUARD("RealEntity::ShowEffect");
  EffectManager *em=IWorld::Get()->GetEffects();
  if(mode==SM_SHOW)
    {
		std::string eff_name;
		point3 pos;
		pos=GetLocation()+point3(0,0,1);
		float rad=0;
		switch(type)
			{
			case ET_FLAME:
				eff_name="hit_small_flame";
				break;
			case ET_SHOCK:
				eff_name="hit_small_flame";
				break;
			case ET_ELECTRIC:
				eff_name="hit_small_flame";
				break;
			case ET_SELECTION:
				pos = GetLocation();
				eff_name="small_selection";
				{
					float dx=(SkinBox.maxx-SkinBox.minx);
					float dy=(SkinBox.maxy-SkinBox.miny);
					rad=0.4*hypot(dx,dy);
					if(rad<0.5 || dx<0 || dy<0) rad=0.5;
				}
				break;
			default:return;
			}
		if(Enabled)
			{
			if(Effects[type]) em->DestroyEffect(Effects[type]);
			Effects[type]=em->CreateMovableEffect(eff_name,pos,rad);
			}
		EffectsState[type]=true;
    }
  else
    {
    if(type<0||type>=ET_SIZE) return;
    EffectsState[type]=false;
    if(!Effects[type]) return;
    switch(mode)
      {
      case SM_HIDE: 
        em->DestroyEffect(Effects[type]);
        Effects[type]=0;
        break;
      case SM_SLOW_HIDE: 
        em->FinishEffect(Effects[type]);
        Effects[type]=0;
        break;
      }
    }
  }
void RealEntity::DrawFOS()
  {
STACK_GUARD("RealEntity::DrawFOS");
  if(!Visible||!Enabled||CurState==DEAD|| !Game::RenderEnabled()) return;
  GraphPipe *Pipe=IWorld::Get()->GetPipe();
  float ang1,ang2;
  float r1,r2;
  r1=FOSrad1;r2=FOSrad2;
  if(!r1&&!r2) return;
  ang1=Angle-FOSangle/2;
  ang2=Angle+FOSangle/2;
  static const int PNTNUM=17;
  point3 pos1[PNTNUM*2+2];
  texcoord tuv[PNTNUM*2+2];
  unsigned col[PNTNUM*2+2];
  point3 pnt=GetLocation()+AXISZ;
  int i=0;
  for(;i<PNTNUM;i++)
    {
    float coeff=(float)i/(PNTNUM-1);
    float phase=ang1+coeff*FOSangle;
    /* by Flif float phase2=ang2+coeff*(TORAD(360)-FOSangle);*/
    pos1[i]=pnt+r2*(sin(phase)*AXISX-cos(phase)*AXISY);
    }
  pos1[i++]=pnt+r1*(sin(ang2)*AXISX-cos(ang2)*AXISY);
  for(;i<PNTNUM*2+1;i++)
    {
    float coeff=(float)(i-PNTNUM-1)/(PNTNUM-1);
    /* by Flif float phase=ang1+coeff*FOSangle;*/
    float phase2=ang2+coeff*(TORAD(360)-FOSangle);
    pos1[i]=pnt+r1*(sin(phase2)*AXISX-cos(phase2)*AXISY);
    }
  pos1[i++]=pnt+r2*(sin(ang1)*AXISX-cos(ang1)*AXISY);

  Primi prim;
  prim.Diffuse=col;
  prim.Pos=pos1;
  prim.UVs[0]=tuv;
  prim.Prim=Primi::LINESSTRIP;
  prim.Contents=Primi::NEEDTRANSFORM;
  prim.VertNum=i;
  prim.IdxNum=0;
  Pipe->Chop("purewhite",&prim);
  }
void RealEntity::SetFOS(float front_radius, float back_radius, float sight_angle)
  {
STACK_GUARD("RealEntity::SetFOS");
  FOSrad1=back_radius;
  FOSrad2=front_radius;
  FOSangle=sight_angle;
  }
float RealEntity::GetAngle()
  {
STACK_GUARD("RealEntity::GetAngle");
  return Angle;
  };

point3 RealEntity::GetShotPoint(point3 suppose_point)
{
	STACK_GUARD("RealEntity::GetShotPoint");
	if(suppose_point!=NULLVEC)
	{
		point3 pnt = m_LastState.GetNearestBone(suppose_point),pnt1;
		PointMatrixMultiply(TODXVECTOR(pnt1),TODXVECTOR(pnt),World);
		
		return pnt1;	
	}
	else
	{
		point3 pnt = m_LastState.GetNearestBone(SkinBox.GetCenter()),pnt1;
		PointMatrixMultiply(TODXVECTOR(pnt1),TODXVECTOR(pnt),World);
		
		return pnt1;	
	}
/*
  point3 p;
  if(SkinBox.minx>SkinBox.maxx)
    {
    p=GetLocation()+1.4*AXISZ;
    }
  else
    {
  float dx,dy,dz;
  dx=(SkinBox.maxx-SkinBox.minx);
  dy=(SkinBox.maxy-SkinBox.miny);
  dz=(SkinBox.maxz-SkinBox.minz);

  point3 dev=point3((frand()-0.5)*dx,(frand()-0.5)*dy,(frand()-0.5)*dz);
  point3 base=SkinBox.GetCenter();
  if(suppose_point!=NULLVEC)
    {
    if(suppose_point.z>base.z+0.2*dz)  base.z+=0.2*dz; else
    if(suppose_point.z<base.z-0.2*dz)  base.z-=0.2*dz;
    if(suppose_point.y>base.y+0.2*dy)  base.y+=0.2*dy; else
    if(suppose_point.y<base.y-0.2*dy)  base.y-=0.2*dy;
    if(suppose_point.x>base.x+0.2*dx)  base.x+=0.2*dx; else
    if(suppose_point.x<base.x-0.2*dx)  base.x-=0.2*dx;
    }
  p=base+0.5*dev;
    }
  return p;	 */
  }
void RealEntity::GetLOSPoints(points_t *points) const
	{
STACK_GUARD("RealEntity::GetLOSPoints");
	points->push_back(GetLocation()+1.4*AXISZ);
	int start=points->size();
 	
  if(LocalSkinBox.minx<LocalSkinBox.maxx)
    {
  	points->push_back(point3(LocalSkinBox.minx,LocalSkinBox.miny,LocalSkinBox.minz));
  	points->push_back(point3(LocalSkinBox.minx,LocalSkinBox.miny,LocalSkinBox.maxz));
  	points->push_back(point3(LocalSkinBox.minx,LocalSkinBox.maxy,LocalSkinBox.minz));
  	points->push_back(point3(LocalSkinBox.minx,LocalSkinBox.maxy,LocalSkinBox.maxz));
  	points->push_back(point3(LocalSkinBox.maxx,LocalSkinBox.miny,LocalSkinBox.minz));
  	points->push_back(point3(LocalSkinBox.maxx,LocalSkinBox.miny,LocalSkinBox.maxz));
  	points->push_back(point3(LocalSkinBox.maxx,LocalSkinBox.maxy,LocalSkinBox.minz));
		points->push_back(point3(LocalSkinBox.maxx,LocalSkinBox.maxy,LocalSkinBox.maxz));
		
		if(LocalSkinBox.maxx-LocalSkinBox.minx>2)
			{
			points->push_back(point3(0.5*(LocalSkinBox.minx+LocalSkinBox.maxx),LocalSkinBox.miny,LocalSkinBox.minz));
			points->push_back(point3(0.5*(LocalSkinBox.minx+LocalSkinBox.maxx),LocalSkinBox.miny,LocalSkinBox.maxz));
			points->push_back(point3(0.5*(LocalSkinBox.minx+LocalSkinBox.maxx),LocalSkinBox.maxy,LocalSkinBox.minz));
			points->push_back(point3(0.5*(LocalSkinBox.minx+LocalSkinBox.maxx),LocalSkinBox.maxy,LocalSkinBox.maxz));
			}
		
		if(LocalSkinBox.maxy-LocalSkinBox.miny>2)
			{
			points->push_back(point3(LocalSkinBox.minx,0.5*(LocalSkinBox.miny+LocalSkinBox.maxy),LocalSkinBox.minz));
			points->push_back(point3(LocalSkinBox.minx,0.5*(LocalSkinBox.miny+LocalSkinBox.maxy),LocalSkinBox.maxz));
			points->push_back(point3(LocalSkinBox.maxx,0.5*(LocalSkinBox.miny+LocalSkinBox.maxy),LocalSkinBox.minz));
			points->push_back(point3(LocalSkinBox.maxx,0.5*(LocalSkinBox.miny+LocalSkinBox.maxy),LocalSkinBox.maxz));
			}
		if(LocalSkinBox.maxz-LocalSkinBox.minz>2)
			{
			points->push_back(point3(LocalSkinBox.minx,LocalSkinBox.miny,0.5*(LocalSkinBox.minz+LocalSkinBox.maxz)));
			points->push_back(point3(LocalSkinBox.minx,LocalSkinBox.maxy,0.5*(LocalSkinBox.minz+LocalSkinBox.maxz)));
			points->push_back(point3(LocalSkinBox.maxx,LocalSkinBox.miny,0.5*(LocalSkinBox.minz+LocalSkinBox.maxz)));
			points->push_back(point3(LocalSkinBox.maxx,LocalSkinBox.maxy,0.5*(LocalSkinBox.minz+LocalSkinBox.maxz)));
			}
		}
	
		for(int i=start;i<points->size();i++)
		{
	  PointMatrixMultiply(TODXVECTOR((*points)[i]),TODXVECTOR((*points)[i]),World);
		}
	}

void RealEntity::SetOwnership(OWNERSHIP ownby)
	{
STACK_GUARD("RealEntity::SetOwnership");
	if(SoundScheme) SoundScheme->SetOwnership(ownby);
	}


void Person::MakeSaveLoad(SavSlot &sl)
  {
STACK_GUARD("Person::MakeSaveLoad");
  if(sl.IsSaving())
    { 
    int i;
    sl<<Name;
    sl<<SysName;
    for(i=0;i<SkelData::SUITSNUM+1;i++) sl<<Skels.SkinNames[i];
    sl<<GetLocation()<<Angle;
    sl<<CurState<<CurWeapon<<CurWeapName<<Stand;
    sl<<Visible<<Enabled;
    sl<<FOSrad1<<FOSrad2<<FOSangle; 
    for(i=0;i<ET_SIZE;i++) sl<<EffectsState[ET_SIZE];
    }
  else
    {
    std::string sk[SkelData::SUITSNUM+1];
    point3 Loc;float Ang;
    unsigned CS,CW,ST;
    std::string weap;
    int i;
    sl>>Name;
    sl>>SysName;
    for(i=0;i<SkelData::SUITSNUM+1;i++) sl>>sk[i];
    sl>>Loc>>Ang;
    sl>>CS>>CW>>weap>>ST;
    Load(Name,sk,SysName);

			SoundPerson *p=SoundScheme;
			SoundScheme=NULL;
    SetAction(CS,0);
			SoundScheme=p;

    SetStanding((STAND)ST,0);
    SetWeapon((WEAPONS)CW,weap,0);
    SetLocation(Loc,Ang);
    sl>>Visible>>Enabled;
    sl>>FOSrad1>>FOSrad2>>FOSangle; 
    for(i=0;i<ET_SIZE;i++) sl>>EffectsState[ET_SIZE];
    Enable(Enabled);
		LastUpdate=-10;
    UpdateOnTime(0);
    }
  }
void Vehicle::MakeSaveLoad(SavSlot &sl)
{
STACK_GUARD("Vehicle::MakeSaveLoad");
  if(sl.IsSaving())
    { 
    int i;
    sl<<Name;
    sl<<GetLocation()<<Angle;
    sl<<CurState;
    sl<<Sounds.Shot<<Sounds.Death<<Sounds.Engine<<Sounds.Walk<<Sounds.Hurt;
    sl<<Visible<<Enabled;
    sl<<FOSrad1<<FOSrad2<<FOSangle; 
    for(i=0;i<ET_SIZE;i++) sl<<EffectsState[ET_SIZE];
    }
  else
    {
    point3 Loc;float Ang;
    unsigned CS;
    std::string weap;
    int i;
    sl>>Name;
    sl>>Loc>>Ang;
    sl>>CS;
    sl>>Sounds.Shot>>Sounds.Death>>Sounds.Engine>>Sounds.Walk>>Sounds.Hurt;
    Load(Name,Sounds);

			SoundPerson *p=SoundScheme;
			SoundScheme=NULL;

    SetAction(CS,0);
			SoundScheme=p;

    SetLocation(Loc,Ang);
    sl>>Visible>>Enabled;
    sl>>FOSrad1>>FOSrad2>>FOSangle; 
    for(i=0;i<ET_SIZE;i++) sl>>EffectsState[ET_SIZE];
    Enable(Enabled);
    UpdateOnTime(0);
    }

}


Person::Person()
{
STACK_GUARD("Person::Person");
	int _size=SkelData::WEAPONSNUM*SkelData::MAXANIMANUM*SkelData::ANIMANAMES;
/* by Flif AnimaLibrary *People=AnimaLibrary::GetInst();*/
  memset(Skels.Animations,0,4*_size*sizeof(SkAnim*));
  memset(Suits,0,SkelData::SUITSNUM*sizeof(SkSkin*));

  Stand=GetBit(PEACESTAY);
  CurWeapon=GetBit(PISTOL);
  CurState=LAZY;
  Dest=point3(0,0,0);

  LastUpdate=0;
  DestLinked=false;
  AnimaQual=Options::GetFloat("game.animaquality");
  SpeedMul=Options::GetFloat("game.anispeed");
	extern float SkSpeed;
	SkSpeed=SpeedMul;

// ~~~~~~~~~~~~~~~~~~~~~ ShadowCaster
  shadow = Shadows::CreateShadow (this);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  }

Person::~Person()
  {
  for(int i=0;i<SkelData::SUITSNUM+1;i++)
    DESTROY(Suits[i]);
  
  // ~~~~~~~~~~~~~~~~~~~~~ ShadowCaster
  if (shadow) delete shadow;
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
  }



//разбор строки, включающей в себя несколько названий, разделенных ','
void Person::ParseAnimas(SkAnim *Animas[],int *Count,std::string &data)
  {
STACK_GUARD("Person::ParseAnimas");

  static const std::string MASK=", .;\t";
  static const std::string EXT=".skel";
    int AnimaNames=0;
   std::string::size_type t,start=0;
   static std::string Name;
	 while(AnimaNames<SkelData::MAXANIMANUM-1) 
    {
    t=data.find_first_of(MASK,start);
    if(t==data.npos)   t=data.size();
    Name=std::string(data,start,t-start);
    start=t;
    start=data.find_first_not_of(MASK,start);
    if(!Name.empty() && Name[0] != '-')
      {
      Animas[AnimaNames]=AnimaLibrary::GetInst()->GetSkAnimation(PERSONSKEL+Name+EXT);

      if(Animas[AnimaNames])
        {
        AnimaNames++;
        }
      else
        {
//        logFile["failed_skels.log"]("ошибка загрузки %s в %s\n",Name.c_str(),this->Name.c_str());
        }
      }
    if(start==data.npos) break;
    }
  *Count=AnimaNames;

  }

//загрузка из .xls
Person::SkelCache_t Person::SkelsCache;
Vehicle::SkelCache_t Vehicle::SkelsCache;

const Person::SkelData& Person::FillSkelsCache(std::string XlsName)
{
STACK_GUARD("Person::FillSkelsCache");
	std::string inp;
	SkelCache_t::iterator it;
	it=SkelsCache.find(XlsName);
	if(it!=SkelsCache.end()) return it->second;

  TxtFile *xls=AnimaLibrary::GetInst()->GetTable(XlsName);
  DataMgr::Release(XlsName.c_str());
  if(!xls->SizeY()) throw CASUS(std::string("Отсутствует таблица описания человека:")+XlsName);
	SkelData data;
  //xls->GetCell(1,0,data.Name);
	data.Name=XlsName;
  for(int stand=0;stand<4;stand++)
    for(int weap=0;weap<SkelData::WEAPONSNUM;weap++)
      for(int anim=0;anim<SkelData::ANIMANAMES;anim++)
        {
        int y;
        int x=1+weap;
        if((1<<anim)==PANIC)//PANIC
          {   y=4; x=10; }
        else if((1<<anim)==SPECANIM)//
          {   y=3; x=10; }
        else
          {   y=4+anim+stand*16; }
        xls->GetCell(y,x,inp);
        data.AnimaCount[stand][anim][weap]=0;
        //logFile["bug.log"]("%d %d %d =>%s,[%s]\n",stand, weap, anim, inp.c_str(),XlsName.c_str());
        ParseAnimas(data.Animations[stand][anim][weap],&data.AnimaCount[stand][anim][weap],inp);
        //logFile["bug.log"]("^^^^^^^^^^^^^^^^^^^^^^^^^\n");

        }
        //logFile["bug.log"]("OK\n");
			SkelsCache[XlsName]=data;
			return SkelsCache[XlsName];
}
const Vehicle::SkelData& Vehicle::FillSkelsCache(std::string name)
  {
STACK_GUARD("Vehicle::FillSkelsCache");
enum {BONE1=10,BONE2=12,BARREL1=9,BARREL2=11};
  std::string inp;
  SkelCache_t::iterator it;
  it=SkelsCache.find(name);
  if(it!=SkelsCache.end()) return it->second;
  
  SkelData data;
  //
  TxtFile *xls=AnimaLibrary::GetInst()->GetTable(VEHXLS);
  DataMgr::Release(VEHXLS.c_str());
  unsigned int y=0;
  xls->FindInCol(name.c_str(),&y,0);
  data.Name=name;
  //читаем анимации
  for(int anim=0;anim<ANIMANAMES;anim++)
    {
    int x;
    x=2+anim;
    xls->GetCell(y,x,inp);
    data.AnimaCount[anim]=0;
    ParseAnimas(data.Animations[anim],&data.AnimaCount[anim],inp);
    }
  xls->GetCell(y,BONE1,data.BarrelBones[0]);
  xls->GetCell(y,BONE2,data.BarrelBones[1]);
  xls->GetCell(y,BARREL1,inp);
  sscanf(inp.c_str(),"%f;%f;%f",&data.BarrelPos[0].x,&data.BarrelPos[0].y,&data.BarrelPos[0].z);
  xls->GetCell(y,BARREL2,inp);
  sscanf(inp.c_str(),"%f;%f;%f",&data.BarrelPos[1].x,&data.BarrelPos[1].y,&data.BarrelPos[1].z);
  //читаем скины
  xls->GetCell(y,1,inp);
  data.SkinName=std::string(VEHICLESKIN+inp+".skin");
  SkelsCache[name]=data;
  return SkelsCache[name];
}

void Person::Load(const std::string &name,const std::string skins[],const std::string &sys_name)
  {
STACK_GUARD("Person::Load");
  int i;
  //читаем анимации
	Skels=FillSkelsCache(name);
	Name=Skels.Name;
	SysName=sys_name;
  //читаем скины
  for(i=0;i<SkelData::SUITSNUM+1;i++)
    {
     Suits[i]=AnimaLibrary::GetInst()->GetSkSkin(PERSONSKIN+skins[i]+".skin");
     Skels.SkinNames[i]=skins[i];
    }
   Normal=Suits[0];

   Stand=GetBit(PEACESTAY);
   CurState=LAZY;
   CurWeapon=GetBit(FREEHANDS);
   SwitchAnimation(0,0,0);
   SkAnim *r=CurAnima;//Skels.Animations[Stand][GetBit(CurState)][CurWeapon][0];
   if(!r) throw CASUS("проблемы со скелетом:"+Name);

   //присобачим все кожи к нашему скелету...
   if(!Normal)
		 throw CASUS("на диске нет такой кожи:"+skins[0]);
   Normal->ValidateLinks(CurAnima);  
   Weapons=NULL;

   CurAnima->Start(0);
   SetLocation(point3(17,37,1.2),PId2);

// ~~~~~~~~~~~~~ ShadowCaster
   if (shadow) shadow->AddObject (Normal->GetMesh());
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

   SetSoundScheme(SoundsParser::ParseForPerson(sys_name));
  }


float Person::SwitchAnimation(int Anima, float Time,float Delay)
  {
STACK_GUARD("Person::SwitchAnimation");
  if(!Skels.AnimaCount[Stand][GetBit(CurState)][CurWeapon])
		{
		    struct {char *Name;int State;}
    anims[]={"LAZY",0x01, "STAY",0x02, "HURT",0x04,"WALK",0x08, "TURN",0x10,
               "SHOOT",0x20,"SHOOTRECOIL",0x40, "DEAD",0x80, "RUN",0x100,
              "FALL",0x200, "USE",0x400, "KNOCKOUT",0x800, "RELOAD",0x1000,
              "AIMSHOOT",0x2000, "AIMSHOOTRECOIL",0x4000, "PANIC",0x8000,
              "SPECANIM",0x10000,"",0};

//		 logFile["failed_skels.log"]("%s: не удалось поставить анимацию:%s, оружие:%d\n",SysName.c_str(),anims[GetBit(CurState)].Name,CurWeapon);
		return Time;
		}

  int _Anima=Anima%Skels.AnimaCount[Stand][GetBit(CurState)][CurWeapon];
  if(Skels.Animations[Stand][GetBit(CurState)][CurWeapon][_Anima])
    {
		SkAnim *NewAnim=Skels.Animations[Stand][GetBit(CurState)][CurWeapon][_Anima];
		if(!LastPose)
			{
			LastPose = new AnimaData;
			CurAnima=NewAnim;
			*LastPose=CurAnima->Start(Time);
			}
		else
			{
			m_LastState=CurAnima->Get(LastPose,Time);
			CurAnima=NewAnim;
			*LastPose=CurAnima->Start(Time,Delay,&m_LastState);
			}
    SetLocation(Location,Angle);
    return LastPose->EndTime;
    }
 return Time;
  }


void Person::SetStanding(STAND Type,float CurTime)
{
STACK_GUARD("Person::SetStanding");
   int i=GetBit(Type);
   if(Stand==i) return;
   Stand=i;
   SwitchAnimation(rand(),CurTime,0.5);
}


void Person::SetWeapon(WEAPONS Weap, const std::string &WName, float CurTime)
  {
STACK_GUARD("Person::SetWeapon");
  CurWeapName=WName;
  std::string n=WEAPONSKIN+WName+".skin";
  int weap=GetBit(Weap);
  //if(Weapons&&CurWeapon==weap) return; для дальнейшей оптимизации нужно знать еще и имя текущего оружия
  CurWeapon=weap;

  SwitchAnimation(rand(),CurTime,0.2);

  DESTROY(Weapons);
  Weapons=AnimaLibrary::GetInst()->GetSkSkin(n);


  if(Weapons)  Weapons->ValidateLinks(CurAnima);

	SkState st=CurAnima->Get(LastPose,CurTime);
  if(st.m_BoneNum!=0)
		{
  if(Normal)    Normal->Update(&st);
  if(Weapons)   Weapons->Update(&st);
		}

  if (shadow) 
    {
    shadow->Clear();
    shadow->AddObject (Normal->GetMesh());
    if(Weapons) shadow->AddObject (Weapons->GetMesh());
    }



  UpdateSkinParms();

  }

void Person::SetArmour(ARMOUR_TYPE Arm)
{
STACK_GUARD("Person::SetArmour");
	if(Suits[Arm])
    {
    if(Normal==Suits[Arm]) return;
		Normal=Suits[Arm];
    }
  Normal->ValidateLinks(CurAnima); 

	SkState st=CurAnima->Get(LastPose,LastUpdate);
  if(st.m_BoneNum!=0)
		{
		if(Normal)    Normal->Update(&st);
		}


  if (shadow) 
    {
    shadow->Clear();
    shadow->AddObject (Normal->GetMesh());
    if(Weapons) shadow->AddObject (Weapons->GetMesh());
    }

}

void  Person::GetBarrel(const point3 &Offs, point3 *pos)
  {
STACK_GUARD("Person::GetBarrel");
  *pos=point3(0,0,0);

  if(!CurAnima) return;
	int ID=CurAnima->GetBoneID("Bip01 R Hand");
	const D3DMATRIX *mat=NULL;
	if(ID>=0)  mat=CurAnima->Get(LastPose,Timer::GetSeconds()).GetBone(ID);
  else return;
	point3 o1;
  PointMatrixMultiply(*(D3DVECTOR*)&o1,*(D3DVECTOR*)&Offs,*(D3DMATRIX*)mat);
  PointMatrixMultiply(*(D3DVECTOR*)pos,*(D3DVECTOR*)&o1,World);
  return;
  }

//Vehicle
Vehicle::Vehicle()
{
STACK_GUARD("Vehicle::Vehicle");
/* by Flif AnimaLibrary *People=AnimaLibrary::GetInst();*/

  CurState=LAZY;
  Dest=point3(0,0,0);
  LastUpdate=0;
  DestLinked=false;
  AnimaQual=Options::GetFloat("game.animaquality");
  SpeedMul=Options::GetFloat("game.anispeed");

// ~~~~~~~~~~~~~~~~~~~~~ ShadowCaster
  shadow = Shadows::CreateShadow (this);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  }

Vehicle::~Vehicle()
{
STACK_GUARD("Vehicle::~Vehicle");
  if(Normal) delete Normal;
// ~~~~~~~~~~~~~~~~~~~~~ ShadowCaster
	if (shadow) delete shadow;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}

//разбор строки, включающей в себя несколько названий, разделенных ','
void Vehicle::ParseAnimas(SkAnim *Animas[],int *Count,std::string &data)
  {
STACK_GUARD("Vehicle::ParseAnimas");
    static const std::string MASK=", .;\t";
  static const std::string EXT=".skel";
    int AnimaNames=0;
   std::string::size_type t,start=0;
   static std::string Name;
	 while(AnimaNames<MAXANIMANUM-1) 
    {
    t=data.find_first_of(MASK,start);
    if(t==data.npos)   t=data.size();
    Name=std::string(data,start,t-start);
    start=t;
    start=data.find_first_not_of(MASK,start);
    if(Name.size()&&Name[0]!='-')
      {
      Animas[AnimaNames]=AnimaLibrary::GetInst()->GetSkAnimation(VEHICLESKEL+Name+EXT);
      if(Animas[AnimaNames])
        {
        AnimaNames++;
        }
      }
    if(start==data.npos) break;
    }
  *Count=AnimaNames;

/*  int AnimaNames=0;
   std::string::size_type t;
   std::string Name;

  while((AnimaNames<MAXANIMANUM-1)&&
       (((t=data.find_first_of(", .;\t"))!=data.npos)|| data.size())) 
    {
    if(t==data.npos)   t=data.size();
    Name=std::string(data.begin(),t)+".skel";
    data.erase(0,t);
    while(data.find_first_of(", .;\t")==0) data.erase(0,1);
    if(Name[0]!='-')
      {
      Animas[AnimaNames]=AnimaLibrary::GetInst()->GetSkAnimation(VEHICLESKEL+Name);
      if(Animas[AnimaNames])
        {
        AnimaNames++;
        }
      }
    }
  *Count=AnimaNames; */
  }
//загрузка из .xls
void Vehicle::Load(const std::string &name,Vehicle::VehSounds &sounds)
  {
STACK_GUARD("Vehicle::Load");
  Sounds=sounds;
  Skels=FillSkelsCache(name);
  CurBarrel=0;
  //читаем скины
  Name=name;
  Normal=AnimaLibrary::GetInst()->GetSkSkin(Skels.SkinName);
  if(!Normal) throw CASUS("Проблемы с загрузкой техники:\n"+Skels.SkinName);

   //создадим скелет по образу и подобию...
/* by Flif    SkAnim *r=Skels.Animations[GetBit(CurState)][0];*/
   CurState=LAZY;
   SwitchAnimation(0, 0,0);
   //присобачим все кожи к нашему скелету...
   Normal->ValidateLinks(CurAnima);  

	 if(CurAnima)
   CurAnima->Start(0);
	 else throw CASUS("Нет анимации для техники:"+name);
   SetLocation(point3(17,37,1.2),PId2);

// ~~~~~~~~~~~~~ ShadowCaster
   if (shadow) shadow->AddObject (Normal->GetMesh());
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
   SetSoundScheme(SoundsParser::ParseForVehicle(&Sounds));
	 if(SoundScheme) SoundScheme->m_SysName=name;
  }
float Vehicle::SwitchAnimation(int Anima, float Time,float Delay)
  {
	STACK_GUARD("Vehicle::SwitchAnimation");
	try
		{
		if(!Skels.AnimaCount[GetBit(CurState)])return Time;
		int _Anima=Anima%Skels.AnimaCount[GetBit(CurState)];
		if(Skels.Animations[GetBit(CurState)][_Anima])
			{
			SkAnim *NewAnim=Skels.Animations[GetBit(CurState)][_Anima];
			if(!LastPose)
				{
				LastPose = new AnimaData;
				CurAnima=NewAnim;
				*LastPose=CurAnima->Start(Time);
				}
			else
				{
				m_LastState=CurAnima->Get(LastPose,Time);
				CurAnima=NewAnim;
				*LastPose=CurAnima->Start(Time,Delay,&m_LastState);
				}
			SetLocation(Location,Angle);
			return LastPose->EndTime;
			}
		}
	catch(CasusImprovisus &a)
		{
		throw CASUS("Error Setting Skeletons! "+SysName+"["+Name+"]:\n" + a.Content());
		}
  return Time;
  }
void  Vehicle::GetBarrel(const point3 &/*фиктивный параметр*/, point3 *pos)
  {
STACK_GUARD("Vehicle::GetBarrel");
  *pos=point3(0,0,0);

  if(!CurAnima) return;
	int ID=CurAnima->GetBoneID(Skels.BarrelBones[CurBarrel]);
	const D3DMATRIX *mat=NULL;
	if(ID>=0)  mat=CurAnima->Get(LastPose,Timer::GetSeconds()).GetBone(ID);
  else return;
  CurBarrel^=1;
	point3 o1;
  PointMatrixMultiply(*(D3DVECTOR*)&o1,*(D3DVECTOR*)&NULLVEC,*(D3DMATRIX*)mat);
  PointMatrixMultiply(*(D3DVECTOR*)pos,*(D3DVECTOR*)&o1,World);
  return;
  }
//* место для тестирования

float spline::Sp(float i,float q,float t)
 {
 float retVAL;
  if (q==1)
	{
	 if ((t>=i)&&(t<i+1)) retVAL=1;
	  else retVAL=0;
	}
  else
	{
	 retVAL= (t-i)   /(q-1) * Sp(i  ,q-1,t)
		+(i+q-t) /(q-1) * Sp(i+1,q-1,t);
	}
  return retVAL;
 }

point3 spline::GetPoint(float t)
  {
		point3 p(NULLVEC);
    t*=v.size()-2;
    t+=2;
		for(int q=0;q<v.size();q++)
		 {
		  p+=Sp(q,power,t)*v[q];
		 }
    return p;
  }

void spline::ShowIt()
  {
  if(v.empty()) return;
  GraphPipe *Pipe=IWorld::Get()->GetPipe();
#define num 77
  point3 pnt[num+10];
  float i=0;
  for(int n=0;i<1;i+=1.f/(num+1),n++)
    {
     pnt[n]=GetPoint(i);
    }
  Primi prim;
  texcoord tuv[num];
  unsigned col[num];
  prim.Diffuse=col;
  prim.Pos=pnt;
  prim.UVs[0]=tuv;
  prim.Prim=Primi::LINESSTRIP;
  prim.Contents=Primi::NEEDTRANSFORM;
  prim.VertNum=num;
  prim.IdxNum=0;
  Pipe->Chop("purewhite",&prim);
#undef num
  }

/**********************************************************************************/
void SoundPerson::AddEvent(EVENTTYPE type, SoundPerson::EventHandler *ev) //сообщение о событии
  {
STACK_GUARD("SoundPerson::AddEvent");
  if(!ev) return;
  Events_t::iterator it=Events.find(type),ite=Events.end();
  if(it==ite) 
    {
    Events[type]=ev;
    }
  else
    {
    delete it->second;
    it->second=ev;
    }
  }
void SoundPerson::SetOwnership(IEntity::OWNERSHIP ownby)
	{
STACK_GUARD("SoundPerson::SetOwnership");
	m_OwnBy=ownby;

	//все ниже, для того, чтобы чуваки не говорили много сразу после загрузки.
  Events_t::iterator it=Events.begin(),ite=Events.end();
	Timer::Update();
	float Time=Timer::GetSeconds()+60*10;
  for(;it!=ite;it++)
		{
    it->second->Tick(Time,true);
		}

	}


void SoundPerson::Event(unsigned type) //сообщение о событии
  {
STACK_GUARD("SoundPerson::Event");

	SoundEventsQueue::Tick(0);
  bool Stop=type&ET_STOPIT?true:false;
  type&=~ET_STOPIT;

  Events_t::iterator it=Events.find((EVENTTYPE)type),ite=Events.end();
  if(it==ite) return;
  if(Stop)     it->second->Stop();
  else
    {
		const EVENTTYPE Type=(EVENTTYPE)type;
		bool TeamMode= (Type==ET_TEAM_COME|| Type==ET_TEAM_DISMISS || Type==ET_TEAM_NEW|| Type==ET_TEAM_LEAVE);
    if(TeamMode)
      {
			SoundEventsQueue::SoundStarted(m_SysName,Type,it->second);
			}
		else if(CanSkipEv(Type))
			{
			it->second->Go();
			}
		else
			{//простое событие
			int pr=it->second->m_Priority;
			if(!m_Current || m_Current->m_Priority < pr )
				{//только если позволяет приоритет
				it->second->Go();
				if(it->second->IsPlaying())	
					{
  				if(m_Current) m_Current->Stop();
					m_Current=it->second;
					}
				}
			}
    }
  }
void SoundPerson::Tick(float Time, bool Enabled)
{
	STACK_GUARD("SoundPerson::Tick");
	bool watchdog=!Enabled;
	EVENTTYPE type;
	
	SoundEventsQueue::Tick(0);
	
	if(m_Current && !m_Current->IsPlaying()) m_Current=NULL;
	
	Events_t::iterator it=Events.begin(),ite=Events.end();
	for(;it!=ite;it++)
	{
		type=it->first;
		bool walking=CanSkipEv(type);
		int pr=it->second->m_Priority;
		int pr1=(m_Current?m_Current->m_Priority:0);
		
		if(it->second->is2d() && m_OwnBy!=IEntity::OS_PLAYER) 		watchdog=true; //2D звуки должны играть только члены команды
		if(!walking&&pr<pr1)	watchdog=true; // и только если можно
		
		it->second->Tick(Time,watchdog);
		
		
		if(it->second->IsPlaying()&&!walking && !watchdog && m_Current!=it->second)		//если звук начал играться - оповестим об этом очередь
		{
			if(m_Current) m_Current->Stop();
			m_Current=it->second;
		}
		
	}
}
void SoundPerson::UpdatePos(const point3 &pos) //обновление позиции
  {
STACK_GUARD("SoundPerson::UpdatePos");
  Events_t::iterator it=Events.begin(),ite=Events.end();
  for(;it!=ite;it++)
    it->second->UpdatePos(pos);
  }
void SoundPerson::Stop() 
  {
STACK_GUARD("SoundPerson::Stop");
  Events_t::iterator it=Events.begin(),ite=Events.end();
  for(;it!=ite;it++)
    it->second->Stop();
  }
SoundPerson::~SoundPerson()
  {
STACK_GUARD("SoundPerson::~SoundPerson");
  Stop();
	//удостовериться, что в звуковой очереди не осталось ссылок на меня
	SoundEventsQueue::PersonDied(m_SysName);
	SoundEventsQueue::Tick(0);



  Events_t::iterator it=Events.begin(),ite=Events.end();
  for(;it!=ite;it++)
    delete it->second;
  Events.clear();
  }

void SoundPerson::EventHandler::Go()
  {
STACK_GUARD("SoundPerson::EventHandler::Go");
  if(m_PlayingType!=T_BYPERCENT)
    {
#ifdef CHECK_SOUNDS
     logFile["person_sounds.log"]("из-за того, что не вероятностный ");
#endif
    return;
    }
  float p=frand();
  if(p<m_Percent)
    {
    int num=rand()%m_SndNames.size();
    #ifdef CHECK_SOUNDS
     logFile["person_sounds.log"](" \"%s\" ",m_SndNames[num].c_str());
    #endif

    if(m_Is2d)  m_Player.Play2d(m_SndNames[num],m_Cycled);
    else        m_Player.Play3d(m_SndNames[num],m_Cycled);
    }
#ifdef CHECK_SOUNDS
  else
    {
     logFile["person_sounds.log"]("из-за вероятности ");
    }
#endif

  };
void SoundPerson::EventHandler::Tick(float Time, bool ClearWatchdog)
  {
STACK_GUARD("SoundPerson::EventHandler::Tick");
  //return ;//fixme
  if(m_PlayingType!=T_BYFREQ) return;
  if(ClearWatchdog)
    {
    m_LastBeat=Time+m_Frequency*(0.5+frand());
    return ;
    }
  if(Time<=m_LastBeat) return;
  if(frand()*m_Frequency<Time-m_LastBeat)
    {
		//Console::AddString("");
		int sz=m_SndNames.size();
		int num=rand()%sz;
		while(m_LastPlayed!=-1 && sz!=1 && num==m_LastPlayed)	num=rand()%sz;
		
    if(m_Is2d)  m_Player.Play2d(m_SndNames[num],m_Cycled);
    else        m_Player.Play3d(m_SndNames[num],m_Cycled);
    m_LastBeat+=m_Frequency*(0.9+0.2*frand());
		m_LastPlayed = num;
    }
  }


void SoundsParser::ParseString(const std::string &in,
															 const std::string &fieldname,
                               SoundsParser::wave_list_t *str
                               )
  {
 STACK_GUARD("SoundsParser::ParseString");
  static const std::string NAME_PREFIX("sounds/units/");
  std::string::size_type it=0;
  std::string::size_type t=0;
  std::string data(in),Name;
  str->parm=0;
  str->names.clear();
/*
шаблон
  fieldname fname,fname,fname = number;fieldname fname,fname,fname = number%;
*/
 //поиск в строке указанного поля
  if(int sz=fieldname.size())
    {
    t=data.find(fieldname); 
    if(t==data.npos) return;
    t+=sz;
    it=t;
    }
 //выцепление имен файлов  
      do{
        t=data.find_first_of(",=",it);
        if(t==data.npos) return;
        std::string gh=std::string(data,it,t-it);
        KillSpaces(gh);
        str->names.push_back(NAME_PREFIX+gh+".wav");
        it=t+1;
        if(data[t]=='=') break;
        }while(true);
//выцепление параметра
    t=data.find_first_of(";%",it);
    Name=std::string(data,it,t-it);
    //уберем все пробелы

    str->parm=atof(Name.c_str());
  }


void SoundsParser::SetLevel(const std::string &Name)
  {
STACK_GUARD("SoundsParser::SetLevel");
  //в зависимости от уровня меняются звуки ходьбы и бега
  CurLevel=Name;
  }
void SoundsParser::Precache()
  {
STACK_GUARD("SoundsParser::Precache");
  static const std::string XlsName="scripts/species/sounds.txt";
  static const std::string XlsLevels="scripts/levels.txt";
  TxtFile *xls=AnimaLibrary::GetInst()->GetTable(XlsName);
  if(xls)
    {
    DataMgr::Release(XlsName.c_str());
    for(int i=1;i<xls->SizeX(0);i++)
      {
      std::string sysname;
      xls->GetCell(0,i,sysname);
      for(int j=1;j<xls->SizeY();j++)
        {
        std::string event_name,data;
        xls->GetCell(j,0,event_name);
        xls->GetCell(j,i,data);
        if(data.size())
          EventCache[sysname][event_name]=data;
        }
      }
    }
  
  TxtFile *xlsl=AnimaLibrary::GetInst()->GetTable(XlsLevels);
  if(xlsl)
    {
    DataMgr::Release(XlsLevels.c_str());
    for(int j=1;j<xlsl->SizeY();j++)
      {
      std::string levelname,data;
      xlsl->GetCell(j,0,levelname);
      if(!levelname.size()) break;
      xlsl->GetCell(j,21,data);
      LevelSteps[levelname]=data;
      }
    }
  CacheLoaded=true;
  }
std::string *SoundsParser::GetEvent(const std::string &sys_name, const std::string &event)
  {
STACK_GUARD("SoundsParser::GetEvent");
  StrMap_t::const_iterator ev;//событие->данные
  SysNameMap_t::const_iterator person;//человек->набор событий

  person=EventCache.find(sys_name);
  if(person==EventCache.end()) return NULL;
  ev=EventCache[sys_name].find(event);
  if(ev==EventCache[sys_name].end()) return NULL;
  return &EventCache[sys_name][event];
  }

SoundPerson *SoundsParser::ParseForPerson(const std::string &Name)
  {
STACK_GUARD("SoundsParser::ParseForPerson");
  static const SoundPerson::EventHandler::TYPE PERCENT=SoundPerson::EventHandler::T_BYPERCENT;
  static const SoundPerson::EventHandler::TYPE FREQ=SoundPerson::EventHandler::T_BYFREQ;
  static const struct{
    char sysname[30];
    char fieldname[30]; 
      SoundPerson::EventHandler::TYPE playtype; 
      SoundPerson::EVENTTYPE  ev;
    bool cycled;
    bool is2d;
    }
  FIELD_NAMES[]={
    {"random sound","2d:",FREQ,SoundPerson::ET_RANDOM2D,false,true},
    {"random sound","3d:",FREQ,SoundPerson::ET_RANDOM3D,false,false},
    {"lazy","",PERCENT,SoundPerson::ET_LAZY,false,false},
      //{"walk","",PERCENT,SoundPerson::ET_RANDOM,false,false},
      //{"run","",PERCENT,SoundPerson::ET_RANDOM,false,false},
    {"hurt","",PERCENT,SoundPerson::ET_HURT,false,false},
    {"critical hurt","",PERCENT,SoundPerson::ET_CHURT,false,false},
    {"lucky strike","",PERCENT,SoundPerson::ET_LUCKYSTRIKE,false,true},
    {"death","",PERCENT,SoundPerson::ET_DEATH,false,false},
    {"enemy sighted","",PERCENT,SoundPerson::ET_ENEMYSIGHTED,false,true},
    {"pretty girl","",PERCENT,SoundPerson::ET_GIRL,false,true},
    {"selection","",PERCENT,SoundPerson::ET_SELECTION,false,true},
    {"move command","",PERCENT,SoundPerson::ET_MOVE,false,true},
    {"attack command","",PERCENT,SoundPerson::ET_ATTACK,false,true},
    {"critical miss","",PERCENT,SoundPerson::ET_MISS,false,true},
    {"under fire","",PERCENT,SoundPerson::ET_UNDERFIRE,false,true},
    {"treatment","low:",PERCENT,SoundPerson::ET_TREATMENT_LOW,false,true},
    {"treatment","med:",PERCENT,SoundPerson::ET_TREATMENT_MED,false,true},
    {"treatment","high:",PERCENT,SoundPerson::ET_TREATMENT_HIGH,false,true},
    {"teammate","come:",PERCENT,SoundPerson::ET_TEAM_COME,false,true},
    {"teammate","dismiss:",PERCENT,SoundPerson::ET_TEAM_DISMISS,false,true},
    {"teammate","new:",PERCENT,SoundPerson::ET_TEAM_NEW,false,true},
    {"teammate","leave:",PERCENT,SoundPerson::ET_TEAM_LEAVE,false,true},
    {"enemy died","2d:",PERCENT,SoundPerson::ET_ENEMYDIED,false,true},
    {"enemy died","3d:",PERCENT,SoundPerson::ET_ENEMYDIED,false,false},
    {"enemy exploded","2d:",PERCENT,SoundPerson::ET_ENEMY_EXPLODED,false,true},
    {"enemy exploded","3d:",PERCENT,SoundPerson::ET_ENEMY_EXPLODED,false,false},
    {"open failed","",PERCENT,SoundPerson::ET_OPEN_FAILED,false,true},
    {"open succeed","",PERCENT,SoundPerson::ET_OPEN_OK,false,true},
    {"car drive failed","",PERCENT,SoundPerson::ET_CAR_FAILED,false,true},
    {"levelup","",PERCENT,SoundPerson::ET_LEVELUP,false,true},
    {"got rocket launcher","",PERCENT,SoundPerson::ET_GOT_RLAUNCHER,false,true},
    {"got autocannon","",PERCENT,SoundPerson::ET_GOT_AUTOCANNON,false,true},
    {"got shotgun","",PERCENT,SoundPerson::ET_GOT_SHOTGUN,false,true},
    {"got energy gun","",PERCENT,SoundPerson::ET_GOT_ENERGYGUN,false,true},
    {"using plasma grenade","",PERCENT,SoundPerson::ET_USE_PLASMA_GREN,false,true},      
    {"saw termoplasma exlosion","",PERCENT,SoundPerson::ET_SAW_BIGBANG,false,true},
    {"no line of fire","",PERCENT,SoundPerson::ET_NO_LOF,false,true},
    {"","",PERCENT,SoundPerson::ET_OPEN_OK,false,false},
    {"","walk:",PERCENT,SoundPerson::ET_WALK,true,false},
    {"","run:",PERCENT,SoundPerson::ET_RUN,true,false},
    {"","",PERCENT,SoundPerson::ET_SCANNER,true,false},
    };


  if(!CacheLoaded) Precache();
  
  SoundPerson *Scheme=new SoundPerson;
  Scheme->m_SysName=Name;
    wave_list_t waves;
    int num=0;
#ifdef CHECK_SOUNDS
     logFile["person_sounds.log"]("====================================================================\n");
     logFile["person_sounds.log"]("Человек:%s\n",Name.c_str());
#endif
  for(;;num++)
    {
#ifdef CHECK_SOUNDS
     logFile["person_sounds.log"]("\tТип звука:%s, поле:%s\n",FIELD_NAMES[num].sysname,FIELD_NAMES[num].fieldname);
#endif
     if(FIELD_NAMES[num].sysname[0]==0) break;
     if(FIELD_NAMES[num].ev==SoundPerson::ET_TEAM_LEAVE&&Name=="player")
       {
       int t=0;
       }
     std::string *str=GetEvent(Name,FIELD_NAMES[num].sysname);
     if(!str) continue;

     ParseString(*str,FIELD_NAMES[num].fieldname,&waves);
    if(waves.names.size())
      {
#ifdef CHECK_SOUNDS
     logFile["person_sounds.log"]("\t\tФайлы:\n");
     for(int i=0;i<waves.names.size();i++)
       {
       logFile["person_sounds.log"]("\t\t%s\n",waves.names[i].c_str());
       }
#endif
     SoundPerson::EventHandler *ev=new SoundPerson::EventHandler;
     ev->m_Cycled=FIELD_NAMES[num].cycled;
     ev->m_Is2d=FIELD_NAMES[num].is2d;
    ev->m_SndNames=waves.names;
    ev->m_Percent=waves.parm/100.f;
    ev->m_Frequency=waves.parm;
    ev->m_PlayingType=FIELD_NAMES[num].playtype;
    std::string *str=GetEvent("Priority",FIELD_NAMES[num].sysname);
		ev->m_Priority=atoi(str->c_str());

    Scheme->AddEvent(FIELD_NAMES[num].ev,ev);
      }
    }
  StrMap_t::const_iterator lvl;//событие->данные
  lvl=LevelSteps.find(CurLevel);
	num++;
  if(lvl!=LevelSteps.end())
    {
	int i;
    for(i=num;i<num+2;i++)
      {
      ParseString(lvl->second,FIELD_NAMES[i].fieldname,&waves);
      if(waves.names.size())
        {
        SoundPerson::EventHandler *ev=new SoundPerson::EventHandler;
        ev->m_Cycled=FIELD_NAMES[i].cycled;
        ev->m_Is2d=FIELD_NAMES[i].is2d;
        ev->m_SndNames=waves.names;
        ev->m_Percent=2;
        ev->m_Frequency=0;
        ev->m_PlayingType=FIELD_NAMES[i].playtype;
				ev->m_Priority=100;
        Scheme->AddEvent(FIELD_NAMES[i].ev,ev);
        }
      }
		num=i;
    }
  else	
  {
	  num = num+2;
  }
		{//сканнер
  ParseString("termoscanner=100%",FIELD_NAMES[num].fieldname,&waves);
	SoundPerson::EventHandler *ev=new SoundPerson::EventHandler;
	ev->m_Cycled=FIELD_NAMES[num].cycled;
	ev->m_Is2d=FIELD_NAMES[num].is2d;
	ev->m_SndNames=waves.names;
	ev->m_Percent=2;
	ev->m_Frequency=0;
	ev->m_PlayingType=FIELD_NAMES[num].playtype;
	ev->m_Priority=100;
		Scheme->AddEvent(FIELD_NAMES[num].ev,ev);
		}
  return Scheme;
  }
SoundPerson *SoundsParser::ParseForVehicle(Vehicle::VehSounds *Sounds)
  {
STACK_GUARD("SoundsParser::ParseForVehicle");
  static const SoundPerson::EventHandler::TYPE PERCENT=SoundPerson::EventHandler::T_BYPERCENT;

  SoundPerson *Scheme=new SoundPerson;
  Scheme->m_SysName="";
  SoundPerson::EventHandler *ev=new SoundPerson::EventHandler;
  ev->m_Cycled=true; ev->m_Is2d=false;
  ev->m_SndNames.push_back(Sounds->Walk);
  ev->m_Percent=100;  ev->m_Frequency=0;  ev->m_PlayingType=PERCENT;
  Scheme->AddEvent(SoundPerson::ET_WALK,ev);

  ev=new SoundPerson::EventHandler;
  ev->m_Cycled=false;  ev->m_Is2d=false;
  ev->m_SndNames.push_back(Sounds->Hurt);
  ev->m_Percent=100;    ev->m_Frequency=0;  ev->m_PlayingType=PERCENT;
  Scheme->AddEvent(SoundPerson::ET_HURT,ev);

  ev=new SoundPerson::EventHandler;
  ev->m_Cycled=false;  ev->m_Is2d=false;
  ev->m_SndNames.push_back(Sounds->Death);
  ev->m_Percent=100;    ev->m_Frequency=0;  ev->m_PlayingType=PERCENT;
  Scheme->AddEvent(SoundPerson::ET_DEATH,ev);

  ev=new SoundPerson::EventHandler;
  ev->m_Cycled=true;  ev->m_Is2d=false;
  ev->m_SndNames.push_back(Sounds->Engine);
  ev->m_Percent=100;    ev->m_Frequency=0;  ev->m_PlayingType=PERCENT;
  Scheme->AddEvent(SoundPerson::ET_VEH_AMBIENT,ev);
  return Scheme;
  }

/**********************************************************************************/

