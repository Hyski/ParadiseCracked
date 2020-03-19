/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ��� (������                                                                )
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "SmokeEffect.h"
#include "../../GameLevel/GameLevel.h"

//
// construction & destruction
//

// ����������� � �����������
SmokeEffect::SmokeEffect(
						 const float quality,            // ��������
						 const point3 rp,                // �������� �����
						 const SMOKE_EFFECT_INFO& info
		)
{
	turns_left = info.TurnsNum;
	reduce_time = info.ReduceTime;
	reduce = false;
	finished = false;
	// ������� ������ ������
	particles_num = 10 + info.MaxPartNum*quality;
	particles = new SmokeParticle[particles_num];

	// �������� �������� �����, ������ � ���������� ���������
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num*PEAK_NUM;

	// ������� �������
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// �����, �� ������� ���� ���
	root_point = rp;
	// ��������� ��������� ��������� ����� - ����� �������� ������
	start_point_factor = info.BirthRange;

	// ��������
	start_velocity = info.Vel;
	// ���������
	accel = info.Accel;
	// ����������� ��������� ������
	min_start_size = info.SizeBegin;
	// ������ ��������� ���������o �������
	size_range = info.SizeRnd;
	// ����������� ��������� ������� ������� � ����������� �� �������
	size_factor = info.SizeRange;
	// ������������ ����� ����� �������
	max_lifetime = info.LifeTime;
	m_exp = info.Exp;

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
SmokeEffect::~SmokeEffect()
{
	delete[] particles;
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// ����
	delete sound_object;
}

//
// ������ ������
//

// ������� ����������� �����
void SmokeEffect::Multiply()
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

	// ������� �������, ��� ��� ����� �����, � ���������� ������� ������ 
	// ����� ������� �������, � ������� ��� �����
	ready_num = ready_vector_size;
	// ������� ���������� �����, ������� ���������� � ������� �������
	k = 0;  
	for(int i = 0; i<points_num; i++)
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

		xsize = particles[i].size*(z*(n-f)+f)*xfactor;
		ysize = particles[i].size*(z*(n-f)+f)*yfactor;

		ready_vector[k*PEAK_NUM].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 1].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 2].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 3].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 4].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 5].x = points2d_vector[i].x - xsize;

		ready_vector[k*PEAK_NUM].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 1].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 2].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 3].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 4].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 5].y = points2d_vector[i].y + ysize;

		// ��������� ����
		color_vector[k*PEAK_NUM] = particles[i].color;
		color_vector[k*PEAK_NUM + 1] = particles[i].color;
		color_vector[k*PEAK_NUM + 2] = particles[i].color;
		color_vector[k*PEAK_NUM + 3] = particles[i].color;
		color_vector[k*PEAK_NUM + 4] = particles[i].color;
		color_vector[k*PEAK_NUM + 5] = particles[i].color;

		// ��������� ���������� ����������
		texcoord_vector[k*PEAK_NUM] = particles[i].texcoords1;
		texcoord_vector[k*PEAK_NUM + 1] = texcoord(particles[i].texcoords2.u, particles[i].texcoords1.v);
		texcoord_vector[k*PEAK_NUM + 2] = particles[i].texcoords2;
		texcoord_vector[k*PEAK_NUM + 3] = particles[i].texcoords1;
		texcoord_vector[k*PEAK_NUM + 4] = particles[i].texcoords2;
		texcoord_vector[k*PEAK_NUM + 5] = texcoord(particles[i].texcoords1.u, particles[i].texcoords2.v);

		// �������� ���������� ������������ �����
		k++;
	}
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool SmokeEffect::Start(const float start_time)
{
	// ������������������� �������
	for(int i = 0; i < particles_num; i++)
	{
		Rebirth(particles[i], start_time, 0.99);

		int sw = (float)rand()/32768.0 * 4;
		switch(sw)
		{
		case 0 : particles[i].texcoords1 = texcoord(0, 0); particles[i].texcoords2 = texcoord(0.5, 0.5); break;
		case 1 : particles[i].texcoords1 = texcoord(0.5, 0); particles[i].texcoords2 = texcoord(1, 0.5); break;
		case 2 : particles[i].texcoords1 = texcoord(0, 0.5); particles[i].texcoords2 = texcoord(0.5, 1); break;
		case 3 : particles[i].texcoords1 = texcoord(0.5, 0.5); particles[i].texcoords2 = texcoord(1, 1); break;
		}
	}

	return true;
}

