/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   20.03.2001

************************************************************************/
#include "Precomp.h"
#include "LoadingScreen.h"

#include "../Common/DataMgr/DataMgr.h"
#include "../Common/Shell/Shell.h"
#include "../Common/UI/Static.h"
#include "WidgetFactory.h"
#include "MouseCursor.h"
#include "DXImageMgr.h"
#include "Interface.h"
#include "Text.h"
#include <ctime>

//************************************************************************************//
//	class LoadingScreen::ImageOscillator
class LoadingScreen::ImageOscillator
{
private:
	static const char* m_LoadingPath;
	static const char* m_ImageNameTemplate;
	static const char* m_ProgressNameTemplate;
private:
	std::string m_ImageName;
	std::string m_ProgresName;
public:
	ImageOscillator()
	{
		std::vector<DataMgr::FileInfo> files;
		std::vector<std::string> image;
		std::vector<std::string> progress;

		DataMgr::Scan(m_LoadingPath,files);

		for(int i=(files.size()-1);i!=0;i--)
		{
			if(strstr(files[i].m_Name.c_str(),m_ImageNameTemplate))
			{
				image.push_back(m_LoadingPath+files[i].m_FullName);
				continue;
			}
			if(strstr(files[i].m_Name.c_str(),m_ProgressNameTemplate))
			{
				progress.push_back(m_LoadingPath+files[i].m_FullName);
			}
		}
		//	----------------------------------------------------------
		srand((unsigned int)time(0));
		if(image.size()) m_ImageName = image[(image.size()==1)?0:(rand()%(image.size()-1))];
		if(progress.size()) m_ProgresName = progress[(progress.size()==1)?0:(rand()%(progress.size()-1))];
	}
	~ImageOscillator(){}
public:
	const std::string& GetImageName(void) {return m_ImageName;}
	const std::string& GetProgressName(void)  {return m_ProgresName;}
};

const char* LoadingScreen::ImageOscillator::m_LoadingPath = "pictures/interface/Loading/";
const char* LoadingScreen::ImageOscillator::m_ImageNameTemplate = "loading";
const char* LoadingScreen::ImageOscillator::m_ProgressNameTemplate = "progress";

//************************************************************************************//
//	class LoadingScreen
const char* LoadingScreen::m_pDialogName = "LoadingScreen";
LoadingScreen::LoadingScreen() : Dialog(m_pDialogName,m_pDialogName),m_Step(S_PREPARE)
{
	ImageOscillator io; 

	Widget* pImage = WidgetFactory::Instance()->Assert(this,"background");
	if(pImage->GetImage()) pImage->GetImage()->Release();
	pImage->SetImage(DXImageMgr::Instance()->CreateImage(io.GetImageName().c_str()));
	pImage->Insert(m_Progress = new Widget("progress"));
	m_Progress->GetLocation()->Create(235,440,615,457);	//	235,439,615,456
	m_Progress->Insert(new Widget("image"));
	m_Progress->Child("image")->GetLocation()->Create(0,0,m_Progress->GetLocation()->Region()->Width(),m_Progress->GetLocation()->Region()->Height());
	m_Progress->Child("image")->SetImage(DXImageMgr::Instance()->CreateImage(io.GetProgressName().c_str()));
	m_ProgressWidth = m_Progress->GetLocation()->Region()->Width();
	//	создаем текстовое поле для заголовка
	pImage->Insert(m_Title = new Text("title"));
	m_Title->Align(LogicalText::T_HCENTER|LogicalText::T_VCENTER);
	m_Title->SetFont("mila_s",0xffdd77);
	m_Title->SetText("");
	int left,top,right,bottom;
	m_Progress->GetLocation()->Region()->GetRect(&left,&top,&right,&bottom);
	m_Title->SetRegion(left,top,right,bottom);
}

LoadingScreen::~LoadingScreen()
{
	//	показываем курсор
	MouseCursor::SetVisible(true);
}

