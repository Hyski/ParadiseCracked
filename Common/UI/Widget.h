/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
		 Базовый класс для предмета интерфеса.
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   26.02.2001

************************************************************************/
#ifndef _WIDGET_H_
#define _WIDGET_H_

#include "VData.h"

class Widget;
class WidgetView;
class WidgetImage;
class WidgetFocus;
class WidgetMouseFocus;
class WidgetKeyboardFocus;

//	ссылка на виджет
struct WidgetRef
{
	Widget* m_pWidget;
	WidgetRef(){m_pWidget = 0;}
	WidgetRef(Widget* pWidget){m_pWidget = pWidget;}
};

//	описание виджета
class Widget
{
public:
	class Iterator;
	class Location;
	class Clipper;
	class Player;
	friend Iterator;
	friend Location;
	friend Clipper;
	friend WidgetMouseFocus;
	friend WidgetKeyboardFocus;
private:
	//	имя данного Widget'а
	std::string m_sName;
	//	родительский виджет
	Widget *m_pParent;
	//	флаг видимости
	bool m_bVisible;
	//	флаг доступности для ввода
	bool m_bEnable;
	//	картинка отображения
	WidgetImage *m_pImage;
	//	месторасположение виджета
	Location *m_pLocation;
	//	модуль отсечения изображения
	Clipper* m_Clipper; 
	//	модуль управления фокусом ввода
	WidgetFocus *m_pFocus;
protected:
	//	модуль управления звуковым проигрывателем
	Player* m_Player;
public:
	struct Tip
	{
		std::string m_Text;
		bool m_Activated;
		Tip() : m_Activated(false) {}
	} m_Tip;
private:
	//	список ссылок на дочернии виджеты
	std::list<WidgetRef> m_lWidgets;
public:
	//	при создании виджета, необходимо задать ему имя
	Widget(const char *pName);
	virtual ~Widget();
public:
	//	поместить в данный виджет экземпляр другого виджета
	//	если виджет с таким же именем уже существует возвращается 0
	virtual Widget* Insert(Widget *pWidget);
	//	уничтожить экземпляр другого виджета по имени, при не удаче
	//	(если данного виджета не имеется) возвращается false
	virtual bool Delete(const char *pName);
	//	отделить виджет с указанным именем от данного родительского виджета
	virtual Widget* Remove(const char *pName);
	//	получить дочерний виджет по имени
	virtual Widget* Child(const char *pName); 
	//	поместить дочерний виджет (по имени) сверху
	virtual bool BringToTop(const char *pName);
	//	поместить дочерний виджет (по имени) снизу
	virtual bool BringToBottom(const char *pName);
	//	получить родительский виджет
	virtual Widget* Parent(void) const;
	//	получить самый глубокий дочерний виджет из точки (0 - если виджета нет)
	virtual Widget* FromPoint(int x,int y);
	//	получить первый по глубине дочерний виджет из точки (0 - если виджета нет)
	virtual Widget* FirstChildFromPoint(int x,int y);
	//	возвращает имя виджета
	virtual const char* Name(void) const;
	//	возвращает тип виджета
	virtual const char* Type(void) const {return "widget";};
	//	уведомление от дочернего окна об изменении состояния
	virtual void OnChange(Widget* pChild);
	//	уведомление об изменении режима работы системы
	virtual void OnSystemChange(void);
	//	уведомление о передачи управления данному виджету
	virtual void OnTick(void);
	//	отображение виджета и всех дочерних виджетов
	virtual void Render(WidgetView* pView);
	//	отображение виджета
	virtual void Draw(WidgetView* pView);
	//	установить видимость виджета
	virtual void SetVisible(bool bVisible);
	//	получить видимость виджета
	virtual bool IsVisible(void);
	//	установить картинку отображения
	virtual void SetImage(WidgetImage* pImage);
	//	получить картинку отображения
	virtual WidgetImage* GetImage(void) const;
	//	получить расположение виджета
	virtual Widget::Location* GetLocation(void) const;
	//	получить клиппер
	virtual Widget::Clipper* GetClipper(void) const;
	//	сместить виджет и все его дочерние виджеты на определенную величину
	virtual void Move(int dx,int dy);
	//	сместить виджет и все его дочерние виджеты в заданную точку
	virtual void MoveTo(int x,int y);
	//	установить доступ ввода 
	virtual void Enable(bool bEnable);
	//	получить доступность виджета
	virtual bool IsEnable(void);
	//	уведомление о наличии мыши на родительском виджете
	virtual bool OnMouse(VMouseState* pMS);
	//	уведомление о наличии клавиатурного фокуса на родительском виджете
	virtual bool OnKeyboard(VKeyboardState* pKS);
	//	обработка потери ввода
	virtual bool ProcessMouseLost(void);
	//	обработка потери ввода
	virtual bool ProcessKeyboardLost(void);
	//	модуль управления фокусом
	virtual WidgetFocus* Focus(void) const;
	//	инициализация модуля проигрывателя звуков
	virtual Player* SetPlayer(Player* player);
	//	работа с итераторм
	Widget::Iterator Widget::Begin(void);
	Widget::Iterator Widget::End(void);
protected:
	//	обработка мышинного ввода
	virtual bool ProcessMouseInput(VMouseState* pMS);
	//	обработка клавиатурного ввода
	virtual bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	уведомление о потере мышинного фокуса
	virtual void OnMouseLost(void);
	//	уведомление о потере клавиатурного фокуса
	virtual void OnKeyboardLost(void);
	//	производим какие-то действия
	virtual bool Tick(void);
	//	произошли изменения в расположении
	virtual void OnLocationChange(void);
};

