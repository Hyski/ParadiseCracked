/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: класс для ошметков мяса
				
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include "../../Skin/animalibrary.h"
#include "../../logic2/TraceUtils.h"

#include "EffectInfo.h"
#include "MeatEffect.h"

using namespace EffectUtility;

static const std::string SKELETON="Animations/anims/items/Fiery_Granade.skel";
static const std::string SKINDIR="Animations/skins/items/";
static const int models_num = 15;

std::string skin_name[models_num] = {
	"head", // 1
		"hand", // 2
		"hand", // 3
		"meat01", // 4
		"meat02", // 5
		"meat03", // 6
		"meat04", // 7
		"meat05", // 8
		"meat06", // 9
		"meat07", // 10
		"meat08", // 11
		"meat09", // 12
		"meat10", // 13
		"meat11", // 14
		"tors", // 15
};


//
// структура для хранения информации об одной модельке
//
struct MeatEffectParticle
{
	SkSkin* m_skin;
	KeyAnimation m_key_animation;
	point3 m_angle_vel;
	point3 m_angles;
	bool m_dead;
	float m_lifetime;
};


//////////////////////////////////////////////////////////////////////////////
//
// эффект ошметков мяса
//
//////////////////////////////////////////////////////////////////////////////

// конструктор с параметрами
MeatEffect::MeatEffect(
					   const unsigned int entity_id,
					   const point3& explosion,
					   const point3& human,
					   const MEAT_EFFECT_INFO& info,
					   float angle_vel,
					   float vel
					   )
{
	start_size = info.Size*0.5;
	end_size = info.SizeEnd*0.5;
	particles_lifetime = info.LifeTime;
	frequency = info.Frequency;

	// шейдер
	SetShader(info.Shader);
	
	bbox.Degenerate();
	bbox.Enlarge(human + point3(10.0, 10.0, 10.0));
	bbox.Enlarge(human - point3(10.0, 10.0, 10.0));

	m_models = new MeatEffectParticle[models_num];
	lifetime = 0.0f;

	// загрузим скелет
	AnimaLibrary *lib=AnimaLibrary::GetInst();
	SkAnim* skeleton = lib->GetSkAnimation(SKELETON);
	m_animadata = skeleton->Start(0);

	// создадим нужные трассы
	point3 dir = Normalize(human - explosion);
	for(int i = 0; i < models_num; i++)
	{
		float xx = get0F();
		float yy = get0F();
		float zz = get0F();
		point3 dir_i;
		dir_i.x = dir.x * xx;
		dir_i.y = dir.y * yy;
		dir_i.z = dir.z * zz;
		dir_i = Normalize(dir_i);
		
		_GrenadeTracer tracer(entity_id, human, dir_i, vel*get0F(), 0, 2.0f);
		m_models[i].m_key_animation = tracer.GetTrace();
		m_models[i].m_lifetime = m_models[i].m_key_animation.GetLastTime();
		if(m_models[i].m_lifetime > lifetime) lifetime = m_models[i].m_lifetime;
		m_models[i].m_angle_vel = point3(get0F(angle_vel), get0F(angle_vel), get0F(angle_vel));

		// загрузим скин
		m_models[i].m_skin = lib->GetSkSkin(SKINDIR + skin_name[i] + ".skin");
		if(!m_models[i].m_skin)
		{
			// выкинем исключение
			std::string str = "illegal skin: \"";
			str += SKINDIR + skin_name[i] + ".skin\"";
			for(int k = 0; k < i; k++)
			{
				delete m_models[k].m_skin;
			}
			delete []m_models;
			throw CASUS(str);
		}
		// прикрепим его к скелету
		m_models[i].m_skin->ValidateLinks(skeleton);
		m_models[i].m_dead = false;
	}

	// создать массивы
	points_num = ceil(frequency * lifetime * models_num);
	ready_vector_size = points_num*PEAK_NUM;
	points_vector = new point3 [points_num];
	size_vector = new float [points_num];
	points2d_vector = new point3 [points_num];
	color_vector = new unsigned int [ready_vector_size];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	particles_num = 0;


	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;
	// флаг, указывающий на необходимость завершения эффекта
	finished = false;

	// звука нет
	sound_object = new EmptySound();
}

// деструктор
MeatEffect::~MeatEffect()
{
	delete [] points_vector;
	delete [] size_vector;
	delete [] points2d_vector;
	delete [] color_vector;
	delete [] texcoord_vector;
	delete [] ready_vector;

	for(int i = 0; i < models_num; i++)
	{
		delete m_models[i].m_skin;
	}
	delete[] m_models;
	// убить звук
	delete sound_object;
}

