/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
		Класс SlotEngine предназначается для операций со слотами,
		контейнером которых он и является.
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   18.09.2001

************************************************************************/
#ifndef _SLOTS_ENGINE_H_
#define _SLOTS_ENGINE_H_

#include "../Common/UI/Widget.h"

//#define _DEBUG_SLOTS_ENGINE

class SEMigrationMgrController;
class SlotsEngine;
class SESlotView;
class SEItemView;
class BaseThing;
class SEShadow;
class DXImage;
class Static;
class SESlot;
class Text;

extern const char* g_InventoryPictures;

//**********************************************************************
//	class SEItem
class SEItem
{
	friend SESlot;
public:
	enum STATE 
	{
		ST_NORMAL		=    0,
		ST_HIGHLIGHTED	= 1<<1,
		ST_GRAYED		= 1<<2
	};
	static const int m_SlotIdIsAbsent;		//	идентификатор слота которого не существует
private:
	unsigned int m_State;					//	состояние
	ipnt2_t m_Position;						//	позиция в слоте (в клетках слота)
	ipnt2_t m_Size;							//	размеры (в клетках слота)
	BaseThing* m_BaseThing;					//	связь с логикой
	int m_SlotId;							//	принадлежность к слоту
	SEItemView* m_View;						//	внешний вид слота
public:
	SEItem(const char* system_name,const ipnt2_t& size,BaseThing* bs);
	virtual ~SEItem();
public:
	//	установить состояние итема
	void SetState(unsigned int state);
	//	установить текстовое поле на итеме
	void SetText(const char* text);
	//	установить позицию итема в слоте (координаты в ячейках слота)
	void SetPosition(const ipnt2_t& pos);
	//	получить позицию итема
	const ipnt2_t& Position(void) const {return m_Position;}
	//	получить размер итема
	const ipnt2_t& Size(void) const {return m_Size;}
	//	получить идентификатор слота
	int SlotId(void) const {return m_SlotId;}
	//	сообщить о начале перетаскивания
	void OnDrag(Widget* new_container);
	//	сообщить о конце перетаскивания
	void OnDrop(void);
	//	получить картинку
	WidgetImage* GetImage(void) const;
	//	получить скриновые координаты отображения итема
	ipnt2_t GetScreenPlace(void) const;
public:
	BaseThing* GetBaseThing(void) {return m_BaseThing;}
private:
	//	переместить внешний вид итема (экранные координаты)
	void MoveTo(Widget* container,int x,int y);
	//	переместить внешний вид итема (экранные координаты)
	void ScrollTo(int x,int y);
	//	инициализировать клиппер
	void EnableClipper(bool enable);
};

//**********************************************************************
//	class SEItemView
class SEItemView : public Widget
{
public:
	static const char* m_WidgetType;
	static const char* m_WrongImage;
private:
	int m_OffsetX;					//	смещение картинки от точного размера
	int m_OffsetY;					//	смещение картинки от точного размера
	int m_Width;					//	точная ширина
	int m_Height;					//	точная высота
	int m_DragX;	
	int m_DragY;
	bool m_IsDragged;
private:
	enum IMAGE
	{
		I_NORMAL,
		I_HIGHLIGHTED,
		I_GRAYED,
		MAX_IMAGE
	};
	DXImage* m_Images[MAX_IMAGE];
	IMAGE m_CurImage;
	Text* m_Text;
public:
	SEItemView(const char* name,const char* system_name,const ipnt2_t& size_in_cells);
	virtual ~SEItemView();
public:
	void MoveTo(int x,int y);
	void SetState(unsigned int state);
	//	сообщить о начале перетаскивания
	void OnDrag(void);
	//	сообщить о конце перетаскивания
	void OnDrop(void);
	//	получить картинку отображения
	WidgetImage* GetBaseImage(void) const;
	//	установить текстовое поле на итеме
	void SetText(const char* text);
private:
	//	производим какие-то действия
	bool Tick(void);
	//	обработка системного сообщения
	void OnSystemChange(void);
public:
	virtual const char* Type(void) const {return m_WidgetType;}
};

