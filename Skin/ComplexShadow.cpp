#include "precomp.h"

#if 0

#include "../Common/GraphPipe/culling.h"
#include "../GameLevel/bsp.h"
#include "../Gamelevel/Gamelevel.h"
#include "../iworld.h"
#include "../gamelevel/grid.h"
#include "Shadow.h"

#include "../common/graphpipe/vshader.h"

#include <fstream>
#include <mll/utils/timer.h>

#define gvzShadowZero	0x00000000
#define gvzShadowOne	0xAF000000

namespace Shadows
{

struct tline
{
	int		bi, li;		// индексы точек
	int		ti[sc_line::MAXNBRS];// индексы соседних точек
	int		ic;			// кол-во ^ индексов
	int		ct;			// кол-во отрисовок
};

ComplexShadow::ComplexShadow (RealEntity *owner) : BaseShadow (owner)
{
	m_shader = "person_complex_shadow";
	if (Options::GetInt("system.video.shadowquality") > 8) throw CASUS("Слишком высокое качество теней\nСтавьте 8 или меньше");

	TEXTX = 1 << Options::GetInt("system.video.shadowquality");
	TEXTY = 1 << Options::GetInt("system.video.shadowquality");
	TXS = Options::GetInt("system.video.shadowquality");

	linez = 0;
	nodez = 0;
	lnCount = 0;
	ndCount = 0;
	bLinez = false;

//	buf = (short *)_aligned_malloc (TEXTX*TEXTY, 16);
	buf = new short int[TEXTX*TEXTY];

	// Создадим DIBData
	dbdTexture = new DIBData;
	dbdTexture->Create (TEXTX, TEXTY, 32);

	shdwSurface = TextureMgr::Instance()->CreateTexture (txtName.c_str(), dbdTexture, 0, 0);
//	shdwSurface = TextureMgr::Instance()->Texture (txtName.c_str());

	GVZ_LOG("Created %s\n", txtName.c_str ());
}

ComplexShadow::ComplexShadow (BaseShadow *elder) : BaseShadow (elder)
{
	m_shader = "person_complex_shadow";

	if (Options::GetInt("system.video.shadowquality") > 8) throw CASUS("Слишком высокое качество теней\nСтавьте 8 или меньше");

	TEXTX = 1 << Options::GetInt("system.video.shadowquality");
	TEXTY = 1 << Options::GetInt("system.video.shadowquality");
	TXS = Options::GetInt("system.video.shadowquality");

	linez = 0;
	nodez = 0;
	lnCount = 0;
	ndCount = 0;
	bLinez = false;

//	buf = (short *)_aligned_malloc (TEXTX*TEXTY, 16);
	buf = new short int[TEXTX*TEXTY];

	// Создадим DIBData
	dbdTexture = new DIBData;
	dbdTexture->Create (TEXTX, TEXTY, 32);

	TextureMgr::Instance()->Release (txtName.c_str());
	shdwSurface = TextureMgr::Instance()->CreateTexture (txtName.c_str(), dbdTexture, 0, 0);
//	shdwSurface = TextureMgr::Instance()->Texture (txtName.c_str());

	for (std::list<SimpleTexturedObject *>::iterator i = elder->objects.begin(); i != elder->objects.end(); ++i)
	{
		AddObject (*i);
	}


	GVZ_LOG("Created %s\n", txtName.c_str ());
}

ComplexShadow::~ComplexShadow ()
{
	if (linez) delete [] linez;
	if (nodez) delete [] nodez;
	if (buf) delete [] buf;//_aligned_free (buf); 
	if (dbdTexture) delete dbdTexture;

	if (!ShadowUtility::NoKill()) TextureMgr::Instance()->Release (txtName.c_str());

	GVZ_LOG("Destroyed %s\n", txtName.c_str());
}

void ComplexShadow::Update (float t)
{
	if (!entity->Visible || !entity->Enabled) return;

	Timer::Update ();
	float time = Timer::GetSeconds ();

	if ((t - tLastCalc > tRedrawPeriod*GetKoef(t)) || !Updated || (t<0.0f) || NeedUpdate)
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
		sc_line *cursor = linez;

		for (int j = 0; j < lnCount; j++, cursor++) cursor->ct = 0;

		Calc ();
		BlitLinez ();
		BlitBuffer (dbdTexture->Image ());

		// Занести DIBData в текстуру
		if (shdwSurface) ShadowUtility::CopyImageToSurface (this);

		Updated = true;
		NeedUpdate = false;
	}

