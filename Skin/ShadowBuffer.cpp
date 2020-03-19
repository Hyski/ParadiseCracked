#include "precomp.h"

#include "shadow.h"
#include "../GameLevel/bsp.h"

namespace Shadows {

  CLog bufLog;

ShadowBuffer::ShadowBuffer ()
{
	vCount = 0;

	m_primitive.IdxNum = 0;
	m_primitive.Prim = Primi::TRIANGLE;
	m_primitive.Contents = Primi::NEEDTRANSFORM|Primi::DIFFUSE;
	m_primitive.Pos = m_pos;
	m_primitive.Diffuse = m_diffuse;
	m_primitive.UVs[0] = m_texcoords;
}

ShadowBuffer::~ShadowBuffer ()
{
}

void ShadowBuffer::Draw (GraphPipe * pipe, const std::string &shader)
{
	m_primitive.VertNum = vCount;
	if(vCount) pipe->Chop(shader,&m_primitive);
//	if (vCount) D3DKernel::GetD3DDevice()->DrawPrimitive (D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, vertices, vCount, 0/*D3DDP_WAIT*/);
}

#define EPS		1e-7

inline void polyAdd (gPoint *outPolygon, unsigned &outVertices, bool lastVertex, float x, float y)
{
	if (outVertices == 0 ||
		(
			fabs(x - outPolygon[outVertices-1].pX) > EPS ||
			fabs(y - outPolygon[outVertices-1].pY) > EPS
		) && (
			!lastVertex ||
			fabs (x - outPolygon[0].pX) > EPS ||
			fabs (y - outPolygon[0].pY) > EPS
		)
	) 
	{
		outPolygon[outVertices].pX = x;
		outPolygon[outVertices].pY = y;
		outVertices++;
	}
}

void ShadowBuffer::FlushGeometry ()
  {
/*  for (unsigned i = 0; i < vCount; i += 3)
    {
    bufLog["shdwbuf.dump"]("(%f, %f, %f); (%f, %f, %f); (%f, %f, %f);\n",
      vertices[i].x, vertices[i].y, vertices[i].z,
      vertices[i+1].x, vertices[i+1].y, vertices[i+1].z,
      vertices[i+2].x, vertices[i+2].y, vertices[i+2].z);
    }*/
  }

void ShadowBuffer::cutPoly (gPoly &triangle, gPoly &result, float minx, float miny, float maxx, float maxy)
{
	gPoint outPoly1[gPoly::MAXVERTS], outPoly2[gPoly::MAXVERTS];
	gPoint *inPolygon = triangle.verts;
	gPoint *outPolygon = outPoly1;
	unsigned inVertices = triangle.vCount;
	unsigned outVertices = 0;

//	const float minx = shadow->minx;
//	const float maxx = shadow->maxx;
//	const float miny = shadow->miny;
//	const float maxy = shadow->maxy;

	do {
		// edge1 - лева€ вертикальна€ граница
		// dx = 0; dy = 1;
		{
			outVertices = 0;
			bool lastVertex = false;
			float px = inPolygon[0].pX;
			float py = inPolygon[0].pY;
			bool prevVertexInside = (px > minx);
			unsigned intersectionCount = 0;

			for (int i = 1; i <= inVertices; ++i)
			{
				float cx, cy; // - текуща€ вершина треугольника

				// инициализаци€ cx, cy
				if (i < inVertices)
				{
					cx = inPolygon[i].pX;
					cy = inPolygon[i].pY;
				} else
				{
					cx = inPolygon->pX;
					cy = inPolygon->pY;
					lastVertex = true;
				}

				// if starting vertex is visible then put it into the output array
				if (prevVertexInside) polyAdd (outPolygon, outVertices, lastVertex, px, py);

				bool curVertexInside = (cx > minx);

				// if vertices are on different sides of the edge then look where we're intersecting
				if (prevVertexInside != curVertexInside)
				{
					float denominator = cx - px;

					if (denominator != 0.0)
					{
						float t = (minx - px)/denominator;
						float tx, ty;

						if (t <= 0.0)
						{
							tx = px;
							ty = py;
						} else if (t >= 1.0)
						{
							tx = cx;
							ty = cy;
						} else
						{
							tx = px+t*(cx-px);
							ty = py+t*(cy-py);
						}

						polyAdd (outPolygon, outVertices, lastVertex, tx, ty);
					}

					if (++intersectionCount >= 2)
					{
						if (fabs(denominator) < 1) intersectionCount = 0; 
						else
						{
							// drop out, after adding all vertices left in input polygon
							if (curVertexInside)
							{
								memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(gPoint));
								outVertices += inVertices - i;
							}

							break;
						}
					}
				}

				px = cx;
				py = cy;

				prevVertexInside = curVertexInside;
			}

			// if polygon is wiped out, break

			if (outVertices < 3) break;

			// switch input/output polygons
			inVertices = outVertices;
			inPolygon = outPolygon;
			if (outPolygon == outPoly2) outPolygon = outPoly1; else outPolygon = outPoly2;
		}

		// edge2 - верхн€€ горизонтальна€ граница
		// dx = 1; dy = 0;
		{
			outVertices = 0;
			bool lastVertex = false;
			float px = inPolygon[0].pX;
			float py = inPolygon[0].pY;
			bool prevVertexInside = (py < maxy);
			unsigned intersectionCount = 0;

			for (int i = 1; i <= inVertices; ++i)
			{
				float cx, cy; // - текуща€ вершина треугольника

				// инициализаци€ cx, cy
				if (i < inVertices)
				{
					cx = inPolygon[i].pX;
					cy = inPolygon[i].pY;
				} else
				{
					cx = inPolygon->pX;
					cy = inPolygon->pY;
					lastVertex = true;
				}

				// if starting vertex is visible then put it into the output array
				if (prevVertexInside) polyAdd (outPolygon, outVertices, lastVertex, px, py);

				bool curVertexInside = (cy < maxy);

				// if vertices are on different sides of the edge then look where we're intersecting
				if (prevVertexInside != curVertexInside)
				{
					float denominator = py - cy;

					if (denominator != 0.0)
					{
						float t = (py - maxy)/denominator;
						float tx, ty;

						if (t <= 0.0)
						{
							tx = px;
							ty = py;
						} else if (t >= 1.0)
						{
							tx = cx;
							ty = cy;
						} else
						{
							tx = px+t*(cx-px);
							ty = py+t*(cy-py);
						}

						polyAdd (outPolygon, outVertices, lastVertex, tx, ty);
					}

					if (++intersectionCount >= 2)
					{
						if (fabs(denominator) < 1) intersectionCount = 0; 
						else
						{
							// drop out, after adding all vertices left in input polygon
							if (curVertexInside)
							{
								memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(gPoint));
								outVertices += inVertices - i;
							}

							break;
						}
					}
				}

				px = cx;
				py = cy;

				prevVertexInside = curVertexInside;
			}

			// if polygon is wiped out, break

			if (outVertices < 3) break;

			// switch input/output polygons
			inVertices = outVertices;
			inPolygon = outPolygon;
			if (outPolygon == outPoly2) outPolygon = outPoly1; else outPolygon = outPoly2;
		}

