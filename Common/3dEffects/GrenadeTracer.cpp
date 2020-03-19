/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект трассера гранаты
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include "EffectInfo.h"
#include "GrenadeTracer.h"
#include "../../Skin/animalibrary.h"
#include "../../logic2/TraceUtils.h"

using namespace EffectUtility;

static const std::string ITEMSDIR="Animations/skins/items/";
static const std::string ITEMSSKEL="Animations/anims/items/Fiery_Granade.skel";

//
// эффект трассера гранаты
//

// конструктор с параметрами
GrenadeTracer::GrenadeTracer(
							 const KeyAnimation& ka,
							 const std::string& skin_name,
							 float angle_vel
							 )
{
	ready_vector_size = ready_num = 0;

	key_animation = ka;
	lifetime = ka.GetLastTime();
	m_angle_vel = point3(get0F(angle_vel), get0F(angle_vel), get0F(angle_vel));

	// загрузим скелет
	AnimaLibrary *lib=AnimaLibrary::GetInst();
	m_skeleton = lib->GetSkAnimation(ITEMSSKEL);
	m_animadata = m_skeleton->Start(0);


	// загрузим скин
	m_skin = lib->GetSkSkin(ITEMSDIR + skin_name + ".skin");
	if(!m_skin)
	{
		// выкинем исключение
		std::string str = "illegal skin: \"";
		str += ITEMSDIR + skin_name + ".skin\"";
		throw CASUS(str);
	}
	// прикрепим его к скелету
	m_skin->ValidateLinks(m_skeleton);
	
	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;
	// флаг, указывающий на необходимость завершения эффекта
	finished = false;

	// звука нет
	sound_object = new EmptySound();
}

// деструктор
GrenadeTracer::~GrenadeTracer()
{
	// убить звук
	delete sound_object;

	// уничтожить скин
	delete m_skin;
}