	Timer::Update();
	time = Timer::GetSeconds () - time;

	ShadowUtility::AddTime (Timer::GetSeconds(), time);
}

void ComplexShadow::Draw (GraphPipe *Pipe)
{
	if (!entity->Visible || !entity->Enabled) return;
	if (!this->ndCount) return;

	if (NeedUpdate) Update(-1.0f);

	if (!bindPoint) return;

	mll::utils::timer timer;
	//static std::ofstream tmp("gvz_perf.log");
	{
		mll::utils::time t1 = timer.get();
		StatesManager::SetTexture(0,shdwSurface);
		t1 = timer.get() - t1;
		//tmp << "StatesManager::SetTexture - " << (float)t1 << "\n";
		//tmp << std::flush;
	}

	if (Updated)
	{
		mll::utils::time t1 = timer.get();
		cache.CollectPlanes(this, 5); Updated = false;
		t1 = timer.get() - t1;
		//tmp << "cache.CollectPlanes - " << (float)t1 << "\n";
		//tmp << std::flush;
	}

	{
		mll::utils::time t1 = timer.get();
		cache.Draw (Pipe, m_shader);
		t1 = timer.get() - t1;
		//tmp << "cache.Draw - " << (float)t1 << "\n";
		//tmp << std::flush;
	}
}	

void PushLine (tline *lns, int &count, int idx1, int idx2, int idx3)
{
	int i;
	bool flag = false;
	int c = 0;

	for (i = 0; (i < count) && (!flag); i++)
	{
		if (((lns[i].bi == idx1) && (lns[i].li == idx2)) ||
			((lns[i].li == idx1) && (lns[i].bi == idx2))) 
		{
			flag = true;
			c = i;
		}
	}

	if (flag) 
	{
		if (lns[c].ic < sc_line::MAXNBRS)
		{
			lns[c].ti[lns[c].ic++] = idx3;
		}/* else {
			throw CASUS("Слишком много граничащих треугольников");
		}*/

		return;
	}

	lns[count].bi = idx1;
	lns[count].li = idx2;
	lns[count].ic = 1;
	lns[count].ti[0] = idx3;
	lns[count].ct = 0;

	count += 1;

	return;
}

