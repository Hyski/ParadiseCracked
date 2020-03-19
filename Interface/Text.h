/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   07.03.2001

************************************************************************/
#ifndef _TEXT_H_
#define _TEXT_H_

#include "../Common/UI/Widget.h"
#include "../Common/FontMgr/LogicalText.h"

class Text : public Widget
{
private:
	int m_ImageWidth;
	int m_ImageHeight;
protected:
	LogicalText m_Text;
	int m_Aling;
	std::string m_FontName;
	int m_FontColor;
public:
	Text(const char *pName);
	virtual ~Text();
public:
	//	задать прямоугольник вывода текста (new_image - создавать новую картинку в любом случае)
	virtual void SetRegion(int left,int top,int right,int bottom,bool new_image = false);
	//	установить шрифт
	virtual void SetFont(const char* name,int color);
	//	устнановить текст
	virtual void SetText(const char* text);
	//	получить текст
	virtual const std::string& GetText(void);
	//	получить логический (разобранный) текст
	virtual const LogicalText& GetLogicalText(void) const;
	//	получить фонт
	virtual const Font* GetFont(void);
	//	получить наименование фонта
	virtual const char* GetFontName(void) const;
	//	получить цвет шрифта
	virtual int GetFontColor(void) const;
	//	выровнять текст
	virtual void Align(int align);
	//	получить значение выравнивания текста
	virtual int GetAlign(void) const;
	//	обновить текст (например если поменялся шрифт)
	virtual void Update(void);
	//	получить ширину текста
	int Width(void) const;
	//	получить высоту текста
	int Height(void) const;
public:
	virtual void OnSystemChange(void);
protected:
	void ClearSurface(LPDIRECTDRAWSURFACE7 surface);
public:
	virtual const char* Type(void) const {return "text";};
};

//	получить высоту текста
inline int Text::Height(void) const
{
	return m_Text.Height();
}
//	получить ширину текста
inline int Text::Width(void) const
{
	return m_Text.Width();
}
//	получить текст
inline const std::string& Text::GetText(void)
{
	return m_Text.SourceText();
}
//	получить логический (разобранный) текст
inline const LogicalText& Text::GetLogicalText(void) const
{
	return m_Text;
}
//	получить наименование фонта
inline const char* Text::GetFontName(void) const
{
	return m_FontName.c_str();
}
//	получить цвет шрифта
inline int Text::GetFontColor(void) const
{
	return m_FontColor;
}
//	получить значение выравнивания текста
inline int Text::GetAlign(void) const
{
	return m_Aling;
}

#endif	//_TEXT_H_