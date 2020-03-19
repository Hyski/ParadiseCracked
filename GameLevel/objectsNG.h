#ifndef _NEXT_GENERATION_OBJECTS_
#define _NEXT_GENERATION_OBJECTS_
/*
набор классов для работы с объектами в игре
класс Shape - геометрическое представление
      Object - Shape + расположение в пространстве

	Grom
*/

class DynamicVB;
class ShapePool;
class TexObject;

class Shape
	{
	public:
		friend class ShapePool;
		typedef std::vector<const D3DMATRIX*> MatSet_t;
		
	public:
		bool TraceRay(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm) const;
		BBox GetBBox() const;
		~Shape();
		Shape(const Shape &a):m_GeomData(NULL){*this=a;};
		Shape& operator=(const Shape &a);
		bool operator ==(const Shape &a) const;
		bool operator !=(const Shape &a) const;
		const TexObject* GetGeometry(){return m_GeomData;};
	protected:
		Shape(TexObject *GeomData);
		
	private:
    enum SKIP_FLAGS{SF_TRANSPARENT=0x01, SF_SOLID=0x20};
		void Draw(const MatSet_t &,DynamicVB *Buf,unsigned skip_flags=0) const;
		
	private:
		unsigned short m_StartVertex;//Начальная вершина в VB, который создастся
		TexObject *m_GeomData;
	};
class ShapedObject
	{
	public:
		BBox GetBBox()const;
    bool TraceRay(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm) const;
		
	private:
		void UpdateVisibility();
		float m_LastTimeVisUpdated;

	private:
		bool m_Visible;
	};

class ShapePool
	{
	public:
		typedef unsigned int Handle;
		Handle AddShape(TexObject *GeomData);
		void CreateVB();
		
		void PushToDraw(const Handle &h,const D3DMATRIX &World);
		void DrawAll();
		void Clear();
		const Shape* GetShape(const Handle &h) const;
		ShapePool();
		~ShapePool();

	private:
		int NumVertices();	
		void FillBuffer(char *Data);

	private:
		typedef std::map<Handle,Shape::MatSet_t>	DrawQueue_t;
		typedef std::vector<Shape> shapes_set_t;

		DrawQueue_t DrawQueue;
		shapes_set_t m_Shapes;
		DynamicVB *m_Buf;
	};
#endif