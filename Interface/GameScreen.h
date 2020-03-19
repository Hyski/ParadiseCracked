/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   07.03.2001

************************************************************************/
#ifndef _GAME_SCREEN_H_
#define _GAME_SCREEN_H_

#include "Dialog.h"

class GameScreenController;
class VehicleEntity;
class HumanEntity;
class WeaponIcon;
class TextBox;
class Marker;
class Static;
class Text;

class GameScreen : public Dialog
{
public:
	class EnemyIconMgr;
	class HackVehicleMgr;
	class PersonIconMgr;
public:
	static const char* m_pDialogName;
	static const char* m_WeaponIconName;
private:
	class MessageWindow;
	MessageWindow* m_pMsgWindow;
	class VehicleMonitor;
	struct VM
	{
		VehicleMonitor* m_pMonitor;
		bool m_bVisible;
	} m_pVehicleMonitors[6];
	GameScreenController *m_pController;
	WeaponIcon* m_WeaponIcon;
	bool m_IsCameraMoveable;				//	флаг, говорящий о возможности перемещения камеры
	bool m_bUnloadOptionsScreen;
	//	менеджер иконок врагов
	EnemyIconMgr* m_EnemyIconMgr;
	//	менеджер иконок хакнутой техники
	HackVehicleMgr* m_HackVehicleMgr;
	//	менеджер иконок на персонажи
	PersonIconMgr* m_PersonIconMgr;
	//	текущий активный диалог
	Widget* m_CurrentActiveDilog;
public:
	GameScreen();
	virtual ~GameScreen();
public:
	//	производим какие-то действия
	bool Tick(void);
public:
	//	сменить дочернее диалоговое окно (0 - убрать все окна)
	void ShowDialog(const char *pName);
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
	//	загрузить меню опций
	void LoadOptionsScreen(void);
	//	выгрузить меню опций
	void UnloadOptionsScreen(void);
	//	спрятать/показать элементы интерфейса
	void ShowControls(bool bShow);
	//	получить утверждение, что камеру можно перемещать
	bool IsCameraMoveable(void);
	//	получить интерфейс на окно сообщений
	MessageWindow* MsgWindow(void);
	//	выйти в главное меню
	void ExitToMainMenu(void);
	//	записать/прочитать файл "автосохранение" и "быстрое сохранение" (если файла нет return false)
	enum SPECIAL_SAVELOAD_TYPE {SST_AUTOSAVE,SST_QUICKSAVE};
	enum SPECIAL_SAVELOAD_MODE {SSM_LOAD,SSM_SAVE};
	bool MakeSpecialSaveLoad(SPECIAL_SAVELOAD_TYPE type,SPECIAL_SAVELOAD_MODE mode);
public:
	//	для работы со скриптовыми сценами
	void OpenScriptSceneScreen(void);
	void CloseScriptSceneScreen(void);
public:
/****************** ВНЕШНИЙ ИНТЕРФЕЙС С ЛОГИКОЙ *********************/
	//	установить контроллер логики для интерфейса
	void SetController(GameScreenController *pController);
	//	**** кнопка переключения в журнал ****
	enum JOURNAL_BUTTON_STATE {JBS_NORMAL,JBS_SELECTED,JBS_BLINK};
	void SetJournalButtonState(JOURNAL_BUTTON_STATE jbs);
	//  **** обычные кнопки ****
	enum BUTTON
	{
		B_RELOAD,
		B_UP_ARROW,
		B_DOWN_ARROW,
		MAX_BUTTON
	};
	// переключение кнопки в состояние Enable/Disable
	void EnableButtonState(BUTTON id,bool bEnable);
	//  **** кнопка управления персонажем ****
	enum CHECKBOX {CB_RUN,
				   CB_SIT,
				   CB_SPECIAL,
				   CB_USE,
				   CB_ROOF,
				   CB_JOURNAL,
				   CB_ENDTURN,
				   CB_MENU,
				   CB_FOS,
				   CB_AUTOSHOT,
				   CB_SNAPSHOT,
				   CB_AIMSHOT,
				   CB_BACKPACK,
				   MAX_CHECKBOX};
	enum CHECKBOX_STATE {CBS_ON=1,CBS_OFF=0};
	enum BN_CLICK {BNC_NONE,BNC_LCLICK,BNC_RCLICK,BNC_LDBLCLICK,BNC_RDBLCLICK};
	// переключение кнопки в заданное состояние
	void SetCBState(CHECKBOX id,CHECKBOX_STATE state);
	// получить состояние кнопки	
	CHECKBOX_STATE GetCBState(CHECKBOX id);
	// переключение кнопки в состояние Enable/Disable
	void EnableCBState(CHECKBOX id,bool bEnable);
	//	новый интерфейс для установки иконок
	PersonIconMgr* GetPersonIconMgr(void) const {return m_PersonIconMgr;}
	//  **** иконки для состояния человека ****
	struct BodyIconTraits
	{
		enum BODY_PARTS_STATE {BPS_NONE,BPS_GREEN,BPS_BLUE,BPS_YELLOW,BPS_RED};
		enum DAMAGE_TYPE {DT_ELECTRIC	= 0x01,
						  DT_SHOCK		= 0x02,
					      DT_FLAME		= 0x04};
		//	-------------------------------------------------------
		BODY_PARTS_STATE m_Head;	//	состояние для ГОЛОВЫ
		BODY_PARTS_STATE m_Body;	//	состояние для ТУЛОВА
		BODY_PARTS_STATE m_Hands;	//	состояние для РУК
		BODY_PARTS_STATE m_Legs;	//	состояние для НОГ
		unsigned int m_Damages;		//	флаги на поражение
	};
	void SetBodyIcon(const BodyIconTraits& bit);
	//	структура для устанавления надписей для мувпоинтов использования предмета
	struct ShootMovePoints
	{//	строка должна присутсвовать обязательно, 0 - недопустимое значение
		const char* m_AimMP;	unsigned int m_AimColor;
		const char* m_SnapMP;	unsigned int m_SnapColor;
		const char* m_AutoMP;	unsigned int m_AutoColor;
	};
	void SetShootMovePoints(const ShootMovePoints& smp);
	//  **** иконки для техники ****
	struct VehicleIconTraits
	{
		enum SEL_TYPE {ST_NONE,ST_NORMAL};
		enum VEHICLE_ICON_FLAG {VIF_INMOVEMENT = 0x01};
		//	-------------------------------------------------------
		unsigned int m_Flags;					//	состояние иконки (флаги) VEHICLE_ICON_FLAG
		bool m_Visible;							//	видим/не видим
		int m_MaxCrew;							//	максимальное кол-во человек
		int m_NumCrew;							//	текущее кол-во человек
		int m_MovePoints;						//	мувпоинты
		int m_HitPoints;						//	кол-во брони у техники
		unsigned int m_Value;					//	значение параметра
		SEL_TYPE m_Selection;					//	тип выделения
	};
	void SetVehicleIcon(unsigned int icon,const VehicleIconTraits& vit);
	//  **** иконки для оружия****
	//	установить сстояние для иконки (0 - не отображать иконку)
	struct WeaponIconTraits
	{
		const char* m_ImageName;				//	системное имя картинки на оружие
		const char* m_AmmoQuantity;				//	кол-во патронов в оружии
		const char* m_AmmoDesc;					//	строка описания патронов
		WeaponIconTraits() : m_ImageName(0),m_AmmoQuantity(0),m_AmmoDesc(0) {}
		WeaponIconTraits(const char* image_name,const char* ammo_quantity,const char* ammo_desc) : m_ImageName(image_name),m_AmmoQuantity(ammo_quantity),m_AmmoDesc(ammo_desc) {}
	};
	//	установить сстояние для иконки (0 - не отображать иконку)
	void SetWeaponIcon(const WeaponIconTraits* traits);
	bool IsCursorOnWeaponIcon(void);
	//	**** Управение иконками видимости врагов ****
	EnemyIconMgr* GetEnemyIconMgr(void) const {return m_EnemyIconMgr;}
	int GetEnemyIconBottom(void);
	int GetEnemyIconTop(void);
	void OnEnemyIconClick(Marker* marker);
	//	**** Управение иконками хакнутой техники ****
	HackVehicleMgr* GetHackVehicleMgr(void) const {return m_HackVehicleMgr;}
	int GetHackVehicleIconLeft(void);
	int GetHackVehicleIconRight(void);
	void OnHackVehicleIconClick(VehicleEntity* ve,GameScreen::BN_CLICK click_type);
//****************************************************************************//
private:
	struct DoubleCheckBox
	{//	структурка для представления и управления чек-боксами
		Static* m_VisualArea;	//	то что видно на экране, отображение
		Static* m_HotArea;		//	область, куда надо кликать мышкой
	};
	DoubleCheckBox m_CheckBoxes[MAX_CHECKBOX];
	Static* m_Buttons[MAX_BUTTON];
	enum MP_TEXT_FIELD {MTF_AIM,MTF_SNAP,MTF_AUTO,MAX_MTF};
	Text* m_MPTextFields[MAX_MTF];
public:
	void OnChange(Widget* pChild);
	void OnStatic(Static *pStatic);
};

