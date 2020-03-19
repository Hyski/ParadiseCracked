/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� ������ �� ������
				����� �������� �������� �� ��������
				
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include "../../Skin/animalibrary.h"

#include "EffectInfo.h"
#include "TailEffect.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// ����� ������ �� ������
//
/////////////////////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
TailEffect::TailEffect(
					   const point3 StartPoint,     // ��������� �����
					   const point3 EndPoint,	    // �������� �����
					   const TAIL_EFFECT_INFO& info
					   )
{
	start_point = StartPoint;                       // ��������� �����
	end_point = EndPoint;                           // �������� �����
	start_size = info.Size*0.5;
	end_size = info.SizeEnd*0.5;
	lifetime = (end_point - start_point).Length() / info.VelFactor;
	velocity = (end_point - start_point)/lifetime;  // ��������
	direction = Normalize(end_point - start_point);
	particles_lifetime = info.LifeTime;
	frequency = info.Frequency;


	// ������� �������
	points_num = ceil(frequency * lifetime);
	ready_vector_size = points_num*PEAK_NUM;
	points_vector = new point3 [points_num];
	size_vector = new float [points_num];
	points2d_vector = new point3 [points_num];
	color_vector = new unsigned int [ready_vector_size];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	particles_num = 0;

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
			sound_object = new FixedSound(info.Sound, StartPoint);
		}
	}
}

// ����������
TailEffect::~TailEffect()
{
	delete [] points_vector;
	delete [] size_vector;
	delete [] points2d_vector;
	delete [] color_vector;
	delete [] texcoord_vector;
	delete [] ready_vector;
	// ����
	delete sound_object;
}

// ��������� ��� �������
// (������ ������ ���������)
bool TailEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	float dtt; // �����, ������� ������� ������� � ������� ������ ��������
	unsigned int c;
	dt = current_time - start_time;

	// ���������� ������� �������
	current_point = start_point + velocity*dt;

	if (dt < lifetime)
	{
		// ���������, ������� ������� ������ � ����������� �������� �, ���� ����, �������� ����� �������
		int n = floor(frequency * (current_time - previous_birth_time));
		if(n > 0)
		{
			// ���� ������� ���� ��� ����� ������
			float delta_t = (current_time - previous_birth_time) / n;
			for(int i = 1; i <= n; i++)
			{
				TailParticle tp;
				tp.birthtime = previous_birth_time + i*delta_t;
				tp.start_point = current_point - velocity*(n - i)*delta_t;
				tp.coords = tp.start_point;
				tp.lifetime = particles_lifetime;
				tp.velocity = point3(0, 0, 0);
				tp.texcoord_type = (float)rand()/32768.0 * 4;
				
				particles.push_back(tp);
			}

			// �������� ����� ����� ��������
			previous_birth_time = current_time;
		}
	}

	// ������ ������� ������� � ���������� ��� ��� �����
	TailParticlesListIterator I, J, E; // ��������, �������������� � �������� ���������
	I = particles.begin();
	E = particles.end();
	while(I != E)
	{
		dtt = (current_time - (*I).birthtime);
		if( dtt >= (*I).lifetime )
		{
			// ������ �������
			J = I;
			I++;
			particles.erase(J);
			continue;
		}
		// ��� ����� ������� - ����������� ��� ��� ���
		(*I).size = start_size + (end_size - start_size)*dtt/(*I).lifetime;
		c = 255*(1 - dtt/(*I).lifetime);
		(*I).color = RGBA_MAKE(c, c, c, c);
		I++;
	}

	// ���� ������ ���� ��� ����� ����� - �������� ������
	if(particles.empty() && (dt > lifetime))
	{
		finished = true;
		return true;
	}
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool TailEffect::Update()
{
	// ����
	sound_object->Update(current_point, velocity);

	float x1 = 0, x2 = 1, y1 = 0, y2 = 1;
	particles_num = 0;
	TailParticlesListIterator I, E; // �������� � �������� ���������
	E = particles.end();
	for(I = particles.begin(); I != E; I++)
	{
		points_vector[particles_num] = (*I).coords;
		size_vector[particles_num] = (*I).size;

		color_vector[particles_num*PEAK_NUM] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 1] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 2] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 3] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 4] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 5] = (*I).color;

		switch((*I).texcoord_type)
		{
		case 0: x1 = 0; y1 = 0; x2 = 0.5; y2 = 0.5; break;
		case 1: x1 = 0.5; y1 = 0; x2 = 1; y2 = 0.5; break;
		case 2: x1 = 0; y1 = 0.5; x2 = 0.5; y2 = 1; break;
		case 3: x1 = 0.5; y1 = 0.5; x2 = 1; y2 = 1; break;
		}
		// ����������� ���������� ����������
		texcoord_vector[particles_num*PEAK_NUM] = texcoord(x1, y1);
		texcoord_vector[particles_num*PEAK_NUM + 1] = texcoord(x2, y1);
		texcoord_vector[particles_num*PEAK_NUM + 2] = texcoord(x2, y2);
		texcoord_vector[particles_num*PEAK_NUM + 3] = texcoord(x1, y1);
		texcoord_vector[particles_num*PEAK_NUM + 4] = texcoord(x2, y2);
		texcoord_vector[particles_num*PEAK_NUM + 5] = texcoord(x1, y2);

		particles_num++;
	}
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(particles_num, points_vector, points2d_vector);
	// ���������� �����
	Multiply();

	return true;
}