//**********************************************************************
//	class SESlot
class SESlot
{
public:
	class Iterator;
	enum STATE {ST_NORMAL,ST_SELECTED};
	enum EVENT_TYPE {ET_NONE,ET_MOUSE_LEAVE,ET_MOUSE_PRESENT,ET_LEFT_CLICK,ET_RIGHT_CLICK};	
	enum MODE 
	{
		M_INSIDE_INTERFACE,					//	слот располагается в интерфейсе и имеет координаты
		M_OUTSIDE_INTERFACE					//	слот является всей областью, не являющейся интерфейсом
	};
	enum PLACEMENT
	{
		P_NORMAL_SLOT,						//	в слот можно пометить столько итемов сколько влезет
		P_ONE_ITEM_IN_SLOT,					//	в слот можно поместить только один итем
		P_UNLIMITED,						//	в слот можно поместить неограниченное число итемов
	};
	enum ALIGN
	{
		A_LEFT_TO_RIGHT,					//	размещение в слоте происходит слева направо
		A_RIGHT_TO_LEFT,					//	размещение в слоте происходит справа налево
	};
	enum EXPANSION_MODE
	{
		EM_TO_RIGHT,
		EM_TO_DOWN
	};
	static const int m_CellWidth;			//	ширина ячейки в пикселах
	static const int m_CellHeight;			//	высота ячейки в пикселах
	static const float m_DetonateTime;		//	время, после которого считается зависание мышки
private:
	int m_X;								//	координаты в относительных коорлинтах (в пикселах)
	int m_Y;								//	координаты в относительных коорлинтах (в пикселах)
	int m_Width;							//	ширина слота в ячейках
	int m_Height;							//	высота слота в ячейках
	//	массив ячеек
	enum CELL_STATE {CS_EMPTY,CS_FILLED,CS_INACCESSIBLE};
	std::vector<std::vector<CELL_STATE> > m_Cells;
	ipnt2_t m_CellsOffset;					//	логическое смещение ячеек
	EXPANSION_MODE m_ExpMode;				//	режим расширения для бесконечного слота
	PLACEMENT m_Placement;					//	свойства расположения итемов
	ALIGN m_Align;							//	свойства размещения итемов
	MODE m_Mode;							//	режим работы слота
	//	обратная связь
	SlotsEngine* m_FeedBack;
	//	внешний вид слота
	SESlotView* m_View;
	//	массив итемов в слоте
	std::list<SEItem*> m_Items; 
	SEItem* m_ActiveItem;					//	предыдущий активный итем
	float m_ActivateTime;					//	время активирования итема
	bool m_IsFirstTimeStart;				//	в первый раз посылка уведомления START
private:
	int m_Id;								//	идентификатор слота
	STATE m_State;							//	состояние слота
public:
	SESlot(int id,Widget* container,SlotsEngine* feed_back);
	virtual ~SESlot();
public:
	//	установить расположение слота (x,y - координаты в пикселах; width,height - в клетках слота)
	void SetLocation(int x,int y,int width,int height);
	void SetState(STATE state);
	STATE State(void) const {return m_State;}
	void SetPlacement(PLACEMENT placement);
	void SetAlign(ALIGN align);
	void SetExpMode(EXPANSION_MODE em);
	void SetInaccessibleCells(int left,int top,int right,int bottom);
	void SetSelectedImage(const char* image_name,int x,int y);
public:
	//	перед вызовом этой функции необходимо вызвать CanInsert() для данного итема
	void Insert(SEItem* item);
	void Swap(SEItem* item_bottom,SEItem* item_top);
	//	удаляет итем из слота (но не из памяти, объект не уничтожается)
	SEItem* Remove(SEItem* item);
	//	удаляет все итемы из слота и удаляет объекты в памяти
	void Clear(void);
	bool CanInsert(SEItem* item,const ipnt2_t* pos = 0);
	bool CanSwap(SEItem* item_bottom,SEItem* item_top);
	//	получить идентификатор слота
	int Id(void) const {return m_Id;}
	//	узнать, поместится ли итем в слот в принцине
	bool CanPlace(SEItem* item);
	//	узнать, поместится ли итем в слот в принцине
	bool CanPlace(const ipnt2_t& size);
	//	узнать является ли ячейка доступной (экранные координаты, не должны выходить за рамки слота)
	bool IsAccessibleCell(int x,int y);
	//	узнать является ли ячейка доступной (координаты в ячейках слота)
	bool IsAccessibleCell(const ipnt2_t& cell_xy);
	//	сместить логические ячейки на указанное кол-во
	void Scroll(int dx,int dy);
	//	сместить логические ячейки в начало
	void UnScroll(void);
	//	получить логические размеры слота
	ipnt2_t Size(void) const;
	//	обработать мышинное колесико
	bool ProcessMouseWheel(float dz);
public:
	//	работа с итератором
	SESlot::Iterator Begin(void);
	SESlot::Iterator End(void);
private: //	работа с ячейками
	//	установить состояние прямоугольной области ячеек
	void SetCells(int left,int top,int right,int bottom,CELL_STATE state);
	//	проверить состояние ячеек
	bool CheckCells(int left,int top,int right,int bottom,CELL_STATE state);
	//	определить место 
	bool GetPlace(const ipnt2_t& size,const ipnt2_t* where,ipnt2_t* result);
	//	разместить слева на право
	bool LeftToRight(const ipnt2_t& size,ipnt2_t* result);
	//	разместить справо на лево
	bool RightToLeft(const ipnt2_t& size,ipnt2_t* result);
	//	качание маятника
	bool SwingOfPendulum(const ipnt2_t& size,const ipnt2_t& where,ipnt2_t* result);
	//	время детонирования события OnHanging
	bool IsDetonate(void) const;
	//	перевод скриновых координат в ячейки слота
	ipnt2_t ScreenToSlot(int x,int y);
	//	перевод в скриновые координаты
	ipnt2_t SlotToScreen(int x,int y);
	//	узнать находится ли точка в итеме (x,y - координаты слота)
	bool PtInItem(const SEItem* item,const ipnt2_t& slot_xy);
	//	показать тень (экранные координаты)
	void ShowShadow(bool enable,SEItem* item_for = 0,int x = 0,int y = 0);
	//	показывать ли тень
	bool IsShowShadow(void);
	//	уничтожаем лишние ячейки 
	void ReleaseRedundantCells(void);
public:
	//	обаботка события
	void OnEvent(int x,int y,EVENT_TYPE type);

#ifdef _DEBUG_SLOTS_ENGINE
public:
	void Render(HDC hdc);
#endif
};

