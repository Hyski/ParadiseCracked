// LevelObjects.cpp: implementation of the DynObject class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include "LevelObjects.h"
#include "../common/saveload/saveload.h"
//#include "../common/slib/sound.h"
#include "../sound/ISound.h"
#include "../common/utils/optslot.h"
#include "LevelToLogic.h"
#include "Grid.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
DynObject::DynObject()
{
  PartNum=0;
  Name="UnInitialized";
  LastTime=0;
  Animation=false;
  State=0;
  Sounder=NULL;
  Animated=false;  
  UseOptions.Usable=false;
  UseOptions.UnlockWisdom=0;
  UseOptions.UnlockKey="";
  Ghost=false;
  Transparent=false;

}

DynObject::~DynObject()
{
if(Sounder)delete Sounder;
}

void invmat(D3DMATRIX &a, const D3DMATRIX &b);
void DynObject::UpdateOnTime(float Time)
  {
STACK_GUARD("DynObject::UpdateOnTime");
	//if(!Animation) return; //FIXME:проверить можно ли так просто
  point3 Translate;
  Quaternion Angle;
  Anima.GetTrans(&Translate,Time);
  Anima.GetAngle(&Angle,Time);

  D3DUtil_SetIdentityMatrix(World);
  Angle.ToRotationMatrix(World.m);
  World._41=Translate.x;World._42=Translate.y;World._43=Translate.z;

  invmat(InvWorld,World);
  if(Sounder)Sounder->SetPos(Translate);
  }

void DynObject::ChangeState(float state, float Time)
  {
STACK_GUARD("DynObject::ChangeState");
	if(Anima.GetLastTime()<0.001)
		{
		State=0;
		return;
		}
	else
		{
		if(Animation)
			{
			float state=fabs(EndState-State)*LastTime;
			float TimeDelta=(Time-StartTime)/state;
			if(TimeDelta>1.f) TimeDelta=1.f;
			State=(State+(EndState-State)*TimeDelta);
			}
		Sounder->SetState(state==0?SndAuto::CLOSING:SndAuto::OPENING);
		Animation=true;
		EndState=state;
		StartTime=Time;
		}
  }

void DynObject::Update(float Time)
{
STACK_GUARD("DynObject::Update");
if(Animation)
  {
  float stdiff=EndState-State;
  float state=fabs(stdiff)*LastTime;
  float TimeDelta=(Time-StartTime)/state;
  if(TimeDelta>1.f)
    {
    TimeDelta=1.f;
    Animation=false;
    State=EndState;
    stdiff=0;
    TimeDelta=0;
    if(Animated)
      {
      State=0;
      Animation=true;
      StartTime=Time;
      }
    else
      Sounder->SetState(State==0?SndAuto::CLOSE:SndAuto::OPEN);
    }
  float CurTime=(State+(stdiff)*TimeDelta)*LastTime;
  UpdateOnTime(CurTime);
  UpdateBound();
  }
}

//функция для объектов типа моноорельса
//обрабатывает переключение зон выхода
//а также управляет переключением состояния объекта
void DynObject::EndTurn(unsigned /*Smth*/)
  {
STACK_GUARD("DynObject::EndTurn");
  //если еще не пришло время, то ждем
  if(--StationInfo.TurnsToGo>0) 
    {
    return;
    }
  float StopSt;
  StopSt=StationInfo.StationAt/Anima.GetLastTime();
  float Time=Timer::GetSeconds();
  if(State<=StopSt)
    {
    StationInfo.TurnsToGo=StationInfo.OutTurns;
    ChangeState(1.f,Time);
     LevelAPI::GetAPI()->EnableJoint(StationInfo.SwitchJoint,false);
    }
  else
  if(State<=1)
    {
    State=0;
    StationInfo.TurnsToGo=StationInfo.StopTurns;
    ChangeState(StopSt,Time);
     LevelAPI::GetAPI()->EnableJoint(StationInfo.SwitchJoint,true);
    }

  }


void DynObject::UpdateBound()
{
STACK_GUARD("DynObject::UpdateBound");
	point3 np;
  Bound.Degenerate();
	for(int i=0;i<8;i++)
	{
		point3 p=LocalBound[i];
    PointMatrixMultiply(TODXVECTOR(np),TODXVECTOR(p),World);
		Bound.Enlarge(np);
	}
}
bool NamedEffect::MakeSaveLoad(SavSlot &slot)
  {
STACK_GUARD("NamedEffect::MakeSaveLoad");
  if(slot.IsSaving())
    {
    slot<<Name;
    slot<<Position<<Front<<Up<<Right<<Color;
    }
  else
    {
    slot>>Name;
    slot>>Position>>Front>>Up>>Right>>Color;
    }
  return true;
  }

