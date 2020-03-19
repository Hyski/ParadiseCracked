/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: огонь (эффект)
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__FIREEFFECT_H__)
#define __FIREEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// класс частицы, используемый дл€ огн€
//

class FireParticle : public BaseParticle
{
public:
	// скорость частицы
	point3 velocity;
	// текстурные координаты частицы
	texcoord texcoords1;
	texcoord texcoords2;
	// врем€ рождени€ частицы
	float birth_time;
	// врем€ жизни частицы
	float life_time;
};

//
// класс огн€
//

class FireEffect : public BaseEffect  
{
public:

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

	// конструктор с параметрами
	FireEffect(
		const float quality,
		const point3 rp,			  // начальна€ точка
		const FIRE_EFFECT_INFO& info
		);

	// деструктор
	virtual ~FireEffect();

	// эффект закончил свою работу?
	// (по умолчанию эффект работает бесконечно)
	virtual bool IsFinished() const { return finished; }

	// сообщение эффекту о начале нового турна игрока
	virtual void NewTurn() { if(turns_left) turns_left--; }

	// сообщение эффекту об изменении позиции
	virtual void SetPosition(const point3& Pos) { root_point = Pos; }

	// сообщение эффекту об изменении интенсивности
	virtual void SetActivity(const float Activity) { activity = Activity; }

	// сообщение эффекту о необходимости плавно погаснуть и умереть
	virtual void Finish() { turns_left = 0; }

protected:
	// с вызова этого метода начинаетс€ отсчет
	// времени дл€ расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//

	// оставшеес€ количество туров до завершени€ эффекта
	int turns_left;
	// флаг режима гашени€ эффекта
	bool reduce;
	// врем€ начала гашени€
	float start_reduce_time;
	// врем€ гашени€
	float reduce_time;
	// величина интенсивности эффекта (€ркости)
	float activity;

	// массив частиц
	FireParticle * particles;
	// количество частиц в массиве
	unsigned int particles_num;

	// точка, из которой горит огонь
	point3 root_point;
	// врем€ начала проработки эффекта
	float start_time;

	// максимальна€ составл€юща€ скорости по ’(Y) при старте эффекта
	float startDX;
	// максимальна€ составл€юща€ скорости по Z при старте эффекта
	float startDZ;
	// минимальный начальный размер
	float min_start_size;
	// предел изменени€ начальногo размера
	float size_range;
	// коэффициент изменени€ размера частицы в зависимости от времени
	float size_factor;
	// максимальное врем€ жизни частицы
	float max_lifetime;
	// флаг окончани€ работы
	bool finished;

	// функци€ размножени€ точек
	void Multiply();
	// функци€ обновл€юща€ частицу при ее рождении
	void Rebirth(FireParticle & sp, float curtime, float disp);

	point3 vel;
};

#endif // !defined(__FIREEFFECT_H__)
