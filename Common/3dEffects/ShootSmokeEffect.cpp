/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: класс дыма от выстрела
				
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include "EffectInfo.h"
#include "ShootSmokeEffect.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// класс дыма от выстрела
//
/////////////////////////////////////////////////////////////////////////////////////////////////

// конструктор с параметрами
ShootSmokeEffect::ShootSmokeEffect(
								   const point3 StartPoint,   // начальная точка
								   const point3 EndPoint,	  // конечная точка
								   const SHOOT_SMOKE_EFFECT_INFO& info
								   )
{
	points_num = info.PartNum;

	// создать массивы
	ready_vector_size = points_num*PEAK_NUM;
	ready_num = ready_vector_size;
	points_vector = new point3 [points_num];
	points2d_vector = new point3 [points_num];
	color_vector = new unsigned int [ready_vector_size];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];
	
	particles = new ShootSmokeParticle[points_num];

	root_point = StartPoint;                       // начальная точка
	direction = Normalize(EndPoint - StartPoint);

	/*
	accel.x = info.Accel.x * direction.x;
	accel.y = info.Accel.y * direction.y;
	*/
	point3 tmp;
	tmp.x = direction.x;
	tmp.y = direction.y;
	tmp.z = 0;
	tmp = Normalize(tmp);
	accel = info.Accel.x*tmp;
	accel.z = info.Accel.z;

	for(int i = 0; i < points_num; i++)
	{
		/*
		particles[i].velocity.x = (info.SlowVel.x + (info.FastVel.x - info.SlowVel.x)*(float)rand()/32768.0)*direction.x;
		particles[i].velocity.y = (info.SlowVel.y + (info.FastVel.y - info.SlowVel.y)*(float)rand()/32768.0)*direction.y;
		*/
		particles[i].velocity = (info.SlowVel.x + (info.FastVel.x - info.SlowVel.x)*(float)rand()/32768.0)*tmp;

		particles[i].velocity.z = (info.SlowVel.z + (info.FastVel.z - info.SlowVel.z)*(float)rand()/32768.0);

		particles[i].size_vel = info.SlowSizeVel + (info.FastSizeVel - info.SlowSizeVel)*(float)rand()/32768.0;
	}
	
	lifetime = info.LifeTime;
	start_size = info.StartSize;

	tmp = root_point + info.FastVel.x*direction*lifetime + 0.5*accel;
	bbox.Degenerate();
	bbox.Enlarge(root_point);
	bbox.Enlarge(tmp);

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;
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
ShootSmokeEffect::~ShootSmokeEffect()
{
	delete [] points_vector;
	delete [] points2d_vector;
	delete [] color_vector;
	delete [] texcoord_vector;
	delete [] ready_vector;
	delete [] particles;
	// звук
	delete sound_object;
}

// следующий тик времени
// (только расчет координат)
bool ShootSmokeEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;

	// если время вышло - завершим эффект
	if(dt > lifetime)
	{
		finished = true;
		return true;
	}

	for(int i = 0; i < points_num; i++)
	{
		// координаты
		particles[i].coords = root_point + dt * particles[i].velocity + 0.5*dt*dt*accel;
		// цвет
		int c = 255*(1 - dt/lifetime);
		color = RGBA_MAKE(c, c, c, c);
		// размер
		particles[i].size = start_size + dt*particles[i].size_vel;
	}
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool ShootSmokeEffect::Update()
{
	// звук
	sound_object->Update(root_point, velocity);

	for(int i = 0; i < points_num; i++)
		points_vector[i] = particles[i].coords;

	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(points_num, points_vector, points2d_vector);
	// размножаем точки
	Multiply();

	return true;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool ShootSmokeEffect::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = start_time;

	// цвет
	color = 0xffffffff;

	for(int i = 0; i < points_num; i++)
	{
		// текстурные координаты
		texcoord_vector[i*PEAK_NUM] = texcoord(0, 1);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(0, 1);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, 0);
	}
	return true;
}

// функция размножения точек
void ShootSmokeEffect::Multiply()
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

	for(int i = 0; i < points_num; i++)
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

		xsize = particles[i].size*(z*(n-f)+f)*xfactor;
		ysize = particles[i].size*(z*(n-f)+f)*yfactor;

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

		color_vector[i*PEAK_NUM] = color;
		color_vector[i*PEAK_NUM + 1] = color;
		color_vector[i*PEAK_NUM + 2] = color;
		color_vector[i*PEAK_NUM + 3] = color;
		color_vector[i*PEAK_NUM + 4] = color;
		color_vector[i*PEAK_NUM + 5] = color;

	}
}