		// edge3 - права€ вертикальна€ граница
		// dx = 0; dy = -1;
		{
			outVertices = 0;
			bool lastVertex = false;
			float px = inPolygon[0].pX;
			float py = inPolygon[0].pY;
			bool prevVertexInside = (px < maxx);
			unsigned intersectionCount = 0;

			for (int i = 1; i <= inVertices; ++i)
			{
				float cx, cy; // - текуща€ вершина треугольника

				// инициализаци€ cx, cy
				if (i < inVertices)
				{
					cx = inPolygon[i].pX;
					cy = inPolygon[i].pY;
				} else
				{
					cx = inPolygon->pX;
					cy = inPolygon->pY;
					lastVertex = true;
				}

				// if starting vertex is visible then put it into the output array
				if (prevVertexInside) polyAdd (outPolygon, outVertices, lastVertex, px, py);

				bool curVertexInside = (cx < maxx);

				// if vertices are on different sides of the edge then look where we're intersecting
				if (prevVertexInside != curVertexInside)
				{
					float denominator = px - cx;

					if (denominator != 0.0)
					{
						float t = (px - maxx)/denominator;
						float tx, ty;

						if (t <= 0.0)
						{
							tx = px;
							ty = py;
						} else if (t >= 1.0)
						{
							tx = cx;
							ty = cy;
						} else
						{
							tx = px+t*(cx-px);
							ty = py+t*(cy-py);
						}

						polyAdd (outPolygon, outVertices, lastVertex, tx, ty);
					}

					if (++intersectionCount >= 2)
					{
						if (fabs(denominator) < 1) intersectionCount = 0; 
						else
						{
							// drop out, after adding all vertices left in input polygon
							if (curVertexInside)
							{
								memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(gPoint));
								outVertices += inVertices - i;
							}

							break;
						}
					}
				}

				px = cx;
				py = cy;

				prevVertexInside = curVertexInside;
			}

