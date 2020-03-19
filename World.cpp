#include "precomp.h"
#include "IWorld.h"
#include "World.h"

#include "common/graphpipe/graphpipe.h"
#include "Options/Options.h"
#include "Interface/OptionsScreen.h"
#include "Interface/WidgetSoundMgr.h"
#include "gamelevel/gamelevel.h"
#include "gamelevel/leveltologic.h"
#include "gamelevel/scattereditems.h"
#include "Common/3dEffects/EffectManager.h"
#include "Common/D3DApp/GammaControl/GammaControl.h"
#include "Skin/Shadow.h"
#include "character.h"
#include "sound/ISound.h"
static ParadiseWorld w; //ќб этой переменной никто никогда не узнает
                        //все делаетс€ через интерфейс IWorld
ParadiseWorld::~ParadiseWorld()
  {
  //Close();
  }
ParadiseWorld::ParadiseWorld()
:Functional(false),Ents(0),Items(0),EffManager(0),Level(0),Pipe(0)
	{
	World=this;
	};

void ParadiseWorld::Close()
  {
STACK_GUARD("ParadiseWorld::Close");
  Functional=false;
  if(Level) Level->Unload();
  delete Ents;
  delete Items;
  delete EffManager;
  delete Level;
  EffManager=NULL;
  Level=NULL;
  Ents=NULL;
  Items=NULL;
  //delete World;
  }

EffectManager* ParadiseWorld::GetEffects()
  {
STACK_GUARD("ParadiseWorld::GetEffects");
  if(!EffManager&&Functional)
    {
    EffManager=new EffectManager;
    EffManager->Init(Get()->GetPipe());
    EffManager->SetEffectsQuality(Options::GetFloat("system.video.effectsquality"));
    }
  return EffManager;
  }
void ParadiseWorld::Tick()
  {
STACK_GUARD("ParadiseWorld::Tick");
  float t=Timer::GetSeconds();
  GetCharacterPool()->Update(t);
  }
void ParadiseWorld::Draw()
  {
STACK_GUARD("ParadiseWorld::Draw");
  /* by Flif GraphPipe *P=GetPipe();*/
  GetCharacterPool()->Draw();
  GetItemsPool()->Draw();
  }


void ParadiseWorld::ChangeLevel(const std::string &LevName)
  {
STACK_GUARD("ParadiseWorld::ChangeLevel");
  static std::string name="",CurGameLevel="";
  if(Level)
    {
    Level->Unload();
	GetEffects()->clear();
    GetPipe()->BindToGrid(0,NULLVEC);
	GetPipe()->UnloadShaderFile("shaders.sys/effects.shader");
	GetPipe()->UnloadShaderFile("shaders.sys/units.shader");
    }
  else {Level=new GameLevel;}
  if(LevName.size())
    {
	GetPipe()->RegisterShaderFile("shaders.sys/effects.shader",false);
	GetPipe()->RegisterShaderFile("shaders.sys/units.shader",false);
    CurGameLevel = LevName;
    
    //«агружаем модель уровн€
    name=LevName;
    Level->Load(name);
    //«агружаем шейдеры дл€ уровн€
    name=std::string("shaders/")+std::string(LevName)+".shader";
    GetPipe()->RegisterShaderFile(name);

    Level->LinkShaders(GetPipe());
    GetCamera()->Move(NULLVEC);
    GetCamera()->Rotate(0,0);
    Level->Env.LevelChanged(LevName,GetPipe());
    //прив€зываем камеру к уровню
    GetPipe()->BindToGrid((GeneralGrid*)Level->LevelGrid,NULLVEC);
    }
  }
bool ParadiseWorld::TraceRay()
  {
  return false;
  }
CharacterPool*  ParadiseWorld::GetCharacterPool()
  {
  return SINGLETON(Ents,CharacterPool);
  }

Camera* ParadiseWorld::GetCamera()
  {
  return GetPipe()->GetCam();
  }
bool ParadiseWorld::MakeSaveLoad(Storage &st)
  {
STACK_GUARD("ParadiseWorld::MakeSaveLoad");
	IWorld::Get()->GetLevel()->MakeSaveLoad(st);
	GetPipe()->RegisterShaderFile("shaders.sys/effects.shader",false);
	GetPipe()->RegisterShaderFile("shaders.sys/units.shader",false);
	IWorld::Get()->GetCharacterPool()->MakeSaveLoad(st);
  SavSlot itcell(&st, "items");

GetItemsPool()->MakeSaveLoad(itcell);
  return true;
  }
ItemsPool*  ParadiseWorld::GetItemsPool()
  {
  return SINGLETON(Items,ItemsPool);
  }


/***************************/
  float EffectsVol;
  float AmbientVol;
  float VoicesVol;
  float MusicVol;
