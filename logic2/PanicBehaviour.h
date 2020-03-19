//
// модели поведения человека в панике
//

#ifndef _PUNCH_PANICBEHAVIOUR_H
#define _PUNCH_PANICBEHAVIOUR_H

class HumanEntity;

//
// абстракция поведения в панике
//
class PanicBehaviour{
public:

    virtual ~PanicBehaviour() {}

    //инициализация модели поведения
    virtual void Init(HumanEntity* human) = 0;

    // проиграть панику для человека
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:  
    //           true   -  необходим повторный вызов 
    //           false  -  модель поведения проиграна полностью
    //
    // ВНИМАНИЕ: 
    //           по окончанию паники кол-во movepoints д.б. == 0
    //
    virtual bool Panic(HumanEntity* human) = 0;
};

//
// фабрика моделей поведения в панике
//
class PanicBehaviourFactory{
public:

    //singleton
    static PanicBehaviourFactory* GetInst();

    //создать шоковое поведение
    PanicBehaviour* CreateShockBehaviour();
    //создать обычную панику
    PanicBehaviour* CreateUsualBehaviour();
    //создать панику берсерка
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