// ���������� bounding box �������
BBox TailEffect::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(start_point - point3(end_size, end_size, end_size));
	b.Enlarge(end_point + point3(end_size, end_size, end_size));
	return b;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool TailEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// ����
	color = 0xffffffff;

	// ���������� ������� �����
	current_point = start_point;
	previous_birth_time = start_time;

	return true;
}



// ������� ����������� �����
void TailEffect::Multiply()
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

	ready_num = 0;
	for(int i = 0; i < particles_num; i++)
	{
		// ����������� ��������� ������
		z = points2d_vector[i].z;

		// ��������� ����������
		ready_vector[i*PEAK_NUM].z = z;
		ready_vector[i*PEAK_NUM + 1].z = z;
		ready_vector[i*PEAK_NUM + 2].z = z;
		ready_vector[i*PEAK_NUM + 3].z = z;
		ready_vector[i*PEAK_NUM + 4].z = z;
		ready_vector[i*PEAK_NUM + 5].z = z;

		xsize = size_vector[i]*(z*(n-f)+f)*xfactor;
		ysize = size_vector[i]*(z*(n-f)+f)*yfactor;

		ready_vector[i*PEAK_NUM].x = points2d_vector[i].x - xsize;
		ready_vector[i*PEAK_NUM + 1].x = points2d_vector[i].x + xsize;
		ready_vector[i*PEAK_NUM + 2].x = points2d_vector[i].x + xsize;
		ready_vector[i*PEAK_NUM + 3].x = points2d_vector[i].x - xsize;
		ready_vector[i*PEAK_NUM + 4].x = points2d_vector[i].x + xsize;
		ready_vector[i*PEAK_NUM + 5].x = points2d_vector[i].x - xsize;

		ready_vector[i*PEAK_NUM].y = points2d_vector[i].y + ysize;
		ready_vector[i*PEAK_NUM + 1].y = points2d_vector[i].y + ysize;
		ready_vector[i*PEAK_NUM + 2].y = points2d_vector[i].y - ysize;
		ready_vector[i*PEAK_NUM + 3].y = points2d_vector[i].y + ysize;
		ready_vector[i*PEAK_NUM + 4].y = points2d_vector[i].y - ysize;
		ready_vector[i*PEAK_NUM + 5].y = points2d_vector[i].y - ysize;

		// �������� ���������� �������� �����
		ready_num += PEAK_NUM;
	}
}


//
// ����� ������ �� ������
//

// ���������� ������� ������� ����� � ������� ������ �����
bool TailCircleEffect::Update()
{
	// ����
	sound_object->Update(current_point, velocity);

	particles_num = 0;
	TailParticlesListIterator I, E; // �������� � �������� ���������
	E = particles.end();
	for(I = particles.begin(); I != E; I++)
	{
		points_vector[particles_num] = (*I).coords;
		size_vector[particles_num] = (*I).size;

		color_vector[particles_num*PEAK_NUM] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 1] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 2] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 3] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 4] = (*I).color;
		color_vector[particles_num*PEAK_NUM + 5] = (*I).color;

		particles_num++;
	}
	// ���������� �����
	Multiply();

	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool TailCircleEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// ����
	color = 0xffffffff;

	// ���������� ������� �����
	current_point = start_point;
	previous_birth_time = start_time;

	// �������� ���������� ����������
	for(int i = 0; i < points_num; i++)
	{
		texcoord_vector[i*PEAK_NUM] = texcoord(0, 1);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 1);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(0, 1);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, 0);
	}

	return true;
}

