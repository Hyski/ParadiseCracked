/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   26.02.2001

************************************************************************/
#include "Precomp.h"
#include "Widget.h"

Widget::Widget(const char *pName) : m_sName(pName),m_pParent(0),m_pImage(0),m_Player(0),m_bVisible(true),m_bEnable(true)
{
	//	создаем месторасположение
	m_pLocation = new Location(this);
	//	создаем модуль отсечения
	m_Clipper = new Clipper(this);
	//	создаем модуль управления фокусом ввода
	m_pFocus = new WidgetFocus(this);
}

Widget::~Widget()
{
	//	уведомляем картинку об удалении ее содержателя
	//	освобождаем ресурсы
	if(m_pImage) m_pImage->Release();
	//	в деструкторе уничтожаются все дочерние виджеты
	for(std::list<WidgetRef>::iterator iW = m_lWidgets.begin();iW != m_lWidgets.end();iW++)
	{
		delete iW->m_pWidget;
	}
	//	освобождаем ресурсы связанные с фокусом ввода
	delete m_pFocus;
	//	освобождаем ресурсы связанные с клиппером
	delete m_Clipper;
	//	освобождаем ресурсы связанные с данным виджетом
	delete m_pLocation;
}
//	поместить в данный виджет экземпляр другого виджета
//	если виджет с таким же именем уже существует возвращается 0
Widget* Widget::Insert(Widget *pWidget)
{	
	std::list<WidgetRef>::iterator iW;

	if(pWidget)
	{
		iW = std::find(m_lWidgets.begin(),m_lWidgets.end(),pWidget->Name());
		//	В виджете имеется список дочерних виджетов.
		//	Виджеты идентифицируются по строковым именам
		//	Виджеты располагаются в списке в определенном порядке,
		//	который задает порядок доступа к виджетам.
		if(iW == m_lWidgets.end())
		{
			pWidget->m_pParent = this;
			if(m_pLocation->Region()->IsValid())
			{
				pWidget->m_pLocation->m_Origin.x = m_pLocation->OriginX()+m_pLocation->Region()->X();
				pWidget->m_pLocation->m_Origin.y = m_pLocation->OriginY()+m_pLocation->Region()->Y();
				pWidget->OnLocationChange();
			}
			m_lWidgets.push_back(WidgetRef(pWidget));
			return pWidget;
		}
	}

	return 0;
}
//	уничтожить экземпляр другого виджета по имени, при неудаче
//	(если данного виджета не имеется) возвращается false
bool Widget::Delete(const char *pName)
{
	std::list<WidgetRef>::iterator iW = std::find(m_lWidgets.begin(),m_lWidgets.end(),pName);

	if(iW != m_lWidgets.end())
	{
		//	проверяем фокус
		m_pFocus->Clear(iW->m_pWidget);
		//	удаляем объект
		delete iW->m_pWidget;
		m_lWidgets.erase(iW);

		return true;
	}

	return false;
}
//	отделить виджет с указанным именем от данного родительского виджета
Widget* Widget::Remove(const char *pName)
{
	std::list<WidgetRef>::iterator iW = std::find(m_lWidgets.begin(),m_lWidgets.end(),pName);
	Widget* rWidget = 0;

	if(iW != m_lWidgets.end())
	{
		//	проверяем фокус
		m_pFocus->Clear(iW->m_pWidget);
		//	удаляем объект из списка
		rWidget = iW->m_pWidget;
		m_lWidgets.erase(iW);
	}

	return rWidget;
}
//	получить дочерний виджет по имени
Widget* Widget::Child(const char *pName) 
{
	std::list<WidgetRef>::iterator iW = std::find(m_lWidgets.begin(),m_lWidgets.end(),pName);

	if(iW != m_lWidgets.end())
		return iW->m_pWidget;

	return 0;
}
//	получить дочерний виджет из точки (0 - если виджета нет)
Widget* Widget::FromPoint(int x,int y)
{
	Widget* pChild;

	if(m_pLocation->Region()->PtInRegion(x-m_pLocation->m_Origin.x,y-m_pLocation->m_Origin.y))
	{
		for(std::list<WidgetRef>::reverse_iterator iW = m_lWidgets.rbegin();iW != m_lWidgets.rend();iW++)
		{
			if(iW->m_pWidget->m_bVisible && iW->m_pWidget->m_pLocation->Region()->PtInRegion(x-iW->m_pWidget->m_pLocation->m_Origin.x,y-iW->m_pWidget->m_pLocation->m_Origin.y))
			{
				if(pChild = iW->m_pWidget->FromPoint(x,y))
					return pChild;
				else
					return iW->m_pWidget;
			}
		}
	}
	
	return 0;
}
//	получить первый по глубине дочерний виджет из точки (0 - если виджета нет)
Widget* Widget::FirstChildFromPoint(int x,int y)
{
	Widget* pChild = 0;

	if(m_pLocation->Region()->PtInRegion(x-m_pLocation->m_Origin.x,y-m_pLocation->m_Origin.y))
	{
		for(std::list<WidgetRef>::iterator iW = m_lWidgets.begin();iW != m_lWidgets.end();iW++)
		{
			if(iW->m_pWidget->m_bVisible && iW->m_pWidget->m_pLocation->Region()->PtInRegion(x-iW->m_pWidget->m_pLocation->m_Origin.x,y-iW->m_pWidget->m_pLocation->m_Origin.y))
			{
				pChild = iW->m_pWidget;
			}
		}
	}
	
	return pChild;
}
//	поместить дочерний виджет (по имени) сверху
bool Widget::BringToTop(const char *pName)
{
	std::list<WidgetRef>::iterator iW = std::find(m_lWidgets.begin(),m_lWidgets.end(),pName);

	if(iW != m_lWidgets.end())
	{
		m_lWidgets.push_back(WidgetRef(iW->m_pWidget));
		m_lWidgets.erase(iW);
		return true;
	}

	return false;
}
//	поместить дочерний виджет (по имени) снизу
bool Widget::BringToBottom(const char *pName)
{
	std::list<WidgetRef>::iterator iW = std::find(m_lWidgets.begin(),m_lWidgets.end(),pName);

	if(iW != m_lWidgets.end())
	{
		m_lWidgets.push_front(WidgetRef(iW->m_pWidget));
		m_lWidgets.erase(iW);
		return true;
	}

	return false;
}
//	отображение виджета и всех дочерних виджетов
void Widget::Render(WidgetView* pView)
{
	if(m_bVisible)
	{
		Draw(pView);						//	отрисовываем данный виджет
		for(std::list<WidgetRef>::iterator iW = m_lWidgets.begin();iW != m_lWidgets.end();iW++)
			iW->m_pWidget->Render(pView);	//	отрисовываем все дочерние виджеты
	}
}
//	отображение виджета
void Widget::Draw(WidgetView* pView)
{
	pView->Render(this);				
}

