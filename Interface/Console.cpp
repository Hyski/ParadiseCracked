/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   04.04.2001

************************************************************************/
#include "Precomp.h"
#include "Edit.h"
#include "TextBox.h"
#include "WidgetFactory.h"
#include "WidgetSoundMgr.h"
//#include "../Common/D3DApp/D3DApp.h"
#include "../Common/D3DApp/D3DInfo/D3DInfo.h"

#include "Console.h"

//---------- Лог файл ------------
#ifdef _HOME_VERSION
CLog console_log;
#define console	console_log["console.log"]
#else
#define console	/##/
#endif
//--------------------------------

const char* Console::m_pDialogName = "Console";
Console* Console::m_Instance = 0;
bool Console::m_IsAvailable = false;
Console::Console() : Dialog(m_pDialogName,m_pDialogName),m_LastEvent(LE_NONE),m_MouseCapture(false)
{
	m_CommandLine = (Edit *)WidgetFactory::Instance()->Assert(this,"command_line");
	m_Content = (TextBox *)WidgetFactory::Instance()->Assert(this,"content");
	console("--- Console Initialized ---\n");
}

Console::~Console()
{
	m_Instance = 0;
	console("--- Console Released ---\n");
}
//	создать консоль
Console* Console::SetAvailable(void)
{
	m_IsAvailable = true;
	if(!m_Instance)
	{
		m_Instance = new Console();
		m_Instance->Starting();
	}

	return m_Instance;
}
//	добавить строку в окно вывода консоли
void Console::AddString(const char* string)
{
	if(m_Instance)
	{
		console("%s\n",string);
		m_Instance->m_Content->AddText(std::string(std::string(string)+"\n").c_str());
		m_Instance->m_Content->Scroll(TextBox::SD_END);
	}
}
//	получить строку текста из окна ввода консоли
const std::string& Console::GetCommandLine(void)
{
	static std::string empty;

	if(m_Instance)
	{
		return m_Instance->m_CommandLine->GetText();
	}

	return empty;
}
//	получить строку текста из окна ввода консоли как список аргументов
int Console::GetCommandLine(std::vector<std::string>& argv)
{
	argv.clear();

	if(m_Instance)
	{
		char* string = new char[m_Instance->m_CommandLine->GetText().size()+1];
		const char* seps = " ";
		char* token;
	
		strcpy(string,m_Instance->m_CommandLine->GetText().c_str());
		token = strtok(string,seps);
		while(token != 0)
		{
			argv.push_back(token);
			token = strtok(0,seps);
		}
		delete[] string;
	}

	return argv.size();
}
//	установить текст в окно ввода консоли
void Console::SetCommandLine(const char* string)
{
	if(m_Instance)
	{
		m_Instance->m_CommandLine->SetText(string);
	}
}
//	подписаться на события ввода от консоли
void Console::AddController(Controller* controller)
{
	if(m_Instance)
	{
		if(std::find(m_Instance->m_Controllers.begin(),m_Instance->m_Controllers.end(),controller)==m_Instance->m_Controllers.end())
		{
			m_Instance->m_Controllers.push_back(controller);
		}
	}
}
//	убрать контроллер на события ввода от консоли
void Console::RemoveController(Controller* controller)
{
	std::vector<Controller*>::iterator ic;

	if(m_Instance)
	{
		if((ic = std::find(m_Instance->m_Controllers.begin(),m_Instance->m_Controllers.end(),controller))!=m_Instance->m_Controllers.end())
		{
			m_Instance->m_Controllers.erase(ic);
		}
	}
}
//	высветить консоль на дисплей
void Console::Show(bool show)
{
	if(m_Instance)
	{
		m_Instance->SetVisible(show);
		if(show && m_Instance->Parent()) 
		{
			m_Instance->Parent()->BringToTop(m_Instance->Name());
			m_Instance->MoveTo((D3DKernel::ResX()-m_Instance->GetLocation()->Region()->Width())/2,(D3DKernel::ResY()-m_Instance->GetLocation()->Region()->Height())/2);
		}
	}
}
//	высвечена ли консоль на экране
bool Console::IsOnScreen(void)
{
	if(m_Instance)
		return m_Instance->IsVisible();

	return false;
}
//	получить последнее событие от консоли
Console::LAST_EVENT Console::LastEvent(void)
{
	if(m_Instance)
		return m_Instance->m_LastEvent;

	return LE_NONE;
}
//	стартовая информация выводимая в консоль
void Console::Starting(void)
{
	char pBuff[200];

	sprintf(pBuff,">> console activated time %0.2d:%0.2d",Timer::GetSysTime()->tm_hour,Timer::GetSysTime()->tm_min);
	AddString(pBuff);
/*	sprintf(pBuff,"using %s on %s",D3DInfo::m_vDriverInfo[GSupport::m_CurVideoMode.m_iDriver].vDevice[GSupport::m_CurVideoMode.m_iDevice].sName.c_str(),D3DInfo::m_vDriverInfo[GSupport::m_CurVideoMode.m_iDriver].sDescription.c_str());
	AddString(pBuff);
	sprintf(pBuff,"video mode: [%dx%dx%d]",GSupport::m_CurVideoMode.m_iWidth,GSupport::m_CurVideoMode.m_iHeight,GSupport::m_CurVideoMode.m_iDepth);
	AddString(pBuff);*/
}

