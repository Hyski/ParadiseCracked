/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: менеджер загрузки шрифтов.
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   14.06.2000

************************************************************************/
#include "Precomp.h"
#include "Font2D.h"
#include "GFont.h"
#include "FastFont.h"
#include "FontMgr.h"

//---------- Ћог файл ------------
#if 0&&defined(_DEBUG_GRAPHICS)
CLog fontmgr_log;
#define fontmgr	fontmgr_log["fontmgr.log"]
#else
#define fontmgr	/##/
#endif
//--------------------------------

FontMgr::Deleter FontMgr::m_Deleter;

FontMgr::FontMgr() : m_pFontPath("fonts/"),m_pFontExt(".mlf")
{
}

FontMgr::~FontMgr()
{
	Release();
}

Font2D* FontMgr::GetFont2D(const char *pName,int color)
{
	static std::map<std::string,Font2D*>::iterator i;
	static std::string font_name;
	static char pBuff[10];

	font_name = pName;
	sprintf(pBuff,":%0.8x",color);
	font_name += pBuff;
	i = m_mFont2Ds.find(font_name);
	if(i == m_mFont2Ds.end())
	{
		fontmgr("Loading 2D font [%s];\n",std::string(std::string(m_pFontPath)+std::string(pName)+m_pFontExt).c_str());
		m_mFont2Ds.insert(std::map<std::string,Font2D*>::value_type(font_name,new Font2D(D3DKernel::GetPS(),std::string(std::string(m_pFontPath)+std::string(pName)+m_pFontExt).c_str(),color)));
		i = m_mFont2Ds.find(font_name);
		fontmgr("    ...font name: [%s];\n",font_name.c_str());
	}
	fontmgr("RETURN:    [%s];\n",font_name.c_str());

	return i->second;
}

GFont* FontMgr::GetGFont(const char* pName)
{
	static std::map<std::string,GFont*>::iterator i;
	static std::string font_name;

	font_name = m_pFontPath;
	font_name += pName;
	font_name += ".mlf";
	i = m_mGFonts.find(font_name);
	if(i == m_mGFonts.end())
	{
		fontmgr("Loading GFont font [%s];\n",std::string(std::string(m_pFontPath)+std::string(pName)+m_pFontExt).c_str());
		m_mGFonts.insert(std::map<std::string,GFont*>::value_type(font_name,new GFont()));
		m_mGFonts[font_name]->Load(D3DKernel::GetD3DDevice(),DataMgr::Load(font_name.c_str()));
		DataMgr::Release(font_name.c_str());
		i = m_mGFonts.find(font_name);
	}

	return i->second;
}

FastFont* FontMgr::GetFastFont(const char* pName)
{
	static std::map<std::string,FastFont*>::iterator i;
	static std::string font_name;

	font_name = m_pFontPath;
	font_name += pName;
	font_name += ".mlff";
	i = m_mFastFonts.find(font_name);
	if(i == m_mFastFonts.end())
	{
		fontmgr("Loading FastFont font [%s];\n",std::string(std::string(m_pFontPath)+std::string(pName)+m_pFontExt).c_str());
		m_mFastFonts.insert(std::map<std::string,FastFont*>::value_type(font_name,new FastFont()));
		m_mFastFonts[font_name]->Load(D3DKernel::GetD3DDevice(),DataMgr::Load(font_name.c_str()));
		DataMgr::Release(font_name.c_str());
		i = m_mFastFonts.find(font_name);
	}

	return i->second;
}

//	уничтожить экземпл€р ассоцированный с именем
void FontMgr::Release(const char *pName)
{
	std::map<std::string,Font2D*>::iterator i;

	i = m_mFont2Ds.find(pName);
	if(i!=m_mFont2Ds.end())
	{
		fontmgr("Unload font: 2D [%s];\n",pName);
		delete i->second;
		m_mFont2Ds.erase(i);
	}
}
//	уничтожить все
void FontMgr::Release(void)
{
	//	уничтожение 2D
	for(std::map<std::string,Font2D*>::iterator i = m_mFont2Ds.begin();i!=m_mFont2Ds.end();i++)
	{
		fontmgr("Unload 2D font: [%s];\n",i->first.c_str());
		delete i->second;
	}
	m_mFont2Ds.clear();
	//	уничтожение GFont
	for(std::map<std::string,GFont*>::iterator j = m_mGFonts.begin();j!=m_mGFonts.end();j++)
	{
		fontmgr("Unload GFont font: [%s];\n",j->first.c_str());
		delete j->second;
	}
	m_mGFonts.clear();
	//	уничтожение FastFont
	for(std::map<std::string,FastFont*>::iterator k = m_mFastFonts.begin();k!=m_mFastFonts.end();k++)
	{
		fontmgr("Unload FastFont font: [%s];\n",k->first.c_str());
		delete k->second;
	}
	m_mFastFonts.clear();
}

/*
namespace FontMgr
{
	std::map<std::string,GFont> m_FontMap;
//	std::map<std::string,FastFont> m_FastFontMap;
}

void FontMgr::LoadFont(LPDIRECT3DDEVICE7 lpd3dDevice,const char* pFontFileName)
{
	std::map<std::string,GFont>::iterator i;
	std::string font_name = std::string("fonts/")+std::string(pFontFileName)+std::string(".mlf");

	i = m_FontMap.find(font_name);
	if(i==m_FontMap.end())
	{
		m_FontMap[font_name].Load(lpd3dDevice,DataMgr::Load(font_name.c_str()));
		DataMgr::Release(font_name.c_str());
	}
}

GFont& FontMgr::Font(const char* pFontFileName)
{
	std::map<std::string,GFont>::iterator i;
	std::string font_name = std::string("fonts/")+std::string(pFontFileName)+std::string(".mlf");

	i = m_FontMap.find(font_name);
	if(i==m_FontMap.end())
	{
		LoadFont(D3DKernel::GetD3DDevice(),pFontFileName);
		i = m_FontMap.find(font_name);
		if(i==m_FontMap.end())
			throw CASUS(std::string("Ўрифт с именем <")+font_name+std::string("> не был загружен.\n"));
	}

	return i->second;
}*/
/*
void FontMgr::LoadFastFont(LPDIRECT3DDEVICE7 lpd3dDevice,const char* pFontFileName)
{
	std::map<std::string,FastFont>::iterator i;
	std::string font_name = std::string("fonts/")+std::string(pFontFileName)+std::string(".mlff");

	i = m_FastFontMap.find(font_name);
	if(i==m_FastFontMap.end())
	{
		m_FastFontMap[font_name].Load(lpd3dDevice,DataMgr::Load(font_name.c_str()));
		DataMgr::Release(font_name.c_str());
	}
}

FastFont& FontMgr::FFont(const char* pFontFileName)
{
	std::map<std::string,FastFont>::iterator i;
	std::string font_name = std::string("fonts/")+std::string(pFontFileName)+std::string(".mlff");

	i = m_FastFontMap.find(font_name);
	if(i==m_FastFontMap.end())
	{
		LoadFastFont(D3DKernel::GetD3DDevice(),pFontFileName);
		i = m_FastFontMap.find(font_name);
		if(i==m_FastFontMap.end())
			throw CASUS(std::string("Ўрифт с именем <")+font_name+std::string("> не был загружен.\n"));
	}

	return i->second;
}
*//*
void FontMgr::Tune(LPDIRECT3DDEVICE7 lpd3dDevice)
{
	std::map<std::string,FastFont>::iterator i;

	for(i=m_FastFontMap.begin();i!=m_FastFontMap.end();i++)
	{
		i->second.Tune(lpd3dDevice);
	}
}*/
