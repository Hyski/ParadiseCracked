/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: класс хвоста из частиц
				класс движения модельки по параболе
				
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include "../../Skin/animalibrary.h"

#include "EffectInfo.h"
#include "TailEffect.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// класс хвоста из частиц
//
/////////////////////////////////////////////////////////////////////////////////////////////////

// конструктор с параметрами
TailEffect::TailEffect(
					   const point3 StartPoint,     // начальная точка
					   const point3 EndPoint,	    // конечная точка
					   const TAIL_EFFECT_INFO& info
					   )
{
	start_point = StartPoint;                       // начальная точка
	end_point = EndPoint;                           // конечная точка
	start_size = info.Size*0.5;
	end_size = info.SizeEnd*0.5;
	lifetime = (end_point - start_point).Length() / info.VelFactor;
	velocity = (end_point - start_point)/lifetime;  // скорость
	direction = Normalize(end_point - start_point);
	particles_lifetime = info.LifeTime;
	frequency = info.Frequency;


	// создать массивы
	points_num = ceil(frequency * lifetime);
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
TailEffect::~TailEffect()
{
	delete [] points_vector;
	delete [] size_vector;
	delete [] points2d_vector;
	delete [] color_vector;
	delete [] texcoord_vector;
	delete [] ready_vector;
	// звук
	delete sound_object;
}

// следующий тик времени
// (только расчет координат)
bool TailEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	float dt; // разница во времени между текущим временем и временем рождения эффекта
	float dtt; // время, которое прожила частица с момента своего рождения
	unsigned int c;
	dt = current_time - start_time;

	// координаты главной частицы
	current_point = start_point + velocity*dt;

	if (dt < lifetime)
	{
		// посмотрим, сколько времени прошло с предыдущего рождения и, если надо, создадим новые частицы
		int n = floor(frequency * (current_time - previous_birth_time));
		if(n > 0)
		{
			// надо создать одну или более частиц
			float delta_t = (current_time - previous_birth_time) / n;
			for(int i = 1; i <= n; i++)
			{
				TailParticle tp;
				tp.birthtime = previous_birth_time + i*delta_t;
				tp.start_point = current_point - velocity*(n - i)*delta_t;
				tp.coords = tp.start_point;
				tp.lifetime = particles_lifetime;
				tp.velocity = point3(0, 0, 0);
				tp.texcoord_type = (float)rand()/32768.0 * 4;
				
				particles.push_back(tp);
			}

			// запомним новое время рождения
			previous_birth_time = current_time;
		}
	}

	// удалим умершие частицы и просчитаем все для живых
	TailParticlesListIterator I, J, E; // основной, дополнительный и конечный итераторы
	I = particles.begin();
	E = particles.end();
	while(I != E)
	{
		dtt = (current_time - (*I).birthtime);
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
		c = 255*(1 - dtt/(*I).lifetime);
		(*I).color = RGBA_MAKE(c, c, c, c);
		I++;
	}

	// если список пуст или время вышло - завершим эффект
	if(particles.empty() && (dt > lifetime))
	{
		finished = true;
		return true;
	}
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool TailEffect::Update()
{
	// звук
	sound_object->Update(current_point, velocity);

	float x1 = 0, x2 = 1, y1 = 0, y2 = 1;
	particles_num = 0;
	TailParticlesListIterator I, E; // основной и конечный итераторы
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

	return true;
}

// возвращает bounding box эффекта
BBox TailEffect::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(start_point - point3(end_size, end_size, end_size));
	b.Enlarge(end_point + point3(end_size, end_size, end_size));
	return b;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool TailEffect::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = start_time;

	// цвет
	color = 0xffffffff;

	// координаты текущей точки
	current_point = start_point;
	previous_birth_time = start_time;

	return true;
}



// функция размножения точек
void TailEffect::Multiply()
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


//
// класс хвоста из частиц
//

