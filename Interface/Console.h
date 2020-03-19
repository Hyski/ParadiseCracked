/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description: игровая консоль имеет в своем составе окно для 
	вывода информации и строку ввода для ввода информации.
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   04.04.2001

************************************************************************/
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "Dialog.h"

class Edit;
class TextBox;

class Console : public Dialog
{
public:
	class Controller;
	static const char* m_pDialogName;
	enum LAST_EVENT 
	{
		LE_NONE,				//	ничего
		LE_ACTIVATE				//	хочет получить фокус ввода
	};
private:
	//	сам объект консоли 
	static Console* m_Instance;
	//	флаг присутсвия консоли в игре
	static bool m_IsAvailable;
private:
	//	последнее событие от консоли
	LAST_EVENT m_LastEvent;
	//	окно ввода текста
	Edit* m_CommandLine;
	//	окно вывода текста
	TextBox* m_Content;
	//	массив контроллеров
	std::vector<Controller*> m_Controllers;
	//	флаг того что окошко перетаскивается вслед за мышью
	bool m_MouseCapture;
	int m_xMouse,m_yMouse;
private:
	Console();
public:
	virtual ~Console();
public:	//	интерфейс инициализации консоли
	//	создать консоль
	static Console* SetAvailable(void); 
	//	проверить присутствует ли консоль
	static bool IsAvailable(void);
public:	//	интерфейс использования консоли
	//	добавить строку в окно вывода консоли
	static void AddString(const char* string);
	//	получить строку текста из окна ввода консоли
	static const std::string& GetCommandLine(void);
	//	получить строку текста из окна ввода консоли как список аргументов
	static int GetCommandLine(std::vector<std::string>& argv);
	//	установить текст в окно ввода консоли
	static void SetCommandLine(const char* string);
	//	подписаться на события ввода от консоли
	static void AddController(Controller* controller);
	//	убрать контроллер на события ввода от консоли
	static void RemoveController(Controller* controller);
	//	высветить консоль на дисплей
	static void Show(bool show);
	//	высвечена ли консоль на экране
	static bool IsOnScreen(void);
	//	получить последнее событие от консоли
	static Console::LAST_EVENT LastEvent(void);
public:
	//	обработка мышинного ввода
	bool ProcessMouseInput(VMouseState* pMS);
	//	обработка потери клавиатурного ввода
	bool ProcessKeyboardLost(void);
	//	потеря ввода от клавиатуры
	void OnKeyboardLost(void);
	//	обработка системного сообщения
	void OnSystemChange(void);
	//	обработка событий от дочерних меню
	void OnChange(Widget* pChild);
	//	установка видимости
	void SetVisible(bool bVisible);
	//	производим какие-то действия
	bool Tick(void);
private:
	//	стартовая информация выводимая в консоль
	void Starting(void);
};

inline bool Console::IsAvailable(void) 
{
	return m_IsAvailable;
}

//***************************************************************************//
//********************       class Console::Controller       ****************//
class Console::Controller
{
public:
	Controller(){}
	virtual ~Controller(){}
public:
	virtual void Activate(void) = 0;
};



#endif	//_CONSOLE_H_