inline GameScreen::MessageWindow* GameScreen::MsgWindow(void)
{
	return m_pMsgWindow;
}

//**********************************************************************//
//*********	              class MessageWindow              *************//
class GameScreen::MessageWindow : public Dialog
{
private:

	class StringBuffer;				//	класс, ответсвенный за хранение строк

private:
	float m_iMaxHeight;				//	макимальная высота окна
	float m_iMinHeight;				//	минимальная высота окна
	float m_iCurHeight;				//	текущее положение окна
	float m_iAimHeight;				//	высота которую необходимо достичь
	const float m_MovingStep;
private:
	std::auto_ptr<StringBuffer> m_StringBuffer;		//	буфер строк
	TextBox *m_MessageBox;							//	окно с текстом
	int m_iMessageBoxHeight;						//	высота окна с текстом
public:
	MessageWindow();
	virtual ~MessageWindow();
public:
	//	уведомление об изменении состояния системы
	void CheckConformMode(void);
public:
	void OnStatic(Static *pStatic);
public:
	//	увеличить видимость окна
	void IncMsgWindow(void);
	//	уменьшить видимость окна
	void DecMsgWindow(void);
	//	узнать - окно выдвинуто или нет
	bool IsPulled(void);
	//	добавить текст в окно
	void AddText(const char* pText);
	//	очистить окошко
	void Clear(void);
protected:
	//	двигаем окно если требуется
	bool Tick(void);
};