// подготовка массива готовых точек и массива цветов точек
bool TailCircleEffect::Update()
{
	// звук
	sound_object->Update(current_point, velocity);

	particles_num = 0;
	TailParticlesListIterator I, E; // основной и конечный итераторы
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

		particles_num++;
	}
	// размножаем точки
	Multiply();

	return true;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool TailCircleEffect::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = start_time;

	// цвет
	color = 0xffffffff;

	// координаты текущей точки
	current_point = start_point;
	previous_birth_time = start_time;

	// пропишем текстурные координаты
	for(int i = 0; i < points_num; i++)
	{
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
void TailCircleEffect::Multiply()
{
	point3 p = direction.Cross(point3(1.0, 0.0, 0.0));
	p = Normalize(p);
	point3 up;
	Quaternion qtpi4, qtpi2, qtpi;
	qtpi4.FromAngleAxis(PId4, direction);
	qtpi2.FromAngleAxis(PId2, direction);
	qtpi.FromAngleAxis(PI, direction);
	ready_num = 0;
	for(int i = 0; i < particles_num; i++)
	{
		up = p*size_vector[i];
		// размножим координаты
		ready_vector[i*PEAK_NUM] = points_vector[i] + qtpi4*(qtpi*up);
		ready_vector[i*PEAK_NUM + 1] = points_vector[i] + qtpi4*(qtpi2*up);
		ready_vector[i*PEAK_NUM + 2] = points_vector[i] + qtpi4*up;
		ready_vector[i*PEAK_NUM + 3] = points_vector[i] + qtpi4*(qtpi*up);
		ready_vector[i*PEAK_NUM + 4] = points_vector[i] + qtpi4*up;
		ready_vector[i*PEAK_NUM + 5] = points_vector[i] + qtpi4*(qtpi2*(qtpi*up));

		// увеличим количество конечных точек
		ready_num += PEAK_NUM;
	}
}


//
// класс хвоста из частиц, летящего по параболе
//

// конструктор с параметрами
TailParabolaEffect::TailParabolaEffect(
									   const point3 StartPoint,                          // начальная точка
									   const point3 EndPoint,	                         // конечная точка
									   const TAIL_EFFECT_INFO& info,
									   const float Gravitation
									   ) : TailEffect(StartPoint, EndPoint, info)
{ 
	gravitation = Gravitation;
	vz = (EndPoint.z - StartPoint.z)/lifetime - gravitation*lifetime*0.5;
}

// следующий тик времени
// (только расчет координат)
bool TailParabolaEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	float dt; // разница во времени между текущим временем и временем рождения эффекта
	float dtt; // время, которое прожила частица с момента своего рождения
	unsigned int c;
	dt = current_time - start_time;

	// координаты главной частицы
	current_point = start_point + velocity*dt;
	current_point.z = start_point.z + vz*dt + gravitation*dt*dt*0.5;

	if (dt < lifetime)
	{
		// посмотрим, сколько времени прошло с предыдущего рождения и, если надо, создадим новые частицы
		int n = floor(frequency * (current_time - previous_birth_time));
		if(n > 0)
		{
			// надо создать одну или более частиц
		float delta_t = (current_time - previous_birth_time) / n;
			for(int i = 1; i <= n; i++)
			{
				TailParticle tp;
				tp.birthtime = previous_birth_time + i*delta_t;
				float dttt = tp.birthtime - start_time;

				tp.start_point = start_point + velocity*dttt;
				tp.start_point.z = start_point.z + vz*dttt + gravitation*dttt*dttt*0.5;
				tp.coords = tp.start_point;
				tp.lifetime = particles_lifetime;
				tp.velocity = point3(0, 0, 0);
				tp.texcoord_type = (float)rand()/32768.0 * 4;
				
				particles.push_back(tp);
			}

			// запомним новое время рождения
			previous_birth_time = current_time;
		}
	}

	// удалим умершие частицы и просчитаем все для живых
	TailParticlesListIterator I, J, E; // основной, дополнительный и конечный итераторы
	I = particles.begin();
	E = particles.end();
	while(I != E)
	{
		dtt = (current_time - (*I).birthtime);
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
		c = 255*(1 - dtt/(*I).lifetime);
		(*I).color = RGBA_MAKE(c, c, c, c);
		I++;
	}

	// если список пуст или время вышло - завершим эффект
	if(particles.empty() && (dt > lifetime))
	{
		finished = true;
		return true;
	}
	return true;
}