// ��������� ��� �������
// (������ ������ ���������)
bool SmokeEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // ����
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� ������� (����� �����)
	
	// �������� �� ���� �� ������ ������
	if( (!turns_left) && (!reduce))
	{
		reduce = true;
		start_reduce_time = current_time;
	}

	// �������� �� ���� �� ��������� ������
	if(reduce && ((current_time - start_reduce_time) > reduce_time))
	{
		finished = true;
	}

	// ����������� ����������, ���� � ������ ��������
	for(int i = 0; i < particles_num; i++)
	{
		dt = current_time - particles[i].birth_time;
		if(dt > particles[i].life_time) // ������ ������� ������
		{
			Rebirth(particles[i], current_time, 0.7);
			dt = current_time - particles[i].birth_time;
		}
		// ����
		c = (particles[i].life_time - dt)*(255.0/particles[i].life_time);
		
		if(reduce && ((current_time - start_reduce_time) < reduce_time))
		{
			float dtt = current_time - start_reduce_time;
			c *= 1.0 - dtt/reduce_time;
		}
		particles[i].color = RGBA_MAKE(c, c, c, c);
		
		// ������
		if(m_exp) particles[i].size = min_start_size + size_factor*dt*dt*dt;
		else particles[i].size = min_start_size + dt*size_factor;

		particles[i].coords =root_point + particles[i].velocity*dt + accel*dt*dt*0.5;
	}
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool SmokeEffect::Update()
{
	// ����
	sound_object->Update(root_point, start_velocity);
	// ����������� ������ ��� ������������� (points_vector)
	for(int i = 0; i < particles_num; i++)
	{
		points_vector[i] = particles[i].coords;
	}
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(points_num,
		points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}

// ������� ����������� ������� ��� �� ��������
void SmokeEffect::Rebirth(SmokeParticle & sp, float curtime, float disp)
{
	// ��� ��������� ���������
	float dx, dy, dz, a, r;

	sp.birth_time = curtime;
	sp.color = 0xffffffff;

#define rnd (1 - (float)rand()/16384.0)*start_point_factor
	sp.coords = root_point + point3(rnd, rnd, rnd);
#undef rnd
	
	sp.size = min_start_size + (float)rand()/32768.0*size_range;
	a = ((float)rand())/32768.0 * PIm2; // ��������� ���� �����
	r = (0.8 + 0.2*(float)rand()/32768.0) * start_velocity.x; // ��������� ������
	dx = r*cos(a);
	dy = r*sin(a);
	dz = (1.0-disp+disp*(float)rand()/32768.0 )* start_velocity.z;
	sp.velocity = point3(dx, dy, dz);
	sp.life_time = (1.0-disp+disp*(float)rand()/32768.0) * max_lifetime;
}

// ���������� bounding box �������
BBox SmokeEffect::GetBBox(const float )
{
	BBox b;
	float dx, dy, dz, dt;
	dt = max_lifetime;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	b.maxx = root_point.x + dx;
	b.maxy = root_point.y + dy;
	b.maxz = root_point.z + dz;
	b.minx = root_point.x - dx;
	b.miny = root_point.y - dy;
	b.minz = root_point.z - dz;
	return b;
}


//
// ����� �������� ����
//


// ����������� � �����������
HeavySmokeEffect::HeavySmokeEffect(
								   const float quality,            // ��������
								   const point3 rp,                // �������� �����
								   const float st_point_factor,    // ������� ������ ��������� �����
								   const point3 stVelocity,        // ��������� ��������
								   const point3 Accel,             // ���������
								   const float stSIZE,             // ��������� ������
								   const float stRANGE,            // ��������� ����� ���������� �������
								   const float sizefactor,         // ������� �������
								   const float maxlifetime,        // ������������ ����� �����
								   const int MaxPartNum,           // ������������ ���������� ������
								   const std::string& Shader,      // ��� �������
								   const std::string& Sound,       // ��� ��������� �������
								   const bool SoundMove            // ��������� �� ����
		)
{
	// ������� ������ ������
	particles_num = 10 + MaxPartNum*quality;
	particles = new SmokeParticle[particles_num];

	// �������� �������� �����, ������ � ���������� ���������
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num*PEAK_NUM;

	// ������� �������
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// �����, �� ������� ���� ���
	root_point = rp;
	// ��������� ��������� ��������� ����� - ����� �������� ������
	start_point_factor = st_point_factor;

	// ��������
	start_velocity = stVelocity;
	// ���������
	accel = Accel;
	// ����������� ��������� ������
	min_start_size = stSIZE;
	// ������ ��������� ���������o �������
	size_range = stRANGE;
	// ����������� ��������� ������� ������� � ����������� �� �������
	size_factor = sizefactor;
	// ������������ ����� ����� �������
	max_lifetime = maxlifetime;

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


// ����������
HeavySmokeEffect::~HeavySmokeEffect()
{
	delete[] particles;
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// ����
	delete sound_object;
}

//
// ������ ������
//

// ������� ����������� �����
void HeavySmokeEffect::Multiply()
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

	// ������� �������, ��� ��� ����� �����, � ���������� ������� ������ 
	// ����� ������� �������, � ������� ��� �����
	ready_num = ready_vector_size;
	// ������� ���������� �����, ������� ���������� � ������� �������
	k = 0;  
	for(int i = 0; i<points_num; i++)
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

		xsize = particles[i].size*(z*(n-f)+f)*xfactor;
		ysize = particles[i].size*(z*(n-f)+f)*yfactor;

		ready_vector[k*PEAK_NUM].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 1].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 2].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 3].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 4].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 5].x = points2d_vector[i].x - xsize;

		ready_vector[k*PEAK_NUM].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 1].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 2].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 3].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 4].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 5].y = points2d_vector[i].y + ysize;

		// ��������� ����
		color_vector[k*PEAK_NUM] = particles[i].color;
		color_vector[k*PEAK_NUM + 1] = particles[i].color;
		color_vector[k*PEAK_NUM + 2] = particles[i].color;
		color_vector[k*PEAK_NUM + 3] = particles[i].color;
		color_vector[k*PEAK_NUM + 4] = particles[i].color;
		color_vector[k*PEAK_NUM + 5] = particles[i].color;

		// ��������� ���������� ����������
		texcoord_vector[k*PEAK_NUM] = particles[i].texcoords1;
		texcoord_vector[k*PEAK_NUM + 1] = texcoord(particles[i].texcoords2.u, particles[i].texcoords1.v);
		texcoord_vector[k*PEAK_NUM + 2] = particles[i].texcoords2;
		texcoord_vector[k*PEAK_NUM + 3] = particles[i].texcoords1;
		texcoord_vector[k*PEAK_NUM + 4] = particles[i].texcoords2;
		texcoord_vector[k*PEAK_NUM + 5] = texcoord(particles[i].texcoords1.u, particles[i].texcoords2.v);

		// �������� ���������� ������������ �����
		k++;
	}
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool HeavySmokeEffect::Start(const float start_time)
{
	// ����������� ���������� ����������
	for(int i = 0; i < particles_num; i++)
	{	
		Rebirth(particles[i], start_time, 0.99);

		int sw = (float)rand()/32768.0 * 4;
		switch(sw)
		{
		case 0 : particles[i].texcoords1 = texcoord(0, 0); particles[i].texcoords2 = texcoord(0.5, 0.5); break;
		case 1 : particles[i].texcoords1 = texcoord(0.5, 0); particles[i].texcoords2 = texcoord(1, 0.5); break;
		case 2 : particles[i].texcoords1 = texcoord(0, 0.5); particles[i].texcoords2 = texcoord(0.5, 1); break;
		case 3 : particles[i].texcoords1 = texcoord(0.5, 0.5); particles[i].texcoords2 = texcoord(1, 1); break;
		}
	}

	return true;
}

