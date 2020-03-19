/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Halo
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__HALOEFFECT_H__)
#define __HALOEFFECT_H__

#include "Effect.h"
#include "Particle.h"

// структура для описания текущего состояния halo с точки зрения видимости
struct HaloState
{
	// состояние видимости эффекта
	enum State { hidden, visible, disappearing, appearing };
	State state;

	// время последнего переключения состояния видимости
	float prev_time;
	// время последнего трассирования луча
	float prev_trace_time;

	// время, в течение которого эффект находится в переходном состоянии
	float appearing_time;
	// время, в течение которого эффект находится в переходном состоянии
	float disappearing_time;

	// эффект виден trace ray'ем
	bool trace_ray_visible;
	// эффект виден bbox'ом
	bool bbox_visible;

	// камера была передвинута, но Trace Ray еще не проверялся!
	bool camera_was_changed;

	HaloState() {   state = hidden;
					prev_time = 0;
					prev_trace_time = 0;
					appearing_time = 0;
					disappearing_time = 0;
					trace_ray_visible = false;
					bbox_visible = false;
					camera_was_changed = false;
				}
};

//
// базовый класс для halo (абстрактный)
//

class BaseHalo : public BaseEffect  
{
public:

	// конструктор с параметрами
	BaseHalo(
		const point3 rp,			  // начальная точка
		const std::string &Shader,    // шейдер эффекта
		const std::string &Sound,     // описатель звука
		const bool SoundMove          // двигается ли звук
		);

	// деструктор
	virtual ~BaseHalo();

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time) = 0;

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float ) { return bbox; }

protected:
	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time) = 0;
	// функция размножения точек
	virtual void Multiply();

	// функция расчета видимости
	void UpdateVisibility(const float current_time);

	//
	// скрытые данные эффекта
	//

	// состояние видимости эффекта
	HaloState state;

	float size;
	// цвет
	unsigned int color;
	unsigned int save_color;

	// корневая точка
	point3 root_point;
	BBox bbox;

	point3 vel;
};

//
// класс simple halo
//

class SimpleHaloEffect : public BaseHalo  
{
public:

	// конструктор с параметрами
	SimpleHaloEffect(
		const point3 rp,			  // начальная точка
        const point3& Color,
		const SIMPLE_HALO_EFFECT_INFO &info
		);

	// деструктор
	virtual ~SimpleHaloEffect();

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

protected:
	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//
};

//
// класс halo
//

class HaloEffect : public BaseHalo  
{
public:

	// конструктор с параметрами
	HaloEffect(
		const point3 rp,			  // начальная точка
		const point3& Color,
		const HALO_EFFECT_INFO &info
		);

	// деструктор
	virtual ~HaloEffect();

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

protected:
	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//

	float small_size;
	float large_size;
	float near_dist;
	float far_dist;
	float near_color_factor;
	float far_color_factor;

	float dist_range;
	float size_range;
	float color_factor_range;

	point3 color3;
};

//
// класс мигающего halo
//
class BlinkingHaloEffect : public BaseHalo  
{
public:

	// конструктор с параметрами
	BlinkingHaloEffect(
		const point3 rp,			  // начальная точка
        const point3& Color,
		const BLINKING_HALO_EFFECT_INFO &info
		);

	// деструктор
	virtual ~BlinkingHaloEffect();

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

protected:
	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//
	float sin_a;
	point3 my_save_color;
	float start_time;
};


#endif // !defined(__HALOEFFECT_H__)