			// if polygon is wiped out, break

			if (outVertices < 3) break;

			// switch input/output polygons
			inVertices = outVertices;
			inPolygon = outPolygon;
			if (outPolygon == outPoly2) outPolygon = outPoly1; else outPolygon = outPoly2;
		}

		// edge4 - нижн€€ горизонтальна€ граница
		// dx = -1; dy = 0;
		{
			outVertices = 0;
			bool lastVertex = false;
			float px = inPolygon[0].pX;
			float py = inPolygon[0].pY;
			bool prevVertexInside = (py > miny);
			unsigned intersectionCount = 0;

			for (int i = 1; i <= inVertices; ++i)
			{
				float cx, cy; // - текуща€ вершина треугольника

				// инициализаци€ cx, cy
				if (i < inVertices)
				{
					cx = inPolygon[i].pX;
					cy = inPolygon[i].pY;
				} else
				{
					cx = inPolygon->pX;
					cy = inPolygon->pY;
					lastVertex = true;
				}

				// if starting vertex is visible then put it into the output array
				if (prevVertexInside) polyAdd (outPolygon, outVertices, lastVertex, px, py);

				bool curVertexInside = (cy > miny);

				// if vertices are on different sides of the edge then look where we're intersecting
				if (prevVertexInside != curVertexInside)
				{
					float denominator = cy - py;

					if (denominator != 0.0)
					{
						float t = (miny - py)/denominator;
						float tx, ty;

						if (t <= 0.0)
						{
							tx = px;
							ty = py;
						} else if (t >= 1.0)
						{
							tx = cx;
							ty = cy;
						} else
						{
							tx = px+t*(cx-px);
							ty = py+t*(cy-py);
						}

						polyAdd (outPolygon, outVertices, lastVertex, tx, ty);
					}

					if (++intersectionCount >= 2)
					{
						if (fabs(denominator) < 1) intersectionCount = 0; 
						else
						{
							// drop out, after adding all vertices left in input polygon
							if (curVertexInside)
							{
								memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(gPoint));
								outVertices += inVertices - i;
							}

							break;
						}
					}
				}

				px = cx;
				py = cy;

				prevVertexInside = curVertexInside;
			}
		}
	} while (0);

	// вывод результата
	if (outVertices < 3) result.vCount = 0;
	else
	{
		result.vCount = outVertices;
		memcpy (result.verts, outPolygon, outVertices*sizeof(gPoint));
	}
}

// —проецировать точку на плоскость
inline void projPoint2Surface (point3 &pt, point3 &lt, point3 &n, float D, float invltDotn)
{
	const float DIST = 0.01;
	pt -= lt*(D + n.Dot(pt))*invltDotn;
	pt += DIST*n;
}

inline void addVertex (point3 *outPolygon, unsigned &outVertices, bool lastVertex, point3 pt)
{
	if (outVertices == 0 ||
		(
			fabs(pt.x - outPolygon[outVertices-1].x) > EPS ||
			fabs(pt.y - outPolygon[outVertices-1].y) > EPS ||
			fabs(pt.z - outPolygon[outVertices-1].z) > EPS
		) && (
			!lastVertex ||
			fabs (pt.x - outPolygon[0].x) > EPS ||
			fabs (pt.y - outPolygon[0].y) > EPS ||
			fabs (pt.z - outPolygon[0].z) > EPS
		)
	) 
	{
		outPolygon[outVertices] = pt;
		outVertices++;
	}
}