void ComplexShadow::AddObject (const TexObject *o)
{
	for (int i = 0; i < o->PartNum; ++i) AddObject (o->Parts[i]);

/*	int i, j;
	SimpleTexturedObject *so=0;
	sc_node *tndb=0, *tnodez=0;
	sc_line *tlnb=0; 
	tline *tlinez=0;
	int nodezCount = 0;
	int linezCount = 0;
	int *nodezIdxs = 0;
	int cnIndex;

	nodezIdxs = new int[o->PartNum];

	for (i = 0; i < o->PartNum; i++)
	{
		nodezIdxs[i] = nodezCount;
		nodezCount += o->Parts[i]->PntNum;
		linezCount += o->Parts[i]->IdxNum;
	}

	tnodez = new sc_node [nodezCount];
	tlinez = new tline [linezCount];


	linezCount = 0;

	for (i = 0, cnIndex = 0; i < o->PartNum; i++)
	{
		so = o->Parts[i];

		for (j = 0; j < so->PntNum; j++, cnIndex++)
		{
			tnodez[cnIndex].ptr = &so->Points[j];
		}

		for (j = 0; j < so->IdxNum; j += 3)
		{
			PushLine (tlinez, linezCount, so->GetIndexesRO()[ j ] + ndCount + nodezIdxs[i],
										  so->GetIndexesRO()[j+1] + ndCount + nodezIdxs[i],
										  so->GetIndexesRO()[j+2] + ndCount + nodezIdxs[i]);
			PushLine (tlinez, linezCount, so->GetIndexesRO()[j+1] + ndCount + nodezIdxs[i],
										  so->GetIndexesRO()[j+2] + ndCount + nodezIdxs[i],
										  so->GetIndexesRO()[ j ] + ndCount + nodezIdxs[i]);
			PushLine (tlinez, linezCount, so->GetIndexesRO()[j+2] + ndCount + nodezIdxs[i],
										  so->GetIndexesRO()[ j ] + ndCount + nodezIdxs[i],
										  so->GetIndexesRO()[j+1] + ndCount + nodezIdxs[i]);
		}
	}

// optimization phase

	if (nodez) {
		tndb = nodez;
		nodez = new sc_node [nodezCount + ndCount];
		memcpy (nodez, tndb, sizeof(sc_node)*ndCount);

		for (i = 0; i < nodezCount; i++)
		{
			nodez[ndCount+i].ptr = tnodez[i].ptr;
		}
	} else {
		nodez = new sc_node [nodezCount];

		for (i = 0; i < nodezCount; i++)
		{
			nodez[i].ptr = tnodez[i].ptr;
		}
	}

	tlnb = linez;
	linez = new sc_line [linezCount + lnCount];
	memcpy (linez, tlnb, sizeof(sc_line)*lnCount);

	for (i = 0; i < lnCount; i++)
	{
		linez[i].bi = nodez + linez[i].bIdx;
		linez[i].li = nodez + linez[i].lIdx;

		for (int j = 0; j < linez[i].nbrCount; ++j)
		{
			linez[i].ti[j] = nodez + linez[i].tIdx[j];
		}
	}

	for (i = 0; i < linezCount; i++)
	{
		linez[lnCount+i].bIdx = tlinez[i].bi;
		linez[lnCount+i].lIdx = tlinez[i].li;

		linez[lnCount+i].bi = nodez + tlinez[i].bi;
		linez[lnCount+i].li = nodez + tlinez[i].li;

		for (int j = 0; j < tlinez[i].ic; ++j)
		{
			linez[lnCount+i].tIdx[j] = tlinez[i].ti[j];
			linez[lnCount+i].ti[j] = nodez + tlinez[i].ti[j];
		}

		linez[lnCount+i].nbrCount = tlinez[i].ic;
	}

	ndCount += nodezCount;
	lnCount += linezCount;

	if (tndb) delete [] tndb;
	if (tnodez) delete [] tnodez;
	if (tlnb) delete [] tlnb;
	if (tlinez) delete [] tlinez;
	if (nodezIdxs) delete [] nodezIdxs;*/
}

unsigned ComplexShadow::describe() const
{
	return lnCount;
}