//	узнать - окно выдвинуто или нет
inline bool GameScreen::MessageWindow::IsPulled(void)
{
	return !(m_iAimHeight == m_iMinHeight);
}

//**********************************************************************//
//*********	              class VehicleMonitor             *************//
class GameScreen::VehicleMonitor : public Dialog
{
private:
	static const char* m_pScriptName;				//	имя скрипта
	enum {MAX_PLACES = 6};
	Static* m_Places[MAX_PLACES];
	static const int m_PlacesX;
	static const int m_PlacesY;
	static const int m_PlacesWidth;
	static const int m_PlacesHeight;
public:
	VehicleMonitor(const char *pName);
	virtual ~VehicleMonitor();
public:
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
	//	сообщение от статика
	void OnStatic(Static *pStatic);
public:
	void SetMaxCrew(int value);
	void SetNumCrew(int value);
	void SetMovePoints(int value);
	void SetHealth(int value);
	void EnableInMovement(bool bEnable);
public:
	virtual const char* Type(void) const {return "vehiclemonitor";};
};

//**********************************************************************//
//*********	       class GameScreenController              *************//
class GameScreenController
{
public:
	virtual ~GameScreenController(){};
public:
	// реакция на клик мыши, передается текущее состояние кнопки
	virtual void OnCheckBoxClick(GameScreen::CHECKBOX id) = 0;
	// реакция на клик мыши, передается текущее состояние кнопки
	virtual void OnButtonClick(GameScreen::BUTTON id) = 0;
	// реакция на клик мыши на иконке персонажа
	virtual void OnIconClick(unsigned int id,GameScreen::BN_CLICK click_type) = 0;
	// реакция на клик мыши на иконке оружия
	virtual void OnWeaponIconClick(void) = 0;
	// реакция на клик мыши на иконке техники
	virtual void OnVehicleIconClick(unsigned int icon_number,GameScreen::BN_CLICK click_type) = 0;
	// реакция на клик мыши на иконке врага
	virtual void OnEnemyIconClick(Marker* marker) = 0;
	// реакция на клик мыши на иконке врага
	virtual void OnHackVehicleClick(VehicleEntity* ve,GameScreen::BN_CLICK click_type) = 0;
};

