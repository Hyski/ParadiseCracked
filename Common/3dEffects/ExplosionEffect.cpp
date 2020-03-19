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
#include "ExplosionEffect.h"



//
// класс взрыва
//


// подготовка массива готовых точек и массива цветов точек
bool ExplosionEffect::Update()
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



// функция размножения точек
void ExplosionEffect::Multiply()
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

		ready_vector[k*PEAK_NUM].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 1].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 2].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 3].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 4].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 5].y = points2d_vector[i].y - ysize;

		// размножим цвет
		color_vector[k*PEAK_NUM] = particles[i].color;
		color_vector[k*PEAK_NUM + 1] = particles[i].color;
		color_vector[k*PEAK_NUM + 2] = particles[i].color;
		color_vector[k*PEAK_NUM + 3] = particles[i].color;
		color_vector[k*PEAK_NUM + 4] = particles[i].color;
		color_vector[k*PEAK_NUM + 5] = particles[i].color;

		// увеличим количество размноженных точек
		k++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// класс взрыва гранаты
//////////////////////////////////////////////////////////////////////////////////////////////

// конструктор с параметрами
GrenadeEffect::GrenadeEffect(const float quality,        // качество
							 const point3 rp,            // корневая точка
							 const GRENADE_EFFECT_INFO& info
							 )
{
	// создать массив частиц
	particles_num =  1 + 8*quality;
	particles = new ExplosionParticle[particles_num];

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

	start_velocity = info.Vel;          // начальная скорость
	accel = info.Accel;                           // ускорение
	start_size = info.Size0;                    // начальный размер
	start_size_range = info.SizeRnd;       // случайная часть начального размера
	size_factor = info.SizeRange;            // изменение размера

	lifetime = info.LifeTime;         // максимальное время жизни
	// флаг, указывающий на необходимость завершения эффекта
	finished = false;

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
GrenadeEffect::~GrenadeEffect()
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
bool GrenadeEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // цвет
	float s; // размер
	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;
	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	c = (lifetime - dt)*(255.0/lifetime);
	c = RGBA_MAKE(c, (int)(c*0.7), (int)(c*0.7), (int)(c*0.7));

	s = start_size + sqrt(dt)*size_factor;

	// пересчитаем координаты, цвет и размер
	for(int i = 0; i < particles_num; i++)
	{
		// цвет
		particles[i].color = c;
		
		// размер
		particles[i].size = s;

		// координаты 
		particles[i].coords =  particles[i].start_point + particles[i].velocity*dt + accel*dt*dt/2.0;
	}
	return true;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool GrenadeEffect::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = start_time;

	// проинициализировать частицы
	for(int i = 0; i < particles_num; i++)	Rebirth(particles[i], start_time);

	// прописываем текстурные координаты
	for(int i = 0; i < particles_num; i++)
	{	
		texcoord_vector[i*PEAK_NUM] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, 1);
	}
	return true;
}

// функция обновляющая частицу при ее рождении
void GrenadeEffect::Rebirth(ExplosionParticle & ep, float )
{
	// для случайных скоростей
	float dx, dy, dz, a, b, r;
	// случайное место рождения
	dx = (0.5 - (float)rand()/32768.0) * 0.9;
	dy = (0.5 - (float)rand()/32768.0) * 0.9;
	dz = (0.5 - (float)rand()/32768.0) * 0.5;
	ep.coords = root_point + point3(dx, dy, dz);
	ep.start_point = root_point + point3(dx, dy, dz);
	a = ((float)rand())/32768.0 * PIm2; // случайный угол альфа
	b = ((float)rand())/32768.0 * PId2; // случайный угол бетта

	ep.size = start_size + (float)rand()/32768.0*start_size_range;
	r = (0.8 + 0.2*(float)rand()/32768.0); // случайный радиус
	dx = r*cos(b)*cos(a) * start_velocity.x;
	dz = r*sin(b)  * start_velocity.z;
	dy = r*cos(b)*sin(a)  * start_velocity.y;
	// цвет
	ep.color = 0xffffa0a0;
	ep.velocity = point3(dx, dy, dz);
}

