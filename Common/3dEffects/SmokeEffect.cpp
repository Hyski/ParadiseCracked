/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: дым (эффект                                                                )
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "SmokeEffect.h"
#include "../../GameLevel/GameLevel.h"

//
// construction & destruction
//

// конструктор с параметрами
SmokeEffect::SmokeEffect(
						 const float quality,            // качество
						 const point3 rp,                // корневая точка
						 const SMOKE_EFFECT_INFO& info
		)
{
	turns_left = info.TurnsNum;
	reduce_time = info.ReduceTime;
	reduce = false;
	finished = false;
	// создать массив частиц
	particles_num = 10 + info.MaxPartNum*quality;
	particles = new SmokeParticle[particles_num];

	// создание массивов точек, цветов и текстурных координат
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num*PEAK_NUM;

	// создать массивы
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;

	// точка, из которой идет дым
	root_point = rp;
	// случайное изменение начальной точки - точки рождения частиц
	start_point_factor = info.BirthRange;

	// скорость
	start_velocity = info.Vel;
	// ускорение
	accel = info.Accel;
	// минимальный начальный размер
	min_start_size = info.SizeBegin;
	// предел изменения начальногo размера
	size_range = info.SizeRnd;
	// коэффициент изменения размера частицы в зависимости от времени
	size_factor = info.SizeRange;
	// максимальное время жизни частицы
	max_lifetime = info.LifeTime;
	m_exp = info.Exp;

	// шейдер
	SetShader(info.Shader);
	// звук
	if(info.Sound == "")
	{
		// звука нет
		sound_object = new EmptySound();
	}
	else
	{
		if(info.SndMov)
		{
			sound_object = new ActiveSound(info.Sound);
		}
		else
		{
			sound_object = new FixedSound(info.Sound, rp);
		}
	}
}


// деструктор
SmokeEffect::~SmokeEffect()
{
	delete[] particles;
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// звук
	delete sound_object;
}

//
// методы класса
//

// функция размножения точек
void SmokeEffect::Multiply()
{
	float f, n, betta;
	float xsize, xfactor;
	float ysize, yfactor;
	float z;
	int xres, yres;
	int k;
	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);

	xfactor = xres/(n*f*tan(betta/2));
	yfactor = yres/(n*f*tan(betta/2));

	// сначала считаем, что все точки видны, и количество готовых вершин 
	// равно размеру массива, в котором они лежат
	ready_num = ready_vector_size;
	// текущее количество точек, которые размножены в готовые вершины
	k = 0;  
	for(int i = 0; i<points_num; i++)
	{
		// размножение координат частиц
		z = points2d_vector[i].z;
		if(z < 0)
		{
			// эту точку не нужно размножать - пропустим ее
			// уменьшим количество готовых вершин
			ready_num -= PEAK_NUM;
			continue;
		}
		// размножим координаты
		ready_vector[k*PEAK_NUM].z = z;
		ready_vector[k*PEAK_NUM + 1].z = z;
		ready_vector[k*PEAK_NUM + 2].z = z;
		ready_vector[k*PEAK_NUM + 3].z = z;
		ready_vector[k*PEAK_NUM + 4].z = z;
		ready_vector[k*PEAK_NUM + 5].z = z;

		xsize = particles[i].size*(z*(n-f)+f)*xfactor;
		ysize = particles[i].size*(z*(n-f)+f)*yfactor;

		ready_vector[k*PEAK_NUM].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 1].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 2].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 3].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 4].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 5].x = points2d_vector[i].x - xsize;

		ready_vector[k*PEAK_NUM].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 1].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 2].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 3].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 4].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 5].y = points2d_vector[i].y + ysize;

		// размножим цвет
		color_vector[k*PEAK_NUM] = particles[i].color;
		color_vector[k*PEAK_NUM + 1] = particles[i].color;
		color_vector[k*PEAK_NUM + 2] = particles[i].color;
		color_vector[k*PEAK_NUM + 3] = particles[i].color;
		color_vector[k*PEAK_NUM + 4] = particles[i].color;
		color_vector[k*PEAK_NUM + 5] = particles[i].color;

		// размножим текстурные координаты
		texcoord_vector[k*PEAK_NUM] = particles[i].texcoords1;
		texcoord_vector[k*PEAK_NUM + 1] = texcoord(particles[i].texcoords2.u, particles[i].texcoords1.v);
		texcoord_vector[k*PEAK_NUM + 2] = particles[i].texcoords2;
		texcoord_vector[k*PEAK_NUM + 3] = particles[i].texcoords1;
		texcoord_vector[k*PEAK_NUM + 4] = particles[i].texcoords2;
		texcoord_vector[k*PEAK_NUM + 5] = texcoord(particles[i].texcoords1.u, particles[i].texcoords2.v);

		// увеличим количество размноженных точек
		k++;
	}
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool SmokeEffect::Start(const float start_time)
{
	// проинициализировать частицы
	for(int i = 0; i < particles_num; i++)
	{
		Rebirth(particles[i], start_time, 0.99);

		int sw = (float)rand()/32768.0 * 4;
		switch(sw)
		{
		case 0 : particles[i].texcoords1 = texcoord(0, 0); particles[i].texcoords2 = texcoord(0.5, 0.5); break;
		case 1 : particles[i].texcoords1 = texcoord(0.5, 0); particles[i].texcoords2 = texcoord(1, 0.5); break;
		case 2 : particles[i].texcoords1 = texcoord(0, 0.5); particles[i].texcoords2 = texcoord(0.5, 1); break;
		case 3 : particles[i].texcoords1 = texcoord(0.5, 0.5); particles[i].texcoords2 = texcoord(1, 1); break;
		}
	}

	return true;
}

