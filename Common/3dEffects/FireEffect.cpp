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
#include "FireEffect.h"

//
// construction & destruction
//

// ����������� � �����������
FireEffect::FireEffect(
					   const float quality,
					   const point3 rp,			   // ��������� �����
					   const FIRE_EFFECT_INFO& info
					   )
{
	turns_left = info.TurnsNum;
	reduce_time = info.ReduceTime;
	reduce = false;
	finished = false;
	first_time = true;
	activity = 1.0;
	vel = point3(0, 0, 0);
	// ������� ������ ������
	particles_num = 10 + 50*quality;
	particles = new FireParticle[particles_num];

	// �������� �������� �����, ������ � ���������� ���������
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num*PEAK_NUM;

	// ������� �������
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// �����, �� ������� ����� �����
	root_point = rp;

	// ������������ ������������ �������� �� �(Y) ��� ������ �������
	startDX = info.VelXY;
	// ������������ ������������ �������� �� Z ��� ������ �������
	startDZ = info.VelZ;
	// ����������� ��������� ������
	min_start_size = info.SizeBegin;
	// ������ ��������� ���������o �������
	size_range = info.SizeRnd;
	// ����������� ��������� ������� ������� � ����������� �� �������
	size_factor = info.SizeRange;
	// ������������ ����� ����� �������
	max_lifetime = info.LifeTime;

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
FireEffect::~FireEffect()
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
void FireEffect::Multiply()
{
	if(activity == 0) return;
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
bool FireEffect::Start(const float start_time)
{
	int i;
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// ������������������� �������
	for(i = 0; i < particles_num; i++)
	{
		int sw = (float)rand()/32768.0 * 4;
		//static int sw = 0;
		switch(sw)
		{
		case 0 : particles[i].texcoords1 = texcoord(0, 0); particles[i].texcoords2 = texcoord(0.5, 0.5); break;
		case 1 : particles[i].texcoords1 = texcoord(0.5, 0); particles[i].texcoords2 = texcoord(1, 0.5); break;
		case 2 : particles[i].texcoords1 = texcoord(0, 0.5); particles[i].texcoords2 = texcoord(0.5, 1); break;
		case 3 : particles[i].texcoords1 = texcoord(0.5, 0.5); particles[i].texcoords2 = texcoord(1, 1); break;
		}
		Rebirth(particles[i], start_time, 0.99);
	}

	return true;
}

// ��������� ��� �������
// (������ ������ ���������)
bool FireEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	if(activity == 0)
	{
		ready_num = 0;
		return true;
	}
	int c, c1, c2; // ����
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
			dt = 0;
		}
		// ����������
		particles[i].coords = root_point + particles[i].velocity * dt;

		// ����
		c = (particles[i].life_time - dt)*(255.0/particles[i].life_time);
		c1 = c * 0.7;
		c2 = c * 0.5;

		// ������� �� ������� ����� �������
		c *= activity;
		c1 *= activity;
		c2 *= activity;
		
		if(reduce && ((current_time - start_reduce_time) < reduce_time))
		{
			float dtt = current_time - start_reduce_time;
			c *= 1.0 - dtt/reduce_time;
			c1 *= 1.0 - dtt/reduce_time;
			c2 *= 1.0 - dtt/reduce_time;
		}
		particles[i].color = RGBA_MAKE(c, c1, c2, c);
		
		//�  ������
		particles[i].size = min_start_size + dt*size_factor;
	}
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool FireEffect::Update()
{
	if(activity == 0) return true;
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

// ������� ����������� ������� ��� �� ��������
void FireEffect::Rebirth(FireParticle & sp, float curtime, float disp)
{

	// ��� ��������� ���������
	float dx, dy, dz, a, r;

	sp.birth_time = curtime;
	sp.color = 0xffffffff;
	sp.coords = root_point;

	sp.size = min_start_size + (float)rand()/32768.0*size_range;
	a = ((float)rand())/32768.0 * PIm2; // ��������� ���� �����
	r = (float)rand()/32768.0 * startDX; // ��������� ������
	dx = r*cos(a);
	dy = r*sin(a);
	dz = (1.0-disp+disp*(float)rand()/32768.0 )* startDZ;
	sp.velocity = point3(dx, dy, dz);
	sp.life_time = (1.0-disp+disp*(float)rand()/32768.0) * max_lifetime;
	
}

// ���������� bounding box �������
BBox FireEffect::GetBBox(const float )
{
	BBox b;
	float dx, dy, dz;
	dx = startDX*max_lifetime;
	dy = startDX*max_lifetime;
	dz = startDZ*max_lifetime;
	b.maxx = root_point.x + dx;
	b.maxy = root_point.y + dy;
	b.maxz = root_point.z + dz;
	b.minx = root_point.x - dx;
	b.miny = root_point.y - dy;
	b.minz = root_point.z - dz;
	return b;
}
