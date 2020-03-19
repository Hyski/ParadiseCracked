/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   13.03.2001

************************************************************************/
#ifndef _JOURNAL_SCREEN_H_
#define _JOURNAL_SCREEN_H_

#include "Dialog.h"

class NavigationSheet;
class DatabaseSheet;
class JournalSheet;
class NewsSheet;
class MultiEdit;
class TextBox;
class Text;
class Edit;

extern const char* g_TechPictures;
extern const char* g_ForcesPictures;
extern const char* g_PeoplePictures;
extern const char* g_LocationPictures;

class JournalScreen : public Dialog
{
public:
	class SheetController;
	class HotLineControl;
	class Controller;
	class Sheet;
	//	наименования закладок
	enum SCREEN_ID
	{
		SID_NONE,
		SID_NEWS,						//	новости
		SID_DATABASE,					//	база данных
		SID_JOURNAL,					//	дневник
		SID_NAVIGATION					//	навигация
	};
public:
	static const char* m_pDialogName;
private:
	Controller*			m_Controller;
	HotLineControl*		m_HotLine;
	NewsSheet*			m_News;
	DatabaseSheet*		m_Database;
	JournalSheet*		m_Journal;
	NavigationSheet*	m_Navigation;
	Sheet*				m_ActiveSheet;
	Text*				m_HLWidget;
public:
	JournalScreen();
	virtual ~JournalScreen();
public:
	//	активизировать определенный экран
	void Activate(SCREEN_ID id); 
	//	получить интерфейс с экраном новостей
	NewsSheet* News(void);
	//	получить интерфейс с экраном энциклопедии
	DatabaseSheet* Database(void);
	//	получить интерфейс с экраном дневника
	JournalSheet* Journal(void);
	//	получить интерфейс с экраном навигации
	NavigationSheet* Navigation(void);
public:
	//	установить контроллер
	void SetController(Controller* controller);
	//	список кнопок 
	enum BUTTON
	{
		B_UP,
		B_DOWN,
		B_LEFT,
		B_RIGHT,
		B_BACK,
		MAX_BUTTON
	};
	//	список кнопок на обновление
	enum UPDATE_BUTTON
	{
		UB_DATABASE,
		UB_JOURNAL,
		UB_NAVIGATION,
		MAX_UPDATE_BUTTON
	};
	//	список закладок
	enum SHEET
	{
		S_DATABASE,					//	база данных
		S_JOURNAL,					//	дневник
		S_NAVIGATION,				//	навигация
		MAX_SHEET,
		S_NEWS						//	новости
	};
	struct FixedStatic 
	{
		Static* m_Static;
		bool m_Fixed;
		FixedStatic() : m_Static(0),m_Fixed(false) {}
	};
	//	активировать кнопку обновления
	enum ACTIVATE_MODE {AM_ACTIVATE,AM_DEACTIVATE};
	void ActivateUpdateButton(UPDATE_BUTTON id,ACTIVATE_MODE mode);
	//	подсветить кнопку переключения закладок
	void IlluminateSheet(SHEET id);
	//	разрешить/запретить доступ до элементов управления
	void EnableControls(bool enable);
	//	установить видимость виджета
	void SetVisible(bool bVisible);
	//	установить линию горячей подсказки
	void SetHotLineText(const char* text);
	//	вернуть интерфейс HotLine
	HotLineControl* HotLine(void) {return m_HotLine;}
private:
	Button* m_Buttons[MAX_BUTTON];
	FixedStatic m_Sheets[MAX_SHEET];
	FixedStatic m_UpdateButtons[MAX_UPDATE_BUTTON];
private:
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
	//	уведомление об изменении состояния кнопок
	void OnButton(Button *pButton);
	//	уведомление об изменении состояния статиков
	void OnStatic(Static *pStatic);
	//	обработка клавиатурного ввода
	bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	обработка мышинного ввода
	bool ProcessMouseInput(VMouseState* pMS);
};

//*********************************************************************//
//	class JournalScreen::Controller
class JournalScreen::Controller
{
public:
	virtual ~Controller() {}
	virtual void OnSheetClick(JournalScreen::SHEET id) = 0;
	virtual void OnUpdateClick(JournalScreen::UPDATE_BUTTON id) = 0;
	virtual void OnBack(void) = 0;
};