void NamedEffect::Save(FILE *f)
    {
STACK_GUARD("NamedEffect::Save");
    fprintf(f,"Name:");
    SaveString(f,Name);
    fprintf(f,"Pos:%f,%f,%f\n",Position.x,Position.y,Position.z);
    fprintf(f,"Front:%f,%f,%f\n",Front.x,Front.y,Front.z);
    fprintf(f,"Up:%f,%f,%f\n",Up.x,Up.y,Up.z);
    fprintf(f,"Right:%f,%f,%f\n",Right.x,Right.y,Right.z);
    fprintf(f,"Color:%f,%f,%f\n",Color.x,Color.y,Color.z);
    }
void NamedEffect::Load(FILE *f)
    {
STACK_GUARD("NamedEffect::Load");
    fscanf(f,"Name:");
    LoadString(f,&Name);
    fscanf(f,"Pos:%f,%f,%f\n",&Position.x,&Position.y,&Position.z);
    fscanf(f,"Front:%f,%f,%f\n",&Front.x,&Front.y,&Front.z);
    fscanf(f,"Up:%f,%f,%f\n",&Up.x,&Up.y,&Up.z);
    fscanf(f,"Right:%f,%f,%f\n",&Right.x,&Right.y,&Right.z);
    fscanf(f,"Color:%f,%f,%f\n",&Color.x,&Color.y,&Color.z);
    }
bool NamedSound::MakeSaveLoad(SavSlot &slot)
  {
STACK_GUARD("NamedSound::MakeSaveLoad");
  if(slot.IsSaving())
    {
		
    slot<<Name;
    slot<<Type<<Freq;
    slot<<Pos<<Vel;
    }
  else
    {
    slot>>Name;
    slot>>Type>>Freq;
    slot>>Pos>>Vel;

		ParseSounds();
		LastSoundNum=0;
    }
  return true;
  }
void NamedSound::ParseSounds()
	{
STACK_GUARD("NamedSound::ParseSounds");
	static const std::string MASK("##");
	std::string::size_type st=0,fn,e=Name.npos;
	bool ex=false;
	do
		{
		fn=Name.find(MASK,st);
		if(fn==e) 
			{
			fn=Name.size()-st;
			ex=true;
			}
		
		std::string hh=Name.substr(st,fn-st);
		if(hh.size())		Names.push_back(hh);
		
		st=fn+MASK.size();
		}
		while(!ex && st<Name.size());

	}
const std::string& NamedSound::GetNextName()
	{
STACK_GUARD("NamedSound::GetNextName");
	if(!Names.size()) return Name;

	int num=rand()%Names.size();
	while(Names.size()>1 && num==LastSoundNum)
	   num=rand()%Names.size();
	LastSoundNum=num;
	return Names[num];
	}



void NamedSound::Save(FILE *f)
    {
STACK_GUARD("NamedSound::Save");
    fprintf(f,"Name:");
    SaveString(f,Name);
    fprintf(f,"Props:%d %f\n",&Type,&Freq);
    fprintf(f,"Pos:%f,%f,%f\n",Pos.x,Pos.y,Pos.z);
    fprintf(f,"Vel:%f,%f,%f\n",Vel.x,Vel.y,Vel.z);
    }
void NamedSound::Load(FILE *f)
    {
STACK_GUARD("NamedSound::Load");
    fscanf(f,"Name:");
    LoadString(f,&Name);
    fscanf(f,"Props:%d %f\n",Type,Freq);
    fscanf(f,"Pos:%f,%f,%f\n",&Pos.x,&Pos.y,&Pos.z);
    fscanf(f,"Vel:%f,%f,%f\n",&Vel.x,&Vel.y,&Vel.z);
    }