// возвращает bounding box эффекта
BBox GrenadeEffect::GetBBox(const float current_time)
{
	BBox b;
	float dx, dy, dz, dt, s;
	dt = current_time - start_time;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	s = start_size + sqrt(dt)*size_factor;
	b.maxx = root_point.x + dx + s;
	b.maxy = root_point.y + dy + s;
	b.maxz = root_point.z + dz + s;
	b.minx = root_point.x - dx - s;
	b.miny = root_point.y - dy - s;
	b.minz = root_point.z - dz - s;
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// класс разлетающихся искр или осколков
//////////////////////////////////////////////////////////////////////////////////////////////

// конструктор с параметрами
SparkleEffect::SparkleEffect(
							 const point3 rp,            // корневая точка
							 const SPARKLE_EFFECT_INFO& info
	)
{
	// создать массив частиц
	particles_num = info.PartNum;
	particles = new ExplosionParticle[particles_num];

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

	// корневая точка
	root_point = rp;

	start_velocity = info.Vel;          // начальная скорость
	accel = info.Accel;                           // ускорение
	start_size = info.Size0;                    // начальный размер
	start_size_range = info.SizeRnd;       // случайная часть начального размера
	size_factor = info.SizeRange;            // изменение размера

	lifetime = info.LifeTime;         // максимальное время жизни
	full = info.Full;                    // если true, то полная сфера, иначе - половина
	// флаг, указывающий на необходимость завершения эффекта
	finished = false;

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
SparkleEffect::~SparkleEffect()
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
bool SparkleEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // цвет
	float s; // размер
	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	c = (lifetime - dt)*(255.0/lifetime);
	c = RGBA_MAKE(0xff, c, c, c);
	s = start_size + dt*size_factor;

	// пересчитаем координаты, цвет и размер
	for(int i = 0; i < particles_num; i++)
	{
		// цвет
		particles[i].color = c;
		
		// размер
		particles[i].size = s;

		// координаты 
		particles[i].coords = root_point + particles[i].velocity*dt + accel*dt*dt/2.0;
	}

	// новое предыдущее время
	prev_time = current_time;
	return true;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool SparkleEffect::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = this->prev_time = start_time;

	// проинициализировать частицы
	for(int i = 0; i < particles_num; i++)	Rebirth(particles[i], start_time);

	// прописываем текстурные координаты
	for(int i = 0; i < particles_num; i++)
	{	
		texcoord_vector[i*PEAK_NUM] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, 1);
	}
	return true;
}

// функция обновляющая частицу при ее рождении
void SparkleEffect::Rebirth(ExplosionParticle & ep, float )
{
	// для случайных скоростей
	float dx, dy, dz, a, b, r;
	ep.coords = root_point;
	a = ((float)rand())/32768.0 * PIm2; // случайный угол альфа
	// случайный угол бетта
	if(full) b = ((float)rand())/32768.0 * PIm2;
	else b = ((float)rand())/32768.0 * PI;

	ep.size = start_size + (float)rand()/32768.0*start_size_range;
	r = (0.8 + 0.2*(float)rand()/32768.0); // случайный радиус
	dx = r*cos(b)*cos(a) * start_velocity.x;
	dz = r*sin(b)  * start_velocity.z;
	dy = r*cos(b)*sin(a)  * start_velocity.y;
	// цвет
	ep.color = 0xffffffff;
	ep.velocity = point3(dx, dy, dz);
}

// возвращает bounding box эффекта
BBox SparkleEffect::GetBBox(const float current_time)
{
	BBox b;

	float dx, dy, dz, dt;
	dt = current_time - start_time;
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

/////////////////////////////////////////////////////////////////////////////////////////////////
// класс дыма для взрывов
/////////////////////////////////////////////////////////////////////////////////////////////////

// конструктор с параметрами
ExplosionSmokeEffect::ExplosionSmokeEffect(const float quality,        // качество
										   const point3 rp,            // корневая точка
										   const EXPLOSION_SMOKE_INFO& info
	)
{
	// создать массив частиц
	particles_num = 1 + 4*quality;
	particles = new ExplosionParticle[particles_num];

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

	start_velocity = info.Vel;          // начальная скорость
	accel = info.Accel;                           // ускорение
	start_size = info.Size0;                    // начальный размер
	start_size_range = info.SizeRnd;       // случайная часть начального размера
	size_factor = info.SizeRange;            // изменение размера

	lifetime = info.LifeTime;         // максимальное время жизни
	// флаг, указывающий на необходимость завершения эффекта
	finished = false;

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
ExplosionSmokeEffect::~ExplosionSmokeEffect()
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
bool ExplosionSmokeEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // цвет
	float s; // размер
	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - this->start_time;
	if (dt >= this->lifetime)
	{
		finished = true;
		return true;
	}

	c = (lifetime - dt)*(255.0/lifetime);
	c = RGBA_MAKE(c, (int)(c*0.7), (int)(c*0.7), c);
	s = start_size + sqrt(dt)*size_factor;

	// пересчитаем координаты, цвет и размер
	for(int i = 0; i < particles_num; i++)
	{
		// цвет
		particles[i].color = c;
		
		// размер
		particles[i].size = s;

		// координаты 
		particles[i].coords =  particles[i].start_point + particles[i].velocity*dt + accel*dt*dt/2.0;
	}

	// новое предыдущее время
	prev_time = current_time;
	return true;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool ExplosionSmokeEffect::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = this->prev_time = start_time;

	// проинициализировать частицы
	for(int i = 0; i < particles_num; i++)	Rebirth(particles[i], start_time);

	// прописываем текстурные координаты
	for(int i = 0; i < particles_num; i++)
	{	
		texcoord_vector[i*PEAK_NUM] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, 1);
	}
	return true;
}

