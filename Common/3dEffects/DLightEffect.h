/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект динамического освещения и эффект вспышки
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__DLIGHTEFFECT_H__)
#define __DLIGHTEFFECT_H__

#include "Effect.h"
#include "Particle.h"
#include "../../GameLevel/mark.h"

//
// базовый класс эффекта динамического освещения
//

class DLightEffect : public BaseEffect  
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// эффект закончил свою работу?
	virtual bool IsFinished() const { return finished; }

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

	// конструктор
	DLightEffect();
	// деструктор
	virtual ~DLightEffect();

protected:
	//
	// скрытые данные эффекта
	//
	
	// указатель на объект динамического освещения
	Mark *d_light;

	float start_time;                         // время начала эффекта
	float lifetime;                           // время жизни эффекта

	point3 current_point;                     // текущая точка
	float radius;                             // радиус
	unsigned int color;                       // цвет

	point3 vel;                          // скорость
	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

//
// класс динамического освещения, летящего по прямой
//

class LineDLightEffect : public DLightEffect  
{
public:

	// конструктор с параметрами
	LineDLightEffect(
		const point3& StartPoint,            // начальная скорость
		const point3& EndPoint,              // конечная точка
		const LINE_DLIGHT_EFFECT_INFO& info
		);

	// деструктор
	virtual ~LineDLightEffect() {}

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
	point3 start_point;                       // начальная точка
	point3 end_point;                         // конечная точка

	point3 direction;                         // направление выстрела
	point3 velocity;                          // скорость
};


//
// класс эффекта вспышки
//

class FlashEffect : public BaseEffect  
{
public:

	// конструктор с параметрами
	FlashEffect(
		const point3& Target,            // точка попадания
		const FLASH_EFFECT_INFO& info
		);

	// деструктор
	virtual ~FlashEffect();

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// эффект закончил свою работу?
	virtual bool IsFinished() const { return finished; }

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float ) { return bbox; }

protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);
	// функция размножения точек
	virtual void Multiply();
	
	//
	// скрытые данные эффекта
	//
	
	float start_time;                         // время начала эффекта
	float lifetime;                           // время жизни эффекта
	float switch_time;                        // время переключения
	float size;                               // размер
	float size_begin;
	float size_end;
	unsigned int color;
	point3 root_point;                     // текущая точка

	BBox bbox;

	point3 vel;                          // скорость
	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

//
// класс эффекта многотекстурной вспышки
//

class MultiTexFlashEffect : public FlashEffect  
{
public:

	// конструктор с параметрами
	MultiTexFlashEffect(
		const point3& Target,            // точка попадания
		const FLASH_EFFECT_INFO& info
		) : FlashEffect(Target, info) { phase = 0; }

protected:
	// функция размножения точек
	virtual void Multiply();
	//
	// скрытые данные эффекта
	//
	int phase;
	float u1, v1, u2, v2;
};

#endif // !defined(__DLIGHTEFFECT_H__)