bool LoadingScreen::Tick(void)
{
	switch(m_Step)
	{
	case S_PREPARE:
		//	проверяем текущее разрешение
		CheckConformMode();
		//	устанавливаем картинку на экран со значением прогресс бара в нуле
		Update(0);
		//	следующий шаг
		m_Step = S_READY_FOR_LOADING;
		break;
	case S_READY_FOR_LOADING:
		//	уведомляем родителя о готовности грузить данные
		Parent()->OnChange(this);
		m_Step = S_LOADING_DONE;
		break;
	case S_LOADING_DONE:
		//	показываем курсор
		MouseCursor::SetVisible(true);
		//	уведомляем
		Parent()->OnChange(this);
		m_Step = S_QUITE;
		break;
	default:
		return true;
	}

	return false;
}
//	отображение виджета
void LoadingScreen::Draw(WidgetView* pView)
{
	D3DKernel::GetD3DDevice()->Clear(0,NULL,D3DCLEAR_TARGET,0x00000000,1.0f,0);
	Dialog::Draw(pView);
}
//	обновление прогресс бара
void LoadingScreen::Update(float value)
{
	static int left,top,right,bottom;

	D3DKernel::GetD3DDevice()->Clear(0,NULL,D3DCLEAR_TARGET,0x00000000,1.0f,0);
	m_Progress->GetLocation()->Region()->GetRect(&left,&top,&right,&bottom);
	right = left+(m_ProgressWidth*value);
	m_Progress->GetLocation()->Create(left,top,right,bottom);
	//	прячем курсор
	MouseCursor::SetVisible(false);
	//	отрисовываем интерфейс
	Interface::Instance()->Render2D();
	DebugInfo::Render();
	//	обновление экрана
	D3DKernel::UpdateFrame();
	//	обработка сообщений от окна
	Frame()->Tick();
}
//	установить заголовок прогресс бара
void LoadingScreen::SetTitle(const char* title)
{
	//	устанавливаем заголовок прогресс бара
	m_Title->SetText(title);
	//	обновляем экран
	D3DKernel::GetD3DDevice()->Clear(0,NULL,D3DCLEAR_TARGET,0x00000000,1.0f,0);
	//	прячем курсор
	MouseCursor::SetVisible(false);
	//	отрисовываем интерфейс
	Interface::Instance()->Render2D();
	//	обновление экрана
	D3DKernel::UpdateFrame();
}
//**********************************************************************//
//*********	          class HiddenMovementScreen           *************//
const char* HiddenMovementScreen::m_pDialogName = "HiddenMovementScreen";
bool HiddenMovementScreen::m_DebugMode = false;
HiddenMovementScreen::HiddenMovementScreen() : Dialog(m_pDialogName,m_pDialogName),m_ipb(0)
{
	m_ProgressImage = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"progress"));
	Remove(m_ProgressImage->Name());
	//	получаем указатель на задний фон
	m_Background = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"background"));
	//	получаем указатель на прогресс
	m_Background->Insert(m_Progress = new Widget("progress"));
	m_Progress->GetLocation()->Create(*m_ProgressImage->GetLocation()->Region());
	m_Progress->Insert(m_ProgressImage);
	m_ProgressImage->GetLocation()->Create(0,0,m_Progress->GetLocation()->Region()->Width(),m_Progress->GetLocation()->Region()->Height());
	m_ProgressWidth = m_Progress->GetLocation()->Region()->Width();
	//	устанавливаем кол-во картинок на 
	m_ipb = m_Background->GetStateSize()*0.5f;
}

HiddenMovementScreen::~HiddenMovementScreen()
{
}
//	уведомление об изменении состояния системы
void HiddenMovementScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	сменить картинку
void HiddenMovementScreen::SetBackground(BELONG belong)
{
	switch(belong)
	{
	case B_CIVILIAN:
		m_Background->SetState(rand()%m_ipb);
		break;
	case B_ENEMY:
		m_Background->SetState(m_ipb+rand()%m_ipb);
		break;
	}
}
// обновить прогресс бар
void HiddenMovementScreen::Update(float value)
{
	static int left,top,right,bottom;

	m_Progress->GetLocation()->Region()->GetRect(&left,&top,&right,&bottom);
	right = left+(m_ProgressWidth*value);
	m_Progress->GetLocation()->Create(left,top,right,bottom);
}
//	отображение виджета и всех дочерних виджетов
void HiddenMovementScreen::Render(WidgetView* pView)
{
	if(IsVisible())
	{
		if(!m_DebugMode)
		{
			D3DKernel::GetD3DDevice()->Clear(0,NULL,D3DCLEAR_TARGET,0x00000000,1.0f,0);
			Dialog::Render(pView);
		}
		else
		{
			m_Progress->Render(pView);
		}
	}
}

