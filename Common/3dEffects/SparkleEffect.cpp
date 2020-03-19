/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ ������������ ����
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "SparkleEffect.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������������ ���������� ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ��������� ��� �������

void SparkleSequence::NextTick(const float dt)
{
	float t;
	int c;
	
	for(int i = 0; i < SEQUENCE_LEN1; i++)
	{
		// ����������
		t = (dt - dt/SEQUENCE_LEN1*0.3*i);
		if(t<0) t = 0;
		sequence[SEQUENCE_LEN1 - i - 1].coords = start_point + velocity*t + accel*t*t*0.5;
		// ������
		sequence[SEQUENCE_LEN1 - i - 1].size = (lifetime-t*0.8)*size;
		// ����
		c = (lifetime - dt)*(255.0/lifetime);
		if(i == 0) sequence[SEQUENCE_LEN1 - i - 1].color = RGBA_MAKE(0xff, 0xff, 0xff, 0xff);
		else sequence[SEQUENCE_LEN1 - i - 1].color = RGBA_MAKE(c, c, c, c);
	}
}


// ������������������� ������������������
void SparkleSequence::Rebirth(
							  const point3 st_point,      // ��������� �����
							  const point3 vecVelocity,   // ������ ����������� ��������
							  const point3 DV,            // ������� ��������
							  const point3 Accel,         // ���������
							  const float stSize,	      // ��������� ������
							  const float stRange,        // ��������� ����� ���������� �������
							  const float Lifetime        // ����� �����
		)
{
	// ��� ��������� ���������
	float dx, dy, dz; //, a, b, r;
	point3 dv;

	// ���������� ������� �������
	coords = st_point;
	// ����� �������� ������� �������
	start_point = st_point;
	// ��������� ������� �������
	accel = Accel;
	// ����� �����
	lifetime = Lifetime;

	dv =DV;
	dx = (1 - 2.0*(float)rand()/32768.0);
	dy = (1 - 2.0*(float)rand()/32768.0);
	dz = (1 - 2.0*(float)rand()/32768.0);
	dv.x *= dx;
	dv.y *= dy;
	dv.z *= dz;
	velocity = vecVelocity + dv;
	// ������, ���� � ��������� ���������� ������
	sequence[0].size = stSize + (float)rand()/32768.0*stRange;
	// �������������� ������
	size = sequence[0].size;
	sequence[0].color = 0xffa0a0a0;
	sequence[0].coords = st_point;
	for(int i = 1; i < SEQUENCE_LEN1; i++)
	{
		sequence[i].size = sequence[0].size + 0.01;
		sequence[i].color = sequence[0].color + 0x00100000;
		sequence[i].coords = st_point;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������ ����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ��������� ��� �������
// (������ ������ ���������)
bool OrientedSparkles::NextTick(const float current_time)
{
	
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;
	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	for(int i = 0; i < sequences_num; i++) sequences[i].NextTick(dt);
	return true;
}

// ����������
OrientedSparkles::~OrientedSparkles()
{
	delete[] sequences;
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// ����
	delete sound_object;
}

// ����������� � �����������
OrientedSparkles::OrientedSparkles(
								   const float quality,        // ��������
								   const point3 rp,            // �������� �����
								   const point3 vecVelocity,   // ������ ����������� ��������
								   const point3 DV,            // ������� ��������
								   const point3 Accel,         // ���������
								   const float stSize,	       // ��������� ������
								   const float stRange,        // ��������� ����� ���������� �������
								   const float sizeFactor,     // ��������� �������
								   const float LifeTime,       // ����� ����� �������
								   const std::string& Shader,  // ��� �������
								   const std::string& Sound,   // ��� ��������� �������
								   const bool SoundMove        // ��������� �� ����
		)
{
	// ������� ������ �������������������
	sequences_num = 10*quality;
	if(!sequences_num) sequences_num = 1;
	sequences = new SparkleSequence[sequences_num];

	// �������� �������� �����, ������ � ���������� ���������
	points_num = sequences_num * SEQUENCE_LEN1;
	ready_vector_size = ready_num = points_num*PEAK_NUM;

	// ������� �������
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// �������� �����
	root_point = rp;

	vec_velocity = vecVelocity;        // ������ ����������� ��������
	dv = DV,                                  // ������� ��������
	accel = Accel;                           // ���������
	start_size = stSize;                    // ��������� ������
	start_size_range = stRange;       // ��������� ����� ���������� �������
	size_factor = sizeFactor;            // ��������� �������

	lifetime = LifeTime;         // ������������ ����� �����

	// ������
	SetShader(Shader);
	// ����
	if(Sound == "")
	{
		// ����� ���
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
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool OrientedSparkles::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = this->prev_time = start_time;

	// ������������������� ������������������
	for(int i = 0; i < sequences_num; i++)
		sequences[i].Rebirth(root_point, vec_velocity, dv, accel, start_size, start_size_range, lifetime);

	// ����������� ���������� ����������
	for(int i = 0; i < sequences_num*SEQUENCE_LEN1; i++)
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

// ���������� ������� ������� ����� � ������� ������ �����
bool OrientedSparkles::Update()
{
	// ����
	sound_object->Update(root_point, vec_velocity);
	// ����������� ������ ��� ������������� (points_vector)
	for(int i = 0; i < sequences_num; i++)
	{
		for(int j = 0; j < SEQUENCE_LEN1; j++)
		{
			points_vector[i*SEQUENCE_LEN1 + j] = sequences[i].sequence[j].coords;
		}
	}
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(points_num,
		points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}

// ������� ����������� �����
void OrientedSparkles::Multiply()
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

	// ������� �������, ��� ��� ����� �����, � ���������� ������� ������ 
	// ����� ������� �������, � ������� ��� �����
	ready_num = ready_vector_size;
	// ������� ���������� �����, ������� ���������� � ������� �������
	k = 0;
	// ������� ����� ����� � ������� ����� �������������
	l = 0; 
	for(i = 0; i < sequences_num; i++) // �� �������������������
	{
		for(j = 0; j < SEQUENCE_LEN1; j++) // �� ������ � ������������������
		{
			// ����������� ������
			z  = points2d_vector[l].z;
			if(z<0)
			{
				// ��� ����� �� ����� ���������� - ��������� ��
				// �������� ���������� ������� ������
				ready_num -= PEAK_NUM;
				// �������� ����� � ������� ����� ����� �������������
				l++;
				continue;
			}
			// ��������� ����������
			ready_vector[k*PEAK_NUM].z = z;
			ready_vector[k*PEAK_NUM + 1].z = z;
			ready_vector[k*PEAK_NUM + 2].z = z;
			ready_vector[k*PEAK_NUM + 3].z = z;
			ready_vector[k*PEAK_NUM + 4].z = z;
			ready_vector[k*PEAK_NUM + 5].z = z;

			xsize = sequences[i].sequence[j].size*(z*(n-f)+f)*xfactor;
			ysize = sequences[i].sequence[j].size*(z*(n-f)+f)*yfactor;

		
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
		
			// ��������� ����
			color_vector[k*PEAK_NUM] = sequences[i].sequence[j].color;
			color_vector[k*PEAK_NUM + 1] = sequences[i].sequence[j].color;
			color_vector[k*PEAK_NUM + 2] = sequences[i].sequence[j].color;
			color_vector[k*PEAK_NUM + 3] = sequences[i].sequence[j].color;
			color_vector[k*PEAK_NUM + 4] = sequences[i].sequence[j].color;
			color_vector[k*PEAK_NUM + 5] = sequences[i].sequence[j].color;
		
			// �������� ���������� ������������ �����
			k++;
			// �������� ����� � ������� ����� ����� �������������
			l++;
		}
	}
}

// ���������� bounding box �������
BBox OrientedSparkles::GetBBox(const float current_time)
{
	BBox b;
	float dt;
	point3 dxyz;
	b.Degenerate();
	dt = current_time - start_time;
	
	dxyz = (vec_velocity + dv)*dt + accel*dt*dt*0.5;

	b.Enlarge(root_point + dxyz);
	b.Enlarge(root_point - dxyz);

	dxyz = (vec_velocity - dv)*dt + accel*dt*dt*0.5;

	b.Enlarge(root_point + dxyz);
	b.Enlarge(root_point - dxyz);
	/*
	b.maxx = root_point.x + dx;
	b.maxy = root_point.y + dy;
	b.maxz = root_point.z + dz;
	b.minx = root_point.x - dx;
	b.miny = root_point.y - dy;
	b.minz = root_point.z - dz;*/
	return b;
}