bool DynObject::TraceRay(const ray &r, float *Pos, point3 *Norm)
	{
STACK_GUARD("DynObject::TraceRay");
	bool inter=false;
	if(Bound.IntersectRay(r,Norm))
		{
		ray nr;
		PointMatrixMultiply(*(D3DVECTOR*)&nr.Origin,
			*(D3DVECTOR*)&r.Origin,
			InvWorld);
		VectorMatrixMultiply(*(D3DVECTOR*)&nr.Direction,
			*(D3DVECTOR*)&r.Direction,
			InvWorld);
		
		inter=TexObject::TraceRay(nr,Pos,Norm);
		if(inter)
			{
			point3 norm;
			VectorMatrixMultiply(*(D3DVECTOR*)&norm,*(D3DVECTOR*)Norm,World);
			*Norm=norm;
			}
		}
  return inter;
	}
static int GetIntValue(const std::string &str,const std::string &Key)
  {
STACK_GUARD("GetIntValue");
  std::string::size_type i,npos=std::string::npos;

  int num=0;
  i=str.find(Key);
  if(i!=npos)
    {
    i+=Key.size();
    while(isspace(str[i]))i++;
    while(isdigit(str[i])) {num*=10;num+=str[i]-'0';i++;}
    }
  return num;
  }
static std::string GetStrValue(const std::string &str,const std::string &Key)
  {
STACK_GUARD("GetStrValue");
  std::string::size_type i,j,npos=std::string::npos;
  static std::string CLOSING_SYMB(")");

  i=str.find(Key);
  j=str.find(CLOSING_SYMB,i);
  if(j==npos) j=str.size();
  if(i!=npos)
    {
    return std::string(str,i+Key.size(),j-i-Key.size());
    }
  return "";
  }
static float GetFloatValue(const std::string &str,const std::string &Key)
  {
STACK_GUARD("GetFloatValue");
  std::string val;
  val=GetStrValue(str,Key);
  return atof(val.c_str());
  }
static void  ParseStationInfo(const std::string &_str,Station *sinfo)
  {
STACK_GUARD("ParseStationInfo");
  //StopTurns(3);OutTurns(3);StopAnimation(0.5);SwitchJoint(5)
	std::string str(_str);
  if(!str.size())
    {
    sinfo->HasStation=false;//объект останавливается 
    return;
    }
	KillSpaces(str);
  sinfo->HasStation=true;//объект останавливается 
  float frame=GetIntValue(str,"stopanimation(");
  sinfo->StationAt=frame/30;//FIXME: рассчитывается, что начало анимации - 0

  sinfo->SwitchJoint=GetIntValue(str,"switchjoint(")-1;//остановка в этой точке анимации
	if(sinfo->SwitchJoint<0) 
		{
		sinfo->SwitchJoint=0;
		}
  sinfo->StopTurns=GetIntValue(str,"stopturns("); //сколько туров длиться остановка
  sinfo->OutTurns=GetIntValue(str,"outturns("); //на сколько туров изчезает
	sinfo->TurnsToGo=rand()%sinfo->OutTurns;
  }
static void GetSounds(const std::string &Str, std::vector<std::string> *a)
  {
STACK_GUARD("GetSounds");
  std::string::size_type t;
  std::string str(Str);
  a->clear();
  while(   (t=str.find_first_of(", ;\t")) !=str.npos) 
    {
    a->push_back(str.substr(0,t));
    str.erase(0,t);
    while(str.find_first_of(", ;\t")==0) str.erase(0,1);
    }
  if(str.size()) a->push_back(str);
  for(int i=0;i<a->size();i++)
    {
    //const char *d=(*a)[i].c_str();
    if((*a)[i]=="none") (*a)[i]="";
    }
  int addnum=5-a->size();
  while(addnum--)
    a->push_back("");
  }

