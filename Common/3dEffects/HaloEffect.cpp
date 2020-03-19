/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Halo
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "HaloEffect.h"
#include "../../logic2/TraceUtils.h"

//
// базовый класс для halo (абстрактный)
//

// конструктор с параметрами
BaseHalo::BaseHalo(
				   const point3 rp,			     // начальная точка
				   const std::string &Shader,    // шейдер эффекта
				   const std::string &Sound,     // описатель звука
				   const bool SoundMove          // двигается ли звук
				   )
{
//	LOG("BaseHalo::BaseHalo begin\n");
	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;
	root_point = rp;

	vel = point3(0, 0, 0);  // скорость

	bbox.Degenerate();
	bbox.Enlarge(root_point - point3(0.00001, 0.00001, 0.00001));
	bbox.Enlarge(root_point + point3(0.00001, 0.00001, 0.00001));
	
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
//	LOG("BaseHalo::BaseHalo end\n");
}

// деструктор
BaseHalo::~BaseHalo()
{
	// звук
	delete sound_object;
}

// подготовка массива готовых точек и массива цветов точек
bool BaseHalo::Update()
{
//	LOG("BaseHalo::Update begin\n");
	// звук
	sound_object->Update(root_point, vel);
	// подготовить массив для проецирования (points_vector)
	points_vector[0] = root_point;
	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(1, points_vector, points2d_vector);
	// размножаем точки
	Multiply();
//	LOG("BaseHalo::Update end\n");
	return true;
}

// функция размножения точек
void BaseHalo::Multiply()
{
//	LOG("BaseHalo::Multiply begin\n");
	if (state.state == HaloState::hidden)
	{
		ready_num = 0;
		return;
	}
	else
	{
		ready_num = PEAK_NUM;
	}

	float z = points2d_vector[0].z;
	if(z == -1)
	{
		ready_num = 0;
		return;
	}

	float f, n, betta;
	float xsize, xfactor;
	float ysize, yfactor;
	int xres, yres;
	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);

	xfactor = xres/(n*f*tan(betta/2));
	yfactor = yres/(n*f*tan(betta/2));

	// размножение координат частиц

	// размножим координаты

	xsize = size*(z*(n-f)+f)*xfactor;
	ysize = size*(z*(n-f)+f)*yfactor;

	ready_vector[0].x = points2d_vector[0].x - xsize;
	ready_vector[1].x = points2d_vector[0].x + xsize;
	ready_vector[2].x = points2d_vector[0].x + xsize;
	ready_vector[3].x = points2d_vector[0].x - xsize;
	ready_vector[4].x = points2d_vector[0].x + xsize;
	ready_vector[5].x = points2d_vector[0].x - xsize;

	ready_vector[0].y = points2d_vector[0].y + ysize;
	ready_vector[1].y = points2d_vector[0].y + ysize;
	ready_vector[2].y = points2d_vector[0].y - ysize;
	ready_vector[3].y = points2d_vector[0].y + ysize;
	ready_vector[4].y = points2d_vector[0].y - ysize;
	ready_vector[5].y = points2d_vector[0].y - ysize;

	color_vector[0] = color;
	color_vector[1] = color;
	color_vector[2] = color;
	color_vector[3] = color;
	color_vector[4] = color;
	color_vector[5] = color;
//	LOG("BaseHalo::Multiply end\n");
}

