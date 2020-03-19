#pragma warning(disable:4786)

#include "logicdefs.h"

#include <iomanip>

#include "spawn.h"
#include "thing.h"
#include "entity.h"
#include "aiutils.h"
#include "dirtylinks.h"
#include "GameObserver.h"

#include "JournalFormImp3.h"

#include "../interface/screens.h"

//================================================================

JournalForm* FormFactory::CreateJournalForm()
{
	return new JournalFormImp3();
}

//================================================================

namespace{

//вычислить состояние кнопки обновлений в меню журнала
JournalScreen::ACTIVATE_MODE CalcUpdateBtnState(JournalFormImp3::Sheet* sheet)
{
    return sheet && sheet->IsUpdated() ? JournalScreen::AM_ACTIVATE : JournalScreen::AM_DEACTIVATE;
}

//================================================================

//
// владка записей
//
class JF_DiarySheet : public  JournalFormImp3::Sheet,
                      private GameObserver,
                      private SpawnObserver,
                      private JournalSheet::Controller
{
public:

    JF_DiarySheet() : m_diary_itor(0), m_cur_rec(0), m_cur_theme(0)
    {
        Spawner::Attach(this, ET_ENTRY_LEVEL);
        GameEvMessenger::GetInst()->Attach(this, EV_DIARY_CHANGE);

        m_diary_itor = DiaryManager::GetInst()->CreateIterator();
    }

    ~JF_DiarySheet()
    {
        Spawner::Detach(this);
        GameEvMessenger::GetInst()->Detach(this);

        delete m_diary_itor;
    }

    void Show(show_mode mode)
    {
        Screens::Instance()->Journal()->Journal()->SetController(this);
        Screens::Instance()->Journal()->Activate(JournalScreen::SID_JOURNAL);

        //удалим из индекса темы которых нет
        index_map_t::iterator itor = m_index.begin();
        while(itor != m_index.end()){

            if(DiaryManager::GetInst()->Get(itor->first) == 0){
                m_index.erase(itor++);
                continue;
            }

            ++itor;
        }

        //покажем так как нужно
        if(mode == SM_USUAL || !ShowUpdateMode()) ShowUsualMode();
    }  
    
    bool IsUpdated() const
    { 
        m_diary_itor->First();
        while(m_diary_itor->IsNotDone()){

            if(m_diary_itor->Get()->IsRaised(DiaryManager::Record::F_NEW))
                return true;

            m_diary_itor->Next();
        }

        return false;
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving()){

            slot << (m_cur_rec != 0);
            if(m_cur_rec) slot << m_cur_rec->GetKey();

            slot << (m_cur_theme != 0);
            if(m_cur_theme) slot << m_cur_theme->GetKey();
            
            slot << m_index.size();

            index_map_t::iterator itor = m_index.begin();
            while(itor != m_index.end()){
                slot << itor->first << itor->second;
                ++itor;
            }

        }else{

            m_cur_rec = 0;
            m_cur_theme = 0;
            m_index.clear();

            bool flag;

            slot >> flag;
            if(flag){
                DiaryManager::key_t key; slot >> key;
                m_cur_rec = DiaryManager::GetInst()->Get(key);
            }

            slot >> flag;
            if(flag){
                DiaryManager::key_t key; slot >> key;
                m_cur_theme = DiaryManager::GetInst()->Get(key);
            }
           
            size_t size; slot >> size;
            while(size--){
                DiaryManager::key_t key1, key2;
                slot >> key1 >> key2;
                m_index[key1] = key2;
            }            
        }
    }

private:

    void OnSheetClick(JournalSheet::SHEET id)
    {
        JournalSheet* sheet = Screens::Instance()->Journal()->Journal();

        switch(id){
        case JournalSheet::S_NEWTHEME:
            //начать редактирование записи
            sheet->EditSection(JournalSheet::S_THEME, "");
            break;

        case JournalSheet::S_DELTHEME:
            if(m_cur_theme){

                if(Forms::GetInst()->ShowMessageBox(DirtyLinks::GetStrRes("jf_can_erase_theme"))){
                    ShowTheme(EraseRec(m_cur_theme));
                    SetFirstRec(m_cur_theme);
                }

                sheet->UnSelectSheet(id);
            }
            break;

        case JournalSheet::S_NEWENTRY:
            //начать редактирование записи
            if(m_cur_theme) sheet->EditSection(JournalSheet::S_ENTRY, "");
            break;

        case JournalSheet::S_DELENTRY:
            if(m_cur_rec){
                
                if(Forms::GetInst()->ShowMessageBox(DirtyLinks::GetStrRes("jf_can_erase_entry")))
                    ShowRecord(EraseRec(m_cur_rec));
                
                sheet->UnSelectSheet(id);
            }
            break;

        case JournalSheet::S_SAVE_THEME:
            {
                //добавим новую тему
                DiaryManager::key_t key = DiaryManager::GetInst()->Insert(DiaryManager::Record(sheet->GetSectionText(JournalSheet::S_THEME), DiaryManager::ROOT_KEY));
                //установим новую тему
                ShowTheme(DiaryManager::GetInst()->Get(key));
                //установим пустую запись
                SetFirstRec(m_cur_theme);
            }
            break;

        case JournalSheet::S_SAVE_ENTRY:
            {
                //добавим новую запись
                DiaryManager::key_t key = DiaryManager::GetInst()->Insert(DiaryManager::Record(sheet->GetSectionText(JournalSheet::S_ENTRY), m_cur_theme->GetKey()));
                //установим пустую запись
                ShowRecord(DiaryManager::GetInst()->Get(key));
            }
            break;
        }
    }

    void OnButtonClick(JournalScreen::BUTTON id)
    {
        switch(id){
        case JournalScreen::B_UP:
            if(m_cur_theme){
                ShowTheme(FindNext(m_cur_theme));
                SetFirstRec(m_cur_theme);
            }
            break;
            
        case JournalScreen::B_DOWN:
            if(m_cur_theme){
                ShowTheme(FindPrev(m_cur_theme));
                SetFirstRec(m_cur_theme);
            }
            break;
            
        case JournalScreen::B_RIGHT:
            if(m_cur_rec) ShowRecord(FindNext(m_cur_rec));                 
            break;
            
        case JournalScreen::B_LEFT:
            if(m_cur_rec) ShowRecord(FindPrev(m_cur_rec));                 
            break;
        }        
    }

    void ShowUsualMode()
    {
        if(!ShowTheme(m_cur_theme)){
            
            m_cur_rec = 0;

            m_diary_itor->First(DiaryManager::ROOT_KEY);
            ShowTheme(m_diary_itor->Get());
        }

        SetFirstRec(m_cur_theme);
    }

