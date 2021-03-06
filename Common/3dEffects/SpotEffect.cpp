/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: �����
                ���������� �����
                �������, ����������� ������ ������ ������
				��������������� ����� (�����������)
				��������� ����� (��������� ��������������� �����)
				������� ����� (������� ��������������� �����)
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                


#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "SpotEffect.h"

// ��������� ��� �������
// (������ ������ ���������)
bool SpotEffect::NextTick(const float current_time)
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
bool SpotEffect::Update()
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
BBox SpotEffect::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(current_point + point3(size, size, size));
	b.Enlarge(current_point - point3(size, size, size));
	return b;
}

// ����������
SpotEffect::~SpotEffect()
{
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// ����
	delete sound_object;
}

// ����������� � �����������
SpotEffect::SpotEffect(
					   const point3 StartPoint,    // ��������� �����
					   const point3 EndPoint,	   // �������� �����
					   const SPOT_EFFECT_INFO& info
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

	

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool SpotEffect::Start(const float start_time)
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

	// ����
	color = 0xffffffff;
		
	// ���������� ������
	current_point = start_point;

	return true;
}

// ������� ����������� �����
void SpotEffect::Multiply()
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
	ready_vector[0].z = z;
	ready_vector[1].z = z;
	ready_vector[2].z = z;
	ready_vector[3].z = z;
	ready_vector[4].z = z;
	ready_vector[5].z = z;

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

	// ��������� ����
	color_vector[0] = color;
	color_vector[1] = color;
	color_vector[2] = color;
	color_vector[3] = color;
	color_vector[4] = color;
	color_vector[5] = color;
}

//
// ����� ����������� �����
//

// ��������� ��� �������
// (������ ������ ���������)
bool ShineSpotEffect::NextTick(const float current_time)
{
	float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// ���������� �������
	current_point = start_point + velocity*dt;
	// ����� ���������� ����������
	switch(phase % 4)
	{
	case 0 : x1 = 0; y1 = 0; x2 = 0.5; y2 = 0.5; break;
	case 1 : x1 = 0.5; y1 = 0; x2 = 1; y2 = 0.5; break;
	case 2 : x1 = 0; y1 = 0.5; x2 = 0.5; y2 = 1; break;
	case 3 : x1 = 0.5; y1 = 0.5; x2 = 1; y2 = 1; break;
	}
	phase++;
	// ����������� ���������� ����������
	texcoord_vector[0] = texcoord(x1, y1);
	texcoord_vector[1] = texcoord(x2, y1);
	texcoord_vector[2] = texcoord(x2, y2);
	texcoord_vector[3] = texcoord(x1, y1);
	texcoord_vector[4] = texcoord(x2, y2);
	texcoord_vector[5] = texcoord(x1, y2);
	return true;
}


//
// �������, ����������� ������ ������ ������
//

