/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: разрушение объектов с помощью моделей
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/   
#include "precomp.h"             
#include "../GraphPipe/GraphPipe.h"
#include "EffectInfo.h"
#include "ModelDestructEffect.h"

#include "../../Skin/animalibrary.h"

using namespace EffectUtility;

//
// класс разрушения объектов с помощью моделей
//

// конструктор с параметрами
ModelDestructEffect::ModelDestructEffect(
										 const std::string* names,
										 unsigned int num,
										 const BBox* LocalBBox,
										 D3DMATRIX* Matrix,
										 const point3& Pnt,
										 const MODEL_DESTRUCT_INFO& info
										 )
{
	points_num = 0;
	ready_vector_size = ready_num = 0;

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;

	// время жизни эффекта
	lifetime = info.LifeTime;
	accel = info.Accel;

	// BBox
	point3 center = getCenter(LocalBBox, Matrix);
	point3 pnt;
	pnt.z = pnt.y = pnt.x = (info.MaxVelFactor + info.ExplVelFactor)*info.LifeTime;
	bbox.Degenerate();
	bbox.Enlarge(center - pnt);
	bbox.Enlarge(center + pnt);
	
	// флаг, указывающий на необходимость завершения эффекта
	finished = false;
	// звука нет
	sound_object = new EmptySound();

	// загрузим склет для всех осколков
	AnimaLibrary *lib=AnimaLibrary::GetInst();
	m_skeleton = lib->GetSkAnimation("Animations/anims/items/bone.skel");
	m_animadata = m_skeleton->Start(0);

	// создадим массив скинов, и загрузим его
	float vol = fabs(LocalBBox->maxx - LocalBBox->minx)*fabs(LocalBBox->maxy - LocalBBox->miny)*fabs(LocalBBox->maxz - LocalBBox->minz);
	float scale = sqrt(sqrt(vol));
	skin_num = 3+static_cast<unsigned int>(info.Density * scale);
	int k = 0;
	m_skins = new SkSkin* [skin_num];
	for(int i = 0; i < skin_num; i++)
	{
		m_skins[i] = lib->GetSkSkin(names[k]);
		if(!m_skins[i])
		{
			// выкинем исключение
			std::string str = "illegal skin: \"";
			str += names[k] + "\"";
			for(int n= 0; n < i; n++)
			{
				delete m_skins[n];
			}
			delete[] m_skins;
			delete sound_object;
			throw CASUS(str);
		}
		m_skins[i]->ValidateLinks(m_skeleton);
		k++;
		if(k >= num) k = 0;
	}
	
	// создадим массив точек привязки
	current_points = new point3[skin_num];
	// заполним точки привязки
	SetCurrentPoints(current_points,
		skin_num,
		LocalBBox,
		Matrix);
	// создадим массив начальных скоростей
	velocities = new point3[skin_num];

	// заполнение массива начальных скоростей
	point3 dir = Normalize(center - Pnt) * info.ExplVelFactor;
	for(int i = 0; i < skin_num; i++)
	{
		float x = info.MinVelFactor + get0F(info.MaxVelFactor - info.MinVelFactor);
		if(get_FF() < 0.0f) x = -x;
		float y = info.MinVelFactor + get0F(info.MaxVelFactor - info.MinVelFactor);
		if(get_FF() < 0.0f) y = -y;
		float z = info.MinVelFactor + get0F(info.MaxVelFactor - info.MinVelFactor);
		velocities[i] = point3(x, y, z) + dir;
	}

	// создание массивов угловых скоростей и текущих углов
	angle_velocities = new point3[skin_num];
	current_angles = new point3[skin_num];
	for(int i = 0; i < skin_num; i++)
	{
		angle_velocities[i] = point3(get0F(info.MaxAngleVel),
			get0F(info.MaxAngleVel),
			get0F(info.MaxAngleVel));

		current_angles[i] = point3(get0F(PIm2), get0F(PIm2), get0F(PIm2));
	}

	// создание и заполнение размеров осколков
	m_scales = new float[skin_num];
	for(int i = 0; i < skin_num; i++)
	{
		m_scales[i] = 0.2*info.ScaleFactor*scale + 0.8*get0F(info.ScaleFactor)*scale;
	}
}