// ќбрезать треугольник плоскостью
inline void chopTriangle (const point3 *inPolygon, const point3 &n, const point3 &Pos, point3 *outPolygon, unsigned &outVertices)
{
/*	outPolygon[0] = inPolygon[0];
	outPolygon[1] = inPolygon[1];
	outPolygon[2] = inPolygon[2];
	outVertices = 3;
	return;*/

	unsigned inVertices = 3;
	outVertices = 0;
	bool lastVertex = false;
	point3 prev = inPolygon[0];
	bool prevVertexInside = (n.Dot(prev - Pos) > 0);
	unsigned intersectionCount = 0;

	for (int i = 0; i <= inVertices; ++i)
	{
		point3 cur;

		if (i < inVertices)
		{
			cur = inPolygon[i];
		} else
		{
			cur = *inPolygon;
			lastVertex = true;
		}

		if (prevVertexInside) addVertex (outPolygon, outVertices, lastVertex, prev);

		bool curVertexInside = (n.Dot(cur - Pos) > 0);

		if (prevVertexInside != curVertexInside)
		{
			float denominator = n.Dot(cur - prev);

			if (denominator != 0.0)
			{
				float t = n.Dot (Pos - prev) / denominator;
				point3 temp;

				if (t <= 0.0f)
				{
					temp = prev;
				} else if (t >= 1.0f)
				{
					temp = cur;
				} else
				{
					temp = prev + t*(cur - prev);
				}

				addVertex (outPolygon, outVertices, lastVertex, temp);
			}

			if (++intersectionCount >= 2)
			{
				if (fabs(denominator) < 1) intersectionCount = 0;
				else
				{
					if (curVertexInside)
					{
						memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(point3));
						outVertices += inVertices - i;
					}

					break;
				}
			}
		}

		prev = cur;
		prevVertexInside = curVertexInside;

	}

	if (outVertices < 3) outVertices = 0;
}

//  ак относительно плоскости расположен параллелепипед
// -1 - позади нормали
// 0 - на обеих сторонах
// 1 - спереди нормали
inline int TestPlanes (const point3 *originz, const Plane *p)
{
//	float testDot = p->Normal.Dot (n);
	float maxd, mind;
	float d1;

	maxd = mind = d1 = p->TestPoint(originz[0]);
	for (int i = 1; i < 8; ++i)
	{
		d1 = p->TestPoint (originz[i]);
		if (maxd < d1) maxd = d1; else if (mind > d1) mind = d1;
	}

//	maxd += 1; mind -= 1;

	if (maxd <= 0) return -1;
	if (mind >= 0)return 1;

	return 0;
}

// Ќаходитс€ ли треугольник в параллелепипеде тени
inline bool cullTriangle (const point3 *triangle, const point3 &x, const point3 &y, const point3 &z, const point3 *originz, float *Rad)
{
	float dot1, dot2, dot3, tdot;

	tdot = x.Dot (*originz);
	dot1 = x.Dot (triangle[0]) - tdot;
	dot2 = x.Dot (triangle[1]) - tdot;
	dot3 = x.Dot (triangle[2]) - tdot;

	if ((dot1 < 0) && (dot2 < 0) && (dot3 < 0)) return true;
	if ((-dot1 + Rad[0] < 0) && (-dot2 + Rad[0] < 0) && (-dot3 + Rad[0] < 0)) return true;

	tdot = y.Dot (*originz);
	dot1 = y.Dot (triangle[0]) - tdot;
	dot2 = y.Dot (triangle[1]) - tdot;
	dot3 = y.Dot (triangle[2]) - tdot;

	if ((dot1 < 0) && (dot2 < 0) && (dot3 < 0)) return true;
	if ((-dot1 + Rad[1] < 0) && (-dot2 + Rad[1] < 0) && (-dot3 + Rad[1] < 0)) return true;

	tdot = z.Dot (*originz);
	dot1 = z.Dot (triangle[0]) - tdot;
	dot2 = z.Dot (triangle[1]) - tdot;
	dot3 = z.Dot (triangle[2]) - tdot;

	if ((dot1 < 0) && (dot2 < 0) && (dot3 < 0)) return true;
	if ((-dot1 + Rad[2] < 0) && (-dot2 + Rad[2] < 0) && (-dot3 + Rad[2] < 0)) return true;

	return false;
}