// следующий тик времени
// (только расчет координат)
bool SmokeEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // цвет
	float dt; // разница во времени между текущим временем и временем рождения частицы (время жизни)
	
	// проверим не пора ли гасить эффект
	if( (!turns_left) && (!reduce))
	{
		reduce = true;
		start_reduce_time = current_time;
	}

	// проверим не пора ли завершать эффект
	if(reduce && ((current_time - start_reduce_time) > reduce_time))
	{
		finished = true;
	}

	// пересчитаем координаты, цвет и размер частицам
	for(int i = 0; i < particles_num; i++)
	{
		dt = current_time - particles[i].birth_time;
		if(dt > particles[i].life_time) // родить частицу заново
		{
			Rebirth(particles[i], current_time, 0.7);
			dt = current_time - particles[i].birth_time;
		}
		// цвет
		c = (particles[i].life_time - dt)*(255.0/particles[i].life_time);
		
		if(reduce && ((current_time - start_reduce_time) < reduce_time))
		{
			float dtt = current_time - start_reduce_time;
			c *= 1.0 - dtt/reduce_time;
		}
		particles[i].color = RGBA_MAKE(c, c, c, c);
		
		// размер
		if(m_exp) particles[i].size = min_start_size + size_factor*dt*dt*dt;
		else particles[i].size = min_start_size + dt*size_factor;

		particles[i].coords =root_point + particles[i].velocity*dt + accel*dt*dt*0.5;
	}
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool SmokeEffect::Update()
{
	// звук
	sound_object->Update(root_point, start_velocity);
	// подготовить массив для проецирования (points_vector)
	for(int i = 0; i < particles_num; i++)
	{
		points_vector[i] = particles[i].coords;
	}
	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(points_num,
		points_vector, points2d_vector);
	// размножаем точки
	Multiply();
	return true;
}

// функция обновляющая частицу при ее рождении
void SmokeEffect::Rebirth(SmokeParticle & sp, float curtime, float disp)
{
	// для случайных скоростей
	float dx, dy, dz, a, r;

	sp.birth_time = curtime;
	sp.color = 0xffffffff;

#define rnd (1 - (float)rand()/16384.0)*start_point_factor
	sp.coords = root_point + point3(rnd, rnd, rnd);
#undef rnd
	
	sp.size = min_start_size + (float)rand()/32768.0*size_range;
	a = ((float)rand())/32768.0 * PIm2; // случайный угол альфа
	r = (0.8 + 0.2*(float)rand()/32768.0) * start_velocity.x; // случайный радиус
	dx = r*cos(a);
	dy = r*sin(a);
	dz = (1.0-disp+disp*(float)rand()/32768.0 )* start_velocity.z;
	sp.velocity = point3(dx, dy, dz);
	sp.life_time = (1.0-disp+disp*(float)rand()/32768.0) * max_lifetime;
}