//**********************************************************************
//	class SESlotView
class SESlotView : public Widget
{
public:
	static const char* m_WidgetType;
private:
	SESlot::EVENT_TYPE m_PrevEvent;						
	SESlot* m_FeedBack;
	Widget* m_SelectedImage;
public:
	SESlotView(Widget* container,SESlot* feed_back);
	virtual ~SESlotView();
public:
	//	установить картинку на выделение
	void SetSelectedImage(const char* image_name,int x,int y);
	//	установить/снять выделение
	void SetSelected(bool enable);
private:
	//	обработка мышинного ввода
	bool ProcessMouseInput(VMouseState* pMS);
	//	обработка чего-либо
	bool Tick(void);
};


//************************************************************************//
//	class SESlot::Iterator
class SESlot::Iterator
{
private:
	std::list<SEItem*>::iterator m_it;
public:
	Iterator(){};
	Iterator(std::list<SEItem*>::iterator it){m_it = it;}
	virtual ~Iterator(){};
public:
	Iterator& operator++();
	Iterator operator++(int);
	SEItem* operator->(void);
	SEItem& operator*(void);
	bool operator!=(const Iterator& it);
};

inline SESlot::Iterator& SESlot::Iterator::operator++()
{
	++m_it;
	return *this;
}

inline SESlot::Iterator SESlot::Iterator::operator++(int)
{
	Iterator it = *this;
	++(*this);
	return it;
}

inline SEItem* SESlot::Iterator::operator->(void)
{
	return *m_it;
}

inline SEItem& SESlot::Iterator::operator*(void)
{
	return *operator->();
}

inline bool SESlot::Iterator::operator!=(const Iterator& it)
{
	return m_it != it.m_it;
}

//**********************************************************************
//	class SESlot
inline SESlot::Iterator SESlot::Begin(void)
{
	return Iterator(m_Items.begin());
}

inline SESlot::Iterator SESlot::End(void)
{
	return Iterator(m_Items.end());
}

//**********************************************************************
//	class SEMigrationMgr
class SEMigrationMgr
{
public:
	static const char* m_ShadowImageName;
private:
	SEMigrationMgrController* m_Controller;
	SlotsEngine* m_FeedBack;
	Widget* m_Container;
	Widget* m_Shadow;
	SEItem* m_Item;
	ipnt2_t m_Offset;
public:
	SEMigrationMgr(SlotsEngine* feed_back,Widget* container);
	virtual ~SEMigrationMgr();
public:
	void Insert(SEItem* item);
	SEItem* Remove(void);
	SEItem* GetItem(void);
	void SetController(SEMigrationMgrController* controller);
public:
	//	активировать тень (экранные координаты)
	void ShowShadow(bool enable,const ipnt2_t& xy);
private:
	//	создать новую тень
	void CreateNewShadow(WidgetImage* wi,const ipnt2_t& size_in_cells);
};