//*********************************************************************//
//	class JournalScreen::SheetController
class JournalScreen::SheetController
{
public:
	virtual ~SheetController() {}
	virtual void OnButtonClick(JournalScreen::BUTTON id) = 0;
};

//*********************************************************************//
//	class JournalScreen::Sheet
class JournalScreen::Sheet : public Dialog
{
protected:
	JournalScreen* m_JournalScreen;
public:
	Sheet(JournalScreen* js,const char *pName,const char *pScriptName) : Dialog(pName,pScriptName),m_JournalScreen(js) {}
	virtual ~Sheet() {}
public:
	virtual SheetController* GetController(void) = 0;
	virtual std::string CheckHotLine(void) = 0;
protected:
	//	уведомление об изменении состояния системы
	virtual void OnSystemChange(void) {Widget::OnSystemChange();}
	//	уведомление о потере фокуса клавиатуры
	virtual void OnKeyboardLost(void) {}
};

//*********************************************************************//
//	class NewsSheet
class NewsSheet : public JournalScreen::Sheet
{
public:
	static const char* m_pDialogName;
	struct Traits
	{
		const char* m_Image;		//	системное имя картинки
		const char* m_Channel;		//	наименование канала
		const char* m_Content;		//	содержимое новостей
	};
private:
	JournalScreen::SheetController* m_Controller;
	VRegion m_ImagePlace;
	Widget* m_Image;
	Text* m_Channel;
	TextBox* m_Content;
public:
	NewsSheet(JournalScreen* js);
	virtual ~NewsSheet();
public:
	void SetController(JournalScreen::SheetController* controller);
	void SetTraits(const Traits* t);
private:
	JournalScreen::SheetController* GetController(void);
	std::string CheckHotLine(void);
};

//*********************************************************************//
//	class DatabaseSheet
class DatabaseSheet : public JournalScreen::Sheet
{
public:
	class Controller;
	enum SHEET
	{
		S_OUTFIT,
		S_TECH,
		S_FORCES,
		S_PEOPLE,
		MAX_SHEET
	};
	struct ItemInfo
	{
		const char* m_Image;
		const char* m_Name;
		const char* m_LeftColumn;
		const char* m_RightColumn;
		const char* m_Text;
	};
public:
	static const char* m_pDialogName;
private:
	Controller* m_Controller;
	Widget* m_Image;
	Text* m_LeftColumn;
	Text* m_RightColumn;
	Text* m_ItemName;
	TextBox* m_Text;
public:
	DatabaseSheet(JournalScreen* js);
	virtual ~DatabaseSheet();
public:
	void SetController(Controller* controller);
	//	подсветить кнопку переключения закладок
	void IlluminateSheet(SHEET id);
	//	разрешить/запретить доступ к закладке
	void EnableSheet(SHEET id,bool enable);
	//	установить информацию для итемов (0 - все спрятать,sheet - для определения каталога с картинками)
	void SetItemInfo(const ItemInfo* ii,SHEET id = S_OUTFIT);
	//	установить наименования раздела
	void SetSectionName(const char* name);
private:
	JournalScreen::FixedStatic m_Sheets[MAX_SHEET];
private:
	JournalScreen::SheetController* GetController(void);
	//	получить каталог с картинками для данной закладки
	const char* GetPathForResource(SHEET id) const;
	//	уведомление об изменении состояния статиков
	void OnStatic(Static *pStatic);
	//	проверка горячей линии
	std::string CheckHotLine(void);
};

//*********************************************************************//
//	class DatabaseSheet::Controller
class DatabaseSheet::Controller : public JournalScreen::SheetController
{
public:
	virtual ~Controller() {}
	virtual void OnSheetClick(DatabaseSheet::SHEET id) = 0;
};

