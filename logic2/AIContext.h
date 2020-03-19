//
// данные AI связанные с каждым существом
//

#ifndef _PUNCH_AICONTEXT_H_
#define _PUNCH_AICONTEXT_H_

//
// контекст AI связанный с существом
//
class AIContext{
public:

    DCTOR_ABS_DEF(AIContext)

    virtual ~AIContext(){}

    //система сохранения/загрузки
    virtual void MakeSaveLoad(SavSlot& slot) = 0;
    //узнать подкоманду к которой принадл существо
    virtual const std::string& getSubteam() const = 0;
};

//
// фабрика создающая контекст AI
//
class AIContextFactory{
public:

    //singleton
    static AIContextFactory* GetInst();

    enum ai_type{
        AI_ENEMY,       //контекст для врага
        AI_CIVILIAN,    //контекст для гражданских
    };

    //создать контекст AI соотв типа
    AIContext* Create(ai_type type, const std::string& subteam);

private:

    AIContextFactory(){}
};

#endif // _PUNCH_AICONTEXT_H_