    bool ShowUpdateMode()
    {
        DiaryManager::Record* new_rec   = 0;

        //ищем новую запись 
        m_diary_itor->First();
        while(m_diary_itor->IsNotDone()){

            DiaryManager::Record* rec = m_diary_itor->Get();

            if(     rec->GetKeyUp() != DiaryManager::ROOT_KEY
                && rec->IsRaised(DiaryManager::Record::F_NEW)){
                new_rec = rec;
                break;
            }

            m_diary_itor->Next();
        }

        //ищем новую тему
        m_diary_itor->First(DiaryManager::ROOT_KEY);
        while(m_diary_itor->IsNotDone()){

            DiaryManager::Record* rec = m_diary_itor->Get();

            if(     (new_rec && new_rec->GetKeyUp() == rec->GetKey())
                ||  (new_rec == 0 && rec->IsRaised(DiaryManager::Record::F_NEW))){
                ShowTheme(rec);
                ShowRecord(new_rec);
                return true;
            }

            m_diary_itor->Next();
        }
        
        return false;
    }

    bool ShowTheme(DiaryManager::Record* rec)
    {
        JournalSheet* sheet = Screens::Instance()->Journal()->Journal();

        m_cur_theme = rec;

        sheet->EnableSheet(JournalSheet::S_NEWENTRY, m_cur_theme != 0);
        sheet->EnableSheet(JournalSheet::S_DELTHEME, m_cur_theme != 0);

        //пустой журнал
        if(m_cur_theme == 0){
            sheet->SetTextField("", JournalSheet::TF_THEME);
            sheet->SetTextField("00/00", JournalSheet::TF_THEME_COUNT);
            return false;
        }

         m_cur_theme->DropFlags(DiaryManager::Record::F_NEW);

        std::ostringstream ostr;

        ostr << std::setfill('0') << std::setw(2) << GetRecordPos(DiaryManager::ROOT_KEY, m_cur_theme->GetKey()) << '/';
        ostr << std::setfill('0') << std::setw(2) << CountRecords(DiaryManager::ROOT_KEY);
        
        sheet->SetTextField(m_cur_theme->GetInfo().c_str(), JournalSheet::TF_THEME);
        sheet->SetTextField(ostr.str().c_str(), JournalSheet::TF_THEME_COUNT);
        return true;
    }

    bool ShowRecord(DiaryManager::Record* rec)
    {
        JournalSheet* sheet = Screens::Instance()->Journal()->Journal();

        m_cur_rec = rec;

        sheet->EnableSheet(JournalSheet::S_DELENTRY, m_cur_rec != 0);

        //пустой журнал
        if(m_cur_rec == 0){
            sheet->SetTextField("", JournalSheet::TF_CONTENT);
            sheet->SetTextField("00/00", JournalSheet::TF_ENTRY_COUNT);
            return false;
        }

        m_cur_rec->DropFlags(DiaryManager::Record::F_NEW);

        //оставим запись в индексе
        m_index[m_cur_rec->GetKeyUp()] = m_cur_rec->GetKey();

        std::ostringstream ostr;
        
        ostr << std::setfill('0') << std::setw(2) << GetRecordPos(m_cur_rec->GetKeyUp(), m_cur_rec->GetKey()) << '/';
        ostr << std::setfill('0') << std::setw(2) << CountRecords(m_cur_rec->GetKeyUp());

        sheet->SetTextField(m_cur_rec->GetInfo().c_str(), JournalSheet::TF_CONTENT);
        sheet->SetTextField(ostr.str().c_str(), JournalSheet::TF_ENTRY_COUNT);
        return true;
    }

    size_t CountRecords(DiaryManager::key_t theme_key) const
    {
        size_t count = 0;

        m_diary_itor->First(theme_key);
        while(m_diary_itor->IsNotDone()){
            count++;
            m_diary_itor->Next();
        }

        return count;
    }

    size_t GetRecordPos(DiaryManager::key_t theme_key, DiaryManager::key_t rec_key) const
    {
        size_t pos = 0;

        m_diary_itor->First(theme_key);
        while(m_diary_itor->IsNotDone()){

            pos++;

            if(m_diary_itor->Get()->GetKey() == rec_key)
                return pos;

            m_diary_itor->Next();
        }

        return 0;        
    }

    DiaryManager::Record* EraseRec(DiaryManager::Record* rec4del)
    {
        DiaryManager::Record* next_rec = FindNext(rec4del);
        next_rec = rec4del->GetKey() != next_rec->GetKey() ?  next_rec : 0;
        DiaryManager::GetInst()->Delete(rec4del->GetKey());                
        return next_rec;    
    }

    DiaryManager::Record* FindNext(DiaryManager::Record* rec)
    {
        m_diary_itor->First(rec->GetKeyUp());
        while(m_diary_itor->IsNotDone()){
            
            if(m_diary_itor->Get()->GetKey() == rec->GetKey()){
                m_diary_itor->Next();
                break;
            }
            
            m_diary_itor->Next();
        }
        
        if(!m_diary_itor->IsNotDone()) m_diary_itor->First(rec->GetKeyUp());
        return m_diary_itor->Get();
    }

    DiaryManager::Record* FindPrev(DiaryManager::Record* rec)
    {
        DiaryManager::Record* prev = 0;
        
        m_diary_itor->First(rec->GetKeyUp());
        while(m_diary_itor->IsNotDone()){
            
            if(m_diary_itor->Get()->GetKey() == rec->GetKey())
                break;
            
            prev = m_diary_itor->Get();
            m_diary_itor->Next();
        }
        
        if(prev == 0){
            m_diary_itor->First(rec->GetKeyUp());
            while(m_diary_itor->IsNotDone()){
                prev = m_diary_itor->Get();
                m_diary_itor->Next();    
            }
        }

        return prev;
    }

    void SetFirstRec(DiaryManager::Record* theme)
    {
        if(theme == 0){
            ShowRecord(0);
            return;
        }

        if(DiaryManager::Record* rec = DiaryManager::GetInst()->Get(m_index[theme->GetKey()])){
            ShowRecord(rec);
            return;
        }
           
        m_diary_itor->First(theme->GetKey());
        ShowRecord(m_diary_itor->Get());        
    }

    //очистить вкладку от информации опредид посещении
    void Clear()
    {
        m_index.clear();
        m_cur_rec   = 0;
        m_cur_theme = 0;
    }

    void Update(SpawnObserver::subject_t subj, SpawnObserver::event_t event, SpawnObserver::info_t info)
    {
        entry_info_s* ptr = static_cast<entry_info_s*>(info);
        if(ptr->m_entry.empty()) Clear();
    }