void ShadowBuffer::CollectPlanes (BaseShadow *shadow, float Rad, int CurIndex)
{
	static D3DCOLOR intensity;

	// “очка прив€зки
	static point3 Pos;

	static point3 bx, by;
	static point3 tpos;
	static point3 originz[8];
	static float radz[3];
	static float invDeltax, invDeltay;

	if (!CurIndex)
	{
		vCount = 0;
		point3 bind = *shadow->bindPoint;// - shadow->lt*10.0f;

		if (shadow->parentWorld)
		{
			// Ќайдем мировые координаты точки прив€зки
			Pos.x = bind.x * shadow->parentWorld->_11 +
					bind.y * shadow->parentWorld->_21 + 
					bind.z * shadow->parentWorld->_31 + 
										   shadow->parentWorld->_41;
			Pos.y = bind.x * shadow->parentWorld->_12 +
					bind.y * shadow->parentWorld->_22 + 
					bind.z * shadow->parentWorld->_32 + 
										   shadow->parentWorld->_42;
			Pos.z = bind.x * shadow->parentWorld->_13 +
					bind.y * shadow->parentWorld->_23 +
					bind.z * shadow->parentWorld->_33 +
										   shadow->parentWorld->_43;
		} else 
		{
			Pos.x = bind.x;
			Pos.y = bind.y;
			Pos.z = bind.z;
		}

		intensity = int(float(0xFF)*ShadowUtility::getLightIntensity (Pos)) << 24;

		bx = point3(shadow->axis._11, shadow->axis._21, shadow->axis._31);
		by = point3(shadow->axis._12, shadow->axis._22, shadow->axis._32);

		tpos = shadow->lt * shadow->lt.Dot(Pos);

		originz[0] = shadow->minx*bx + shadow->miny*by + tpos;
		originz[1] = shadow->minx*bx + shadow->maxy*by + tpos;
		originz[2] = shadow->maxx*bx + shadow->maxy*by + tpos;
		originz[3] = shadow->maxx*bx + shadow->miny*by + tpos;

		originz[4] = shadow->minx*bx + shadow->miny*by + tpos + shadow->lt*Rad;
		originz[5] = shadow->minx*bx + shadow->maxy*by + tpos + shadow->lt*Rad;
		originz[6] = shadow->maxx*bx + shadow->maxy*by + tpos + shadow->lt*Rad;
		originz[7] = shadow->maxx*bx + shadow->miny*by + tpos + shadow->lt*Rad;

		radz[0] = shadow->maxx - shadow->minx;
		radz[1] = shadow->maxy - shadow->miny;
		radz[2] = Rad;

		invDeltax = 1/(shadow->maxx - shadow->minx);
		invDeltay = 1/(shadow->maxy - shadow->miny);
	}

	if (vCount >= MAXTRIANGLES*3) return;

	Bsp *node=&Bsp::Nodes[CurIndex];
	Plane *p=Bsp::Planes+node->PlaneIdx;
	
	point3 altLt = point3(shadow->lt.x, shadow->lt.y, 0);
	/* by Flif float LTonNp = altLt.Dot(p->Normal);*/ // проекци€ вектора направлени€ света на нормаль
	float aLTonNp = shadow->lt.Dot(p->Normal); // проекци€ вектора направлени€ света на нормаль

	switch (TestPlanes (originz, p))
	{
		case -1:
			if (node->Back) CollectPlanes (shadow, Rad, node->Back);
			return;
		case 1:
			if (node->Front) CollectPlanes (shadow, Rad, node->Front);
			return;
	}

	if (node->Front) CollectPlanes (shadow, Rad, node->Front);
	if (vCount >= MAXTRIANGLES*3) return;
	if (node->Back) CollectPlanes (shadow, Rad, node->Back);
	if (vCount >= MAXTRIANGLES*3) return;

	{
		int i, iend, j;
		unsigned *faceIterator, resTVerts;
		point3 triVerts[3], resT[4];
		gPoly triangle, poly;
		bool flag = false;

		if (aLTonNp < 0)
		{
			i = node->StartFace;
			iend = node->FrontNum + node->StartFace;
		} else 
		{
			flag = true;
			i = node->FrontNum + node->StartFace;
			iend = node->AllFacesNum + node->StartFace;
		}

		point3 t0;
		point3 t, tp;
		point3 Normal = flag?-p->Normal:p->Normal;
		float curD = flag?-p->D:p->D;
		float invltDotn = 1/shadow->lt.Dot(Normal);

		for (faceIterator = Bsp::Faces + i*3; i < iend; i++)
		{
			int shd=Bsp::FaceShd[i];

			if(!(Bsp::ShdFlags[shd]&Bsp::SHDPICK))    continue;
			if(!(Bsp::ShdFlags[shd]&Bsp::SHDSHADOW))  continue;

			triVerts[0] = (Bsp::Verts + *faceIterator)->Pos; faceIterator++;
			triVerts[1] = (Bsp::Verts + *faceIterator)->Pos; faceIterator++;
			triVerts[2] = (Bsp::Verts + *faceIterator)->Pos; faceIterator++;

			// “реугольник полностью находитс€ за параллелепипедом отсечени€
			if (cullTriangle (triVerts, bx, by, shadow->lt, originz, radz)) continue;

			chopTriangle (triVerts, shadow->lt, Pos, resT, resTVerts);

			if (resTVerts < 3) continue;

			// найдем локальные координаты треугольника
			for (j = 0; j < resTVerts; ++j)
			{
				triangle.verts[j].pX = resT[j].Dot (bx);
				triangle.verts[j].pY = resT[j].Dot (by);
			}

			triangle.vCount = resTVerts;
			cutPoly (triangle, poly, shadow->minx, shadow->miny, shadow->maxx, shadow->maxy);

			t0 = bx*poly.verts->pX + by*poly.verts->pY;
			projPoint2Surface (t0, shadow->lt, Normal, curD, invltDotn);
			tp = bx*poly.verts[1].pX + by*poly.verts[1].pY;
			projPoint2Surface (tp, shadow->lt, Normal, curD, invltDotn);

			if (poly.vCount >= 3)
			for (j = 0; j < poly.vCount - 2; j++)
			{
				t = bx*poly.verts[j+2].pX + by*poly.verts[j+2].pY;
				projPoint2Surface (t, shadow->lt, Normal, curD, invltDotn);

				m_pos[vCount] = t0;
				m_diffuse[vCount] = intensity;
				m_texcoords[vCount].u = (poly.verts[0].pX - shadow->minx)*invDeltax;
				m_texcoords[vCount].v = 1.0f-(poly.verts[0].pY - shadow->miny)*invDeltay;
				vCount++;
				if(vCount>=MAXTRIANGLES*3) return;

				m_pos[vCount] = tp;
				m_diffuse[vCount] = intensity;
				m_texcoords[vCount].u = (poly.verts[j+1].pX - shadow->minx)*invDeltax;
				m_texcoords[vCount].v = 1.0f-(poly.verts[j+1].pY - shadow->miny)*invDeltay;
				vCount++;
				if(vCount>=MAXTRIANGLES*3) return;

				m_pos[vCount] = t;
				m_diffuse[vCount] = intensity;
				m_texcoords[vCount].u = (poly.verts[j+2].pX - shadow->minx)*invDeltax;
				m_texcoords[vCount].v = 1.0f-(poly.verts[j+2].pY - shadow->miny)*invDeltay;
				vCount++;
				if(vCount>=MAXTRIANGLES*3) return;

/*				vertices[vCount++] = D3DLVERTEX (
													D3DVECTOR (t0.x, t0.y, t0.z),
													intensity, 0,
													(poly.verts[0].pX - shadow->minx)*invDeltax,
													1-(poly.verts[0].pY - shadow->miny)*invDeltay
												);

				if (vCount >= MAXTRIANGLES*3) return;

				vertices[vCount++] = D3DLVERTEX (
													D3DVECTOR (tp.x, tp.y, tp.z),
													intensity, 0,
													(poly.verts[j+1].pX - shadow->minx)*invDeltax,
													1-(poly.verts[j+1].pY - shadow->miny)*invDeltay
												);

				if (vCount >= MAXTRIANGLES*3) return;

				vertices[vCount++] = D3DLVERTEX (
													D3DVECTOR (t.x, t.y, t.z),
													intensity, 0,
													(poly.verts[j+2].pX - shadow->minx)*invDeltax,
													1-(poly.verts[j+2].pY - shadow->miny)*invDeltay
												);*/

//				if (vCount >= MAXTRIANGLES*3) return;

				tp = t;
			}
		}
	}
}

}; // namespace