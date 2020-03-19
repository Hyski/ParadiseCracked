/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: класс хвоста из частиц
				класс движения модельки по параболе
				
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__TAILEFFECT_H__)
#define __TAILEFFECT_H__

#include "Effect.h"
#include "Particle.h"

#include "../../Skin/skanim.h"

//
// класс частицы для хвоста
//
class TailParticle : public BaseParticle
{
public:
	point3 start_point;
	point3 velocity;
	float birthtime;
	float lifetime;
	int texcoord_type;                        // тип размножения текстурных координат
	TailParticle(): start_point(0, 0, 0), velocity(0, 0, 0), birthtime(0), lifetime(0), texcoord_type(0)
	{}
};

// тип - список частиц
typedef std::list<TailParticle> TailParticlesList;
// тип - итератор для списка
typedef TailParticlesList::iterator TailParticlesListIterator;

//
// класс хвоста из частиц
//


class TailEffect : public BaseEffect  
{
public:

	// конструктор с параметрами
	TailEffect(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const TAIL_EFFECT_INFO& info
		);

	// деструктор
	virtual ~TailEffect();


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


protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	// функция размножения точек
	virtual void Multiply();

	//
	// скрытые данные эффекта
	//

	// список частиц
	TailParticlesList particles;
	int particles_num;                        // текущее количество частиц

	float * size_vector;                      // вектор с размерами частиц
	
	float start_time;                         // время начала эффекта
	point3 start_point;                       // начальная точка
	point3 end_point;                         // конечная точка
	point3 direction;                         // направление выстрела (единичный вектор)
	unsigned int color;                       // цвет
	float start_size;                         // начальный размер
	float end_size;                           // конечный размер

	point3 current_point;                     // текущая точка

	float lifetime;                           // время жизни эффекта
	float particles_lifetime;                 // время жизни частичек
	int frequency;                            // частота появления (количество новых частиц в секунду)
	float previous_birth_time;                // время рождения предыдущей частицы (серии частиц)

	point3 velocity;                          // скорость

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};


//
// класс хвоста из частиц
//

class TailCircleEffect : public TailEffect  
{
public:

	// конструктор с параметрами
	TailCircleEffect(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const TAIL_EFFECT_INFO& info
		) : TailEffect(StartPoint, EndPoint, info) {}

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D_TRIANGLE; }

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	// функция размножения точек
	virtual void Multiply();
};

//
// класс хвоста из частиц, летящего по параболе
//

class TailParabolaEffect : public TailEffect  
{
public:

	// конструктор с параметрами
	TailParabolaEffect(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const TAIL_EFFECT_INFO& info,
		const float Gravitation
		);


	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

protected:
	float gravitation;
	float vz;
};

//
// класс движения модельки по параболе
//

class TracerParabolaModel : public BaseEffect
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// конструктор с параметрами
	TracerParabolaModel(
		const point3 StartPoint,                         // начальная точка
		const point3 EndPoint,	                         // конечная точка
		const TRACER_PARABOLA_MODEL_INFO& info
		);

	// деструктор
	virtual ~TracerParabolaModel();

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
	float gravitation;                        // гравитация
	float vz;
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

#endif // !defined(__TAILEFFECT_H__)
