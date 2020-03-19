/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   18.07.2001

************************************************************************/
#ifndef _WIDGET_SOUND_MGR_H_
#define _WIDGET_SOUND_MGR_H_

#include "../Common/UI/Widget.h"

class ISndScript;
class ISndEmitter;

class WidgetSoundMgr
{
public:
	class Sample;
	class Player;
private:
	struct Deleter 
	{
	public:
		WidgetSoundMgr *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	Player* m_Player;
private:
	WidgetSoundMgr();
	virtual ~WidgetSoundMgr();
private:
	class Reference;
	std::map<std::string,Reference*> m_Samples;		//	карта сэмплов
public:
	//	увеличить ссылку на сэмпл
	WidgetSoundMgr::Sample* CreateSample(const char* name);
	WidgetSoundMgr::Sample* CreateSample(const char* name,const char* description);
	//	уменьшить ссылку на сэмпл
	void Release(WidgetSoundMgr::Sample* sample);
	//	очистить все
	void Release(void);
	//	обновить громкость звуков
	void UpdateVolume(void);
	//	получить указатель на плеер
	WidgetSoundMgr::Player* GetPlayer(void) {return m_Player;}
public:
	static WidgetSoundMgr *Instance(void);
};

inline WidgetSoundMgr* WidgetSoundMgr::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new WidgetSoundMgr();
	return m_Deleter.m_pInstance;
}

//***************************************************************//
//	class WidgetSoundMgr::Sample
class WidgetSoundMgr::Sample : public Widget::Player::Sample
{
private:
	const ISndScript*    m_Script;
	ISndEmitter*   m_Emitter;
	std::string*   m_Description;
	std::string*   m_Name;
public:
	Sample(const char* description);
	Sample(const char* description,const char* name);
	virtual ~Sample();
public:
	const ISndScript* Script() {return m_Script;}
	const std::string* Description(void) {return m_Description;}
	const std::string* Name(void) {return m_Name;}
	ISndEmitter*& Emitter(void) {return m_Emitter;}
	void Release(void);
};

//***************************************************************//
//	class WidgetSoundMgr::Reference
class WidgetSoundMgr::Reference
{
private:
	WidgetSoundMgr::Sample* m_Value;
	int m_Counter;	
public:	
	Reference(WidgetSoundMgr::Sample* value);
	virtual	~Reference();
public:	
	WidgetSoundMgr::Sample* Get(void){m_Counter++;return m_Value;}
	int Release(void){if(m_Counter){m_Counter--;} return m_Counter;}
	int Counter(void){return m_Counter;}
};

//***************************************************************//
//	class WidgetSoundMgr::Player
class WidgetSoundMgr::Player : public Widget::Player
{
public:
	Player();
	virtual ~Player();
public:
	void Play(Widget::Player::Sample* sample);
	void Stop(Widget::Player::Sample* sample);
};


#endif	//_WIDGET_SOUND_MGR_H_