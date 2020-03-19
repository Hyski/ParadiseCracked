
/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: классы эффектов и пространство имен для утилит эффектов
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#if !defined (__EFFECT_H__)
#define __EFFECT_H__

class ISndEmitter;
class ISndScript;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  здесь определена иерархия классов для добавления звука в эффекты
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

//
// абстрактный класс SoundObject
//
class SoundObject
{
public:
	// чисто виртуальная функция, ее необходимо переписать,
	// чтобы определить поведение звука при изменении координат
	// объекта и его скорости
	virtual void Update(const point3& Position, const point3& Velocity) = 0;
	virtual ~SoundObject() {}
};

//
// класс EmptySound - для эффекта, который не проигрывает звук
//
class EmptySound : public SoundObject
{
public:
	// звука нет, так что Update ничего не делает
	virtual void Update(const point3&, const point3&) {}
	
	EmptySound() {}
	virtual ~EmptySound() {}
};

//
// класс FixedSound - для эффекта, который проигрывает неподвижный звук
//
class FixedSound : public SoundObject
{
public:
	// звук неподвижный, так что Update ничего не делает
	virtual void Update(const point3&, const point3&) {}

	FixedSound(const std::string& Name, const point3& Target);
	virtual ~FixedSound() {}
};

//
// класс ActiveSound - для эффекта, который проигрывает неподвижный звук
//
class ActiveSound : public SoundObject
//#pragma SMART_WARNING("Old sound lib")
//, public sound::sdemit
{
	ISndEmitter *emitter;

public:
	// звук подвижный, так что Update должна изменить координаты и скорость
	virtual void Update(const point3& Position, const point3& Velocity);

	ActiveSound(const std::string& Name);
	virtual ~ActiveSound();

private:
	// перепишем чисто виртуальные функции
	virtual point3& GetPos() { return pos; }
	virtual point3& GetVel() { return vel; }
	point3 pos;
	point3 vel;
};



/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  здесь определен базовый класс для всех эффектов
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Particle.h"

class GraphPipe;
struct BBox;


// базовый класс эффекта
class BaseEffect
{
public:
	enum { PEAK_NUM = 6 };
	enum EFFECT_TYPE
	{
		_2D,
		_3D,
		_3D_TRIANGLE,
		_3D_LINESSTRIP,
		_3D_POINTS
	};
	virtual ~BaseEffect() {}
	// первоначальная инициализация
	void Init(GraphPipe * const pGP)
	{ pGraphPipe = pGP; }
////////////////////
//  основные функции поддержки сценария работы с эффектом
////////////////////

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time) = 0;

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update() = 0;

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time) = 0;

////////////////////
//  дополнительные функции поддержки сценария работы с эффектом
////////////////////

	// получить имя шейдера
	std::string GetShader() const { return strShader; }
	// установить новое имя шейдера
	void SetShader(const std::string& str) { strShader = str; }

	// получить количество готовых точек
	unsigned int GetReadyNum() const { return ready_num; }
	// получить адрес массива готовых точек
	point3 * GetReadyVector() const
	{ return ready_vector; }
	// получить адрес массива цветов точек
	unsigned int * GetColorsVector() const
	{ return color_vector; }
	// получить адрес массива текстурных координат
	texcoord * GetTexCoordsVector() const
	{ return texcoord_vector; }

	// эффект закончил свою работу?
	// (по умолчанию эффект работает бесконечно)
	virtual bool IsFinished() const { return false; }
	
	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }

	// сообщение эффекту о начале нового турна игрока
	virtual void NewTurn() {}

	// сообщение эффекту об изменении позиции
	virtual void SetPosition(const point3&) {}

	// сообщение эффекту об изменении интенсивности
	virtual void SetActivity(const float) {}

	// сообщение эффекту о необходимости плавно погаснуть и умереть
	virtual void Finish() {}

////////////////////
//  данные эффекта
////////////////////
protected:
	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time) = 0;

	// указатель на объект GraphPipe
	GraphPipe * pGraphPipe;

	// шейдер эффекта
	std::string strShader;
	// размер массива координат точек
	// и массива точек после проецирования.
	int points_num;
	// размер массива координат готовых к отрисовке вершин,
	// массива цветов, массив текстурных координат
	int ready_vector_size;
	// реальное количество точек для отрисовки (многоугольников)
	int ready_num;
	// указатель на массив координат точек для проецирования
	point3* points_vector;
	// указатель на массив координат точек после проецирования
	point3* points2d_vector;
	// указатель на массив координат готовых к отрисовке точек
	point3* ready_vector;
	// указатель на массив цветов точек
	unsigned int * color_vector;
	// указатель на массив текстурных координат
	texcoord* texcoord_vector;

	// флаг, который показывает первый ли это вызов NextTick
	bool first_time;

	// объект - звук
	SoundObject * sound_object;
};

typedef BaseEffect * PBaseEffect;


///////////////////////////////////////////////////////////////////////////////
//
// пространство имен для утилит эффектов
//
///////////////////////////////////////////////////////////////////////////////
namespace EffectUtility
{
	// получить случайное число в диапазоне от 0 до 1 (float)
	float get0F();
	// получить случайное число в диапазоне от 0 до f (float)
	float get0F(float f);
	
	// получить случайное число в диапазоне от -1 до 1
	float get_FF();
	// получить случайное число в диапазоне от -f до f
	float get_FF(float f);
	// получить случайное число в диапазоне от -f1 до f2
	float get_FF(float f1, float f2);
};

#endif // __EFFECT_H__                                                                                