// ����������� � �����������
RotateSpotEffect::RotateSpotEffect(
								   const point3 StartPoint,      // ��������� �����
								   const point3 EndPoint,	     // �������� �����
								   const ROTATE_SPOT_EFFECT_INFO& info
								   )
{
	// �������� �������� �����, ������ � ���������� ���������
	sequence_number = info.SequenceNumber;
	sequence_size = info.SequenceSize;
	points_num = sequence_number*sequence_size;
	ready_num = ready_vector_size = points_num*PEAK_NUM;

	// ������� �������
	color_vector = new unsigned int [ready_vector_size];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	
	current_vectors = new point3 [sequence_size];
	current_points = new point3 [sequence_size];
	current_vectors_disp = new float [sequence_number];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	start_point = StartPoint;                       // ��������� �����
	end_point = EndPoint;                           // �������� �����

	// ����� ����� �������
	lifetime = (end_point - start_point).Length() / info.VelFactor;

	size_first = info.SizeFirst;                         // ������ ������ �������
	size_last = info.SizeLast;                           // ������ ��������� �������
	radius_start = info.RadiusStart;                                // ������
	radius_end = info.RadiusEnd;
	disp = info.Disp;

	velocity = (end_point - start_point)/lifetime;  // ��������
	ang_vel = info.AngVel;
	delta = info.Delta;
	
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
RotateSpotEffect::~RotateSpotEffect()
{
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	delete[] current_vectors;
	delete[] current_points;
	delete[] current_vectors_disp;
	// ����
	delete sound_object;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool RotateSpotEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;
	for(int i = 0; i < points_num; i++)
	{
		// ����������� ���������� ����������
		texcoord_vector[PEAK_NUM*i] = texcoord(0, 0);
		texcoord_vector[PEAK_NUM*i + 1] = texcoord(1, 0);
		texcoord_vector[PEAK_NUM*i + 2] = texcoord(1, 1);
		texcoord_vector[PEAK_NUM*i + 3] = texcoord(0, 0);
		texcoord_vector[PEAK_NUM*i + 4] = texcoord(1, 1);
		texcoord_vector[PEAK_NUM*i + 5] = texcoord(0, 1);

		// � ����
		color_vector[PEAK_NUM*i] = 0xffffffff;
		color_vector[PEAK_NUM*i + 1] = 0xffffffff;
		color_vector[PEAK_NUM*i + 2] = 0xffffffff;
		color_vector[PEAK_NUM*i + 3] = 0xffffffff;
		color_vector[PEAK_NUM*i + 4] = 0xffffffff;
		color_vector[PEAK_NUM*i + 5] = 0xffffffff;
	}

	for(int i = 0; i < sequence_number; i++)
	{
		// ���� ����� �������
		if(disp) current_vectors_disp[i] = (float)rand()/32768.0;
		else current_vectors_disp[i] = 1.0;
	}
	
	// ������ �����������
	direction = Normalize(end_point - start_point);
	// ��������� ������ ��������
	start_vector = Normalize(direction.Cross(point3(1, 0, 0))) * radius_start;

	return true;
}

// ��������� ��� �������
// (������ ������ ���������)
bool RotateSpotEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	float dtt;
	dt = current_time - start_time;

	if ( (dt - delta*sequence_size) >= lifetime)
	{
		finished = true;
		return true;
	}

	Quaternion qt;
	for(int i = 0; i < sequence_size; i++)
	{
		dtt = dt - delta*i;
		if( (dtt <= 0) || (dtt >= lifetime) )
		{
			current_points[i] = point3(10000, 10000, 10000);
		}
		else
		{
			// ���������� ������
			current_points[i] = start_point + velocity*dtt;
			// ����� ������� ������
			qt.FromAngleAxis(fmod((ang_vel*dtt), PIm2), direction);

			// ��������� ��������� �� ������
			float r = radius_start + (radius_end - radius_start)*dtt/lifetime;
			current_vectors[i] = (qt * start_vector) * r;
		}
	}
	return true;
}


// ���������� ������� ������� ����� � ������� ������ �����
bool RotateSpotEffect::Update()
{
	// ����
	sound_object->Update(current_points[0], velocity);
	Quaternion qt;
	// ����������� ������ ��� ������������� (points_vector)
	for(int k = 0; k < sequence_size; k++) // ���� �� ������ � ������������������
	{
		// ������ ����� � ������ ������������������ �� ������� ��������:
		points_vector[k*sequence_number] = current_points[k] + current_vectors[k];
		// ��������� - �������:
		for(int i = 1; i < sequence_number; i++) // ���� �� ���� �������������������
		{
			qt.FromAngleAxis(fmod((PIm2*i)/sequence_number, PIm2), direction);
			points_vector[k*sequence_number + i] = current_points[k] + (qt * current_vectors[k])*current_vectors_disp[i];
		}
	}
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(points_num, points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}

// ���������� bounding box �������
BBox RotateSpotEffect::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(start_point);
	b.Enlarge(end_point);
	return b;
}