//	уведомление от дочернего окна об изменении состояния
void Widget::OnChange(Widget* /*pChild*/)
{
}
//	уведомление об изменении режима работы системы
void Widget::OnSystemChange(void)
{
	if(m_pImage) m_pImage->Reload();
	for(std::list<WidgetRef>::iterator iW = m_lWidgets.begin();iW != m_lWidgets.end();iW++)
		iW->m_pWidget->OnSystemChange();	//	всем дочерние виджеты
}
//	произошли изменения в расположении
void Widget::OnLocationChange(void)
{
	static int left,top,right,bottom;

	//	клиппер
	if(Parent())
	{
		if(m_Clipper->IsActive())
		{
			left	= Parent()->GetClipper()->SubX();
			top		= Parent()->GetClipper()->SubY();
			right	= left + Parent()->GetClipper()->Region()->Width();
			bottom	= top + Parent()->GetClipper()->Region()->Height();
			m_Clipper->Clip(m_pLocation->m_Region,left,top,right,bottom);
		}
		else
		{
			m_pLocation->m_Region.GetRect(&left,&top,&right,&bottom);
			m_Clipper->Clip(m_pLocation->m_Region,left,top,right,bottom);
		}
	}
	else
		m_Clipper->Clip(m_pLocation->m_Region,0,0,m_pLocation->m_Region.Width(),m_pLocation->m_Region.Height());
	//	дочернии виджеты
	for(std::list<WidgetRef>::iterator iW = m_lWidgets.begin();iW != m_lWidgets.end();iW++)
	{
		iW->m_pWidget->m_pLocation->m_Origin.x = m_pLocation->OriginX()+m_pLocation->Region()->X();
		iW->m_pWidget->m_pLocation->m_Origin.y = m_pLocation->OriginY()+m_pLocation->Region()->Y();
		iW->m_pWidget->OnLocationChange();
	}
}
//	уведомление о передачи управления данному виджету
void Widget::OnTick(void)
{
	if(Tick())
	{
		for(std::list<WidgetRef>::iterator iW = m_lWidgets.begin();iW != m_lWidgets.end();iW++)
			iW->m_pWidget->OnTick();	//	всем дочерним виджетам
	}
}
//	производим какие-то действия
bool Widget::Tick(void)
{
	return true;
}
//	двигать виджет и все его дочерние виджеты 
void Widget::Move(int dx,int dy)
{
	m_pLocation->m_Region.Transform(1,0,dx,0,1,dy);
	OnLocationChange();
}
//	сместить виджет и все его дочерние виджеты в заданную точку
void Widget::MoveTo(int x,int y)
{
	Move(x-m_pLocation->Region()->X(),y-m_pLocation->Region()->Y());
}
//	уведомление о наличии мыши на родительском виджете
//	true - продолжить низхождение
bool Widget::OnMouse(VMouseState* pMS)
{
	if(ProcessMouseInput(pMS))
	{
		for(std::list<WidgetRef>::reverse_iterator iW = m_lWidgets.rbegin();iW != m_lWidgets.rend();iW++)
		{
			Location* l = iW->m_pWidget->GetLocation();
			const VRegion* region = l->Region();
			bool in_region = region->PtInRegion(pMS->x-l->m_Origin.x,pMS->y-l->m_Origin.y);
			if(iW->m_pWidget->m_bVisible && iW->m_pWidget->m_bEnable && in_region)
			{//	нашли виджет на котором присутствует мышь
				if(m_pFocus->Get(WidgetFocus::FF_MOUSE) != iW->m_pWidget)
				{
					m_pFocus->Set(WidgetFocus::FF_MOUSE,iW->m_pWidget);
				}
				if(m_pFocus->Get(WidgetFocus::FF_MOUSE))
					m_pFocus->Get(WidgetFocus::FF_MOUSE)->OnMouse(pMS);
				
				return false;
			}
		}
		if(m_pFocus->Get(WidgetFocus::FF_MOUSE))
			m_pFocus->Get(WidgetFocus::FF_MOUSE)->OnMouseLost();
		m_pFocus->Clear(WidgetFocus::FF_MOUSE,m_pFocus->Get(WidgetFocus::FF_MOUSE));
	}

	return true;
}

