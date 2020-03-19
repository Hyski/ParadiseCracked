/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.03.2001

************************************************************************/
#ifndef _TEXT_BOX_H_
#define _TEXT_BOX_H_

#include "Text.h"

class Static;

class TextBox : public Text
{
	class Strategy;
	class NormalStrategy;
	class ConsoleStrategy;
	friend NormalStrategy;
	friend ConsoleStrategy;
private:
	static const float m_Delay;		//	время задержки
	Static* m_pUp;
	Static* m_pDown;
	int m_ScrollStep;			//	шаг смещения текста
	int m_TextHeight;			//	высота текста
	int m_TextOffset;			//	смещение текста сверху
	Strategy* m_Strategy;		//	стратегия поведения
public:
	TextBox(const char *pName);
	virtual ~TextBox();
public:
	//	инициализация 
	struct ScrollTraits
	{
		const char* m_UpImageName;
		const char* m_DownImageName;
		int m_Width;
		int m_Height;
	};
	virtual void SetScroll(const ScrollTraits& st);
	virtual void SetRegion(int left,int top,int right,int bottom);
	//	установить шрифт
	virtual void SetFont(const char* name,int color);
	//	установить текст
	virtual void SetText(const char* text);
	virtual void AddText(const char* text);
	virtual void Clear(void);
	//	пролистать текст 
	enum SCROLL_DIRECTION {SD_UP,SD_DOWN,SD_BEGIN,SD_END};
	//	sd: тип скроллинга check_time: учитывать время
	virtual void Scroll(SCROLL_DIRECTION sd,bool check_time = false);
	virtual void Scroll(int pixel_count);
	virtual void ScrollTo(int offset);
	//	промотано до конца
	virtual bool IsScrollEnd(void) const;
	//	установить режим по-буквенного вывода текста
	enum MODE {M_NORMAL,M_CONSOLE};
	virtual void SetMode(MODE mode);
	//	завершить работу
	virtual void ToEnd(void);
private:
	//	для скроллинга
	SCROLL_DIRECTION m_PrevSD;	//	предыдущее состояние для скроллинга
	float m_LastScrollSeconds;	//	время засечки секунд
protected:
	virtual void OnSystemChange(void);
	//	сообщение от дочернего окна
	virtual void OnChange(Widget* pChild);
	//	обработка мышинного ввода
	virtual bool ProcessMouseInput(VMouseState* pMS);
	//	обработка клавиатурного ввода
	virtual bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	обработка потери мышиного ввода
	virtual bool ProcessMouseLost(void);
	//	обработка потери клавиатурного ввода
	virtual bool ProcessKeyboardLost(void);
	//	двигаем окно если требуется
	virtual bool Tick(void);
public:
	virtual const char* Type(void) const {return "textbox";};
private:
	//	обновить поверхность
	void UpdateSurface(void);
};

//**********************************************************************
//	class TextBox::Strategy
class TextBox::Strategy
{
protected:
	TextBox* m_FeedBack;
public:
	Strategy(TextBox* feed_back) : m_FeedBack(feed_back) {}
	virtual ~Strategy() {}
	virtual void UpdateSurface(void) = 0;
	virtual void SetText(const char* text) = 0;
	virtual void AddText(const char* text) = 0;
	virtual void Clear(void) = 0;
	virtual void Tick(void) = 0;
	virtual void FontChanged(void) = 0;
	virtual void OnSystemChange(void) = 0;
	virtual void OnScroll(int pixels) = 0;
	virtual void ToEnd(void) = 0;
};

#endif	//_TEXT_BOX_H_