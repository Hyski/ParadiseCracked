/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   26.03.2001

************************************************************************/
#ifndef _EDIT_H_
#define _EDIT_H_

#include "Text.h"

class Edit : public Text
{
public:
	class Controller;
private:
	class Caret;
	Caret* m_Caret;				//	каретка
	Controller* m_Controller;
private:
	bool m_InsertMode;			//	включена/выключена вставка символов
public:
	Edit(const char *pName);
	virtual ~Edit();
public:
	//	параметры
	struct Traits
	{
		const char* m_CaretImageName;
	};
	//	проинициализировать параметры
	virtual void SetTraits(const Traits& t);
public:
	//	задать максимальное кол-во символов
	virtual void SetLength(int symbols);
	//	задать прямоугольник вывода текста
	virtual void SetRegion(int left,int top,int right,int bottom);
	//	установить шрифт
	virtual void SetFont(const char* name,int color);
	//	устнановить текст
	virtual void SetText(const char* text);
	//	выровнять текст
	virtual void Align(int align);
	//	установить режим ввода текста
	virtual void SetInsertMode(bool insert_mode);
	//	переместить курсор в определенную позицию
	enum SEEK {S_BEGIN,S_CURRENT,S_END};
	void SeekCaret(int pos,SEEK seek);
	//	получить позицию курсора в символах
	unsigned int GetCaretPos(void);
	//	установить контроллер
	void SetController(Controller* controller);
private:
	virtual void OnSystemChange(void);
	//	обработка мышинного ввода
	virtual bool ProcessMouseInput(VMouseState* pMS);
	//	обработка клавиатурного ввода
	virtual bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	обработка потери клавиатурного ввода
	virtual bool ProcessKeyboardLost(void);
private:
	void InsertCharToText(unsigned char ch,int pos);
	void DeleteCharFromText(int pos);
	void MoveCaret(int dx,int new_width);
	//	переместить каретку в заданную позицию с проверкой
	void MoveCaretTo(int pos);
public:
	virtual const char* Type(void) const {return "edit";};
};

//***********************************************************************//
//**********************    class Edit::Caret     ***********************//
class Edit::Caret : public Widget
{
private:
	int m_SymMax;				//	длина в символах
	int m_SymPos;				//	позиция курсора в символах
	int m_SpaceWidth;			//	ширина пробела
	bool m_bActivated;			//	флаг активирования курсора
private:
	static const float m_CaretBlinkFreq;		//	частота мигания курсора
	static float m_BlinkSeconds;				//	текущее положение секунд
	static const int m_CaretHeight;				//	высота курсора в пикселах
public:
	Caret();
	virtual ~Caret();
public:
	//	установить размеры курсора
	void SetMovingLine(int top,int space_width);
	//	установить максимальное кол-во символов
	void SetMaxSym(int symbols);
	//	переместить каретку назад
	bool Reverse(void);
	//	переместить каретку вперед
	bool Forward(void);
	//	переместить каретку
	void SetPos(unsigned int pos);
	//	установить новую ширину каретки
	void SetNewWidth(int width);
	//	установить видимость курсора
	void Activate(bool bActivate);
public:
	int Pos(void){return m_SymPos;}
	int SpaceWidth(void){return m_SpaceWidth;}
	bool IsActivated(void){return m_bActivated;}
public:
	//	подмигивание курсора
	bool Tick(void);
};

//***********************************************************************//
//	class Edit::Controller
class Edit::Controller
{
public:
	virtual ~Controller() {}
	virtual void OnReturn(void) = 0;
	virtual void OnBackspace(void) = 0;
	virtual void OnDelete(void) = 0;
	virtual void OnLeft(void) = 0;
	virtual void OnRight(void) = 0;
};

#endif	//_EDIT_H_