/***************************/
/*
class RealUpdater:public OptionsUpdater
{
private:
	enum MODE {M_GET,M_SET};
public:
	//	обработать гамму
	float Gamma(MODE mode,float value = 0)
	{
		const float offset = 0;
		const float length = 3;

		switch(mode)
		{
		case M_GET:
			return (Options::GetFloat("system.video.gamma")-offset)/length;
		case M_SET:
			value = offset+value*length;
			Options::Set("system.video.gamma",value);
			GammaControl::SetGamma(value);
			break;
		}

		return 0;
	}
private:
	//	вернуть значение
	float GetValue(TYPE type)
	{
		const MODE mode = M_GET;

		switch(type)
		{
		case T_MARKS_ON_WALLS:
			break;
		case T_DYNAMIC_LIGHT:
			break;
		case T_ENVIRONMENT:
			break;
		case T_PATH:
			break;
		case T_HEX_GRID:
			break;
		case T_SHADOW_Q:
			break;
		case T_EFFECTS_Q:
			break;
		case T_GAMMA: return Gamma(mode);
		case T_VOICES_VOLUME:
			break;
		case T_AMBIENT_VOLUME:
			break;
		case T_EFFECTS_VOLUME:
			break;
		case T_MUSIC_VOLUME:
			break;
		case T_ANIMA_SPEED:
			break;
		case T_CAMERA_SPEED:
			break;
		case T_MOUSE_SENSITIVITY:
			break;
		}

		return 0;
	}
	//	установить значение
	void SetValue(TYPE type,float value)
	{
		const MODE mode = M_SET;

		switch(type)
		{
		case T_MARKS_ON_WALLS:
			break;
		case T_DYNAMIC_LIGHT:
			break;
		case T_ENVIRONMENT:
			break;
		case T_PATH:
			break;
		case T_HEX_GRID:
			break;
		case T_SHADOW_Q:
			break;
		case T_EFFECTS_Q:
			break;
		case T_GAMMA: Gamma(mode,value); break;
		case T_VOICES_VOLUME:
			break;
		case T_AMBIENT_VOLUME:
			break;
		case T_EFFECTS_VOLUME:
			break;
		case T_MUSIC_VOLUME:
			break;
		case T_ANIMA_SPEED:
			break;
		case T_CAMERA_SPEED:
			break;
		case T_MOUSE_SENSITIVITY:
			break;
		}
	}
};
*/

class RealUpdater : public OptionsUpdater
{
public:
	void SetMarksOnWalls(bool value)
	{//	отметины на стенах
		Options::Set("game.marks_on_walls",value);			
		if(IWorld::Get()->GetLevel())
			IWorld::Get()->GetLevel()->EnableMarks(GameLevel::MT_STATIC,value);
	}
	void SetDynamicLights(bool value)
	{//	динамический свет
		Options::Set("game.dynamic_lights",value);
		if(IWorld::Get()->GetLevel())
		IWorld::Get()->GetLevel()->EnableMarks(GameLevel::MT_DYNAMIC,value);
	}
	void SetEnvironment(bool value)
	{//	окружающа€ среда
		Options::Set("game.environment",value);
		if(IWorld::Get()->GetLevel())
		IWorld::Get()->GetLevel()->Env.Enable(value);
	}
	
