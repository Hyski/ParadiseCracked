/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   05.03.2001

************************************************************************/
#ifndef _WIDGET_FACTORY_H_
#define _WIDGET_FACTORY_H_

#include "../Common/UI/VData.h"
#include "../Common/UI/Widget.h"

class TxtFile;
class DXImage;
class Dialog;

class WidgetFactory
{
public:
	enum WIDGET_TYPE
	{
		WT_UNKNOWN,
		WT_WIDGET,
		WT_BUTTON,
		WT_STATIC,
		WT_TEXT,
		WT_TEXTBOX,
		WT_EDIT,
		WT_SLIDER,
		WT_MULTIEDIT,
		WT_DIALOG,
	};
private:
	struct Deleter 
	{
	public:
		WidgetFactory *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	const char *m_pScriptPath;
	const char *m_pScriptExt;
	//	нужно только при передачи данных при загрузке скрипта
	float m_OriginX;
	float m_OriginY;
private:
	WidgetFactory();
	virtual ~WidgetFactory();
public:
	void LoadScript(Dialog* pOwner,const char* pScript);
	//	возвращает тип виджета эквивалентный типу указанному в передаваемой строке
	WidgetFactory::WIDGET_TYPE WidgetType(const char *pType);
	//	возвращает тип виджета
	WidgetFactory::WIDGET_TYPE WidgetType(Widget* widget);
	//	осуществляет проверку на существование дочернего виджета
	Widget* Assert(Widget *pOwner,const char *pName);
private:
	Widget* LoadWidget(const char *pName,TxtFile* pScript,int iString);
	Widget* LoadButton(const char *pName,TxtFile* pScript,int iString);
	Widget* LoadStatic(const char *pName,TxtFile* pScript,int iString);
	Widget* LoadText(const char *pName,TxtFile* pScript,int iString);
	Widget* LoadTextBox(const char *pName,TxtFile* pScript,int iString);
	Widget* LoadEdit(const char *pName,TxtFile* pScript,int iString);
	Widget* LoadSlider(const char *pName,TxtFile* pScript,int iString);
	Widget* LoadMultiEdit(const char *pName,TxtFile* pScript,int iString);
private:
	const VRegion& GetRegion(TxtFile* pScript,int iString);
	DXImage* GetImage(const char *pKey,TxtFile* pScript,int iString);
	Widget::Player::Sample* GetSound(const char *pKey,TxtFile* pScript,int iString);
	int GetLink(TxtFile* pScript,int iString);
	void SetTip(Widget* pWidget,TxtFile* pScript,int iString);
public:
	static WidgetFactory *Instance(void);
};

inline WidgetFactory* WidgetFactory::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new WidgetFactory();
	return m_Deleter.m_pInstance;
}

#endif	//_WIDGET_FACTORY_H_