bool DynObject::MakeSaveLoad(SavSlot &slot)
  {
STACK_GUARD("DynObject::MakeSaveLoad");
   std::map<std::string,std::string> KEYS;
   std::map<std::string,std::string>::iterator kit,kite;
  if(slot.IsSaving())
    {
    /*slot<<Name;
    slot<<(int)Shadow<<(int)Pick<<(int)Animated;
    slot<<(int)Destruct.Destructable<<Destruct.HitPoints<<(int)Destruct.DType;
    //slot<<(IsStorage?StorageName:std::string("none"));
    for(i=0;i<5;i++)
      slot<<Sounds[i];
			*/
		OptSlot out;
		int q;
    Props.Rewind();
		Props>>q;
    std::map<std::string,std::string> keys;
		while(q--)
		{
			std::string Key,Val;
			Props>>Key>>Val;
      keys[Key]=Val;
    }
    char t[100];
    sprintf(t,"%f",State>0.5?1.f:0.f);
    keys["curstate"]=t;
    out<<keys.size();
    std::map<std::string,std::string>::iterator it=keys.begin(),it1=keys.end();
    for(;it!=it1;it++)
      {
      out<<it->first<<it->second;
      }
    Props=out;
		Props.Save(slot);
    }
  else
    {
  Animated=false;  
  UseOptions.Usable=false;
  UseOptions.UnlockKey="";
  UseOptions.UnlockWisdom=0;

		OptSlot &prop=Props;
		prop.Load(slot);
		int q;
		Destruct.HitPoints=10000;

		std::string Key,Val;
		prop>>q;
		while(q--)
		{
    Key.resize(0);Val.resize(0);
			prop>>Key>>Val;
      KEYS[Key]=Val;
    }
    kite=KEYS.end();
    if(kite!= (kit=KEYS.find("name")) )
      {	
      Name=kit->second;				
      std::transform(Name.begin(),Name.end(),Name.begin(),tolower);
      }
    if(kite!= (kit=KEYS.find("shadow")) )
      {	 Shadow=(kit->second=="yes")?true:false;}

    if(kite!= (kit=KEYS.find("use")) )
      {	
      UseOptions.UseHint=kit->second;
      UseOptions.Usable=true;
      }

    if(kite!= (kit=KEYS.find("usekey")) )
      {	
      UseOptions.UnlockKey=kit->second;
      UseOptions.Usable=true;
      }
			if(kite!= (kit=KEYS.find("usewisdom")) )
			{	
      UseOptions.UnlockWisdom=atoi(kit->second.c_str());
      UseOptions.Usable=true;
      }
			if(kite!= (kit=KEYS.find("animated")) )
			{	 Animated=(kit->second=="auto")?true:false;}
			
      if(kite!= (kit=KEYS.find("ghost")) )
			{	 Ghost=(kit->second=="yes")?true:false;}

			if(kite!= (kit=KEYS.find("transparent")) )
			{	 Transparent=(kit->second=="yes")?true:false;}


			if(kite!= (kit=KEYS.find("station")) )
			{	ParseStationInfo(kit->second,&StationInfo);}

			if(kite!= (kit=KEYS.find("sounds")) )
			{	
				std::vector<std::string> SndArr;
				GetSounds(kit->second,&SndArr);
        for(int i=0;i<5;i++) Sounds[i]=SndArr[i];
			}

      if(kite!= (kit=KEYS.find("crushtype")) || kite!= (kit=KEYS.find("crashtype")) )
        ParseDestruction(kit->second);
			if(kite!= (kit=KEYS.find("hitpoints")) )
			{	Destruct.HitPoints=atoi(kit->second.c_str());}
			if(kite!= (kit=KEYS.find("storage")) )
			{	
				CaseInfo.isStorage=true;
				CaseInfo.EXP=GetIntValue(kit->second,"experience(");
				CaseInfo.Respawn=GetIntValue(kit->second,"respawn(");
				CaseInfo.SetName=GetStrValue(kit->second,"items(");
				CaseInfo.Weight=GetIntValue(kit->second,"weight(");
				CaseInfo.Wisdom=GetIntValue(kit->second,"wisdom(");
        if(!UseOptions.UnlockWisdom) UseOptions.UnlockWisdom=CaseInfo.Wisdom;
			}
			if(kite!= (kit=KEYS.find("database")) )
			{
				CaseInfo.DB=true;
				CaseInfo.EXP=GetIntValue(kit->second,"experience(");
				CaseInfo.SetName=GetStrValue(kit->second,"items(");
				CaseInfo.Wisdom=GetIntValue(kit->second,"wisdom(");
        if(!UseOptions.UnlockWisdom) UseOptions.UnlockWisdom=CaseInfo.Wisdom;
			}

			///fixme: сделать по-другому
		if(Animated&&StationInfo.HasStation)
			{Animated=false;}
		///
    if(UseOptions.UnlockKey.size()&&!UseOptions.UnlockWisdom)
      UseOptions.UnlockWisdom=150;//если дверь с ключом и не указан порог ума для взлома, то по умолчанию 150

		CaseInfo.ObjName=Name;
		Destruct.Destructable=(Destruct.HitPoints==10000?false:true);
		Sounder=new SndAuto(Sounds);
		Sounder->SetState(SndAuto::CLOSE);
		
    } //else Load


    TexObject::MakeSaveLoad(slot);
    Anima.MakeSaveLoad(slot);
     LocalBound=TexObject::GetBBox();
     
     
  LastTime=GetLastTime();
  
  if(!slot.IsSaving())
    {
			if(kite!= (kit=KEYS.find("curstate")) )
			{
      float st=atof(kit->second.c_str());
      ChangeState(st,-50);
			if(st>0.5)
				{
				bool Locked=false; //Объекты однажды открытые не должны закрываться.
				if(Grid *LevelGrid=(Grid*)HexGrid::GetInst())
					{
					if(LevelGrid->ManDirect[Name].size()) Locked=true;
					if(LevelGrid->ManInvert[Name].size()) Locked=true;
					if(LevelGrid->HeightDirect[Name].size()) Locked=false;
					}
				if(Locked) UseOptions.Usable=false;
				}
			}
    UpdateOnTime(0);//fixme:hint чтобы успела если нужно проиграться поставленная анимация
    UpdateBound();
    Update(0);
    
    if(Animated)
			{
			Animation=false;
      ChangeState(1,0);
			}

	  point3 Translate;
    Anima.GetTrans(&Translate,0);
		Sounder->SetPos(Translate);
    }
    return true;
  }
