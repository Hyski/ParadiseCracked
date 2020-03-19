/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Класс, реализующий отметину на стенах, полах, и т.д.                                                                 
                                                                                
                                                                                
   Author: Alexander Sorokin aka Grom                                                                     
***************************************************************/                
                                                                                
                                                                                
 
#if !defined(AFX_MARK_H__1B53F7E0_49D0_11D4_A0E0_525405F0AA60__INCLUDED_)
#define AFX_MARK_H__1B53F7E0_49D0_11D4_A0E0_525405F0AA60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

////////////////////////////////////////////////////////////////////////////
//
// базовый класс отметины
//
////////////////////////////////////////////////////////////////////////////
class BaseMark
{
public:
	enum{MAXTRIS=1000};
	BaseMark(const point3& Position, const float Radius, const std::string& Shader);
	virtual ~BaseMark() {FindTris=true;}
	// функции
	virtual void Update(const float CurTime) = 0;// обновление состояния

	// данные
	bool Dead;// если умер - true - будет уничтожен
  bool FindTris;// если необходимо заново собрать плоскости , то true
	unsigned int Col;// текущий цвет
	bool Visible;// видимость
  bool Savable;//должен ли эффект сохраняться

	point3 Pos;// позиция
	float Rad;// радиус
	int TriNum;// количество треугольников
	point3 Tris[MAXTRIS*3];// массив вершин треугольников
	unsigned int Color[MAXTRIS*3];// цвета вершин
	texcoord UVs[MAXTRIS*3];  //текстурные координаты вершин
	std::string ShaderName;//имя шейдера
};

////////////////////////////////////////////////////////////////////////////
//
// класс постоянной отметины и динамического освещения
//
////////////////////////////////////////////////////////////////////////////
class Mark : public BaseMark
{
public:
	// функции

	// конструктор
	Mark(const point3& Position, const float Radius, const std::string& Shader) :
	  BaseMark(Position, Radius, Shader), dynamic(false){Savable=true;}
	// обновление состояния
	virtual void Update(const float )
	{  
		if(dynamic) FindTris = true;
		else FindTris = false;
	}

	// флаг, показывающий движется ли отметина
	bool dynamic;
};

////////////////////////////////////////////////////////////////////////////
//
// класс мерцающей отметины
//
////////////////////////////////////////////////////////////////////////////
class Flicker : public Mark
{
public:
	// функции

	// конструктор
	Flicker(const point3& Position, const float Radius, const std::string& Shader) :
	  Mark(Position, Radius, Shader){}
	// обновление состояния
	virtual void Update(const float CurTime);
};

////////////////////////////////////////////////////////////////////////////
//
// класс вспышки
//
////////////////////////////////////////////////////////////////////////////
class Splash : public BaseMark
{
public:
	// функции

	// конструктор
	Splash(const point3& Position, const float Radius, const std::string& Shader,
		const point3& Color, const float BirthTime, const float LifeTime);
	// обновление состояния
	virtual void Update(const float CurTime);

	// данные
	float birthtime;// время рождения
	float lifetime;// время жизни
	point3 start_color;// начальный цвет
};

////////////////////////////////////////////////////////////////////////////
//
// класс временной отметины
//
////////////////////////////////////////////////////////////////////////////
class DMark: public Splash
{
public:
	// функции

	// конструктор
	DMark(const point3& Position, float Radius,
		const std::string& Shader,	const point3& Color,
		float BirthTime, float LifeTime,
		float SwitchTime, unsigned int StartColor) :
	  Splash(Position, Radius, Shader, Color, BirthTime, LifeTime)
	  { switchtime = SwitchTime; Col = StartColor; }
	// обновление состояния
	virtual void Update(const float CurTime);
	// данные
	float switchtime;// время зажигания и гашения
};

/////////////////////////////////////////////////////////////////////////
// Пул для отметин
/////////////////////////////////////////////////////////////////////////
class MarksPool
{
public:
	class iterator;
	friend class iterator;
	void clear(); //очищает массив
	void Draw();
	void Update(float Time);
	void Add(BaseMark *Mark){m_Marks.push_back(Mark);}
	void MakeSaveLoad(SavSlot &slot);
	//операции перебора
	iterator begin();
	iterator end();
	//создание/уничтожение
	MarksPool(){;};
	~MarksPool();
private:
	typedef std::list<BaseMark*> MarksSet;
	MarksSet m_Marks;
};
class MarksPool::iterator
{
public:
	iterator& operator++(){if(m_Cur!=m_End)m_Cur++;return *this;};//++i
  iterator operator++(int){iterator i(*this);	if(m_Cur!=m_End)m_Cur++;return i;};						
	bool operator !=(const iterator &a)const {return m_Cur!=a.m_Cur;};
	BaseMark* operator->()const {return *m_Cur;};
	BaseMark* operator*()const {return *m_Cur;};
	iterator(const MarksSet::iterator &beg,const MarksSet::iterator &end,const MarksSet::iterator &cur)
		:m_Beg(beg),m_End(end),m_Cur(cur){};
private:
	MarksSet::iterator m_Beg,m_End,m_Cur;
};
inline MarksPool::iterator MarksPool::begin()
{return iterator(m_Marks.begin(),m_Marks.end(),m_Marks.begin());}
inline MarksPool::iterator MarksPool::end()
{return iterator(m_Marks.end(),m_Marks.end(),m_Marks.end());}

#endif // !defined(AFX_MARK_H__1B53F7E0_49D0_11D4_A0E0_525405F0AA60__INCLUDED_)
