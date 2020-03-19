#if !defined(__GVOZDODER_SHADOW_TEXTURE__)
#define __GVOZDODER_SHADOW_TEXTURE__

#include <list>
#include "..\Common\PreComp.h"
#include "..\Common\GraphPipe\SimpleTexturedObject.h"
#include "..\Options\Options.h"
#include "..\Common\TextureMgr\TextureMgr.h"
#include "..\Common\TextureMgr\DIBData.h"
#include "..\Common\GraphPipe\GraphPipe.h"
#include "person.h"

#include "Slicer.h"

class VShader;

namespace Shadows {
	class BaseShadow;
	class ComplexShadow;
	class SimpleShadow;
	class ShadowUtility;

//#	define GVZ_SHADOW_LOG

#	if defined(GVZ_SHADOW_LOG)
#		include "..\Common\Log\log.h"
		extern CLog gvzLog;
#		define GVZ_LOG	gvzLog["Shadow.log"]
#	endif // GVZ_SHADOW_LOG

#	if !defined(GVZ_SHADOW_LOG)
#		define GVZ_LOG	;/##/
#	endif

	struct sc_node
	{
		point3		*ptr;
		float		x, y;
		int			xi, yi;
	};

	struct sc_line 
	{
		enum {MAXNBRS = 10};
		int			bIdx, lIdx; // ������� ������ �����
		int			tIdx[MAXNBRS];	// ������� �������� �����
		sc_node		*bi, *li;	// ��������� ������ �����
		sc_node		*ti[MAXNBRS];	// ��������� �������� �����
		short int	ct;		// ���-�� ���������
		short int	nbrCount; // ���-�� �������
	};

////////////////////////////////////////////////////////// ShadowBuffer
	typedef std::list<Triangle> trilist;

	class ShadowBuffer
	{
		Primi m_primitive;

		enum {MAXTRIANGLES = 200};
		point3 m_pos[MAXTRIANGLES*3];
//		point3 m_normals[MAXTRIANGLES*3];
		unsigned m_diffuse[MAXTRIANGLES*3];
		texcoord m_texcoords[MAXTRIANGLES*3];

	public:
		ShadowBuffer ();
		~ShadowBuffer ();

		void CollectPlanes (BaseShadow *, float, int CurIndex = 0);

		void Draw (GraphPipe *, const std::string &);

    void FlushGeometry ();

	private:
//		D3DLVERTEX vertices[MAXTRIANGLES*3];
		unsigned vCount;

		void cutPoly (gPoly &, gPoly &, float ,float ,float, float);
	};

////////////////////////////////////////////////////////// BaseShadow 
	class BaseShadow // ������� ����� ��� ����
	{
	friend ShadowUtility;
	friend ShadowBuffer;

	protected:
		std::string m_shader;

	public:
		BaseShadow (RealEntity *);
		BaseShadow (BaseShadow *);
		virtual ~BaseShadow ();

		virtual void Update (float) {};					// ���������� ����

		// ������� ��� ����
		virtual void AddObject (const TexObject *);		// �������� ������
		virtual void AddObject (SimpleTexturedObject *);		// �������� ������
		virtual void Clear ();						// �������� ���������� ��� ���� ��������

		virtual void Draw (GraphPipe *) {};		// ��������� ����
		
		virtual void UpdateOnModeChange () {};

		virtual void SetShadowTexture (const std::string &) {};
		void SetRedrawPeriod (float p);
		void SetLight (point3 &);

		float GetKoef (float);

		std::list<SimpleTexturedObject *> objects;
		RealEntity *entity; // �������� ����

		virtual unsigned describe() const { return 0; }

	protected:
		D3DMATRIX axis;
		point3 *bindPoint;		// ����� �������� ����
		const D3DMATRIX *parentWorld;	// ������� �������������� �������


		const D3DMATRIX *invParentWorld;// ��������
		std::string txtName;	// ��� ��������

		static unsigned int cntShdwGen; // ������� ��� ��������� ���������� �������� �������

		point3 lt;				// ������ ���������

		float tRedrawPeriod;		// � ����� �������� ��������� ����
		float tLastCalc;			// ����� ���������� ����������
		float deadTime;

		ShadowBuffer cache;	// ���
		bool Updated, NeedUpdate;

