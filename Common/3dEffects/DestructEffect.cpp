/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект разрушения объектов
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "Effect.h"
#include "DestructEffect.h"
/*
#if defined (GLASS_DEBUG) //for log
CLog LOG_GLASS;
#endif // for log
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс, использующийся для отображения (унаследован от TexObject)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// конструктор
DestructObject::DestructObject(const float MaxSquare)
{
	PartNum = 0;
	max_square = MaxSquare;
}

// деструктор
DestructObject::~DestructObject()
{
}

// оператор присваивания для инициализации объекта
DestructObject& DestructObject::operator = (const DynObject &a)
{
	// уничтожим в нашем объекте все лишнее
	Release();

	// скопируем из предоставленного объекта части, которые необходимо копировать вручную
	PartNum=a.PartNum;
	for(int i = 0; i < PartNum; i++)
	{
		// 1. скопируем из предоставленного объекта в два списка
		TriangleList ready_triangles;
		TriangleList temp_triangles;
		CopyToList(a.Parts[i], (D3DMATRIX&)a.World, ready_triangles, temp_triangles);
		// 2. преобразуем список разбив большие треугольники
		ReformList(ready_triangles, temp_triangles);
		// 3. скопируем из списка в новый объект
		// создаем новый SimpleTexturedObject (STObject)
		Parts[i] = new STObject;
		Parts[i]->MaterialName = a.Parts[i]->MaterialName;
		CopyFromList(Parts[i], ready_triangles);
	}
	return *this;
}

// конструктор копирования
DestructObject::DestructObject(const DestructObject &a)
{
	Release();
	TexObject::operator=(a);
}

// для копирования из предоставленного объекта в список
void DestructObject::CopyToList(STObject * stobj, D3DMATRIX& matr, TriangleList& ready_tr, TriangleList& temp_tr)
{
	TriangleInf tr;
	for(int i = 0; i < stobj->IdxNum;)
	{
		VectorMatrixMultiply( *(D3DVECTOR*)&tr.Normals[0],
			*(D3DVECTOR*)&stobj->Normals[stobj->GetIndexesRO()[i]],
			matr);
		VectorMatrixMultiply( *(D3DVECTOR*)&tr.Normals[1],
			*(D3DVECTOR*)&stobj->Normals[stobj->GetIndexesRO()[i + 1]],
			matr);
		VectorMatrixMultiply( *(D3DVECTOR*)&tr.Normals[2],
			*(D3DVECTOR*)&stobj->Normals[stobj->GetIndexesRO()[i + 2]],
			matr);

		PointMatrixMultiply( *(D3DVECTOR*)&tr.Points[0],
			*(D3DVECTOR*)&stobj->Points[stobj->GetIndexesRO()[i]],
			matr);
		PointMatrixMultiply( *(D3DVECTOR*)&tr.Points[1],
			*(D3DVECTOR*)&stobj->Points[stobj->GetIndexesRO()[i + 1]],
			matr);
		PointMatrixMultiply( *(D3DVECTOR*)&tr.Points[2],
			*(D3DVECTOR*)&stobj->Points[stobj->GetIndexesRO()[i + 2]],
			matr);

		tr.uv[0] = stobj->uv[stobj->GetIndexesRO()[i]];
		tr.uv[1] = stobj->uv[stobj->GetIndexesRO()[i + 1]];
		tr.uv[2] = stobj->uv[stobj->GetIndexesRO()[i + 2]];

		tr.square = TriangleSquare(tr.Points[0], tr.Points[1], tr.Points[2]);

		// если площадь большая - кладем во временный список, иначе - в конечный
		if(tr.square > max_square)
		{
			temp_tr.push_back(tr);
		}
		else
		{
			ready_tr.push_back(tr);
		}
		i += 3;
	}
}

// для копирования из списка в новый объект
void DestructObject::CopyFromList(STObject * stobj, const TriangleList& tlist)
{
	stobj->PntNum = 3 * tlist.size();
	stobj->IdxNum = stobj->PntNum;

	stobj->indexes = new short int[stobj->IdxNum];
	stobj->Points = new point3[stobj->IdxNum];
	stobj->Normals = new point3[stobj->IdxNum];
	stobj->uv = new texcoord[stobj->IdxNum];

	int index = 0;
	TriangleList::const_iterator i, end;
	end = tlist.end();
	for(i = tlist.begin(); i != end; i++)
	{
		stobj->GetIndexesFull()[index] = index;
		stobj->Points[index] = (*i).Points[0];
		stobj->Normals[index] = (*i).Normals[0];
		stobj->uv[index] = (*i).uv[0];
		index++;

		stobj->GetIndexesFull()[index] = index;
		stobj->Points[index] = (*i).Points[1];
		stobj->Normals[index] = (*i).Normals[1];
		stobj->uv[index] = (*i).uv[1];
		index++;

		stobj->GetIndexesFull()[index] = index;
		stobj->Points[index] = (*i).Points[2];
		stobj->Normals[index] = (*i).Normals[2];
		stobj->uv[index] = (*i).uv[2];
		index++;

	}
}

// преобразование списока с разбиением больших треугольников
void DestructObject::ReformList(TriangleList& ready_tr, TriangleList& temp_tr)
{
/*	LOGG("DestructObject::ReformList: begin\n");
	LOGG("  ready_tr.size = %d, temp_tr.size = %d, max_square = %f\n", ready_tr.size(), temp_tr.size(), max_square);
	int n = 0;*/
	while(!temp_tr.empty())
	{
		//LOGG("\t While %d\n", n);
		TriangleList::iterator i = temp_tr.begin();
		TriangleList::iterator end = temp_tr.end();
		while(i != end)
		{
			if((*i).square < max_square)
			{
				// переносим треугольник из временного в конечный список
				//LOGG("\t Moving triangle: square = %f\n", (*i).square);

				ready_tr.push_front(*i);

				//n++;
				TriangleList::iterator j = i;
				i++;
				temp_tr.erase(j);
				//LOGG("\t ready_tr.size = %d, temp_tr.size = %d\n", ready_tr.size(), temp_tr.size());
			}	
			else
			{
				// разбиваем
				//LOGG("\t Breaking triangle: square = %f\n", (*i).square);

				// посчитаем центральную точку
				point3 pnt = ((*i).Points[0] + (*i).Points[1] + (*i).Points[2])/3.0;
				point3 nrm = ((*i).Normals[0] + (*i).Normals[1] + (*i).Normals[2])/3.0;
				float u = ((*i).uv[0].u + (*i).uv[1].u + (*i).uv[2].u)/3.0;
				float v = ((*i).uv[0].v + (*i).uv[1].v + (*i).uv[2].v)/3.0;
				texcoord uv = texcoord(u, v);
				float square = (*i).square / 3.0;

				// создаем, заполняем и кладем в список три новых треугольника
				TriangleInf tr;

				// первый
				tr.Points[0] = (*i).Points[0];
				tr.Normals[0] = (*i).Normals[0];
				tr.uv[0] = (*i).uv[0];

				tr.Points[1] = (*i).Points[1];
				tr.Normals[1] = (*i).Normals[1];
				tr.uv[1] = (*i).uv[1];

				tr.Points[2] = pnt;
				tr.Normals[2] = nrm;
				tr.uv[2] = uv;

				tr.square = square;

				temp_tr.push_front(tr);

				// второй
				tr.Points[0] = pnt;
				tr.Normals[0] = nrm;
				tr.uv[0] = uv;

				tr.Points[1] = (*i).Points[1];
				tr.Normals[1] = (*i).Normals[1];
				tr.uv[1] = (*i).uv[1];

				tr.Points[2] = (*i).Points[2];
				tr.Normals[2] = (*i).Normals[2];
				tr.uv[2] = (*i).uv[2];

				tr.square = square;

				temp_tr.push_front(tr);

				// третий
				tr.Points[0] = (*i).Points[0];
				tr.Normals[0] = (*i).Normals[0];
				tr.uv[0] = (*i).uv[0];

				tr.Points[1] = pnt;
				tr.Normals[1] = nrm;
				tr.uv[1] = uv;

				tr.Points[2] = (*i).Points[2];
				tr.Normals[2] = (*i).Normals[2];
				tr.uv[2] = (*i).uv[2];

				tr.square = square;

				temp_tr.push_front(tr);

				TriangleList::iterator j = i;
				i++;
				temp_tr.erase(j);

				//LOGG("\t ready_tr.size = %d, temp_tr.size = %d\n", ready_tr.size(), temp_tr.size());
			}
			i++;
		}
		//n++;
	}
	//LOGG("DestructObject::ReformList: end\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс эффекта разбивающегося стекла
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// конструктор
GlassDestruct::GlassDestruct(const DynObject& DynObj,
							 const point3& ExpPos,
							 const GLASS_DESTRUCT_EFFECT_INFO& info) : BaseDestructEffect(info.MaxSquare)
{
	triangles = 0;
	sound_object = 0;
	destruct_object = DynObj;
	//save_object = DynObj;
	if(!destruct_object.PartNum) { finished = true; return;}
	direction = Normalize(destruct_object.GetBBox().GetCenter() - ExpPos);
	first_time = true;
	finished = false;
	accel = info.Accel;
	friction_factor = info.FrictionFactor;
	lifetime = info.LifeTime;

	// посчитаем количество всех треугольников в объекте
	triangles_num = 0;
	for(int i = 0; i < destruct_object.PartNum; i++)
	{
		triangles_num += destruct_object.Parts[i]->IdxNum / 3;
	}

	//LOGG("GlassDestruct::GlassDestruct: triangles_num = %d\n", triangles_num);

	// создадим массив треугольников
	triangles = new TriangleDesc[triangles_num];
	// заполним его
	unsigned int index = 0;
	for(int i = 0; i < destruct_object.PartNum; i++)
	{
		for(int j = 0; j < destruct_object.Parts[i]->IdxNum;)
		{
			// из трех идущих подряд точек (вершин одного тр-ка) строим центр масс
			point3 tmp = destruct_object.Parts[i]->Points[j];
			tmp += destruct_object.Parts[i]->Points[j + 1];
			tmp += destruct_object.Parts[i]->Points[j + 2];
			tmp = tmp / 3.0;
			triangles[index].center = tmp;
			triangles[index].nodes[0] = destruct_object.Parts[i]->Points[j] - tmp;
			triangles[index].nodes[1] = destruct_object.Parts[i]->Points[j + 1] - tmp;
			triangles[index].nodes[2] = destruct_object.Parts[i]->Points[j + 2] - tmp;

			tmp.x = info.VelDisp.x*(1 - (float)rand()/16384.0);
			tmp.y = info.VelDisp.y*(1 - (float)rand()/16384.0);
			tmp.z = info.VelDisp.z*(1 - (float)rand()/16384.0);
			tmp = Normalize(direction + tmp);
			triangles[index].velocity = tmp * (info.MinVelFactor + (info.MaxVelFactor - info.MinVelFactor)*(float)rand()/32768.0);

			triangles[index].angle_velocity.x = info.MaxAngleVel*(1 - (float)rand()/16384.0);
			triangles[index].angle_velocity.y = info.MaxAngleVel*(1 - (float)rand()/16384.0);
			triangles[index].angle_velocity.z = info.MaxAngleVel*(1 - (float)rand()/16384.0);

			index++;
			j += 3;
		}
	}

	// звук
	if(info.Sound == "")
	{
		// звука нет
		sound_object = new EmptySound();
	}
	else
	{
		sound_object = new FixedSound(info.Sound, triangles[0].center);
	}
}

