/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: огонь (эффект)
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "FireEffect.h"

//
// construction & destruction
//

// конструктор с параметрами
FireEffect::FireEffect(
					   const float quality,
					   const point3 rp,			   // начальная точка
					   const FIRE_EFFECT_INFO& info
					   )
{
	turns_left = info.TurnsNum;
	reduce_time = info.ReduceTime;
	reduce = false;
	finished = false;
	first_time = true;
	activity = 1.0;
	vel = point3(0, 0, 0);
	// создать массив частиц
	particles_num = 10 + 50*quality;
	particles = new FireParticle[particles_num];

	// создание массивов точек, цветов и текстурных координат
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num*PEAK_NUM;

	// создать массивы
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// точка, из которой горит огонь
	root_point = rp;

	// максимальная составляющая скорости по Х(Y) при старте эффекта
	startDX = info.VelXY;
	// максимальная составляющая скорости по Z при старте эффекта
	startDZ = info.VelZ;
	// минимальный начальный размер
	min_start_size = info.SizeBegin;
	// предел изменения начальногo размера
	size_range = info.SizeRnd;
	// коэффициент изменения размера частицы в зависимости от времени
	size_factor = info.SizeRange;
	// максимальное время жизни частицы
	max_lifetime = info.LifeTime;

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
FireEffect::~FireEffect()
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
void FireEffect::Multiply()
{
	if(activity == 0) return;
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
bool FireEffect::Start(const float start_time)
{
	int i;
	// запомнить время начала проработки эффекта
	this->start_time = start_time;

	// проинициализировать частицы
	for(i = 0; i < particles_num; i++)
	{
		int sw = (float)rand()/32768.0 * 4;
		//static int sw = 0;
		switch(sw)
		{
		case 0 : particles[i].texcoords1 = texcoord(0, 0); particles[i].texcoords2 = texcoord(0.5, 0.5); break;
		case 1 : particles[i].texcoords1 = texcoord(0.5, 0); particles[i].texcoords2 = texcoord(1, 0.5); break;
		case 2 : particles[i].texcoords1 = texcoord(0, 0.5); particles[i].texcoords2 = texcoord(0.5, 1); break;
		case 3 : particles[i].texcoords1 = texcoord(0.5, 0.5); particles[i].texcoords2 = texcoord(1, 1); break;
		}
		Rebirth(particles[i], start_time, 0.99);
	}

	return true;
}

// следующий тик времени
// (только расчет координат)
bool FireEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	if(activity == 0)
	{
		ready_num = 0;
		return true;
	}
	int c, c1, c2; // цвет
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
			dt = 0;
		}
		// координаты
		particles[i].coords = root_point + particles[i].velocity * dt;

		// цвет
		c = (particles[i].life_time - dt)*(255.0/particles[i].life_time);
		c1 = c * 0.7;
		c2 = c * 0.5;

		// умножим на текущую общую яркость
		c *= activity;
		c1 *= activity;
		c2 *= activity;
		
		if(reduce && ((current_time - start_reduce_time) < reduce_time))
		{
			float dtt = current_time - start_reduce_time;
			c *= 1.0 - dtt/reduce_time;
			c1 *= 1.0 - dtt/reduce_time;
			c2 *= 1.0 - dtt/reduce_time;
		}
		particles[i].color = RGBA_MAKE(c, c1, c2, c);
		
		//и  размер
		particles[i].size = min_start_size + dt*size_factor;
	}
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool FireEffect::Update()
{
	if(activity == 0) return true;
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

// функция обновляющая частицу при ее рождении
void FireEffect::Rebirth(FireParticle & sp, float curtime, float disp)
{

	// для случайных скоростей
	float dx, dy, dz, a, r;

	sp.birth_time = curtime;
	sp.color = 0xffffffff;
	sp.coords = root_point;

	sp.size = min_start_size + (float)rand()/32768.0*size_range;
	a = ((float)rand())/32768.0 * PIm2; // случайный угол альфа
	r = (float)rand()/32768.0 * startDX; // случайный радиус
	dx = r*cos(a);
	dy = r*sin(a);
	dz = (1.0-disp+disp*(float)rand()/32768.0 )* startDZ;
	sp.velocity = point3(dx, dy, dz);
	sp.life_time = (1.0-disp+disp*(float)rand()/32768.0) * max_lifetime;
	
}

// возвращает bounding box эффекта
BBox FireEffect::GetBBox(const float )
{
	BBox b;
	float dx, dy, dz;
	dx = startDX*max_lifetime;
	dy = startDX*max_lifetime;
	dz = startDZ*max_lifetime;
	b.maxx = root_point.x + dx;
	b.maxy = root_point.y + dy;
	b.maxz = root_point.z + dz;
	b.minx = root_point.x - dx;
	b.miny = root_point.y - dy;
	b.minz = root_point.z - dz;
	return b;
}
