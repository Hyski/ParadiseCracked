/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: разрушение объектов с помощью моделей
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__MODEL_DESTRUCT_EFFECT_H__)
#define __MODEL_DESTRUCT_EFFECT_H__

#include "Effect.h"
#include "Particle.h"

#include "../../Skin/skanim.h"

//
// класс разрушения объектов с помощью моделей
//

class ModelDestructEffect : public BaseEffect
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// конструктор с параметрами
	ModelDestructEffect(
		const std::string* names,
		unsigned int num,
		const BBox* LocalBBox,
		D3DMATRIX* Matrix,
		const point3& Pnt,
		const MODEL_DESTRUCT_INFO& info
		);

	// деструктор
	virtual ~ModelDestructEffect();

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

	// заполним точки привязки
	void SetCurrentPoints(point3 * points,
		unsigned int num,
		const BBox* LocalBBox,
		D3DMATRIX* Matrix);

	// получить центр объекта
	point3 getCenter(const BBox* LocalBBox, D3DMATRIX* Matrix);


	//
	// скрытые данные эффекта
	//
	BBox bbox;
	float start_time;                         // время начала эффекта
	float previous_time;                      // время предыдущего тика
	point3 start_point;                       // начальная точка

	float lifetime;                           // время жизни эффекта
	point3 direction;                         // направление выстрела

	point3 accel;

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;

	// для модельки
	SkAnim* m_skeleton;        // скелет один на всех
	SkSkin** m_skins;          // скины каждый на свой обломок
	AnimaData m_animadata;    // структурка с информацией
	point3* current_points;  // текущие положения точек привязки моделек
	point3* velocities;      // начальные скорости
	unsigned int skin_num;   // количество осколков
	point3* angle_velocities; // угловые скорости вращения
	point3* current_angles;   // текущие углы поворота осколков
	float* m_scales;
};

#endif // !defined(__LINEEFFECT_H__)