// ������� ����������� �����
void TailCircleEffect::Multiply()
{
	point3 p = direction.Cross(point3(1.0, 0.0, 0.0));
	p = Normalize(p);
	point3 up;
	Quaternion qtpi4, qtpi2, qtpi;
	qtpi4.FromAngleAxis(PId4, direction);
	qtpi2.FromAngleAxis(PId2, direction);
	qtpi.FromAngleAxis(PI, direction);
	ready_num = 0;
	for(int i = 0; i < particles_num; i++)
	{
		up = p*size_vector[i];
		// ��������� ����������
		ready_vector[i*PEAK_NUM] = points_vector[i] + qtpi4*(qtpi*up);
		ready_vector[i*PEAK_NUM + 1] = points_vector[i] + qtpi4*(qtpi2*up);
		ready_vector[i*PEAK_NUM + 2] = points_vector[i] + qtpi4*up;
		ready_vector[i*PEAK_NUM + 3] = points_vector[i] + qtpi4*(qtpi*up);
		ready_vector[i*PEAK_NUM + 4] = points_vector[i] + qtpi4*up;
		ready_vector[i*PEAK_NUM + 5] = points_vector[i] + qtpi4*(qtpi2*(qtpi*up));

		// �������� ���������� �������� �����
		ready_num += PEAK_NUM;
	}
}


//
// ����� ������ �� ������, �������� �� ��������
//

// ����������� � �����������
TailParabolaEffect::TailParabolaEffect(
									   const point3 StartPoint,                          // ��������� �����
									   const point3 EndPoint,	                         // �������� �����
									   const TAIL_EFFECT_INFO& info,
									   const float Gravitation
									   ) : TailEffect(StartPoint, EndPoint, info)
{ 
	gravitation = Gravitation;
	vz = (EndPoint.z - StartPoint.z)/lifetime - gravitation*lifetime*0.5;
}

// ��������� ��� �������
// (������ ������ ���������)
bool TailParabolaEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	float dtt; // �����, ������� ������� ������� � ������� ������ ��������
	unsigned int c;
	dt = current_time - start_time;

	// ���������� ������� �������
	current_point = start_point + velocity*dt;
	current_point.z = start_point.z + vz*dt + gravitation*dt*dt*0.5;

	if (dt < lifetime)
	{
		// ���������, ������� ������� ������ � ����������� �������� �, ���� ����, �������� ����� �������
		int n = floor(frequency * (current_time - previous_birth_time));
		if(n > 0)
		{
			// ���� ������� ���� ��� ����� ������
		float delta_t = (current_time - previous_birth_time) / n;
			for(int i = 1; i <= n; i++)
			{
				TailParticle tp;
				tp.birthtime = previous_birth_time + i*delta_t;
				float dttt = tp.birthtime - start_time;

				tp.start_point = start_point + velocity*dttt;
				tp.start_point.z = start_point.z + vz*dttt + gravitation*dttt*dttt*0.5;
				tp.coords = tp.start_point;
				tp.lifetime = particles_lifetime;
				tp.velocity = point3(0, 0, 0);
				tp.texcoord_type = (float)rand()/32768.0 * 4;
				
				particles.push_back(tp);
			}

			// �������� ����� ����� ��������
			previous_birth_time = current_time;
		}
	}

	// ������ ������� ������� � ���������� ��� ��� �����
	TailParticlesListIterator I, J, E; // ��������, �������������� � �������� ���������
	I = particles.begin();
	E = particles.end();
	while(I != E)
	{
		dtt = (current_time - (*I).birthtime);
		if( dtt >= (*I).lifetime )
		{
			// ������ �������
			J = I;
			I++;
			particles.erase(J);
			continue;
		}
		// ��� ����� ������� - ����������� ��� ��� ���
		(*I).size = start_size + (end_size - start_size)*dtt/(*I).lifetime;
		c = 255*(1 - dtt/(*I).lifetime);
		(*I).color = RGBA_MAKE(c, c, c, c);
		I++;
	}

	// ���� ������ ���� ��� ����� ����� - �������� ������
	if(particles.empty() && (dt > lifetime))
	{
		finished = true;
		return true;
	}
	return true;
}

