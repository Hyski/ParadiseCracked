/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: класс дыма от выстрела
				
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__SHOOTSMOKEEFFECT_H__)
#define __SHOOTSMOKEEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// класс частицы
//
class ShootSmokeParticle : public BaseParticle
{
public:
	point3 velocity;
	float size_vel;
	ShootSmokeParticle(): velocity(0, 0, 0), size_vel(0)
	{}
};

//
// класс дыма от выстрела
//


class ShootSmokeEffect : public BaseEffect  
{
public:

	// конструктор с параметрами
	ShootSmokeEffect(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const SHOOT_SMOKE_EFFECT_INFO& info
		);

	// деструктор
	virtual ~ShootSmokeEffect();


	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// эффект закончил свою работу?
	virtual bool IsFinished() const { return finished; }

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float) { return bbox; }


protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	// функция размножения точек
	virtual void Multiply();

	//
	// скрытые данные эффекта
	//

	// список частиц
	ShootSmokeParticle* particles;
	int particles_num;                        // текущее количество частиц

	float start_time;                         // время начала эффекта
	point3 root_point;                        // начальная точка
	point3 direction;                         // направление выстрела (единичный вектор)
	unsigned int color;                       // цвет
	
	
	point3 accel;                             // ускорение

	float start_size;                         // начальный размер
	float size_velocity;                      // скорость изменения размера


	float lifetime;                           // время жизни эффекта

	BBox bbox;

	point3 velocity;                          // скорость
	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};


#endif // !defined(__SHOOTSMOKEEFFECT_H__)