//	обработка мышинного ввода
bool Widget::ProcessMouseInput(VMouseState* /*pMS*/)
{
	return true;
}

//	уведомление о наличии клавиатурного фокуса на родительском виджете
bool Widget::OnKeyboard(VKeyboardState* pKS)
{
	if(m_bVisible && m_bEnable)
	{
		if(ProcessKeyboardInput(pKS))
		{
			if(m_pFocus->Get(WidgetFocus::FF_KEYBOARD))
				m_pFocus->Get(WidgetFocus::FF_KEYBOARD)->OnKeyboard(pKS);
		}

		return false;
	}

	return true;
}
//	обработка клавиатурного ввода
bool Widget::ProcessKeyboardInput(VKeyboardState* /*pKS*/)
{
	return true;
}
//	уведомление о потере мышинного фокуса
void Widget::OnMouseLost(void)
{
	if(ProcessMouseLost())
	{
		if(m_pFocus->Get(WidgetFocus::FF_MOUSE))
			m_pFocus->Get(WidgetFocus::FF_MOUSE)->OnMouseLost();
	}
}

bool Widget::ProcessMouseLost(void)
{
	return true;
}
//	уведомление о потере клавиатурного фокуса
void Widget::OnKeyboardLost(void)
{
	if(ProcessKeyboardLost())
	{
		if(m_pFocus->Get(WidgetFocus::FF_KEYBOARD))
			m_pFocus->Get(WidgetFocus::FF_KEYBOARD)->OnKeyboardLost();
	}
}