//	оператор сравнения ссылки по имени
inline bool operator == (const WidgetRef& x,const char *pName)
{
	return !strcmp(x.m_pWidget->Name(),pName);
}

//	возвращает имя виджета
inline const char* Widget::Name(void) const
{
	return m_sName.c_str();
}

//	получить родительский виджет
inline Widget* Widget::Parent(void) const
{
	return m_pParent;
}

//	установить картинку отображения
inline void Widget::SetImage(WidgetImage* pImage)
{
	m_pImage = pImage;
}
//	получить расположение виджета
inline Widget::Location* Widget::GetLocation(void) const
{
	return m_pLocation;
}
//	получить клиппер
inline Widget::Clipper* Widget::GetClipper(void) const
{
	return m_Clipper;
}

inline WidgetFocus* Widget::Focus(void) const
{
	return m_pFocus;
}

inline WidgetImage* Widget::GetImage(void) const
{
	return m_pImage;
}

inline bool Widget::IsVisible(void)
{
	return m_bVisible;
}

inline bool Widget::IsEnable(void)
{
	return m_bEnable;
}

//*********************************************************************//
//********************* class Widget::Location **************************//
class Widget::Location
{
private:
//	class Clipper;
	//	обладатель расположения
	Widget *m_pOwner;
	friend Widget;
	//	регион ввода и отображения в локальных координатах родителя
	VRegion m_Region;
	//	точка начала отсчета в глобальных координатах
	VPoint m_Origin;
/*	//	модуль отсечения изображения
	Clipper* m_Clipper;*/
public:
	Location(Widget *pOwner);
	virtual ~Location();
public:
	void Create(VPoint *pPoints,unsigned int num);
	void Create(int left,int top,int right,int bottom);
	void Create(const VRegion& r);
public:
	const VRegion* Region(void){return &m_Region;}
	int OriginX(void){return m_Origin.x;}
	int OriginY(void){return m_Origin.y;}
/*public:
	//	включение и отключение клиппера
	void ActivateClipper(bool activate);
	//	получение смещение изображения
	int SubClipX(void);
	int SubClipY(void);*/
};

//*********************************************************************//
//************************* class VFocus ******************************//
class VFocus
{
protected:
	//	обладатель модуля
	Widget *m_pOwner;
	//	обладатель фокуса
	Widget *m_pUnderFocus;
	//	действителен или нет
	bool m_bEnable;
public:
	VFocus(Widget *pOwner){m_pOwner = pOwner;m_pUnderFocus = 0;m_bEnable = true;}
	virtual ~VFocus(){};
public:
	virtual bool Set(Widget *pChild) = 0;		//	установить фокус	
	virtual Widget *Get(void) = 0;				//	получить элемент с фокусом	
	virtual Widget *Next(void) = 0;				//	сместить фокус	
	virtual Widget *Prev(void) = 0;				//	сместить фокус
	virtual void Clear(Widget *pChild) = 0;		//	если на pChild есть фокус переместить его или убрать
	void Enable(bool bEnable){m_bEnable = bEnable;};
	bool IsEnable(void){return m_bEnable;};
};

//*********************************************************************//
//******************** class WidgetMouseFocus *************************//
class WidgetMouseFocus : public VFocus
{
public:
	WidgetMouseFocus(Widget *pOwner);
	virtual ~WidgetMouseFocus(){};
public:
	bool Set(Widget *pChild);	
	Widget *Get(void){return m_pUnderFocus;}				
	Widget *Next(void){return m_pUnderFocus;}
	Widget *Prev(void){return m_pUnderFocus;}
	void Clear(Widget *pChild);
};

//*********************************************************************//
//****************** class WidgetKeyboardFocus ************************//
class WidgetKeyboardFocus : public VFocus
{
public:
	WidgetKeyboardFocus(Widget *pOwner);
	virtual ~WidgetKeyboardFocus(){};
public:
	bool Set(Widget *pChild);	
	Widget *Get(void){return m_pUnderFocus;}				
	Widget *Next(void);
	Widget *Prev(void);
	void Clear(Widget *pChild);
};

