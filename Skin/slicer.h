#if !defined(__GVZ_SLICER_INCLUDED__)
#define __GVZ_SLICER_INCLUDED__

#define cNoCross	0
#define cCrosses	1

#define gNo			0
#define gExist		1
#define gNew		2

#define gtNo		0
#define gtCross		1
#define gt1st		2
#define gt2nd		3
#define gtSplit		4

class gCross;

class gPoint
{
public:
	float	pX, pY;

	gPoint () {pX = pY = 0;}
	gPoint (float _pX, float _pY) {pX = _pX; pY = _pY;}
	gPoint (const gPoint &_) {pX = _.pX; pY = _.pY;}
	gPoint &operator = (const gPoint &_) {pX = _.pX; pY = _.pY; return *this;}
};

/*typedef std::list<gPoint>						pList;
typedef std::list<gPoint>::iterator				ipList;
typedef std::list<gPoint>::reverse_iterator		rpList;
typedef std::list<gPoint>::reference			rList;*/

class gLine
{
public:
	gLine () {nX = nY = pX = pY = 0;}
	gLine (float _pX, float _pY, float _nX, float _nY)
	{
		pX = _pX; pY = _pY;
		nX = _nX; nY = _nY;
	}

	float	nX, nY;
	float	pX, pY;
};

class gPoly
{
public:
	enum {MAXVERTS = 15};

	gPoint	verts[MAXVERTS];
	unsigned vCount;

	gPoly () : vCount(0) {};
	gPoly &operator = (const gPoly &poly)
	{
		this->vCount = poly.vCount;
		memcpy (this->verts, poly.verts, this->vCount*sizeof(gPoint));
		return *this;
	}

	/*gPoly*/void Slice (const gLine &);
	int GetCrosses (const gLine &, gCross &, gCross &);
	int TestCross (const gLine &, gPoint &, gPoint &, float &);
//	bool crosses (const gLine &);
	float GetHalfPlane (const gLine &);
	unsigned next (unsigned);

//	void Add (gPoint &);
};

#endif