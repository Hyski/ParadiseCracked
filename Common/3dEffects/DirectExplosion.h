
/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект направленного взрыва
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#if !defined (__DIRECTEXPLOSION_EFFECT_H__)
#define __DIRECTEXPLOSION_EFFECT_H__

#include "Effect.h"
#include "Particle.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс последовательности нескольких частиц
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DirectSparkleSequence
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
	BaseParticle *sequence;
	// размер массива частиц
	unsigned int seq_len;

	// конструктор
	DirectSparkleSequence(const unsigned int SeqLen);
	~DirectSparkleSequence();
	
	// проинициализировать последовательность
	void Rebirth(
		const point3 st_point,         // начальная точка
		const point3 vecVelocity,    // вектор направления скорости
		const point3 DV,                // разброс скорости
		const point3 Accel,            // ускорение
		const float stSize,	              // начальный размер
		const float stRange,           // случайная часть начального размера
		const float Lifetime              // время жизни
		);
	
	// следующий тик времени
	void NextTick(const float dt);
	
};

typedef DirectSparkleSequence * PDirectSparkleSequence;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс направленных искр направленного взрыва
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DirectSparkles : public BaseEffect
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
	virtual ~DirectSparkles();
	// конструктор с параметрами
	DirectSparkles(
		const point3 rp,                 // корневая точка
		const point3 vecVelocity,        // вектор направления скорости
		const DIRECT_SPARKLES_INFO &info
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
	float lifetime;               // время жизни эффекта
	unsigned int seq_len;               // длина последовательности

	// количество последовательностей в массиве
	unsigned int sequences_num;

	// массив указателей на последовательность
	PDirectSparkleSequence * sequences;

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;

	// функция размножения точек
	void Multiply();
};


//
// класс частицы, используемый для направленного взрыва
//
class DirectParticle : public BaseParticle
{
public:
	// скорость частицы
	point3 velocity;
	// ускорение частицы
	point3 accel;
	// место рождения частицы
	point3 start_point;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс направленного дыма
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DirectExplosion : public BaseEffect
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
	virtual ~DirectExplosion();
	// конструктор с параметрами
	DirectExplosion(
		const point3 rp,             // корневая точка
		const point3 vecVelocity,    // вектор направления скорости
		const DIRECT_EXPLOSION_INFO& info
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

	// массив частиц
	DirectParticle * particles;
	// количество частиц в массиве
	unsigned int particles_num;

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;

	// функция размножения точек
	void Multiply();

	// функция обновляющая частицу при ее рождении
	void Rebirth(DirectParticle & dp, float curtime);
};


#endif // __DIRECTEXPLOSION_EFFECT_H__                                                                                