//**********************************************************************
//	class GameScreen::EnemyIconMgr
class GameScreen::EnemyIconMgr
{
public:
	class EnemyIcon;
	class EnemyIconExtended;
	class EnemyIconAsWidget;
private:
	static int m_Bottom;							//	нижняя граница (не изменяется)
	static int m_Top;								//	верхняя граница 
private:
	std::list<EnemyIconExtended*> m_Icons;
	GameScreen* m_Owner;								//	экран GameScreen
public:
	EnemyIconMgr(GameScreen* owner);
	virtual ~EnemyIconMgr();
public:
	void Insert(EnemyIcon& ei);
	void Remove(Marker* marker);
	void Clear(void);
	EnemyIcon* ItemAt(const Marker* marker);
	EnemyIcon* ItemAt(unsigned int number);
	//	установить флаги для всех иконок стразу
	void SetFlags(unsigned int flags);
	//	обновить расположение иконок
	void UpdateLocation(void);
private:
	ipnt2_t GetNextLocation(unsigned int size);
};

//**********************************************************************
//	class GameScreen::EnemyIconMgr::EnemyIcon
class GameScreen::EnemyIconMgr::EnemyIcon
{
public:
	enum FLAGS
	{
		F_NONE		= 0,
		F_SELECTED	= 1<<0
	};
protected:
	std::string m_Tip;
	Marker* m_Marker;
	unsigned int m_Flags;
public:
	//	конструктор копирования
	EnemyIcon(const EnemyIcon& ei);
	//	конструктор инициализации
	EnemyIcon(const std::string& tip = "",Marker* marker = 0,unsigned int flags = F_NONE);
	//	деструктор
	virtual ~EnemyIcon();
public:
	std::string GetTip(void) const;
	unsigned int GetFlags(void) const;
	virtual void SetFlags(unsigned int flags);
	Marker* GetMarker(void) const;
};

//**********************************************************************
//	class GameScreen::EnemyIconMgr::EnemyIconExtended
class GameScreen::EnemyIconMgr::EnemyIconExtended : public GameScreen::EnemyIconMgr::EnemyIcon
{
private:
	GameScreen* m_Owner;					//	родительский виджет
	EnemyIconAsWidget* m_Icon;				//	виджет, отображающий иконку
public:
	EnemyIconExtended(GameScreen* owner,const GameScreen::EnemyIconMgr::EnemyIcon& ei);
	virtual ~EnemyIconExtended();
public:
	void SetFlags(unsigned int flags);
	void MoveTo(const ipnt2_t& pt);
	void SetNumber(int number);
public:
	//	уведомление от иконки
	void OnClick(void);
};

//**********************************************************************
//	class GameScreen::EnemyIconMgr::EnemyIconAsWidget
class GameScreen::EnemyIconMgr::EnemyIconAsWidget : public Widget
{
public:
	static const char* m_EnemyIconName;
	static const char* m_EnemyIconSelectedName;
	static const char* m_NameDesc;
private:
	static int m_Width;
	static int m_Height;
private:
	Static* m_Icon;							//	виджет, отображающий иконку
	Text* m_Number;							//	текстовое поле для отображения номера иконки
	bool m_Selected;
	EnemyIconExtended* m_Link;				//	связь виджета с логикой
public:
	EnemyIconAsWidget(EnemyIconExtended* link);
	virtual ~EnemyIconAsWidget();
public:
	//	установить номер иконки
	void SetNumber(const char* number);
	//	установить/снять выделение
	void SetSelection(bool enable);
	//	вернуть размеры иконки
	static int Width(void) {return m_Width;}
	static int Height(void) {return m_Height;}
private:
	std::string GenerateName(void);
	//	сообщение от дочерних окон
	void OnChange(Widget* pChild);
};