// возвращает bounding box эффекта
BBox SmokeEffect::GetBBox(const float )
{
	BBox b;
	float dx, dy, dz, dt;
	dt = max_lifetime;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	b.maxx = root_point.x + dx;
	b.maxy = root_point.y + dy;
	b.maxz = root_point.z + dz;
	b.minx = root_point.x - dx;
	b.miny = root_point.y - dy;
	b.minz = root_point.z - dz;
	return b;
}


//
// класс тяжелого дыма
//


// конструктор с параметрами
HeavySmokeEffect::HeavySmokeEffect(
								   const float quality,            // качество
								   const point3 rp,                // корневая точка
								   const float st_point_factor,    // разброс вокруг начальной точки
								   const point3 stVelocity,        // начальная скорость
								   const point3 Accel,             // ускорение
								   const float stSIZE,             // начальный размер
								   const float stRANGE,            // случайная часть начального размера
								   const float sizefactor,         // прирост размера
								   const float maxlifetime,        // максимальное время жизни
								   const int MaxPartNum,           // максимальное количество частиц
								   const std::string& Shader,      // имя шейдера
								   const std::string& Sound,       // имя звукового шейдера
								   const bool SoundMove            // двигается ли звук
		)
{
	// создать массив частиц
	particles_num = 10 + MaxPartNum*quality;
	particles = new SmokeParticle[particles_num];

	// создание массивов точек, цветов и текстурных координат
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num*PEAK_NUM;

	// создать массивы
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;

	// точка, из которой идет дым
	root_point = rp;
	// случайное изменение начальной точки - точки рождения частиц
	start_point_factor = st_point_factor;

	// скорость
	start_velocity = stVelocity;
	// ускорение
	accel = Accel;
	// минимальный начальный размер
	min_start_size = stSIZE;
	// предел изменения начальногo размера
	size_range = stRANGE;
	// коэффициент изменения размера частицы в зависимости от времени
	size_factor = sizefactor;
	// максимальное время жизни частицы
	max_lifetime = maxlifetime;

	// шейдер
	SetShader(Shader);
	// звук
	if(Sound == "")
	{
		// звука нет
		sound_object = new EmptySound();
	}
	else
	{
		if(SoundMove)
		{
			sound_object = new ActiveSound(Sound);
		}
		else
		{
			sound_object = new FixedSound(Sound, rp);
		}
	}
}


// деструктор
HeavySmokeEffect::~HeavySmokeEffect()
{
	delete[] particles;
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// звук
	delete sound_object;
}

//
// методы класса
//