// следующий тик времени
// (только расчет координат)
bool MeatEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	m_dt = current_time - start_time;

	if(m_dt < lifetime)
	{
		// все еще куда-то летит - эффект продолжается

		// посмотрим, сколько времени прошло с предыдущего рождения и,
		// если надо, создадим новые частицы

		// количество частиц, которые нужно создать
		int n = floor(frequency * (current_time - previous_birth_time));
		// цикл по моделькам
		for(int i = 0; i < models_num; i++)
		{
			if(m_models[i].m_dead) continue; // моделька уже долетела
			if(m_models[i].m_lifetime < m_dt)
			{
				// моделька долетела только что
				m_models[i].m_dead = true;
				continue;
			}
			// теперь модельки, которые еще летят
			point3 current_point;
			m_models[i].m_key_animation.GetTrans(&current_point, m_dt);
			// добавим частиц, если нужно
			if(n > 0)
			{
				// надо создать одну или более частиц
				float delta_t = (current_time - previous_birth_time) / n;
				for(int j = 1; j <= n; j++)
				{
					BloodParticle bp;
					bp.birthtime = previous_birth_time + j*delta_t;
					bp.start_point = current_point;// - velocity*(n - j)*delta_t; // FIXME
					bp.coords = bp.start_point;
					bp.lifetime = particles_lifetime;
					bp.velocity = point3(0, 0, -1.5f);
					bp.texcoord_type = (float)rand()/32768.0 * 4;
					
					particles.push_back(bp);
				} // for
			} // if(n > 0)
		}// for по моделькам
		if(n>0)
		{
			// запомним новое время рождения
			previous_birth_time = current_time;
		}
	}

	// удалим умершие частицы и просчитаем все для живых
	BloodParticlesListIterator I, J, E; // основной, дополнительный и конечный итераторы
	I = particles.begin();
	E = particles.end();
	while(I != E)
	{
		float dtt = (current_time - (*I).birthtime);
		if( dtt >= (*I).lifetime )
		{
			// удалим частицу
			J = I;
			I++;
			particles.erase(J);
			continue;
		}
		// это живая частица - пересчитаем для нее все
		(*I).size = start_size + (end_size - start_size)*dtt/(*I).lifetime;
		(*I).coords = (*I).start_point + (*I).velocity * dtt;
		unsigned int c = 255*(1 - dtt/(*I).lifetime);
		(*I).color = RGBA_MAKE(c, c, c, c);
		I++;
	}
	
	// если список пуст или время вышло - завершим эффект
	if(particles.empty() && (m_dt > lifetime))
	{
		finished = true;
		return true;
	}
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool MeatEffect::Update()
{
	// хвосты частиц
	float x1 = 0.0f, x2 = 1.0f, y1 = 0.0f, y2 = 1.0f;
	particles_num = 0;
	BloodParticlesListIterator I, E; // основной и конечный итераторы
	E = particles.end();
	for(I = particles.begin(); I != E; I++)
	{
		points_vector[particles_num] = (*I).coords;
		size_vector[particles_num] = (*I).size;

		color_vector[particles_num*PEAK_NUM] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 1] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 2] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 3] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 4] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 5] = (*I).color;

		switch((*I).texcoord_type)
		{
		case 0: x1 = 0; y1 = 0; x2 = 0.5; y2 = 0.5; break;
		case 1: x1 = 0.5; y1 = 0; x2 = 1; y2 = 0.5; break;
		case 2: x1 = 0; y1 = 0.5; x2 = 0.5; y2 = 1; break;
		case 3: x1 = 0.5; y1 = 0.5; x2 = 1; y2 = 1; break;
		}
		// прописываем текстурные координаты
		texcoord_vector[particles_num*PEAK_NUM] = texcoord(x1, y1);
		texcoord_vector[particles_num*PEAK_NUM + 1] = texcoord(x2, y1);
		texcoord_vector[particles_num*PEAK_NUM + 2] = texcoord(x2, y2);
		texcoord_vector[particles_num*PEAK_NUM + 3] = texcoord(x1, y1);
		texcoord_vector[particles_num*PEAK_NUM + 4] = texcoord(x2, y2);
		texcoord_vector[particles_num*PEAK_NUM + 5] = texcoord(x1, y2);

		particles_num++;
	}
	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(particles_num, points_vector, points2d_vector);
	// размножаем точки
	Multiply();

	// модельки
	for(int i = 0; i < models_num; i++)
	{
		if(m_models[i].m_dead) continue;
		point3 current_point;
		m_models[i].m_key_animation.GetTrans(&current_point, m_dt);
		
		m_models[i].m_angles.x = fmod(m_models[i].m_angle_vel.x * m_dt, PIm2);
		m_models[i].m_angles.y = fmod(m_models[i].m_angle_vel.y * m_dt, PIm2);
		m_models[i].m_angles.z = fmod(m_models[i].m_angle_vel.z * m_dt, PIm2);
		
		D3DMATRIX mat(
			FastCos(m_models[i].m_angles.y)*FastCos(m_models[i].m_angles.z),
			FastCos(m_models[i].m_angles.y)*FastSin(m_models[i].m_angles.z),
			-FastSin(m_models[i].m_angles.y),
			0,
			
			FastSin(m_models[i].m_angles.x)*FastSin(m_models[i].m_angles.y)*FastCos(m_models[i].m_angles.z) - FastCos(m_models[i].m_angles.x)*FastSin(m_models[i].m_angles.z),
			FastSin(m_models[i].m_angles.x)*FastSin(m_models[i].m_angles.y)*FastSin(m_models[i].m_angles.z) + FastCos(m_models[i].m_angles.x)*FastCos(m_models[i].m_angles.z),
			FastSin(m_models[i].m_angles.x)*FastCos(m_models[i].m_angles.y),
			0,
			
			FastCos(m_models[i].m_angles.x)*FastSin(m_models[i].m_angles.y)*FastCos(m_models[i].m_angles.z) + FastSin(m_models[i].m_angles.x)*FastSin(m_models[i].m_angles.z),
			FastCos(m_models[i].m_angles.x)*FastSin(m_models[i].m_angles.y)*FastSin(m_models[i].m_angles.z) - FastSin(m_models[i].m_angles.x)*FastCos(m_models[i].m_angles.z),
			FastCos(m_models[i].m_angles.x)*FastCos(m_models[i].m_angles.y),
			0,
			
			current_point.x,
			current_point.y,
			current_point.z,
			1);
		
		m_animadata.LastState.m_SkelState[0].World = mat;
		m_models[i].m_skin->Update(&m_animadata.LastState);
		pGraphPipe->Chop(m_models[i].m_skin->GetMesh());
	}

	return true;
}