bool Widget::ProcessKeyboardLost(void)
{
	return true;
}
//	установить видимость виджета
void Widget::SetVisible(bool bVisible)
{
	m_bVisible = bVisible;
	if(!m_bVisible && m_pParent) m_pParent->Focus()->Clear(this);
}

//	установить доступ ввода
void Widget::Enable(bool bEnable)
{
	m_bEnable = bEnable;
	if(!m_bEnable && m_pParent) m_pParent->Focus()->Clear(this);
}

//	инициализация модуля проигрывателя звуков
Widget::Player* Widget::SetPlayer(Player* player)
{
	Player* old_player = m_Player;
	m_Player = player;

	return old_player;
}

//*********************************************************************//
//********************* class Widget::Location **************************//
Widget::Location::Location(Widget *pOwner) : m_pOwner(pOwner)
{
}

Widget::Location::~Location()
{
}

void Widget::Location::Create(VPoint *pPoints,unsigned int num)
{
	m_Region.Create(pPoints,num);
	m_pOwner->OnLocationChange();
}

void Widget::Location::Create(int left,int top,int right,int bottom)
{
	m_Region.Create(left,top,right,bottom);
	m_pOwner->OnLocationChange();
}

void Widget::Location::Create(const VRegion& r)
{
	m_Region = r;
	m_pOwner->OnLocationChange();
}

//*********************************************************************//
//******************** class WidgetMouseFocus *************************//
WidgetMouseFocus::WidgetMouseFocus(Widget *pOwner) : VFocus(pOwner)
{
}

bool WidgetMouseFocus::Set(Widget *pChild)
{
	if(pChild && pChild->m_bEnable && pChild->m_bVisible && pChild->Focus()->IsEnable(WidgetFocus::FF_MOUSE))
	{
		if(m_pUnderFocus) m_pUnderFocus->OnMouseLost();
		m_pUnderFocus = pChild;
		return true;
	}

	return false;
}

//	если на pChild есть фокус переместить его или убрать
void WidgetMouseFocus::Clear(Widget * /*pChild*/)
{
	m_pUnderFocus = 0;
}
//*********************************************************************//
//****************** class WidgetKeyboardFocus ************************//
WidgetKeyboardFocus::WidgetKeyboardFocus(Widget *pOwner) : VFocus(pOwner)
{
}

bool WidgetKeyboardFocus::Set(Widget *pChild)
{
	if(pChild && pChild->m_bEnable && pChild->m_bVisible && pChild->Focus()->IsEnable(WidgetFocus::FF_KEYBOARD))
	{
		if(m_pUnderFocus) m_pUnderFocus->OnKeyboardLost();
		m_pUnderFocus = pChild;
		return true;
	}

	return false;
}

Widget* WidgetKeyboardFocus::Next(void)
{
	std::list<WidgetRef>::reverse_iterator iW;
	int size = m_pOwner->m_lWidgets.size();

	if(m_pUnderFocus)
	{
		iW = std::find(m_pOwner->m_lWidgets.rbegin(),m_pOwner->m_lWidgets.rend(),m_pUnderFocus->Name());
		iW++;	//	смещаем итератор
		if(iW == m_pOwner->m_lWidgets.rend()) iW = m_pOwner->m_lWidgets.rbegin();
	}
	else
	{
		iW = m_pOwner->m_lWidgets.rbegin();
	}
	if(iW != m_pOwner->m_lWidgets.rend())
	{
		for(int i=0;i<size;i++)
		{
			if(iW->m_pWidget->m_bEnable && iW->m_pWidget->m_bVisible && iW->m_pWidget->Focus()->IsEnable(WidgetFocus::FF_KEYBOARD))
			{
				if(m_pUnderFocus && (m_pUnderFocus != iW->m_pWidget))
					m_pUnderFocus->OnKeyboardLost();
				m_pUnderFocus = iW->m_pWidget;
				break;
			}
			iW++;//	смещаем итератор
			if(iW == m_pOwner->m_lWidgets.rend()) iW = m_pOwner->m_lWidgets.rbegin();
		}
	}

	return m_pUnderFocus;
}

