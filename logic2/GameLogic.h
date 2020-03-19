//
//так игра видит модуль искусств. интеллекта
//

#ifndef _PUNCH_GAMELOGIC_H_
#define _PUNCH_GAMELOGIC_H_

class Storage;

//
// интерфейс AI игры
//
class GameLogic{
public:

    virtual ~GameLogic(){}

    static GameLogic* GetInst();

    //инициализация/деинициализация
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //начало обычной игры
    virtual void BegNewGame() = 0;               
    //начало сетевой игры
    virtual void BegNetGame() = 0;

    //отрисовка чего - либо
    virtual void Draw() = 0;
    //AI думает в этом tick'е
    virtual void Think() = 0;

    //загрузка/сохранение игры
    virtual void MakeSaveLoad(Storage& st) = 0;  

protected:

    GameLogic(){}
};

#endif // _PUNCH_GAMELOGIC_H_