// ���������� bounding box �������
BBox TailParabolaEffect::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(start_point - point3(end_size, end_size, end_size));
	b.Enlarge(end_point + point3(end_size, end_size, end_size));
	point3 tmp = start_point + velocity*lifetime*0.5;
	tmp.z = start_point.z + vz*0.5*lifetime + gravitation*0.5*0.5*0.5*lifetime*lifetime;
	return b;
}

//
// ����� �������� �������� �� ��������
//

// ����������� � �����������
TracerParabolaModel::TracerParabolaModel(
										 const point3 StartPoint,  // ��������� �����
										 const point3 EndPoint,	   // �������� �����
										 const TRACER_PARABOLA_MODEL_INFO& info
										 )
{
	// �������� �������� �����, ������ � ���������� ���������
	points_num = 0;
	ready_vector_size = ready_num = 0;

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// 
	start_point = StartPoint;                       // ��������� �����
	end_point = EndPoint;                           // �������� �����

	// ����� ����� �������
	lifetime = (EndPoint - StartPoint).Length()/info.VelFactor;

	direction = Normalize(EndPoint - StartPoint);
	vel = direction * info.VelFactor;  // ��������
	gravitation = info.Gravitation;
	vz = (EndPoint.z - StartPoint.z)/lifetime - gravitation*lifetime*0.5;
	bbox.Degenerate();
	bbox.Enlarge(start_point);
	bbox.Enlarge(end_point);
	
	// ����, ����������� �� ������������� ���������� �������
	finished = false;

	// ����� ���
	sound_object = new EmptySound();

	// �������� ��������
	AnimaLibrary *lib=AnimaLibrary::GetInst();
	m_skeleton = lib->GetSkAnimation(std::string("Animations/anims/items/") + info.Shader);
	m_animadata = m_skeleton->Start(0);

	// �������� ����
	m_skin = lib->GetSkSkin(std::string("Animations/skins/items/") + info.Sound);
	if(!m_skin)
	{
		// ������� ����������
		std::string str = "illegal skin: \"";
		str += std::string("Animations/skins/items/") + info.Sound + "\"";
		delete sound_object;
		throw CASUS(str);
	}
	// ��������� ��� � �������
	m_skin->ValidateLinks(m_skeleton);


	m_mat._11 = 1;
	m_mat._12 = 0;
	m_mat._13 = 0;
	m_mat._14 = 0;

	m_mat._21 = 0;
	m_mat._22 = 1;
	m_mat._23 = 0;
	m_mat._24 = 0;

	m_mat._31 = 0;
	m_mat._32 = 0;
	m_mat._33 = 1;
	m_mat._34 = 0;

	m_mat._41 = 0;
	m_mat._42 = 0;
	m_mat._43 = 0;
	m_mat._44 = 1;

	m_animadata.LastState.m_SkelState[0].World = m_mat;
	m_skin->Update(&m_animadata.LastState);
}

// ����������
TracerParabolaModel::~TracerParabolaModel()
{
	// �����
	delete sound_object;
	// ��������
	delete m_skin;
}

// ��������� ��� �������
// (������ ������ ���������)
bool TracerParabolaModel::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}
	current_point = start_point + vel*dt;
	current_point.z = start_point.z + vz*dt + gravitation*dt*dt*0.5;
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool TracerParabolaModel::Update()
{
	// ����
	sound_object->Update(start_point, vel);

	// ��������
	m_mat._41 = current_point.x;
	m_mat._42 = current_point.y;
	m_mat._43 = current_point.z;

	m_animadata.LastState.m_SkelState[0].World = m_mat;
	m_skin->Update(&m_animadata.LastState);
	pGraphPipe->Chop(m_skin->GetMesh());
	return true;
}

// ���������� bounding box �������
BBox TracerParabolaModel::GetBBox(const float )
{
	return bbox;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool TracerParabolaModel::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;
	return true;
}