//**********************************************************************
//	class GameScreen::HackVehicleMgr
class GameScreen::HackVehicleMgr
{
public:
	class Icon;
	class IconExtended;
	class IconAsWidget;
private:
	static int m_Right;							//	верхняя граница
	static int m_Left;							//	нижняя граница (не изменяется)
private:
	std::list<IconExtended*> m_Icons;
	GameScreen* m_Owner;								//	экран GameScreen
public:
	HackVehicleMgr(GameScreen* owner);
	virtual ~HackVehicleMgr();
public:
	void Insert(Icon& icon);
	void Remove(VehicleEntity* ve);
	void Clear(void);
	Icon* ItemAt(const VehicleEntity* ve);
	Icon* ItemAt(unsigned int number);
	//	установить флаги для всех иконок стразу
	void SetFlags(unsigned int flags);
	//	обновить расположение иконок
	void UpdateLocation(void);
private:
	ipnt2_t GetNextLocation(unsigned int size);
};


//**********************************************************************
//	class GameScreen::HackVehicleMgr::Icon
class GameScreen::HackVehicleMgr::Icon
{
public:
	enum FLAGS
	{
		F_NONE		= 0,
		F_SELECTED	= 1<<0
	};
protected:
	std::string m_Tip;
	VehicleEntity* m_VehicleEntity;
	unsigned int m_Flags;
public:
	//	конструктор копирования
	Icon(const Icon& icon) : m_Tip(icon.m_Tip),m_VehicleEntity(icon.m_VehicleEntity),m_Flags(icon.m_Flags) {}
	//	конструктор инициализации
	Icon(const std::string& tip = "",VehicleEntity* ve = 0,unsigned int flags = F_NONE) : m_Tip(tip),m_VehicleEntity(ve),m_Flags(flags) {}
	//	деструктор
	virtual ~Icon() {}
public:
	std::string GetTip(void) const {return m_Tip;}
	unsigned int GetFlags(void) const {return m_Flags;}
	virtual void SetFlags(unsigned int flags) {m_Flags = flags;}
	VehicleEntity* GetVehicleEntity(void) const {return m_VehicleEntity;}
};


//**********************************************************************
//	class GameScreen::HackVehicleMgr::IconExtended
class GameScreen::HackVehicleMgr::IconExtended : public GameScreen::HackVehicleMgr::Icon
{
private:
	GameScreen* m_Owner;					//	родительский виджет
	IconAsWidget* m_Icon;					//	виджет, отображающий иконку
public:
	IconExtended(GameScreen* owner,const GameScreen::HackVehicleMgr::Icon& icon);
	virtual ~IconExtended();
public:
	void SetFlags(unsigned int flags);
	void MoveTo(const ipnt2_t& pt);
	void SetNumber(int number);
public:
	//	уведомление от иконки
	void OnClick(GameScreen::BN_CLICK btn_clk);
};


//**********************************************************************
//	class GameScreen::HackVehicleMgr::IconAsWidget
class GameScreen::HackVehicleMgr::IconAsWidget : public Widget
{
public:
	static const char* m_IconName;
	static const char* m_IconSelectedName;
	static const char* m_NameDesc;
private:
	static int m_Width;
	static int m_Height;
private:
	Static* m_Icon;							//	виджет, отображающий иконку
	Text* m_Number;							//	текстовое поле для отображения номера иконки
	bool m_Selected;
	IconExtended* m_Link;					//	связь виджета с логикой
	int m_lastHealth;				//	здоровье
	int m_lastMovepnts;			//	очки хода
public:
	IconAsWidget(IconExtended* link);
	virtual ~IconAsWidget();
public:
	//	установить номер иконки
	void SetNumber(const char* number);
	//	установить/снять выделение
	void SetSelection(bool enable);
	//	вернуть размеры иконки
	static int Width(void) {return m_Width;}
	static int Height(void) {return m_Height;}
private:
	std::string GenerateName(void);
	//	сообщение от дочерних окон
	void OnChange(Widget* pChild);
	//	собрать всплывающую подсказку
	std::string AssemblyTip(int h,int mp);
	//	производим проверку на изменение состояния
	bool Tick(void);
};

