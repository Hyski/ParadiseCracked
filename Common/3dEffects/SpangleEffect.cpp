/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: класс искр, остающихся от трассера
				класс клубков линий
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include "EffectInfo.h"
#include "SpangleEffect.h"


// следующий тик времени
// (только расчет координат)
bool SpangleEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// координаты главной частицы
	current_point = start_point + velocity*dt;

	// координаты частиц
	for(int i = 0; i < particles_num; i++)
	{
		dt = current_time - particles[i].birthtime;
		if(dt < particles[i].lifetime)
		{
			// изменить координаты
			particles[i].coords = particles[i].start_point + particles[i].velocity*dt;
		}
		else
		{
			// заново родить частицу
			particles[i].start_point = current_point - (current_point - particles[i].start_point)*(float)rand()/32768.0;
			particles[i].coords = particles[i].start_point;
			particles[i].birthtime = current_time;
		}
	}
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool SpangleEffect::Update()
{
	// звук
	sound_object->Update(current_point, velocity);
	// подготовить массив для проецирования (points_vector)
	for(int i = 0; i < particles_num; i++)
	{
		points_vector[i] = particles[i].coords;
	}
	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(particles_num, points_vector, points2d_vector);
	// размножаем точки
	Multiply();
	return true;
}

// возвращает bounding box эффекта
BBox SpangleEffect::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	for(int i = 0; i < particles_num; i++)
	{
		b.Enlarge(particles[i].coords);
	}
	return b;
}

// деструктор
SpangleEffect::~SpangleEffect()
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

// конструктор с параметрами
SpangleEffect::SpangleEffect(const float quality,        // качество
							 const point3 StartPoint,    // начальная точка
							 const point3 EndPoint,	     // конечная точка
							 const SPANGLE_EFFECT_INFO& info
							 )
{
	// создание массивов точек, цветов и текстурных координат
	particles_num = 5 + quality*30;
	ready_vector_size = ready_num = particles_num*PEAK_NUM;
	// создать массивы
	particles = new SpangleParticle [particles_num];
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [particles_num];
	points_vector = new point3 [particles_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;

	start_point = StartPoint;                       // начальная точка
	end_point = EndPoint;                           // конечная точка

	// время жизни эффекта
	lifetime = (end_point - start_point).Length() / info.VelFactor;
	particles_lifetime = info.LifeTime;

	size = info.Size;                                    // размер

	velocity = (end_point - start_point)/lifetime;  // скорость

	direction = (end_point - start_point)/(end_point - start_point).Length();
	
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
			sound_object = new FixedSound(info.Sound, StartPoint);
		}
	}
}

	

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool SpangleEffect::Start(const float start_time)
{
	point3 tmp;
	// запомнить время начала проработки эффекта
	this->start_time = start_time;
	
	// цвет
	color = 0xffffffff;

	// прописываем текстурные координаты
	for(int i = 0; i < particles_num; i++)
	{
		texcoord_vector[i*PEAK_NUM] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, 1);

		// цвет
		color_vector[i*PEAK_NUM] = color;
		color_vector[i*PEAK_NUM + 1] = color;
		color_vector[i*PEAK_NUM + 2] = color;
		color_vector[i*PEAK_NUM + 3] = color;
		color_vector[i*PEAK_NUM + 4] = color;
		color_vector[i*PEAK_NUM + 5] = color;

		particles[i].coords = start_point;
		particles[i].start_point = start_point;
		particles[i].size = size*(0.4 + 0.6*(float)rand()/32768.0);
		particles[i].birthtime = start_time;
		particles[i].lifetime = particles_lifetime*(0.4 + 0.6*(float)rand()/32768.0);
		// создадим случайный вектор
		tmp = point3(1.0 - 2.0*(float)rand()/32768.0,
			1.0 - 2.0*(float)rand()/32768.0,
			1.0 - 2.0*(float)rand()/32768.0)*2.0;
		particles[i].velocity = tmp.Cross(direction) + velocity*0.8;
	}

		
	// координаты частиц
	current_point = start_point;
	return true;
}

