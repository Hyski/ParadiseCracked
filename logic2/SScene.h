//
// менеджер для проигрывания скриптовых сцен
//

#ifndef _PUNCH_SSCENE_H_
#define _PUNCH_SSCENE_H_

class Activity;

//
// класс для порождения и проигрывания скриптовых сцен
//
class SSceneMgr{
public:

    virtual ~SSceneMgr(){}

    static SSceneMgr* GetInst();

    //инициализация/деинициаозация
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //породить скриптовую сцену для проигрывания
    virtual Activity* CreateScriptScene() = 0;
    //сохранить скриптовыве сцены
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //была ли проиграна скриптовая сцена
    virtual bool IsScriptSceneFinished(const rid_t& rid) const = 0;
};

#endif // _PUNCH_SSCENE_H_