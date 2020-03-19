#include "precomp.h"
#include "objectsNG.h"
#include "../common/graphpipe/dynamicVB.h"
#include "../common/graphpipe/simpletexturedobject.h"
#include "../iworld.h"
#include "../common/graphpipe/graphpipe.h"

#include "../globals.h"

ShapePool::ShapePool():
m_Buf(NULL)
	{
	}

ShapePool::Handle ShapePool::AddShape(TexObject *GeomData)
	{
	Shape s(GeomData);
	for(int i=0;i<m_Shapes.size();i++)
		{
		if(s==m_Shapes[i]) 
			return i;
		}
	m_Shapes.push_back(s);
	return m_Shapes.size()-1;
	}

int ShapePool::NumVertices()
	{
	int num=0;
	for(int i=0;i<m_Shapes.size();i++)
		{
		for(int j=0;j<m_Shapes[i].m_GeomData->PartNum;j++)
			{
			num+=m_Shapes[i].m_GeomData->Parts[j]->PntNum;
			}
		}
	return num;
	}
void ShapePool::FillBuffer(char *VBData)
	{
	static const int vert_size=12+12+8+4;
	char *data=VBData;

	for(int i=0;i<m_Shapes.size();i++)
		{
		m_Shapes[i].m_StartVertex=(data-VBData)/vert_size;
		for(int j=0;j<m_Shapes[i].m_GeomData->PartNum;j++)
			{
			for(int k=0;k<m_Shapes[i].m_GeomData->Parts[j]->PntNum;k++)
				{
				*(point3*)data         =m_Shapes[i].m_GeomData->Parts[j]->Points[k];
				*(point3*)(data+12)         =m_Shapes[i].m_GeomData->Parts[j]->Normals[k];
				*(texcoord*)(data+2*12+4)  =m_Shapes[i].m_GeomData->Parts[j]->uv[k];
				data+=vert_size;
				}
			}
		}
	}

void ShapePool::CreateVB()
	{	
	if(m_Buf) delete m_Buf;
  unsigned FVFflags=D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_DIFFUSE ;
	int num_vert=NumVertices();
	if(!num_vert) return;
	m_Buf = new DynamicVB( D3DKernel::GetD3D(),FVFflags,num_vert,2*sizeof(point3)+sizeof(texcoord)+4);
	unsigned int startv;
	char *data=(char*)m_Buf->Lock(num_vert,&startv);
	FillBuffer(data);
	m_Buf->Unlock();
	//m_Buf->GetInterface()->Optimize(D3DKernel::GetD3DDevice(),0);
	}

void ShapePool::PushToDraw(const Handle &h,const D3DMATRIX &World)
	{
	//сюда напихается куча ссылок на объекты, которые надо отрисовать
	DrawQueue[h].push_back(&World);
	}

void ShapePool::DrawAll()
	{
	if(!m_Buf) return;
  int i;
	//а здесь все они отрисуются
#if 1 //new method	
/*	for(i=m_Shapes.size()-1;i>=0;i--)
		{
    if(DrawQueue[i].size()) m_Shapes[i].Draw(DrawQueue[i],m_Buf,Shape::SF_TRANSPARENT);
		}
	for(i=m_Shapes.size()-1;i>=0;i--)
		{
		if(DrawQueue[i].size()) m_Shapes[i].Draw(DrawQueue[i],m_Buf,Shape::SF_SOLID);
		}
*/
  for(i=0; i<m_Shapes.size();i++)
		{
	  if(DrawQueue[i].size()) m_Shapes[i].Draw(DrawQueue[i],m_Buf,Shape::SF_TRANSPARENT);
		}
  for(i=0; i<m_Shapes.size();i++)
		{
	  if(DrawQueue[i].size()) m_Shapes[i].Draw(DrawQueue[i],m_Buf,Shape::SF_SOLID);
		}
#else//старый метод
	
	/*for(i=m_Shapes.size()-1;i>=0;i--)
		{
		for(int j=0;j<DrawQueue[i].size();j++)
			{
			StatesManager::SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX*)DrawQueue[i][j]);
  		IWorld::Get()->GetPipe()->Chop(m_Shapes[i].m_GeomData);
			}
	StatesManager::SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX*)IDENTITYMAT);

		}	 */
		
#endif														 
	DrawQueue.clear();
	}
void ShapePool::Clear()
	{
	if(m_Buf) {delete m_Buf;m_Buf=NULL;}
	m_Shapes.clear();
	}
const Shape* ShapePool::GetShape(const Handle &h) const
	{
	return &m_Shapes[h];
	}
ShapePool::~ShapePool()
	{
	if(m_Buf) delete m_Buf;
	}


Shape& Shape::operator=(const Shape &a)
	{
	if(m_GeomData) delete m_GeomData;
	m_GeomData=new TexObject(*a.m_GeomData);
	m_StartVertex = a.m_StartVertex;

	return *this;
	}
bool Shape::operator ==(const Shape &a) const
	{
	if(!m_GeomData||!a.m_GeomData) return false;
	TexObject *x=m_GeomData;
	TexObject *y=a.m_GeomData;
	if(x->PartNum!=y->PartNum) return false;
	for(int i=0;i<x->PartNum;i++)
		{
		if(x->Parts[i]->PntNum!=y->Parts[i]->PntNum) return false;
		if(x->Parts[i]->IdxNum!=y->Parts[i]->IdxNum) return false;
		if(x->Parts[i]->MaterialName!=y->Parts[i]->MaterialName) return false;
		int j;
		for(j=0;j<x->Parts[i]->PntNum;j++)
			{	
			point3 pnt_diff(x->Parts[i]->Points[j]-y->Parts[i]->Points[j]);
			point3 uv_diff(x->Parts[i]->uv[j].u-y->Parts[i]->uv[j].u,x->Parts[i]->uv[j].v-y->Parts[i]->uv[j].v,0);
			if(fabs(pnt_diff.x)+fabs(pnt_diff.y)+fabs(pnt_diff.z)>0.01) return false;
			if(fabs(uv_diff.x)+fabs(uv_diff.y)>0.001) return false;
			}
		for(j=0;j<x->Parts[i]->IdxNum;j++)
			{
			if(x->Parts[i]->indexes[j]!=y->Parts[i]->indexes[j]) return false;
			}
		}
	return true;
	}
bool Shape::operator !=(const Shape &a) const
	{
	return !(*this==a);
	}


bool Shape::TraceRay(const point3 & /*From*/, const point3 & /*Dir*/, point3 * /*Res*/, point3 * /*Norm*/) const
	{
	return false;
	}
BBox Shape::GetBBox() const
		{
		if(m_GeomData) return m_GeomData->GetBBox();
		BBox a;
		a.Box(NULLVEC,0);
		return a;
		}
Shape::Shape(TexObject *GeomData):m_GeomData(NULL),m_StartVertex(0xffff)
	{
	m_GeomData=new TexObject(*GeomData);
	}
Shape::~Shape()
	{
	if(m_GeomData) delete m_GeomData;
	}
void Shape::Draw(const MatSet_t &Worlds,DynamicVB *Buf,unsigned skip_flags) const
		{
		IWorld::Get()->GetPipe()->Chop(m_GeomData,Worlds,Buf,m_StartVertex, skip_flags);
		if(m_StartVertex == 0xffff) throw CASUS("ЕГГОГ!");
		}

