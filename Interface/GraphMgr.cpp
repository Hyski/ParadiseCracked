/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   23.05.2001

************************************************************************/
#include "Precomp.h"
#include "../Common/GLayer/GLayer.h"
#include "DXImageMgr.h"
#include "Console.h"
#include "Text.h"
#include "GraphMgr.h"


//*********************************************************************************//
//	class GraphConsole
class GraphMgr::Console : public ::Console::Controller
{
	enum COMMAND {C_UNKNOWN,C_CREATE,C_CALIBRATE,C_ADD,C_CLEAR,C_AUTO,C_SHOW,C_HIDE,C_DESTROY};
	enum C_ERROR {E_WRONG_PARAMS,E_ABSENT_GRAPH,E_EXIST_GRAPH,E_CREATE_FAILED};
public:
	Console(){}
	virtual ~Console(){}
public:
	void Activate(void)
	{
		std::vector<std::string> argv;
		int argc = ::Console::GetCommandLine(argv);

		if(argc)
		{
			if(argv[0] == "graph")
			{
				if(argc>2)
				{
					const char* name = argv[1].c_str();
					switch(GetCommand(argv[2].c_str()))
					{
					case C_CREATE:
						if(argc>4)
						{
							if(!GraphMgr::Window(name))
							{
								if(!GraphMgr::BuildWindow(name,atoi(argv[3].c_str()),atoi(argv[4].c_str())))
								{
									Error(E_CREATE_FAILED);
								}
							}
							else
							{
								Error(E_EXIST_GRAPH);
							}
						}
						else
						{
							Error(E_WRONG_PARAMS);
						}
						break;
					case C_CALIBRATE:
						if(argc>5)
						{
							if(GraphMgr::Window(name))
							{
								GraphMgr::Window(name)->Calibrate(atof(argv[3].c_str()),atof(argv[4].c_str()),atof(argv[5].c_str()));
							}
							else
							{
								Error(E_ABSENT_GRAPH);
							}
						}
						else
						{
							Error(E_WRONG_PARAMS);
						}
						break;
					case C_ADD:
						if(argc>4)
						{
							if(GraphMgr::Window(name))
							{
								GraphMgr::Window(name)->AddValue(atof(argv[3].c_str()),atof(argv[4].c_str()));
							}
							else
							{
								Error(E_ABSENT_GRAPH);
							}
						}
						else
						{
							Error(E_WRONG_PARAMS);
						}
						break;
					case C_CLEAR:
						if(GraphMgr::Window(name)) GraphMgr::Window(name)->Clear();
						else Error(E_ABSENT_GRAPH);
						break;
					case C_AUTO:
						if(GraphMgr::Window(name)) GraphMgr::Window(name)->AutoScale();
						else Error(E_ABSENT_GRAPH);
						break;
					case C_SHOW:
						if(GraphMgr::Window(name)) GraphMgr::Window(name)->Show();
						else Error(E_ABSENT_GRAPH);
						break;
					case C_HIDE:
						if(GraphMgr::Window(name)) GraphMgr::Window(name)->Hide();
						else Error(E_ABSENT_GRAPH);
						break;
					case C_DESTROY:
						if(GraphMgr::Window(name)) GraphMgr::DestroyWindow(name);
						else Error(E_ABSENT_GRAPH);
						break;
					default:
						Error(E_WRONG_PARAMS);
						break;
					}
				}
				else
				{//	формат команд
					::Console::AddString("graph <name> <command> [param1] [param2] ...");
					::Console::AddString("commands:");
					::Console::AddString("  create <width> <height>");
					::Console::AddString("  calibrate <time> <min> <max>");
					::Console::AddString("  add <time> <value>");
					::Console::AddString("  clear");
					::Console::AddString("  auto");
					::Console::AddString("  show");
					::Console::AddString("  hide");
					::Console::AddString("  destroy");
				}
			}
		}
	}
	GraphMgr::Console::COMMAND GetCommand(const char* command)
	{
		if(!strcmp(command,"create")) return C_CREATE;
		if(!strcmp(command,"calibrate")) return C_CALIBRATE;
		if(!strcmp(command,"add")) return C_ADD;
		if(!strcmp(command,"clear")) return C_CLEAR;
		if(!strcmp(command,"auto")) return C_AUTO;
		if(!strcmp(command,"show")) return C_SHOW;
		if(!strcmp(command,"hide")) return C_HIDE;
		if(!strcmp(command,"destroy")) return C_DESTROY;
		return C_UNKNOWN;
	}
	void Error(C_ERROR error)
	{
		switch(error)
		{
		case E_WRONG_PARAMS:
			::Console::AddString("error: wrong params");
			break;
		case E_ABSENT_GRAPH:
			::Console::AddString("error: graph isn't exist");
			break;
		case E_EXIST_GRAPH:
			::Console::AddString("error: graph already exist");
			break;
		case E_CREATE_FAILED:
			::Console::AddString("error: graph isn't created");
			break;
			
		}
	}
};

