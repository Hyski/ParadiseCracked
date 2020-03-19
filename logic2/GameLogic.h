//
//��� ���� ����� ������ ��������. ����������
//

#ifndef _PUNCH_GAMELOGIC_H_
#define _PUNCH_GAMELOGIC_H_

class Storage;

//
// ��������� AI ����
//
class GameLogic{
public:

    virtual ~GameLogic(){}

    static GameLogic* GetInst();

    //�������������/���������������
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //������ ������� ����
    virtual void BegNewGame() = 0;               
    //������ ������� ����
    virtual void BegNetGame() = 0;

    //��������� ���� - ����
    virtual void Draw() = 0;
    //AI ������ � ���� tick'�
    virtual void Think() = 0;

    //��������/���������� ����
    virtual void MakeSaveLoad(Storage& st) = 0;  

protected:

    GameLogic(){}
};

#endif // _PUNCH_GAMELOGIC_H_