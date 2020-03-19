/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: искры, остающиеся от трассера
				класс клубков линий
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__SPANGLEEFFECT_H__)
#define __SPANGLEEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// класс искр, остающихся от трассера
//

// класс частиц
class SpangleParticle : public BaseParticle
{
public:
	// скорость частицы
	point3 velocity;
	// время жизни частицы
	float lifetime;
	// время рождения частицы
	float birthtime;
	// начальная точка
	point3 start_point;
};

class SpangleEffect : public BaseEffect  
{
public:

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
	virtual BBox GetBBox(const float current_time);

	// деструктор
	virtual ~SpangleEffect();
	// конструктор с параметрами
	SpangleEffect(const float quality,  // качество
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const SPANGLE_EFFECT_INFO& info
		);

protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	// функция размножения точек
	virtual void Multiply();

	//
	// скрытые данные эффекта
	//

	// массив частиц
	SpangleParticle * particles;
	// количество частиц
	unsigned int particles_num;
	
	float start_time;                         // время начала эффекта
	point3 start_point;                       // начальная точка
	point3 end_point;                         // конечная точка
	point3 direction;                         // направление выстрела (единичный вектор)
	float size;                               // размер
	unsigned int color;                       // цвет

	point3 current_point;                     // текущая точка

	float lifetime;                           // время жизни эффекта
	float particles_lifetime;                 // время жизни частичек

	point3 velocity;                          // скорость

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

//
// класс клубка линий
//


class LineBallEffect : public BaseEffect  
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D_LINESSTRIP; }

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
	virtual ~LineBallEffect();
	// конструктор с параметрами
	LineBallEffect(const float quality,  // качество
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const LINE_BALL_EFFECT_INFO& info
		);

protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	// функция размножения точек
	virtual void Multiply() {}

	//
	// скрытые данные эффекта
	//

	// массив частиц
	SpangleParticle * particles;
	// количество частиц
	unsigned int particles_num;
	
	float start_time;                         // время начала эффекта
	point3 start_point;                       // начальная точка
	point3 end_point;                         // конечная точка
	point3 direction;                         // направление выстрела (единичный вектор)
	float length;                             // длина
	float width;                              // ширина
	unsigned int color;                       // цвет

	point3 first_point;                       // первая точка
	point3 second_point;                      // вторая точка

	float lifetime;                           // время жизни эффекта
	float particles_lifetime;                 // время жизни частичек

	point3 velocity;                          // скорость

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

#endif // !defined(__SPANGLEEFFECT_H__)
