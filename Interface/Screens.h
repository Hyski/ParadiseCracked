/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   13.03.2001

************************************************************************/
#ifndef _SCREENS_H_
#define _SCREENS_H_

class HiddenMovementScreen;
class DescriptionScreen;
class InventoryScreen;
class HeroInfoScreen;
class FlagDescScreen;
class JournalScreen;
class OptionsScreen;
class OptionsScreen;
class DialogScreen;
class HelperScreen;
class TradeScreen;
class PanicScreen;
class GameScreen;

class Screens
{
public:
	enum SCREEN_ID
	{
		SID_GAME,
		SID_INVENTORY,
		SID_TRADE,
		SID_JOURNAL,
		SID_DIALOG,
		SID_HEROINFO,
		SID_HELPER,
		SID_OPTIONS,
		SID_HIDDENMOVEMENT,
		SID_PANIC,
		SID_DESCRIPTION
	};
private:
	struct Deleter 
	{
	public:
		Screens *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	Screens();
	virtual ~Screens();
public:
	//	получить интерфейс с игровым экраном
	GameScreen*	Game(void);
	//	получить интерфейс с экраном оборудования
	InventoryScreen* Inventory(void);
	//	получить интерфейс с экраном торговли
	TradeScreen* Trade(void);
	//	получить интерфейс с экраном журнала
	JournalScreen* Journal(void);
	//	получить интерфейс с экраном проведения диалога
	DialogScreen* Dialog(void);
	//	получить интерфейс с экраном описания героя
	HeroInfoScreen* HeroInfo(void);
	//	получить интерфейс с экраном опций внутри игры
	OptionsScreen* Options(void);
	//	получить интерфейс с экраном скрытых перемещений
	HiddenMovementScreen* HiddenMovement(void);
	//	получить интерфейс с экраном проигрывания паники
	PanicScreen* Panic(void);
	//	получить интерфейс с экраном описания предмета
	DescriptionScreen* Description(void);
	//	получить интерфейс с экраном помощника
	HelperScreen* Helper(void);
	//	получить интерфейс с экраном описанием флага
	FlagDescScreen* FlagDesc(void);
public:
	//	активизировать определенный экран
	void Activate(SCREEN_ID id);
	//	выйти в главное меню
	void ExitToMainMenu(void);
	//	проверить наличие экрана опций
	bool IsOptionsPresent(void);
public:
	static Screens *Instance(void);
};

inline Screens* Screens::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new Screens();
	return m_Deleter.m_pInstance;
}


#endif	//_SCREENS_H_