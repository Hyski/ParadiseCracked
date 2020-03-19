#ifndef __IWORLD_H_
#define __IWORLD_H_
class GameLevel;
class CharacterPool;
class ItemsPool;
class EffectManager;
class Camera;
class SavSlot;
class Storage;
class Entity;
class GraphPipe;
class OptionsUpdater;

class IWorld
  {
  public: //Синглетоны
    virtual CharacterPool* GetCharacterPool()=0;
    virtual ItemsPool*  GetItemsPool()=0;
    virtual EffectManager* GetEffects()=0;
  public: //Функционирование
    virtual void Init(GraphPipe *)=0;
    virtual void Close()=0;
    virtual void Tick()=0;
    virtual void Draw()=0;
  public:
		virtual OptionsUpdater* GetOptionsUpdater()=0;
    virtual GraphPipe* GetPipe(){return NULL;};
    virtual GameLevel* GetLevel()=0;
		virtual std::string GetLevelName(const std::string lev_sys_name)const=0;

    virtual void ChangeLevel(const std::string &LevName)=0;
    virtual void CreateLevel()=0;

    virtual bool TraceRay()=0;
    virtual Camera* GetCamera()=0;
    virtual bool MakeSaveLoad(Storage &st)=0;
  protected:
    static IWorld *World;
    IWorld(){World=this;};
  public:
    virtual ~IWorld(){};
    //FIXME:разобраться с throw
    static IWorld* Get(){if(!World){throw 666;} return World;};
  };


#endif