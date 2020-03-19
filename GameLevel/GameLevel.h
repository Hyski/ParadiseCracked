// GameLevel.h: interface for the GameLevel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMELEVEL_H__792421A0_3176_11D4_A0E0_525405F0AA60__INCLUDED_)
#define AFX_GAMELEVEL_H__792421A0_3176_11D4_A0E0_525405F0AA60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../gamelevel/mark.h"
#include "../common/textureMgr/dibdata.h"
#include "levelobjects.h"
#include "longshot.h"
#include "../world.h"
#include "grid.h"

struct BSPnode;
class Storage;
class GameLevel;

class ISndEmitter;

class SoundUtter:
public NamedSound
{
public:
	ISndEmitter *m_Emitter;

	SoundUtter() : m_Emitter(0) {}

	point3& GetPos(){return Pos;};
	point3& GetVel(){return Vel;};
};

///////////////////////////////////////////////////
struct BusPath
  {
  typedef std::vector<ipnt2_t> PathSeg;
  std::string Name;
  std::vector<PathSeg> Segments;
  };
struct DestPoints
  {
  struct DestPoint
    {
    point3 Pos;
    point3 Dir;
    int Special;
    };
  typedef std::vector<DestPoint> PntVec;
  std::string Name;
  PntVec Points;
  };
///////////////////////////////////////////////////
class IExplosionAPI;
class ShapePool;
struct hit_s;
class LogicDamageStrategy;
class DynObjectPool
{
public:
  class iterator;
	friend class iterator;
  enum TRACE_TYPE{TT_SIGHT, TT_PHYSIC};
	enum SWITCH_STATE{SS_OFF,SS_ON,SS_SWITCH};

	float SwitchState(const std::string& Name,float Time,SWITCH_STATE State=SS_SWITCH);
  void DoExplosion(std::string ObjName,const hit_s& hit, LogicDamageStrategy *LogicStrategy);//При взрыве уничтожаются объекты...

  bool TraceRay(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm,std::string *Name, TRACE_TYPE trace_type=TT_SIGHT);
  bool TraceSegment(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm,std::string *Name);
	void EraseByName(const std::string& Name);
	const LevelCase* GetCase(const std::string& Name);
	iterator begin(const std::string& ObjName);
	iterator end();
	void Draw(GraphPipe *Pipe, bool trans);
	void CollectHexesForObject(const std::string &ObjName,std::vector<ipnt2_t> *vec);
	void ClearObjectInfluence();


  //очищать множество должен клиент!
  void CollectObjectsForPoints(const std::vector<point3> &pnts, std::set<std::string> *objects);
private:
	friend class GameLevel;
	void Start();
	void Stop();
	void Update(float Time);
	void Load(FILE *in);
	void MakeSaveLoad(SavSlot &s);
	void EndTurn(unsigned Smth);
	void UpdateHG(DynObject *Obj);
	void UpdateHGFull();
	void ChangeHG(const std::string &ObjName,float State, float z);
  void SetStationInfo(const std::string &Name,const Station &st);
	void Clear();
private:
  typedef std::multimap<std::string,DynObject> ObjContaner;
	typedef ObjContaner::iterator ObjectsIt;
	static ShapePool *m_Shapes;
	ObjContaner m_Objects;
public:
		ShapePool* GetShapePool();
		~DynObjectPool();

};
class DynObjectPool::iterator
{
	public:
		typedef DynObjectPool::ObjContaner::iterator ObjIt;
		iterator(const ObjIt &Start,const ObjIt &End,const std::string& Name)
			:m_Start(Start),m_Cur(Start),m_End(End),m_Name(Name)
		{
			if(!Name.size()) return;
			while(m_Cur!=m_End && m_Cur->second.Name!=m_Name)
				m_Cur++;
		};
		bool operator ==(const iterator &a)const {return m_Cur==a.m_Cur;};
		bool operator !=(const iterator &a)const {return m_Cur!=a.m_Cur;};
		iterator &operator++()
		{
			while(m_Cur!=m_End)
			{
				m_Cur++;
				if(m_Cur == m_End || m_Cur->second.Name==m_Name || !m_Name.size()) break;
			}
			return *this;
		};
		iterator operator++(int)
		{
			iterator i(*this);
			while(m_Cur!=m_End)
			{
				m_Cur++;
				if(m_Cur == m_End || m_Cur->second.Name==m_Name || !m_Name.size()) break;
			}
			return i;
		};
		DynObject* operator->()
		{return m_Cur==m_End?NULL:&m_Cur->second;};
		DynObject& operator*()
		{return m_Cur->second;};
	private:
		std::string m_Name;
		ObjIt m_Start,m_End,m_Cur;
};
inline DynObjectPool::iterator DynObjectPool::begin(const std::string& ObjName)
{
	return iterator(m_Objects.begin(),m_Objects.end(),ObjName);
}
inline DynObjectPool::iterator DynObjectPool::end()
{
	return iterator(m_Objects.end(),m_Objects.end(),"");
}

