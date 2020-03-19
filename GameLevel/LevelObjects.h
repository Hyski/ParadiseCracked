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
	//��� ������� ����
	bool IsStorage() const
    {
    return isStorage;
    };
	//��� ���� ������
	bool IsDatabase() const 
    {
    return DB;
    };
	
	//������ ���������� ����� �����
	const std::string& GetName() const {return ObjName;};
	//������ �������� ������ ���������
	const std::string& GetItems() const {return SetName;};    
	//�������� �� ������
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
	  unsigned int id;    // id �������, ������� ���������� EffectManager
  std::string Name;//��� �������
  std::string EffectType; // �������� ������� (smoke)
  point3 Color;

  point3 Position;//��� ������������
  point3 Front;    //.. � ����������
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
    unsigned Type; //��� �����
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
    bool HasStation;//������ ��������������� 
    float StationAt;//��������� � ���� ����� ��������
    int  TurnsToGo; //����� ������� ����� ������. ����������� ��������
    int  StopTurns; //������� ����� ������� ���������
    int  OutTurns; //�� ������� ����� ��������
    int SwitchJoint;
    Station():HasStation(false){};
  };
class DynObject:public TexObject  
  {
private:
	OptSlot Props;
  public://�������������� ��������� ������
    enum DESTRTYPE{NONE=0x0000,GLASS=0x0001,BLAST=0x0002,
		               METAL=0x0004,WOOD=0x0008,STONE=0x0010,
									 CARTON=0x0020,
		               BLACKSMOKE=0x1000, WHITESMOKE=0x2000,
									 SPARKLES=0x4000, FIRE=0x8000};
  struct UseOpt
    {
    bool Usable;       //����� �� ������ �� �� ���� �������
    std::string UseHint; //��������� ��� ���
    int         UnlockWisdom; //� ����� ���� �� ����� ��������
    std::string UnlockKey;    //����� ������ ����� �������
    };
  struct Destruction
    {
    bool Destructable;//����� �� ������ ����������
    int HitPoints;//����� ����� ����� ���������� ������
    unsigned DType;//��� ����������
    hit_s Damage;// ������� � ������ ���� ����������� �������
    };
  
    public:
      DynObject();
      virtual ~DynObject();
    public:
      bool MakeSaveLoad(SavSlot &slot);

  public:	 //�������� �����
    SndAuto *Sounder;
    std::string Sounds[5]; //�����, ���������� �� ������
  public: //���������� ��������
    //����� �� ��������, ������ ��������� � ����
    std::string Name;
    bool Shadow; //��������� �� ������� �� ����
    bool Ghost;      //������ ���������
    bool Transparent;  //������ ���������

    Destruction Destruct;
    LevelCase CaseInfo;
    UseOpt UseOptions;
    Station StationInfo;
  public: //�������� �����
    BBox Bound;     //BBox � ������� �����������
    BBox LocalBound;//BBox � ��������� ����������� �������  (���������)

    D3DMATRIX World;    //������� �������
    D3DMATRIX InvWorld; //�������� � ������� �������
		ShapePool::Handle m_MyShape;
  public: //�������� ��������
    KeyAnimation Anima;
    bool Animated; //������ ��������� �����������
    bool Animation;//���� - � ������ ������ ���� ��������
    float LastTime;
    //����� ������� ���������
    float State;//����������� ��������� �������� 0 - ������ 1 - �����
    float EndState; //����������� - � �������� ��������� ��������
    float StartTime;//�����, ����� ������� ������� ����� �����������

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
