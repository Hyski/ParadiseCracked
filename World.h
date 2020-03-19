#ifndef __WORLD_H_
#define __WORLD_H_
#include "iworld.h"
class GraphPipe;
#define SINGLETON(Var,Class) (Var?Var:(Var=new Class))
class IExplosionAPI
{
public:
	enum damage_type{
    DT_NONE,
			DT_STRIKE,
			DT_SHOCK,
			DT_ENERGY,
			DT_ELECTRIC,
			DT_EXPLOSIVE,
			DT_FLAME,
	};
	struct Damage
	{
		damage_type Type;
		float				Power;
	};
	public:
		virtual void DamageEntity(const point3 &Pos, const point3& dir, const Damage dam[3], unsigned Handle)=0;
};
class ParadiseWorld: public IWorld
  {
  private:
    CharacterPool *Ents;
    ItemsPool *Items;
    EffectManager *EffManager;
    GameLevel *Level;
    GraphPipe *Pipe; //WARNING:Это только указатель, он здесь не создается!
  private:
    bool Functional;
		public: //игровые функции
		private:
			std::map<std::string,std::string> Sys2RealLevName;
			void LoadLevelNames();
  public:
    ParadiseWorld();
    virtual ~ParadiseWorld();
  public: //Синглетоны
    CharacterPool* GetCharacterPool();
    ItemsPool*  GetItemsPool();
    EffectManager* GetEffects();
  public: //Функционирование
    void Init(GraphPipe *pipe){	LoadLevelNames();Pipe=pipe;Functional=true;};
    void Close();
		OptionsUpdater* GetOptionsUpdater();

    GraphPipe* GetPipe(){return Pipe;};
    void Tick();
    void Draw();
  public:
	  GameLevel* GetLevel(void);
		std::string GetLevelName(const std::string lev_sys_name)const;
    void ChangeLevel(const std::string &LevName);
    void CreateLevel();

    bool TraceRay();
    Camera* GetCamera();
    bool MakeSaveLoad(Storage &st);
  };

#endif