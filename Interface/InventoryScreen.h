/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   12.03.2001

************************************************************************/
#ifndef _INVENTORY_SCREEN_H_
#define _INVENTORY_SCREEN_H_

#include "Dialog.h"

//#define _DEBUG_SLOTS_ENGINE

class InventoryScreenController;
class MMgrController;
class SlotScroller;
class SlotsEngine;
class BaseThing;
class SESlot;
class Text;

class InventoryScreen : public Dialog
{
//	class DSController;
public:
	static const char* m_pDialogName;
	static const char* m_PortraitPath;
	static const char* m_PortraitExt;
private:
	InventoryScreenController* m_Controller;
	MMgrController* m_MMgrController;
//	DSController* m_DSController;
	SlotScroller* m_GroundScroller;
	SlotsEngine* m_SlotsEngine;
public:
	InventoryScreen();
	virtual ~InventoryScreen();
public:
//****************** ВНЕШНИЙ ИНТЕРФЕЙС С ЛОГИКОЙ *********************//
	//	установить контроллер логики для интерфейса
	void SetController(InventoryScreenController* controller);
//	*******************************************************************
//	кнопка управления персонажем ****
	enum BUTTON 
	{
		B_DISMISS,
		B_UNLOAD,
		B_BACK,
		B_INFO,
		B_PREV_HERO,
		B_NEXT_HERO,
		MAX_BUTTON
	};
	// показать/скрыть кнопку
	void EnableButton(BUTTON id,bool enable);
	//	*******************************************************************
	enum SCROLLER
	{
		S_GND_TO_LEFT,
		S_GND_TO_RIGHT,
		MAX_SCROLLER
	};
	//	*******************************************************************
	enum LEVEL_UP
	{
		LU_STRENGTH,
		LU_DEXTERITY,
		LU_REACTION,
		LU_ACCURACY,
		LU_WISDOM,
		LU_MECHANICS,
		LU_SIGHT,
		LU_HEALTH,
		MAX_LEVEL_UP
	};
	enum LEVEL_UP_SIGN
	{
		LUS_PLUS,					//	значек плюс
		LUS_MINUS					//	значек минус
	};
	void ShowLevelUp(bool show);
	void ShowLevelUp(LEVEL_UP index,LEVEL_UP_SIGN sign,bool show);
	//********* Надписи в меню *********************//
	struct HeroTraits
	{
		std::string m_NextLevel;
		int m_Experience;
		int m_Level;
		int m_LevelUpPoints;
		  int m_Strength;
		 int m_Dexterity;
	 	 int m_Reaction;
		 int m_Accuracy;
		 int m_Wisdom;
		  int m_Mechanics;
		  int m_Sight;
		int m_ShockRes;
		int m_FireRes;
		int m_ElectricRes;
		int m_MoveCurrent;
		int m_MoveMax;
		 int m_HealthMax;
		int m_MoralCurrent;
		int m_MoralMax;
		int m_Money;
		int m_WeightCurrent;
		int m_WeightMax;
	};
	//	установить текстовое значение	
	void SetHeroTraits(const HeroTraits& t);
//	*******************************************************************
	struct PortraitTraits
	{
		const char* m_portrait_name;		//	системное имя героя
		const char* m_hero_name;			//	не системное имя героя
	};
	void SetPortraitTraits(const PortraitTraits& t);
//	*******************************************************************
	enum SLOT
	{
		S_GROUND,
		S_BACKPACK,
		S_KNEE_LEFT,
		S_KNEE_RIGHT,
		S_HANDS,
		S_BODY,
		S_IMPLANT,
		S_HEAD,
		MAX_SLOT,
		S_NONE
	};
public:
	//	получить указатель на механизм работы со слотами
	SlotsEngine* GetSlotsEngine(void) const {return m_SlotsEngine;}
	//	заблокировать/разблокировать кнопки
	void BlockButtons(bool block);
private:
	//	создаем слоты для раскладки итемов
	void CreateSlots(void);
	//	массив кнопок
	Button* m_Buttons[MAX_BUTTON];
	//	массив скроллеров
	Static* m_Scrollers[MAX_SCROLLER];
	//	массив LevelUp'оп
	struct LevelUpSigns
	{
		Button* m_Plus;
		Button* m_Minus;
		Static* m_Place;
	} m_LevelUps[MAX_LEVEL_UP];
	//	*******************************************************************
private:
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
	//	производим какие-то действия
	bool Tick(void);

#ifdef _DEBUG_SLOTS_ENGINE
	//	отображение виджета и всех дочерних виджетов
	void Render(WidgetView* pView);
#endif
	//	сообщения от кнопок
	void OnButton(Button *pButton);
	//	сообщения от статиков
	void OnStatic(Static* pStatic);
};

//**********************************************************************//
//*********	       class InventoryScreenController         *************//
class InventoryScreenController
{
public:
	virtual ~InventoryScreenController(){};
public:
	virtual void OnButtonClick(InventoryScreen::BUTTON id) = 0;
	virtual void OnLevelUpClick(InventoryScreen::LEVEL_UP id,InventoryScreen::LEVEL_UP_SIGN sign) = 0;
	virtual void OnLevelUpDescription(InventoryScreen::LEVEL_UP id) = 0;
};

/////////////////////////////////////////////////////////////////////////
////////////////////////    class SlotScroller    ///////////////////////
/////////////////////////////////////////////////////////////////////////
class SlotScroller
{
	static const float m_Pause;
	static const float m_Speed;
public:
	enum DIRECTION {D_LEFT,D_RIGHT,D_UP,D_DOWN};
private:
	SESlot* m_Slot;
	float m_Start;
	float m_Seconds;
public:
	SlotScroller(SESlot* slot);
	~SlotScroller();
public:
	void Start(DIRECTION dir);
	void Scroll(DIRECTION dir);
private:
	void Translate(DIRECTION dir,int* dx,int* dy);
};

#endif	//_INVENTORY_SCREEN_H_