//**********************************************************************
//	class GameScreen::PersonIconMgr
class GameScreen::PersonIconMgr
{
public:
	class Icon;
	class Monitor;
	//	свойства монитора
	struct MonitorTraits
	{
		enum FLAGS 
		{	
			F_HEALTH		= 1<<0,
			F_PORTRAIT		= 1<<1,
			F_SELECTION		= 1<<2,
			F_ICONFLAGS		= 1<<3,
			F_MOVEPOINTS	= 1<<4,
			F_HEARTBEAT		= 1<<5
		};
		unsigned int m_Flags;		//	флаги полей которые следует учитывать
		//	тип выделения
		enum SEL_TYPE {ST_NONE,ST_NORMAL,ST_DEAD};
		SEL_TYPE m_Selection;		//	тип выделения
		//	тип серцебиения
		enum HEARTBEAT {HB_NONE,HB_CALM,HB_EXITED,HB_WOUND,HB_DEAD};
		HEARTBEAT m_Heartbeat;		//	тип серцебиения
		//	состояние иконки (флаги)
		enum ICON_FLAG {IF_INVEHICLE  = 0x01,
						IF_LEVELUP    = 0x02,
						IF_INMOVEMENT = 0x04,
						IF_INPANIC	  = 0x08
					   };
		unsigned int m_StateFlags;	//	состояние иконки (флаги)
		int m_MovePoints;			//	кол-во мувпоинтов
		int m_Health;				//	уровень здоровья
		std::string m_Portrait;		//	имя картинки на портрет персонажа
		MonitorTraits() : m_Flags(0) {}
		MonitorTraits(const MonitorTraits& t) : m_Flags(t.m_Flags),m_Selection(t.m_Selection),
			m_Health(t.m_Health),m_Portrait(t.m_Portrait),m_Heartbeat(t.m_Heartbeat),
			m_StateFlags(t.m_StateFlags),m_MovePoints(t.m_MovePoints) {}
	};
public:
	static unsigned int m_MaxMonitors;					//	максимальное кол-во мониторов
	enum REMOVE_TYPE {RT_QUICK,RT_SLOW};
private:
	GameScreen* m_Owner;								//	экран GameScreen
	std::list<Monitor*> m_Monitors;						//	мониторы персонажей
	std::list<Monitor*> m_MonitorViews;					//	мониторы персонажей на экране
	std::list<Icon*> m_Icons;
public:
	PersonIconMgr(GameScreen* owner);
	virtual ~PersonIconMgr();
public:
	//	добавить иконку (иконка выдвигается)
	void InsertIcon(const Icon& icon);
	//	получить описание иконки по идентификатору HumanEntity
	Icon* IconAt(const HumanEntity* he);
	//	получить описание иконки по порядковому номеру
	Icon* IconAt(unsigned int number);
	//	удалить иконку  (иконка задвигается)
	void RemoveIcon(const HumanEntity* he,REMOVE_TYPE rt);
	//	удалить все иконки 
	void RemoveAll(void);
public:
	//	тик
	void Tick(void);
	//	переопределить координаты мониторов
	void SwapMonitors(void);
};

//**********************************************************************
//	class GameScreen::PersonIconMgr::Icon
class GameScreen::PersonIconMgr::Icon
{
private:
	HumanEntity* m_HumanEntity;					//	основной идентификатор
	Monitor* m_Monitor;							//	монитор связанный с иконкой
	MonitorTraits m_MonitorState;				//	изменившееся состояние монитора
public:
	Icon(HumanEntity* he = 0);
	Icon(const Icon& icon,GameScreen::PersonIconMgr::Monitor* monitor);
	virtual ~Icon();
public:
	HumanEntity* GetHumanEntity(void) const {return m_HumanEntity;}
	Monitor* GetMonitor(void) const {return m_Monitor;}
	//	установить различные свойства
	void SetHealth(int health);
	void SetMovePoints(int mp);
	void SetHeartbeat(MonitorTraits::HEARTBEAT hb);
	void SetStateFlags(unsigned int flags);
	void SetPortrait(const char* name,MonitorTraits::SEL_TYPE selection);
	//	передача управления в кадре
	void Tick(void);
public:
	//	удалить иконку и уничтожить объект this в памяти
	void Remove(REMOVE_TYPE rt);
};

#endif	//_GAME_SCREEN_H_