// функция размножения точек
void HeavySmokeEffect::Multiply()
{
	float f, n, betta;
	float xsize, xfactor;
	float ysize, yfactor;
	float z;
	int xres, yres;
	int k;
	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);

	xfactor = xres/(n*f*tan(betta/2));
	yfactor = yres/(n*f*tan(betta/2));

	// сначала считаем, что все точки видны, и количество готовых вершин 
	// равно размеру массива, в котором они лежат
	ready_num = ready_vector_size;
	// текущее количество точек, которые размножены в готовые вершины
	k = 0;  
	for(int i = 0; i<points_num; i++)
	{
		// размножение координат частиц
		z = points2d_vector[i].z;
		if(z < 0)
		{
			// эту точку не нужно размножать - пропустим ее
			// уменьшим количество готовых вершин
			ready_num -= PEAK_NUM;
			continue;
		}
		// размножим координаты
		ready_vector[k*PEAK_NUM].z = z;
		ready_vector[k*PEAK_NUM + 1].z = z;
		ready_vector[k*PEAK_NUM + 2].z = z;
		ready_vector[k*PEAK_NUM + 3].z = z;
		ready_vector[k*PEAK_NUM + 4].z = z;
		ready_vector[k*PEAK_NUM + 5].z = z;

		xsize = particles[i].size*(z*(n-f)+f)*xfactor;
		ysize = particles[i].size*(z*(n-f)+f)*yfactor;

		ready_vector[k*PEAK_NUM].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 1].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 2].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 3].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 4].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 5].x = points2d_vector[i].x - xsize;

		ready_vector[k*PEAK_NUM].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 1].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 2].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 3].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 4].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 5].y = points2d_vector[i].y + ysize;

		// размножим цвет
		color_vector[k*PEAK_NUM] = particles[i].color;
		color_vector[k*PEAK_NUM + 1] = particles[i].color;
		color_vector[k*PEAK_NUM + 2] = particles[i].color;
		color_vector[k*PEAK_NUM + 3] = particles[i].color;
		color_vector[k*PEAK_NUM + 4] = particles[i].color;
		color_vector[k*PEAK_NUM + 5] = particles[i].color;

		// размножим текстурные координаты
		texcoord_vector[k*PEAK_NUM] = particles[i].texcoords1;
		texcoord_vector[k*PEAK_NUM + 1] = texcoord(particles[i].texcoords2.u, particles[i].texcoords1.v);
		texcoord_vector[k*PEAK_NUM + 2] = particles[i].texcoords2;
		texcoord_vector[k*PEAK_NUM + 3] = particles[i].texcoords1;
		texcoord_vector[k*PEAK_NUM + 4] = particles[i].texcoords2;
		texcoord_vector[k*PEAK_NUM + 5] = texcoord(particles[i].texcoords1.u, particles[i].texcoords2.v);

		// увеличим количество размноженных точек
		k++;
	}
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool HeavySmokeEffect::Start(const float start_time)
{
	// прописываем текстурные координаты
	for(int i = 0; i < particles_num; i++)
	{	
		Rebirth(particles[i], start_time, 0.99);

		int sw = (float)rand()/32768.0 * 4;
		switch(sw)
		{
		case 0 : particles[i].texcoords1 = texcoord(0, 0); particles[i].texcoords2 = texcoord(0.5, 0.5); break;
		case 1 : particles[i].texcoords1 = texcoord(0.5, 0); particles[i].texcoords2 = texcoord(1, 0.5); break;
		case 2 : particles[i].texcoords1 = texcoord(0, 0.5); particles[i].texcoords2 = texcoord(0.5, 1); break;
		case 3 : particles[i].texcoords1 = texcoord(0.5, 0.5); particles[i].texcoords2 = texcoord(1, 1); break;
		}
	}

	return true;
}

// следующий тик времени
// (только расчет координат)
bool HeavySmokeEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // цвет
	float dt; // разница во времени между текущим временем и временем рождения частицы (время жизни)
	// пересчитаем координаты, цвет и размер частицам
	for(int i = 0; i < particles_num; i++)
	{
		dt = current_time - particles[i].birth_time;
		if(dt > particles[i].life_time) // родить частицу заново
		{
			Rebirth(particles[i], current_time, 0.7);
			dt = current_time - particles[i].birth_time;
		}
		// цвет
		c = (particles[i].life_time - dt)*(255.0/particles[i].life_time);
		particles[i].color = RGBA_MAKE(c, c, c, c);
		
		// размер
		particles[i].size = min_start_size + dt*size_factor;

		// координаты 
		particles[i].coords =root_point + particles[i].velocity*dt + accel*dt*dt*0.5;
	}
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool HeavySmokeEffect::Update()
{
	// звук
	sound_object->Update(root_point, start_velocity);
	// подготовить массив для проецирования (points_vector)
	for(int i = 0; i < particles_num; i++)
	{
		points_vector[i] = particles[i].coords;
	}
	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(points_num,
		points_vector, points2d_vector);
	// размножаем точки
	Multiply();
	return true;
}

