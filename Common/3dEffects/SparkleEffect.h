
/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект направленных искр
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#if !defined (__SPARKLE_EFFECT_H__)
#define __SPARKLE_EFFECT_H__

#include "Effect.h"
#include "Particle.h"

#define SEQUENCE_LEN1 6 // длина последовательности частиц

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс последовательности нескольких частиц
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SparkleSequence
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
	BaseParticle sequence[SEQUENCE_LEN1];
	
	
	// проинициализировать последовательность
	void Rebirth(
		const point3 st_point,       // начальная точка
		const point3 vecVelocity,    // вектор направления скорости
		const point3 DV,             // разброс скорости
		const point3 Accel,          // ускорение
		const float stSize,	         // начальный размер
		const float stRange,         // случайная часть начального размера
		const float Lifetime         // время жизни
		);
	
	// следующий тик времени
	void NextTick(const float dt);
	
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс направленных искр
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class OrientedSparkles : public BaseEffect
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

	// деструктор
	virtual ~OrientedSparkles();
	// конструктор с параметрами
	OrientedSparkles(
		const float quality,         // качество
		const point3 rp,             // корневая точка
		const point3 vecVelocity,    // вектор направления скорости
		const point3 DV,             // разброс скорости
		const point3 Accel,          // ускорение
		const float stSize,	         // начальный размер
		const float stRange,         // случайная часть начального размера
		const float sizeFactor,      // изменение размера
		const float LifeTime,        // время жизни эффекта
		const std::string& Shader,   // имя шейдера
		const std::string& Sound,    // имя звукового шейдера
		const bool SoundMove         // двигается ли звук
		);

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
	point3 vec_velocity;                 // вектор направления скорости
	point3 dv;                               // разброс скорости
	point3 accel;                           // ускорение
	float start_size;                        // начальный размер
	float start_size_range;              // случайная часть начального размера
	float size_factor;                      // изменение размера
	float lifetime;               // время жизни эффекта

	// количество последовательностей в массиве
	unsigned int sequences_num;

	// указатель на массив последовательностей
	SparkleSequence * sequences;

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;

	// функция размножения точек
	void Multiply();
};


#endif // __SPARKLE_EFFECT_H__                                                                                