//**********************************************************************//
//	class GraphWindow
HPEN GraphWindow::m_BasePen = CreatePen(PS_SOLID,1,0x117711);
HBRUSH GraphWindow::m_BaseBrush = CreateSolidBrush(0x117711);
GraphWindow::GraphWindow(const char* name,int width,int height) : Widget(name),m_MouseCapture(false),m_Layer(new GLayer())
{
	GetLocation()->Create(0,0,width,height);
	SetImage(DXImageMgr::Instance()->CreateImage(width,height));
	//	создаем технологическую часть
	HDC hdc;
	LPDIRECTDRAWSURFACE7 surface = static_cast<DXImage*>(GetImage())->m_pSurface;
	if(surface)
	{
		surface->GetDC(&hdc);
		m_Layer->Init(hdc,width,height);
		surface->ReleaseDC(hdc);
	}
	//	-----------------------------
	//	добавим название графика
	Text* text = new Text("title");
	text->SetFont("oskar",0x00dd00);
	text->SetRegion(0,0,width,20);
	text->SetText(name);
	Insert(text);
	//	добавим значения диапазона
	m_minText = new Text("min");
	m_minText->SetFont("oskar",0x00dd00);
	m_minText->SetRegion(0,height-20,width,height);
	m_minText->Align(LogicalText::T_BOTTOM|LogicalText::T_RIGHT);
	Insert(m_minText);
	m_maxText = new Text("max");
	m_maxText->SetFont("oskar",0x00dd00);
	m_maxText->SetRegion(0,0,width,20);
	m_maxText->Align(LogicalText::T_TOP|LogicalText::T_RIGHT);
	Insert(m_maxText);
	//	-----------------------------
	Calibrate(3,0,0.1f);
}

GraphWindow::~GraphWindow()
{
	delete m_Layer;
}

void GraphWindow::Calibrate(float time,float min_value,float max_value)
{
	static char buff[20];

	m_Time = time;
	m_MinValue = min_value;
	m_MaxValue = max_value;
	sprintf(buff,"%0.2f",m_MinValue);
	m_minText->SetText(buff);
	sprintf(buff,"%0.2f",m_MaxValue);
	m_maxText->SetText(buff);
	Build();
}

void GraphWindow::AddValue(float time,float value)
{
	m_Values[time] = value;
	Build();
}

void GraphWindow::Clear(void)
{
	m_Values.clear();
	Build();
}

void GraphWindow::AutoScale(void)
{
	if(m_Values.size())
	{
		m_MinValue = m_Values.rbegin()->second;
		m_MaxValue = m_MinValue;
		for(std::map<float,float>::iterator i = m_Values.begin();i!=m_Values.end();i++)
		{
			if(i->second<m_MinValue) m_MinValue = i->second;
			if(i->second>m_MaxValue) m_MaxValue = i->second;
		}
		Calibrate(m_Time,m_MinValue,m_MaxValue);
	}
}

void GraphWindow::Show(void)
{
	SetVisible(true);
	Parent()->BringToTop(Name());
}