DynObject& DynObject::operator=(const DynObject &a)
  {
STACK_GUARD("DynObject::operator=");
  if(&a==this) return *this;
  TexObject::operator=(a);
	m_MyShape=a.m_MyShape;
  Anima=a.Anima;
  Name=a.Name;
  LocalBound=a.LocalBound;
  Bound=a.Bound;
  Props=a.Props;
  LastTime=a.LastTime;
  State=a.State;
  Animation=a.Animation;
  Animated=a.Animated;
  World=a.World;
  // by Flif !!!
  UseOptions=a.UseOptions;
	CaseInfo=a.CaseInfo;
  Shadow = a.Shadow;
  Destruct = a.Destruct;
  for(int i = 0; i < 5; i++)
    {
		  Sounds[i] = a.Sounds[i];
    }
  InvWorld = a.InvWorld;
  EndState = a.EndState;
  StartTime = a.StartTime;
  
  Sounder=new SndAuto(a.Sounds);
	if(a.Sounder) Sounder->pos=a.Sounder->pos;
  StationInfo = a.StationInfo;
  Ghost=a.Ghost;
  Transparent=a.Transparent;

  return *this;
  }
const DynObject::UseOpt& DynObject::GetUseOptions()
  {
  return UseOptions;
  }

extern float EffectsVol;
class PositionesSnd
 {
  public:
 ISndEmitter *m_Emitter;
 point3 Pos;
 point3 Vel;
public:
	PositionesSnd() : m_Emitter(0) {};
	point3& GetPos(){return Pos;};
	point3& GetVel(){return Vel;};
  };
static const std::string Prefix("sounds/"),Suffix(".wav");
SndAuto::SndAuto(const std::string names[5])
  {
  Started=false;
  for(int i=0;i<5;i++)
    {
    Names[i]=names[i];//std::string("sounds/")+names[i]+".wav";
    utter[i]=NULL;
    }
  State=CLOSE;
	pos=NULLVEC;
  }
SndAuto::~SndAuto()
  {
  for(int i=0;i<5;i++)
    {
    if(!utter[i]) continue;
	  if (utter[i]->m_Emitter)
	  {
		  utter[i]->m_Emitter->stop();
		  utter[i]->m_Emitter->Release();
		  utter[i]->m_Emitter = 0;
	  }
    delete utter[i];
    }
  }
void SndAuto::SetPos(const point3 &_pos)
  {
STACK_GUARD("SndAuto::SetPos");
	pos=_pos;
  for(int i=0;i<5;i++)
    {
    if(utter[i])
      {
			if (utter[i]->m_Emitter)
				{
				utter[i]->Pos = pos;
				utter[i]->m_Emitter->setPosition(pos);
				}
      }
    }
  }
void SndAuto::SetState(STATE state)
  {
STACK_GUARD("SndAuto::SetState");
  if(State!=state)
    {
    if(utter[State])
      {
		  if (utter[State]->m_Emitter)
		  {
			  utter[State]->m_Emitter->stop();
			  utter[State]->m_Emitter->Release();
			  utter[State]->m_Emitter = 0;
		  }
      }
    State=state;
    if(utter[State]&&State!=AMBIENT) 
      {
			std::string h=Prefix+Names[State]+Suffix;
			const ISndScript *snd_script = ISound::instance()->getScript("loambient");
			utter[State]->m_Emitter = ISound::instance()->createEmitter(snd_script,h.c_str());
			utter[State]->m_Emitter->play();
      }
    }
  }
