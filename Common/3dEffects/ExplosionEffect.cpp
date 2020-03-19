/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� (������)
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "ExplosionEffect.h"



//
// ����� ������
//


// ���������� ������� ������� ����� � ������� ������ �����
bool ExplosionEffect::Update()
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



// ������� ����������� �����
void ExplosionEffect::Multiply()
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

		ready_vector[k*PEAK_NUM].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 1].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 2].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 3].y = points2d_vector[i].y + ysize;
		ready_vector[k*PEAK_NUM + 4].y = points2d_vector[i].y - ysize;
		ready_vector[k*PEAK_NUM + 5].y = points2d_vector[i].y - ysize;

		// ��������� ����
		color_vector[k*PEAK_NUM] = particles[i].color;
		color_vector[k*PEAK_NUM + 1] = particles[i].color;
		color_vector[k*PEAK_NUM + 2] = particles[i].color;
		color_vector[k*PEAK_NUM + 3] = particles[i].color;
		color_vector[k*PEAK_NUM + 4] = particles[i].color;
		color_vector[k*PEAK_NUM + 5] = particles[i].color;

		// �������� ���������� ������������ �����
		k++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������ �������
//////////////////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
GrenadeEffect::GrenadeEffect(const float quality,        // ��������
							 const point3 rp,            // �������� �����
							 const GRENADE_EFFECT_INFO& info
							 )
{
	// ������� ������ ������
	particles_num =  1 + 8*quality;
	particles = new ExplosionParticle[particles_num];

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

	start_velocity = info.Vel;          // ��������� ��������
	accel = info.Accel;                           // ���������
	start_size = info.Size0;                    // ��������� ������
	start_size_range = info.SizeRnd;       // ��������� ����� ���������� �������
	size_factor = info.SizeRange;            // ��������� �������

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
GrenadeEffect::~GrenadeEffect()
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
bool GrenadeEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // ����
	float s; // ������
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;
	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	c = (lifetime - dt)*(255.0/lifetime);
	c = RGBA_MAKE(c, (int)(c*0.7), (int)(c*0.7), (int)(c*0.7));

	s = start_size + sqrt(dt)*size_factor;

	// ����������� ����������, ���� � ������
	for(int i = 0; i < particles_num; i++)
	{
		// ����
		particles[i].color = c;
		
		// ������
		particles[i].size = s;

		// ���������� 
		particles[i].coords =  particles[i].start_point + particles[i].velocity*dt + accel*dt*dt/2.0;
	}
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool GrenadeEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// ������������������� �������
	for(int i = 0; i < particles_num; i++)	Rebirth(particles[i], start_time);

	// ����������� ���������� ����������
	for(int i = 0; i < particles_num; i++)
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

// ������� ����������� ������� ��� �� ��������
void GrenadeEffect::Rebirth(ExplosionParticle & ep, float )
{
	// ��� ��������� ���������
	float dx, dy, dz, a, b, r;
	// ��������� ����� ��������
	dx = (0.5 - (float)rand()/32768.0) * 0.9;
	dy = (0.5 - (float)rand()/32768.0) * 0.9;
	dz = (0.5 - (float)rand()/32768.0) * 0.5;
	ep.coords = root_point + point3(dx, dy, dz);
	ep.start_point = root_point + point3(dx, dy, dz);
	a = ((float)rand())/32768.0 * PIm2; // ��������� ���� �����
	b = ((float)rand())/32768.0 * PId2; // ��������� ���� �����

	ep.size = start_size + (float)rand()/32768.0*start_size_range;
	r = (0.8 + 0.2*(float)rand()/32768.0); // ��������� ������
	dx = r*cos(b)*cos(a) * start_velocity.x;
	dz = r*sin(b)  * start_velocity.z;
	dy = r*cos(b)*sin(a)  * start_velocity.y;
	// ����
	ep.color = 0xffffa0a0;
	ep.velocity = point3(dx, dy, dz);
}

// ���������� bounding box �������
BBox GrenadeEffect::GetBBox(const float current_time)
{
	BBox b;
	float dx, dy, dz, dt, s;
	dt = current_time - start_time;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	s = start_size + sqrt(dt)*size_factor;
	b.maxx = root_point.x + dx + s;
	b.maxy = root_point.y + dy + s;
	b.maxz = root_point.z + dz + s;
	b.minx = root_point.x - dx - s;
	b.miny = root_point.y - dy - s;
	b.minz = root_point.z - dz - s;
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������� ���� ��� ��������
//////////////////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
SparkleEffect::SparkleEffect(
							 const point3 rp,            // �������� �����
							 const SPARKLE_EFFECT_INFO& info
	)
{
	// ������� ������ ������
	particles_num = info.PartNum;
	particles = new ExplosionParticle[particles_num];

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

	// �������� �����
	root_point = rp;

	start_velocity = info.Vel;          // ��������� ��������
	accel = info.Accel;                           // ���������
	start_size = info.Size0;                    // ��������� ������
	start_size_range = info.SizeRnd;       // ��������� ����� ���������� �������
	size_factor = info.SizeRange;            // ��������� �������

	lifetime = info.LifeTime;         // ������������ ����� �����
	full = info.Full;                    // ���� true, �� ������ �����, ����� - ��������
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
SparkleEffect::~SparkleEffect()
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
bool SparkleEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // ����
	float s; // ������
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	c = (lifetime - dt)*(255.0/lifetime);
	c = RGBA_MAKE(0xff, c, c, c);
	s = start_size + dt*size_factor;

	// ����������� ����������, ���� � ������
	for(int i = 0; i < particles_num; i++)
	{
		// ����
		particles[i].color = c;
		
		// ������
		particles[i].size = s;

		// ���������� 
		particles[i].coords = root_point + particles[i].velocity*dt + accel*dt*dt/2.0;
	}

	// ����� ���������� �����
	prev_time = current_time;
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool SparkleEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = this->prev_time = start_time;

	// ������������������� �������
	for(int i = 0; i < particles_num; i++)	Rebirth(particles[i], start_time);

	// ����������� ���������� ����������
	for(int i = 0; i < particles_num; i++)
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

// ������� ����������� ������� ��� �� ��������
void SparkleEffect::Rebirth(ExplosionParticle & ep, float )
{
	// ��� ��������� ���������
	float dx, dy, dz, a, b, r;
	ep.coords = root_point;
	a = ((float)rand())/32768.0 * PIm2; // ��������� ���� �����
	// ��������� ���� �����
	if(full) b = ((float)rand())/32768.0 * PIm2;
	else b = ((float)rand())/32768.0 * PI;

	ep.size = start_size + (float)rand()/32768.0*start_size_range;
	r = (0.8 + 0.2*(float)rand()/32768.0); // ��������� ������
	dx = r*cos(b)*cos(a) * start_velocity.x;
	dz = r*sin(b)  * start_velocity.z;
	dy = r*cos(b)*sin(a)  * start_velocity.y;
	// ����
	ep.color = 0xffffffff;
	ep.velocity = point3(dx, dy, dz);
}

// ���������� bounding box �������
BBox SparkleEffect::GetBBox(const float current_time)
{
	BBox b;

	float dx, dy, dz, dt;
	dt = current_time - start_time;
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

/////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ���� ��� �������
/////////////////////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
ExplosionSmokeEffect::ExplosionSmokeEffect(const float quality,        // ��������
										   const point3 rp,            // �������� �����
										   const EXPLOSION_SMOKE_INFO& info
	)
{
	// ������� ������ ������
	particles_num = 1 + 4*quality;
	particles = new ExplosionParticle[particles_num];

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

	start_velocity = info.Vel;          // ��������� ��������
	accel = info.Accel;                           // ���������
	start_size = info.Size0;                    // ��������� ������
	start_size_range = info.SizeRnd;       // ��������� ����� ���������� �������
	size_factor = info.SizeRange;            // ��������� �������

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
ExplosionSmokeEffect::~ExplosionSmokeEffect()
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
bool ExplosionSmokeEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // ����
	float s; // ������
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - this->start_time;
	if (dt >= this->lifetime)
	{
		finished = true;
		return true;
	}

	c = (lifetime - dt)*(255.0/lifetime);
	c = RGBA_MAKE(c, (int)(c*0.7), (int)(c*0.7), c);
	s = start_size + sqrt(dt)*size_factor;

	// ����������� ����������, ���� � ������
	for(int i = 0; i < particles_num; i++)
	{
		// ����
		particles[i].color = c;
		
		// ������
		particles[i].size = s;

		// ���������� 
		particles[i].coords =  particles[i].start_point + particles[i].velocity*dt + accel*dt*dt/2.0;
	}

	// ����� ���������� �����
	prev_time = current_time;
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool ExplosionSmokeEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = this->prev_time = start_time;

	// ������������������� �������
	for(int i = 0; i < particles_num; i++)	Rebirth(particles[i], start_time);

	// ����������� ���������� ����������
	for(int i = 0; i < particles_num; i++)
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

// ������� ����������� ������� ��� �� ��������
void ExplosionSmokeEffect::Rebirth(ExplosionParticle & ep, float )
{
	// ��� ��������� ���������
	float dx, dy, dz, a, b, r;
	// ��������� ����� ��������
	dx = (0.5 - (float)rand()/32768.0) * 0.9;
	dy = (0.5 - (float)rand()/32768.0) * 0.9;
	dz = (0.5 - (float)rand()/32768.0) * 0.5;
	//ep.coords = root_point;// + point3(dx, dy, dz);
	ep.start_point = root_point;// + point3(dx, dy, dz);
	a = ((float)rand())/32768.0 * PIm2; // ��������� ���� �����
	b = ((float)rand())/32768.0 * PId2; // ��������� ���� �����

	ep.size = start_size + (float)rand()/32768.0*start_size_range;
	r = (0.8 + 0.2*(float)rand()/32768.0); // ��������� ������
	dx = r*cos(b)*cos(a) * start_velocity.x;
	dz = r*sin(b)  * start_velocity.z;
	dy = r*cos(b)*sin(a)  * start_velocity.y;
	// ����
	ep.color = 0xffffa0a0;
	ep.velocity = point3(dx, dy, dz);
}

// ���������� bounding box �������
BBox ExplosionSmokeEffect::GetBBox(const float current_time)
{
	BBox b;
	float dx, dy, dz, dt, s;
	dt = current_time - start_time;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	s = start_size + sqrt(dt)*size_factor;
	b.maxx = root_point.x + dx + s;
	b.maxy = root_point.y + dy + s;
	b.maxz = root_point.z + dz + s;
	b.minx = root_point.x - dx - s;
	b.miny = root_point.y - dy - s;
	b.minz = root_point.z - dz - s;
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������ ���� ��� �������
/////////////////////////////////////////////////////////////////////////////////////////////////

bool ExplosionSmokeRing::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // ����
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// ����
	c = (lifetime - dt)*(255.0/lifetime);
	color = RGBA_MAKE(c, (int)(c*0.7), (int)(c*0.7), c);
	for(int i = 0; i < points_num; i++) color_vector[i] = color;
		
	// ������
	size = start_size + sqrt(dt)*size_factor;
	
	// ���������� ������
	ready_vector[0] = root_point + point3(size, 0, 0);
	ready_vector[1] = root_point + point3(0, size, 0);
	ready_vector[2] = root_point + point3(-size, 0, 0);
	ready_vector[3] = root_point + point3(0, -size, 0);
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool ExplosionSmokeRing::Update()
{
	// ����
	sound_object->Update(root_point, start_velocity);
	return true;
}

// ����������� � �����������
ExplosionSmokeRing::ExplosionSmokeRing(
									   const point3 rp,            // �������� �����
									   const SMOKE_RING_INFO &info
									   )
{
	start_velocity = point3(0, 0, 0);
	// ������� ������ ������
	particles_num = 4;
	coords = new point3[particles_num];
	// �������� �������� �����, ������ � ���������� ���������
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num;

	// ������� �������
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// ����� � ������� ��������� ������
	root_point = rp + point3(0, 0, 0.2);

	start_size = info.Size0;                    // ��������� ������
	size_factor = info.SizeRange;            // ��������� �������
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
ExplosionSmokeRing::~ExplosionSmokeRing()
{
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	delete[] coords;
	// ����
	delete sound_object;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool ExplosionSmokeRing::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	color = 0xffffffff;

	// �������� ���������� ������
	for(int i = 0; i < particles_num; i++)
	{
		coords[i] = root_point;
	}

	// ����������� ���������� ����������
	texcoord_vector[0] = texcoord(0, 0);
	texcoord_vector[1] = texcoord(1, 0);
	texcoord_vector[2] = texcoord(1, 1);
	texcoord_vector[3] = texcoord(0, 1);
	return true;
}

// ������� ����������� �����
void ExplosionSmokeRing::Multiply()
{
}

// ���������� bounding box �������
BBox ExplosionSmokeRing::GetBBox(const float current_time)
{
	BBox b;

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - this->start_time;
	// ������
	size = start_size + sqrt(dt)*size_factor;

	b.maxx = root_point.x + size;
	b.maxy = root_point.y + size;
	b.maxz = root_point.z + 0.01;
	b.minx = root_point.x - size;
	b.miny = root_point.y - size;
	b.minz = root_point.z - 0.01;

	return b;
}

