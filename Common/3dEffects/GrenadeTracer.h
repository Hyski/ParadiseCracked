/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект трассера гранаты
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__GRENADE_TRACER_H__)
#define __GRENADE_TRACER_H__

#include "Effect.h"
#include "Particle.h"

#include "../../Skin/skanim.h"

//
// эффект трассера гранаты
//

class GrenadeTracer : public BaseEffect
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }

	// конструктор с параметрами
	GrenadeTracer(
		const KeyAnimation& ka,
		const std::string& skin_name,
		float angle_vel
		);

	// деструктор
	virtual ~GrenadeTracer();

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
	virtual bool Start(const float ) { return true; };

	//
	// скрытые данные эффекта
	//

	BBox bbox;
	float start_time;                         // время начала эффекта
	float lifetime;                           // время жизни эффекта

	point3 current_point;
	KeyAnimation key_animation;
	SkAnim* m_skeleton;    // скелет
	SkSkin* m_skin;           // скин
	AnimaData m_animadata;    // структурка с информацией
	point3 m_angle_vel;
	point3 m_angles;

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

//
// эффект трассера гильзы
//

class ShellTracer : public BaseEffect
{
public:

	// получить тип эффекта (с точки зрения отображения)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }

	// конструктор с параметрами
	ShellTracer(
		const unsigned int entity_id,
		const point3& source,
		const point3& target,
		const std::string& skin_name,
		float angle_vel,
		float vel = 2.0f
		);

	// деструктор
	virtual ~ShellTracer();

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
	virtual bool Start(const float ) { return true; };

	//
	// скрытые данные эффекта
	//

	BBox bbox;
	float start_time;                         // время начала эффекта
	float lifetime;                           // время жизни эффекта

	point3 current_point;
	KeyAnimation key_animation;
	SkAnim* m_skeleton;    // скелет
	SkSkin* m_skin;           // скин
	AnimaData m_animadata;    // структурка с информацией
	point3 m_angle_vel;
	point3 m_angles;

	// флаг, указывающий на необходимость завершения эффекта
	bool finished;
};

#endif // !defined(__GRENADE_TRACER_H__)