//*********************************************************************//
//	class JournalSheet
class JournalSheet : public JournalScreen::Sheet
{
public:
	class Controller;
	enum SHEET
	{
		S_NEWTHEME,
		S_DELTHEME,
		S_NEWENTRY,
		S_DELENTRY,
		MAX_SHEET,
		S_SAVE_THEME,
		S_SAVE_ENTRY
	};
public:
	static const char* m_pDialogName;
private:
	Controller* m_Controller;
	Text* m_ThemeName;
	Edit* m_ThemeEdit;
	TextBox* m_Content;
	MultiEdit* m_ContentEdit;
public:
	JournalSheet(JournalScreen* js);
	virtual ~JournalSheet();
public:
	//	установить контроллер
	void SetController(Controller* controller);
	//	установить текстовое поле
	enum TEXT_FIELD {TF_THEME,TF_THEME_COUNT,TF_ENTRY_COUNT,TF_CONTENT};
	void SetTextField(const char* text,TEXT_FIELD tf);
	//	начать создание темы или записи
	enum SECTION {S_THEME,S_ENTRY};
	void EditSection(SECTION section,const char* text);
	//	узнать содержимое полей редактирования
	const char* GetSectionText(SECTION section) const; 
	//	определить режим редактирования
	bool IsEdit(void) const;
	//	прервать редактирование
	enum CONTINUE_MODE {CM_OK,CM_CANCEL};
	void ContinueEdit(CONTINUE_MODE cm);
	//	снять выделение с кнопки
	void UnSelectSheet(SHEET id);
	//	разрешить/запретить доступ к закладке
	void EnableSheet(SHEET id,bool enable);
private:
	struct FixedStaticEx : public JournalScreen::FixedStatic
	{
		bool m_Enable;	//	этот флаг означает что кнопки не серые/серые
		FixedStaticEx() : FixedStatic(),m_Enable(true) {}
	};
	FixedStaticEx m_Sheets[MAX_SHEET];
	enum SPECIAL_BUTTON {SB_OK,SB_CANCEL,MAX_SPECIAL_BUTTON};
	JournalScreen::FixedStatic m_SButtons[MAX_SPECIAL_BUTTON];
private:
	JournalScreen::SheetController* GetController(void);
	//	уведомление об изменении состояния статиков
	void OnStatic(Static *pStatic);
	//	уведомление об изменении состояния
	void OnChange(Widget* pChild);
	//	разрешить/запретить доступ до элементов управления
	void EnableControls(bool enable);
	//	проверка горячей линии
	std::string CheckHotLine(void);
};

//*********************************************************************//
//	class JournalSheet::Controller
class JournalSheet::Controller : public JournalScreen::SheetController
{
public:
	virtual ~Controller() {}
	virtual void OnSheetClick(SHEET id) = 0;
};

//*********************************************************************//
//	class NavigationSheet
class NavigationSheet : public JournalScreen::Sheet
{
	class Flag;
public:
	class Controller;
	enum SHEET
	{
		S_LOCALMAP,
		S_CITYMAP,
		MAX_SHEET
	};
public:
	static const char* m_pDialogName;
	static const char* m_LevelImagePath;
	static const char* m_FlagsScript;
	static const char* m_EpisodeMapDialogName;
	static const char* m_EpisodeMapScriptName;
private:
	Controller* m_Controller;
	Text* m_LevelName;
	Widget* m_LevelImage;
	std::string m_CurrentLevel;
	Dialog* m_EpisodeMap;
	unsigned int m_Episode;
public:
	NavigationSheet(JournalScreen* js);
	virtual ~NavigationSheet();
public:
	void SetController(Controller* controller);
	//	подсветить кнопку переключения закладок
	void IlluminateSheet(SHEET id);
	//	показать ту или иную закладку
	void ShowSheet(SHEET id);
	//	установить текущий эпизод (считывается определенный скрипт)
	void SetEpisode(unsigned int episode);
	//	показать кусочек карты
	enum TYPE {T_NORMAL,T_SELECTED};
	void ShowEpisodePart(const char* level,TYPE type);
	//	очистить всю карту эпизода
	void ClearEpisodeMap(void);
private:
	JournalScreen::FixedStatic m_Sheets[MAX_SHEET];
private:
	JournalScreen::SheetController* GetController(void);
	//	уведомление об изменении состояния статиков
	void OnStatic(Static *pStatic);
	//	создаем карту уровня
	void CreateLevelMap(const char* level);
	//	установить видимость виджета
	void SetVisible(bool bVisible);
	//	проверка горячей линии
	std::string CheckHotLine(void);
	//	обработка мышинного ввода
	bool ProcessMouseInput(VMouseState* pMS);
	//	получить наименование эпизода по номеру эпизода
	std::string NavigationSheet::GetEpisodeName(unsigned int episode) const;
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
};

//*********************************************************************//
//	class NavigationSheet::Controller
class NavigationSheet::Controller : public JournalScreen::SheetController
{
public:
	virtual ~Controller() {}
	virtual void OnSheetClick(SHEET id) = 0;
};

#endif	//_JOURNAL_SCREEN_H_