class Grid;
class Bsp;
class DynamicVB;
class VShader;

class GameLevel
{
	
public:
    enum {MAXMARKS=50};
    typedef std::vector<VShader*> ShaderPtrVector;
    typedef Grid* LevelGridptr;
	// вектор для хранения постоянных отметин
    typedef std::vector<Mark>    MarkSet;
    typedef std::vector<NamedEffect> EffectSet;
    typedef std::vector<SoundUtter> SoundSet;
    typedef std::set<int> LightmapSet;
    typedef std::map<std::string,BusPath> BusPaths;
    typedef std::map<std::string,DestPoints> PathPoints;
    typedef std::map<std::string,KeyAnimation> CameraPaths;
		enum MARK_TYPE {MT_DYNAMIC, MT_STATIC};
public: 
    unsigned VERSION;
    std::string LevelName;

    LevelGridptr LevelGrid;
    LongShot  Env;
protected:
	MarksPool Marks;
		LightmapSet LMs;
		ShaderPtrVector ShaderPtr;  //ссылки на шейдеры
		SoundSet  LevelSounds;
		EffectSet LevelEffects;
		bool DoMarks,DoLights;
public://касательно интерактивных объектов
	DynObjectPool LevelObjects;
	//?
	BusPaths BPaths;  //маршруты дыижения автобусов
	PathPoints PPoints;//точки следования для лиюдей
	CameraPaths CamPaths;
	DynamicVB *BspVB;
public:
	bool RoofVisible;

	void EnableMarks(MARK_TYPE type, bool enable);
	bool MarksEnabled(MARK_TYPE type);

	void UpdateObjects(float Time);     //обновить состояния объектов ... может и не надо
	bool MakeSaveLoad(std::string FName,bool IsSaving);
	void Load(std::string FName);
	void LoadBSP(std::string FName);
	bool MakeSaveLoad(Storage &slot);
	void LoadGrid(std::string FName);
	void Unload();
	void Start();
	void Stop();
	GameLevel();
	virtual ~GameLevel();
public :
  //сообщение о конце тура
  void EndTurn(unsigned Smth);
	// добавить отметину
	void AddMark(BaseMark *L);
	//оставить в определенной точке уровня след
	void DoMark(const point3 &Where, const float Rad, const std::string& Shader);
	// проапдейтить все отметины (кроме постоянных)
	void UpdateMarks(float Time);
	//сопоставить названиям шейдеров указатели на них
	void LinkShaders(GraphPipe *Pipe);
	//отрисовать все отметки
	void DrawMarks();
	//возвращает точку пересечения луча с уровнем
	bool TraceRay(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm, bool Sight=true); //Sight = видимость или физика
	//вызывается перед запросом данных
	void PrepareData(GraphPipe *Pipe); 
	//вызывается после окончания сеанса 
	void Draw(GraphPipe *Pipe,bool Transparent);
	// собрать и рассчитать все треугольники, на кот. влияет отметина
	static void CollectPlanes(point3 &Pos, float Rad, BaseMark &m,int CurIndex=0);
private:
  void LoadCameras(); //загрузка сплайнов для камеры
  void LoadEffects(SavSlot &slot);
  void LoadSplines(SavSlot &lvlspl);
	void CreateVB();
	void CreateLMtextures(Storage *st=NULL);
};

#endif // !defined(AFX_GAMELEVEL_H__792421A0_3176_11D4_A0E0_525405F0AA60__INCLUDED_)
