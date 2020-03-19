/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� �����
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "BombEffect.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������������ ���������� ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ��������� ��� �������
void ParticleSequence::NextTick(const float dt)
{
	float t;
	int c;
	
	for(int i = 0; i < SEQUENCE_LEN; i++)
	{
		// ����������
		t = (dt - dt/SEQUENCE_LEN*0.3*i);
		if(t<0) t = 0;
		sequence[SEQUENCE_LEN - i - 1].coords = start_point + velocity*t + accel*t*t*0.5;
		// ������
		sequence[SEQUENCE_LEN - i - 1].size = (lifetime-t*0.8)*size;
		// ����
		c = (lifetime - dt)*(255.0/lifetime);
		if(i == 0) sequence[SEQUENCE_LEN - i - 1].color = RGBA_MAKE(0xff, 0xff, 0xff, 0xff);
		else sequence[SEQUENCE_LEN - i - 1].color = RGBA_MAKE(c, c, c, c);
	}
}

// ������������������� ������������������
void ParticleSequence::Rebirth(
		const point3 st_point,         // ��������� �����
		const point3 stVelocity,       // ��������� ��������
		const point3 Accel,       // ���������
		const float stSize,	              // ��������� ������
		const float stRange,            // ��������� ����� ���������� �������
		const float Lifetime,             // ����� �����
		const bool bFull                  // ���� true, �� ������ �����, ����� - ��������
		)
{
	// ��� ��������� ���������
	float dx, dy, dz, a, b, r;

	// ���������� ������� �������
	coords = st_point;
	// ����� �������� ������� �������
	start_point = st_point;
	// ��������� ������� �������
	accel = Accel;
	// ����� �����
	lifetime = Lifetime;
	// �������� ������� �������
	a = ((float)rand())/32768.0 * PIm2; // ��������� ���� �����
	// ��������� ���� �����
	if(bFull) b = ((float)rand())/32768.0 * PIm2;
	else b = ((float)rand())/32768.0 * PI;
	r = (0.8 + 0.2*(float)rand()/32768.0); // ��������� ������
	dx = r*cos(b)*cos(a) * stVelocity.x;
	dz = r*sin(b)  * stVelocity.z;
	dy = r*cos(b)*sin(a)  * stVelocity.y;
	velocity = point3(dx, dy, dz);

	// ������, ���� � ��������� ���������� ������
	sequence[0].size = stSize + (float)rand()/32768.0*stRange;
	// �������������� ������
	size = sequence[0].size;
	sequence[0].color = 0xffa0a0a0;
	sequence[0].coords = st_point;
	for(int i = 1; i < SEQUENCE_LEN; i++)
	{
		sequence[i].size = sequence[0].size + 0.01;
		sequence[i].color = sequence[0].color + 0x00100000;
		sequence[i].coords = st_point;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������� ������������������� ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
BombSparkles::BombSparkles(
						   const point3 rp,            // �������� �����
						   const BOMB_SPARKLES_INFO& info
						   )
{
	vel = point3(0, 0, 0);
	// ������� ������ �������������������
	sequences_num = info.PartNum;
	sequences = new ParticleSequence[sequences_num];

	// �������� �������� �����, ������ � ���������� ���������
	points_num = sequences_num * SEQUENCE_LEN;
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

	start_velocity = info.Vel;          // ��������� ��������
	accel = info.Accel;                           // ���������
	start_size = info.Size0;                    // ��������� ������
	start_size_range = info.SizeRnd;       // ��������� ����� ���������� �������
	size_factor = info.SizeRange;            // ��������� �������
	full = info.Full;                  // ���� true, �� ������ �����, ����� - ��������

	lifetime = info.LifeTime;         // ������������ ����� �����
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
			sound_object = new FixedSound(info.Sound, rp);
		}
	}
}

// ����������
BombSparkles::~BombSparkles()
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

// ��������� ��� �������
// (������ ������ ���������)
bool BombSparkles::NextTick(const float current_time)
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

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool BombSparkles::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = this->prev_time = start_time;

	// ������������������� ������������������
	for(int i = 0; i < sequences_num; i++)
		sequences[i].Rebirth(root_point, start_velocity, accel, start_size, start_size_range, lifetime, full);

	// ����������� ���������� ����������
	for(int i = 0; i < sequences_num*SEQUENCE_LEN; i++)
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
bool BombSparkles::Update()
{
	// ����
	sound_object->Update(root_point, vel);
	// ����������� ������ ��� ������������� (points_vector)
	for(int i = 0; i < sequences_num; i++)
	{
		for(int j = 0; j < SEQUENCE_LEN; j++)
		{
			points_vector[i*SEQUENCE_LEN + j] = sequences[i].sequence[j].coords;
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
void BombSparkles::Multiply()
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
		for(j = 0; j < SEQUENCE_LEN; j++) // �� ������ � ������������������
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
BBox BombSparkles::GetBBox(const float current_time)
{
	BBox b;
	float dx, dy, dz, dt;
	dt = current_time - start_time;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	b.Degenerate();
	b.Enlarge(root_point + point3(dx, dy, dz));
	b.Enlarge(root_point - point3(dx, dy, dz));
	return b;
}
