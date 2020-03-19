//
// все что необходимо для абстракции игрока
//

#ifndef _PUNCH_PLAYER_H_
#define _PUNCH_PLAYER_H_

class BasePlayer;

//
// фабрика игроков
//
class PlayerFactory{
public:

    static PlayerFactory* GetInst();

    //создать игрока - человека
    BasePlayer* CreateHumanPlayer();
    //создать врагов 
    BasePlayer* CreateEnemyPlayer();
    //создать штатских
    BasePlayer* CreateCivilianPlayer();

private:

    PlayerFactory(){}
};

//
// базовый класс для хранилища игроков
//
class PlayerPool{
public:

    ~PlayerPool();

    //singleton
    static PlayerPool* GetInst();

    //сбросить всех игроков
    void Reset();
    
    //вызывается раз в кадр
    void Think();
    //сохранить игроков
    void MakeSaveLoad(SavSlot& st);
    //добавить игрока в массив игроков
    void Insert(BasePlayer* player);

protected:

    PlayerPool();

private:

    unsigned    m_current;            //текущий игрок
    BasePlayer* m_players[MAX_TEAMS];
};

#endif // _PUNCH_PLAYER_H_