// функция обновляющая частицу при ее рождении
void HeavySmokeEffect::Rebirth(SmokeParticle & sp, float curtime, float disp)
{
	// для случайных скоростей
	float dx, dy, dz, a, r;

	sp.birth_time = curtime;
	sp.color = 0xffffffff;

#define rnd (1 - (float)rand()/16384.0)*start_point_factor
	sp.coords = root_point + point3(rnd, rnd, rnd);
#undef rnd
	
	sp.size = min_start_size + (float)rand()/32768.0*size_range;
	a = ((float)rand())/32768.0 * PIm2; // случайный угол альфа
	r = (0.8 + 0.2*(float)rand()/32768.0) * start_velocity.x; // случайный радиус
	dx = r*cos(a);
	dy = r*sin(a);
	dz = (1.0-disp+disp*(float)rand()/32768.0 )* start_velocity.z;
	sp.velocity = point3(dx, dy, dz);
	sp.life_time = (1.0-disp+disp*(float)rand()/32768.0) * max_lifetime;
}

// возвращает bounding box эффекта
BBox HeavySmokeEffect::GetBBox(const float )
{
	BBox b;
	float dx, dy, dz, dt;
	dt = max_lifetime;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	b.maxx = root_point.x + dx;
	b.maxy = root_point.y + dy;
	b.maxz = root_point.z + dz;
	b.minx = root_point.x - dx;
	b.miny = root_point.y - dy;
	b.minz = root_point.z - dz;
	return b;
}

//
// класс мух
//

// конструктор с параметрами
FlyEffect::FlyEffect(
					 const float quality,            // качество
					 const point3 rp,                // корневая точка
					 const FLY_EFFECT_INFO& info
					 )
{
	// создать массив частиц
	particles_num = 2 + info.MaxPartNum*quality;
	particles = new FlyParticle[particles_num];

	// создание массивов точек, цветов и текстурных координат
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num*PEAK_NUM;

	// создать массивы
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;

	// центральная точка
	root_point = rp;
	// минимальный начальный размер
	min_start_size = info.Size;
	// предел изменения начальногo размера
	size_range = info.SizeRnd;

	min_radius = info.MinRadius;
	radius_range = info.RadiusRange;
	min_vel = info.MinVel;

	vel = point3(0.0, 0.0, 0.0);

	// шейдер
	SetShader(info.Shader);
	// звук
	if(info.Sound == "")
	{
		// звука нет
		sound_object = new EmptySound();
	}
	else
	{
		if(info.SndMov)
		{
			sound_object = new ActiveSound(info.Sound);
		}
		else
		{
			sound_object = new FixedSound(info.Sound, rp);
		}
	}
}

// деструктор
FlyEffect::~FlyEffect()
{
	delete[] particles;
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// звук
	delete sound_object;
}

// следующий тик времени
// (только расчет координат)
bool FlyEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dtt = current_time - previous_time;
	for(int i = 0; i < particles_num; i++)
	{
		particles[i].alpha = fmod(particles[i].alpha + dtt*particles[i].vel.x, PIm2);
		particles[i].betta = fmod(particles[i].betta + dtt*particles[i].vel.y, PIm2);
		particles[i].gamma = fmod(particles[i].gamma + dtt*particles[i].vel.z, PIm2);
		//particles[i].r = particles[i].radius + 0.5*sin(particles[i].gamma);
		particles[i].r = particles[i].radius*sin(particles[i].gamma);
		particles[i].setCoords(root_point);
	}
	previous_time = current_time;
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool FlyEffect::Update()
{
	// звук
	sound_object->Update(root_point, vel);
	// подготовить массив для проецирования (points_vector)
	for(int i = 0; i < particles_num; i++)
	{
		points_vector[i] = particles[i].coords;
	}
	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(points_num,
		points_vector, points2d_vector);
	// размножаем точки
	Multiply();
	return true;
}