// функция расчета видимости
void BaseHalo::UpdateVisibility(const float current_time)
{
	point3 camera = pGraphPipe->GetCam()->GetPos();
	bool camera_changed = pGraphPipe->GetCam()->IsViewChanged();

	///////////////////////////////////////////////////
	/////////////  HIDDEN or DISSAPPEARING
	///////////////////////////////////////////////////
	if( (state.state == HaloState::hidden) ||
		(state.state == HaloState::disappearing)
		)
	{
		if(state.state == HaloState::disappearing)
		{
			//DebugInfo::Add(100, 100, "halo: disappearing");
			// halo исчезло и теряет яркость 
			if((current_time - state.prev_time) > state.disappearing_time)
			{
				// нулевая яркость => переключаем в невидимое состояние
				state.prev_time = current_time;
				state.state = HaloState::hidden;
				return;
			}
			
			unsigned int r, g, b;
			b = RGB_GETBLUE(save_color);
			g = RGB_GETGREEN(save_color);
			r = RGB_GETRED(save_color);
			
			//DebugInfo::Add(100, 190, "r = %d, g = %d, b = %d", r, g, b);
			
			r = r * (1 - (current_time - state.prev_time)/state.disappearing_time);
			g = g * (1 - (current_time - state.prev_time)/state.disappearing_time);
			b = b * (1 - (current_time - state.prev_time)/state.disappearing_time);
			
			//DebugInfo::Add(100, 210, "r = %d, g = %d, b = %d", r, g, b);
			
			color = RGB_MAKE(r, g, b);
		}
		/*
		else DebugInfo::Add(100, 100, "halo: hidden");

		if(state.bbox_visible)
			DebugInfo::Add(100, 130, "halo bbox: visible");
		else
			DebugInfo::Add(100, 130, "halo bbox: not visible");
		if(state.trace_ray_visible)
			DebugInfo::Add(100, 160, "halo traceray: visible");
		else
			DebugInfo::Add(100, 160, "halo traceray: not visible");*/

		// halo не было видно

		if((!camera_changed) && (!state.camera_was_changed))
		{
			// камера не двигалась и не поворачивалась
			return;
		}
		
		// здесь камера либо сдвинулась, либо повернулась

		// проверим, находится ли BBox halo в области видимости
		Frustum * fru = &pGraphPipe->GetCam()->Cone;
		if(fru->TestPoint(root_point) == Frustum::NOTVISIBLE)
		{
			// BBox не виден
			if(state.bbox_visible)
			{
				// BBox был виден, а теперь вышел из зоны видимости
				state.bbox_visible = false;
				// специально для disappearing
				if(state.state == HaloState::disappearing)
				{
					// сразу переключаем в невидимое состояние
					state.prev_time = current_time;
					state.state = HaloState::hidden;
				}
			}
			return;
		}

		// BBox виден
		if(state.bbox_visible)
		{
			// BBox был виден и остался видимым
			if((current_time - state.prev_trace_time) > 0.2)
			{
				state.prev_trace_time = current_time;
				state.camera_was_changed = false;
				// оттрассируем луч
				ShotTracer tr(0, camera,
					(root_point - camera),
					(root_point - camera).Length(),
					ShotTracer::F_VISROOFS|ShotTracer::F_SKIP_SHIELDS|ShotTracer::F_SKIP_INVISIBLE_ENTS|ShotTracer::F_SIGHT);
				
				if(tr.GetMaterial() == ShotTracer::MT_AIR)
				{
					// луч попал в воздух => halo стал виден => переводим в режим зажигания
					state.trace_ray_visible = true;
					state.state = HaloState::appearing;
					state.prev_time = current_time;
					state.prev_trace_time = current_time;
					color = 0;
				}
			}
			else
			{
				// еще не время проверять trace ray, но камера двигалась
				state.camera_was_changed = true;
			}
			return;
		}

		// BBox не был виден, но стал видимым
		state.bbox_visible = true;
		// оттрассируем луч
		ShotTracer tr(0, camera,
			(root_point - camera), (root_point - camera).Length(),
			ShotTracer::F_VISROOFS|ShotTracer::F_SKIP_SHIELDS|ShotTracer::F_SKIP_INVISIBLE_ENTS|ShotTracer::F_SIGHT);
		if(tr.GetMaterial() == ShotTracer::MT_AIR)
		{
			// луч попал в воздух => halo стал виден => переводим в режим зажигания
			state.trace_ray_visible = true;
			state.prev_trace_time = current_time;
			state.state = HaloState::appearing;
			state.prev_time = current_time;
			color = 0;
		}
		else
		{
			// луч попал не в воздух
			state.prev_trace_time = current_time;
			state.trace_ray_visible = false;
		}
		return;
	}

	///////////////////////////////////////////////////
	/////////////  VISIBLE or APPEARING
	///////////////////////////////////////////////////
	if( (state.state == HaloState::visible) ||
		(state.state == HaloState::appearing)
		)
	{
		if(state.state == HaloState::appearing)
		{
			//DebugInfo::Add(100, 100, "halo: appearing");

			// halo появилось и наращивает яркость
			if((current_time - state.prev_time) > state.appearing_time)
			{
				// полная яркость
				state.prev_time = current_time;
				state.state = HaloState::visible;
				color = save_color;
				return;
			}
			
			unsigned int r, g, b;
			b = RGB_GETBLUE(save_color);
			g = RGB_GETGREEN(save_color);
			r = RGB_GETRED(save_color);
			
			//DebugInfo::Add(100, 190, "r = %d, g = %d, b = %d", r, g, b);
			
			r = r * (current_time - state.prev_time)/state.appearing_time;
			g = g * (current_time - state.prev_time)/state.appearing_time;
			b = b * (current_time - state.prev_time)/state.appearing_time;
			
			//DebugInfo::Add(100, 210, "r = %d, g = %d, b = %d", r, g, b);
			
			color = RGB_MAKE(r, g, b);
		}
		else 
		{
			unsigned int r, g, b;
			b = RGB_GETBLUE(save_color);
			g = RGB_GETGREEN(save_color);
			r = RGB_GETRED(save_color);
			color = RGB_MAKE(r, g, b);
		}
		
		/*
		if(state.bbox_visible)
			DebugInfo::Add(100, 130, "halo bbox: visible");
		else
			DebugInfo::Add(100, 130, "halo bbox: not visible");
		if(state.trace_ray_visible)
			DebugInfo::Add(100, 160, "halo traceray: visible");
		else
			DebugInfo::Add(100, 160, "halo traceray: not visible");*/

		// halo было видно
		if((!camera_changed) && (!state.camera_was_changed))
		{
			// камера не двигалась и не поворачивалась
			return;
		}
		
		// проверим, находится ли BBox halo в области видимости
		Frustum * fru = &pGraphPipe->GetCam()->Cone;
		if(fru->TestPoint(root_point) == Frustum::NOTVISIBLE)
		{
			// BBox не виден => halo должно исчезнуть сразу
			state.bbox_visible = false;
			state.prev_time = current_time;
			state.state = HaloState::hidden;
			return;

		}
		
		// BBox был виден и остался видимым
		if((current_time - state.prev_trace_time) > 0.2)
		{
			state.prev_trace_time = current_time;
			state.camera_was_changed = false;
			// оттрассируем луч
			ShotTracer tr(0, camera,
				(root_point - camera),
				(root_point - camera).Length(),
				ShotTracer::F_VISROOFS|ShotTracer::F_SKIP_SHIELDS|ShotTracer::F_SKIP_INVISIBLE_ENTS|ShotTracer::F_SIGHT);
			
			if(tr.GetMaterial() == ShotTracer::MT_AIR)
			{
				// луч попал в воздух => halo по-прежнему виден
				state.prev_trace_time = current_time;
			}
			else
			{
				// луч попал не в воздух => halo перестал быть видимым
				state.prev_time = current_time;
				state.prev_trace_time = current_time;
				state.state = HaloState::disappearing;
				state.trace_ray_visible = false;
			}
		}
		else
		{
			// еще не время проверять trace ray, но камера двигалась
			state.camera_was_changed = true;
		}
		return;		
	}
}