// ������� ����������� �����
void RotateSpotEffect::Multiply()
{
	float f, n, betta;
	float xsize, xfactor;
	float ysize, yfactor;
	float size = size_first;
	float z;
	int xres, yres;
	int k;
	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);

	xfactor = xres/(n*f*tan(betta/2));
	yfactor = yres/(n*f*tan(betta/2));

	color = 0xffffffff;
	// ������� �������, ��� ��� ����� �����, � ���������� ������� ������ 
	// ����� ������� �������, � ������� ��� �����
	ready_num = ready_vector_size;
	// ������� ���������� �����, ������� ���������� � ������� �������
	k = 0;
	for(int i = 0; i < points_num; i++)
	{
		if(!(i % sequence_number))
		{
			size = size_first - (size_first - size_last)/points_num*i;
			unsigned int c = 255 - 200.0*i/points_num;
			color = RGBA_MAKE(c, c, c, c);
		}
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

//
// ����� ���������������� ����� (�������, �����������)
//

// ����������� � �����������
UpsizingSpot::UpsizingSpot(
						   const point3 StartPoint,        // ��������� �����
						   const point3 EndPoint,	       // �������� �����
						   const UPSIZING_SPOT_INFO &info  // ���������� � ����������
						   )
{
	// �������� �������� �����, ������ � ���������� ���������
	ready_num = 4;
	// ������� �������
	color_vector = new unsigned int [4];
	texcoord_vector = new texcoord [4];
	ready_vector = new point3 [4];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	start_point = StartPoint;                       // ��������� �����
	direction = Normalize(EndPoint - StartPoint);   // ����������� ��������

	// ����� ����� �������
	lifetime = info.LifeTime;
	start_vel = info.StartVel;
	accel = info.Accel;
	current_point = start_point;
	width_first = info.WidthFirst;
	width_second = info.WidthSecond;
	width_vel = info.WidthVel;

	velocity = point3(0, 0, 0);  // ��������
	
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
UpsizingSpot::~UpsizingSpot()
{
	delete[] color_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// ����
	delete sound_object;
}

// ��������� ��� �������
// (������ ������ ���������)
bool UpsizingSpot::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	// ������� �� ������� ����� ������� �������� � �������� �������� �������
	float dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// ���������� ������
	current_point = start_point + direction*(start_vel*dt + accel*dt*dt*0.5);
	// ������� � ������
	d_width = width_vel*dt;
	// ����
	unsigned int c;
	if(dt < (0.4*lifetime)) c = 255;
	else c = 1.666667*(lifetime - dt)/lifetime*255;
	color = RGB_MAKE(c, c, c);
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool UpsizingSpot::Update()
{
	// ����
	sound_object->Update(current_point, velocity);
	// ���������� �����
	Multiply();
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool UpsizingSpot::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// ����������� ���������� ����������
	texcoord_vector[0] = texcoord(1, 1);
	texcoord_vector[1] = texcoord(0, 1);
	texcoord_vector[2] = texcoord(0, 0);
	texcoord_vector[3] = texcoord(1, 0);
	// ����
	color = 0xffffffff;
	return true;
}

//
// ����� ���������� ������� (��������� ��������������� �����)
//

// ���������� bounding box �������
BBox OneUpsizingSpot::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(start_point - point3(0.1, 0.1, 0.1));
	b.Enlarge(current_point + point3(0.1, 0.1, 0.1));
	return b;
}

// ������� ����������� �����
void OneUpsizingSpot::Multiply()
{
	point3 CameraDir = direction.Cross(pGraphPipe->GetCam()->GetFront());
	ready_vector[0] = start_point - (width_second + d_width)*CameraDir;
	ready_vector[1] = start_point + (width_second + d_width)*CameraDir;
	ready_vector[2] = current_point + (width_first + d_width)*CameraDir;
	ready_vector[3] = current_point - (width_first + d_width)*CameraDir;
	// ��������� ����
	color_vector[0] = color;
	color_vector[1] = color;
	color_vector[2] = color;
	color_vector[3] = color;
}

//
// ����� �������� ������� (������� ��������������� �����)

// ���������� bounding box �������
BBox TwoUpsizingSpot::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(start_point - (current_point - start_point) - point3(0.1, 0.1, 0.1));
	b.Enlarge(current_point + point3(0.1, 0.1, 0.1));
	return b;
}

// ������� ����������� �����
void TwoUpsizingSpot::Multiply()
{
	point3 CameraDir = direction.Cross(pGraphPipe->GetCam()->GetFront());
	ready_vector[0] = start_point - (current_point - start_point) - (width_second + d_width)*CameraDir;
	ready_vector[1] = start_point - (current_point - start_point) + (width_second + d_width)*CameraDir;
	ready_vector[2] = current_point + (width_first + d_width)*CameraDir;
	ready_vector[3] = current_point - (width_first + d_width)*CameraDir;
	// ��������� ����
	color_vector[0] = color;
	color_vector[1] = color;
	color_vector[2] = color;
	color_vector[3] = color;
}

//
// ����� ��������� �����
//

