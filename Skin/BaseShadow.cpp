#include "precomp.h"

#include "../Common/GraphPipe/culling.h"
#include "../GameLevel/bsp.h"
#include "Shadow.h"

#define MAX_ITERATIONS	50

namespace Shadows
  {
unsigned int BaseShadow::cntShdwGen = 0;
const std::string shdwPrefix = "gvz_st_";

BaseShadow::BaseShadow (RealEntity *owner)
{
	char tmprr[11];

	this->bindPoint = 0;

	itoa (this->cntShdwGen++, tmprr, 16);
	this->txtName = shdwPrefix + tmprr;

	ShadowUtility::AddShadow (this->txtName, this);

	parentWorld = &owner->World;
	invParentWorld = &owner->InvWorld;
	tRedrawPeriod = Options::GetFloat("system.video.shadowrate");

	entity = owner;

	tLastCalc = 0;
	deadTime = -1;

	Updated = false;
	NeedUpdate = true;

	GVZ_LOG("Created shadow\n");
}

BaseShadow::BaseShadow (BaseShadow *elder)
{
	this->bindPoint = elder->bindPoint;

	this->txtName = elder->txtName;

	parentWorld = elder->parentWorld;
	invParentWorld = elder->invParentWorld;
	tRedrawPeriod = Options::GetFloat("system.video.shadowrate");

	entity = elder->entity;

	tLastCalc = 0;
	deadTime = -1;

	entity->shadow = this;

	Updated = false;
	NeedUpdate = true;
	GVZ_LOG("ReCreated shadow\n");
}

BaseShadow::~BaseShadow ()
{
	if (!ShadowUtility::NoKill()) ShadowUtility::KillShadow (this->txtName);
	GVZ_LOG("Killed shadow\n");
}

void BaseShadow::SetRedrawPeriod (float p)
{
	tRedrawPeriod = Options::GetFloat("system.video.shadowrate")*p;
}

void BaseShadow::AddObject (const TexObject *obj)
{
	GVZ_LOG("Adding object to %s...\n", txtName.c_str());
	for (int i = 0; i < obj->PartNum; ++i) AddObject(obj->Parts[i]);
	NeedUpdate = true;
}

void BaseShadow::AddObject (SimpleTexturedObject *obj)
{
	GVZ_LOG("Adding object to %s...\n", txtName.c_str());
  if (!obj->PntNum) GVZ_LOG("\t(Not added because it is void)\n");
  else
    {
	  objects.push_back (obj);
	  NeedUpdate = true;
    }
}

void BaseShadow::Clear ()
{
	objects.clear();
	NeedUpdate = true;
	GVZ_LOG("Deleted all objects in %s.\n", txtName.c_str());
}

float BaseShadow::GetKoef (float t)
{
	float koef = 1;
	if (entity->CurState & (LAZY|STAY)) koef = 10;
	if (entity->CurState & DEAD)
	{
		if (deadTime < 0)
		{
			deadTime = t;
		} else if ((t - deadTime) > 10)
		{
			koef = 1000;
		}
	}

	return koef;
}

//=====================================================================================//
//                            void BaseShadow::SetLight ()                             //
//=====================================================================================//
void BaseShadow::SetLight (point3 &light)
{
	lt = light/light.Length();

/*	float tl = lt.x*lt.x + lt.y*lt.y;

	point3 x(lt.y, -lt.x,0);
	point3 y(lt.x*lt.z, lt.y*lt.z, -tl);

	x = x/x.Length();
	y = y/y.Length();*/

	point3 x = lt.Cross(point3(1.0f,0.0f,0.0f));
	if(x.Length() < 0.01f) x = lt.Cross(point3(0.0f,1.0f,0.0f));
	point3 y = lt.Cross(x);

	x = x / x.Length();
	y = y / y.Length();

	axis = D3DMATRIX(x.x, y.x, lt.x,  0,
				     x.y, y.y, lt.y,  0,
				     x.z, y.z, lt.z,  0,
				       0,   0,    0,  1);
}

} // namespace