// возвращает bounding box эффекта
BBox TailParabolaEffect::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(start_point - point3(end_size, end_size, end_size));
	b.Enlarge(end_point + point3(end_size, end_size, end_size));
	point3 tmp = start_point + velocity*lifetime*0.5;
	tmp.z = start_point.z + vz*0.5*lifetime + gravitation*0.5*0.5*0.5*lifetime*lifetime;
	return b;
}

//
// класс движения модельки по параболе
//

// конструктор с параметрами
TracerParabolaModel::TracerParabolaModel(
										 const point3 StartPoint,  // начальная точка
										 const point3 EndPoint,	   // конечная точка
										 const TRACER_PARABOLA_MODEL_INFO& info
										 )
{
	// создание массивов точек, цветов и текстурных координат
	points_num = 0;
	ready_vector_size = ready_num = 0;

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;

	// 
	start_point = StartPoint;                       // начальная точка
	end_point = EndPoint;                           // конечная точка

	// время жизни эффекта
	lifetime = (EndPoint - StartPoint).Length()/info.VelFactor;

	direction = Normalize(EndPoint - StartPoint);
	vel = direction * info.VelFactor;  // скорость
	gravitation = info.Gravitation;
	vz = (EndPoint.z - StartPoint.z)/lifetime - gravitation*lifetime*0.5;
	bbox.Degenerate();
	bbox.Enlarge(start_point);
	bbox.Enlarge(end_point);
	
	// флаг, указывающий на необходимость завершения эффекта
	finished = false;

	// звука нет
	sound_object = new EmptySound();

	// загрузим модельку
	AnimaLibrary *lib=AnimaLibrary::GetInst();
	m_skeleton = lib->GetSkAnimation(std::string("Animations/anims/items/") + info.Shader);
	m_animadata = m_skeleton->Start(0);

	// загрузим скин
	m_skin = lib->GetSkSkin(std::string("Animations/skins/items/") + info.Sound);
	if(!m_skin)
	{
		// выкинем исключение
		std::string str = "illegal skin: \"";
		str += std::string("Animations/skins/items/") + info.Sound + "\"";
		delete sound_object;
		throw CASUS(str);
	}
	// прикрепим его к скелету
	m_skin->ValidateLinks(m_skeleton);


	m_mat._11 = 1;
	m_mat._12 = 0;
	m_mat._13 = 0;
	m_mat._14 = 0;

	m_mat._21 = 0;
	m_mat._22 = 1;
	m_mat._23 = 0;
	m_mat._24 = 0;

	m_mat._31 = 0;
	m_mat._32 = 0;
	m_mat._33 = 1;
	m_mat._34 = 0;

	m_mat._41 = 0;
	m_mat._42 = 0;
	m_mat._43 = 0;
	m_mat._44 = 1;

	m_animadata.LastState.m_SkelState[0].World = m_mat;
	m_skin->Update(&m_animadata.LastState);
}

// деструктор
TracerParabolaModel::~TracerParabolaModel()
{
	// звука
	delete sound_object;
	// моделька
	delete m_skin;
}

// следующий тик времени
// (только расчет координат)
bool TracerParabolaModel::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}
	current_point = start_point + vel*dt;
	current_point.z = start_point.z + vz*dt + gravitation*dt*dt*0.5;
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool TracerParabolaModel::Update()
{
	// звук
	sound_object->Update(start_point, vel);

	// моделька
	m_mat._41 = current_point.x;
	m_mat._42 = current_point.y;
	m_mat._43 = current_point.z;

	m_animadata.LastState.m_SkelState[0].World = m_mat;
	m_skin->Update(&m_animadata.LastState);
	pGraphPipe->Chop(m_skin->GetMesh());
	return true;
}

// возвращает bounding box эффекта
BBox TracerParabolaModel::GetBBox(const float )
{
	return bbox;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool TracerParabolaModel::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = start_time;
	return true;
}
