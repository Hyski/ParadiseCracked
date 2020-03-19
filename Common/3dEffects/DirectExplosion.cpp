/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: направленный взрыв
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "DirectExplosion.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс последовательности нескольких частиц
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// конструктор
DirectSparkleSequence::DirectSparkleSequence(const unsigned int SeqLen)
{
	seq_len = SeqLen;
	sequence = new BaseParticle[seq_len];
}

DirectSparkleSequence::~DirectSparkleSequence()
{
	delete [] sequence;
}

// следующий тик времени

void DirectSparkleSequence::NextTick(const float dt)
{
	float t;
	int c;
	
	for(int i = 0; i < seq_len; i++)
	{
		// координаты
		t = (dt - dt/seq_len*0.3*i);
		if(t<0) t = 0;
		sequence[seq_len - i - 1].coords = start_point + velocity*t + accel*t*t*0.5;
		// размер
		sequence[seq_len - i - 1].size = (lifetime-t*0.8)*size;
		// цвет
		c = (lifetime - dt)*(255.0/lifetime);
		if(i == 0) sequence[seq_len - i - 1].color = RGBA_MAKE(0xff, 0xff, 0xff, 0xff);
		else sequence[seq_len - i - 1].color = RGBA_MAKE(c, c, c, c);
	}
}


