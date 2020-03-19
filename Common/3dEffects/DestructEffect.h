/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект разрушения объектов
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__DESTRUCTEFFECT_H__)
#define __DESTRUCTEFFECT_H__

/*
#define GLASS_DEBUG 1 // enable for log file

#if defined (GLASS_DEBUG) //for log
#include "..\Log\Log.h"
extern CLog LOG_GLASS;
#define  LOGG LOG_GLASS["glass.log"]
#endif // for log
#if !defined (GLASS_DEBUG) //for log
#define LOGG /##/
#endif // for log
*/

#include "../../GameLevel/LevelObjects.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс, использующийся для отображения (унаследован от TexObject)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DestructObject : public TexObject
{
public:
	// конструктор
	DestructObject(const float MaxSquare = 0.1);
	// конструктор копирования
	DestructObject(const DestructObject &a);
	// деструктор
	virtual ~DestructObject();
	// оператор присваивания для инициализации объекта
	DestructObject &operator =(const DynObject &a);

private: // данные
	float max_square;

public:

	// вычисление площади треугольника по трем вершинам
	float TriangleSquare(const point3& v1, const point3& v2, const point3& v3)
	{ return ((v2 - v1).Cross(v3 - v1)).Length()*0.5; }

	// структура с информацией о треугольнике, получаемой от TexObject'a
	struct TriangleInf
	{
		point3 Points[3];
		point3 Normals[3];
		texcoord uv[3];
		float square;
	};

	typedef std::list<TriangleInf> TriangleList;

	// для копирования из предоставленного объекта в список
	void CopyToList(STObject * stobj, D3DMATRIX& matr, TriangleList& ready_tr, TriangleList& temp_tr);

	// для копирования из списка в новый объект
	void CopyFromList(STObject * stobj, const TriangleList& tlist);

	// преобразование списока с разбиением больших треугольников
	void ReformList(TriangleList& ready_tr, TriangleList& temp_tr);
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// базовый класс эффекта разрушающегося объекта
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BaseDestructEffect
{
public:
	BaseDestructEffect(const float MaxSquare) : destruct_object(MaxSquare) {}
	virtual ~BaseDestructEffect() {}
	// получить флаг окончания работы
	virtual bool IsFinished() const { return finished; }

	// следующий тик времени
	virtual void NextTick(const float time) = 0;

	// отрисовать эффект
	virtual void Draw(GraphPipe * const graph_pipe) = 0;

protected:
	// объект отображения
	DestructObject destruct_object;
	//DestructObject save_object;
	// флаг окончания работы эффекта
	bool finished;
	// флаг первого тика
	bool first_time;


	// объект - звук
	SoundObject * sound_object;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс эффекта разбивающегося стекла
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// структура для описания отдельного треугольника
struct TriangleDesc
{
	// текущий центр масс
	point3 center;
	// начальные вектора от центра масс к вершинам
	point3 nodes[3];
	// текущие вектора от центра масс к вершинам
	point3 next_nodes[3];
	// угловые скорости поворота вокруг соответствующих осей
	point3 angle_velocity;
	// начальная скорость центра масс
	point3 velocity;
};

// сам класс
class GlassDestruct : public BaseDestructEffect
{
public:
	// конструктор
	GlassDestruct(const DynObject& DynObj,
		const point3& ExpPos,
		const GLASS_DESTRUCT_EFFECT_INFO& info);
	// деструктор
	virtual ~GlassDestruct();

	// следующий тик времени
	virtual void NextTick(const float time);

	// отрисовать эффект
	virtual void Draw(GraphPipe * const graph_pipe);
private:
	BBox debug_bbox;
	// конструктор без параметров - чтобы никто не вздумал создать мой объект без копирования
	GlassDestruct();
	// время рождения эффекта
	float start_time;

	// ускорение
	point3 accel;
	// коэффициент трения об воздух
	float friction_factor;
	// направление разлета осколков
	point3 direction;
	// время жизни
	float lifetime;
	// время предыдущего расчета координат
	float prev_time;

	// количество всех треугольников в объекте
	unsigned int triangles_num;

	// массив описаний треугольников
	TriangleDesc * triangles;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс менеджера эффектов разрушающихся объектов
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// список эффектов
typedef std::list<BaseDestructEffect *> EffectsList;
// сам класс
class DestructEffectManager
{
public:
	// конструктор
	DestructEffectManager();
	// деструктор
	~DestructEffectManager();

	// следующий тик времени
	void NextTick(const float time);

	// отрисовка всех эффектов
	void Draw(GraphPipe * const graph_pipe);

	// временная функция для разбивания стекла
	void DestroyGlass(const DynObject& DynObj,
		const point3& ExpPos,
		const GLASS_DESTRUCT_EFFECT_INFO& info);

private:
	// список эффектов
	EffectsList effects;
};

#endif // !defined(__DESTRUCTEFFECT_H__)