// функция размножения точек
void SpangleEffect::Multiply()
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
	for(int i = 0; i < particles_num; i++)
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

		// увеличим количество размноженных точек
		k++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// класс клубка линий
//
/////////////////////////////////////////////////////////////////////////////////////////////////

// конструктор с параметрами
LineBallEffect::LineBallEffect(const float quality,        // качество
							   const point3 StartPoint,    // начальная точка
							   const point3 EndPoint,	   // конечная точка
							   const LINE_BALL_EFFECT_INFO& info
							   )
{
	// создание массивов точек, цветов и текстурных координат
	particles_num = ready_num = ready_vector_size =  4 + 40*quality;
	// создать массивы
	particles = new SpangleParticle [particles_num];
	color_vector = new unsigned int [particles_num];
	texcoord_vector = new texcoord [particles_num];
	ready_vector = new point3 [particles_num];

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;

	start_point = StartPoint;                       // начальная точка
	end_point = EndPoint;                           // конечная точка

	// время жизни эффекта
	lifetime = (end_point - start_point).Length() / info.VelFactor;
	particles_lifetime = info.PartLifeTime;

	length = info.Length;                                // длина
	width = info.Width;                                  // ширина

	velocity = (end_point - start_point)/lifetime;  // скорость

	direction = Normalize(end_point - start_point);
	
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
			sound_object = new FixedSound(info.Sound, StartPoint);
		}
	}
}


// деструктор
LineBallEffect::~LineBallEffect()
{
	delete[] particles;
	delete[] color_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// звук
	delete sound_object;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool LineBallEffect::Start(const float start_time)
{
	point3 tmp;
	// запомнить время начала проработки эффекта
	this->start_time = start_time;
	
	// цвет
	color = 0xffffffff;

	// прописываем текстурные координаты
	for(int i = 0; i < particles_num; i++)
	{
		texcoord_vector[0] = texcoord(0, 0);
		texcoord_vector[1] = texcoord(1, 1);
		// цвет
		color_vector[i] = color;

		particles[i].coords = start_point;
		particles[i].start_point = start_point;
		particles[i].birthtime = start_time;
		particles[i].lifetime = particles_lifetime*(0.4 + 0.6*(float)rand()/32768.0);
		// создадим случайный вектор
		tmp = point3(1.0 - 2.0*(float)rand()/32768.0,
			1.0 - 2.0*(float)rand()/32768.0,
			1.0 - 2.0*(float)rand()/32768.0)*width;
		particles[i].velocity = tmp.Cross(direction) + velocity*0.8;
	}

		
	// координаты частиц
	first_point = start_point;
	second_point = start_point;
	return true;
}

// следующий тик времени
// (только расчет координат)
bool LineBallEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// координаты ограничивающих частиц
	first_point = start_point + velocity*dt;
	if((first_point - start_point).Length() < length) second_point = start_point;
	else second_point = first_point - length*direction;
	if((end_point - second_point).Length() < length) first_point = end_point;

	// координаты частиц
	for(int i = 0; i < particles_num; i++)
	{
		float dtt = current_time - particles[i].birthtime;
		if(dtt < particles[i].lifetime)
		{
			// изменение координат
			particles[i].coords = particles[i].start_point + particles[i].velocity*dtt;
		}
		else
		{
			// типа родить заново
			particles[i].birthtime = current_time;
			particles[i].coords = second_point + length*((float)rand()/32768.0)*direction;
			particles[i].start_point = particles[i].coords;
		}
	}
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool LineBallEffect::Update()
{
	// звук
	sound_object->Update(first_point, velocity);
	// подготовить массив
	for(int i = 0; i < particles_num; i++)
	{
		ready_vector[i] = particles[i].coords;
	}
	return true;
}

// возвращает bounding box эффекта
BBox LineBallEffect::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	for(int i = 0; i < particles_num; i++)
	{
		b.Enlarge(particles[i].coords);
	}
	return b;
}