void GraphWindow::Hide(void)
{
	SetVisible(false);
}
//	обработка мышинного ввода
bool GraphWindow::ProcessMouseInput(VMouseState* pMS)
{
	if(pMS->LButtonFront) 
	{
		m_MouseCapture = false;
		Hide();
	}
	if(pMS->RButtonFront) 
	{
		if(m_MouseCapture)
		{
			m_MouseCapture = false;
		}
		else
		{
			m_MouseCapture = true;
			m_xMouse = pMS->x-GetLocation()->OriginX()-GetLocation()->Region()->X();
			m_yMouse = pMS->y-GetLocation()->OriginY()-GetLocation()->Region()->Y();
			Parent()->BringToTop(Name());
		}
	}

	return true;

}
//	производим какие-то действия
bool GraphWindow::Tick(void)
{
	if(m_MouseCapture)
	{
		MoveTo(Input::MouseState().x-m_xMouse-GetLocation()->OriginX(),
			   Input::MouseState().y-m_yMouse-GetLocation()->OriginY());
	}

	return true;
}
//	составляем график
void GraphWindow::Build(void)
{
	const int width = GetLocation()->Region()->Width();
	const int height = GetLocation()->Region()->Height();
	std::map<float,float>::reverse_iterator ri;

	SelectObject(m_Layer->m_hDC,GetStockObject(BLACK_BRUSH));
	SelectObject(m_Layer->m_hDC,GetStockObject(BLACK_PEN));
	Rectangle(m_Layer->m_hDC,0,0,width,height);
	if(m_Values.size())
	{
		const float max_time = m_Values.rbegin()->first;
		const float start = std::max(0.0f,max_time-m_Time);
		const float k_width = max_time-start;
		const float k_height = m_MaxValue-m_MinValue;

		SelectObject(m_Layer->m_hDC,m_BaseBrush);
		SelectObject(m_Layer->m_hDC,m_BasePen);
		if((k_width>0) && (k_height>0))
		{
			static POINT points[4];

			ri = m_Values.rbegin();

			int x = (ri->first-start)*width/k_width;
			int y = height-((ri->second-m_MinValue)*height/k_height);

			for(ri++;ri!=m_Values.rend();ri++)
			{
				points[0].x = x;
				points[0].y = y;

				points[1].x = (ri->first-start)*width/k_width; 
				points[1].y = height-((ri->second-m_MinValue)*height/k_height);

				points[2].x = points[1].x;
				points[2].y = height;

				points[3].x = points[0].x;
				points[3].y = height;

				Polygon(m_Layer->m_hDC,points,4);

				x = points[1].x;
				y = points[1].y;

				if(ri->first < start) break;
			}
/*			//	обыкновенный вывод линий
			MoveToEx(m_Layer->m_hDC,width,height,0);
			for(ri=m_Values.rbegin();ri!=m_Values.rend();ri++)
			{
				int x = (ri->first-start)*width/k_width;
				int y = height-((ri->second-m_MinValue)*height/k_height);

				LineTo(m_Layer->m_hDC,x,y);

				if((y<0) || (y>height))
				{
					SetPixel(m_Layer->m_hDC,x,std::max(0,std::min(y,height)),0xffffff);
				}

				if(ri->first < start) break;
			}
*/
			m_Values.erase(m_Values.rend().base(),ri.base());
		}
		SelectObject(m_Layer->m_hDC,GetStockObject(BLACK_BRUSH));
		SelectObject(m_Layer->m_hDC,GetStockObject(WHITE_PEN));
	}
	//	блиттим
	HDC hdc;
	LPDIRECTDRAWSURFACE7 surface = static_cast<DXImage*>(GetImage())->m_pSurface;
	if(surface)
	{
		surface->GetDC(&hdc);
		m_Layer->Flip(hdc,0,0,width,height,0,0);
		surface->ReleaseDC(hdc);
	}
}
//	уведомление об изменении режима работы системы
void GraphWindow::OnSystemChange(void)
{
	Widget::OnSystemChange();
	DXImageMgr::Instance()->Release(static_cast<DXImage*>(GetImage()));
	SetImage(DXImageMgr::Instance()->CreateImage(GetLocation()->Region()->Width(),GetLocation()->Region()->Height()));
	//	создаем технологическую часть
	HDC hdc;
	LPDIRECTDRAWSURFACE7 surface = static_cast<DXImage*>(GetImage())->m_pSurface;
	if(surface)
	{
		surface->GetDC(&hdc);
		m_Layer->Init(hdc,GetLocation()->Region()->Width(),GetLocation()->Region()->Height());
		surface->ReleaseDC(hdc);
	}
	Build();
}

//**********************************************************************//
//	class GraphMgr
GraphMgr* GraphMgr::m_Instance = 0;
bool GraphMgr::m_IsAvailable = false;
Widget* GraphMgr::m_Owner = 0;
GraphMgr::GraphMgr() : m_Console(new GraphMgr::Console)
{
	GraphWindow::m_BasePen = CreatePen(PS_SOLID,1,0x117711);
	GraphWindow::m_BaseBrush = CreateSolidBrush(0x117711);

	::Console::AddController(m_Console);
}

GraphMgr::~GraphMgr()
{
	DeleteObject(GraphWindow::m_BasePen);
	DeleteObject(GraphWindow::m_BaseBrush);

	delete m_Console;
}
//	создать менеджер
GraphMgr* GraphMgr::SetAvailable(Widget* pOwner)
{
	m_IsAvailable = true;
	if(!m_Instance && pOwner)
	{
		m_Instance = new GraphMgr();
		m_Owner = pOwner;
	}

	return m_Instance;
}
//	уничтожить менеджер
void GraphMgr::Release(void)
{
	m_IsAvailable = false;
	if(m_Instance)
	{
		delete m_Instance;
		m_Instance = 0;
	}
}

//	создать окно для вывод графика
GraphWindow* GraphMgr::BuildWindow(const char* name,int width,int height)
{
	GraphWindow* gw = 0;

	if(m_Instance)
	{
		gw = new GraphWindow(name,width,height);
		if(!m_Owner->Insert(gw))
		{
			delete gw;
			gw = 0;
		}
	}

	return gw;
}
//	получить окно по имени
GraphWindow* GraphMgr::Window(const char* name)
{
	if(m_Instance)
	{
		if(m_Owner->Child(name) && !strcmp(m_Owner->Child(name)->Type(),"graphwindow"))
		{
			return static_cast<GraphWindow*>(m_Owner->Child(name));
		}
	}

	return 0;
}
//	уничтожить окно
void GraphMgr::DestroyWindow(const char* name)
{
	if(m_Instance)
	{
		m_Owner->Delete(name);
	}

}