// проинициализировать последовательность
void DirectSparkleSequence::Rebirth(
		const point3 st_point,         // начальная точка
		const point3 vecVelocity,    // вектор направления скорости
		const point3 DV,                // разброс скорости
		const point3 Accel,       // ускорение
		const float stSize,	              // начальный размер
		const float stRange,            // случайная часть начального размера
		const float Lifetime             // время жизни
		)
{
	// для случайных скоростей
	float dx, dy, dz; //, a, b, r;
	point3 dv;

	// координаты главной частицы
	coords = st_point;
	// место рождения главной частицы
	start_point = st_point;
	// ускорение главной частицы
	accel = Accel;
	// время жизни
	lifetime = Lifetime;

	dv =DV;
	dx = (1 - 2.0*(float)rand()/32768.0);
	dy = (1 - 2.0*(float)rand()/32768.0);
	dz = (1 - 2.0*(float)rand()/32768.0);
	dv.x *= dx;
	dv.y *= dy;
	dv.z *= dz;
	velocity = vecVelocity + dv;
	// размер, цвет и начальные координаты частиц
	sequence[0].size = stSize + (float)rand()/32768.0*stRange;
	// первоначальный размер
	size = sequence[0].size;
	sequence[0].color = 0xffa0a0a0;
	sequence[0].coords = st_point;
	for(int i = 1; i < seq_len; i++)
	{
		sequence[i].size = sequence[0].size + 0.01;
		sequence[i].color = sequence[0].color + 0x00100000;
		sequence[i].coords = st_point;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс направленных искр
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// следующий тик времени
// (только расчет координат)
bool DirectSparkles::NextTick(const float current_time)
{
	
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;
	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	for(int i = 0; i < sequences_num; i++) sequences[i]->NextTick(dt);
	return true;
}

// деструктор
DirectSparkles::~DirectSparkles()
{
	for(unsigned int i = 0; i < sequences_num; i++) delete sequences[i];
	delete[] sequences;
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// звук
	delete sound_object;
}

// конструктор с параметрами
DirectSparkles::DirectSparkles(
							   const point3 rp,            // корневая точка
							   const point3 vecVelocity,   // вектор направления скорости
							   const DIRECT_SPARKLES_INFO &info							   )
{
	// создать массив последовательностей
	seq_len = info.PartNum;
	sequences_num = info.SeqNum;
	sequences = new PDirectSparkleSequence[sequences_num];
	for(unsigned int i = 0; i < sequences_num; i++)
	{
		sequences[i] = new DirectSparkleSequence(info.PartNum);
	}

	// создание массивов точек, цветов и текстурных координат
	points_num = sequences_num * info.PartNum;
	ready_vector_size = ready_num = points_num*PEAK_NUM;

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

	vec_velocity = vecVelocity;        // вектор направления скорости
	vec_velocity *= info.VelFactor;
	dv = info.VelDisp;                                  // разброс скорости
	accel = info.Accel;                           // ускорение
	start_size = info.SizeBegin;                    // начальный размер
	start_size_range = info.SizeRnd;       // случайная часть начального размера

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

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool DirectSparkles::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = this->prev_time = start_time;

	// проинициализировать последовательности
	for(int i = 0; i < sequences_num; i++)
		sequences[i]->Rebirth(root_point, vec_velocity, dv, accel, start_size, start_size_range, lifetime);

	// прописываем текстурные координаты
	for(int i = 0; i < sequences_num*seq_len; i++)
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

// подготовка массива готовых точек и массива цветов точек
bool DirectSparkles::Update()
{
	// звук
	sound_object->Update(root_point, vec_velocity);
	// подготовить массив для проецирования (points_vector)
	for(int i = 0; i < sequences_num; i++)
	{
		for(int j = 0; j < seq_len; j++)
		{
			points_vector[i*seq_len + j] = sequences[i]->sequence[j].coords;
		}
	}
	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(points_num,
		points_vector, points2d_vector);
	// размножаем точки
	Multiply();
	return true;
}

// функция размножения точек
void DirectSparkles::Multiply()
{
	float f, n, betta;
	float xsize, xfactor;
	float ysize, yfactor;
	float z;
	int xres, yres;
	int k, l, i, j;
	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);

	xfactor = xres/(n*f*tan(betta/2));
	yfactor = yres/(n*f*tan(betta/2));

	// сначала считаем, что все точки видны, и количество готовых вершин 
	// равно размеру массива, в котором они лежат
	ready_num = ready_vector_size;
	// текущее количество точек, которые размножены в готовые вершины
	k = 0;
	// текущий номер точки в массиве после проецирования
	l = 0; 
	for(i = 0; i < sequences_num; i++) // по последовательностям
	{
		for(j = 0; j < seq_len; j++) // по точкам в последовательности
		{
			// размножение частиц
			z  = points2d_vector[l].z;
			if(z<0)
			{
				// эту точку не нужно размножать - пропустим ее
				// уменьшим количество готовых вершин
				ready_num -= PEAK_NUM;
				// увеличим номер в массиве точек после проецирования
				l++;
				continue;
			}
			// размножим координаты
			ready_vector[k*PEAK_NUM].z = z;
			ready_vector[k*PEAK_NUM + 1].z = z;
			ready_vector[k*PEAK_NUM + 2].z = z;
			ready_vector[k*PEAK_NUM + 3].z = z;
			ready_vector[k*PEAK_NUM + 4].z = z;
			ready_vector[k*PEAK_NUM + 5].z = z;

			xsize = sequences[i]->sequence[j].size*(z*(n-f)+f)*xfactor;
			ysize = sequences[i]->sequence[j].size*(z*(n-f)+f)*yfactor;

		
			ready_vector[k*PEAK_NUM].x = points2d_vector[l].x - xsize;
			ready_vector[k*PEAK_NUM + 1].x = points2d_vector[l].x + xsize;
			ready_vector[k*PEAK_NUM + 2].x = points2d_vector[l].x + xsize;
			ready_vector[k*PEAK_NUM + 3].x = points2d_vector[l].x - xsize;
			ready_vector[k*PEAK_NUM + 4].x = points2d_vector[l].x + xsize;
			ready_vector[k*PEAK_NUM + 5].x = points2d_vector[l].x - xsize;
		
			ready_vector[k*PEAK_NUM].y = points2d_vector[l].y + ysize;
			ready_vector[k*PEAK_NUM + 1].y = points2d_vector[l].y + ysize;
			ready_vector[k*PEAK_NUM + 2].y = points2d_vector[l].y - ysize;
			ready_vector[k*PEAK_NUM + 3].y = points2d_vector[l].y + ysize;
			ready_vector[k*PEAK_NUM + 4].y = points2d_vector[l].y - ysize;
			ready_vector[k*PEAK_NUM + 5].y = points2d_vector[l].y - ysize;
		
			// размножим цвет
			color_vector[k*PEAK_NUM] = sequences[i]->sequence[j].color;
			color_vector[k*PEAK_NUM + 1] = sequences[i]->sequence[j].color;
			color_vector[k*PEAK_NUM + 2] = sequences[i]->sequence[j].color;
			color_vector[k*PEAK_NUM + 3] = sequences[i]->sequence[j].color;
			color_vector[k*PEAK_NUM + 4] = sequences[i]->sequence[j].color;
			color_vector[k*PEAK_NUM + 5] = sequences[i]->sequence[j].color;
		
			// увеличим количество размноженных точек
			k++;
			// увеличим номер в массиве точек после проецирования
			l++;
		}
	}
}

// возвращает bounding box эффекта
BBox DirectSparkles::GetBBox(const float current_time)
{
	BBox b;
	float dx, dy, dz, dt;
	b.Degenerate();
	dt = current_time - start_time;
	dx = (vec_velocity.x + 1.0)*dt + accel.x*dt*dt*0.5;
	dy = (vec_velocity.y + 1.0)*dt + accel.y*dt*dt*0.5;
	dz = (vec_velocity.z + 1.0)*dt + accel.z*dt*dt*0.5;
	b.Enlarge(root_point);
	b.Enlarge(root_point + point3(dx, dy, dz));
	b.Enlarge(root_point - point3(dx, dy, dz));
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс направленного дыма
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// следующий тик времени
// (только расчет координат)
bool DirectExplosion::NextTick(const float )
{
	return true;
}

// возвращает bounding box эффекта
BBox DirectExplosion::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	return b;
}

// подготовка массива готовых точек и массива цветов точек
bool DirectExplosion::Update()
{
	return true;
}

// деструктор
DirectExplosion::~DirectExplosion()
{
}

// конструктор с параметрами
DirectExplosion::DirectExplosion(
								 const point3 ,            // корневая точка
								 const point3 ,   // вектор направления скорости
								 const DIRECT_EXPLOSION_INFO& 
								 )
{
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool DirectExplosion::Start(const float )
{
	return true;
}

// функция размножения точек
void DirectExplosion::Multiply()
{
}

// функция обновляющая частицу при ее рождении
void DirectExplosion::Rebirth(DirectParticle & , float )
{
}