    void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info)
    {
        //нам интересно только удаление записи
        if(!static_cast<diary_change*>(info)->IsDel())
            return;

        DiaryManager::key_t key = static_cast<diary_change*>(info)->m_record;

        //если удаляетются все темы
        if(m_cur_theme && m_cur_theme->GetKeyUp() == key){
            ShowTheme(0);
            SetFirstRec(m_cur_theme);
            return;
        }

        //если удаляется текущая тема
        if(m_cur_theme && m_cur_theme->GetKey() == key){
            DiaryManager::Record* rec = FindNext(m_cur_theme);            
            ShowTheme((rec && rec->GetKey() == m_cur_theme->GetKey()) ? 0 : rec);
            SetFirstRec(m_cur_theme);
            return;
        }

        //если удаляется текущая запись
        if(m_cur_rec && m_cur_rec->GetKey() == key){
            DiaryManager::Record* rec = FindNext(m_cur_rec);
            ShowRecord(rec&&(rec->GetKey() == m_cur_rec->GetKey()) ? 0 : rec);
            return;
        }
    }

private:

    typedef std::map<DiaryManager::key_t, DiaryManager::key_t> index_map_t;
    index_map_t  m_index;

    DiaryManager::Record*   m_cur_rec;
    DiaryManager::Record*   m_cur_theme;
    DiaryManager::Iterator* m_diary_itor;
};

//================================================================

//
// вкладка карт
//
class JF_NavigationSheet : public  JournalFormImp3::Sheet,
                           private SpawnObserver,
                           private NavigationSheet::Controller
{
public:
        
    JF_NavigationSheet() : m_fcreate(true), m_current(NavigationSheet::S_CITYMAP)
    {
        Spawner::Attach(this, ET_ENTRY_LEVEL);
    }

    ~JF_NavigationSheet()
    {
        Spawner::Detach(this);
    }
        
    void Show(show_mode mode)
    {
        Screens::Instance()->Journal()->Navigation()->SetController(this);
        Screens::Instance()->Journal()->Activate(JournalScreen::SID_NAVIGATION);

        NavigationSheet* sheet = Screens::Instance()->Journal()->Navigation();
   
        if(m_fcreate || EpisodeMapMgr::GetInst()->IsUpdated()){
  
            m_fcreate = false;
            EpisodeMapMgr::GetInst()->DropUpdatedFlag();
            
            sheet->ClearEpisodeMap();
            sheet->SetEpisode(Spawner::GetInst()->GetEpisode());
            
            EpisodeMapMgr::iterator itor = EpisodeMapMgr::GetInst()->begin();
            while(itor != EpisodeMapMgr::GetInst()->end()){
                
                sheet->ShowEpisodePart( (*itor).c_str(),
                                            *itor == DirtyLinks::GetLevelSysName()
                                        ?   NavigationSheet::T_SELECTED
                                        :   NavigationSheet::T_NORMAL);
                ++itor;                        
            }
        }

        sheet->ShowSheet(m_current);
        sheet->IlluminateSheet(m_current);
    }        

    bool IsUpdated() const { return EpisodeMapMgr::GetInst()->IsUpdated(); }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving()){
            
            slot << static_cast<int>(m_current);

        }else{

            m_fcreate = true;

            int tmp; slot >> tmp;
            m_current = static_cast<NavigationSheet::SHEET>(tmp);
        }
    }   

private:

    void OnButtonClick(JournalScreen::BUTTON id) {}

    void OnSheetClick(NavigationSheet::SHEET id)
    {
        Screens::Instance()->Journal()->Navigation()->ShowSheet(m_current = id);
    }
    
    void Update(subject_t subj, event_t event, info_t info)
    {
        entry_info_s* ptr = static_cast<entry_info_s*>(info);
        if(ptr->m_entry.empty()) Clear();
    }

    void Clear() { m_current = NavigationSheet::S_CITYMAP; }

private:

    bool m_fcreate;
    NavigationSheet::SHEET m_current;
};

//================================================================
    
//
// вкладка энциклопедии
//
class JF_HandbookSheet : public  JournalFormImp3::Sheet,
                         private SpawnObserver,
                         private DatabaseSheet::Controller
{
public:
        
    JF_HandbookSheet() : m_current(0)
    {
        Spawner::Attach(this, ET_ENTRY_LEVEL);

        memset(m_unit, 0, sizeof(m_unit));

        m_unit[DatabaseSheet::S_TECH] = Unit::Create(Unit::UT_VEHICLES);
        m_unit[DatabaseSheet::S_PEOPLE] = Unit::Create(Unit::UT_HUMANS);
        m_unit[DatabaseSheet::S_FORCES] = Unit::Create(Unit::UT_FORCES);
        m_unit[DatabaseSheet::S_OUTFIT] = Unit::Create(Unit::UT_EQUIPMENT);
    }

    ~JF_HandbookSheet()
    {
        Spawner::Detach(this);
        for(int k = 0; k < MAX_SHEETS; delete m_unit[k++]);
    }
        
    void Show(show_mode mode)
    {
        Screens::Instance()->Journal()->Database()->SetController(this);  
        Screens::Instance()->Journal()->Activate(JournalScreen::SID_DATABASE);
        DatabaseSheet* sheet = Screens::Instance()->Journal()->Database();

        //запретить пустые вкладки, разрешить полные
        for(size_t k = 0; k < MAX_SHEETS; k++){
            DatabaseSheet::SHEET current = static_cast<DatabaseSheet::SHEET>(k);
            sheet->EnableSheet(current, !m_unit[k]->IsEmpty()); 
        }

        if(mode == SM_UPDATES){
            
            //ищем первую вкладку с обновлениями
            for(int k = 0; k < MAX_SHEETS; k++){
                
                if(!m_unit[k]->IsEmpty() && m_unit[k]->IsUpdated()){
                    ShowUnit(k, Unit::SM_UPDATES);
                    return;
                }
            }
        }
        
        //если в обычном режиме нет активной вкладки
        if(m_unit[m_current]->IsEmpty()){
            
            //ищем первую активную вкладку
            for(m_current = 0; m_current < MAX_SHEETS; m_current++){
                if(!m_unit[m_current]->IsEmpty()) break;
            }
            
            //нормировка
            if(m_current >= MAX_SHEETS) m_current = 0;
        }
        
        if(!m_unit[m_current]->IsEmpty()){

            //отобразить раздел
            ShowUnit(m_current, Unit::SM_USUAL);
            
        }else{
            
            //все очистить
           
            sheet->SetItemInfo(0);
            sheet->SetSectionName("");
        }        
    }

    bool IsUpdated() const
    { 
        for(int k = 0; k < MAX_SHEETS; k++){
            if(m_unit[k]->IsUpdated())
                return true;
        }

        return false;
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        for(int k = 0; k < MAX_SHEETS; m_unit[k++]->MakeSaveLoad(slot));

        if(slot.IsSaving())
            slot << m_current;
        else
            slot >> m_current;
    }

    //
    // раздел энциклопедии
    //
    class Unit{
    public:

        virtual ~Unit() {}

        enum unit_type{
            UT_HUMANS,
            UT_FORCES,
            UT_VEHICLES,
            UT_EQUIPMENT,
        };

        static Unit* Create(unit_type type);

        //очистить от предид ввода
        virtual void Clear() = 0;
        //пуста ли вкладка
        virtual bool IsEmpty() const = 0;
        //изменилась ли вкладка
        virtual bool IsUpdated() const = 0;

        enum show_mode{
            SM_USUAL,
            SM_UPDATES,
        };

        //показать вкладку
        virtual void Show(show_mode mode) = 0;

        enum scroll_command{
            SC_NEXT_TOPIC,
            SC_PREV_TOPIC,

            SC_NEXT_DESC,
            SC_PREV_DESC,
        };

        //перемотка информации внутри вкладки
        virtual void Scroll(scroll_command cmd) = 0;
        //сохранить данные раздела
        virtual void MakeSaveLoad(SavSlot& slot) = 0;
    };

private:

    void ShowUnit(int index, Unit::show_mode mode)
    {
        m_unit[m_current = index]->Show(mode);
        Screens::Instance()->Journal()->Database()->IlluminateSheet(static_cast<DatabaseSheet::SHEET>(m_current));
    }

    //обработка переключения вкладок
    void OnSheetClick(DatabaseSheet::SHEET id)
    {
        if(!m_unit[id]->IsEmpty()) ShowUnit(id, Unit::SM_USUAL);
    }

    //обработка кнопок навигации
    void OnButtonClick(JournalScreen::BUTTON id)
    {
        if(m_unit[m_current]->IsEmpty())
            return;

        switch(id){
        case JournalScreen::B_UP:
            m_unit[m_current]->Scroll(Unit::SC_NEXT_TOPIC);
            break;

        case JournalScreen::B_DOWN:
            m_unit[m_current]->Scroll(Unit::SC_PREV_TOPIC);
            break;
		
        case JournalScreen::B_LEFT:
            m_unit[m_current]->Scroll(Unit::SC_PREV_DESC);
            break;
		
        case JournalScreen::B_RIGHT:
            m_unit[m_current]->Scroll(Unit::SC_NEXT_DESC);
            break;
        }
    }

    void Update(subject_t subj, event_t event, info_t info)
    {
        entry_info_s* ptr = static_cast<entry_info_s*>(info);
        if(ptr->m_entry.empty()) Clear();
    }

    void Clear()
    {
        m_current = 0;
        for(int k = 0; k < MAX_SHEETS; m_unit[k++]->Clear());
    }

private:

    enum { MAX_SHEETS = DatabaseSheet::MAX_SHEET };

    int   m_current;  
    Unit* m_unit[MAX_SHEETS];
};

