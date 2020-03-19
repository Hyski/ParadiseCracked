/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: базовый класс для формирования всех элементов интерфейса.
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   22.05.2000

************************************************************************/
#ifndef _LAGOON_H_
#define _LAGOON_H_

//---------- Лог файл ------------
#ifdef _DEBUG_INTERFACE
class CLog;
extern CLog lagoon_log;
#define lag	lagoon_log["lagoon.log"]
#else
#define lag	/##/
#endif
//--------------------------------

//------------- Структурка ZOrder -------------------
class Lagoon;

struct ZOrder
{
	Lagoon* pLagoon;
	ZOrder(Lagoon* pL)
	{
		pLagoon = pL;
	};
};
//---------------------------------------------------

class GraphPipe;

class Lagoon
{
protected:
	//	виртуальные координаты
	int m_x;
	int m_y;										//	начало области ввода
	int m_width;	
	int m_height;									//	размеры области ввода	
	//	регион ввода	
	HRGN m_hVirtualRegion;							//	виртуальная региональная область ввода
	HRGN m_hScreenRegion;							//	экранная региональная область ввода
protected:
	std::string m_sName;							//	имя-идентификатор области
	std::string m_sClass;							//	имя типа объекта
	std::map<std::string,Lagoon*> m_ChildMap;		//	карта дочерних областей
protected:
	//	то что связанно с фокусом ввода от клавиатуры
	bool m_bFocus;	//	флаг наличия фокуса
public:
	bool m_bFocusValid;		//	флаг верности флага m_bFocus
	bool m_bMouseCapture;	//	хочу чтобы мышь принадлежала только мне
public:
	Lagoon *m_pParent;								//	родительская лагуна
	bool m_bScaleX;									//	при трансформации использовать X-координату
	bool m_bScaleY;									//	при трансформации использовать Y-координату
public:
	//	все что связанно с z координатой
	float m_zBegin;		//	начало отпущенного промежутка - текущая координа Z
	float m_zEnd;		//	конец отпущенного промежутка
protected:
	//	расположение объектов по z
	std::set<ZOrder> m_zBuffer;	
public:
	//	данные для отрисовки
	point3 m_Vertex[4];
	texcoord m_TexCoord[4];
	Primi m_Object;
public:
	std::string m_sShader;
	bool m_bVisible;			//	отображать данную лагуну или нет
	bool m_bEnable;				//	замечать ли эту лагуну или нет
public:
	Lagoon(const char *pName);
	virtual ~Lagoon();
public:
	//	функции для работы с дочерними окнами
	void AddChild(Lagoon* pLagoon);
	void DeleteChild(const char *pName);
	void ClearChildMap(void);
	void RefreshZ(void);
public:
	//	функции изменения размеров (виртуальные координаты)
	virtual void SetRect(RECT *pRect);
	virtual void SetRect(int left,int top,int right,int bottom);
	virtual void SetRegion(HRGN hRegion);
	//	функции изменения местоположение (виртуальные координаты)
	virtual void MoveTo(int x,int y);
	//	функция перевода виртуальных координат в экранные
	virtual void TransformToScreen(void);
	//	функции отображения
	virtual void PaintRegion(HDC hdc);
	virtual void Render(GraphPipe *lpGraphPipe);
	virtual void SetVisible(const char *pChild,bool bVisible);
	virtual void SetVisible(bool bVisible);
	//	функция подстройки лагуны при смене видеорежима
	virtual void Tune(void);
	//	функции передачи управления
	virtual void Tick(void);
	//	перемещения мыши
	virtual void MouseIn(void);
	virtual void MousePresent(void);
	virtual void MouseOut(void);
	virtual void MouseCaptureOut(void);
	//	перемещения фокуса
	virtual void SetFocus(void);
	virtual void KillFocus(void);
	virtual void FocusPresent(void);
	//	проверка на принадлежность к указанному классу
	virtual bool IsClass(const char *pClass);
	//	функии передачи уведомлений
	virtual void SetMessage(unsigned int iMsg,Lagoon *pLagoon);
protected:
	virtual void SetVertex(void);
public:
	//	информационные функции
	inline const char* Name(void);
	inline bool IsFocus(void);
	//	информация о размерах и местоположении (виртуальные координаты)
	inline int X(void);
	inline int Y(void);
	inline int Width(void);
	inline int Height(void);
	//	информация о принадлежности точки региону (экранные координаты)
	inline BOOL PtInLagoon(int x,int y);
	//	информация об окне в точке (m_bEnable == true && m_bVisible == true)
	Lagoon* GetLagoon(int x,int y,bool bAlways);
	Lagoon* GetLagoon(const char *pName);
	//	информация об окне на котором фокус
	Lagoon* GetFocus(void);
};

//********************* Структурка ZOrder *****************************//
inline bool operator < (const ZOrder& x, const ZOrder& y)
{
	if(x.pLagoon->m_zEnd > y.pLagoon->m_zEnd)
		return true;
	
	return false;
}
//---------------------------------------------------

//********************* Информационные функции *****************************//
const char* Lagoon::Name(void)
{
	return m_sName.c_str();
}

bool Lagoon::IsFocus(void)
{
	return m_bFocus;
}

int Lagoon::X(void)
{
	return m_x;
}

int Lagoon::Y(void)
{
	return m_y;
}

int Lagoon::Width(void)
{
	return m_width;
}

int Lagoon::Height(void)
{
	return m_height;
}

//	x,y - экранные координаты
BOOL Lagoon::PtInLagoon(int x,int y)
{
	return PtInRegion(m_hScreenRegion,x,y);
}

inline Lagoon* Lagoon::GetLagoon(const char *pName)
{
	std::map<std::string,Lagoon*>::iterator il;

	il = m_ChildMap.find(pName);
	if(il != m_ChildMap.end())
		return il->second;

	return 0;
}

#endif	//_LAGOON_H_