#include "precomp.h"
#include "chop.h"

float Chopper::EPS = 1e-7;

bool Chopper::Chop (const point3 *InPolygon, int InVertsCount,
					const point3 &Normal, const point3 &Origin,
					point3 *OutPolygon, int &OutVertsCount)
{
	OutVertsCount = 0;
	bool lastVertex = false;
	point3 prev = InPolygon[0];
	bool prevVertexInside = (Normal.Dot(prev - Origin) > 0);
	unsigned intersectionCount = 0;

	for (int i = 0; i <= InVertsCount; ++i)
	{
		point3 cur;

		if (i < InVertsCount)
		{
			cur = InPolygon[i];
		} else
		{
			cur = *InPolygon;
			lastVertex = true;
		}

		if (prevVertexInside) AddVertex (OutPolygon, OutVertsCount, lastVertex, prev);

		bool curVertexInside = (Normal.Dot(cur - Origin) > 0);

		if (prevVertexInside != curVertexInside)
		{
			float denominator = Normal.Dot(cur - prev);

			if (denominator != 0.0)
			{
				float t = Normal.Dot (Origin - prev) / denominator;
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

				AddVertex (OutPolygon, OutVertsCount, lastVertex, temp);
			}

			if (++intersectionCount >= 2)
			{
				if (fabs(denominator) < 1) intersectionCount = 0;
				else
				{
					if (curVertexInside)
					{
						memcpy (OutPolygon + OutVertsCount, InPolygon + i, (InVertsCount-i)*sizeof(point3));
						OutVertsCount += InVertsCount - i;
					}

					break;
				}
			}
		}

		prev = cur;
		prevVertexInside = curVertexInside;

	}

	if (OutVertsCount < 3) {OutVertsCount = 0;return false;}

	return true;
}

void Chopper::AddVertex (point3 *OutPolygon, int &OutVertsCount, 
						 bool LastVertex, point3 Pt)
{
	if (OutVertsCount == 0 ||
		(
			fabs(Pt.x - OutPolygon[OutVertsCount-1].x) > EPS ||
			fabs(Pt.y - OutPolygon[OutVertsCount-1].y) > EPS ||
			fabs(Pt.z - OutPolygon[OutVertsCount-1].z) > EPS
		) && (
			!LastVertex ||
			fabs (Pt.x - OutPolygon[0].x) > EPS ||
			fabs (Pt.y - OutPolygon[0].y) > EPS ||
			fabs (Pt.z - OutPolygon[0].z) > EPS
		)
	) 
	{
		OutPolygon[OutVertsCount] = Pt;
		OutVertsCount++;
	}
}