// деструктор
GlassDestruct::~GlassDestruct()
{
	delete[] triangles;
	delete sound_object;
}

// следующий тик времени
void GlassDestruct::NextTick(const float time)
{
	if(finished) return;
	if(first_time)
	{
		start_time = time;
		prev_time = time;
		first_time = false;
	}

	// время, прошедшее с момента запуска эффекта
	float dt = time - start_time;
	if(dt > lifetime)
	{
		finished = true;
		return;
	}
	
	// время, прошедшее с момента предыдущего расчета координат
	float dtt = time - prev_time;
	
	for(unsigned int i = 0; i < triangles_num; i++)
	{
		// пересчитаем скорость
		triangles[i].velocity += accel*dtt;
		point3 friction = -triangles[i].velocity * (friction_factor*dtt);
		triangles[i].velocity += friction;
		//friction.z = 0;
		// переместим центры масс
		triangles[i].center += triangles[i].velocity*dtt;
		// пересчитаем текущие вектора от центра масс к вершинам
		Quaternion qt;
		for(int j = 0; j < 3; j++)
		{
			// поворот вокруг OX
			qt.FromAngleAxis(fmod((triangles[i].angle_velocity.x*dt), PIm2), point3(1.0, 0, 0));
			triangles[i].next_nodes[j] = qt * triangles[i].nodes[j];

			// поворот вокруг OY
			qt.FromAngleAxis(fmod((triangles[i].angle_velocity.y*dt), PIm2), point3(0, 1.0, 0));
			triangles[i].next_nodes[j] = qt * triangles[i].next_nodes[j];

			// поворот вокруг OZ
			qt.FromAngleAxis(fmod((triangles[i].angle_velocity.z*dt), PIm2), point3(0, 0, 1.0));
			triangles[i].next_nodes[j] = qt * triangles[i].next_nodes[j];
		}
	}

	// проапдейтим все вершины
	unsigned int index = 0;
	for(int i = 0; i < destruct_object.PartNum; i++)
	{
		for(int j = 0; j < destruct_object.Parts[i]->IdxNum;)
		{
			// заполняем по три точки, идущих подряд
			destruct_object.Parts[i]->Points[j] = triangles[index].center + triangles[index].next_nodes[0];
			destruct_object.Parts[i]->Points[j + 1] = triangles[index].center + triangles[index].next_nodes[1];
			destruct_object.Parts[i]->Points[j + 2] = triangles[index].center + triangles[index].next_nodes[2];

			index++;
			j += 3;
		}
	}

	// новое предыдущее время
	prev_time = time;
}