void ComplexShadow::AddObject (SimpleTexturedObject *so)
{
	BaseShadow::AddObject (so);

  if (!so->PntNum) return;

	int i, j;
	sc_node *tndb=0, *tnodez=0;
	sc_line *tlnb=0; 
	tline *tlinez=0;
	int nodezCount = 0;
	int linezCount = 0;
	int nodezIdxs = 0;
	int cnIndex = 0;

	nodezIdxs = nodezCount;
	nodezCount += so->PntNum;
	linezCount += so->IdxNum;

	tnodez = new sc_node [nodezCount];
	tlinez = new tline [linezCount];

	linezCount = 0;

		for (j = 0; j < so->PntNum; j++, cnIndex++)
		{
			tnodez[cnIndex].ptr = &so->Points[j];
		}

		for (j = 0; j < so->IdxNum; j += 3)
		{
			PushLine (tlinez, linezCount, so->GetIndexesRO()[ j ] + ndCount,
										  so->GetIndexesRO()[j+1] + ndCount,
										  so->GetIndexesRO()[j+2] + ndCount);
			PushLine (tlinez, linezCount, so->GetIndexesRO()[j+1] + ndCount,
										  so->GetIndexesRO()[j+2] + ndCount,
										  so->GetIndexesRO()[ j ] + ndCount);
			PushLine (tlinez, linezCount, so->GetIndexesRO()[j+2] + ndCount,
										  so->GetIndexesRO()[ j ] + ndCount,
										  so->GetIndexesRO()[j+1] + ndCount);
		}

// optimization phase

	if (nodez) {
		tndb = nodez;
		nodez = new sc_node [nodezCount + ndCount];
		memcpy (nodez, tndb, sizeof(sc_node)*ndCount);

		for (i = 0; i < nodezCount; i++)
		{
			nodez[ndCount+i].ptr = tnodez[i].ptr;
		}
	} else {
		nodez = new sc_node [nodezCount];

		for (i = 0; i < nodezCount; i++)
		{
			nodez[i].ptr = tnodez[i].ptr;
		}
	}

	tlnb = linez;
	linez = new sc_line [linezCount + lnCount];
	memcpy (linez, tlnb, sizeof(sc_line)*lnCount);

	for (i = 0; i < lnCount; i++)
	{
		linez[i].bi = nodez + linez[i].bIdx;
		linez[i].li = nodez + linez[i].lIdx;

		for (int j = 0; j < linez[i].nbrCount; ++j)
		{
			linez[i].ti[j] = nodez + linez[i].tIdx[j];
		}
	}

	for (i = 0; i < linezCount; i++)
	{
		linez[lnCount+i].bIdx = tlinez[i].bi;
		linez[lnCount+i].lIdx = tlinez[i].li;

		linez[lnCount+i].bi = nodez + tlinez[i].bi;
		linez[lnCount+i].li = nodez + tlinez[i].li;

		for (int j = 0; j < tlinez[i].ic; ++j)
		{
			linez[lnCount+i].tIdx[j] = tlinez[i].ti[j];
			linez[lnCount+i].ti[j] = nodez + tlinez[i].ti[j];
		}

		linez[lnCount+i].nbrCount = tlinez[i].ic;
	}

	ndCount += nodezCount;
	lnCount += linezCount;

	if (tndb) delete [] tndb;
	if (tnodez) delete [] tnodez;
	if (tlnb) delete [] tlnb;
	if (tlinez) delete [] tlinez;
}

void ComplexShadow::Clear ()
{
	BaseShadow::Clear ();

	GVZ_LOG("Deleted all objects in %s...\n", txtName.c_str());
	if (linez)
	{
		lnCount = 0;
		delete [] linez;
		linez = 0;
	}

	if (nodez)
	{
		ndCount = 0;
		delete [] nodez;
		nodez = 0;
	}
}

void ComplexShadow::UpdateOnModeChange ()
{
	shdwSurface = TextureMgr::Instance()->CreateTexture (txtName.c_str(), dbdTexture, 0, 0);
//	shdwSurface = TextureMgr::Instance()->Texture (txtName.c_str());
	Updated = false;

	GVZ_LOG("    %s\n", txtName.c_str());
}