    void SetPath(bool value)
	{
		Options::Set("game.mark_path",value);
        LogicAPI::GetAPI()->OptionsChanged();
	}
	void SetHexGrid(bool value)
	{
		Options::Set("game.mark_wave_front",value);
		Options::Set("game.mark_exit_zone",value);
		Options::Set("game.mark_lands",value);
        LogicAPI::GetAPI()->OptionsChanged();
	}
	void SetShadowsQ(float value)
	{
		const float length = 6;
		Options::Set("system.video.shadowquality",static_cast<int>(value*length));
		Shadows::OptionsChanged();
	}
	void SetEffectsQ(float value)
	{
		IWorld::Get()->GetEffects()->SetEffectsQuality(value);
		Options::Set("system.video.effectsquality",value);
	};
	void SetGamma(float value)
	{
		const float offset = 0;
		const float length = 3;
		value = offset+value*length;
		Options::Set("system.video.gamma",value);
		GammaControl::SetGamma(value);
	}
	void SetVoicesVolume(float value)
    {
      //[0..100]
//    value*=100;
		Options::Set("game.sound.voicesvol",(int)(value*100));
//    VoicesVol=50*20*log(value/100.f);
	ISound::instance()->setVolume(ISound::cSpeech,value);
    };
	void SetAmbientVolume(float value)
    {
      //[0..100]
//    value*=100;
		Options::Set("game.sound.ambientvol",(int)(value*100));
//    AmbientVol=50*20*log(value/100.f);
	ISound::instance()->setVolume(ISound::cAmbient,value);
//    if(IWorld::Get()->GetLevel())
//      {
//    IWorld::Get()->GetLevel()->Stop();
//    IWorld::Get()->GetLevel()->Start();
//      }
    };
	void SetEffectsVolume(float value)
    {
      //[0..100]
//    value*=100;
		Options::Set("game.sound.effectsvol",(int)(value*100));
//    EffectsVol=50*20*log(value/100.f);
	ISound::instance()->setVolume(ISound::cEffects,value);
//    if(IWorld::Get()->GetLevel())
//      {
//    IWorld::Get()->GetLevel()->Stop();
//    IWorld::Get()->GetLevel()->Start();
//      }
	//	обновление громкости играющих звуков в меню
//	WidgetSoundMgr::Instance()->UpdateVolume();
    };
	void SetMusicVolume(float value)
    {
      //[0..100]
//    value*=100;
		Options::Set("game.sound.musicvol",(int)(value*100));
//    MusicVol=50*20*log(value/100.f);
	ISound::instance()->setVolume(ISound::cThemes,value);
    };
	void SetAnimaSpeed(float value)
		{
    //[0.5..5]
		extern float SpeedMul;
    extern float SkSpeed;
		value=saturate(value,0.0f,1.f);
		SpeedMul=0.5+value*(5-0.5f);
    SkSpeed=SpeedMul;
		Options::Set("game.anispeed",SpeedMul);
		};
	void SetCameraSpeed(float value)
		{
    //[0.5..5]
		extern float KEYSPEED;
		extern float MOUSESPD;
		value=saturate(value,0.0f,1.f);
		KEYSPEED=MOUSESPD=0.5+value*(5-0.5f);
		Options::Set("game.camera_speed",KEYSPEED);
		};
	void SetMouseSensitivity(float value)
	{
		const float offset = 0.5;
		const float length = 6;
		value = offset+value*length;
		Options::Set("system.input.sensitivity",value);
		Input::Mouse()->SetSensitivity(value);
	}
	void SetEnemyMovement(bool value)
	{
		Options::Set("game.hidden_movement.enemy_related.move",value);
	}
	void SetNeutralMovement(bool value)
	{
	    Options::Set("game.hidden_movement.neutral_related.move",value);
	}
	void SetAllyMovement(bool value)
	{
	    Options::Set("game.hidden_movement.friend_related.move",value);
	}
	void SetCivilianMovement(bool value)
	{
	    Options::Set("game.hidden_movement.civilian_humans_traders.move",value);
	}
	void SetVehicleMovement(bool value)
	{
	    Options::Set("game.hidden_movement.civilian_vehicles.move",value);
	}
	void SetEnemyFire(bool value)
	{
		Options::Set("game.hidden_movement.enemy_related.shoot",value);
	}
	void SetNeutralFire(bool value)
	{
	    Options::Set("game.hidden_movement.neutral_related.shoot",value);
	}
	void SetAllyFire(bool value)
	{
	    Options::Set("game.hidden_movement.friend_related.shoot",value);
	}
	void SetCivilianFire(bool value)
	{
	    Options::Set("game.hidden_movement.civilian_humans_traders.shoot",value);
	}
	void SetVehicleFire(bool value)
	{
		Options::Set("game.hidden_movement.civilian_vehicles.shoot",value);
	}
public:
	bool GetPath(void)
	{
		return Options::GetInt("game.mark_path") != 0;
	}
	bool GetHexGrid(void)
	{
		return Options::GetInt("game.mark_wave_front") ||
			   Options::GetInt("game.mark_exit_zone")  ||
			   Options::GetInt("game.mark_lands");
	}
};

RealUpdater Updater;

OptionsUpdater* ParadiseWorld::GetOptionsUpdater()
{
	return &Updater;
}

GameLevel* ParadiseWorld::GetLevel(void)
{
	return Level;
}
void ParadiseWorld::CreateLevel()
{
	if(!Level)
	{
		Level = new GameLevel();
	}
	//GetPipe()->RegisterShaderFile("shaders.sys/effects.shader");
	//GetPipe()->RegisterShaderFile("shaders.sys/units.shader");
}

void ParadiseWorld::LoadLevelNames()
{
STACK_GUARD("ParadiseWorld::LoadLevelNames");
  TxtFile table(DataMgr::Load("scripts/levels.txt"));
  DataMgr::Release("scripts/levels.txt");
	unsigned int num;


	Sys2RealLevName.clear();

    if(!table.FindInRow("LevelName",0,&num))
      {
			return;
      }
  std::string levsysname,lev_name;
  for(int i=1;i<table.SizeY();i++)
	{
		table.GetCell(i,0,levsysname);
		table.GetCell(i,num,lev_name);
		if(!levsysname.size()) break;
    Sys2RealLevName[levsysname]=lev_name;
	}

	}
std::string ParadiseWorld::GetLevelName(const std::string lev_sys_name)	const
	{
	static const std::string Empty("");
	std::map<std::string,std::string>::const_iterator it=	 Sys2RealLevName.find(lev_sys_name);
	if(it!=Sys2RealLevName.end()) return it->second;
	else return Empty;
	}