//*********************************************************************//
//************************* class WidgetFocus **************************//
class WidgetFocus
{
public:
	enum FOCUS_FIXER
	{
		FF_MOUSE,
		FF_KEYBOARD,
		MAX_FIXER
	};
private:
	VFocus *m_pFFixers[MAX_FIXER];
public:
	WidgetFocus(Widget *pOwner);
	virtual ~WidgetFocus();
public:
	bool Set(FOCUS_FIXER ff,Widget *pChild);			//	установить фокус	
	Widget *Get(FOCUS_FIXER ff);						//	получить элемент с фокусом	
	Widget *Next(FOCUS_FIXER ff);						//	сместить фокус	
	Widget *Prev(FOCUS_FIXER ff);						//	сместить фокус
	void Clear(FOCUS_FIXER ff,Widget *pChild);			//	если на pChild есть фокус переместить его или убрать
	void Clear(Widget *pChild);							//	если на pChild есть фокус переместить его или убрать
	void Enable(FOCUS_FIXER ff,bool bEnable){m_pFFixers[ff]->Enable(bEnable);};
	bool IsEnable(FOCUS_FIXER ff){return m_pFFixers[ff]->IsEnable();};
};
//*********************************************************************//
//************************* class WidgetImage **************************//
class WidgetImage	//	абстракция от реализации битового образа
{
public:
	WidgetImage(){};
	virtual ~WidgetImage(){};
public:
	//	перегрузить картинку всвязи с изменениями в системе
	virtual void Reload(void) = 0;
	//	освободить ресурсы связанные с картинкой
	virtual void Release(void) = 0;
};

//*********************************************************************//
//************************* class WidgetView **************************//
class WidgetView	//	абстракция от реализации вывода виджета на экран
{
public:
	WidgetView(){};
	virtual ~WidgetView(){};
public:
	virtual void Render(Widget *pWidget) = 0;
};

//************************************************************************//
//****************          class Widget::Iterator       *****************//
class Widget::Iterator
{
private:
	std::list<WidgetRef>::iterator m_it;
public:
	Iterator(){};
	Iterator(std::list<WidgetRef>::iterator it){m_it = it;}
	virtual ~Iterator(){};
public:
	Iterator& operator++();
	Iterator operator++(int);
	Widget* operator->(void);
	Widget& operator*(void);
	bool operator!=(const Iterator& it);
};

inline Widget::Iterator& Widget::Iterator::operator++()
{
	++m_it;
	return *this;
}

inline Widget::Iterator Widget::Iterator::operator++(int)
{
	Iterator it = *this;
	++(*this);
	return it;
}

inline Widget* Widget::Iterator::operator->(void)
{
	return m_it->m_pWidget;
}

inline Widget& Widget::Iterator::operator*(void)
{
	return *operator->();
}

inline bool Widget::Iterator::operator!=(const Iterator& it)
{
	return m_it != it.m_it;
}

//************************************************************************//
//*****************	             class Widget          *******************//
inline Widget::Iterator Widget::Begin(void)
{
	return Iterator(m_lWidgets.begin());
}

inline Widget::Iterator Widget::End(void)
{
	return Iterator(m_lWidgets.end());
}

//************************************************************************//
//*****************	class Widget::Clipper    *******************//
class Widget::Clipper
{
private:
	//	обладатель модуля
	Widget *m_pOwner;
	//	прямоугольник отсечения
	VRegion m_Region;
	//	разница начал двух прямоугольников - (отсечения и полного)
	int m_SubX;
	int m_SubY;
	//	флаг активирования клиппера
	bool m_IsActive;
public:
	Clipper(Widget *pOwner);
	virtual ~Clipper();
public:
	//	обрезать полный регион отображения (создать текущую область отображения)
	void Clip(const VRegion& rgn,int left,int top,int right,int bottom);
	//	активировать/деактивировать 
	void Activate(bool activate);
public:
	const VRegion* Region(void){return &m_Region;}
	int SubX(void){return m_SubX;}
	int SubY(void){return m_SubY;}
	bool IsActive(void){return m_IsActive;}
};

//************************************************************************//
//	класс для проигрывания звуков
//	class Widget::Player
class Widget::Player
{
public:
	//************************************************************************//
	//	тип данных для обозначения звука
	//	class Widget::Player::Sample
	class Sample
	{
	public:
		virtual ~Sample() {}
		//	освободить ресурсы связанные с сэмплом
		virtual void Release(void) = 0;
	};
public:
	virtual ~Player() {}
	virtual void Play(Sample* sample) = 0;
	virtual void Stop(Sample* sample) = 0;
};

#endif	//_WIDGET_H_