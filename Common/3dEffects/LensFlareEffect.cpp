/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ lens flare
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                


#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "LensFlareEffect.h"

//
// ����� lens flare
//

// ����������� � �����������
LensEffect::LensEffect(
					   const point3 StartPoint,    // ��������� �����
					   const point3 EndPoint,	   // �������� �����
					   const LENS_FLARE_EFFECT_INFO& info
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

	start_point = StartPoint;                       // ��������� �����
	end_point = EndPoint;                           // �������� �����

	// ����� ����� �������
	lifetime = (end_point - start_point).Length() / info.VelFactor;

	size = info.Size;                                    // ������

	distance = info.Distance;                            // ���������� �� ������

	velocity = (end_point - start_point)/lifetime;  // ��������
	
	// ����, ����������� �� ������������� ���������� �������
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
			sound_object = new FixedSound(info.Sound, StartPoint);
		}
	}
}

// ����������
LensEffect::~LensEffect()
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
bool LensEffect::NextTick(const float current_time)
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

// ���������� ������� ������� ����� � ������� ������ �����
bool LensEffect::Update()
{
	// ����
	sound_object->Update(current_point, velocity);
	// ����������� ������ ��� ������������� (points_vector)
	points_vector[0] = current_point;
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(1, points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}

// ���������� bounding box �������
BBox LensEffect::GetBBox(const float)
{
	BBox b;
	float s = 1.0;
	b.Degenerate();
	b.Enlarge(current_point + point3(s, s, s));
	b.Enlarge(current_point - point3(s, s, s));
	return b;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool LensEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;
	// ����������� ���������� ����������
	texcoord_vector[0] = texcoord(0, 0);
	texcoord_vector[1] = texcoord(1, 0);
	texcoord_vector[2] = texcoord(1, 1);
	texcoord_vector[3] = texcoord(0, 0);
	texcoord_vector[4] = texcoord(1, 1);
	texcoord_vector[5] = texcoord(0, 1);

	// z - ����������
	ready_vector[0].z = 0;
	ready_vector[1].z = 0;
	ready_vector[2].z = 0;
	ready_vector[3].z = 0;
	ready_vector[4].z = 0;
	ready_vector[5].z = 0;

	// ����
	color = 0xffffffff;
	color_vector[0] = color;
	color_vector[1] = color;
	color_vector[2] = color;
	color_vector[3] = color;
	color_vector[4] = color;
	color_vector[5] = color;
	// ���������� ������
	current_point = start_point;
	return true;
}

// ������� ����������� �����
void LensEffect::Multiply()
{
	float f, n, betta;
	float xsize, xfactor;
	float ysize, yfactor;
	int xres, yres;

	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);

	xfactor = xres/(n*f*tan(betta*0.5));
	yfactor = yres/(n*f*tan(betta*0.5));

	point3 target = point3(points2d_vector[0].x, points2d_vector[0].y, 0.0);
	point3 center = point3(xres*0.5, yres*0.5, 0.0);
	point3 flare = center + (center - target)*distance;

	xsize = size*n*xfactor;
	ysize = size*n*yfactor;

	// ��������� ����������
	ready_vector[0].x = flare.x - xsize;
	ready_vector[1].x = flare.x + xsize;
	ready_vector[2].x = flare.x + xsize;
	ready_vector[3].x = flare.x - xsize;
	ready_vector[4].x = flare.x + xsize;
	ready_vector[5].x = flare.x - xsize;

	ready_vector[0].y = flare.y + ysize;
	ready_vector[1].y = flare.y + ysize;
	ready_vector[2].y = flare.y - ysize;
	ready_vector[3].y = flare.y + ysize;
	ready_vector[4].y = flare.y - ysize;
	ready_vector[5].y = flare.y - ysize;
}
