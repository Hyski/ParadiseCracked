/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   21.08.2000

************************************************************************/
#ifndef _EL_TEXT_H_
#define _EL_TEXT_H_

#include "../Lagoon.h"
#include "../../Common/FontMgr/FormattedText.h"
#include "../../Common/TextureMgr/D3DTexture.h"
#include "../../Common/FontMgr/VirtualTexture.h"
#include "../../Common/FontMgr/FontMgr.h"
#include "../../Common/FontMgr/GFont.h"

class elText : public Lagoon
{
public:
	enum TEXT_HALIGN	{THA_LEFT,
						 THA_RIGHT,
						 THA_CENTER,
						 THA_WIDE
	                    };
	enum TEXT_VALIGN	{TVA_TOP,
						 TVA_BOTTOM,
						 TVA_CENTER
	                    };
protected:
	unsigned int m_uiHAlign;				//	режим выравнивания текста
	unsigned int m_uiVAlign;				//	режим выравнивания текста
protected:
	//	для вывода графики
	D3DTLVERTEX m_d3dTLVertex[4];		//	треугольники
	D3DTexture m_d3dTexture;			//	текстура под текст
	VirtualTexture m_vTexture;			//	виртуальная текстура
	DWORD m_dwTextColor;				//	цвет текста
protected:
	std::string m_sText;				//	выводимый текст
	FormattedText m_fText;				//	форматированный текст
	std::string m_sFont;				//	имя шрифта
	bool m_bTextResolved;				//	флаг 
public:
	int m_iOffsetY;						//  смещение по Y
public:
	elText(const char *pName);
	virtual ~elText();
protected:
	virtual void CreateTexture(void);
public:
	void SetFont(const char *pFont);
	void SetText(const char *pText);
	void SetTextColor(DWORD dwTextColor);
	void SetTextHAlign(TEXT_HALIGN ha);
	void SetTextVAlign(TEXT_VALIGN va);
	void AddText(const char *pText);
public:
	unsigned int GetHeight(void);			//	получить высоту форматированного текста (виртуальные координаты)
public:
	//	функции изменения размеров (виртуальные координаты)
//	virtual void SetRect(RECT *pRect);
//	virtual void SetRect(int left,int top,int right,int bottom);
	virtual void SetRegion(HRGN hRegion);
	//	функция подстройки
	virtual void Tune(void);
	//	функция отображения
	virtual void Render(GraphPipe *lpGraphPipe);
	//	функции изменения местоположение (виртуальные координаты)
	virtual void MoveTo(int x,int y);
public:
	void TextBlit(void);
	void ResolveText(void);
	void ClearSurface(void);
public:
	std::string& GetFontName(void);
	std::string& GetText(void);
	unsigned int GetStringNum(void);
};

inline std::string& elText::GetFontName(void)
{
	return m_sFont;
}

//	получить высоту форматированного текста (виртуальные координаты)
inline unsigned int elText::GetHeight(void)
{
	return m_fText.m_vFS.size()*FontMgr::Instance()->GetGFont(m_sFont.c_str())->Height();
}

inline unsigned int elText::GetStringNum(void)
{
	return m_fText.m_vFS.size();
}

inline std::string& elText::GetText(void)
{
	return m_sText;
}

#endif	//_EL_TEXT_H_