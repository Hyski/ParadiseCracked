/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: класс для ошметков мяса
				
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__MEATEFFECT_H__)
#define __MEATEFFECT_H__

#include "Effect.h"
#include "Particle.h"

#include "../../Skin/skanim.h"

// структура для хранения информации об одной модельке
struct MeatEffectParticle;
//
// класс частицы для капелек крови
//
class BloodParticle : public BaseParticle
{
public:
	point3 start_point;
	point3 velocity;
	float birthtime;
	float lifetime;
	int texcoord_type;                        // тип размножения текстурных координат
	BloodParticle(): start_point(0, 0, 0), velocity(0, 0, 0), birthtime(0), lifetime(0), texcoord_type(0)
	{}
};

// тип - список частиц
typedef std::list<BloodParticle> BloodParticlesList;
// тип - итератор для списка
typedef BloodParticlesList::iterator BloodParticlesListIterator;


//
// эффект ошметков мяса (полет моделек)
//

class MeatEffect : public BaseEffect
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }

	// конструктор с параметрами
	MeatEffect(
		const unsigned int entity_id,
		const point3& explosion,
		const point3& human,
		const MEAT_EFFECT_INFO& info,
		float angle_vel,
		float vel = 2.0f
		);

	// деструктор
	virtual ~MeatEffect();

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// эффект закончил свою работу?
	virtual bool IsFinished() const { return finished; }

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time) { return bbox; }
protected:
	virtual bool Start(const float );

	// функция размножения точек
	virtual void Multiply();

	//
	// скрытые данные эффекта
	//

	// список частиц
	BloodParticlesList particles;
	int particles_num;                        // текущее количество частиц
	float * size_vector;                      // вектор с размерами частиц
	unsigned int color;                       // цвет
	float start_size;                         // начальный размер
	float end_size;                           // конечный размер
	float particles_lifetime;                 // время жизни частичек
	int frequency;                            // частота появления (количество новых частиц в секунду)
	float previous_birth_time;                // время рождения предыдущей частицы (серии частиц)

	MeatEffectParticle* m_models;
	BBox bbox;
	float start_time;                         // время начала эффекта
	float lifetime;                           // время жизни эффекта
	AnimaData m_animadata;    // структурка с информацией
	// разница во времени между текущим временем и временем рождения эффекта
	float m_dt; 

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

#endif // !defined(__MEATEFFECT_H__)