//
// обобщенный раздел энциклопедии
//
template<class Handbook, class Topic>
class CommonUnit : public JF_HandbookSheet::Unit{
public:

    typedef Topic topic_t;
    typedef Handbook handbook_t;
    typedef typename Handbook::info_class info_t;
    typedef std::vector<Topic> topic_vec_t; 

    CommonUnit() : m_current(0) {}
    ~CommonUnit() {}

    //пуста ли вкладка
    bool IsEmpty() const
    { return MakeBeginIterator(0, 0) == Handbook::GetInst()->end(); }

    //изменилась ли вкладка
    bool IsUpdated() const
    { return MakeBeginIterator(0, Handbook::F_NEW_REC) != Handbook::GetInst()->end(); }

    //показать вкладку
    void Show(show_mode mode)
    {
        if(m_topics.empty()) UpdateTopics();
        
        if(mode == SM_USUAL || !ShowUpdateMode())
            ShowUsualMode();
    }

    //перемотка информации внутри вкладки
    void Scroll(scroll_command cmd)
    {
        switch(cmd){
        case SC_NEXT_TOPIC:
            {
                int iterations = 2;
                int cursor = m_current;
                
                while(iterations--){
                    
                    for(cursor++ ; cursor < m_topics.size(); cursor++)
                        if(ShowTopic(cursor)) return;
                                            
                    cursor = -1;
                }
            }
            break;

        case SC_PREV_TOPIC:
            {
                int iterations = 2;
                int cursor = m_current;
                
                while(iterations--){
                    
                    for(cursor--; cursor >= 0; cursor--)
                        if(ShowTopic(cursor)) return;
   
                    cursor = m_topics.size();
                }
            }
            break;
            
       case SC_NEXT_DESC:
            {                
                Handbook::iterator itor = MakeBeginIterator(&m_topics[m_current]);
                
                while(itor != Handbook::GetInst()->end()){

                    if(m_topics[m_current].IsCurrent(*itor)){

                        do{
                            ++itor;
                        }while(     itor != Handbook::GetInst()->end()
                                &&  !m_topics[m_current].IsSuitable(*itor));

                        break;
                    }

                    ++itor;
                }

                //нашли подходящий
                if(itor != Handbook::GetInst()->end()){
                    SetCurrentRecord(*itor);
                    return;
                }

                //ищем первый подходящий
                itor = MakeBeginIterator(&m_topics[m_current]);
                while(itor != Handbook::GetInst()->end()){

                    if(m_topics[m_current].IsSuitable(*itor)){
                        SetCurrentRecord(*itor);
                        return;
                    }

                    ++itor;
                }
            }
            break;

        case SC_PREV_DESC:
            {                
                Handbook::iterator prev = Handbook::GetInst()->end(),
                                   itor = MakeBeginIterator(&m_topics[m_current]);
                
                while(itor != Handbook::GetInst()->end()){

                    if(m_topics[m_current].IsCurrent(*itor))
                        break;

                    if(m_topics[m_current].IsSuitable(*itor))
                        prev = itor;

                    ++itor;
                }

                //если не последний элемент списка покажем его
                if(prev != Handbook::GetInst()->end()){
                    SetCurrentRecord(*prev);
                    return;
                }

                //получим сслыку на предпоследний элемент списка
                itor = MakeBeginIterator(&m_topics[m_current]);
                while(itor != Handbook::GetInst()->end()){
                    
                    if(m_topics[m_current].IsSuitable(*itor))
                        prev = itor;

                    ++itor;
                }

                SetCurrentRecord(*prev);
            }
            break;
        }
    }

    void Clear()
    {
        m_current = 0;
        m_topics.clear();
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving()){
            
            slot << m_current;         
            
            slot << m_topics.size();
            for(size_t k = 0; k < m_topics.size(); slot << m_topics[k++]);
            
        }else{

            m_current = 0;
            m_topics.clear();
            
            slot >> m_current;

            size_t size; slot >> size;
            while(size--){
                m_topics.push_back(Topic());
                slot >> m_topics.back();
            }
        }
    }