//**********************************************************************
//	class SEMigrationMgrController
class SEMigrationMgrController
{
public:
	virtual ~SEMigrationMgrController() {}
	virtual void OnDrag(void) = 0;
	virtual void OnDrop(void) = 0;
};

//**********************************************************************
//	class SEController
class SEController
{
public:
	enum LIGHTING_MODE 
	{
		LM_START,				//	требуется начать подсвечивать
		LM_FINISH				//	тебуется завершение подсветки
	};
public:
	virtual ~SEController(){}
	virtual void OnLighting(SEItem* item,LIGHTING_MODE lm) = 0;
	virtual void OnDrag(SESlot* slot,SEItem* item) = 0;
	virtual void OnDrop(SESlot* slot,SEItem* item,const ipnt2_t& pos) = 0;
	virtual void OnItemClick(SEItem* item) = 0;
};

//**********************************************************************
//	class SlotsEngine
class SlotsEngine
{
	class Iterator;
private:
	std::map<int,SESlot*> m_Slots;
	Widget* m_Container;
	SEController* m_Controller;						//	связь с логикой
	SEMigrationMgr* m_MMgr;							//	мышка
public:
	SlotsEngine(Widget* container);
	virtual ~SlotsEngine();
public:
	//	создать слот c соответсвующим идентификатором и свойствами
	SESlot* InsertSlot(int id);
	//	удалить слот (удалить объект из памяти)
	bool RemoveSlot(int id);
	//	получить слот по идентификатору
	SESlot* SlotAt(int id);
public:	
	//	создать экземпляр итема
	SEItem* CreateItem(const char* system_name,const ipnt2_t& size,BaseThing* bs);
public:
	//	установить контроллер
	void SetController(SEController* controller);
	SEController* Controller(void) const {return m_Controller;}
	SEMigrationMgr* MigrationMgr(void) const {return m_MMgr;}
public:
	//	работа с итератором
	SlotsEngine::Iterator Begin(void);
	SlotsEngine::Iterator End(void);
#ifdef _DEBUG_SLOTS_ENGINE
public:
	void Render(void);
#endif
};

//************************************************************************//
//	class SlotsEngine::Iterator
class SlotsEngine::Iterator
{
private:
	std::map<int,SESlot*>::iterator m_it;
public:
	Iterator(){};
	Iterator(std::map<int,SESlot*>::iterator it){m_it = it;}
	virtual ~Iterator(){};
public:
	Iterator& operator++();
	Iterator operator++(int);
	SESlot* operator->(void);
	SESlot& operator*(void);
	bool operator!=(const Iterator& it);
};

inline SlotsEngine::Iterator& SlotsEngine::Iterator::operator++()
{
	++m_it;
	return *this;
}

inline SlotsEngine::Iterator SlotsEngine::Iterator::operator++(int)
{
	Iterator it = *this;
	++(*this);
	return it;
}

inline SESlot* SlotsEngine::Iterator::operator->(void)
{
	return m_it->second;
}

inline SESlot& SlotsEngine::Iterator::operator*(void)
{
	return *operator->();
}

inline bool SlotsEngine::Iterator::operator!=(const Iterator& it)
{
	return m_it != it.m_it;
}

//**********************************************************************
//	class SlotsEngine
inline SlotsEngine::Iterator SlotsEngine::Begin(void)
{
	return Iterator(m_Slots.begin());
}

inline SlotsEngine::Iterator SlotsEngine::End(void)
{
	return Iterator(m_Slots.end());
}

//**********************************************************************
//	class SEFactory
class SEFactory
{
public:
	static const char* GenerateSlotName(void);
	static const char* GenerateItemName(void);
	static void GenerateShadow(DXImage* image);
	static void GenerateHighlight(DXImage* image);
	static void GenerateGrayed(DXImage* image);
	static DXImage* DoHighlightedImage(DXImage* base_image);
	static DXImage* DoGrayedImage(DXImage* base_image);
};

#endif	//_SLOTS_ENGINE_H_