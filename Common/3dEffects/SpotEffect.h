/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: пятно
                сверкающее пятно
                частицы, вращающиеся вокруг общего центра
                частицы, вращающиеся вокруг общего центра2
				увеличивающееся пятно (абстрактный)
				одинарное пламя (одинарное увеличивающееся пятно)
				двойное пламя (двойное увеличивающееся пятно)
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__SPOTEFFECT_H__)
#define __SPOTEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// класс пятна
//

class SpotEffect : public BaseEffect  
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
	virtual ~SpotEffect();
	// конструктор с параметрами
	SpotEffect(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const SPOT_EFFECT_INFO& info
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
	unsigned int color;                       // цвет

	point3 current_point;                     // текущая точка

	float lifetime;                           // время жизни эффекта

	point3 velocity;                          // скорость

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};


//
// класс сверкающего пятна
//

class ShineSpotEffect : public SpotEffect  
{
public:
	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// конструктор с параметрами
	ShineSpotEffect(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const SPOT_EFFECT_INFO& info
		) : SpotEffect(StartPoint, EndPoint, info), phase(0)
	{}

protected:
	unsigned int phase;
};

//
// частицы, вращающиеся вокруг общего центра
//

class RotateSpotEffect : public BaseEffect  
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
	virtual ~RotateSpotEffect();
	// конструктор с параметрами
	RotateSpotEffect(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const ROTATE_SPOT_EFFECT_INFO& info
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
	float size_first;                         // размер первой частицы
	float size_last;                          // размер последней частицы
	float radius_start;                       // начальный радиус
	float radius_end;                         // конечный радиус
	unsigned int color;                       // цвет
	bool disp;                                // есть ли разброс радиусов

	point3 * current_vectors;                 // массив начальных векторов
	point3 * current_points;                  // массив текущих точек
	float * current_vectors_disp;             // массив для разброса начальных векторов
	point3 start_vector;                      // начальный вектор
	point3 direction;                         // направление выстрела

	float lifetime;                           // время жизни эффекта

	point3 velocity;                          // скорость
	float ang_vel;                            // угловая скорость
	float delta;                              // задержка во времени между последовательностями
	int sequence_number;                      // количество последовательностей
	int sequence_size;                        // размер последовательности

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

//
// частицы, вращающиеся вокруг общего центра2
//

class RotateSpotEffect2 : public BaseEffect  
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D_TRIANGLE; }

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// эффект закончил свою работу?
	virtual bool IsFinished() const { return finished; }

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float ) { return bbox; }

	// деструктор
	virtual ~RotateSpotEffect2();
	// конструктор с параметрами
	RotateSpotEffect2(
		const point3 Target,                         // центральная точка
		const ROTATE_SPOT_EFFECT2_INFO& info
		);

protected: 

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	//
	// скрытые данные эффекта
	//
	
	float start_time;                         // время начала эффекта
	point3 root_point;                        // центральная точка
	point3 start_point;                       // начальная точка
	float size;                               // размер
	float radius_begin;                       // радиус
	float radius_end;                         // радиус
	unsigned int color;                       // цвет

	unsigned int number;                      // количество

	float lifetime;                           // время жизни эффекта
	float ang_vel;                            // угловая скорость

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;

	// BBOX эффекта
	BBox bbox;

	point3 vel;
};

//
// класс увеличивающегося пятна (базовый, абстрактный)
//

class UpsizingSpot : public BaseEffect
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// конструктор с параметрами
	UpsizingSpot(
		const point3 StartPoint,            // начальная точка
		const point3 EndPoint,	            // конечная точка
		const UPSIZING_SPOT_INFO &info  // информация о параметрах
		);

	// деструктор
	virtual ~UpsizingSpot();

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// эффект закончил свою работу?
	virtual bool IsFinished() const { return finished; }

protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	// функция размножения точек
	virtual void Multiply() = 0;

	//
	// скрытые данные эффекта
	//
	
	float start_time;                         // время начала эффекта
	point3 start_point;                       // начальная точка
	point3 current_point;                     // текущая точка
	point3 direction;                         // направление выстрела
	unsigned int color;                       // цвет

	float lifetime;                           // время жизни эффекта
	float start_vel;                          // начальная скорость
	float accel;                              // ускорение
	float width_first;                        // ширина в начале линии
	float width_second;                       // ширина в конце линии
	float width_vel;                          // скорость изменения ширины
	float d_width;                            // текущая добавка к ширине

	point3 velocity;                          // скорость

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

//
// класс одинарного пламени (одинарное увеличивающееся пятно)
//

class OneUpsizingSpot : public UpsizingSpot  
{
public:

	// конструктор с параметрами
	OneUpsizingSpot(
		const point3 StartPoint,            // начальная точка
		const point3 EndPoint,	            // конечная точка
		const UPSIZING_SPOT_INFO &info      // информация о параметрах
		) : UpsizingSpot(StartPoint, EndPoint, info) {}

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

protected:

	// функция размножения точек
	void Multiply();
};

//
// класс двойного пламени (двойное увеличивающееся пятно)
//

class TwoUpsizingSpot : public UpsizingSpot  
{
public:

	// конструктор с параметрами
	TwoUpsizingSpot(
		const point3 StartPoint,            // начальная точка
		const point3 EndPoint,	            // конечная точка
		const UPSIZING_SPOT_INFO &info      // информация о параметрах
		) : UpsizingSpot(StartPoint, EndPoint, info) {}

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

protected:

	// функция размножения точек
	void Multiply();
};

//
// класс волнового пятна
//

class WaveSpot : public BaseEffect  
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// конструктор с параметрами
	WaveSpot(
		const point3 Target,                // начальная точка
		const WAVE_SPOT_INFO &info          // информация о параметрах
		);

	// деструктор
	virtual ~WaveSpot();

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// эффект закончил свою работу?
	virtual bool IsFinished() const { return finished; }

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

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
	point3 root_point;                        // начальная точка
	float start_size;                         // начальный размер
	float size;                               // текущий размер
	float size_velocity;                      // скорость возрастания размера

	unsigned int color;

	float lifetime;                           // время жизни эффекта
	point3 velocity;                          // скорость

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

//
// класс перпендикулярного волнового пятна
//

class WaveSpot2 : public BaseEffect  
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// конструктор с параметрами
	WaveSpot2(
		const point3 Source,                // точка выстрела
		const point3 Target,                // точка попадания
		const WAVE_SPOT_INFO &info          // информация о параметрах
		);

	// деструктор
	virtual ~WaveSpot2();

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

	// эффект закончил свою работу?
	virtual bool IsFinished() const { return finished; }

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

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
	point3 root_point;                        // начальная точка
	point3 direction;                         // направление
	point3 base;                              // базовый вектор
	float start_size;                         // начальный размер
	float size;                               // текущий размер
	float size_velocity;                      // скорость возрастания размера

	unsigned int color;

	float lifetime;                           // время жизни эффекта
	point3 velocity;                          // скорость

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

#endif // !defined(__SPOTEFFECT_H__)