void SndAuto::Start()
  {
STACK_GUARD("SndAuto::Start");
  if(!Started)
    {
    Started=true;
    for(int i=0;i<5;i++)
      {
      if(Names[i].size()) utter[i]=new PositionesSnd;
      else                utter[i]=NULL;
      }
    }
  if(utter[State]) 
      {
			std::string h=Prefix+Names[State]+Suffix;
			const ISndScript *snd_script = ISound::instance()->getScript((State==CLOSE||State==OPEN)?"lodinamiccycled":"lodinamic");
			utter[State]->Pos=pos;
			utter[State]->m_Emitter = ISound::instance()->createEmitter(snd_script,h.c_str());
			utter[State]->m_Emitter->play();
			utter[State]->m_Emitter->setPosition(pos);
      }
    if(utter[AMBIENT]) 
      {
			/*std::string h=Prefix+Names[AMBIENT]+Suffix;
			ISndScript *snd_script = ISound::instance()->getScript("loambient");
			utter[AMBIENT]->Pos=pos;
			utter[AMBIENT]->m_Emitter = ISound::instance()->createEmitter(snd_script,h.c_str());
			utter[AMBIENT]->m_Emitter->play();
			utter[AMBIENT]->m_Emitter->setPosition(pos);*/
      }
  }
void SndAuto::Stop()
  {
STACK_GUARD("SndAuto::Stop");
  for(int i=0;i<5;i++)
    {
    if(!utter[i]) continue;
	  if (utter[i]->m_Emitter)
	  {
		  utter[i]->m_Emitter->stop();
		  utter[i]->m_Emitter->Release();
		  utter[i]->m_Emitter = 0;
	  }
    }
  }
