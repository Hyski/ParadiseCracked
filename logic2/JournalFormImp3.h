//
// реализация меню журнала iteration3
//

#ifndef _PUNCH_JOURNALFORMIMP3_H_
#define _PUNCH_JOURNALFORMIMP3_H_

#include "form.h"
#include "../interface/journalscreen.h"

//
// Реализация меню журнала
//
class JournalFormImp3 : public  JournalForm,
                        private JournalScreen::Controller
{
public:

    JournalFormImp3();
    ~JournalFormImp3();

    //показать меню 
    void Show();
    //инициализация меню
    void Init(const ini_s& ini);
    //сохранить/загрузить настройки меню
    void MakeSaveLoad(SavSlot& slot);
    //обработка ввода
    void HandleInput(state_type* st);

    //есть ли новая информация в меню
    bool IsUpdated() const;

    //
    // Интерфейс вкладки журнала
    //
    class Sheet{
    public:
        
        virtual ~Sheet(){}
        
        enum show_mode{
            SM_USUAL,   //обычный показ вкладки
            SM_UPDATES, //покзать обновления для вкладки 
        };
        
        //была ли вкладка обновлена со времени последнего просмотра
        virtual bool IsUpdated() const = 0;
        //показать вкладку
        virtual void Show(show_mode mode) = 0;
        //сохранить вкладку
        virtual void MakeSaveLoad(SavSlot& slot) = 0;
    };

private:

    //закрыть меню журнала
    void HandleExitFormReq();

    //обработка закрытия меню
    void OnBack(void);
    //обработка активации вкладки
    void OnSheetClick(JournalScreen::SHEET id);
    //обработка нажатия кнопки обновления
    void OnUpdateClick(JournalScreen::UPDATE_BUTTON id);

private:

    enum{ MAX_SHEETS = 5, };

    Sheet* m_sheet[MAX_SHEETS];

    float m_delta_time;   //интервал обновления кнопок
    float m_update_time;  //время последнего обновления кнопок
};

#endif // _PUNCH_JOURNALFORMIMP3_H_