//**********************************************************************//
//	class PanicScreen
const char* PanicScreen::m_pDialogName = "PanicScreen";
PanicScreen::PanicScreen() : Dialog(m_pDialogName,m_pDialogName)
{
	m_Content = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"content"));
}

PanicScreen::~PanicScreen()
{
}
//	установить текст описания
void PanicScreen::SetContent(const char* text)
{
	m_Content->SetText(text);
}
//	уведомление об изменении состояния системы
void PanicScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}

//**********************************************************************//
//	class ScriptSceneScreen
const char* ScriptSceneScreen::m_pDialogName = "ScriptSceneScreen";
ScriptSceneScreen::ScriptSceneScreen() : Dialog(m_pDialogName,m_pDialogName)
{
	GetLocation()->Create(0,0,D3DKernel::ResX(),D3DKernel::ResY());

	Insert(m_Top = new Curtain("top"),L_LEFT_TOP);
	Insert(m_Bottom = new Curtain("bottom"),L_LEFT_BOTTOM);

	m_Top->Typify(Curtain::T_TOP);
	m_Bottom->Typify(Curtain::T_BOTTOM);

	//	убираем мышинный курсор
	MouseCursor::SetVisible(false);

	SetVisible(true);
}

ScriptSceneScreen::~ScriptSceneScreen()
{
	MouseCursor::SetVisible(true);
}
//	уведомление об изменении состояния системы
void ScriptSceneScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}

//**********************************************************************//
//	class ScriptSceneScreen::Curtain
const float ScriptSceneScreen::Curtain::m_PerCent = 0.15f;
ScriptSceneScreen::Curtain::Curtain(const char* pName) : Widget(pName),m_Type(T_TOP)
{
}

ScriptSceneScreen::Curtain::~Curtain()
{
}
//	типизировать
void ScriptSceneScreen::Curtain::Typify(TYPE type)
{
	LPDIRECTDRAWSURFACE7 surface;
	
	if(GetImage()) DXImageMgr::Instance()->Release(static_cast<DXImage*>(GetImage()));
	if(Parent())
	{
		int width  = Parent()->GetLocation()->Region()->Width();
		int height = Parent()->GetLocation()->Region()->Height()*m_PerCent;

		SetImage(DXImageMgr::Instance()->CreateImage(width,height));
		
		if(GetImage() && (surface = ((DXImage *)GetImage())->m_pSurface))
		{
			ClearSurface(surface);
			
			switch(type)
			{
			case T_TOP:
				GetLocation()->Create(0,0,width,height);
				break;
			case T_BOTTOM:
				GetLocation()->Create(0,Parent()->GetLocation()->Region()->Height()-height,width,Parent()->GetLocation()->Region()->Height());
				break;
			}
		}
	}
}
//	привестив начальное состояние
void ScriptSceneScreen::Curtain::OnStart(void)
{
	m_StartTime = Timer::GetSeconds();

	switch(m_Type)
	{
	case T_TOP:
		MoveTo(0,0);
		break;
	case T_BOTTOM:
		MoveTo(0,Parent()->GetLocation()->Region()->Height()-GetLocation()->Region()->Height());
		break;
	}
}
//	обновить состояние
void ScriptSceneScreen::Curtain::OnUpdate(void)
{
	switch(m_Type)
	{
	case T_TOP:
		break;
	case T_BOTTOM:
		break;
	}
}
//*********************************************************************************//
void ScriptSceneScreen::Curtain::ClearSurface(LPDIRECTDRAWSURFACE7 surface)
{
	DDSURFACEDESC2 ddsd;
	memset(&ddsd,0,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	surface->GetSurfaceDesc(&ddsd);
	DDBLTFX ddfx;
	memset(&ddfx,0,sizeof(DDBLTFX));
	ddfx.dwSize = sizeof(DDBLTFX);
	ddfx.dwFillColor = 0;
	//	очищаем поверхность
	surface->Blt(0,0,0,DDBLT_COLORFILL,&ddfx);
}
