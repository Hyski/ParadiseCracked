/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: взрывы
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__EXPLOSIONEFFECT_H__)
#define __EXPLOSIONEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// класс частицы, используемый для взрыва
//
class ExplosionParticle : public BaseParticle
{
public:
	// скорость частицы
	point3 velocity;
	// место рождения частицы
	point3 start_point;
};


//
// класс взрыва
//

class ExplosionEffect : public BaseEffect  
{
public:

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// деструктор
	virtual ~ExplosionEffect() {}

	// эффект закончил свою работу?
	virtual bool IsFinished() const { return finished; }

protected:
	// функция размножения точек
	void Multiply();
	// конструктор по умолчанию
	ExplosionEffect() {};

	//
	// скрытые данные эффекта
	//

	// массив частиц
	ExplosionParticle * particles;
	// количество частиц в массиве
	unsigned int particles_num;
	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
	point3 root_point;                       // корневая точка
	point3 start_velocity;                // начальная скорость
};

//
// класс взрыва гранаты
//

class GrenadeEffect : public ExplosionEffect  
{
public:

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

	// деструктор
	virtual ~GrenadeEffect();
	// конструктор с параметрами
	GrenadeEffect(const float quality,  // качество
		const point3 rp,                // корневая точка
		const GRENADE_EFFECT_INFO& info
		);

protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//

	float start_time;                         // время начала эффекта

	point3 accel;                           // ускорение
	float start_size;                        // начальный размер
	float start_size_range;              // случайная часть начального размера
	float size_factor;                      // изменение размера
	float lifetime;               // время жизни эффекта

	// функция размножения точек
	void Multiply();
	// функция обновляющая частицу при ее рождении
	void Rebirth(ExplosionParticle & ep, float curtime);
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс разлетающихся искр или осколков
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SparkleEffect : public ExplosionEffect  
{
public:

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

	// деструктор
	virtual ~SparkleEffect();
	// конструктор с параметрами
	SparkleEffect(
		const point3 rp,                // корневая точка
		const SPARKLE_EFFECT_INFO& info
		);

protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//

	
	float prev_time;                         // предыдущее текущее время
	float start_time;                         // время начала эффекта

	point3 accel;                           // ускорение
	float start_size;                        // начальный размер
	float start_size_range;              // случайная часть начального размера
	float size_factor;                      // изменение размера
	float lifetime;               // время жизни эффекта
	bool full;                  // если true, то полная сфера, иначе - половина


	// функция размножения точек
	void Multiply();
	// функция обновляющая частицу при ее рождении
	void Rebirth(ExplosionParticle & ep, float curtime);
};



//
// класс дыма для взрывов
//

class ExplosionSmokeEffect : public ExplosionEffect  
{
public:

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

	// деструктор
	virtual ~ExplosionSmokeEffect();
	// конструктор с параметрами
	ExplosionSmokeEffect(const float quality,  // качество
		const point3 rp,                       // корневая точка
		const EXPLOSION_SMOKE_INFO& info
		);

protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//

	float prev_time;                         // предыдущее текущее время
	float start_time;                         // время начала эффекта

	point3 accel;                           // ускорение
	float start_size;                        // начальный размер
	float start_size_range;              // случайная часть начального размера
	float size_factor;                      // изменение размера
	float lifetime;               // время жизни эффекта

	// функция обновляющая частицу при ее рождении
	void Rebirth(ExplosionParticle & ep, float curtime);
};

//
// класс дымового кольца для взрывов
//

class ExplosionSmokeRing : public BaseEffect  
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// эффект закончил свою работу?
	virtual bool IsFinished() const { return finished; }

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

	// деструктор
	virtual ~ExplosionSmokeRing();
	// конструктор с параметрами
	ExplosionSmokeRing(
		const point3 rp,                     // корневая точка
		const SMOKE_RING_INFO &info
		);

protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	// функция размножения точек
	void Multiply();

	//
	// скрытые данные эффекта
	//

	
	float start_time;                         // время начала эффекта
	point3 root_point;                       // корневая точка
	point3 start_velocity; // по идее на фиг не нужно

	float start_size;                        // начальный размер
	float size_factor;                      // изменение размера
	float size;                                // текущий размер частицы
	float lifetime;               // время жизни эффекта
	float color;                  // текущий цвет эффекта
	// количество частиц в массиве
	unsigned int particles_num;
	// массив координат для частиц
	point3 * coords;

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};


#endif // !defined(__EXPLOSIONEFFECT_H__)
