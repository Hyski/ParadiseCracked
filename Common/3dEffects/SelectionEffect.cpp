/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ ���������
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

//#define SPOT_EFFECT_DEBUG 1 // enable for log file

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "SelectionEffect.h"

//
// ����� ���������
//

// ����������� � �����������
SelectionEffect::SelectionEffect(
								 const point3 Target,        // ����������� �����
								 const SELECTION_EFFECT_INFO &info
								 )
{
	visible = true;
	// �������� �����
	root_point = Target + point3(0.0f, 0.0f, 0.5f);

	// ���������
	radius = info.Radius;
	small_size = info.SmallSize;
	large_size = info.LargeSize;
	angle_velocity = info.AngVel;
	angle_delta = info.AngDelta;

	// ����������� ��������
	points_num = 2 * info.PartNum;
	ready_vector_size = points_num * PEAK_NUM;
	ready_num = ready_vector_size;

	// ������� �������
	points_vector = new point3 [points_num];
	points2d_vector = new point3 [points_num];
	color_vector = new unsigned int [ready_num];
	texcoord_vector = new texcoord [ready_num];
	ready_vector = new point3 [ready_num];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;
	finished = false;
	// ������
	SetShader(info.Shader);
	// ����
	if(info.Sound == "")
	{
		// ����� ���
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

// ����������
SelectionEffect::~SelectionEffect()
{
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// ����
	delete sound_object;
}

// ��������� ��� �������
// (������ ������ ���������)
bool SelectionEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	if(!visible)
	{
		ready_num = 0;
		return true;
	}
	bbox.Degenerate();
	bbox.Enlarge(root_point - point3(radius+large_size, radius+large_size, 0.1));
	bbox.Enlarge(root_point + point3(radius+large_size, radius+large_size, 0.1));

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	float dtt;


	dt = current_time - start_time;

	dtt=dt+0.37*sin(dt*PIm2);

	Quaternion qt;
	qt.FromAngleAxis(fmod(angle_velocity*dtt, PIm2), axis);

	current_vector = (qt * start_vector) * radius;

	// ����
	unsigned int c = 200.0 + 55.0*sin(dt*PIm2);
	color = RGB_MAKE(c, c, c);

	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool SelectionEffect::Update()
{
	if(!visible) return true;
	// ����
	sound_object->Update(root_point, velocity);
	// ����������� ������ ��� ������������� (points_vector)
	for(int i = 0; i < points_num; i++)
	{
		Quaternion qt;
		qt.FromAngleAxis(fmodf((PIm2*i*2.0f)/points_num, PIm2), axis);
		if(i % 2)
		{
			Quaternion qt1;
			qt1.FromAngleAxis(angle_delta, axis);
			points_vector[i] = root_point + (qt1 * (qt * current_vector));
		}
		else
		{
			points_vector[i] = root_point + (qt * current_vector);
		}
	}
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(points_num, points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool SelectionEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;
	start_vector = Normalize(point3(1, 1, 0));
	current_vector = start_vector;
	axis = point3(0, 0, 1.0);
	
	// ����
	color = 0xffffffff;

	// ����������� ���������� ����������
	for(int i = 0; i < points_num; i++)
	{
		texcoord_vector[i*PEAK_NUM] = texcoord(0, 1);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(0, 1);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, 0);

		// ����
		color_vector[i*PEAK_NUM] = color;
		color_vector[i*PEAK_NUM + 1] = color;
		color_vector[i*PEAK_NUM + 2] = color;
		color_vector[i*PEAK_NUM + 3] = color;
		color_vector[i*PEAK_NUM + 4] = color;
		color_vector[i*PEAK_NUM + 5] = color;
	}

	return true;
}

// ������� ����������� �����
void SelectionEffect::Multiply()
{
	if(!visible) return;
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

	// ������� �������, ��� ��� ����� �����, � ���������� ������� ������ 
	// ����� ������� �������, � ������� ��� �����
	ready_num = ready_vector_size;
	// ������� ���������� �����, ������� ���������� � ������� �������
	k = 0;  
	for(int i = 0; i < points_num; i++)
	{
		// ����������� ��������� ������
		z = points2d_vector[i].z;
		if(z < 0)
		{
			// ��� ����� �� ����� ���������� - ��������� ��
			// �������� ���������� ������� ������
			ready_num -= PEAK_NUM;
			continue;
		}
		// ��������� ����������
		ready_vector[k*PEAK_NUM].z = z;
		ready_vector[k*PEAK_NUM + 1].z = z;
		ready_vector[k*PEAK_NUM + 2].z = z;
		ready_vector[k*PEAK_NUM + 3].z = z;
		ready_vector[k*PEAK_NUM + 4].z = z;
		ready_vector[k*PEAK_NUM + 5].z = z;

		if(i % 2)
		{
			xsize = small_size*(z*(n-f)+f)*xfactor;
			ysize = small_size*(z*(n-f)+f)*yfactor;
		}
		else
		{
			xsize = large_size*(z*(n-f)+f)*xfactor;
			ysize = large_size*(z*(n-f)+f)*yfactor;
		}

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

		// ����
		color_vector[k*PEAK_NUM] = color;
		color_vector[k*PEAK_NUM + 1] = color;
		color_vector[k*PEAK_NUM + 2] = color;
		color_vector[k*PEAK_NUM + 3] = color;
		color_vector[k*PEAK_NUM + 4] = color;
		color_vector[k*PEAK_NUM + 5] = color;

		// �������� ���������� ������������ �����
		k++;
	}
}