// ����������� � �����������
WaveSpot::WaveSpot(
				   const point3 Target,                // ��������� �����
				   const WAVE_SPOT_INFO &info          // ���������� � ����������
				   )
{
	// �������� �������� �����, ������ � ���������� ���������
	ready_num = 4;
	// ������� �������
	color_vector = new unsigned int [4];
	texcoord_vector = new texcoord [4];
	ready_vector = new point3 [4];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;
	// ����, ����������� �� ������������� ���������� �������
	finished = false;

	root_point = Target;                       // ��������� �����
	// ����� ����� �������
	lifetime = info.LifeTime;
	start_size = info.StartSize;
	size_velocity = info.SizeVel;

	velocity = point3(0, 0, 0);  // ��������
	
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
WaveSpot::~WaveSpot()
{
	delete[] color_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// ����
	delete sound_object;
}

// ��������� ��� �������
// (������ ������ ���������)
bool WaveSpot::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	// ������� �� ������� ����� ������� �������� � �������� �������� �������
	float dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// ���������� ������
	size = start_size + size_velocity*dt;
	// ����
	unsigned int c;
	if(dt < (0.4*lifetime)) c = 255;
	else c = 1.666667*(lifetime - dt)/lifetime*255;
	color = RGB_MAKE(c, c, c);
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool WaveSpot::Update()
{
	// ����
	sound_object->Update(root_point, velocity);
	// ���������� �����
	Multiply();
	return true;
}

// ���������� bounding box �������
BBox WaveSpot::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(root_point - point3(size, size, 0.1));
	b.Enlarge(root_point + point3(size, size, 0.1));
	return b;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool WaveSpot::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// ����������� ���������� ����������
	texcoord_vector[0] = texcoord(1, 1);
	texcoord_vector[1] = texcoord(0, 1);
	texcoord_vector[2] = texcoord(0, 0);
	texcoord_vector[3] = texcoord(1, 0);
	// ����
	color = 0xffffffff;
	return true;
}

// ������� ����������� �����
void WaveSpot::Multiply()
{
	ready_vector[0] = root_point + point3(size, size, 0);
	ready_vector[1] = root_point + point3(-size, size, 0);
	ready_vector[2] = root_point + point3(-size, -size, 0);
	ready_vector[3] = root_point + point3(size, -size, 0);
	// ��������� ����
	color_vector[0] = color;
	color_vector[1] = color;
	color_vector[2] = color;
	color_vector[3] = color;
}

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ����������������� ��������� �����
//
/////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
WaveSpot2::WaveSpot2(
					 const point3 Source,                // ����� ��������
					 const point3 Target,                // ����� ���������
					 const WAVE_SPOT_INFO &info          // ���������� � ����������
				   )
{
	// �������� �������� �����, ������ � ���������� ���������
	ready_num = 4;
	// ������� �������
	color_vector = new unsigned int [4];
	texcoord_vector = new texcoord [4];
	ready_vector = new point3 [4];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;
	// ����, ����������� �� ������������� ���������� �������
	finished = false;

	root_point = Source;                       // ��������� �����
	direction = Normalize(Target - Source);    // �����������
	// ������� ������
	base = Normalize(direction.Cross(point3(0.0f, 1.0f, 0.0f)));
	float angle = static_cast<float>(rand())/32768.0f*PIm2;
	Quaternion qt;
	qt.FromAngleAxis(angle, direction);
	base = qt*base;
	// ����� ����� �������
	lifetime = info.LifeTime;
	start_size = info.StartSize;
	size_velocity = info.SizeVel;

	velocity = point3(0, 0, 0);  // ��������
	
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
WaveSpot2::~WaveSpot2()
{
	delete[] color_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// ����
	delete sound_object;
}

// ��������� ��� �������
// (������ ������ ���������)
bool WaveSpot2::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	// ������� �� ������� ����� ������� �������� � �������� �������� �������
	float dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// ���������� ������
	size = start_size + size_velocity*dt;
	// ����
	unsigned int c;
	if(dt < (0.4*lifetime)) c = 255;
	else c = 1.666667*(lifetime - dt)/lifetime*255;
	color = RGB_MAKE(c, c, c);
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool WaveSpot2::Update()
{
	// ����
	sound_object->Update(root_point, velocity);
	// ���������� �����
	Multiply();
	return true;
}

// ���������� bounding box �������
BBox WaveSpot2::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(root_point - point3(size, size, 0.1));
	b.Enlarge(root_point + point3(size, size, 0.1));
	return b;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool WaveSpot2::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// ����������� ���������� ����������
	texcoord_vector[0] = texcoord(1, 1);
	texcoord_vector[1] = texcoord(0, 1);
	texcoord_vector[2] = texcoord(0, 0);
	texcoord_vector[3] = texcoord(1, 0);
	// ����
	color = 0xffffffff;
	return true;
}