// ��������� ��� �������
// (������ ������ ���������)
bool HeavySmokeEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // ����
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� ������� (����� �����)
	// ����������� ����������, ���� � ������ ��������
	for(int i = 0; i < particles_num; i++)
	{
		dt = current_time - particles[i].birth_time;
		if(dt > particles[i].life_time) // ������ ������� ������
		{
			Rebirth(particles[i], current_time, 0.7);
			dt = current_time - particles[i].birth_time;
		}
		// ����
		c = (particles[i].life_time - dt)*(255.0/particles[i].life_time);
		particles[i].color = RGBA_MAKE(c, c, c, c);
		
		// ������
		particles[i].size = min_start_size + dt*size_factor;

		// ���������� 
		particles[i].coords =root_point + particles[i].velocity*dt + accel*dt*dt*0.5;
	}
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool HeavySmokeEffect::Update()
{
	// ����
	sound_object->Update(root_point, start_velocity);
	// ����������� ������ ��� ������������� (points_vector)
	for(int i = 0; i < particles_num; i++)
	{
		points_vector[i] = particles[i].coords;
	}
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(points_num,
		points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}

// ������� ����������� ������� ��� �� ��������
void HeavySmokeEffect::Rebirth(SmokeParticle & sp, float curtime, float disp)
{
	// ��� ��������� ���������
	float dx, dy, dz, a, r;

	sp.birth_time = curtime;
	sp.color = 0xffffffff;

#define rnd (1 - (float)rand()/16384.0)*start_point_factor
	sp.coords = root_point + point3(rnd, rnd, rnd);
#undef rnd
	
	sp.size = min_start_size + (float)rand()/32768.0*size_range;
	a = ((float)rand())/32768.0 * PIm2; // ��������� ���� �����
	r = (0.8 + 0.2*(float)rand()/32768.0) * start_velocity.x; // ��������� ������
	dx = r*cos(a);
	dy = r*sin(a);
	dz = (1.0-disp+disp*(float)rand()/32768.0 )* start_velocity.z;
	sp.velocity = point3(dx, dy, dz);
	sp.life_time = (1.0-disp+disp*(float)rand()/32768.0) * max_lifetime;
}

// ���������� bounding box �������
BBox HeavySmokeEffect::GetBBox(const float )
{
	BBox b;
	float dx, dy, dz, dt;
	dt = max_lifetime;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	b.maxx = root_point.x + dx;
	b.maxy = root_point.y + dy;
	b.maxz = root_point.z + dz;
	b.minx = root_point.x - dx;
	b.miny = root_point.y - dy;
	b.minz = root_point.z - dz;
	return b;
}

//
// ����� ���
//

// ����������� � �����������
FlyEffect::FlyEffect(
					 const float quality,            // ��������
					 const point3 rp,                // �������� �����
					 const FLY_EFFECT_INFO& info
					 )
{
	// ������� ������ ������
	particles_num = 2 + info.MaxPartNum*quality;
	particles = new FlyParticle[particles_num];

	// �������� �������� �����, ������ � ���������� ���������
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num*PEAK_NUM;

	// ������� �������
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// ����������� �����
	root_point = rp;
	// ����������� ��������� ������
	min_start_size = info.Size;
	// ������ ��������� ���������o �������
	size_range = info.SizeRnd;

	min_radius = info.MinRadius;
	radius_range = info.RadiusRange;
	min_vel = info.MinVel;

	vel = point3(0.0, 0.0, 0.0);

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
FlyEffect::~FlyEffect()
{
	delete[] particles;
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
bool FlyEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dtt = current_time - previous_time;
	for(int i = 0; i < particles_num; i++)
	{
		particles[i].alpha = fmod(particles[i].alpha + dtt*particles[i].vel.x, PIm2);
		particles[i].betta = fmod(particles[i].betta + dtt*particles[i].vel.y, PIm2);
		particles[i].gamma = fmod(particles[i].gamma + dtt*particles[i].vel.z, PIm2);
		//particles[i].r = particles[i].radius + 0.5*sin(particles[i].gamma);
		particles[i].r = particles[i].radius*sin(particles[i].gamma);
		particles[i].setCoords(root_point);
	}
	previous_time = current_time;
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool FlyEffect::Update()
{
	// ����
	sound_object->Update(root_point, vel);
	// ����������� ������ ��� ������������� (points_vector)
	for(int i = 0; i < particles_num; i++)
	{
		points_vector[i] = particles[i].coords;
	}
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(points_num,
		points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}

// ���������� bounding box �������
BBox FlyEffect::GetBBox(const float )
{
	return bb;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool FlyEffect::Start(const float start_time)
{
	previous_time = start_time;
	// ������������������� �������
	for(int i = 0; i < particles_num; i++)
	{
		particles[i].size = min_start_size + size_range * (float) rand()/32768.0f;

		particles[i].radius = min_radius + radius_range * (float)rand()/32768.0;
		particles[i].alpha = PIm2 * (float)rand()/32768.0;
		particles[i].betta = PIm2 * (float)rand()/32768.0;
		particles[i].gamma = PIm2 * (float)rand()/32768.0;

		particles[i].vel.x = min_vel.x + 2.0*(float)rand()/32768.0;
		if(rand() > 16384) particles[i].vel.x = -particles[i].vel.x;
		particles[i].vel.y = min_vel.y + 2.0*(float)rand()/32768.0;
		if(rand() > 16384) particles[i].vel.y = -particles[i].vel.y;
		particles[i].vel.z = min_vel.z + 2.0*(float)rand()/32768.0;
		if(rand() > 16384) particles[i].vel.z = -particles[i].vel.z;

		color_vector[i*PEAK_NUM] = 0xffffffff;
		color_vector[i*PEAK_NUM + 1] = 0xffffffff;
		color_vector[i*PEAK_NUM + 2] = 0xffffffff;
		color_vector[i*PEAK_NUM + 3] = 0xffffffff;
		color_vector[i*PEAK_NUM + 4] = 0xffffffff;
		color_vector[i*PEAK_NUM + 5] = 0xffffffff;

		texcoord_vector[i*PEAK_NUM] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, 1);
	}
	
	bb.Degenerate();
	bb.Enlarge(root_point + point3(min_radius + radius_range, min_radius + radius_range, min_radius + radius_range));
	bb.Enlarge(root_point - point3(min_radius + radius_range, min_radius + radius_range, min_radius + radius_range));

	return true;
}

// ������� ����������� �����
void FlyEffect::Multiply()
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

	// ������� �������, ��� ��� ����� �����, � ���������� ������� ������ 
	// ����� ������� �������, � ������� ��� �����
	ready_num = ready_vector_size;
	// ������� ���������� �����, ������� ���������� � ������� �������
	k = 0;  
	for(int i = 0; i<points_num; i++)
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

		xsize = particles[i].size*(z*(n-f)+f)*xfactor;
		ysize = particles[i].size*(z*(n-f)+f)*yfactor;

		ready_vector[k*PEAK_NUM].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 1].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 2].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 3].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 4].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 5].x = points2d_vector[i].x - xsize;

		ready_vector[k*PEAK_NUM].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 1].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 2].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 3].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 4].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 5].y = points2d_vector[i].y + ysize;

			// �������� ���������� ������������ �����
		k++;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ��� � ��������, �������� �� ��������
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����������� � �����������
FlyCylinderEffect::FlyCylinderEffect(
									 const float quality,            // ��������
									 const point3 rp,                // �������� �����
									 const FLY_CYLINDER_EFFECT_INFO& info
									 )
{
	// ��������� ��������� �������
	m_height = info.Height;
	m_particles_num = 5 + info.MaxPartNum*quality;
	m_deltat = info.PartDeltat;
	m_part_size = info.PartSize;
	m_radius = info.Radius;
	m_tail_part_num = info.TailPartNum;
	m_velh = info.VelH;
	m_velalpha = info.VelAlpha;
	m_color = info.Color;

	// �������� �������� �����, ������ � ���������� ���������
	points_num = m_particles_num*(m_tail_part_num + 1);
	ready_vector_size = points_num*PEAK_NUM;
	ready_num = 0; // ����� ����������� ��������

	// ������� �������
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// �������� �����
	m_root = rp;

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
	// �������� ��� �����
	m_vel = point3(0.0f, 0.0f, 0.0f);
}

