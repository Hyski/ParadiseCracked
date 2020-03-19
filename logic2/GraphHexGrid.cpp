/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: отрисовка hexgrid'a
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#pragma warning(disable:4786)

#include "logicdefs.h"
#include "DirtyLinks.h"
#include "HexGrid.h"
#include "HexUtils.h"
#include "GraphHexGrid.h"
#include "../Common/GraphPipe/GraphPipe.h"

//////////////////////////////////////////////////////////////////////////////////
//
// класс для отрисовки хекс грида
//
//////////////////////////////////////////////////////////////////////////////////

// конструктор
GraphHexGrid::GraphHexGrid()
{
	hex_info[0].shader = "joint_grid";
	hex_info[0].texcoord_factor = 1.0;
	hex_info[0].visible = false;

	hex_info[1].shader = "front_grid";
	hex_info[1].texcoord_factor = 1.0;
	hex_info[1].visible = false;

	hex_info[2].shader = "path_grid";
	hex_info[2].texcoord_factor = 1.0;
	hex_info[2].visible = false;

	hex_info[3].shader = "land_grid";
	hex_info[3].texcoord_factor = 1.0;
	hex_info[3].visible = false;

	initialized = false;
}

// установить новые хексы для отрисовки
void GraphHexGrid::SetHexes(const hex_type type, const pnt_vec_t& vec)
{
	if(!initialized) Init();

	// Punch hex_info[type].visible = true;

	// максимальное количество вершин
	int max_vert_num = vec.size()*12;
	// максимальное оно же реальное количество индексов
	int max_index_num = vec.size()*12;
	// следующий номер индекса в векторе
	int next_ind_num = 0;
	// вектор вершин
	VertVector vert_vector;
	// зарезервируем место для добавление вершин в вектор
	vert_vector.reserve(max_vert_num);
	// вектор индексов (создаем сразу на все элементы)
	IndexVector index_vector(max_index_num);
	// карта уникальных вершин
	VertMap vert_map;

	// бежим по списку центров хексов
	pnt_vec_t::const_iterator I = vec.begin();
	pnt_vec_t::const_iterator end = vec.end();
	while(I != end)
	{
		// получим шесть вершин хекса
		point3 cell_center = HexGrid::GetInst()->Get(*I);
		// положим их в вектор вершин и вектор индексов

		// 1
		index_vector[next_ind_num] = PutPoint(cell_center + hex_vert[1], vert_vector, vert_map);
		next_ind_num++;

		index_vector[next_ind_num] = PutPoint(cell_center + hex_vert[5], vert_vector, vert_map);
		next_ind_num++;

		index_vector[next_ind_num] = PutPoint(cell_center + hex_vert[0], vert_vector, vert_map);
		next_ind_num++;

		// 2
		index_vector[next_ind_num] = PutPoint(cell_center + hex_vert[1], vert_vector, vert_map);
		next_ind_num++;

		index_vector[next_ind_num] = PutPoint(cell_center + hex_vert[4], vert_vector, vert_map);
		next_ind_num++;

		index_vector[next_ind_num] = PutPoint(cell_center + hex_vert[5], vert_vector, vert_map);
		next_ind_num++;

		// 3
		index_vector[next_ind_num] = PutPoint(cell_center + hex_vert[1], vert_vector, vert_map);
		next_ind_num++;

		index_vector[next_ind_num] = PutPoint(cell_center + hex_vert[3], vert_vector, vert_map);
		next_ind_num++;

		index_vector[next_ind_num] = PutPoint(cell_center + hex_vert[4], vert_vector, vert_map);
		next_ind_num++;

		// 4
		index_vector[next_ind_num] = PutPoint(cell_center + hex_vert[1], vert_vector, vert_map);
		next_ind_num++;

		index_vector[next_ind_num] = PutPoint(cell_center + hex_vert[2], vert_vector, vert_map);
		next_ind_num++;

		index_vector[next_ind_num] = PutPoint(cell_center + hex_vert[3], vert_vector, vert_map);
		next_ind_num++;

		++I;
	}

	//

	hex_info[type].tex_object.Release();
	hex_info[type].tex_object.PartNum = 1;
	hex_info[type].tex_object.Parts[0] = new SimpleTexturedObject;
	hex_info[type].tex_object.Parts[0]->Alloc(vert_vector.size(), max_index_num);
	hex_info[type].tex_object.Parts[0]->MaterialName = hex_info[type].shader;
	for(int k = 0; k < hex_info[type].tex_object.Parts[0]->PntNum; k++)
	{
		hex_info[type].tex_object.Parts[0]->Points[k] = vert_vector[k];
		hex_info[type].tex_object.Parts[0]->Normals[k] = point3(0.0f, 0.0f, 1.0f);
		hex_info[type].tex_object.Parts[0]->uv[k] = texcoord(hex_info[type].texcoord_factor*hex_info[type].tex_object.Parts[0]->Points[k].x,
			hex_info[type].texcoord_factor*hex_info[type].tex_object.Parts[0]->Points[k].y/0.8660254f*0.5);
	}
	for(int k = 0; k < max_index_num; k++)
	{
		hex_info[type].tex_object.Parts[0]->GetIndexesFull()[k] = index_vector[k];
	}
}

// установить флаг отрисовки хексов
void GraphHexGrid::Show(const hex_type type, const bool flag)
{
	hex_info[type].visible = flag;
}

// отрисовать хексы
void GraphHexGrid::Draw()
{
	for(int i = 0; i < 4; i++)
	{
        if(hex_info[i].visible) DirtyLinks::GetGraphPipe()->Chop(&(hex_info[i].tex_object));
	}
}

// возвращает индекс точки, которая была найдена в векторе или вставлена в него
int GraphHexGrid::PutPoint(const point3& pnt, VertVector& vec, VertMap& vmap)
{
	// попытаемся добавить в карту точку
	std::pair<VertMapIterator, bool> P = vmap.insert(std::pair<const point3, int>(pnt, 0));
	// проверим что нам вернула вставка
	VertMapIterator I = P.first;
	if(P.second)
	{
		// элемент был добавлен в карту, т.е. это новая уникальная точка
		// добавим ее в вектор вершин
		vec.push_back(pnt);
		// изменим данные в карте
		(*I).second = vec.size() - 1;
	}
	// теперь в карте такая точка уже точно есть => вернем ее индекс
	return (*I).second;
}

void GraphHexGrid::Init()
{
	initialized = true;
	// заполним массив из 6-ти точек для размножения вершин хекса
	Quaternion qt;
	point3 tmp(0.0f, 1.0f, 0.0f);
	point3 addon(0.0f, 0.0f, 0.1f);

	hex_vert[0] = addon + HexTraits::radius*tmp;

	qt.FromAngleAxis(PId3, point3(0.0f, 0.0f, 1.0f));
	hex_vert[1] = addon + HexTraits::radius*(qt*tmp);

	qt.FromAngleAxis(2.0*PId3, point3(0.0f, 0.0f, 1.0f));
	hex_vert[2] = addon + HexTraits::radius*(qt*tmp);

	qt.FromAngleAxis(PI, point3(0.0f, 0.0f, 1.0f));
	hex_vert[3] = addon + HexTraits::radius*(qt*tmp);

	qt.FromAngleAxis(4.0*PId3, point3(0.0f, 0.0f, 1.0f));
	hex_vert[4] = addon + HexTraits::radius*(qt*tmp);

	qt.FromAngleAxis(5.0*PId3, point3(0.0f, 0.0f, 1.0f));
	hex_vert[5] = addon + HexTraits::radius*(qt*tmp);
}
