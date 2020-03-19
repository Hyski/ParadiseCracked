/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   13.03.2001

************************************************************************/
#include "Precomp.h"
#include "DescriptionScreen.h"
#include "InventoryScreen.h"
#include "OptionsScreen.h"
#include "LoadingScreen.h"
#include "JournalScreen.h"
#include "WidgetFactory.h"
#include "DialogScreen.h"
#include "TradeScreen.h"
#include "MouseCursor.h"
#include "GameScreen.h"
#include "MenuMgr.h"
#include "Screens.h"

Screens::Deleter Screens::m_Deleter;

Screens::Screens()
{
}

Screens::~Screens()
{
}

//	получить интерфейс с игровым экраном
GameScreen*	Screens::Game(void)
{
	return (GameScreen *)WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName);
}
//	получить интерфейс с экраном оборудовани€
InventoryScreen* Screens::Inventory(void)
{
	return (InventoryScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),InventoryScreen::m_pDialogName);
}
//	получить интерфейс с экраном торговли
TradeScreen* Screens::Trade(void)
{
	return (TradeScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),TradeScreen::m_pDialogName);
}
//	получить интерфейс с экраном журнала
JournalScreen* Screens::Journal(void)
{
	return (JournalScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),JournalScreen::m_pDialogName);
}
//	получить интерфейс с экраном журнала
DialogScreen* Screens::Dialog(void)
{
	return (DialogScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),DialogScreen::m_pDialogName);
}
//	получить интерфейс с экраном описани€ геро€
HeroInfoScreen* Screens::HeroInfo(void)
{
	return (HeroInfoScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),HeroInfoScreen::m_pDialogName);
}
//	получить интерфейс с экраном опций внутри игры
OptionsScreen* Screens::Options(void)
{
	return (OptionsScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),OptionsScreen::m_pDialogName);
}
//	получить интерфейс с экраном скрытых перемещений
HiddenMovementScreen* Screens::HiddenMovement(void)
{
	return (HiddenMovementScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),HiddenMovementScreen::m_pDialogName);
}
//	получить интерфейс с экраном проигрывани€ паники
PanicScreen* Screens::Panic(void)
{
	return (PanicScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),PanicScreen::m_pDialogName);
}
//	получить интерфейс с экраном описани€ предмета
DescriptionScreen* Screens::Description(void)
{
	return (DescriptionScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),DescriptionScreen::m_pDialogName);
}
//	получить интерфейс с экраном помощника
HelperScreen* Screens::Helper(void)
{
	return (HelperScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),HelperScreen::m_pDialogName);
}
//	получить интерфейс с экраном описанием флага
FlagDescScreen* Screens::FlagDesc(void)
{
	return (FlagDescScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),FlagDescScreen::m_pDialogName);
}
//	активизировать определенный экран
void Screens::Activate(SCREEN_ID id)
{
	GameScreen* pGameScreen = static_cast<GameScreen*>(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName));

	switch(id)
	{
	case SID_GAME:
		//	показываем курсор
		MouseCursor::SetVisible(true);
		pGameScreen->UnloadOptionsScreen();
		pGameScreen->ShowDialog(0);
		break;
	case SID_INVENTORY: 
		pGameScreen->ShowDialog(InventoryScreen::m_pDialogName);
		break;
	case SID_TRADE: 
		pGameScreen->ShowDialog(TradeScreen::m_pDialogName);
		break;
	case SID_JOURNAL: 
		pGameScreen->ShowDialog(JournalScreen::m_pDialogName);
		break;
	case SID_DIALOG: 
		pGameScreen->ShowDialog(DialogScreen::m_pDialogName);
		break;
	case SID_HEROINFO: 
		pGameScreen->ShowDialog(HeroInfoScreen::m_pDialogName);
		break;
	case SID_HELPER: 
		pGameScreen->ShowDialog(HelperScreen::m_pDialogName);
		break;
	case SID_OPTIONS:
		//	делаем активным экран опций
		pGameScreen->LoadOptionsScreen();
		pGameScreen->ShowDialog(OptionsScreen::m_pDialogName);
		break;
	case SID_HIDDENMOVEMENT: 
		//	пр€чем курсор
		MouseCursor::SetVisible(false);
		pGameScreen->ShowDialog(HiddenMovementScreen::m_pDialogName);
		break;
	case SID_PANIC: 
		//	пр€чем курсор
		MouseCursor::SetVisible(false);
		pGameScreen->ShowDialog(PanicScreen::m_pDialogName);
		break;
	case SID_DESCRIPTION: 
		pGameScreen->ShowDialog(DescriptionScreen::m_pDialogName);
		break;
	}
}
//	выйти в главное меню
void Screens::ExitToMainMenu(void)
{
	Game()->ExitToMainMenu();
}
//	проверить наличие экрана опций
bool Screens::IsOptionsPresent(void)
{
	return Game()->Child(OptionsScreen::m_pDialogName)?true:false;
}

