/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#ifndef _MENU_MGR_H_
#define _MENU_MGR_H_

#include "../Common/UI/Widget.h"
#include "../ModMgr.h"

class ISndThemeSession;
class LoadingScreen;
class OptionsMenu;
class LoadingMenu;
class GameScreen;
class MainMenu;
class GameMenu;
class Dialog;
class DXView;
class Text;

class MenuMgr : public Widget
{
public:
	class ic_State;
	class ic_MouseCursor;
	class ic_LoadLevelModifier;
	class ic_LoadSaveGameModifier;
private:
	class ic_Tip;
	class ic_InterfaceTip;
private:
	struct Deleter 
	{
	public:
		MenuMgr *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	DXView *m_pDXView;						//	модуль отображения виджетов
	bool m_bLockInput;						//	флаг блокировки ввода 
	ic_Tip* m_Tip;							//	всплывающая подсказка
	ic_InterfaceTip* m_InterfaceTip;		//	управление всплывающей подсказкой
	ic_State* m_State;						//	режим работы интерфейса
	//	модификаторы
	ic_LoadLevelModifier* m_LoadLevelModifier;			//	на загрузку уровня
	ic_LoadSaveGameModifier* m_LoadSaveGameModifier;	//	на загрузку сохраненной игры
	//	сэмпл для проигрывания в главном меню
	Widget::Player::Sample* m_RainSound;
	ISndThemeSession* m_AmbientSound;
	//	текст-поле версии приложения
	Text* m_Version;
#if FIX
	//	текущий мод
	const Mod m_currentMod;			
#endif
	//	мышинный курсор
	static ic_MouseCursor* m_MouseCursor;
public:
	static const int m_MainMenuResX;
	static const int m_MainMenuResY;
	static const char* m_StartLevel;
private:
	MenuMgr();
	virtual ~MenuMgr();
public:
	//	отрисовка текущего меню
	void Render(void);
	void Draw(WidgetView* /*pView*/){}
	//	обработка ввода
	void OnMouse(void);
	void OnKeyboard(void);
	//	обработка событий от дочерних меню
	void OnChange(Widget* pChild);
	//	реагирование на смену видео режима
	void OnChangeVideoMode(void);
	//	обработка всего всего
	bool Tick(void);
public:
	//	загрузка уровня
	void LoadLevel(const char* name);
	//	обновление прогресc бара ЗАГРУЗКИ УРОВНЯ
	void UpdateLoadingProgressBar(float value);
	//	установить заголовок прогресc бара ЗАГРУЗКИ УРОВНЯ
	void SetProgressBarTitle(const char* title);
	//	записать/считать данные необходимые интерфейсу
	void MakeSaveLoad(Storage& store);
	//	вывести на экран MessageBox
	int QuestionBox(const char* text);
	//	можно ли вывести на экран MessageBox
	bool IsQBFree(void);
	//	выдать указатель на Tip'сину
	ic_Tip* Tip(void);
	//	заблокировать ввод на интерфейсе
	void LockInput(bool lock);
	//	заблокирован ли ввод на интерфейсе
	bool IsLockInput(void);
	//	получить утверждение, что камеру можно перемещать
	bool IsCameraMoveable(void);
	//	быстрый выход из программы
	void ExitFromProgram(void);
	//	освобождение всех ресурсов
	void Release(void);
public:
	//	показать то или иное меню
	void ShowDialog(const char *pName);
	//	выйти из игры
	void ExitFromGame(void);
	//	получить строковый ресурс
	std::string GetStrResourse(const std::string& name);
	//	получить элемент отображения версии игры
	Text* Version(void) const;
public:
	ic_LoadLevelModifier* LoadLevelModifier(void);
	ic_LoadSaveGameModifier* LoadSaveGameModifier(void);
	//	сбросить все модификаторы
	void ResetModifiers(void);
	//	получить мышинный курсор
	static ic_MouseCursor* MouseCursor(void) {return m_MouseCursor;}
	//	получить указатель на модуль воспроизведения виджетов
	WidgetView* GetDXView(void) {return reinterpret_cast<WidgetView*>(m_pDXView);}
	//	получить сэмпл звук 
	Widget::Player::Sample* RainSound(void) {return m_RainSound;}
	void PlayAmbientSound(void);
	void StopAmbientSound(void);
public:
	//	информация о ключах
	bool IsLevelKeyPresent(void) const;
private:
	void OnConsole(Widget* pChild);
public:
	//	проверка соответсвия видео режима текущему состоянию игры
	enum CHECK_VIDEO_MODE {CVM_FOR_MAINMENU,CVM_FOR_CFG};
	static void CheckVideoMode(CHECK_VIDEO_MODE cvm);
public:
	static MenuMgr *Instance(void);
};

inline MenuMgr* MenuMgr::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new MenuMgr();
	return m_Deleter.m_pInstance;
}
//	заблоктрован ли ввод на интерфейсе
inline bool MenuMgr::IsLockInput(void)
{
	return m_bLockInput;
}
//	выдать указатель на Tip'сину
inline MenuMgr::ic_Tip* MenuMgr::Tip(void)
{
	return m_Tip;
}
//	вернуть указатель на LoadLevelModifier
inline MenuMgr::ic_LoadLevelModifier* MenuMgr::LoadLevelModifier(void)
{
	return m_LoadLevelModifier;
}
//	вернуть указатель на LoadSaveGameModifier
inline MenuMgr::ic_LoadSaveGameModifier* MenuMgr::LoadSaveGameModifier(void)
{
	return m_LoadSaveGameModifier;
}
//	получить элемент отображения версии игры
inline Text* MenuMgr::Version(void) const
{
	return m_Version;
}

