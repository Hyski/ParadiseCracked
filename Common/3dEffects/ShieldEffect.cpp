/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффекта броника
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "ShieldEffect.h"

// функция для заполнения массива единичных векторов
namespace
{
	void setUnitVectors(point3* vectors, unsigned int num)
	{
		float alpha = 0;
		for(int i = 0; i < num; i++)
		{
			vectors[i] = point3(FastCos(alpha), FastSin(alpha), 1.0f);
			alpha += PIm2/num;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс эффекта броника
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// конструктор с параметрами
ShieldEffect::ShieldEffect(
						   const point3& Source,
						   const point3& Target,
						   const SHIELD_INFO& info
						   )
{

	// корневая точка
	root_point = Target;
	vel = point3(0, 0, 0);
	
	spot_num = info.PointsNum;        // количество пятен в окружности
	circles_num = info.CirclesNum;    // количество окружностей
	dx = info.DX;                     // приращение по ху
	a = info.ParabolaFactor;          // коэффициет у параболы
	lifetime = info.LifeTime;         // время жизни эффекта
	size = 0.02;

	// создание массивов точек, цветов и текстурных координат
	points_num = circles_num * spot_num;
	ready_vector_size = ready_num = points_num * PEAK_NUM;

	// создать массивы
	unit_vectors = new point3[spot_num];
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;
	// флаг, указывающий на необходимость завершения эффекта
	finished = false;

	direction = Normalize(Source - Target);

	bbox.Degenerate();
	bbox.Enlarge(root_point - point3(dx*circles_num, dx*circles_num, dx*circles_num));
	bbox.Enlarge(root_point + point3(dx*circles_num, dx*circles_num, dx*circles_num));

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
			sound_object = new FixedSound(info.Sound, Target);
		}
	}
}

// деструктор
ShieldEffect::~ShieldEffect()
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
bool ShieldEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;
	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// поменяем цвет
	/*
	float factor = 1.0;
	if(dt < 0.1*lifetime) factor = 10.0f*dt/lifetime;
	if(dt > 0.2*lifetime) factor = 1.25f*(lifetime - dt)/lifetime;*/
	for(int i = 0; i < circles_num; i++) // цикл по окружностям
	{
		for(int j = 0; j < spot_num; j++) // цикл по точкам в окружности
		{
			// пропишем цвет
			int c = (255 - 240.0f*i/circles_num);
			if(dt < 0.2*lifetime)
				c *= 5.0f*dt/lifetime;
			if(dt > 0.2*lifetime)
			{
				c = (255*i/circles_num);
				c *= 1.25f*(lifetime - dt)/lifetime;
			}
			
			unsigned int color = RGB_MAKE(c, c, c);
			color_vector[(i*spot_num + j)*PEAK_NUM] = color;
			color_vector[(i*spot_num + j)*PEAK_NUM + 1] = color;
			color_vector[(i*spot_num + j)*PEAK_NUM + 2] = color;
			color_vector[(i*spot_num + j)*PEAK_NUM + 3] = color;
			color_vector[(i*spot_num + j)*PEAK_NUM + 4] = color;
			color_vector[(i*spot_num + j)*PEAK_NUM + 5] = color;

		}
	}

	return true;
}

// возвращает bounding box эффекта
BBox ShieldEffect::GetBBox(const float )
{
	return bbox;
}

// подготовка массива готовых точек и массива цветов точек
bool ShieldEffect::Update()
{
	// звук
	sound_object->Update(root_point, vel);
	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(points_num,
		points_vector, points2d_vector);
	// размножаем точки
	Multiply();
	return true;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool ShieldEffect::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = start_time;


	// заполним массив единичных векторов
	setUnitVectors(unit_vectors, spot_num);

	// составим систему координат, связанную с трассером
	point3 OZ = direction;
	point3 OY = Normalize(direction.Cross(point3(0.0f, 0.0f, 1.0f)));
	point3 OX = Normalize(OY.Cross(OZ));

	float xy_shift = dx;

	// расставим все точки
	for(int i = 0; i < circles_num; i++) // цикл по окружностям
	{
		for(int j = 0; j < spot_num; j++) // цикл по точкам в окружности
		{
			// точка в локальной системе координат
			point3 p = unit_vectors[j]*xy_shift;
			p.z *= xy_shift*a; 

			// переведем в мировую
			float x = p.x*OX.x + p.y*OY.x + p.z*OZ.x;
			float y = p.x*OX.y + p.y*OY.y + p.z*OZ.y;
			float z = p.x*OX.z + p.y*OY.z + p.z*OZ.z;

			points_vector[i*spot_num + j] = root_point + point3(x, y, z);

			// пропишем цвет
			int c = 255 - 240.0f*i/circles_num;
			unsigned int color = RGB_MAKE(c, c, c);
			color_vector[(i*spot_num + j)*PEAK_NUM] = color;
			color_vector[(i*spot_num + j)*PEAK_NUM + 1] = color;
			color_vector[(i*spot_num + j)*PEAK_NUM + 2] = color;
			color_vector[(i*spot_num + j)*PEAK_NUM + 3] = color;
			color_vector[(i*spot_num + j)*PEAK_NUM + 4] = color;
			color_vector[(i*spot_num + j)*PEAK_NUM + 5] = color;

		}
		xy_shift += dx;
	}


	// прописываем текстурные координаты
	for(int i = 0; i < points_num; i++)
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

// функция размножения точек
void ShieldEffect::Multiply()
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
	for(int i = 0; i < points_num; i++)
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

		xsize = size*(z*(n-f)+f)*xfactor;
		ysize = size*(z*(n-f)+f)*yfactor;

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
