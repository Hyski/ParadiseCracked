/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   20.03.2001

************************************************************************/
#ifndef _LOADING_SCREEN_H_
#define _LOADING_SCREEN_H_

#include "Dialog.h"

class Static;
class Text;

class LoadingScreen : public Dialog
{
	class ImageOscillator;
public:
	static const char* m_pDialogName;
	enum STEP {S_PREPARE,S_READY_FOR_LOADING,S_LOADING_DONE,S_QUITE};
private:
	Widget* m_Progress;
	Text* m_Title;
	int m_ProgressWidth;
	STEP m_Step;
public:
	LoadingScreen();
	virtual ~LoadingScreen();
public:
	bool Tick(void);
	//	отображение виджета
	void Draw(WidgetView* pView);
public:
	//	обновление прогресс бара
	void Update(float value);
	//	установить заголовок прогресс бара
	void SetTitle(const char* title);
	//	получение ступени действия
	LoadingScreen::STEP Step(void) const;
};

inline LoadingScreen::STEP LoadingScreen::Step(void) const
{
	return m_Step;
}

//**********************************************************************//
//*********	          class HiddenMovementScreen           *************//
class HiddenMovementScreen : public Dialog
{
public:
	static const char* m_pDialogName;
	enum BELONG {B_CIVILIAN,B_ENEMY};			//	принадлежность
	static bool m_DebugMode;
private:
	Static* m_Background;
	Static* m_ProgressImage;
	Widget* m_Progress;
	int m_ProgressWidth;
	int m_ipb;									//	image per belong
public:
	HiddenMovementScreen();
	virtual ~HiddenMovementScreen();
public:
	//	сменить картинку
	void SetBackground(BELONG belong);
	// обновить прогресс бар
	void Update(float value);
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
	//	отображение виджета и всех дочерних виджетов
	void Render(WidgetView* pView);
};

//**********************************************************************//
//	class PanicScreen
class PanicScreen : public Dialog
{
public:
	static const char* m_pDialogName;
private:
	Text* m_Content;
public:
	PanicScreen();
	virtual ~PanicScreen();
public:
	//	установить текст описания
	void SetContent(const char* text);
private:
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
};

//**********************************************************************//
//	class ScriptSceneScreen
class ScriptSceneScreen : public Dialog
{
	class Curtain;
public:
	static const char* m_pDialogName;
private:
	Curtain* m_Top;
	Curtain* m_Bottom;
public:
	ScriptSceneScreen();
	virtual ~ScriptSceneScreen();
public:
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
};

//**********************************************************************//
//	class ScriptSceneScreen::Curtain
class ScriptSceneScreen::Curtain : public Widget
{
private:
	static const float m_PerCent;				//	проценты от занимаемой площади экрана
public: 
	enum TYPE {T_TOP,T_BOTTOM};
private:
	TYPE m_Type;
	float m_StartTime;
public:
	Curtain(const char* pName);
	virtual ~Curtain();
public:
	//	типизировать
	void Typify(TYPE type);
	//	привести в начальное состояние
	void OnStart(void);
	//	обновить состояние
	void OnUpdate(void);
private:
	void ClearSurface(LPDIRECTDRAWSURFACE7 surface);
};


#endif	//_LOADING_SCREEN_H_