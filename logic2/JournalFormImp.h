//
// реализация меню журнала
//

#ifndef _PUNCH_JOURNALFORMIMP_H_
#define _PUNCH_JOURNALFORMIMP_H_

#include "questserver.h"
#include "journalform.h"

#include "../interface/screens.h"
#include "../interface/journalscreen.h"

class InsetMgr;

struct quest_traits;

class BookSheetController;
class QuestSheetController;
class JournalScreenController;

//
// базовый класс на вкладку
//
class InsetMgr{
public:

    virtual ~InsetMgr() {}

    //показать вкладку
    virtual void Show() = 0;
};

//
// вкладка для журнала
//
class QuestInsetMgr : public InsetMgr,
                      private QuestSheetController
{
public:

    QuestInsetMgr();
    ~QuestInsetMgr();

    //показать quest'ы
    void Show();

private:

    //обработка кнопки вперед
    void HandleNextQuestReq();
    //обработка кнопкт назад
    void HandlePrevQuestReq();

    //показать информацию о текущем квесте
    void ShowQuest(QuestInfoPool::Iterator* quest);

    //обработка сообщений меню
    void OnButtonClick(QuestSheet::BUTTON button);

private:

    rid_t m_current;
    QuestInfoPool::Iterator* m_itor;
};

//
// вкладка для энциклопедии
//
class BookInsetMgr : public  InsetMgr,
                     private BookSheetController
{
public:

    BookInsetMgr();
    ~BookInsetMgr();

    //показать энциклопедию
    void Show();

    class InfoMgr{
    public:
        
        virtual ~InfoMgr(){}
        
        virtual void Show() = 0;
        
        virtual void Next() = 0;
        virtual void Prev() = 0;
    };

private:

    enum info_type{
        RT_WEAPONS,
        RT_ARMOURS,
        RT_GRENADES,
        RT_IMPLANTS,
        RT_MEDIKITS,
        RT_AMMUNITION,

        RT_VEHICLES,
        RT_ORGANIZATIONS,

        MAX_INFOS,
    };

    //обработка кнопки следующее описание
    void HandleNextDescReq();
    //обработка кнопки предидущее описание
    void HandlePrevDescReq();
    //обработка кнопки следущая группа
    void HandleNextGroupReq();
    //обработка кнопки предидущая группа
    void HandlePrevGroupReq();

    //обработка сообщений меню
    void OnButtonClick(BookSheet::BUTTON button);

private:

    info_type m_current;    
    InfoMgr*  m_infos[MAX_INFOS];
};


//
// класс на вкладку для карты эпизода
//
class EpisodeMapInsetMgr : public InsetMgr{
public:

	EpisodeMapInsetMgr();
	~EpisodeMapInsetMgr();

	//показать вкладку
	void Show();

private:

    bool m_first_time;
};

//
// собственно меню журнала
//
class JournalFormImp : public  JournalForm,
                       private JournalScreenController
{
public:

    JournalFormImp();
    ~JournalFormImp();

    //показать меню 
    void Show();

private:

    //обработка ввода
    void HandleInput(state_type* st);

    //обработка выхода из меню 
    void HandleExitFormReq();
    //обработка запроса на показ вкладки
    void HandleShowInsetReq(int inset_id);

    //обработка конопок в меню журанала
    void OnButtonClick(JournalScreen::BUTTON button);

private:

    enum{
        MAX_INSETS = 4, //количество вкладок в меню
    };

    InsetMgr* m_inset_mgr[MAX_INSETS];
};

#endif // _PUNCH_JOURNALFORMIMP_H_