// отрисовать эффект
void GlassDestruct::Draw(GraphPipe * const graph_pipe)
{
	graph_pipe->Chop(&destruct_object);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс менеджера эффектов разрушающихся объектов
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// конструктор
DestructEffectManager::DestructEffectManager()
{
}

// деструктор
DestructEffectManager::~DestructEffectManager()
{
	if(effects.empty()) return;

	EffectsList::iterator i;
	for(i = effects.begin(); i != effects.end(); i++)
	{
		delete (*i);
	}
	effects.clear();
}

// следующий тик времени
void DestructEffectManager::NextTick(const float time)
{
	if(effects.empty()) return;

	// вызовем тики у всех эффектов
	EffectsList::iterator i;
	EffectsList::const_iterator end = effects.end();
	for(i = effects.begin(); i != end; i++)
	{
		(*i)->NextTick(time);
	}

	// уничтожим отработавшие эффекты
	bool not_clear = true;
	while(not_clear)
	{
		// если список пуст - закончим очистку:
		if(effects.empty()) break;

		end = effects.end();
		for(i = effects.begin(); i != end; i++)
		{
			// если эффект отработал - удалим его
			if((*i)->IsFinished())
			{
				delete (*i);
				effects.erase(i);
				not_clear = true;
				break;
			}

			// если попался работающий эффект
			not_clear = false;
		}
	}
}

// отрисовка всех эффектов
void DestructEffectManager::Draw(GraphPipe * const graph_pipe)
{
	if(effects.empty()) return;

	// вызовем отрисовку у всех эффектов
	EffectsList::iterator i;
	EffectsList::const_iterator end = effects.end();
	for(i = effects.begin(); i != end; i++)
	{
		(*i)->Draw(graph_pipe);
	}
}

// временная функция для разбивания стекла
void DestructEffectManager::DestroyGlass(const DynObject& DynObj,
										 const point3& ExpPos,
										 const GLASS_DESTRUCT_EFFECT_INFO& info)
{
	BaseDestructEffect * bde = new GlassDestruct(DynObj, ExpPos, info);
	effects.push_back(bde);
}

