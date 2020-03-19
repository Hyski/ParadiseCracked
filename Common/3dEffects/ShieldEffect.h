/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффекта броника
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__SHIELDEFFECT_H__)
#define __SHIELDEFFECT_H__

#include "Effect.h"
#include "Particle.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс эффекта броника
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ShieldEffect : public BaseEffect
{
public:
	// конструктор с параметрами
	ShieldEffect(
		const point3& Source,
		const point3& Tartget,
		const SHIELD_INFO& info
		);
	// деструктор
	virtual ~ShieldEffect();

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }
	
	// эффект закончил свою работу?
	virtual bool IsFinished() const { return finished; }

	// следующий тик времени
	// (только расчет координат)
	virtual bool NextTick(const float current_time);

	// возвращает bounding box эффекта
	virtual BBox GetBBox(const float current_time);

	// подготовка массива готовых точек и массива цветов точек
	virtual bool Update();

protected:

	// с вызова этого метода начинается отсчет
	// времени для расчета эффекта
	virtual bool Start(const float start_time);

	// функция размножения точек
	void Multiply();

	//
	// скрытые данные эффекта
	//

	unsigned int spot_num;        // количество пятен в окружности
	unsigned int circles_num;     // количество окружностей
	float dx;                     // приращение по ху
	float a;                      // коэффициет у параболы
	float lifetime;               // время жизни эффекта
	float size;                   // размер частиц
	
	point3* unit_vectors;         // единичные вектора на единичной окружности
	float start_time;             // время начала эффекта
	point3 root_point;            // корневая точка
	point3 direction;             // направление
	BBox bbox;

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;

	point3 vel;
};

#endif // !defined(__SHIELDEFFECT_H__)