// возвращает bounding box эффекта
BBox FlyEffect::GetBBox(const float )
{
	return bb;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool FlyEffect::Start(const float start_time)
{
	previous_time = start_time;
	// проинициализировать частицы
	for(int i = 0; i < particles_num; i++)
	{
		particles[i].size = min_start_size + size_range * (float) rand()/32768.0f;

		particles[i].radius = min_radius + radius_range * (float)rand()/32768.0;
		particles[i].alpha = PIm2 * (float)rand()/32768.0;
		particles[i].betta = PIm2 * (float)rand()/32768.0;
		particles[i].gamma = PIm2 * (float)rand()/32768.0;

		particles[i].vel.x = min_vel.x + 2.0*(float)rand()/32768.0;
		if(rand() > 16384) particles[i].vel.x = -particles[i].vel.x;
		particles[i].vel.y = min_vel.y + 2.0*(float)rand()/32768.0;
		if(rand() > 16384) particles[i].vel.y = -particles[i].vel.y;
		particles[i].vel.z = min_vel.z + 2.0*(float)rand()/32768.0;
		if(rand() > 16384) particles[i].vel.z = -particles[i].vel.z;

		color_vector[i*PEAK_NUM] = 0xffffffff;
		color_vector[i*PEAK_NUM + 1] = 0xffffffff;
		color_vector[i*PEAK_NUM + 2] = 0xffffffff;
		color_vector[i*PEAK_NUM + 3] = 0xffffffff;
		color_vector[i*PEAK_NUM + 4] = 0xffffffff;
		color_vector[i*PEAK_NUM + 5] = 0xffffffff;

		texcoord_vector[i*PEAK_NUM] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, 1);
	}
	
	bb.Degenerate();
	bb.Enlarge(root_point + point3(min_radius + radius_range, min_radius + radius_range, min_radius + radius_range));
	bb.Enlarge(root_point - point3(min_radius + radius_range, min_radius + radius_range, min_radius + radius_range));

	return true;
}

// функция размножения точек
void FlyEffect::Multiply()
{
	float f, n, betta;
	float xsize, xfactor;
	float ysize, yfactor;
	float z;
	int xres, yres;
	int k;
	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);

	xfactor = xres/(n*f*tan(betta/2));
	yfactor = yres/(n*f*tan(betta/2));

	// сначала считаем, что все точки видны, и количество готовых вершин 
	// равно размеру массива, в котором они лежат
	ready_num = ready_vector_size;
	// текущее количество точек, которые размножены в готовые вершины
	k = 0;  
	for(int i = 0; i<points_num; i++)
	{
		// размножение координат частиц
		z = points2d_vector[i].z;
		if(z < 0)
		{
			// эту точку не нужно размножать - пропустим ее
			// уменьшим количество готовых вершин
			ready_num -= PEAK_NUM;
			continue;
		}
		// размножим координаты
		ready_vector[k*PEAK_NUM].z = z;
		ready_vector[k*PEAK_NUM + 1].z = z;
		ready_vector[k*PEAK_NUM + 2].z = z;
		ready_vector[k*PEAK_NUM + 3].z = z;
		ready_vector[k*PEAK_NUM + 4].z = z;
		ready_vector[k*PEAK_NUM + 5].z = z;

		xsize = particles[i].size*(z*(n-f)+f)*xfactor;
		ysize = particles[i].size*(z*(n-f)+f)*yfactor;

		ready_vector[k*PEAK_NUM].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 1].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 2].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 3].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 4].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 5].x = points2d_vector[i].x - xsize;

		ready_vector[k*PEAK_NUM].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 1].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 2].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 3].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 4].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 5].y = points2d_vector[i].y + ysize;

			// увеличим количество размноженных точек
		k++;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс мух с хвостами, летающих по цилиндру
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// конструктор с параметрами
FlyCylinderEffect::FlyCylinderEffect(
									 const float quality,            // качество
									 const point3 rp,                // корневая точка
									 const FLY_CYLINDER_EFFECT_INFO& info
									 )
{
	// прочитаем параметры эффекта
	m_height = info.Height;
	m_particles_num = 5 + info.MaxPartNum*quality;
	m_deltat = info.PartDeltat;
	m_part_size = info.PartSize;
	m_radius = info.Radius;
	m_tail_part_num = info.TailPartNum;
	m_velh = info.VelH;
	m_velalpha = info.VelAlpha;
	m_color = info.Color;

	// создание массивов точек, цветов и текстурных координат
	points_num = m_particles_num*(m_tail_part_num + 1);
	ready_vector_size = points_num*PEAK_NUM;
	ready_num = 0; // будет динамически меняться

	// создать массивы
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;

	// корневая точка
	m_root = rp;

	// шейдер
	SetShader(info.Shader);
	// звук
	if(info.Sound == "")
	{
		// звука нет
		sound_object = new EmptySound();
	}
	else
	{
		if(info.SndMov)
		{
			sound_object = new ActiveSound(info.Sound);
		}
		else
		{
			sound_object = new FixedSound(info.Sound, rp);
		}
	}
	// скорость для звука
	m_vel = point3(0.0f, 0.0f, 0.0f);
}

