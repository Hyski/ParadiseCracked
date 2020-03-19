#ifndef _QUESTSERVER_H_
#define _QUESTSERVER_H_

class BaseEntity;

//
// механизм для работы с quest'ами
//
class QuestServer{
public:

    virtual ~QuestServer() {}

    //singleton
    static QuestServer* GetInst();

    enum sheme_type{
        ST_START_NEW_GAME,   
        ST_CHANGE_EPISODE,
    };

    //закрытие сервера
    virtual void Shut() = 0;
    //инициализация сервера
    virtual void Init(sheme_type sheme, int episode) = 0;

    //загрузка/сохранение quest'ов
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //общий алгоритм для use на существе
    virtual void HandleUseEntity(BaseEntity* user, BaseEntity* used) = 0;
    //сказать первую фразу
    virtual void HandleFirstPhrase(BaseEntity* actor, BaseEntity* entity) = 0;
    //обработка убийства существа
    virtual void HandleKillEntity(BaseEntity* killer, BaseEntity* victim) = 0;

    virtual void TestSmth() = 0;
};

#endif _QUESTSERVER_H_
