/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: �����
				������ ����������� ������
				������ ������
				������ �������� �������� �� �����
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include "../../Skin/animalibrary.h"
#include "EffectInfo.h"
#include "LineEffect.h"


// ��������� ��� �������
// (������ ������ ���������)
bool LineEffect::NextTick(const float current_time)
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
	first_point = start_point + velocity*dt;
	if((first_point - start_point).Length() < length) second_point = start_point;
	else second_point = first_point - length*direction;
	if((end_point - second_point).Length() < length) first_point = end_point;

	/*LOG("LineEffect::NextTick first point = (%f, %f, %f), second point = (%f, %f, %f)\n",
		first_point.x, first_point.y, first_point.z, second_point.x,
		second_point.y, second_point.z);*/
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool LineEffect::Update()
{
	// ����
	sound_object->Update(first_point, velocity);
	// ������� �� ���� ����� ��� ������������
	Multiply();
	return true;
}

// ���������� bounding box �������
BBox LineEffect::GetBBox(const float)
{
	BBox b;
	b.Degenerate();
	b.Enlarge(first_point);
	b.Enlarge(second_point);
	return b;
}

// ����������
LineEffect::~LineEffect()
{
	delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// �����
	delete sound_object;
}

// ����������� � �����������
LineEffect::LineEffect(
					   const point3 StartPoint,    // ��������� �����
					   const point3 EndPoint,	   // �������� �����
					   const LINE_EFFECT_INFO& info
					   )
{
	// �������� �������� �����, ������ � ���������� ���������
	points_num = 4;
	ready_vector_size = ready_num = 4;

	// ������� �������
	color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// 
	start_point = StartPoint;                       // ��������� �����
	end_point = EndPoint;                           // �������� �����

	// ����� ����� �������
	lifetime = (end_point - start_point).Length() / info.VelFactor;

	length = info.Length;                                // �����
	width_first = info.WidthFirst;                       // ������ � ������ �����
	width_second = info.WidthSecond;                     // ������ � ����� �����

	velocity = (end_point - start_point)/lifetime;  // ��������
	lifetime += length / info.VelFactor;
	
	// ����, ����������� �� ������������� ���������� �������
	finished = false;
	first_point = start_point;

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
bool LineEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;
	direction = Normalize(end_point - start_point);

	// ����������� ���������� ����������
	texcoord_vector[0] = texcoord(1, 1);
	texcoord_vector[1] = texcoord(0, 1);
	texcoord_vector[2] = texcoord(0, 0);
	texcoord_vector[3] = texcoord(1, 0);

	// ����
	for(int i = 0; i < points_num; i++) color_vector[i] = 0xffffffff;
		
	// ���������� ������
	first_point = start_point;
	second_point = end_point;

	return true;
}

// ������� ����������� �����
void LineEffect::Multiply()
{
	point3 CameraDir = direction.Cross(pGraphPipe->GetCam()->GetFront());
	ready_vector[0] = second_point - width_second*CameraDir;
	ready_vector[1] = second_point + width_second*CameraDir;
	ready_vector[2] = first_point + width_first*CameraDir;
	ready_vector[3] = first_point - width_first*CameraDir;
}

//
// ����� ����������� ������
//

