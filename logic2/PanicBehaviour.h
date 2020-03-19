//
// ������ ��������� �������� � ������
//

#ifndef _PUNCH_PANICBEHAVIOUR_H
#define _PUNCH_PANICBEHAVIOUR_H

class HumanEntity;

//
// ���������� ��������� � ������
//
class PanicBehaviour{
public:

    virtual ~PanicBehaviour() {}

    //������������� ������ ���������
    virtual void Init(HumanEntity* human) = 0;

    // ��������� ������ ��� ��������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:  
    //           true   -  ��������� ��������� ����� 
    //           false  -  ������ ��������� ��������� ���������
    //
    // ��������: 
    //           �� ��������� ������ ���-�� movepoints �.�. == 0
    //
    virtual bool Panic(HumanEntity* human) = 0;
};

//
// ������� ������� ��������� � ������
//
class PanicBehaviourFactory{
public:

    //singleton
    static PanicBehaviourFactory* GetInst();

    //������� ������� ���������
    PanicBehaviour* CreateShockBehaviour();
    //������� ������� ������
    PanicBehaviour* CreateUsualBehaviour();
    //������� ������ ��������
    PanicBehaviour* CreateBerserkBehaviour();

private:
    
    PanicBehaviourFactory(){}

	struct Container
	{
		PanicBehaviourFactory* m_instance;
		Container() { m_instance = 0; }
		~Container() { delete m_instance; }
	};

	static Container m_container;
};

#endif // _PUNCH_PANICBEHAVIOUR_H