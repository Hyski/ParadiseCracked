/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: дым (эффект)
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__SMOKEEFFECT_H__)
#define __SMOKEEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// класс частицы, используемый для дыма
//

class SmokeParticle : public BaseParticle
{
public:
	// скорость частицы
	point3 velocity;
	// время рождения частицы
	float birth_time;
	// текстурные координаты частицы
	texcoord texcoords1;
	texcoord texcoords2;

	// время жизни частицы
	float life_time;
};

//
// класс дыма
//

class SmokeEffect : public BaseEffect  
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
	SmokeEffect(
		const float quality,            // качество
		const point3 rp,                // корневая точка
		const SMOKE_EFFECT_INFO& info
		);

	// деструктор
	virtual ~SmokeEffect();

	// эффект закончил свою работу?
	// (по умолчанию эффект работает бесконечно)
	virtual bool IsFinished() const { return finished; }

	// сообщение эффекту о начале нового турна игрока
	virtual void NewTurn() { turns_left--; }
protected:
	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//

	// оставшееся количество туров до завершения эффекта
	int turns_left;
	// флаг режима гашения эффекта
	bool reduce;
	// время начала гашения
	float start_reduce_time;
	// время гашения
	float reduce_time;

	// массив частиц
	SmokeParticle * particles;
	// количество частиц в массиве
	unsigned int particles_num;

	// точка, из которой идет дым
	point3 root_point;
	// случайное изменение начальной точки - точки рождения частиц
	float start_point_factor;
	// скорость
	point3 start_velocity;
	// ускорение
	point3 accel;
	// минимальный начальный размер
	float min_start_size;
	// предел изменения начальногo размера
	float size_range;
	// коэффициент изменения размера частицы в зависимости от времени
	float size_factor;
	// максимальное время жизни частицы
	float max_lifetime;
	// флаг окончания работы
	bool finished;
	// флаг изменения размера по экспоненте
	bool m_exp;

	// функция размножения точек
	void Multiply();
	// функция обновляющая частицу при ее рождении
	void Rebirth(SmokeParticle & sp, float curtime, float disp);
};

//
// класс тяжелого дыма
//

class HeavySmokeEffect : public BaseEffect  
{
public:

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

	// конструктор по умолчанию
	HeavySmokeEffect();

	// конструктор с параметрами
	HeavySmokeEffect(
		const float quality,            // качество
		const point3 rp,                // корневая точка
		const float st_point_factor,    // разброс вокруг начальной точки
		const point3 stVelocity,        // начальная скорость
		const point3 Accel,             // ускорение
		const float stSIZE,             // начальный размер
		const float stRANGE,            // случайная часть начального размера
		const float sizefactor,         // прирост размера
		const float maxlifetime,        // максимальное время жизни
		const int MaxPartNum,           // максимальное количество частиц
		const std::string& Shader,      // имя шейдера
		const std::string& Sound,       // имя звукового шейдера
		const bool SoundMove            // двигается ли звук
		);

	// деструктор
	virtual ~HeavySmokeEffect();
protected:
	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//

	// массив частиц
	SmokeParticle * particles;
	// количество частиц в массиве
	unsigned int particles_num;

	// точка, из которой идет дым
	point3 root_point;
	// случайное изменение начальной точки - точки рождения частиц
	float start_point_factor;
	// время начала проработки эффекта
	float prev_time;
	// скорость
	point3 start_velocity;
	// ускорение
	point3 accel;
	// минимальный начальный размер
	float min_start_size;
	// предел изменения начальногo размера
	float size_range;
	// коэффициент изменения размера частицы в зависимости от времени
	float size_factor;
	// максимальное время жизни частицы
	float max_lifetime;

	// функция размножения точек
	void Multiply();
	// функция обновляющая частицу при ее рождении
	void Rebirth(SmokeParticle & sp, float curtime, float disp);
};


//
// класс частицы, используемый для мух
//

class FlyParticle : public BaseParticle
{
public:
	// сферические координаты:
	// радиус
	float r;
	// угол альфа
	float alpha;
	// угол бетта
	float betta;


	// угол гамма (для изменения радиуса по синусу)
	float gamma;
	// средняя скорость для координат
	point3 vel;
	// средний радиус
	float radius;
	// функция установки координат
	void setCoords(const point3& root)
	{
		coords.x = r * cos(betta);
		coords.y = r * sin(betta);
		coords.z = r * sin(alpha);
		coords += root;
	}
};


//
// класс мух
//

class FlyEffect : public BaseEffect  
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
	FlyEffect(
		const float quality,            // качество
		const point3 rp,                // корневая точка
		const FLY_EFFECT_INFO& info
		);

	// деструктор
	virtual ~FlyEffect();

protected:
	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//


	// массив частиц
	FlyParticle * particles;
	// количество частиц в массиве
	unsigned int particles_num;
	// время предыдущего тика
	float previous_time;

	// центральная точка
	point3 root_point;
	// минимальный начальный размер
	float min_start_size;
	// предел изменения начальногo размера
	float size_range;

	float min_radius, radius_range;
	point3 min_vel;

	point3 vel;
	BBox bb;

	// функция размножения точек
	void Multiply();
};

//
// класс частицы, используемый для мух
//

class FlyCylinderParticle : public BaseParticle
{
public:
	// цилиндрические координаты (радиус постоянный):
	// высота
	float h;
	// угол альфа
	float alpha;
	// скорость по выстоте
	float vh;
	// скорость по углу
	float valpha;
	// задержка в рождении частицы
	float delay;
	// функция установки координат
	void setCoords(const point3& root, float r)
	{
		coords.x = r * cos(alpha);
		coords.y = r * sin(alpha);
		coords.z = h;
		coords += root;
	}
};

//
// класс мух с хвостами, летающих по цилиндру
//

class FlyCylinderEffect : public BaseEffect  
{
public:
	// конструктор с параметрами
	FlyCylinderEffect(
		const float quality,            // качество
		const point3 rp,                // корневая точка
		const FLY_CYLINDER_EFFECT_INFO& info
		);

	// деструктор
	virtual ~FlyCylinderEffect();

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time) { return m_bbox; }

protected:
	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//
	BBox m_bbox;
	// высота эффекта
	float m_height;
	// количество основных частиц
	unsigned int m_particles_num;
	// задержка по времени между частичками хвоста
	float m_deltat;
	// размер основных частиц
	float m_part_size;
	// радиус эффекта
	float m_radius;
	// количество частиц в хвосте
	unsigned int m_tail_part_num;
	// скорость частиц
	float m_velh;
	float m_velalpha;
	// корневая точка
	point3 m_root;
	// вектор частиц
	typedef std::vector<FlyCylinderParticle> ParticlesVector_t;
	ParticlesVector_t m_particles;
	ParticlesVector_t m_not_delayed_particles;
	// время предыдущего тика
	float m_previous_time;
	// скорость для звука
	point3 m_vel;
	// цвет отметины под эффектом
	point3 m_color;
	// сколько осталось времени до создания новой отметины
	float m_mark_time;

	// функция размножения точек
	void Multiply();
};

#endif // !defined(__SMOKEEFFECT_H__)