//***************************************************************//
//	class MenuMgr::ic_Tip 
class MenuMgr::ic_Tip
{
private:
	static const int m_TieX;
	static const int m_TieY;
	Text* m_Text;
public:
	ic_Tip(Widget* pParent);
	virtual ~ic_Tip();
public:
	void Activate(bool on);
	void SetColor(unsigned int color);
	void MoveTo(int x,int y);
	void SetText(const char* text);
};

//***************************************************************//
//	class MenuMgr::ic_State 
class MenuMgr::ic_State
{
public:
	enum MODE {M_EXTERNAL,M_INTERNAL};
protected:
	MenuMgr* m_MenuMgr;
	bool m_Destroy;								//	флаг самоуничтожения
public:
	ic_State(MenuMgr* mm) : m_MenuMgr(mm),m_Destroy(false) {}
	virtual ~ic_State() {}
public:
	//	передача управления от дочерних окон
	virtual bool OnChange(Widget* pChild) = 0;
	//	передача управления 
	virtual void OnTick(void) = 0;
	//	обновление прогресс бара
	virtual void UpdateLoadingProgressBar(float value) = 0;
	//	установить заголовок прогресc бара ЗАГРУЗКИ УРОВНЯ
	virtual void SetProgressBarTitle(const char* title) = 0;
	//	возвращаем режим работы меню
	virtual MenuMgr::ic_State::MODE Mode(void) = 0;
	//	загружаем уровень
	virtual void LoadLevel(const char* name) = 0;
public:
	bool IsDestroy(void) {return m_Destroy;}
};

//***************************************************************//
//	class MenuMgr::ic_LoadLevelModifier 
class MenuMgr::ic_LoadLevelModifier
{
private:
	std::string m_LevelName;
	bool m_Available;								//	флаг доступности
public:
	ic_LoadLevelModifier() : m_Available(false) {}
	virtual ~ic_LoadLevelModifier() {}
public:
	void SetLevelName(const char* level_name) {m_LevelName = level_name;}
	void SetAvailable(bool available) {m_Available = available;}
	const std::string& LevelName(void) const {return m_LevelName;}
	bool IsAvailable(void) const {return m_Available;}
};

//***************************************************************//
//	class MenuMgr::ic_LoadSaveGameModifier 
class MenuMgr::ic_LoadSaveGameModifier
{
public:
	enum AVAILABLE {A_NONE,A_LOAD,A_SAVE};
private:
	std::string m_SaveGameName;
	std::string m_SaveGameDesc;
	AVAILABLE m_Available;								//	флаг доступа
public:
	ic_LoadSaveGameModifier() : m_Available(A_NONE) {}
	virtual ~ic_LoadSaveGameModifier() {}
public:
	void SetSaveGameName(const char* name) {m_SaveGameName = name;}
	void SetSaveGameDesc(const char* desc) {m_SaveGameDesc = desc;}
	void SetAvailable(AVAILABLE available) {m_Available = available;}
	const std::string& SaveGameName(void) const {return m_SaveGameName;}
	const std::string& SaveGameDesc(void) const {return m_SaveGameDesc;}
	AVAILABLE Available(void) const {return m_Available;}
};

//***************************************************************//
//	class MenuMgr::ic_MouseCursor
class MenuMgr::ic_MouseCursor
{
	class Params;
public:
	enum CURSOR_TYPE {CT_NORMAL,
					  CT_CANMOVE,
					  CT_CANNOTMOVE,
					  CT_CANATTACK,
					  CT_CANNOTATTACK,
					  CT_CANUSE,
					  CT_SELECT,
					  CT_UNLOAD,
					  CT_SHIPMENT,
					  CT_SPECIAL,
					  CT_PICKUP,
					  CT_MEDIC = CT_PICKUP,
					  MAX_CURSORS = CT_MEDIC+1};
	struct TextField
	{
		const char* m_Text;
		unsigned int m_Color;
		TextField() : m_Text(0),m_Color(0) {}
		TextField(const char* text,unsigned int color) : m_Text(text),m_Color(color) {}
	};
	enum {MAX_TEXT_FIELDS = 3};
private:
	struct CursorInfo
	{
		std::string m_ImageName;		//	имя картинки
		int m_Width;					//	ширина 
		int m_Height;					//	высота
		int m_LinkX;					//	точка привязки 
		int m_LinkY;					//	точка привязки
		ipnt2_t m_TextXY[3];			//	точки привязки для текста
	};
private:
	static const char* m_CursorsPath;
	LPDIRECTDRAWSURFACE7 m_Surface;
	CursorInfo m_Cursors[MAX_CURSORS];
	RECT m_rcBorder;
	RECT m_rcCursor;
	int m_CursorX;
	int m_CursorY;
	bool m_Visible;
//	CURSOR_TYPE m_Type;
	Text* m_TextFields[MAX_TEXT_FIELDS];
	ipnt2_t m_TextFieldSize;
	std::string m_Font;
	enum {P_OLD,P_NEW,P_MAX};
	Params* m_Params[P_MAX];
public:
	ic_MouseCursor();
	virtual ~ic_MouseCursor();
public:
	//	установить позицию курсора
	void SetPos(int x,int y);
	//	установить границы действия курсора
	void SetBorder(int left,int top,int right,int bottom);
	//	установить тип курсора и текстовые поля
	void SetCursor(CURSOR_TYPE type,
				   const TextField* t1 = 0,
				   const TextField* t2 = 0,
				   const TextField* t3 = 0);
	//	уведомление о смене видео режима
	void OnChangeVideoMode(void);
	//	отрисовать курсор
	void Render(void);
	//	видимость курсора
	void SetVisible(bool visible);
private:
	void TextMoveTo(Widget* widget,int x,int y);
};

#endif	//_MENU_MGR_H_