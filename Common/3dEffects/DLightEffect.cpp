/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ ������������� ���������
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                


#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "../../GameLevel/GameLevel.h"
#include "../../iworld.h"
#include "EffectInfo.h"
#include "DLightEffect.h"
#include "../../logic2/TraceUtils.h"


//
// ������� ����� ������� ������������� ���������
//

// �����������
DLightEffect::DLightEffect()
{
	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;
	vel = point3(0, 0, 0);  // ��������
	ready_num = 0;
	sound_object = new EmptySound();
}

// ����������
DLightEffect::~DLightEffect()
{
	// ����
	delete sound_object;
	// ������������ ���������
	d_light->Dead = true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool DLightEffect::Update()
{
	// ����
	sound_object->Update(current_point, vel);
	d_light->Pos = current_point;
	d_light->FindTris = true;
	return true;
}

// ���������� bounding box �������
BBox DLightEffect::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(current_point - point3(100.1, 100.1, 100.1));
	b.Enlarge(current_point + point3(100.1, 100.1, 100.1));
	return b;
}


//
// ����� ������������� ���������, �������� �� ������
//

// ����������� � �����������
LineDLightEffect::LineDLightEffect(
								   const point3& StartPoint,            // ��������� ��������
								   const point3& EndPoint,              // �������� �����
								   const LINE_DLIGHT_EFFECT_INFO& info
								   )
{
	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;
	// ����, ����������� �� ������������� ���������� �������
	finished = false;

	start_point = StartPoint;
	end_point = EndPoint;
	color = info.Color;
	radius = info.Radius;
	current_point = start_point;

	// ����� ����� �������
	lifetime = (end_point - start_point).Length() / info.VelFactor;

	velocity = (end_point - start_point)/lifetime;  // ��������

	direction = Normalize(end_point - start_point);
}

// ��������� ��� �������
// (������ ������ ���������)
bool LineDLightEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// ���������� ������
	current_point = start_point + velocity*dt;
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool LineDLightEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// �������� ������������� ��������� �����
	d_light = new Mark(current_point, radius, "dynlight");
	d_light->dynamic = true;
	d_light->Col = color;

	IWorld::Get()->GetLevel()->AddMark(d_light);

	return true;
}


//
// ����� ������� �������
//

// ����������� � �����������
FlashEffect::FlashEffect(
						 const point3& Target,            // ����� ���������
						 const FLASH_EFFECT_INFO& info
						 )
{
	// �������� �������� �����, ������ � ���������� ���������
	ready_num = PEAK_NUM;

	// ������� �������
	color_vector = new unsigned int [PEAK_NUM];
	points2d_vector = new point3 [1];
	points_vector = new point3 [1];
	texcoord_vector = new texcoord [PEAK_NUM];
	ready_vector = new point3 [PEAK_NUM];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// 
	root_point = Target;                       // ��������� �����

	// ����� ����� �������
	lifetime = info.LifeTime;
	switch_time = info.SwitchTime;
	size_begin = info.SizeBegin;
	size_end = info.SizeEnd;

	// ����, ����������� �� ������������� ���������� �������
	finished = false;

	bbox.Degenerate();
	bbox.Enlarge(point3(1000, 1000, 1000));
	bbox.Enlarge(point3(-1000, -1000, -1000));

	vel = point3(0, 0, 0);

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
FlashEffect::~FlashEffect()
{
	// ����
	delete sound_object;
	delete[] color_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	delete[] points2d_vector;
	delete[] points_vector;
}

// ��������� ��� �������
// (������ ������ ���������)
bool FlashEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	if(finished) return true;

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}
	
	if(dt < switch_time)
	{
		// ���������� ������
		size = size_begin + (size_end - size_begin)*dt/switch_time;
	}
	else
	{
		// �����������
		size = size_end;
		unsigned int c = 255*(1.0 - (dt - switch_time)/(lifetime - switch_time));
		color = RGB_MAKE(c, c, c);
	}

	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool FlashEffect::Update()
{
	// ����
	sound_object->Update(root_point, vel);
	// ����������� ������ ��� ������������� (points_vector)
	points_vector[0] = root_point;
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(1, points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool FlashEffect::Start(const float start_time)
{
	// �������� trace ray, ���� �� ������, �� ������ ����� �����
	point3 camera = pGraphPipe->GetCam()->GetPos();
	// ������������ ���
	ShotTracer tr(0, camera,
		(root_point - camera),
    (root_point - camera).Length()-0.1,
	ShotTracer::F_SKIP_SHIELDS|ShotTracer::F_SKIP_INVISIBLE_ENTS|ShotTracer::F_SIGHT);
				
	if(tr.GetMaterial() != ShotTracer::MT_AIR)
	{
		// ��� ����� � �����-�� ������ => ������� ������
		finished = true;
		return true;
	}

	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// ����
	for(int i = 0; i < ready_num; i++) color_vector[i] = 0xffffffff;

	// ����������� ���������� ����������
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

	color = 0xffffffff;
	
	return true;
}
	
// ������� ����������� �����
void FlashEffect::Multiply()
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

	// ����������� ��������� ������
	z = points2d_vector[0].z;

	// ��������� ����������

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
}

//
// ����� ������� ��������������� �������
//

// ������� ����������� �����
void MultiTexFlashEffect::Multiply()
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

	// ����������� ��������� ������
	z = points2d_vector[0].z;

	// ��������� ����������

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

	switch(phase)
	{
	case 0 : 
		{
			u1 = 0;
			v1 = 0;
			u2 = 0.5;
			v2 = 0.5;
			break;
		}
	case 1 : 
		{
			u1 = 0.5;
			v1 = 0;
			u2 = 1;
			v2 = 0.5;
			break;
		}
	case 2 :
		{
			u1 = 0.5;
			v1 = 0.5;
			u2 = 1;
			v2 = 1;
			break;
		}
	case 3 : 
		{
			u1 = 0;
			v1 = 0.5;
			u2 = 0.5;
			v2 = 1;
			phase = 0;
			break;
		}
	}
	phase++;
	// ����������� ���������� ����������
	texcoord_vector[0] = texcoord(u1, v2);
	texcoord_vector[1] = texcoord(u2, v2);
	texcoord_vector[2] = texcoord(u2, v1);
	texcoord_vector[3] = texcoord(u1, v2);
	texcoord_vector[4] = texcoord(u2, v1);
	texcoord_vector[5] = texcoord(u1, v1);
}