// следующий тик времени
// (только расчет координат)
bool GrenadeTracer::NextTick(const float current_time)
{
	if(first_time) { first_time = false; start_time = current_time; }

	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// центр модели
	key_animation.GetTrans(&current_point, dt);

	m_angles.x = fmod(m_angle_vel.x * dt, PIm2);
	m_angles.y = fmod(m_angle_vel.y * dt, PIm2);
	m_angles.z = fmod(m_angle_vel.z * dt, PIm2);

	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool GrenadeTracer::Update()
{
	// модель
	D3DMATRIX mat(
		FastCos(m_angles.y)*FastCos(m_angles.z),
		FastCos(m_angles.y)*FastSin(m_angles.z),
		-FastSin(m_angles.y),
		0,

		FastSin(m_angles.x)*FastSin(m_angles.y)*FastCos(m_angles.z) - FastCos(m_angles.x)*FastSin(m_angles.z),
		FastSin(m_angles.x)*FastSin(m_angles.y)*FastSin(m_angles.z) + FastCos(m_angles.x)*FastCos(m_angles.z),
		FastSin(m_angles.x)*FastCos(m_angles.y),
		0,

		FastCos(m_angles.x)*FastSin(m_angles.y)*FastCos(m_angles.z) + FastSin(m_angles.x)*FastSin(m_angles.z),
		FastCos(m_angles.x)*FastSin(m_angles.y)*FastSin(m_angles.z) - FastSin(m_angles.x)*FastCos(m_angles.z),
		FastCos(m_angles.x)*FastCos(m_angles.y),
		0,

		current_point.x,
		current_point.y,
		current_point.z,
		1);

	m_animadata.LastState.m_SkelState[0].World = mat;
	m_skin->Update(&m_animadata.LastState);
	pGraphPipe->Chop(m_skin->GetMesh());
	return true;
}

// возвращает bounding box эффекта
BBox GrenadeTracer::GetBBox(const float )
{
	bbox.Degenerate();
	bbox.Enlarge(current_point + point3(5.0, 5.0, 5.0));
	bbox.Enlarge(current_point - point3(5.0, 5.0, 5.0));
	return bbox;
}

//////////////////////////////////////////////////////////////////////////////
//
// эффект трассера гильзы
//
//////////////////////////////////////////////////////////////////////////////

// получить тип эффекта (с точки зрения отображения)

// конструктор с параметрами
ShellTracer::ShellTracer(
						 const unsigned int entity_id,
						 const point3& source,
						 const point3& target,
						 const std::string& skin_name,
						 float angle_vel,
						 float vel
						 )
{
	ready_vector_size = ready_num = 0;

	// создадим нужную трассу
	point3 AXIS_dir = Normalize(target - source);
	point3 AXIS_y = Normalize(AXIS_dir.Cross(point3(0.0f, 0.0f, 1.0f)));
	point3 AXIS_x = Normalize(AXIS_dir.Cross(AXIS_y));
	float xx = get0F();
	float yy = get0F();
	float zz = get0F(0.2f);

	point3 dir = Normalize(xx*AXIS_x + yy*AXIS_y + zz*AXIS_dir);

	_GrenadeTracer tracer(entity_id, source, dir, vel, 0);
	key_animation = tracer.GetTrace();
	
	
	lifetime = key_animation.GetLastTime();
	m_angle_vel = point3(get0F(angle_vel), get0F(angle_vel), get0F(angle_vel));

	// загрузим скелет
	AnimaLibrary *lib=AnimaLibrary::GetInst();
	m_skeleton = lib->GetSkAnimation(ITEMSSKEL);
	m_animadata = m_skeleton->Start(0);


	// загрузим скин
	m_skin = lib->GetSkSkin(ITEMSDIR + skin_name + ".skin");
	// прикрепим его к скелету
	m_skin->ValidateLinks(m_skeleton);
	if(!m_skin)
	{
		// выкинем исключение
		std::string str = "illegal skin: \"";
		str += ITEMSDIR + skin_name + ".skin\"";
		throw CASUS(str);
	}
	
	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;
	// флаг, указывающий на необходимость завершения эффекта
	finished = false;

	// звука нет
	sound_object = new EmptySound();
}

// деструктор
ShellTracer::~ShellTracer()
{
	// убить звук
	delete sound_object;

	// уничтожить скин
	delete m_skin;
}

// следующий тик времени
// (только расчет координат)
bool ShellTracer::NextTick(const float current_time)
{
	if(first_time) { first_time = false; start_time = current_time; }

	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// центр модели
	key_animation.GetTrans(&current_point, dt);

	m_angles.x = fmod(m_angle_vel.x * dt, PIm2);
	m_angles.y = fmod(m_angle_vel.y * dt, PIm2);
	m_angles.z = fmod(m_angle_vel.z * dt, PIm2);

	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool ShellTracer::Update()
{
	// модель
	D3DMATRIX mat(
		FastCos(m_angles.y)*FastCos(m_angles.z),
		FastCos(m_angles.y)*FastSin(m_angles.z),
		-FastSin(m_angles.y),
		0,

		FastSin(m_angles.x)*FastSin(m_angles.y)*FastCos(m_angles.z) - FastCos(m_angles.x)*FastSin(m_angles.z),
		FastSin(m_angles.x)*FastSin(m_angles.y)*FastSin(m_angles.z) + FastCos(m_angles.x)*FastCos(m_angles.z),
		FastSin(m_angles.x)*FastCos(m_angles.y),
		0,

		FastCos(m_angles.x)*FastSin(m_angles.y)*FastCos(m_angles.z) + FastSin(m_angles.x)*FastSin(m_angles.z),
		FastCos(m_angles.x)*FastSin(m_angles.y)*FastSin(m_angles.z) - FastSin(m_angles.x)*FastCos(m_angles.z),
		FastCos(m_angles.x)*FastCos(m_angles.y),
		0,

		current_point.x,
		current_point.y,
		current_point.z,
		1);

	m_animadata.LastState.m_SkelState[0].World = mat;
	m_skin->Update(&m_animadata.LastState);
	pGraphPipe->Chop(m_skin->GetMesh());
	return true;
}

// возвращает bounding box эффекта
BBox ShellTracer::GetBBox(const float )
{
	bbox.Degenerate();
	bbox.Enlarge(current_point + point3(5.0, 5.0, 5.0));
	bbox.Enlarge(current_point - point3(5.0, 5.0, 5.0));
	return bbox;
}

