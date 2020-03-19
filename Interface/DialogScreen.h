/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.03.2001

************************************************************************/
#ifndef _DIALOG_SCREEN_H_
#define _DIALOG_SCREEN_H_

#include "Dialog.h"

class DialogScreenController;

class DialogScreen : public Dialog
{
public:
	static const char* m_pDialogName;
	static const char* m_PortraitPath;
	static const char* m_PortraitExt;
private:
	DialogScreenController* m_pController;
	Widget::Player::Sample* m_SpeechSound;
public:
	DialogScreen();
	virtual ~DialogScreen();
private:
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
	//	сообщение от кнопок
	void OnButton(Button* pButton);
	//	сообщение от статиков
	void OnStatic(Static* pStatic);
	//	производим какие-то действия
	bool Tick(void);
public:
	struct DialogTraits
	{
		std::string m_PortraitName;			//	системное имя героя
		std::string m_Text;					//	текст :)
		std::string m_Speech;				//	путь до вавчика
	};
	//	становить параметры диалогового окна 
	void SetTraits(const DialogTraits& dt);
	//	установить контроллер
	void SetController(DialogScreenController* pController);
	//	вызвать окошко как модальное
	void DoModal(const DialogTraits& dt);
};

//**********************************************************************//
//*********	       class DialogScreenController            *************//
class DialogScreenController
{
public:
	virtual ~DialogScreenController(){};
public:
	// событие закрытия окна
	virtual void OnClose(void) = 0;
};

//**********************************************************************//
//	class HeroInfoScreen
class HeroInfoScreenController;

class HeroInfoScreen : public Dialog
{
public:
	static const char* m_pDialogName;
	static const char* m_PortraitPath;
	static const char* m_PortraitExt;
private:
	HeroInfoScreenController* m_Controller;
public:
	HeroInfoScreen();
	virtual ~HeroInfoScreen();
private:
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
	//	сообщение от кнопок
	void OnButton(Button* pButton);
	//	производим какие-то действия
	bool Tick(void);
public:
	struct HeroTraits
	{
		std::string m_PortraitName;			//	системное имя героя
		std::string m_Text;					//	текст :)
	};
	//	установить контроллер
	void SetController(HeroInfoScreenController* controller);
	//	вызвать окошко как модальное
	void DoModal(const HeroTraits& ht);
private:
	//	становить параметры диалогового окна 
	void SetTraits(const HeroTraits& ht);
};

//**********************************************************************//
//	class HeroInfoScreenController
class HeroInfoScreenController
{
public:
	virtual ~HeroInfoScreenController(){};
public:
	// событие закрытия окна
	virtual void OnClose(void) = 0;
};

//**********************************************************************//
//	class HelperScreen
class HelperScreen : public Dialog
{
public:
	struct Traits
	{
		std::string m_Image;				//	картинка
		std::string m_Text;					//	текст :)
		bool m_ShowThisMessage;				//	показывать это сообщение впоследствие
		bool m_ShowHelper;					//	показывать любые сообщения впоследствие
	};
	static const char* m_pDialogName;
private:
	VRegion m_ImagePlace;
	Widget* m_Image;
	Static* m_ShowThisMessage;
	Static* m_ShowHelper;
public:
	HelperScreen();
	virtual ~HelperScreen();
private:
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
	//	сообщение от статиков
	void OnStatic(Static* pStatic);
	//	производим какие-то действия
	bool Tick(void);
public:
	//	вызвать окошко как модальное
	void DoModal(Traits& t);
private:
	//	становить параметры диалогового окна 
	void SetTraits(const Traits& t);
};

//**********************************************************************//
//	class FlagDescScreen
class FlagDescScreen : public Dialog
{
public:
	struct Traits
	{
		std::string m_Image;				//	картинка
		std::string m_Text;					//	текст :)
	};
	static const char* m_pDialogName;
private:
	VRegion m_ImagePlace;
	Widget* m_Image;
public:
	FlagDescScreen();
	virtual ~FlagDescScreen();
private:
	//	уведомление об изменении состояния системы
	void OnSystemChange(void);
	//	сообщение от статиков
	void OnStatic(Static* pStatic);
	//	производим какие-то действия
	bool Tick(void);
public:
	//	вызвать окошко как модальное
	void DoModal(const Traits& t);
private:
	//	становить параметры диалогового окна 
	void SetTraits(const Traits& t);
};

#endif	//_DIALOG_SCREEN_H_