bool MeatEffect::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = start_time;
	// цвет
	color = 0xffffffff;
	previous_birth_time = start_time;
	return true;
}

// функция размножения точек
void MeatEffect::Multiply()
{
	float f, n, betta;
	float xsize, xfactor;
	float ysize, yfactor;
	float z;
	int xres, yres;
	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);

	xfactor = xres/(n*f*tan(betta/2));
	yfactor = yres/(n*f*tan(betta/2));

	ready_num = 0;
	for(int i = 0; i < particles_num; i++)
	{
		// размножение координат частиц
		z = points2d_vector[i].z;

		// размножим координаты
		ready_vector[i*PEAK_NUM].z = z;
		ready_vector[i*PEAK_NUM + 1].z = z;
		ready_vector[i*PEAK_NUM + 2].z = z;
		ready_vector[i*PEAK_NUM + 3].z = z;
		ready_vector[i*PEAK_NUM + 4].z = z;
		ready_vector[i*PEAK_NUM + 5].z = z;

		xsize = size_vector[i]*(z*(n-f)+f)*xfactor;
		ysize = size_vector[i]*(z*(n-f)+f)*yfactor;

		ready_vector[i*PEAK_NUM].x = points2d_vector[i].x - xsize;
		ready_vector[i*PEAK_NUM + 1].x = points2d_vector[i].x + xsize;
		ready_vector[i*PEAK_NUM + 2].x = points2d_vector[i].x + xsize;
		ready_vector[i*PEAK_NUM + 3].x = points2d_vector[i].x - xsize;
		ready_vector[i*PEAK_NUM + 4].x = points2d_vector[i].x + xsize;
		ready_vector[i*PEAK_NUM + 5].x = points2d_vector[i].x - xsize;

		ready_vector[i*PEAK_NUM].y = points2d_vector[i].y + ysize;
		ready_vector[i*PEAK_NUM + 1].y = points2d_vector[i].y + ysize;
		ready_vector[i*PEAK_NUM + 2].y = points2d_vector[i].y - ysize;
		ready_vector[i*PEAK_NUM + 3].y = points2d_vector[i].y + ysize;
		ready_vector[i*PEAK_NUM + 4].y = points2d_vector[i].y - ysize;
		ready_vector[i*PEAK_NUM + 5].y = points2d_vector[i].y - ysize;

		// увеличим количество конечных точек
		ready_num += PEAK_NUM;
	}
}
