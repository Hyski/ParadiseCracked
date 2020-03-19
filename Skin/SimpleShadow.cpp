#include "precomp.h"

#include "../Common/GraphPipe/culling.h"
#include "../GameLevel/bsp.h"
#include "../Gamelevel/Gamelevel.h"
#include "../iworld.h"
#include "../gamelevel/grid.h"
#include "Shadow.h"

#define MAX_ITERATIONS	50

namespace Shadows
{

SimpleShadow::SimpleShadow (RealEntity *owner) : BaseShadow(owner)
{
	m_shader = "person_simple_shadow";

	txtSimple = "";
}

SimpleShadow::SimpleShadow (BaseShadow *elder) : BaseShadow(elder)
{
	m_shader = "person_simple_shadow";

	txtSimple = "";
	for (std::list<SimpleTexturedObject *>::iterator i = elder->objects.begin(); i != elder->objects.end(); ++i)
	{
		AddObject (*i);
	}
}

SimpleShadow::~SimpleShadow ()
{
}

void SimpleShadow::SetShadowTexture (const std::string &name)
{
	txtSimple = name;
}

void SimpleShadow::Update (float t)
{
	if (!entity->Visible || !entity->Enabled) return;

	Timer::Update ();
	float time = Timer::GetSeconds ();

	if ((t - tLastCalc > tRedrawPeriod*GetKoef(t)) || !Updated || NeedUpdate)
	{
		tLastCalc = t;
		GameLevel::LevelGridptr grid = IWorld::Get()->GetLevel()->LevelGrid;

		if(grid)
		{
			SetLight (
				ShadowUtility::getLightDirection (
					point3 (
						entity->GetLocation().x,
						entity->GetLocation().y,
						grid->Height(entity->GetLocation())
					)
				)
			);
		}

		// Обновить тень
		Calc ();
    Updated = true;
	NeedUpdate = false;
	}

	Timer::Update();
	time = Timer::GetSeconds () - time;

	ShadowUtility::AddTime (Timer::GetSeconds(), time);
}

void SimpleShadow::Draw (GraphPipe *pipe)
{
	if (!entity->Visible || !entity->Enabled) return;
	if (!objects.size()) return;

	if (NeedUpdate) Update(-1.0f);

	if ((!bindPoint) || (txtSimple == "")) return;

	LPDIRECTDRAWSURFACE7 surface = TextureMgr::Instance()->Texture(txtSimple.c_str());

	if (surface) 
	{
		StatesManager::SetTexture(0,surface);
		if (Updated) {cache.CollectPlanes(this, 5); Updated = false;}
		cache.Draw (pipe, m_shader);
	}
}

void SimpleShadow::AddObject (const TexObject *obj)
{
	BaseShadow::AddObject(obj);
}

void SimpleShadow::AddObject (SimpleTexturedObject *obj)
{
	BaseShadow::AddObject(obj);
}

void SimpleShadow::Clear ()
{
	BaseShadow::Clear ();
}

void SimpleShadow::Calc ()
{
	if (!objects.size()) return;

	SimpleTexturedObject *current;
	point3 *curpt;
	int /*pcount=0,pc=0,*/i;
	float x, y, z;
	float /*tl, */ltx, lty, ltz;
	float curx, cury;

	ltx = lt.x;//invParentWorld->_11*lt.x + invParentWorld->_21*lt.y + invParentWorld->_31*lt.z;
	lty = lt.y;//invParentWorld->_12*lt.x + invParentWorld->_22*lt.y + invParentWorld->_32*lt.z;
	ltz = lt.z;//invParentWorld->_13*lt.x + invParentWorld->_23*lt.y + invParentWorld->_33*lt.z;

	x = (*objects.begin())->Points->x;
	y = (*objects.begin())->Points->y;
	z = (*objects.begin())->Points->z;

	D3DMATRIX m;

	if (parentWorld) m = (*parentWorld)*axis; else m = axis;

	curx = m._11*x + m._21*y + m._31*z + m._41;
	cury = m._12*x + m._22*y + m._32*z + m._42;
	z    = m._13*x + m._23*y + m._33*z + m._43;

	minx = maxx = curx;
	miny = maxy = cury;

	bindPoint = (*objects.begin())->Points;

	d = z;
//	d = curx*ltx + cury*lty + z*ltz;

	for (std::list<SimpleTexturedObject *>::iterator ist = objects.begin(); ist != objects.end(); ++ist)
	{
		current = *ist;
		curpt = current->Points;

		for (i = 0; i < current->PntNum; i += 3, curpt += 3)
		{
			x = curpt->x;
			y = curpt->y;
			z = curpt->z;

			curx = m._11*x + m._21*y + m._31*z + m._41;
			cury = m._12*x + m._22*y + m._32*z + m._42;
			z    = m._13*x + m._23*y + m._33*z + m._43;

			if (minx > curx) 
			{
				minx = curx;
			} else if (maxx < curx) 
			{
				maxx = curx;
			}
			
			if (miny > cury)
			{
				miny = cury;
			} else if (maxy < cury)
			{
				maxy = cury;
			}

			if (d > z)
			{
				bindPoint = curpt;
				d = z;
			}

		}
	}

}

} // namespace