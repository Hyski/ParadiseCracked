//
// ���������� ���� �������
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
// ������� ����� �� �������
//
class InsetMgr{
public:

    virtual ~InsetMgr() {}

    //�������� �������
    virtual void Show() = 0;
};

//
// ������� ��� �������
//
class QuestInsetMgr : public InsetMgr,
                      private QuestSheetController
{
public:

    QuestInsetMgr();
    ~QuestInsetMgr();

    //�������� quest'�
    void Show();

private:

    //��������� ������ ������
    void HandleNextQuestReq();
    //��������� ������ �����
    void HandlePrevQuestReq();

    //�������� ���������� � ������� ������
    void ShowQuest(QuestInfoPool::Iterator* quest);

    //��������� ��������� ����
    void OnButtonClick(QuestSheet::BUTTON button);

private:

    rid_t m_current;
    QuestInfoPool::Iterator* m_itor;
};

//
// ������� ��� ������������
//
class BookInsetMgr : public  InsetMgr,
                     private BookSheetController
{
public:

    BookInsetMgr();
    ~BookInsetMgr();

    //�������� ������������
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

    //��������� ������ ��������� ��������
    void HandleNextDescReq();
    //��������� ������ ���������� ��������
    void HandlePrevDescReq();
    //��������� ������ �������� ������
    void HandleNextGroupReq();
    //��������� ������ ���������� ������
    void HandlePrevGroupReq();

    //��������� ��������� ����
    void OnButtonClick(BookSheet::BUTTON button);

private:

    info_type m_current;    
    InfoMgr*  m_infos[MAX_INFOS];
};


//
// ����� �� ������� ��� ����� �������
//
class EpisodeMapInsetMgr : public InsetMgr{
public:

	EpisodeMapInsetMgr();
	~EpisodeMapInsetMgr();

	//�������� �������
	void Show();

private:

    bool m_first_time;
};

//
// ���������� ���� �������
//
class JournalFormImp : public  JournalForm,
                       private JournalScreenController
{
public:

    JournalFormImp();
    ~JournalFormImp();

    //�������� ���� 
    void Show();

private:

    //��������� �����
    void HandleInput(state_type* st);

    //��������� ������ �� ���� 
    void HandleExitFormReq();
    //��������� ������� �� ����� �������
    void HandleShowInsetReq(int inset_id);

    //��������� ������� � ���� ��������
    void OnButtonClick(JournalScreen::BUTTON button);

private:

    enum{
        MAX_INSETS = 4, //���������� ������� � ����
    };

    InsetMgr* m_inset_mgr[MAX_INSETS];
};

#endif // _PUNCH_JOURNALFORMIMP_H_