// ������� ����������� �����
void WaveSpot2::Multiply()
{
	ready_vector[0] = root_point + base*size;
	Quaternion qt;
	qt.FromAngleAxis(PId2, direction);
	ready_vector[1] = root_point + Normalize(qt*base)*size;
	qt.FromAngleAxis(PI, direction);
	ready_vector[2] = root_point + Normalize(qt*base)*size;
	qt.FromAngleAxis(3*PId2, direction);
	ready_vector[3] = root_point + Normalize(qt*base)*size;

	// ��������� ����
	color_vector[0] = color;
	color_vector[1] = color;
	color_vector[2] = color;
	color_vector[3] = color;
}


//
// �������, ����������� ������ ������ ������2
//

// ����������� � �����������
RotateSpotEffect2::RotateSpotEffect2(
									 const point3 Target,  // ����������� �����
									 const ROTATE_SPOT_EFFECT2_INFO& info
									 )
{
	start_point = Target;
	root_point = Target;
	vel = info.Vel;
	// ����� ����� �������
	lifetime = info.LifeTime;
	number = info.Number;
	radius_begin = info.RadiusBegin;
	radius_end = info.RadiusEnd;
	ang_vel = info.AngVel;
	size = info.Size;

	bbox.Degenerate();
	bbox.Enlarge(root_point + point3(-radius_end, -radius_end, -size));
	bbox.Enlarge(root_point + point3(radius_end, radius_end, size));

	// ������� �������
	ready_num = PEAK_NUM*number;
	color_vector = new unsigned int [ready_num];
	texcoord_vector = new texcoord [ready_num];
	ready_vector = new point3 [ready_num];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;
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
			sound_object = new FixedSound(info.Sound, root_point);
		}
	}
}

// ����������
RotateSpotEffect2::~RotateSpotEffect2()
{
	delete[] color_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// ����
	delete sound_object;
}

// ��������� ��� �������
// (������ ������ ���������)
bool RotateSpotEffect2::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if ( dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// ����� ����������� �����
	root_point = start_point + vel*dt;
	bbox.Enlarge(root_point + point3(-radius_end, -radius_end, -size));
	bbox.Enlarge(root_point + point3(radius_end, radius_end, size));
	// ���������� ������� ����� � ��������������
	Quaternion qt;
	point3 cur_radius;
	point3 cur_point;
	for(int i = 0; i < number; i++)
	{
		qt.FromAngleAxis(fmod(ang_vel*dt + PIm2*i/number, PIm2), point3(0, 0, 1.0));
		cur_radius = (qt*point3(1.0, 0, 0))*(radius_begin + (radius_end - radius_begin)*dt/lifetime);

		// ������ ����� ��������������
		qt.FromAngleAxis(PId4, Normalize(cur_radius));
		cur_point = root_point + cur_radius + (qt*point3(0, 0, 1.0))*size;
		ready_vector[i*PEAK_NUM + 1] = cur_point;
		ready_vector[i*PEAK_NUM + 3] = cur_point;
		
		// ������ ����� ��������������
		qt.FromAngleAxis(PId4 + PId2, Normalize(cur_radius));
		cur_point = root_point + cur_radius + (qt*point3(0, 0, 1.0))*size;
		ready_vector[i*PEAK_NUM + 4] = cur_point;
		
		// ������ ����� ��������������
		qt.FromAngleAxis(PId4 + PI, Normalize(cur_radius));
		cur_point = root_point + cur_radius + (qt*point3(0, 0, 1.0))*size;
		ready_vector[i*PEAK_NUM + 2] = cur_point;
		ready_vector[i*PEAK_NUM + 5] = cur_point;
		
		// �������� ����� ��������������
		qt.FromAngleAxis(PIm2 - PId4, Normalize(cur_radius));
		cur_point = root_point + cur_radius + (qt*point3(0, 0, 1.0))*size;
		ready_vector[i*PEAK_NUM] = cur_point;
	}

	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool RotateSpotEffect2::Update()
{
	// ����
	sound_object->Update(root_point, vel);
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool RotateSpotEffect2::Start(const float start_time)
{
	this->start_time = start_time;
	for(int i = 0; i < number; i++)
	{
		texcoord_vector[i*PEAK_NUM] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(0, 1);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, 1);
		color_vector[i*PEAK_NUM] = 0xffffffff;
		color_vector[i*PEAK_NUM + 1] = 0xffffffff;
		color_vector[i*PEAK_NUM + 2] = 0xffffffff;
		color_vector[i*PEAK_NUM + 3] = 0xffffffff;
		color_vector[i*PEAK_NUM + 4] = 0xffffffff;
		color_vector[i*PEAK_NUM + 5] = 0xffffffff;
	}
	return true;
}