void ComplexShadow::Calc ()
{
	int /*pcount=0,pc=0,*/i;
	float scalex, scaley;
	float x, y, z;
	sc_node *cnode;
	sc_line *cline;
	float ltx, lty, ltz;

	if (!ndCount) return;

	ltx = lt.x;
	lty = lt.y;
	ltz = lt.z;

	x = nodez->ptr->x;
	y = nodez->ptr->y;
	z = nodez->ptr->z;

	D3DMATRIX m;

	if (parentWorld) m = (*parentWorld)*axis; else m = axis;

	nodez->x = m._11*x + m._21*y + m._31*z + m._41;
	nodez->y = m._12*x + m._22*y + m._32*z + m._42;
	z        = m._13*x + m._23*y + m._33*z + m._43;

	minx = maxx = nodez->x;
	miny = maxy = nodez->y;
	bindPoint = nodez->ptr;
	d = z;

	cnode = nodez + 1;

	for (i = 1; i < ndCount; i++, cnode += 1)
	{
		x = cnode->ptr->x;
		y = cnode->ptr->y;
		z = cnode->ptr->z;

		cnode->x = m._11*x + m._21*y + m._31*z + m._41;
		cnode->y = m._12*x + m._22*y + m._32*z + m._42;
		z        = m._13*x + m._23*y + m._33*z + m._43;

		if (minx > cnode->x) 
		{
			minx = cnode->x;
		} else if (maxx < cnode->x) 
		{
			maxx = cnode->x;
		}
		
		if (miny > cnode->y)
		{
			miny = cnode->y;
		} else if (maxy < cnode->y)
		{
			maxy = cnode->y;
		}

		if (d > z)
		{
			bindPoint = cnode->ptr;
			d = z;
		}
	}

	scalex = (TEXTX-1)/(maxx-minx);
	scaley = (TEXTY-1)/(maxy-miny);

// Переведем координаты на плоскости в текстурные координаты

	float _miny = miny, _minx = minx;

	_asm {
			mov		esi, this
			mov		edi, [esi]ComplexShadow.nodez
			mov		ecx, [esi]ComplexShadow.ndCount

cvtloop:	fld		dword ptr scaley
			fld		dword ptr [edi]sc_node.y
			fld		dword ptr _miny
			fld		dword ptr scalex
			fld		dword ptr [edi]sc_node.x
			fld		dword ptr _minx
			fsubp	st(1), st(0)
			fmulp	st(1), st(0)
			fistp	dword ptr [edi]sc_node.xi
			fsubp	st(1), st(0)
			fmulp	st(1), st(0)
			fistp	dword ptr [edi]sc_node.yi

			add		edi, SIZE sc_node
			loop	cvtloop
	}

	int dy1, dx1, dy2, dx2;
	int	sc1;

	cline = linez;
	for (i = 0; i < lnCount; i++, cline += 1)
	{
		dx1 = cline->bi->yi - cline->li->yi;
		if (dx1 == 0) continue;
		dy1 = cline->li->xi - cline->bi->xi;

		if (dx1 < 0)
		{
			dx1 = -dx1;
			dy1 = -dy1;
		}

		for (int j = 0; j < cline->nbrCount; ++j)
		{
			dx2 = cline->ti[j]->xi - cline->bi->xi;
			dy2 = cline->ti[j]->yi - cline->bi->yi;

			sc1 = dx1*dx2 + dy1*dy2;

			if (sc1 == 0.0f) continue;
			if (sc1 > 0) cline->ct += 1; else cline->ct -= 1;
		}
	}

	bLinez = true;

}

void ComplexShadow::BlitBuffer (unsigned char *bmp)
{
	int TY = TEXTY;
	unsigned char S = TXS;
	short int *b = buf;

	_asm {
		mov		edi, bmp
		mov		eax, gvzShadowOne
		mov		esi, b
		xor		bx, bx

		mov		edx, TY
		mov		cl, S
		shl		edx, cl
		mov		ecx, edx

aloop_lbl:	
		add		bx, word ptr [esi]
		jz		abbe
		stosd
		add		esi, 2
		loop	aloop_lbl
		jmp		aelp1

abbe:	mov		dword ptr [edi], gvzShadowZero
		add		edi, 4
		add		esi, 2
		loop	aloop_lbl

aelp1:	mov		edi, b
		mov		eax, TY
		mov		cl, S
		shl		eax, cl
		shr		eax, 1
		mov		ecx, eax
		xor		eax, eax
		rep		stosd
	}
}

void ComplexShadow::BlitLinez ()
{
	if (!bLinez) return;

	sc_line *cline;
	int i;

	cline = linez;
	for (i = 0; i < lnCount; i++, cline += 1)
	{
		if (cline->ct)
		{
			ShadowUtility::Line (this, cline->bi->xi, cline->bi->yi, cline->li->xi, cline->li->yi, cline->ct);
		}
	}

	bLinez = false;
}

} // namespace

#endif