void DynObject::ParseDestruction(std::string &Val)
  {
STACK_GUARD("DynObject::ParseDestruction");
  Destruct.DType=DESTRTYPE::GLASS;
  std::string::size_type w=std::string::npos;
  Destruct.Damage.m_dmg[0].Type=Destruct.Damage.m_dmg[1].Type=Destruct.Damage.m_dmg[2].Type=hit_s::DT_NONE;
  if(Val.find("#")==w)
    {
    if(Val.find("glass")!=w) Destruct.DType=DESTRTYPE::GLASS;
    else if(Val.find("blast")!=w) Destruct.DType=DESTRTYPE::BLAST;
    else if(Val.find("metal")!=w) Destruct.DType=DESTRTYPE::METAL;
    else if(Val.find("wood")!=w)	Destruct.DType=DESTRTYPE::WOOD;
    else if(Val.find("stone")!=w) Destruct.DType=DESTRTYPE::STONE;
    else if(Val.find("carton")!=w) Destruct.DType=DESTRTYPE::CARTON;
    
    if(Val.find("blacksmoke")!=w) Destruct.DType|=DESTRTYPE::BLACKSMOKE;
    if(Val.find("whitesmoke")!=w) Destruct.DType|=DESTRTYPE::WHITESMOKE;
    if(Val.find("fire")!=w) Destruct.DType|=DESTRTYPE::FIRE;
    if(Val.find("sparkles")!=w) Destruct.DType|=DESTRTYPE::SPARKLES;
    Destruct.Damage.m_radius=2;
    Destruct.Damage.m_dmg[0].Type=hit_s::DT_STRIKE;
    Destruct.Damage.m_dmg[0].Value=20;//fixme:число от балды
    }
  else
    { //новый вариант
    std::transform(Val.begin(), Val.end(), Val.begin(), tolower);
    std::string ExpType=GetStrValue(Val,"type(");
    float Radius=GetFloatValue(Val,"rad(");
    if(Radius==0) Radius=1.f;
    struct {
      unsigned bit;
      hit_s::damage_type dam;
      float val;
      char *name;
      } exp_types[]={{DESTRTYPE::GLASS,hit_s::DT_STRIKE,5,"glass"},{DESTRTYPE::BLAST,hit_s::DT_STRIKE,10,"blast"},
      {DESTRTYPE::METAL,hit_s::DT_STRIKE,3,"metal"},{DESTRTYPE::WOOD,hit_s::DT_STRIKE,2,"wood"},
      {DESTRTYPE::STONE,hit_s::DT_STRIKE,3,"stone"},{DESTRTYPE::CARTON,hit_s::DT_NONE,0,"carton"},
      {DESTRTYPE::BLACKSMOKE,hit_s::DT_STRIKE,0,"blacksmoke"},{DESTRTYPE::WHITESMOKE,hit_s::DT_STRIKE,0,"whitesmoke"},
      {DESTRTYPE::FIRE,hit_s::DT_FLAME,3,"fire"},{DESTRTYPE::SPARKLES,hit_s::DT_STRIKE,0,"sparkles"},
      {DESTRTYPE::NONE,hit_s::DT_NONE,0,""}};
    bool skipother=false;
    for(int t=0;exp_types[t].name[0];t++)
      {
      if(ExpType.find(exp_types[t].name)!=ExpType.npos)
        {
				if(t<5)    Destruct.DType=exp_types[t].bit;
				else       Destruct.DType|=exp_types[t].bit;
        if(!skipother)
          {
          skipother=true;
          Destruct.Damage.m_dmg[0].Type=exp_types[t].dam;
          Destruct.Damage.m_dmg[0].Value=exp_types[t].val;
          }
        }
      }

    std::string Damage=GetStrValue(Val,"dam(");
    if(Damage.size())
      {
      std::string::size_type start,end;
      struct {
        hit_s::damage_type dam;
        float val;
        int start;
        } damages[2];
      damages[0].dam=damages[2].dam=damages[1].dam=hit_s::DT_NONE;
      char *a[]={"strike:","shock:","energy:","electric:","explosive:","flame:",""};
      int number=0;
      for(int j=0;a[j][0]&&number<2;j++)
        {
        start=Damage.find(a[j]);
				int len=strlen(a[j]);
        if(start==Damage.npos) continue;
        end=Damage.find_first_of(",.;",start);
        if(end==Damage.npos) end=Damage.size();
        damages[number].val=atof(std::string(Damage,start+len,end-start-len).c_str());
        damages[number].start=start+len;
        switch(j)
          {
          case 0:damages[number].dam=hit_s::DT_STRIKE;break;
          case 1:damages[number].dam=hit_s::DT_SHOCK;break;
          case 2:damages[number].dam=hit_s::DT_ENERGY;break;
          case 3:damages[number].dam=hit_s::DT_ELECTRIC;break;
          case 4:damages[number].dam=hit_s::DT_EXPLOSIVE;break;
          case 5:damages[number].dam=hit_s::DT_FLAME;break;
          }
        number++;
        }
      bool right=damages[0].start<damages[1].start;
      Destruct.Damage.m_dmg[0].Type=right?damages[0].dam:damages[1].dam;
      Destruct.Damage.m_dmg[0].Value=right?damages[0].val:damages[1].val;
      Destruct.Damage.m_dmg[1].Type=right?damages[1].dam:damages[0].dam;;
      Destruct.Damage.m_dmg[1].Value=right?damages[1].val:damages[0].val;
      }
    Destruct.Damage.m_radius=Radius;
    }
  }

 //тестирование нового подхода к динамическим объектам
class DynamicObject
  {
  public:
    class LogicProps;
    class AnimProp;
    class GeomProp;
    class UseOpt;
  public:
    void EndTurn(unsigned Smth);
    void SwitchState(float St);
    void Update(float Time);
    const LogicProps& GetLogicProps() const;
    const BBox& GetBBox();
    bool TraceRay(const ray &r, float *Pos, point3 *Norm);
    bool Damage(unsigned damage_type, float value);
  public:
    DynamicObject():LogicProperty(NULL),AnimationProperty(NULL),Shape(NULL),SoundProperty(NULL){};
    ~DynamicObject();

    DynamicObject& operator=(const DynamicObject &a);
    DynamicObject(const DynamicObject &a);
  private:
    class SoundProp;
    class CrushType;
  private:
    LogicProps *LogicProperty;
    AnimProp   *AnimationProperty;
    GeomProp   *Shape;
    SoundProp  *SoundProperty;

  };

