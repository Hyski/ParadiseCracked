/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: отрисовка хексов
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__GRAPH_HEX_GRID_H__)
#define __GRAPH_HEX_GRID_H__

#include "../common/graphpipe/SimpleTexturedObject.h"
class GraphPipe;

//////////////////////////////////////////////////////////////////////////////////
//
// класс для отрисовки хексов
//
//////////////////////////////////////////////////////////////////////////////////
class GraphHexGrid
{
public:
	// типы отрисовываемых хексов
	enum hex_type
	{
		joint = 0,
		front = 1,
		path = 2,
		land = 3
	};
	// установить новые хексы для отрисовки
	void SetHexes(const hex_type type, const pnt_vec_t& vec);
	// установить флаг отрисовки хексов
	void Show(const hex_type type, const bool flag);
	// отрисовать хексы
	void Draw();
	// конструктор
	GraphHexGrid();
private:

	// структурка, содержащая информацию для работы с хексом определенного типа
	struct hex_info_t
	{
		bool visible;
		// объект, который умеет отрисовывать графика
		TexObject tex_object;
		// имя шейдера
		std::string shader;
		// множитель текстурных координат
		float texcoord_factor;
	};

	// массив структурок, содержащих информацию для работы с хексами
	hex_info_t hex_info[4];
	// массив из 6 точек для размножения вершин хекса
	point3 hex_vert[6];
	// вектор, хранящий индексы треугольников, входящих в хексы
	typedef std::vector<int> IndexVector;
	// вектор, хранящий вершины этих треугольников
	typedef std::vector<point3> VertVector;
	typedef VertVector::iterator VertVectorIterator;

	// структура для определения функции сравнения для карты
	struct pntcmp
	{
		bool operator ()(const point3& p1, const point3& p2)
		{
			unsigned int s1 = static_cast<int>(p1.x*10.0);
			s1 = s1 << 10;
			s1 += static_cast<int>(p1.y*10.0);
			s1 = s1 << 10;
			s1 += static_cast<int>(p1.z*2.0);
			unsigned int s2 = static_cast<int>(p2.x*10.0);
			s2 = s2 << 10;
			s2 += static_cast<int>(p2.y*10.0);
			s2 = s2 << 10;
			s2 += static_cast<int>(p2.z*2.0);

			return s1 < s2;

		}
	};

	// карта для хранения точек с номерами
	typedef std::map<point3, int, pntcmp> VertMap;
	// итератор для карты
	typedef VertMap::iterator VertMapIterator;

	// возвращает индекс точки, которая была найдена в векторе или вставлена в него
	int PutPoint(const point3& pnt, VertVector& vec, VertMap& vmap);

	// инициализация
	bool initialized;
	void Init();
};

#endif // !defined(__GRAPH_HEX_GRID_H__)
