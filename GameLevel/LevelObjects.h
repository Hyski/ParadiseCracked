// LevelObjects.h: interface for the DynObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEVELOBJECTS_H__C1CF8E22_5E71_4465_A088_EF6C1A261F11__INCLUDED_)
#define AFX_LEVELOBJECTS_H__C1CF8E22_5E71_4465_A088_EF6C1A261F11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../common/graphpipe/simpletexturedobject.h"
#include "../common/utils/optslot.h"
#include "../skin/keyanimation.h"
#include "objectsNG.h"
#include "explosionmanager.h"

class DynObject;
class LevelCase{
	friend class DynObject;
private:
	int Wisdom,Weight,Respawn,EXP;
	std::string SetName;
	std::string ObjName;
	bool isStorage,DB;
public:
  bool Elevator;
	LevelCase():Wisdom(0),Weight(0),Respawn(0),EXP(0),isStorage(false),DB(false),Elevator(false){};
	//это простой ящик
	bool IsStorage() const
    {
    return isStorage;
    };
	//это база данных
	bool IsDatabase() const 
    {
    return DB;
    };
	
	//узнать символьную метку ящика
	const std::string& GetName() const {return ObjName;};
	//узнать название набора предметов
	const std::string& GetItems() const {return SetName;};    
	//является ли лифтом
  bool IsElevator() const
    {
    return Elevator;
    };
	int GetWisdom() const 
    {
    return Wisdom;
    };
	int GetWeight() const 
    {
    return Weight;
    };
	int GetRespawn() const 
    {
    return Respawn;
    };
	int GetExperience() const 
    {
    return EXP;
    };
};

class SavSlot;
class NamedEffect
  {
  public:
	  unsigned int id;    // id эффекта, которое возвращает EffectManager
  std::string Name;//имя эффекта
  std::string EffectType; // название эффекта (smoke)
  point3 Color;

  point3 Position;//его расположение
  point3 Front;    //.. и ориентация
  point3 Up;       //..
  point3 Right;    //..
  void Load(FILE *f);
  void Save(FILE *f);
  bool MakeSaveLoad(SavSlot &slot);
  };
class NamedSound
  {
  public:
    enum {NS_STATIC=0x01, NS_RANDOM=0x02, NS_CYCLED=0x04};
		std::vector<std::string> Names;
		std::string Name;
    int LastSoundNum;
    unsigned Type; //тип звука
    float Freq;
    float LastTimePlayed;
  public:
    point3 Pos;
    point3 Vel;
    void Load(FILE *f);
    void Save(FILE *f);
    bool MakeSaveLoad(SavSlot &slot);
		void ParseSounds();
		const std::string &GetNextName();
    NamedSound():LastSoundNum(0),LastTimePlayed(0){;};
  };
class PositionesSnd;
class SndAuto
  {
  PositionesSnd *utter[5];
  std::string Names[5];
  bool Started;
  public:
    enum STATE{AMBIENT, OPENING,CLOSING, OPEN, CLOSE};
    STATE State;
		point3 pos;
  public:
    void SetPos(const point3 &Pos);
    void SetState(STATE state);
    SndAuto(const std::string names[5]);
    ~SndAuto();
    void Start();
    void Stop();
  };
class Station
  {
  public:
    bool HasStation;//объект останавливается 
    float StationAt;//остановка в этой точке анимации
    int  TurnsToGo; //Через сколько туров поедем. динамически меняется
    int  StopTurns; //сколько туров длиться остановка
    int  OutTurns; //на сколько туров изчезает
    int SwitchJoint;
    Station():HasStation(false){};
  };
class DynObject:public TexObject  
  {
private:
	OptSlot Props;
  public://дополнительные структуры данных
    enum DESTRTYPE{NONE=0x0000,GLASS=0x0001,BLAST=0x0002,
		               METAL=0x0004,WOOD=0x0008,STONE=0x0010,
									 CARTON=0x0020,
		               BLACKSMOKE=0x1000, WHITESMOKE=0x2000,
									 SPARKLES=0x4000, FIRE=0x8000};
  struct UseOpt
    {
    bool Usable;       //можно ли делать юз на этом объекте
    std::string UseHint; //подсказка для юза
    int         UnlockWisdom; //с каким умом ее можно взломать
    std::string UnlockKey;    //каким ключом можно открыть
    };
  struct Destruction
    {
    bool Destructable;//можно ли объект уничтожить
    int HitPoints;//какой силой можно уничтожить объект
    unsigned DType;//тип разрушения
    hit_s Damage;// сколько и какого типа повреждения наносит
    };
  
    public:
      DynObject();
      virtual ~DynObject();
    public:
      bool MakeSaveLoad(SavSlot &slot);

  public:	 //свойства звука
    SndAuto *Sounder;
    std::string Sounds[5]; //звуки, повешенные на объект
  public: //логические свойства
    //здесь те свойства, кторые относятся к игре
    std::string Name;
    bool Shadow; //оказывает ли влияние на свет
    bool Ghost;      //объект неосязаем
    bool Transparent;  //объект прозрачен

    Destruction Destruct;
    LevelCase CaseInfo;
    UseOpt UseOptions;
    Station StationInfo;
  public: //свойства формы
    BBox Bound;     //BBox в мировых координатах
    BBox LocalBound;//BBox в локальных координатах объекта  (константа)

    D3DMATRIX World;    //мировая матрица
    D3DMATRIX InvWorld; //обратная к мировой матрица
		ShapePool::Handle m_MyShape;
  public: //свойства анимации
    KeyAnimation Anima;
    bool Animated; //объект постоянно анимируется
    bool Animation;//флаг - в данный момент идет анимация
    float LastTime;
    //более высокий интерфейс
    float State;//коэффициент положения анимации 0 - начало 1 - конец
    float EndState; //коэффициент - к которому стремится анимация
    float StartTime;//Время, когда начался переход между состояниями

  public:    
    bool GetState(){return State>0.5f?true:false;};
    const UseOpt& GetUseOptions();
    virtual bool TraceRay(const ray &r, float *Pos, point3 *Norm);
    void ChangeState(float State, float Time);
    void UpdateBound();
    void EndTurn(unsigned Smth);
    void Update(float Time);
    float GetLastTime()
      {
      return Anima.GetLastTime();
      }
    void UpdateOnTime(float Time);
    DynObject& operator=(const DynObject &a);
	// by Flif !!!
	DynObject(const DynObject &a) { *this = a; }
  private:
    void ParseDestruction(std::string &Val);
  };

#endif // !defined(AFX_LEVELOBJECTS_H__C1CF8E22_5E71_4465_A088_EF6C1A261F11__INCLUDED_)