class DynamicObject::UseOpt
  {
  public:
    bool IsDataBase() const {return isDB;};
    bool IsStorage() const {return isStorage;};
    //узнать символьную метку ящика
    const std::string& GetName() const {return ObjName;};
    //узнать название набора предметов
    const std::string& GetItems() const {return SetName;};    
    //каким ключом можно открыть
    const std::string& WhichKey() const {return UnlockKey;};    
    
    int GetWisdom() const {return Wisdom;};
    int GetWeight() const {return Weight;};
    int GetRespawn() const {return Respawn;};
    int GetExperience() const {return EXP;};


  private:
    int Wisdom,Weight,Respawn,EXP;
    std::string SetName;
    std::string ObjName;
    bool isStorage,isDB;
    std::string UnlockKey;    //каким ключом можно открыть

  public:
    UseOpt():Wisdom(0),Weight(0),Respawn(0),EXP(0),isStorage(false),isDB(false){};
    
  };

class DynamicObject::SoundProp
 {
  };
class DynamicObject::CrushType
  {
  public:
  enum DESTRTYPE{NONE=0x0000,GLASS=0x0001,BLAST=0x0002, METAL=0x0004,WOOD=0x0008,STONE=0x0010,
    CARTON=0x0020, BLACKSMOKE=0x1000, WHITESMOKE=0x2000,    SPARKLES=0x4000, FIRE=0x8000};
  unsigned Destruction;
  unsigned PrimaryType;
  unsigned SecondaryType;
  float PrimaryDamage;
  float SecondaryDamage;
  CrushType():Destruction(NONE),PrimaryDamage(0),SecondaryDamage(0){};
  };

class DynamicObject::LogicProps
  {
  public:
    float GetState() const;
    const UseOpt* GetUseOpt() const {return m_UseOptions;};
    bool Damage(unsigned damage_type, float value);
  public://оповещения
    void StateChanged(float State);
    void EndTurn();

  public:
    LogicProps():m_Crush(NULL), m_Station(NULL), m_UseOptions(NULL){};
    LogicProps(const LogicProps &a);
    LogicProps& operator=(const LogicProps &a);

    ~LogicProps();
  protected:
    CrushType *m_Crush;
    Station   *m_Station;
    UseOpt    *m_UseOptions;
  };
class DynamicObject::AnimProp
  { 
  public:
    void Init();
    void Update(DynamicObject *Parent);//ссылка на объект для оповещения об изменении состояния
    float SwitchState(float cur_time,float state);//возвращает время, когда анимация закончится
    float GetState() const;
    const D3DMATRIX& GetWorld() const;
  private:
    KeyAnimation Anima;
    bool Animated; //объект постоянно анимируется
    bool Animation;//флаг - в данный момент идет анимация
    float LastTime;
    //более высокий интерфейс
    float State;//коэффициент положения анимации 0 - начало 1 - конец
    float EndState; //коэффициент - к которому стремится анимация
    float StartTime;//Время, когда начался переход между состояниями
  };
class DynamicObject::GeomProp
  {
  public:
    const TexObject* GetShape() const{return &m_Shape;};
  private:
 		ShapePool::Handle m_MyShape;
    TexObject m_Shape;
  };




//часть реализации
DynamicObject::LogicProps::LogicProps(const LogicProps &a)
  {
  LogicProps();
  *this=a;
  }
DynamicObject::LogicProps& DynamicObject::LogicProps::operator=(const LogicProps &a)
  {
  if(!m_Crush) m_Crush=new CrushType;
  if(!m_Station) m_Station= new Station;
  if(!m_UseOptions) m_UseOptions=new UseOpt;
  *m_Crush=*a.m_Crush;
  *m_Station=*a.m_Station;
  *m_UseOptions=*a.m_UseOptions;
  return *this;
  }
DynamicObject::LogicProps::~LogicProps()
  {
    delete m_Crush;
    delete m_Station;
    delete m_UseOptions;
  }
DynamicObject& DynamicObject::operator=(const DynamicObject &a)
  {
  if(!LogicProperty)  LogicProperty = new LogicProps;
  *LogicProperty=*a.LogicProperty;
  if(!AnimationProperty)  AnimationProperty = new AnimProp;
  *AnimationProperty=*a.AnimationProperty;
  if(!Shape)  Shape = new GeomProp;
  *Shape=*a.Shape;
  if(!SoundProperty)  SoundProperty = new SoundProp;
  *SoundProperty=*a.SoundProperty;
  return *this;
  }
DynamicObject::DynamicObject(const DynamicObject &a)
  {
  DynamicObject();
   *this=a;
  }
DynamicObject::~DynamicObject()
  {
  delete LogicProperty;
  delete AnimationProperty;
  delete Shape;
  delete SoundProperty;
  }