protected:

    //добавить тему во вкладку
    void InsertTopic(const Topic& topic)
    {
        for(size_t k = 0; k < m_topics.size(); k++){
            if(topic.IsEqual(m_topics[k])) return;
        }

        m_topics.push_back(topic);
    }

    //обновить список тем для вкладки
    virtual void UpdateTopics() = 0;

    //показать информацию о технике
    virtual void ShowRecordInfo(const info_t& info)  = 0;
    //конструктор для интератора по энциклопедии
    virtual typename handbook_t::iterator MakeBeginIterator(const topic_t* topic = 0, unsigned flags = 0) const = 0;

private:
 
    //показать вкладку с обновлениями
    bool ShowUpdateMode()
    {
        for(size_t k = 0; k < m_topics.size(); k++){
            
            Handbook::iterator itor = MakeBeginIterator(&m_topics[k], EntityHandbook::F_NEW_REC);
            
            while(itor != Handbook::GetInst()->end()){
               
                if(m_topics[k].IsSuitable(*itor)){
                    SetCurrentTopic(k);
                    SetCurrentRecord(*itor);
                    return true; 
                }
                
                ++itor;
            }
        }

        return false;
    }

    //показать вкладку в обычном режиме
    void ShowUsualMode()
    {
        if(ShowTopic(m_current))
            return;

        for(size_t k = 0; k < m_topics.size() && !ShowTopic(k); k++);
    }

    //показать вкладку
    bool ShowTopic(int index)
    {
        Handbook::iterator itor = MakeBeginIterator(&m_topics[index]);
        
        while(itor != Handbook::GetInst()->end()){

            if(m_topics[index].IsCurrent(*itor)){                
                SetCurrentTopic(index);   
                SetCurrentRecord(*itor);
                return true; 
            }

            ++itor;
        }

        return false;
    }

    //обертка для показа темы
    void SetCurrentTopic(size_t index)
    {
        Screens::Instance()->Journal()->Database()->SetSectionName(m_topics[m_current = index].GetTitle().c_str());
    }

    //обертка для показа записи
    void SetCurrentRecord(const info_t& info)
    {
        ShowRecordInfo(info);
        m_topics[m_current].SetCurrent(&info);
        Handbook::GetInst()->DropFlags(info.GetType(), info.GetRID(), Handbook::F_NEW_REC);
    }

private:

    int         m_current;
    topic_vec_t m_topics;
};

//
// тема для раздела оборудования
//
class EquipmentTopic{
public:

    EquipmentTopic(){}

    EquipmentTopic(const std::string& title, unsigned mask) :
      m_title(title), m_mask(mask), m_cur_type(0) {}

    ~EquipmentTopic(){}

    //узнать маску для темы
    unsigned GetMask() const { return m_mask; }

    //узнать название темы раздела
    const std::string& GetTitle() const { return m_title; }

    //подходит ли информация к разделу
    bool IsSuitable(const ThingInfo& info) const 
    { return (info.GetType() & m_mask) != 0; }

    //установить текущую запись
    void SetCurrent(const ThingInfo* info)
    {
        if(info){
            m_cur_rid  = info->GetRID();
            m_cur_type = info->GetType();
            return;
        }

        m_cur_rid.clear();
        m_cur_type = 0;
    }

    //это текущая запись?
    bool IsCurrent(const ThingInfo& info) const
    { 
        return     (m_cur_rid.empty() && info.GetType() & m_mask)
                || (info.GetType() == m_cur_type && info.GetRID() == m_cur_rid);
    }

    //оператор сравнения записей
    bool IsEqual(const EquipmentTopic& t1) const
    { return m_mask == t1.m_mask ;}

    friend SavSlot& operator << (SavSlot& slot, const EquipmentTopic& topic)
    { 
        slot << topic.m_mask << topic.m_title << topic.m_cur_rid << topic.m_cur_type;
        return slot;
    }
    
    friend SavSlot& operator >> (SavSlot& slot, EquipmentTopic& topic)
    { 
        slot >> topic.m_mask >> topic.m_title >> topic.m_cur_rid >> topic.m_cur_type;
        return slot;
    }

private:

    unsigned    m_mask;
    std::string m_title;

    rid_t    m_cur_rid;
    unsigned m_cur_type;   
};

//
// раздел оборудования
//
class EquipmentUnit1 : public CommonUnit<ThingHandbook, EquipmentTopic>{
public:

    EquipmentUnit1() {}
    ~EquipmentUnit1() {}

private:

    //обновить список тем для вкладки
    void UpdateTopics()
    {
        InsertTopic(EquipmentTopic(DirtyLinks::GetStrRes("jf_armor"), TT_ARMOUR));
        InsertTopic(EquipmentTopic(DirtyLinks::GetStrRes("jf_weapons"), TT_WEAPON));
        InsertTopic(EquipmentTopic(DirtyLinks::GetStrRes("jf_ammunition"), TT_AMMO));
        InsertTopic(EquipmentTopic(DirtyLinks::GetStrRes("jf_grenades"), TT_GRENADE));
        
        unsigned equipment = TT_IMPLANT|TT_MEDIKIT/*|TT_MONEY|TT_KEY*/|TT_CAMERA|TT_SHIELD|TT_SCANNER;

        InsertTopic(EquipmentTopic(DirtyLinks::GetStrRes("jf_equipment"), equipment));        
    }

    //показать информацию о записи
    void ShowRecordInfo(const info_t& info)
    {
        DatabaseSheet::ItemInfo  traits;
        ThingInfo::traits_string str_traits;
        
        traits.m_Image = info.GetShader().c_str();
        traits.m_Name  = info.GetName().c_str();
        traits.m_Text  = info.GetDesc().c_str();
        
        info.GetTraitsString(&str_traits);
        
        traits.m_LeftColumn = str_traits.m_left_col.c_str();
        traits.m_RightColumn = str_traits.m_right_col.c_str();
        
        //установить свойство
        Screens::Instance()->Journal()->Database()->SetItemInfo(&traits, DatabaseSheet::S_OUTFIT);
    }
    
    //конструктор для интератора по энциклопедии
    handbook_t::iterator MakeBeginIterator(const topic_t* topic, unsigned flags) const
    {
        if(topic) return ThingHandbook::GetInst()->begin(topic->GetMask(), flags);

        unsigned all_items = TT_ARMOUR|TT_WEAPON|TT_AMMO|TT_GRENADE|TT_IMPLANT|TT_MEDIKIT/*|TT_MONEY|TT_KEY*/|TT_CAMERA|TT_SHIELD|TT_SCANNER; 

        return ThingHandbook::GetInst()->begin(all_items, flags);
    }
};

//
// тема для раздела оборудования
//
class VehicleTopic{
public:

    VehicleTopic(){}
    VehicleTopic(const std::string& title, int type) :
      m_title(title), m_type(type) {}

    //узнать название темы раздела
    const std::string& GetTitle() const { return m_title; }