// деструктор
FlyCylinderEffect::~FlyCylinderEffect()
{
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// звук
	delete sound_object;
}

// следующий тик времени
// (только расчет координат)
bool FlyCylinderEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dtt = current_time - m_previous_time;
	m_not_delayed_particles.clear();
	for(unsigned int i = 0; i < m_particles.size(); i++)
	{
		// проверим задержку
		if(m_particles[i].delay > 0.0f)
		{
			// обновим задержку
			m_particles[i].delay -= dtt;
			// перейдем к следующей частице
			continue;
		}
		// обновление координат
		m_particles[i].h += m_particles[i].vh*dtt;
		m_particles[i].alpha = fmod(m_particles[i].alpha + dtt*m_particles[i].valpha, PIm2);
		// если вылетели вверх или вниз - изменим скорость
		// по высоте на противоположную и подправим координату по высоте
		if( (m_particles[i].h > m_height) &&
			(m_particles[i].vh > 0.0f)
			)
		{
			m_particles[i].vh *= -1.0f;
			m_particles[i].h -= (m_particles[i].h - m_height)*2.0f;
		}
		if( (m_particles[i].h < 0.0f) &&
			(m_particles[i].vh < 0.0f)
			)
		{
			m_particles[i].vh *= -1.0f;
			m_particles[i].h -= m_particles[i].h*2.0f;
		}
		m_particles[i].setCoords(m_root, m_radius);
		// добавим в вектор частицу
		m_not_delayed_particles.push_back(m_particles[i]);
	}
	// проверим время до новой отметины
	if(m_mark_time <= 0.0f)
	{
		// создание вспышки света
		DMark *dmk = new DMark(m_root, m_radius*2.0f,
			"dynlight",
			m_color,
			Timer::GetSeconds(), 5.0f,
			2.0, 0);
		
		IWorld::Get()->GetLevel()->AddMark(dmk);
		m_mark_time += 2.5f;
	}
	else
	{
		m_mark_time -= dtt;
	}
	m_previous_time = current_time;
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool FlyCylinderEffect::Update()
{
	// звук
	sound_object->Update(m_root, m_vel);
	// подготовить массив для проецирования (points_vector)
	for(unsigned int i = 0; i < m_not_delayed_particles.size(); i++)
	{
		points_vector[i] = m_not_delayed_particles[i].coords;
	}
	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(m_not_delayed_particles.size(),
		points_vector, points2d_vector);
	// размножаем точки
	Multiply();
	return true;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool FlyCylinderEffect::Start(const float start_time)
{
	m_particles.reserve(points_num);
	m_not_delayed_particles.reserve(points_num);
	m_previous_time = start_time;
	m_mark_time = 0.0f;
	// проинициализировать частицы
	for(unsigned int i = 0; i < m_particles_num; i++)
	{
		// инициализируем основную частицу
		FlyCylinderParticle main_particle;
		main_particle.alpha = EffectUtility::get0F(PIm2);
		main_particle.h = EffectUtility::get0F(m_height);
		main_particle.size = m_part_size;
		main_particle.setCoords(m_root, m_radius);
		main_particle.color = 0xffffffff;
		main_particle.vh = m_velh + EffectUtility::get_FF(0.2f*m_velh);
		if(EffectUtility::get0F() > 0.5f) main_particle.vh *= -1.0f;
		main_particle.valpha = m_velalpha + EffectUtility::get_FF(0.3f*m_velalpha);
		if(EffectUtility::get0F() > 0.5f) main_particle.valpha *= -1.0f;
		main_particle.delay = 0;
		// добавим ее в вектор
		m_particles.push_back(main_particle);
		// проинициализируем и добавим частицы хвоста
		for(unsigned int j = 0; j < m_tail_part_num; j++)
		{
			// инициализируем хвостовую частицу
			FlyCylinderParticle particle = main_particle;
			particle.size *= 1.0f - static_cast<float>(j + 1)/m_tail_part_num;
			particle.color *= 1.0f - static_cast<float>(j + 1)/m_tail_part_num;
			particle.delay = m_deltat*(j + 1);
			// добавим ее в вектор
			m_particles.push_back(particle);
		}
	}
	// проинициализируем текстурные координаты
	for(int i = 0; i < points_num; i++)
	{
		texcoord_vector[i*PEAK_NUM] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, 1);
	}
	// проинициализируем bbox
	m_bbox.Degenerate();
	m_bbox.Enlarge(m_root - point3(m_radius, m_radius, 0.0f));
	m_bbox.Enlarge(m_root + point3(m_radius, m_radius, m_height));
	return true;
}

