/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект выделения
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__SELECTIONEFFECT_H__)
#define __SELECTIONEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// класс выделения
//

class SelectionEffect : public BaseEffect  
{
public:

	// конструктор с параметрами
	SelectionEffect(
		const point3 Target,        // центральная точка
		const SELECTION_EFFECT_INFO &info
		);

	// деструктор
	virtual ~SelectionEffect();

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
	virtual BBox GetBBox(const float ) { return bbox; }

	// сообщение эффекту об изменении позиции
	virtual void SetPosition(const point3& Pos) { root_point = Pos + point3(0.0f, 0.0f, 0.5f); }

	// сообщение эффекту об изменении интенсивности
	virtual void SetActivity(const float Activity)
	{
		if(Activity == 0) visible = false;
		else visible = true;
	}

	// сообщение эффекту о необходимости плавно погаснуть и умереть
	virtual void Finish() { finished = true; }

protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	// функция размножения точек
	void Multiply();

	//
	// скрытые данные эффекта
	//

	// флаг видимости эффекта
	bool visible;
	float radius;
	float small_size;
	float large_size;
	float angle_velocity;
	float angle_delta;
	// цвет
	unsigned int color;
	
	point3 root_point;                     // корневая точка
	float start_time;
	point3 start_vector;                   // начальный вектор
	point3 current_vector;                 // текущий вектор
	point3 axis;                           // ось вращения
	BBox bbox;

	point3 velocity;                          // скорость

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

#endif // !defined(__SELECTIONEFFECT_H__)
