/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: менеджер загрузки шрифтов.
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   14.06.2000

************************************************************************/
#ifndef _FONT_MGR_H_
#define _FONT_MGR_H_

class Font2D;
class GFont;
class FastFont;

class FontMgr
{
private:
	struct Deleter 
	{
	public:
		FontMgr *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	std::map<std::string,Font2D*> m_mFont2Ds;		//	карта двухмерных шрифтов
//	возможно в будущем этого не будет
	std::map<std::string,GFont*> m_mGFonts;			//	карта трехмерного медленного шрифта
	std::map<std::string,FastFont*> m_mFastFonts;	//	карта трехмерного быстрого шрифта
private:
	const char *m_pFontPath;
	const char *m_pFontExt;
private:
	FontMgr();
	virtual ~FontMgr();
public:
	//	интерфейс
	Font2D* GetFont2D(const char *pName,int color);
	//	уничтожить экземпл€р ассоцированный с именем
	void Release(const char *pName);
	//	уничтожить все
	void Release(void);
public:	//	возможно в будущем этого не будет
//	void LoadFont(LPDIRECT3DDEVICE7 lpd3dDevice,const char* pFontFileName);
	GFont* GetGFont(const char* pName);
	//	------------------------------------------------------------------
//	void LoadFastFont(LPDIRECT3DDEVICE7 lpd3dDevice,const char* pFontFileName);
	FastFont* GetFastFont(const char* pName);
public:
	static FontMgr *Instance(void);
};

inline FontMgr* FontMgr::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new FontMgr();
	return m_Deleter.m_pInstance;
}





/*
#include "GFont.h"
//#include "FastFont.h"

namespace FontMgr
{
	void LoadFont(LPDIRECT3DDEVICE7 lpd3dDevice,const char* pFontFileName);
	GFont& Font(const char* pFontFileName);
	//	------------------------------------------------------------------
//	void LoadFastFont(LPDIRECT3DDEVICE7 lpd3dDevice,const char* pFontFileName);
//	FastFont& FFont(const char* pFontFileName);
	//	------------------------------------------------------------------
	void Tune(LPDIRECT3DDEVICE7 lpd3dDevice);
}
*/
#endif	//_FONT_MGR_H_