Widget* WidgetKeyboardFocus::Prev(void)
{
	std::list<WidgetRef>::iterator iW;
	int size = m_pOwner->m_lWidgets.size();

	if(m_pUnderFocus)
	{
		iW = std::find(m_pOwner->m_lWidgets.begin(),m_pOwner->m_lWidgets.end(),m_pUnderFocus->Name());
		iW++;	//	смещаем итератор
		if(iW == m_pOwner->m_lWidgets.end()) iW = m_pOwner->m_lWidgets.begin();
	}
	else
	{
		iW = m_pOwner->m_lWidgets.begin();
	}
	if(iW != m_pOwner->m_lWidgets.end())
	{
		for(int i=0;i<size;i++)
		{
			if(iW->m_pWidget->m_bEnable && iW->m_pWidget->m_bVisible && iW->m_pWidget->Focus()->IsEnable(WidgetFocus::FF_KEYBOARD))
			{
				if(m_pUnderFocus && (m_pUnderFocus != iW->m_pWidget))
					m_pUnderFocus->OnKeyboardLost();
				m_pUnderFocus = iW->m_pWidget;
				break;
			}
			iW++;//	смещаем итератор
			if(iW == m_pOwner->m_lWidgets.end()) iW = m_pOwner->m_lWidgets.begin();
		}
	}

	return m_pUnderFocus;
}

void WidgetKeyboardFocus::Clear(Widget *pChild)
{
	if(pChild)
	{
		if(m_pUnderFocus == pChild)
		{
			m_pUnderFocus = 0;
		}
	}
}
//*********************************************************************//
//************************* class WidgetFocus **************************//
WidgetFocus::WidgetFocus(Widget *pOwner) 
{
	m_pFFixers[FF_MOUSE] = new WidgetMouseFocus(pOwner);
	m_pFFixers[FF_KEYBOARD] = new WidgetKeyboardFocus(pOwner);
}

WidgetFocus::~WidgetFocus()
{
	delete m_pFFixers[FF_MOUSE];
	delete m_pFFixers[FF_KEYBOARD];
}

//	установить фокус
bool WidgetFocus::Set(FOCUS_FIXER ff,Widget *pChild)
{
	return m_pFFixers[ff]->Set(pChild);
}
//	получить элемент с фокусом	
Widget* WidgetFocus::Get(FOCUS_FIXER ff)
{
	return m_pFFixers[ff]->Get();
}
//	сместить фокус
Widget* WidgetFocus::Next(FOCUS_FIXER ff)
{
	return m_pFFixers[ff]->Next();
}
//	сместить фокус
Widget* WidgetFocus::Prev(FOCUS_FIXER ff)
{
	return m_pFFixers[ff]->Prev();
}
//	если на pChild есть фокус переместить его или убрать
void WidgetFocus::Clear(FOCUS_FIXER ff,Widget *pChild)
{
	m_pFFixers[ff]->Clear(pChild);
}
//	если на pChild есть фокус переместить его или убрать
void WidgetFocus::Clear(Widget *pChild)
{
	for(int ff=0;ff<MAX_FIXER;ff++)
		m_pFFixers[ff]->Clear(pChild);
}

//************************************************************************//
//*****************	       class Widget::Clipper       *******************//
Widget::Clipper::Clipper(Widget *pOwner) : m_pOwner(pOwner),m_IsActive(true)
{
	m_SubX = m_SubY = 0;
}

Widget::Clipper::~Clipper()
{
}
//	активировать/деактивировать 
void Widget::Clipper::Activate(bool activate)
{
	m_IsActive = activate;
	m_pOwner->OnLocationChange();
}
//	обрезать полный регион отображения (создать текущую область отображения)
void Widget::Clipper::Clip(const VRegion& rgn,int left,int top,int right,int bottom)
{
	m_Region.Create(std::max(rgn.X(),left),
					std::max(rgn.Y(),top),
					std::min(rgn.X()+rgn.Width(),right),
					std::min(rgn.Y()+rgn.Height(),bottom));

	m_SubX		= m_Region.X()-rgn.X();
	m_SubY		= m_Region.Y()-rgn.Y();
}


