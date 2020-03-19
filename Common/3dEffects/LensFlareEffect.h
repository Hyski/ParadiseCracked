/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект lens flare
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__LENSFLAREEFFECT_H__)
#define __LENSFLAREEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// класс lens flare
//


class LensEffect : public BaseEffect  
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
	virtual ~LensEffect();
	// конструктор с параметрами
	LensEffect(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const LENS_FLARE_EFFECT_INFO& info
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
	point3 start_point;                       // начальная точка
	point3 end_point;                         // конечная точка
	float size;                               // размер
	float distance;                           // расстояние до центра
	unsigned int color;                       // цвет

	point3 current_point;                     // текущая точка

	float lifetime;                           // время жизни эффекта

	point3 velocity;                          // скорость

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

#endif // !defined(__LENSFLAREEFFECT_H__)