//********************** Реализация функция виджета ***************************//
//	обработка мышинного ввода
bool Console::ProcessMouseInput(VMouseState* pMS)
{
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
		}
	}
	if(Parent() && (Parent()->Focus()->Get(WidgetFocus::FF_KEYBOARD) != this) && (pMS->dx || pMS->dy))
	{
		m_LastEvent = LE_ACTIVATE;
		Focus()->Set(WidgetFocus::FF_KEYBOARD,m_CommandLine);
		Parent()->OnChange(this);
	}

	return true;
}
//	установка видимости
void Console::SetVisible(bool bVisible)
{
	Dialog::SetVisible(bVisible);
	if(bVisible)
	{
		m_LastEvent = LE_ACTIVATE;
		Focus()->Set(WidgetFocus::FF_KEYBOARD,m_CommandLine);
		Parent()->OnChange(this);
		Widget::Player::Sample* sample = WidgetSoundMgr::Instance()->CreateSample("CONSOLE");
		WidgetSoundMgr::Instance()->GetPlayer()->Play(sample);
		sample->Release();
	}
}
//	потеря ввода от клавиатуры
void Console::OnKeyboardLost(void)
{
	Widget::OnKeyboardLost();
}
//	обработка потери клавиатурного ввода
bool Console::ProcessKeyboardLost(void)
{
	Focus()->Set(WidgetFocus::FF_KEYBOARD,0);
	return true;
}
//	обработка системного сообщения
void Console::OnSystemChange(void)
{
	Widget::OnSystemChange();
	MoveTo((D3DKernel::ResX()-m_Instance->GetLocation()->Region()->Width())/2,(D3DKernel::ResY()-m_Instance->GetLocation()->Region()->Height())/2);
}

//	обработка событий от дочерних меню
void Console::OnChange(Widget* pChild)
{
	if(m_CommandLine == pChild)
	{
		AddString(std::string(std::string("/")+m_CommandLine->GetText()).c_str());
		for(int i=0;i<m_Controllers.size();i++)
		{
			m_Controllers[i]->Activate();
		}
		m_CommandLine->SetText("");
	}
}
//	производим какие-то действия
bool Console::Tick(void)
{
	if(m_MouseCapture)
	{
		MoveTo(Input::MouseState().x-m_xMouse-GetLocation()->OriginX(),
			   Input::MouseState().y-m_yMouse-GetLocation()->OriginY());
	}
//	TEST
	//	так тестируем
	if(Input::KeyFront(DIK_NUMPADMINUS))
	{
		m_Instance->m_Content->Scroll(-5);
	}
	if(Input::KeyFront(DIK_NUMPADPLUS))
	{
		m_Instance->m_Content->Scroll(5);
	}
//	TEST

	return true;
}

