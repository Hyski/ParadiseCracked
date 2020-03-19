/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   26.04.2001

************************************************************************/
#ifndef _DESCRIPTION_SCREEN_H_
#define _DESCRIPTION_SCREEN_H_

#include "Dialog.h"

class DescriptionScreenController;
class Text;

class DescriptionScreen : public Dialog
{
public:
	static const char* m_pDialogName;
private:
	DescriptionScreenController* m_pController;
	VRegion m_ImagePlace;		//	месторасположение картинки
	VRegion m_TextPlace;		//	месторасположение текстовой информации
	Widget* m_Image;			//	изображение предмета описания
	Text* m_Caption;			//	заголовок 
	Text* m_LeftColumn;			//	характеристики (левая колонка)
	Text* m_RightColumn;		//	значения характеристик (правая колонка)
	Text* m_Info;				//	информация (собственно описание)
//	int m_TextDown;
public:
	DescriptionScreen();
	virtual ~DescriptionScreen();
public:
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
	bool Tick(void);
	void SetVisible(bool bVisible);
	//**************** ИНТЕРФЕЙС С ЛОГИКОЙ ***********************//
	void SetController(DescriptionScreenController* controller);
	enum ITEM_TYPE {IT_OUTFIT,IT_TECH,IT_FORCES,IT_PEOPLE/*,IT_LOCATION*/};
	struct ItemInfo
	{
		const char* m_Image;
		const char* m_Name;
		const char* m_LeftColumn;
		const char* m_RightColumn;
		const char* m_Text;
		ITEM_TYPE m_Type;
		ItemInfo() : m_Image(0),m_Name(0),m_LeftColumn(0),m_RightColumn(0),
					 m_Text(0),m_Type(IT_OUTFIT) {}
	};
	//	установить параметры
	void SetItemInfo(const ItemInfo& ii);
private:
	//	получить каталог с картинками для данной закладки
	const char* GetPathForResource(ITEM_TYPE it) const;
};

//************************************************************************//
//************************************************************************//
class DescriptionScreenController
{
public:
	virtual ~DescriptionScreenController(){}
	virtual void OnClose(void) = 0;
};



#endif	//_DESCRIPTION_SCREEN_H_