//
// класс simple halo
//

// конструктор с параметрами
SimpleHaloEffect::SimpleHaloEffect(
								   const point3 rp,			  // начальная точка
								   const point3& Color,
								   const SIMPLE_HALO_EFFECT_INFO &info
								   ): BaseHalo(rp, info.Shader, info.Sound, info.SndMov)
{
//	LOG("SimpleHaloEffect::SimpleHaloEffect begin\n");
	// создание массивов точек, цветов и текстурных координат
	ready_num = PEAK_NUM;
	// создать массивы
	color_vector = new unsigned int [PEAK_NUM];
	texcoord_vector = new texcoord [PEAK_NUM];
	ready_vector = new point3 [PEAK_NUM];
	points2d_vector = new point3 [1];
	points_vector = new point3 [1];

	size = info.Size;

	unsigned int c1 = Color.x * 255.0;
	unsigned int c2 = Color.y * 255.0;
	unsigned int c3 = Color.z * 255.0;
	color = RGB_MAKE(c1, c2, c3);
	save_color = color;
	//LOG("SimpleHaloEffect::SimpleHaloEffect end\n");
}

// деструктор
SimpleHaloEffect::~SimpleHaloEffect()
{
	delete[] color_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	delete[] points2d_vector;
	delete[] points_vector;
}