    //подходит ли информация к разделу
    bool IsSuitable(const EntityInfo& info) const 
    { return static_cast<const VehicleInfo&>(info).GetVehicleType() == m_type; }

    //установить текущую запись
    void SetCurrent(const EntityInfo* info)
    {
        if(info)
            m_current = info->GetRID();
        else
            m_current.clear();
    }

    //это текущая запись?
    bool IsCurrent(const EntityInfo& info) const
    { 
        return      (info.GetRID() == m_current)
                ||  (m_current.empty() && IsSuitable(info));
    }

    //оператор сравнения записей
    bool IsEqual(const VehicleTopic& t1) const
    { return m_type == t1.m_type; }

    friend SavSlot& operator >> (SavSlot& slot, VehicleTopic& topic)
    { 
        slot >> topic.m_type >> topic.m_current >> topic.m_title;
        return slot;
    }

    friend SavSlot& operator << (SavSlot& slot, const VehicleTopic& topic)
    { 
        slot << topic.m_type << topic.m_current << topic.m_title;
        return slot;
    }
    
private:

    int   m_type;
    rid_t m_current;

    std::string m_title;
};

//
// раздел оборудования
//
class VehiclesUnit1 : public CommonUnit<EntityHandbook, VehicleTopic>{
public:

    VehiclesUnit1(){}
    ~VehiclesUnit1(){}

private:

    //обновить список тем для вкладки
    void UpdateTopics()
    {
        InsertTopic(VehicleTopic(DirtyLinks::GetStrRes("jf_tech"), VehicleInfo::VT_TECH));
        InsertTopic(VehicleTopic(DirtyLinks::GetStrRes("jf_robot"), VehicleInfo::VT_ROBOT));
    }

    //показать информацию о записи
    void ShowRecordInfo(const info_t& base_info)
    {
        const VehicleInfo& info = static_cast<const VehicleInfo&>(base_info);
        
        DatabaseSheet::ItemInfo item_info;

        item_info.m_Image = info.GetRID().c_str();
        item_info.m_Name  = info.GetName().c_str();
        item_info.m_Text  = info.GetDesc().c_str();

        std::ostringstream lstr, rstr;
        
        lstr << DirtyLinks::GetStrRes("jf_drive") << std::endl;      rstr << info.GetMechanics() << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_armor") << std::endl;      rstr << AIUtils::Dmg2Str(info.GetArmorType()) << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_wisdom") << std::endl;     rstr << info.GetWisdom() << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_health") << std::endl;     rstr << info.GetHealth() << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_accuracy") << std::endl;   rstr << info.GetAccuracy() << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_capacity") << std::endl;   rstr << info.GetCapacity() << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_dexterity") << std::endl;  rstr << info.GetDexterity() << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_mps") << std::endl;        rstr << info.GetMovepnts() << std::endl;
        lstr << DirtyLinks::GetStrRes("tp_bdmg") << std::endl;     rstr << info.GetBDmg().m_val << " (" << AIUtils::Dmg2Str(info.GetBDmg().m_type, false) << ")" << std::endl;
        lstr << DirtyLinks::GetStrRes("tp_admg") << std::endl;     rstr << info.GetADmg().m_val << " (" << AIUtils::Dmg2Str(info.GetADmg().m_type, false) << ")" << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_exp4kill") << std::endl;   rstr << info.GetExp4Kill() << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_exp4hack") << std::endl;   rstr << info.GetExp4Hack() << std::endl;
        
        std::string ls = lstr.str(), rs = rstr.str();

        item_info.m_LeftColumn  = ls.c_str();
        item_info.m_RightColumn = rs.c_str();

        //установить свойство
        Screens::Instance()->Journal()->Database()->SetItemInfo(&item_info, DatabaseSheet::S_TECH);
    }
    
    //конструктор для интератора по энциклопедии
    handbook_t::iterator MakeBeginIterator(const topic_t* topic, unsigned flags) const
    { return EntityHandbook::GetInst()->begin(ET_VEHICLE, flags); }
};

//
// тема для раздела оборудования
//
class HumanTopic{
public:

    HumanTopic(){}
    HumanTopic(const ForceInfo* force) : m_title(force->GetName()), m_force(force->GetRID()) {}

    //узнать название темы раздела
    const std::string& GetTitle() const
    { return m_title; }

    //подходит ли информация к разделу
    bool IsSuitable(const EntityInfo& info) const 
    { return static_cast<const HumanInfo&>(info).GetForce()->GetRID() == m_force; }

    //установить текущую запись
    void SetCurrent(const EntityInfo* info)
    {
        if(info)
            m_current = info->GetRID();
        else
            m_current.clear();
    }

    //это текущая запись?
    bool IsCurrent(const EntityInfo& info) const
    { 
        return      (info.GetRID() == m_current)
                ||  (m_current.empty() && IsSuitable(info));
    }

    //оператор сравнения записей
    bool IsEqual(const HumanTopic& t1) const
    { return m_force == t1.m_force; }

    friend SavSlot& operator >> (SavSlot& slot, HumanTopic& topic)
    { 
        slot >> topic.m_force >> topic.m_current >> topic.m_title;
        return slot;
    }

    friend SavSlot& operator << (SavSlot& slot, const HumanTopic& topic)
    { 
        slot << topic.m_force << topic.m_current << topic.m_title;
        return slot;
    }
    
private:

    rid_t m_force;
    rid_t m_current;  

    std::string m_title;
};

//
// раздел оборудования
//
class HumansUnit1 : public CommonUnit<EntityHandbook, HumanTopic>{
public:

    HumansUnit1(){}
    ~HumansUnit1(){}

    //показать вкладку
    void Show(show_mode mode)
    {
        UpdateTopics();
        CommonUnit<EntityHandbook, HumanTopic>::Show(mode);
    }

private:

    //обновить список тем для вкладки
    void UpdateTopics()
    {
        //добавим новые организации в разделы
        EntityHandbook::iterator itor = EntityHandbook::GetInst()->begin(ET_HUMAN);
        while(itor != EntityHandbook::GetInst()->end()){
            InsertTopic(HumanTopic(static_cast<const HumanInfo&>(*itor).GetForce()));
            ++itor;
        }
    }