// ����������� � �����������
PlasmaBeamEffect::PlasmaBeamEffect(
								 const point3 StartPoint,    // ��������� �����
								 const point3 EndPoint,	     // �������� �����
								 const PLASMA_BEAM_INFO& info
								 )
{
	// �������� �������� �����, ������ � ���������� ���������
	points_num = 4;
	ready_vector_size = ready_num = 4;

	// ������� �������
	color_vector = new unsigned int [ready_vector_size];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// 
	start_point = StartPoint;                       // ��������� �����
	end_point = EndPoint;                           // �������� �����

	// ����� ����� �������
	lifetime = info.LifeTime;

	length = (end_point - start_point).Length();         // �����
	width_target = info.WidthTarget;                       // ������ � ������ �����
	width_source = info.WidthSource;                     // ������ � ����� �����

	vel = point3(0.0f, 0.0f, 0.0f);  // ��������
	bbox.Degenerate();
	bbox.Enlarge(start_point);
	bbox.Enlarge(end_point);
	
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
PlasmaBeamEffect::~PlasmaBeamEffect()
{
	delete[] color_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	// �����
	delete sound_object;
}

// ��������� ��� �������
// (������ ������ ���������)
bool PlasmaBeamEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool PlasmaBeamEffect::Update()
{
	// ����
	sound_object->Update(start_point, vel);
	// ������� �� ���� ����� ��� ������������
	Multiply();
	return true;
}

// ���������� bounding box �������
BBox PlasmaBeamEffect::GetBBox(const float)
{
	return bbox;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool PlasmaBeamEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;
	direction = Normalize(end_point - start_point);

	// ����������� ���������� ����������
	texcoord_vector[0] = texcoord(1, length*0.5);
	texcoord_vector[1] = texcoord(0, length*0.5);
	texcoord_vector[2] = texcoord(0, 0);
	texcoord_vector[3] = texcoord(1, 0);

	// ����
	for(int i = 0; i < points_num; i++) color_vector[i] = 0xffffffff;
		
	return true;
}

// ������� ����������� �����
void PlasmaBeamEffect::Multiply()
{
	point3 CameraDir = direction.Cross(pGraphPipe->GetCam()->GetFront());
	ready_vector[0] = start_point - width_source*CameraDir;
	ready_vector[1] = start_point + width_source*CameraDir;
	ready_vector[2] = end_point + width_target*CameraDir;
	ready_vector[3] = end_point - width_target*CameraDir;
}

//
// ����� ������
//

// ����������� � �����������
LightningEffect::LightningEffect(
								 const point3 StartPoint,  // ��������� �����
								 const point3 EndPoint,	 // �������� �����
								 const LIGHTNING_INFO& info
								 )
{
	// �������� �������� �����, ������ � ���������� ���������
	start_point = StartPoint;                       // ��������� �����
	end_point = EndPoint;                           // �������� �����
	length = (end_point - start_point).Length();         // �����
	brunch_num = info.BranchNum;
	segment_num = length/info.SegmentLength;
	points_num = segment_num * brunch_num;
	ready_vector_size = ready_num = points_num*PEAK_NUM;

	// ������� �������
	color_vector = new unsigned int [ready_vector_size];
	texcoord_vector = new texcoord [ready_vector_size];
	ready_vector = new point3 [ready_vector_size];
	nodes = new point3 [points_num];
	start_nodes = new point3 [points_num];

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// ����� ����� �������
	lifetime = info.LifeTime;

	length = (end_point - start_point).Length();         // �����
	width = info.Width;                                  // ������

	vel = point3(0.0f, 0.0f, 0.0f);  // ��������
	bbox.Degenerate();
	bbox.Enlarge(start_point);
	bbox.Enlarge(end_point);
	
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
LightningEffect::~LightningEffect()
{
	delete[] color_vector;
	delete[] texcoord_vector;
	delete[] ready_vector;
	delete[] nodes;
	delete[] start_nodes;
	// �����
	delete sound_object;
}

// ��������� ��� �������
// (������ ������ ���������)
bool LightningEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	for(int i = 0; i < points_num; i++)
	{
		float x = (1 - 2*(rand()/32768.0))*0.3;
		float y = (1 - 2*(rand()/32768.0))*0.3;
		float z = (1 - 2*(rand()/32768.0))*0.3;
		nodes[i] = start_nodes[i] + point3(x, y, z);
		if((lifetime - dt) < (0.35*lifetime))
		{
			unsigned int c = (lifetime - dt)/(lifetime*0.35)*255;
			color = RGBA_MAKE(c, c, c, c);
		}
	}
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool LightningEffect::Update()
{
	// ����
	sound_object->Update(start_point, vel);
	// ������� �� ���� ����� ��� ������������
	Multiply();
	return true;
}

// ���������� bounding box �������
BBox LightningEffect::GetBBox(const float)
{
	return bbox;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool LightningEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;
	direction = Normalize(end_point - start_point);

	// ����������� ���������� ����������
	for(int i = 0; i < points_num; i++)
	{
		// ���������� ����������
		texcoord_vector[i*PEAK_NUM] = texcoord(1, length/segment_num);
		texcoord_vector[i*PEAK_NUM + 1] = texcoord(1, 0);
		texcoord_vector[i*PEAK_NUM + 2] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 3] = texcoord(1, length/segment_num);
		texcoord_vector[i*PEAK_NUM + 4] = texcoord(0, 0);
		texcoord_vector[i*PEAK_NUM + 5] = texcoord(0, length/segment_num);
	}

	// ����������� ����
	for(int j = 0; j < brunch_num; j++) // ���� �� ������
	{
		// ��� ������ �����
		for(int i = 0; i < segment_num; i++) // ���� �� ��������� �����
		{
			start_nodes[j*segment_num + i] = start_point + direction*(i+1)*length/segment_num;
		}
	}
	return true;
}

// ������� ����������� �����
void LightningEffect::Multiply()
{
	point3 CameraDir = direction.Cross(pGraphPipe->GetCam()->GetFront());

	for(int i = 0; i < points_num; i++)
	{

		if(!(i % segment_num)) // ��� 0,3,5-�� �����
		{
			ready_vector[i*PEAK_NUM] = start_point - width*CameraDir;
			ready_vector[i*PEAK_NUM + 3] = ready_vector[i*PEAK_NUM];
			ready_vector[i*PEAK_NUM + 5] = start_point + width*CameraDir;
		}
		else
		{
			ready_vector[i*PEAK_NUM] = nodes[i - 1] - width*CameraDir;
			ready_vector[i*PEAK_NUM + 3] = ready_vector[i*PEAK_NUM];
			ready_vector[i*PEAK_NUM + 5] = nodes[i - 1] + width*CameraDir;
		}

		// ��� 1,2,4-�� �����
		ready_vector[i*PEAK_NUM + 1] = nodes[i] - width*CameraDir;
		ready_vector[i*PEAK_NUM + 2] = nodes[i] + width*CameraDir;
		ready_vector[i*PEAK_NUM + 4] = ready_vector[i*PEAK_NUM + 2];
	
		// ����
		color_vector[i*PEAK_NUM] = color;
		color_vector[i*PEAK_NUM + 1] = color;
		color_vector[i*PEAK_NUM + 2] = color;
		color_vector[i*PEAK_NUM + 3] = color;
		color_vector[i*PEAK_NUM + 4] = color;
		color_vector[i*PEAK_NUM + 5] = color;
	}
}


//
// �������� �������� �� �����
//

// ����������� � �����������
TracerLineModel::TracerLineModel(
								 const point3 StartPoint,  // ��������� �����
								 const point3 EndPoint,	   // �������� �����
								 const TRACER_LINE_MODEL_INFO& info
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
	// ��������� ��� � �������
	if(!m_skin)
	{
		// ������� ����������
		std::string str = "illegal skin: \"";
		str += std::string("Animations/skins/items/") + info.Sound + "\"";
		delete sound_object;
		throw CASUS(str);
	}
	m_skin->ValidateLinks(m_skeleton);

	point3 OX, OY, OZ;
	OZ = direction;
	OY = Normalize(direction.Cross(point3(0.0f, 0.0f, 1.0f)));
	OX = Normalize(OY.Cross(OZ));

	m_mat._11 = OX.x;
	m_mat._21 = OY.x;
	m_mat._31 = OZ.x;
	m_mat._14 = 0;

	m_mat._12 = OX.y;
	m_mat._22 = OY.y;
	m_mat._32 = OZ.y;
	m_mat._24 = 0;

	m_mat._13 = OX.z;
	m_mat._23 = OY.z;
	m_mat._33 = OZ.z;
	m_mat._34 = 0;

	m_mat._41 = 0;
	m_mat._42 = 0;
	m_mat._43 = 0;
	m_mat._44 = 1;	

	m_animadata.LastState.m_SkelState[0].World = m_mat;
	m_animadata.LastState.m_SkelState[1].World = m_mat;
	m_skin->Update(&m_animadata.LastState);
}

// ����������
TracerLineModel::~TracerLineModel()
{
	// �����
	delete sound_object;
	// ��������
	delete m_skin;
}

// ��������� ��� �������
// (������ ������ ���������)
bool TracerLineModel::NextTick(const float current_time)
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
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool TracerLineModel::Update()
{
	// ����
	sound_object->Update(start_point, vel);

	// ��������
	m_mat._41 = current_point.x;
	m_mat._42 = current_point.y;
	m_mat._43 = current_point.z;

	m_animadata.LastState.m_SkelState[0].World = m_mat;
	m_animadata.LastState.m_SkelState[1].World = m_mat;
	m_skin->Update(&m_animadata.LastState);
	pGraphPipe->Chop(m_skin->GetMesh());
	return true;
}

// ���������� bounding box �������
BBox TracerLineModel::GetBBox(const float)
{
	return bbox;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool TracerLineModel::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;
	return true;
}