// функция обновляющая частицу при ее рождении
void ExplosionSmokeEffect::Rebirth(ExplosionParticle & ep, float )
{
	// для случайных скоростей
	float dx, dy, dz, a, b, r;
	// случайное место рождения
	dx = (0.5 - (float)rand()/32768.0) * 0.9;
	dy = (0.5 - (float)rand()/32768.0) * 0.9;
	dz = (0.5 - (float)rand()/32768.0) * 0.5;
	//ep.coords = root_point;// + point3(dx, dy, dz);
	ep.start_point = root_point;// + point3(dx, dy, dz);
	a = ((float)rand())/32768.0 * PIm2; // случайный угол альфа
	b = ((float)rand())/32768.0 * PId2; // случайный угол бетта

	ep.size = start_size + (float)rand()/32768.0*start_size_range;
	r = (0.8 + 0.2*(float)rand()/32768.0); // случайный радиус
	dx = r*cos(b)*cos(a) * start_velocity.x;
	dz = r*sin(b)  * start_velocity.z;
	dy = r*cos(b)*sin(a)  * start_velocity.y;
	// цвет
	ep.color = 0xffffa0a0;
	ep.velocity = point3(dx, dy, dz);
}

// возвращает bounding box эффекта
BBox ExplosionSmokeEffect::GetBBox(const float current_time)
{
	BBox b;
	float dx, dy, dz, dt, s;
	dt = current_time - start_time;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	s = start_size + sqrt(dt)*size_factor;
	b.maxx = root_point.x + dx + s;
	b.maxy = root_point.y + dy + s;
	b.maxz = root_point.z + dz + s;
	b.minx = root_point.x - dx - s;
	b.miny = root_point.y - dy - s;
	b.minz = root_point.z - dz - s;
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// класс кольца дыма для взрывов
/////////////////////////////////////////////////////////////////////////////////////////////////

bool ExplosionSmokeRing::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // цвет
	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// цвет
	c = (lifetime - dt)*(255.0/lifetime);
	color = RGBA_MAKE(c, (int)(c*0.7), (int)(c*0.7), c);
	for(int i = 0; i < points_num; i++) color_vector[i] = color;
		
	// размер
	size = start_size + sqrt(dt)*size_factor;
	
	// координаты частиц
	ready_vector[0] = root_point + point3(size, 0, 0);
	ready_vector[1] = root_point + point3(0, size, 0);
	ready_vector[2] = root_point + point3(-size, 0, 0);
	ready_vector[3] = root_point + point3(0, -size, 0);
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool ExplosionSmokeRing::Update()
{
	// звук
	sound_object->Update(root_point, start_velocity);
	return true;
}

// конструктор с параметрами
ExplosionSmokeRing::ExplosionSmokeRing(
									   const point3 rp,            // корневая точка
									   const SMOKE_RING_INFO &info
									   )
{
	start_velocity = point3(0, 0, 0);
	// создать массив частиц
	particles_num = 4;
	coords = new point3[particles_num];
	// создание массивов точек, цветов и текстурных координат
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num;

	// создать массивы
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;

	// точка в которой рождается кольцо
	root_point = rp + point3(0, 0, 0.2);

	start_size = info.Size0;                    // начальный размер
	size_factor = info.SizeRange;            // изменение размера
	lifetime = info.LifeTime;         // максимальное время жизни
	// флаг, указывающий на необходимость завершения эффекта
	finished = false;

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
ExplosionSmokeRing::~ExplosionSmokeRing()
{
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	delete[] coords;
	// звук
	delete sound_object;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool ExplosionSmokeRing::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = start_time;

	color = 0xffffffff;

	// пропишем координаты частиц
	for(int i = 0; i < particles_num; i++)
	{
		coords[i] = root_point;
	}

	// прописываем текстурные координаты
	texcoord_vector[0] = texcoord(0, 0);
	texcoord_vector[1] = texcoord(1, 0);
	texcoord_vector[2] = texcoord(1, 1);
	texcoord_vector[3] = texcoord(0, 1);
	return true;
}

// функция размножения точек
void ExplosionSmokeRing::Multiply()
{
}

// возвращает bounding box эффекта
BBox ExplosionSmokeRing::GetBBox(const float current_time)
{
	BBox b;

	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - this->start_time;
	// размер
	size = start_size + sqrt(dt)*size_factor;

	b.maxx = root_point.x + size;
	b.maxy = root_point.y + size;
	b.maxz = root_point.z + 0.01;
	b.minx = root_point.x - size;
	b.miny = root_point.y - size;
	b.minz = root_point.z - 0.01;

	return b;
}