// следующий тик времени
// (только расчет координат)
bool SimpleHaloEffect::NextTick(const float current_time)
{
//	LOG("SimpleHaloEffect::NextTick begin\n");
	if(first_time) { first_time = false; Start(current_time); }

	UpdateVisibility(current_time);

	return true;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool SimpleHaloEffect::Start(const float start_time)
{
//	LOG("SimpleHaloEffect::Start begin\n");
	// прописываем текстурные координаты
	texcoord_vector[0] = texcoord(0, 1);
	texcoord_vector[1] = texcoord(1, 1);
	texcoord_vector[2] = texcoord(1, 0);
	texcoord_vector[3] = texcoord(0, 1);
	texcoord_vector[4] = texcoord(1, 0);
	texcoord_vector[5] = texcoord(0, 0);
	
	ready_vector[0].z = 0;
	ready_vector[1].z = 0;
	ready_vector[2].z = 0;
	ready_vector[3].z = 0;
	ready_vector[4].z = 0;
	ready_vector[5].z = 0;

	state.prev_time = start_time;
	state.state = HaloState::hidden;
	state.appearing_time = 0.5;
	state.disappearing_time = 0.1;
//	LOG("SimpleHaloEffect::Start end\n");

	return true;
}


//
// класс halo
//

// конструктор с параметрами
HaloEffect::HaloEffect(
					   const point3 rp,			  // начальная точка
					   const point3& Color,
					   const HALO_EFFECT_INFO &info
					   ): BaseHalo(rp, info.Shader, info.Sound, info.SndMov)
{
	// создание массивов точек, цветов и текстурных координат
	ready_num = PEAK_NUM;
	// создать массивы
	color_vector = new unsigned int [PEAK_NUM];
	texcoord_vector = new texcoord [PEAK_NUM];
	ready_vector = new point3 [PEAK_NUM];
	points2d_vector = new point3 [1];
	points_vector = new point3 [1];

	far_dist = info.FarDist;
	near_dist = info.NearDist;
	large_size = info.LargeSize;
	small_size = info.SmallSize;
	near_color_factor = info.NearColorFactor;
	far_color_factor = info.FarColorFactor;
	dist_range = far_dist - near_dist;
	size_range = large_size - small_size;
	color_factor_range = far_color_factor - near_color_factor;

	color3 = Color;
	unsigned int c1 = Color.x * 255.0;
	unsigned int c2 = Color.y * 255.0;
	unsigned int c3 = Color.z * 255.0;
	color = RGB_MAKE(c1, c2, c3);
	save_color = color;
}

// деструктор
HaloEffect::~HaloEffect()
{
	delete[] color_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	delete[] points2d_vector;
	delete[] points_vector;
}

// следующий тик времени
// (только расчет координат)
bool HaloEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	
	// размер и цвет halo в зависимости от расстояния
	point3 tmpcolor = color3;
	point3 camera = pGraphPipe->GetCam()->GetPos();
	float dist = (camera - root_point).Length();
	//DebugInfo::Add(100, 270, "halo: dist = %f", dist);

	if(dist < near_dist)
	{
		size = small_size;
		//DebugInfo::Add(100, 240, "halo: nearest");
	}
	else
	{
		if(dist > far_dist)
		{
			size = large_size;
			tmpcolor *= far_color_factor;
			//DebugInfo::Add(100, 240, "halo: furthest");
		}
		else
		{
			size = small_size + size_range*(dist - near_dist)/dist_range;
			tmpcolor *= (near_color_factor + color_factor_range*(dist - near_dist)/dist_range);
			//DebugInfo::Add(100, 240, "halo: middle");
		}
	}

	unsigned int c1 = tmpcolor.x * 255.0;
	unsigned int c2 = tmpcolor.y * 255.0;
	unsigned int c3 = tmpcolor.z * 255.0;

	save_color = RGB_MAKE(c1, c2, c3);
	//DebugInfo::Add(100, 300, "halo: before update visibility: save_color = %X, color = %X", save_color, color);
	UpdateVisibility(current_time);
	//DebugInfo::Add(100, 330, "halo: after update visibility: save_color = %X, color = %X", save_color, color);

	return true;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool HaloEffect::Start(const float start_time)
{
	// прописываем текстурные координаты
	texcoord_vector[0] = texcoord(0, 1);
	texcoord_vector[1] = texcoord(1, 1);
	texcoord_vector[2] = texcoord(1, 0);
	texcoord_vector[3] = texcoord(0, 1);
	texcoord_vector[4] = texcoord(1, 0);
	texcoord_vector[5] = texcoord(0, 0);
	
	ready_vector[0].z = 0;
	ready_vector[1].z = 0;
	ready_vector[2].z = 0;
	ready_vector[3].z = 0;
	ready_vector[4].z = 0;
	ready_vector[5].z = 0;

	state.prev_time = start_time;
	state.state = HaloState::hidden;
	state.appearing_time = 0.5;
	state.disappearing_time = 0.1;
	return true;
}

//
// класс мигающего halo
//

// конструктор с параметрами
BlinkingHaloEffect::BlinkingHaloEffect(
								   const point3 rp,			  // начальная точка
								   const point3& Color,
								   const BLINKING_HALO_EFFECT_INFO &info
								   ): BaseHalo(rp, info.Shader, info.Sound, info.SndMov)
{
	// создание массивов точек, цветов и текстурных координат
	ready_num = PEAK_NUM;
	// создать массивы
	color_vector = new unsigned int [PEAK_NUM];
	texcoord_vector = new texcoord [PEAK_NUM];
	ready_vector = new point3 [PEAK_NUM];
	points2d_vector = new point3 [1];
	points_vector = new point3 [1];

	size = info.Size;

	unsigned int c1 = Color.x * 255.0;
	unsigned int c2 = Color.y * 255.0;
	unsigned int c3 = Color.z * 255.0;
	color = RGB_MAKE(c1, c2, c3);
	save_color = color;
	my_save_color = Color;

	sin_a = info.SinA;
}

// деструктор
BlinkingHaloEffect::~BlinkingHaloEffect()
{
	delete[] color_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	delete[] points2d_vector;
	delete[] points_vector;
}

// следующий тик времени
// (только расчет координат)
bool BlinkingHaloEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	UpdateVisibility(current_time);

	// заставим его мигать
	float dt = current_time - start_time;
	unsigned int c1 = (my_save_color*sin(sin_a*dt)*sin(sin_a*dt)).x * 255.0;
	unsigned int c2 = (my_save_color*sin(sin_a*dt)*sin(sin_a*dt)).y * 255.0;
	unsigned int c3 = (my_save_color*sin(sin_a*dt)*sin(sin_a*dt)).z * 255.0;
	save_color = RGB_MAKE(c1, c2, c3);


	return true;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool BlinkingHaloEffect::Start(const float start_time)
{
	// прописываем текстурные координаты
	texcoord_vector[0] = texcoord(0, 1);
	texcoord_vector[1] = texcoord(1, 1);
	texcoord_vector[2] = texcoord(1, 0);
	texcoord_vector[3] = texcoord(0, 1);
	texcoord_vector[4] = texcoord(1, 0);
	texcoord_vector[5] = texcoord(0, 0);
	
	ready_vector[0].z = 0;
	ready_vector[1].z = 0;
	ready_vector[2].z = 0;
	ready_vector[3].z = 0;
	ready_vector[4].z = 0;
	ready_vector[5].z = 0;

	state.prev_time = start_time;
	state.state = HaloState::hidden;
	state.appearing_time = 0.5;
	state.disappearing_time = 0.1;
	this->start_time = start_time;
	return true;
}
