/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: линии выстрелов
				эффект плазменного резака
				эффект молнии
				эффект движения модельки по линии
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__LINEEFFECT_H__)
#define __LINEEFFECT_H__

#include "Effect.h"
#include "Particle.h"

#include "../../Skin/skanim.h"

//
// класс линии
//

class LineEffect : public BaseEffect
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
	virtual ~LineEffect();
	// конструктор с параметрами
	LineEffect(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const LINE_EFFECT_INFO& info
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

	point3 first_point;                       // первая точка
	point3 second_point;                      // вторая точка

	float lifetime;                           // время жизни эффекта
	float length;                             // длина линии
	float width_first;                        // ширина в начале линии
	float width_second;                       // ширина в конце линии
	point3 direction;                         // направление выстрела

	point3 velocity;                          // скорость

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

//
// класс плазменного резака
//

class PlasmaBeamEffect : public BaseEffect
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// конструктор с параметрами
	PlasmaBeamEffect(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const PLASMA_BEAM_INFO& info
		);

	// деструктор
	virtual ~PlasmaBeamEffect();

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
	void Multiply();

	//
	// скрытые данные эффекта
	//

	BBox bbox;
	float start_time;                         // время начала эффекта
	point3 start_point;                       // начальная точка
	point3 end_point;                         // конечная точка

	float lifetime;                           // время жизни эффекта
	float length;                             // длина линии
	float width_target;                        // ширина в начале линии
	float width_source;                       // ширина в конце линии
	point3 direction;                         // направление выстрела

	point3 vel;

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

//
// класс молнии
//

class LightningEffect : public BaseEffect
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D_TRIANGLE; }

	// конструктор с параметрами
	LightningEffect(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const LIGHTNING_INFO& info
		);

	// деструктор
	virtual ~LightningEffect();

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
	void Multiply();

	//
	// скрытые данные эффекта
	//

	point3* nodes;
	point3* start_nodes;
	BBox bbox;
	float start_time;                         // время начала эффекта
	point3 start_point;                       // начальная точка
	point3 end_point;                         // конечная точка

	float lifetime;                           // время жизни эффекта
	float length;                             // длина линии
	float width;                              // ширина
	unsigned int brunch_num;                  // количество ветвей молнии
	unsigned int segment_num;                 // количество сегментов в ветви
	unsigned int color;
	point3 direction;                         // направление выстрела

	point3 vel;

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};


//
// движения модельки по линии
//

class TracerLineModel : public BaseEffect
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// конструктор с параметрами
	TracerLineModel(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const TRACER_LINE_MODEL_INFO& info
		);

	// деструктор
	virtual ~TracerLineModel();

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

	//
	// скрытые данные эффекта
	//
	BBox bbox;
	float start_time;                         // время начала эффекта
	point3 start_point;                       // начальная точка
	point3 end_point;                         // конечная точка
	point3 current_point;

	float lifetime;                           // время жизни эффекта
	point3 direction;                         // направление выстрела

	point3 vel;

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;

	// для модельки
	SkAnim* m_skeleton;    // скелет
	SkSkin* m_skin;           // скин
	AnimaData m_animadata;    // структурка с информацией
	D3DMATRIX m_mat; // матрица
};

#endif // !defined(__LINEEFFECT_H__)
