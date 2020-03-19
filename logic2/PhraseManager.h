//
// сервисы для работы с фразами персонажей
//

#ifndef _PUNCH_PHRASEMANAGER_H_
#define _PUNCH_PHRASEMANAGER_H_

//
// фраза которую говорит персонаж
//
struct phrase_s{
    std::string m_text;
    std::string m_sound;

    phrase_s(){}

    //создание фразы путем парсинга строки
    phrase_s(const std::string& str4parse);
    //явное создание фразы
    phrase_s(const std::string& txt, const std::string& snd) :
        m_text(txt), m_sound(snd) {}
};

// 
// менеджер фраз для персонажей 
//
class PhraseManager {
public:

    virtual ~PhraseManager(){}

    //singleton
    static PhraseManager* GetUsePhrases();
    static PhraseManager* GetFirstPhrases();

    //инициализация/деинициализация
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //запись/чтение из Save'a
    virtual void MakeSaveLoad(SavSlot& st) = 0;

    //можно ли сказать фразу
    virtual bool CanSay(BaseEntity* entity) = 0;
    //пометить фразу как сказанную
    virtual void MarkAsSaid(BaseEntity* entity) = 0;

    //существует ли фраза
    virtual bool IsExists(BaseEntity* entity) = 0;
    //получить фразу
    virtual const phrase_s& GetPhrase(BaseEntity* entity) = 0;
};    

#endif // _PUNCH_PHRASEMANAGER_H_