// функция размножения точек
void FlyCylinderEffect::Multiply()
{
	float f, n, betta;
	float xsize, xfactor;
	float ysize, yfactor;
	float z;
	int xres, yres;
	int k;
	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);

	xfactor = xres/(n*f*tan(betta/2));
	yfactor = yres/(n*f*tan(betta/2));

	// сначала считаем, что все точки видны, и количество готовых вершин 
	// равно размеру массива, в котором они лежат
	ready_num = m_not_delayed_particles.size()*PEAK_NUM;
	// текущее количество точек, которые размножены в готовые вершины
	k = 0;  
	for(unsigned int i = 0; i<m_not_delayed_particles.size(); i++)
	{
		// размножение координат частиц
		z = points2d_vector[i].z;
		if(z < 0)
		{
			// эту точку не нужно размножать - пропустим ее
			// уменьшим количество готовых вершин
			ready_num -= PEAK_NUM;
			continue;
		}
		// размножим координаты
		ready_vector[k*PEAK_NUM].z = z;
		ready_vector[k*PEAK_NUM + 1].z = z;
		ready_vector[k*PEAK_NUM + 2].z = z;
		ready_vector[k*PEAK_NUM + 3].z = z;
		ready_vector[k*PEAK_NUM + 4].z = z;
		ready_vector[k*PEAK_NUM + 5].z = z;

		xsize = m_not_delayed_particles[i].size*(z*(n-f)+f)*xfactor;
		ysize = m_not_delayed_particles[i].size*(z*(n-f)+f)*yfactor;

		ready_vector[k*PEAK_NUM].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 1].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 2].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 3].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 4].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 5].x = points2d_vector[i].x - xsize;

		ready_vector[k*PEAK_NUM].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 1].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 2].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 3].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 4].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 5].y = points2d_vector[i].y + ysize;

		// цвет
		color_vector[k*PEAK_NUM] = m_not_delayed_particles[i].color;
		color_vector[k*PEAK_NUM + 1] = m_not_delayed_particles[i].color;
		color_vector[k*PEAK_NUM + 2] = m_not_delayed_particles[i].color;
		color_vector[k*PEAK_NUM + 3] = m_not_delayed_particles[i].color;
		color_vector[k*PEAK_NUM + 4] = m_not_delayed_particles[i].color;
		color_vector[k*PEAK_NUM + 5] = m_not_delayed_particles[i].color;
		// увеличим количество размноженных точек
		k++;
	}
}