    //показать информацию о записи
    void ShowRecordInfo(const info_t& base_info)
    {
        const HumanInfo& info = static_cast<const HumanInfo&>(base_info);

        DatabaseSheet::ItemInfo item_info;

        item_info.m_Image = info.GetRID().c_str();
        item_info.m_Name  = info.GetName().c_str();
        item_info.m_Text  = info.GetDesc().c_str();

        std::ostringstream lstr, rstr;

        lstr << DirtyLinks::GetStrRes("jf_wisdom") << std::endl;     rstr << info.GetWisdom() << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_health") << std::endl;     rstr << info.GetHealth() << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_accuracy")  << std::endl;  rstr << info.GetAccuracy() << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_dexterity") << std::endl;  rstr << info.GetDexterity() << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_strength") << std::endl;   rstr << info.GetStrength() << std::endl;
        lstr << DirtyLinks::GetStrRes("jf_exp4kill") << std::endl;   rstr << info.GetExp4Kill() << std::endl;

        std::string ls = lstr.str(), rs = rstr.str();

        item_info.m_LeftColumn  = ls.c_str();
        item_info.m_RightColumn = rs.c_str();

        //установить свойство
        Screens::Instance()->Journal()->Database()->SetItemInfo(&item_info, DatabaseSheet::S_PEOPLE);
    }
    
    //конструктор для интератора по энциклопедии
    handbook_t::iterator MakeBeginIterator(const topic_t* topic, unsigned flags) const
    { return EntityHandbook::GetInst()->begin(ET_HUMAN, flags); }
};

//
// тема для раздела оборудования
//
class ForceTopic{
public:

    ForceTopic(){}
    ForceTopic(const std::string& title, int type) : 
        m_title(title), m_type(type) {}

    //узнать название темы раздела
    const std::string& GetTitle() const { return m_title; }

    //подходит ли информация к разделу
    bool IsSuitable(const ForceInfo& info) const 
    { return info.GetForceType() == m_type; }

    //установить текущую запись
    void SetCurrent(const ForceInfo* info)
    {
        if(info)
            m_current = info->GetRID();
        else
            m_current.clear();
    }

    //это текущая запись?
    bool IsCurrent(const ForceInfo& info) const
    { 
        return      (info.GetRID() == m_current)
                ||  (m_current.empty() && IsSuitable(info));
    }

    //оператор сравнения записей
    bool IsEqual(const ForceTopic& t1) const
    { return m_type == t1.m_type; }

    friend SavSlot& operator >> (SavSlot& slot, ForceTopic& topic)
    { 
        slot >> topic.m_type >> topic.m_current >> topic.m_title;
        return slot;
    }

    friend SavSlot& operator << (SavSlot& slot, const ForceTopic& topic)
    { 
        slot << topic.m_type << topic.m_current << topic.m_title;
        return slot;
    }
    
private:

    int   m_type;
    rid_t m_current;  

    std::string m_title;
};

//
// раздел оборудования
//
class ForcesUnit1 : public CommonUnit<ForceHandbook, ForceTopic>{
public:

    ForcesUnit1(){}
    ~ForcesUnit1(){}

private:

    //обновить список тем для вкладки
    void UpdateTopics()
    {
        InsertTopic(ForceTopic(DirtyLinks::GetStrRes("jf_private"), ForceInfo::FT_PRIVATE));
        InsertTopic(ForceTopic(DirtyLinks::GetStrRes("jf_goverment"), ForceInfo::FT_GOVERMENT));
    }

    //показать информацию о записи
    void ShowRecordInfo(const info_t& info)
    {
        DatabaseSheet::ItemInfo item_info;

        item_info.m_Image = info.GetRID().c_str();
        item_info.m_Name  = info.GetName().c_str();
        item_info.m_Text  = info.GetDesc().c_str();

        item_info.m_LeftColumn  = "";
        item_info.m_RightColumn = "";

        //установить свойство
        Screens::Instance()->Journal()->Database()->SetItemInfo(&item_info, DatabaseSheet::S_FORCES);
    }
    
    //конструктор для интератора по энциклопедии
    handbook_t::iterator MakeBeginIterator(const topic_t* topic, unsigned flags) const
    { return ForceHandbook::GetInst()->begin(0, flags); }
};

JF_HandbookSheet::Unit* JF_HandbookSheet::Unit::Create(unit_type type)
{
    if(type == UT_HUMANS) return new HumansUnit1();
    if(type == UT_FORCES) return new ForcesUnit1();
    if(type == UT_VEHICLES) return new VehiclesUnit1();
    if(type == UT_EQUIPMENT) return new EquipmentUnit1();

    return 0;
}

//================================================================

//
// вкладка новостей
//
class JF_NewsSheet : public  JournalFormImp3::Sheet,
                     private GameObserver,
                     private JournalScreen::SheetController
{
public:
        
    JF_NewsSheet() : m_news_itor(0), m_cur_news(0)
    {
        GameEvMessenger::GetInst()->Attach(this, EV_NEW_NEWS);
        m_news_itor = NewsPool::GetInst()->CreateIterator();
    }

    ~JF_NewsSheet()
    {
        GameEvMessenger::GetInst()->Detach(this);
        delete m_news_itor;
    }
        
    void Show(show_mode mode)
    {
        Screens::Instance()->Journal()->News()->SetController(this);
        Screens::Instance()->Journal()->Activate(JournalScreen::SID_NEWS);

        //соберем информацию о каналах
        m_channel.clear();
        m_news_itor->First();
        while(m_news_itor->IsNotDone()){

            if(GetChannelPos(m_news_itor->Get()->GetChannel()) < 0){
                m_channel.push_back(m_news_itor->Get()->GetChannel());
            }

            m_news_itor->Next();
        }

        //покажем последнюю новость
        m_news_itor->First();
        ShowNewsInfo(m_news_itor->Get());        
    }        

    bool IsUpdated() const { return false; }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving()){
			m_cur_news = 0;
            slot << (m_cur_news != 0);
            if(m_cur_news) slot << m_cur_news->GetRID();

            slot << m_channel.size();
            for(size_t k = 0; k < m_channel.size(); slot << m_channel[k++]);

        }else{

            m_cur_news = 0;
            m_channel.clear();

            bool flag; slot >> flag;
            if(flag){

                rid_t rid; slot >> rid;

                m_news_itor->First();
                while(m_news_itor->IsNotDone()){

                    if(m_news_itor->Get()->GetRID() == rid){
                        m_cur_news = m_news_itor->Get();
                        break;
                    }

                    m_news_itor->Next();
                }
            }

            size_t size; slot >> size;
            while(size--){
                m_channel.push_back(rid_t());
                slot >> m_channel.back();
            }
        }
    }