// деструктор
ModelDestructEffect::~ModelDestructEffect()
{
	// звука
	delete sound_object;
	// моделька
	for(int i = 0; i < skin_num; i++)
	{
		delete m_skins[i];
	}
	delete []m_skins;
	delete []current_points;
	delete []velocities;
	delete []angle_velocities;
	delete []current_angles;
	delete []m_scales;
}

// следующий тик времени
// (только расчет координат)
bool ModelDestructEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;
	float dtt = current_time - previous_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}
	// расчет координат
	for(int i = 0; i < skin_num; i++)
	{
		velocities[i] += accel*dtt;
		current_points[i] += velocities[i]*dtt + accel*dtt*dtt*0.5;
		current_angles[i] += angle_velocities[i]*dtt;
		current_angles[i].x = fmod(current_angles[i].x, PIm2);
		current_angles[i].y = fmod(current_angles[i].y, PIm2);
		current_angles[i].z = fmod(current_angles[i].z, PIm2);
	}

	previous_time = current_time;
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool ModelDestructEffect::Update()
{
	// моделька
	for (int i = 0; i < skin_num; i++)
	{
		
		D3DMATRIX mat(
			FastCos(current_angles[i].y)*FastCos(current_angles[i].z)*m_scales[i],
			FastCos(current_angles[i].y)*FastSin(current_angles[i].z)*m_scales[i],
			-FastSin(current_angles[i].y)*m_scales[i],
			0,
			
			FastSin(current_angles[i].x)*FastSin(current_angles[i].y)*FastCos(current_angles[i].z) - FastCos(current_angles[i].x)*FastSin(current_angles[i].z)*m_scales[i],
			FastSin(current_angles[i].x)*FastSin(current_angles[i].y)*FastSin(current_angles[i].z) + FastCos(current_angles[i].x)*FastCos(current_angles[i].z)*m_scales[i],
			FastSin(current_angles[i].x)*FastCos(current_angles[i].y)*m_scales[i],
			0,
			
			FastCos(current_angles[i].x)*FastSin(current_angles[i].y)*FastCos(current_angles[i].z) + FastSin(current_angles[i].x)*FastSin(current_angles[i].z)*m_scales[i],
			FastCos(current_angles[i].x)*FastSin(current_angles[i].y)*FastSin(current_angles[i].z) - FastSin(current_angles[i].x)*FastCos(current_angles[i].z)*m_scales[i],
			FastCos(current_angles[i].x)*FastCos(current_angles[i].y)*m_scales[i],
			0,
			
			current_points[i].x,
			current_points[i].y,
			current_points[i].z,
			1
			);
		
		m_animadata.LastState.m_SkelState[0].World = mat;
		m_animadata.LastState.m_SkelState[1].World = mat;
		m_skins[i]->Update(&m_animadata.LastState);
		pGraphPipe->Chop(m_skins[i]->GetMesh());

	}
	return true;
}

// возвращает bounding box эффекта
BBox ModelDestructEffect::GetBBox(const float )
{
	return bbox;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool ModelDestructEffect::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = start_time;
	previous_time = start_time;
	return true;
}

// заполним точки привязки
void ModelDestructEffect::SetCurrentPoints(point3 * points,
										   unsigned int num,
										   const BBox* LocalBBox,
										   D3DMATRIX* Matrix)
{
	float dx = fabs(LocalBBox->maxx - LocalBBox->minx)*0.5;
	float dy = fabs(LocalBBox->maxy - LocalBBox->miny)*0.5;
	float dz = fabs(LocalBBox->maxz - LocalBBox->minz)*0.5;

	for(int i = 0; i < num; i++)
	{
		point3 pnt = LocalBBox->GetCenter() + point3(get_FF(dx),
			get_FF(dy), get_FF(dz));
		PointMatrixMultiply(*(reinterpret_cast<D3DVECTOR *>(&points[i])),
			*(reinterpret_cast<D3DVECTOR *>(&pnt)),
			*Matrix);
	}
}

// получить центр объекта
point3 ModelDestructEffect::getCenter(const BBox* LocalBBox, D3DMATRIX* Matrix)
{
	point3 p;
	PointMatrixMultiply(*(reinterpret_cast<D3DVECTOR *>(&p)),
		*(reinterpret_cast<D3DVECTOR *>(&LocalBBox->GetCenter())),
		*Matrix);
	return p;
}

