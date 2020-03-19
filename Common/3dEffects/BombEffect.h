/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: взрыв бомбы
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__BOMBEFFECT_H__)
#define __BOMBEFFECT_H__

#include "Effect.h"
#include "Particle.h"

#define SEQUENCE_LEN 10

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс последовательности нескольких частиц
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParticleSequence
{
public:
	// координаты главной частицы
	point3 coords;
	// скорость главной частицы
	point3 velocity;
	// ускорение главной частицы
	point3 accel;
	// первоначальный размер
	float size;
	// место рождения главной частицы
	point3 start_point;
	// время жизни
	float lifetime;
	// массив частиц
	BaseParticle sequence[SEQUENCE_LEN];
	// проинициализировать последовательность
	void Rebirth(
		const point3 st_point,         // начальная точка
		const point3 stVelocity,       // начальная скорость
		const point3 Accel,            // ускорение
		const float stSize,	              // начальный размер
		const float stRange,           // случайная часть начального размера
		const float Lifetime,              // время жизни
		const bool bFull                  // если true, то полная сфера, иначе - половина
		);
	// следующий тик времени
	void NextTick(const float dt);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс разлетающихся последовательностей частиц
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BombSparkles : public BaseEffect
{
public:
	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }
	// эффект закончил свою работу?
	virtual bool IsFinished() const { return finished; }

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// конструктор с параметрами
	BombSparkles(
		const point3 rp,               // корневая точка
		const BOMB_SPARKLES_INFO& info
		);
	// деструктор
	virtual ~BombSparkles();

protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//

	
	float prev_time;                       // предыдущее текущее время
	float start_time;                       // время начала эффекта
	point3 root_point;                    // корневая точка
	point3 start_velocity;                // начальная скорость
	point3 accel;                           // ускорение
	float start_size;                        // начальный размер
	float start_size_range;              // случайная часть начального размера
	float size_factor;                      // изменение размера
	float lifetime;               // время жизни эффекта
	bool full;                  // если true, то полная сфера, иначе - половина

	// количество последовательностей в массиве
	unsigned int sequences_num;

	// указатель на массив последовательностей
	ParticleSequence * sequences;

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;

	// функция размножения точек
	void Multiply();

	point3 vel;
};

#endif // !defined(__BOMBEFFECT_H__)