private:

    void OnButtonClick(JournalScreen::BUTTON id)
    {
        if(m_cur_news == 0) return;

        switch(id){
        case JournalScreen::B_UP:
            {
                size_t pos = GetChannelPos(m_cur_news->GetChannel());
                
                int iterations = m_channel.size();
                while(iterations--){

                    //перемотаем каналы вперед
                    if(++pos >= m_channel.size()) pos = 0;         
                    
                    //найдем последнюю новость этого канала
                    if(ShowLastChannelNews(m_channel[pos]))
                        return;
                }
            }
            break;

        case JournalScreen::B_DOWN:
            {
                int pos = GetChannelPos(m_cur_news->GetChannel());
                
                int iterations = m_channel.size();
                while(iterations--){

                    //перемотаем каналы назад
                    if(--pos < 0) pos = m_channel.size() - 1;
                    
                    //найдем последнюю новость этого канала
                    if(ShowLastChannelNews(m_channel[pos]))
                        return;
                }
            }
            break;
		
        case JournalScreen::B_LEFT:
            {
                m_news_itor->First();
                const NewsPool::Info* prev = 0;

                //найдем предид. новость в списке
                while(m_news_itor->IsNotDone()){

                    if(m_news_itor->Get()->GetRID() == m_cur_news->GetRID())
                        break;

                    if(m_news_itor->Get()->GetChannel() == m_cur_news->GetChannel())
                        prev = m_news_itor->Get();

                    m_news_itor->Next();
                }   
                
                if(prev) ShowNewsInfo(prev);
            }
            break;
		
        case JournalScreen::B_RIGHT:
           {
                m_news_itor->First();

                //найдем текущую новость в списке
                while(m_news_itor->IsNotDone()){

                    if(m_cur_news->GetRID() == m_news_itor->Get()->GetRID())
                        break;

                    m_news_itor->Next();
                }   

                if(m_news_itor->IsNotDone()) m_news_itor->Next();

                //найдем следущую новость
                while(m_news_itor->IsNotDone()){

                    if(m_news_itor->Get()->GetChannel() == m_cur_news->GetChannel()){
                        ShowNewsInfo(m_news_itor->Get());
                        break;
                    }

                    m_news_itor->Next();
                }
            }
            break;
        }
    }

    int GetChannelPos(const rid_t& rid) const
    {
        for(size_t k = 0; k <  m_channel.size(); k++){
            if(m_channel[k] == rid) return k;
        }

        return -1;
    }

    void ShowNewsInfo(const NewsPool::Info* ptr)
    {
        NewsSheet* sheet = Screens::Instance()->Journal()->News();

        if(ptr == 0){
            m_cur_news = 0;
            sheet->SetTraits(0);
            return;
        }

        m_cur_news = ptr;

        NewsSheet::Traits traits;

        traits.m_Image = ptr->GetShader().c_str();
        traits.m_Content = ptr->GetDesc().c_str();
        traits.m_Channel = ptr->GetChannel().c_str();

        sheet->SetTraits(&traits);
    }

    bool ShowLastChannelNews(const rid_t& channel)
    {
        m_news_itor->First();
        while(m_news_itor->IsNotDone()){
            
            if(m_news_itor->Get()->GetChannel() == channel){
                ShowNewsInfo(m_news_itor->Get());
                return true;
            }
            
            m_news_itor->Next();
        }
        
        return false;
    }

    void Update(subject_t subj, event_t event, info_t info)
    {
        m_cur_news = 0;
        m_channel.clear();
    }

private:

    NewsPool::Iterator*   m_news_itor;
    const NewsPool::Info* m_cur_news;

    typedef std::vector<rid_t> channel_vec_t;
    channel_vec_t  m_channel;
};

//================================================================

} //namespace

//================================================================

JournalFormImp3::JournalFormImp3() :
    m_update_time(0), m_delta_time(0.3f)
{
    memset(m_sheet, 0, sizeof(m_sheet));

    m_sheet[JournalScreen::S_NEWS] = new JF_NewsSheet();
    m_sheet[JournalScreen::S_JOURNAL] = new JF_DiarySheet();
    m_sheet[JournalScreen::S_DATABASE] = new JF_HandbookSheet();
    m_sheet[JournalScreen::S_NAVIGATION] = new JF_NavigationSheet();
}

JournalFormImp3::~JournalFormImp3()
{
    for(int k = 0; k < MAX_SHEETS; delete m_sheet[k++]);
}

void JournalFormImp3::Show()
{
    Screens::Instance()->Journal()->SetController(this);
    Screens::Instance()->Activate(Screens::SID_JOURNAL);

    //активируем вкладку news
    OnSheetClick(JournalScreen::S_NEWS);
}

void JournalFormImp3::HandleInput(state_type* st)
{   
    if(m_update_time < Timer::GetSeconds()){

        JournalScreen* screen = Screens::Instance()->Journal();

        m_update_time = Timer::GetSeconds() + m_delta_time;

        screen->ActivateUpdateButton(JournalScreen::UB_JOURNAL, CalcUpdateBtnState(m_sheet[JournalScreen::S_JOURNAL]));        
        screen->ActivateUpdateButton(JournalScreen::UB_DATABASE, CalcUpdateBtnState(m_sheet[JournalScreen::S_DATABASE]));        
        screen->ActivateUpdateButton(JournalScreen::UB_NAVIGATION, CalcUpdateBtnState(m_sheet[JournalScreen::S_NAVIGATION]));        
    }
}

void JournalFormImp3::OnBack(void)
{
    HandleExitFormReq();
}

void JournalFormImp3::OnSheetClick(JournalScreen::SHEET id)
{
    if(m_sheet[id]) m_sheet[id]->Show(Sheet::SM_USUAL);
}

void JournalFormImp3::HandleExitFormReq()
{
    Forms::GetInst()->ShowGameForm();
}

void JournalFormImp3::OnUpdateClick(JournalScreen::UPDATE_BUTTON id)
{
    switch(id){
    case JournalScreen::UB_DATABASE:
        if(m_sheet[JournalScreen::S_DATABASE]){
            m_sheet[JournalScreen::S_DATABASE]->Show(Sheet::SM_UPDATES);
            Screens::Instance()->Journal()->IlluminateSheet(JournalScreen::S_DATABASE);
        }
        break;

    case JournalScreen::UB_JOURNAL:
        if(m_sheet[JournalScreen::S_JOURNAL]){
            m_sheet[JournalScreen::S_JOURNAL]->Show(Sheet::SM_UPDATES);
            Screens::Instance()->Journal()->IlluminateSheet(JournalScreen::S_JOURNAL);
        }
        break;

    case JournalScreen::UB_NAVIGATION:
        if(m_sheet[JournalScreen::S_NAVIGATION]){
            m_sheet[JournalScreen::S_NAVIGATION]->Show(Sheet::SM_UPDATES);
            Screens::Instance()->Journal()->IlluminateSheet(JournalScreen::S_NAVIGATION);
        }
        break;
    }
}

bool JournalFormImp3::IsUpdated() const
{
    for(int k = 0; k < MAX_SHEETS; k++){
        if(m_sheet[k] && m_sheet[k]->IsUpdated())
            return true;
    }

    return false;
}

void JournalFormImp3::Init(const ini_s& ini)
{
}

void JournalFormImp3::MakeSaveLoad(SavSlot& slot)
{
    for(int k = 0; k < MAX_SHEETS; k++){
        if(m_sheet[k]) m_sheet[k]->MakeSaveLoad(slot);
    }
}