// ����������
FlyCylinderEffect::~FlyCylinderEffect()
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
bool FlyCylinderEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dtt = current_time - m_previous_time;
	m_not_delayed_particles.clear();
	for(unsigned int i = 0; i < m_particles.size(); i++)
	{
		// �������� ��������
		if(m_particles[i].delay > 0.0f)
		{
			// ������� ��������
			m_particles[i].delay -= dtt;
			// �������� � ��������� �������
			continue;
		}
		// ���������� ���������
		m_particles[i].h += m_particles[i].vh*dtt;
		m_particles[i].alpha = fmod(m_particles[i].alpha + dtt*m_particles[i].valpha, PIm2);
		// ���� �������� ����� ��� ���� - ������� ��������
		// �� ������ �� ��������������� � ��������� ���������� �� ������
		if( (m_particles[i].h > m_height) &&
			(m_particles[i].vh > 0.0f)
			)
		{
			m_particles[i].vh *= -1.0f;
			m_particles[i].h -= (m_particles[i].h - m_height)*2.0f;
		}
		if( (m_particles[i].h < 0.0f) &&
			(m_particles[i].vh < 0.0f)
			)
		{
			m_particles[i].vh *= -1.0f;
			m_particles[i].h -= m_particles[i].h*2.0f;
		}
		m_particles[i].setCoords(m_root, m_radius);
		// ������� � ������ �������
		m_not_delayed_particles.push_back(m_particles[i]);
	}
	// �������� ����� �� ����� ��������
	if(m_mark_time <= 0.0f)
	{
		// �������� ������� �����
		DMark *dmk = new DMark(m_root, m_radius*2.0f,
			"dynlight",
			m_color,
			Timer::GetSeconds(), 5.0f,
			2.0, 0);
		
		IWorld::Get()->GetLevel()->AddMark(dmk);
		m_mark_time += 2.5f;
	}
	else
	{
		m_mark_time -= dtt;
	}
	m_previous_time = current_time;
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool FlyCylinderEffect::Update()
{
	// ����
	sound_object->Update(m_root, m_vel);
	// ����������� ������ ��� ������������� (points_vector)
	for(unsigned int i = 0; i < m_not_delayed_particles.size(); i++)
	{
		points_vector[i] = m_not_delayed_particles[i].coords;
	}
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(m_not_delayed_particles.size(),
		points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool FlyCylinderEffect::Start(const float start_time)
{
	m_particles.reserve(points_num);
	m_not_delayed_particles.reserve(points_num);
	m_previous_time = start_time;
	m_mark_time = 0.0f;
	// ������������������� �������
	for(unsigned int i = 0; i < m_particles_num; i++)
	{
		// �������������� �������� �������
		FlyCylinderParticle main_particle;
		main_particle.alpha = EffectUtility::get0F(PIm2);
		main_particle.h = EffectUtility::get0F(m_height);
		main_particle.size = m_part_size;
		main_particle.setCoords(m_root, m_radius);
		main_particle.color = 0xffffffff;
		main_particle.vh = m_velh + EffectUtility::get_FF(0.2f*m_velh);
		if(EffectUtility::get0F() > 0.5f) main_particle.vh *= -1.0f;
		main_particle.valpha = m_velalpha + EffectUtility::get_FF(0.3f*m_velalpha);
		if(EffectUtility::get0F() > 0.5f) main_particle.valpha *= -1.0f;
		main_particle.delay = 0;
		// ������� �� � ������
		m_particles.push_back(main_particle);
		// ����������������� � ������� ������� ������
		for(unsigned int j = 0; j < m_tail_part_num; j++)
		{
			// �������������� ��������� �������
			FlyCylinderParticle particle = main_particle;
			particle.size *= 1.0f - static_cast<float>(j + 1)/m_tail_part_num;
			particle.color *= 1.0f - static_cast<float>(j + 1)/m_tail_part_num;
			particle.delay = m_deltat*(j + 1);
			// ������� �� � ������
			m_particles.push_back(particle);
		}
	}
	// ����������������� ���������� ����������
	for(int i = 0; i < points_num; i++)
	{
		texcoord_vector[i*PEAK_NUM] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, 1);
	}
	// ����������������� bbox
	m_bbox.Degenerate();
	m_bbox.Enlarge(m_root - point3(m_radius, m_radius, 0.0f));
	m_bbox.Enlarge(m_root + point3(m_radius, m_radius, m_height));
	return true;
}

// ������� ����������� �����
void FlyCylinderEffect::Multiply()
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

	// ������� �������, ��� ��� ����� �����, � ���������� ������� ������ 
	// ����� ������� �������, � ������� ��� �����
	ready_num = m_not_delayed_particles.size()*PEAK_NUM;
	// ������� ���������� �����, ������� ���������� � ������� �������
	k = 0;  
	for(unsigned int i = 0; i<m_not_delayed_particles.size(); i++)
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

		xsize = m_not_delayed_particles[i].size*(z*(n-f)+f)*xfactor;
		ysize = m_not_delayed_particles[i].size*(z*(n-f)+f)*yfactor;

		ready_vector[k*PEAK_NUM].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 1].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 2].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 3].x = points2d_vector[i].x - xsize;
		ready_vector[k*PEAK_NUM + 4].x = points2d_vector[i].x + xsize;
		ready_vector[k*PEAK_NUM + 5].x = points2d_vector[i].x - xsize;

		ready_vector[k*PEAK_NUM].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 1].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 2].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 3].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 4].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 5].y = points2d_vector[i].y + ysize;

		// ����
		color_vector[k*PEAK_NUM] = m_not_delayed_particles[i].color;
		color_vector[k*PEAK_NUM + 1] = m_not_delayed_particles[i].color;
		color_vector[k*PEAK_NUM + 2] = m_not_delayed_particles[i].color;
		color_vector[k*PEAK_NUM + 3] = m_not_delayed_particles[i].color;
		color_vector[k*PEAK_NUM + 4] = m_not_delayed_particles[i].color;
		color_vector[k*PEAK_NUM + 5] = m_not_delayed_particles[i].color;
		// �������� ���������� ������������ �����
		k++;
	}
}