		float minx, maxx, miny, maxy;
	};

///////////////////////////////////////////////////// ComplexShadow
	class ComplexShadow : public BaseShadow // ������� ����
	{
	friend ShadowUtility;
	public:
		ComplexShadow (RealEntity *);
		ComplexShadow (BaseShadow *);
		~ComplexShadow ();

		void Update (float);					// ���������� ����

		// ������� ��� ����
		void AddObject (const TexObject *);		// �������� ������
		void AddObject (SimpleTexturedObject *);		// �������� ������
		void Clear ();							// �������� ���������� ��� ���� ��������

		void Draw (GraphPipe *);			// ��������� ����
		
		void UpdateOnModeChange ();

		unsigned describe() const;

	private:
		void Calc ();	// ����������� �����
		void BlitLinez ();	// ���������� ����� � ������ � �������� �����
		void BlitBuffer (unsigned char *);	// ���������� ����� � bitmap � �������� �����

	private:
		DIBData *dbdTexture;	// ���������� ��������

		unsigned int TEXTX;	// Shadow texture width
		unsigned int TEXTY;	// Shadow texture height
		unsigned char TXS;	// TEXTX = 2**TXS

		float d;

		LPDIRECTDRAWSURFACE7 shdwSurface;	// ����������� ��� ����

		short int *buf;		// ����� ��� ������������� ������

		sc_node *nodez;
		int ndCount;

		sc_line *linez;
		int lnCount;

		bool bLinez;
	};

//////////////////////////////////////////////////////////// SimpleShadow
	class SimpleShadow : public BaseShadow // ������� ����
	{
	friend ShadowUtility;
	public:
		SimpleShadow (RealEntity *);
		SimpleShadow (BaseShadow *);
		~SimpleShadow ();

		void SetShadowTexture (const std::string &);

		void Update (float);

		void AddObject (const TexObject *);
		void AddObject (SimpleTexturedObject *);		// �������� ������
		void Clear ();

		void Draw (GraphPipe *);
	private:
		std::string txtSimple;
		float d;

		void Calc ();
	};

	class NullShadow : public BaseShadow
	{
	friend ShadowUtility;
	public:
		NullShadow (RealEntity *entity) : BaseShadow(entity) {}
		NullShadow (BaseShadow *elder) : BaseShadow(elder)
		{
			for (std::list<SimpleTexturedObject *>::iterator i = elder->objects.begin(); i != elder->objects.end(); ++i)
			{
				AddObject (*i);
			}
		}
		virtual ~NullShadow () {}

		void Update (float) {};					// ���������� ����

		// ������� ��� ����
		void AddObject (const TexObject *o) {BaseShadow::AddObject(o);}		// �������� ������
		void AddObject (SimpleTexturedObject *o) {BaseShadow::AddObject(o);}// �������� ������
		void Clear () {BaseShadow::Clear ();} // �������� ���������� ��� ���� ��������

		void Draw (GraphPipe *) {};		// ��������� ����
		
		void UpdateOnModeChange () {};
	};

////////////////////////////////////////////////////////// ShadowUtility
	class ShadowUtility
	{
	friend BaseShadow;
	friend ComplexShadow;
	friend SimpleShadow;
	friend void OptionsChanged();
	public:
		static void AddShadow (std::string &, BaseShadow *);
		static void KillShadow (const std::string &);

		static void CopyImageToSurface (const ComplexShadow *);

		static point3 projPoint2Surface (const point3 &, const point3 &, const point3 &, const point3 &);

		static void Line (const ComplexShadow *, int, int, int, int, short int);

		static void DrawLights (GraphPipe *);

		static void AddTime (float curTime, float relTime);
		static float GetTime ();

		static void AddTimeD (float curTime, float relTime);
		static float GetTimeD ();

		static float getLightIntensity (const point3 &); // �������� ������������� ����� � �����
		static point3 getLightDirection (const point3 &); // �������� ����������� ����� � �����
		static bool NoKill ();

	private:

		static float timeCalc;
		static float timeInProcess;
		static float timeBegin;

		static float timeCalcD;
		static float timeInProcessD;
		static float timeBeginD;

		static bool bNoKill;
	};

	void UpdateOnModeChange ();
	void OptionsChanged ();
	void DrawShadows (GraphPipe *);

	// ��������� �����
	void AddLight (const point3 &, float, float);
	void ClearLights ();
	BaseShadow *CreateShadow (RealEntity *);
	BaseShadow *ReCreateShadow (BaseShadow *);

} // namespace

#endif