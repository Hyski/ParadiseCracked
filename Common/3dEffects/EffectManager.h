/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: �������� ��������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                


#if !defined(__EEFFECTMANAGER_H__)
#define __EFFECTMANAGER_H__

#include "Effect.h"
#include "EffectInfo.h"
#include "DestructEffect.h"

class ModelManager;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//
// ����� ��������� ��������
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define INFO_FILE_NAME "scripts/Effects/3DEffects.txt"

// ��� ����� ��� ��������
typedef std::map<long, PBaseEffect> EffectsMap;
// �������� ��� ����� ��������
typedef EffectsMap::iterator Effects_Map_Iterator;

class EffectManager
{
public:
	// ������������
	enum ObjType          // ��� ������������ �������
	{
		OT_WOOD,
		OT_METAL,
		OT_STONE
	};
	enum ObjEffectType    // ��� ��������������� ������� ��� ���������� �������
	{
		OET_DUST,
		OET_BLACKSMOKE,
		OET_FLAME
	};
	// �������� ������
public:
	// �������� ��� �������
	void clear();
	// ����������� �� ���������
	EffectManager();
	// ����������
	~EffectManager();
	// ������������������� �������� (�������� ��������� �� graphpipe)
	void Init(GraphPipe * const pGP);
	// ��������� ��� ������� ��� ���� ��������
	void NextTick(float time);
	// ���������� �������� ������� � ����������� ������
	// ��� ���� ��������
	void Update();
	// ��������� ���� ��������
	void DrawEffects();

	// ��������� ��������� � ������ ������ �����
	void NewTurn();

	// �������� �������� ��������
	const float GetEffectsQuality() { return quality; } const
	// ���������� �������� ��������
	void SetEffectsQuality(float q) { quality = q; }

	// ���������� ������
	void DestroyEffect(long id);

	// �������� ������� �������
	void SetPosition(unsigned int id, const point3& Pos);

	// �������� ������������� �������
	void SetActivity(unsigned int id, const float Activity);

	// ������ �������� � ���������� ������
	void FinishEffect(unsigned int id);

	/////////////////
	// �������� �������� �� �����
	/////////////////

	// ������� ��� �������� �� ������
	void CreateFlashEffect(const std::string& strSysName, const point3& Source, const point3& Target);
	// ��������
	float CreateTracerEffect(const std::string& strSysName, const point3& Source, const point3& Target);
	// ������ ���������
	void CreateHitEffect(const std::string& strSysName, const point3& Source, const point3& Target, const point3& Ground, const float Growth);
	// ������ ������� �������
	unsigned int CreateAmbientEffect(const std::string& strSysName, const point3& Target, const point3& Color);
	// ������ ���������
	unsigned int CreateSelectionEffect(const std::string& strSysName, const point3& Target);
	// ������, ������� ����� ����������� � �������� �����
	unsigned int CreateMovableEffect(const std::string& strSysName, const point3& Target, float Radius = 0.0f);
	// ������ ������ �������
	void CreateGrenadeEffect(const KeyAnimation& ka, const std::string& skin);
	// ������ ������ ������
	void CreateShellEffect(unsigned int eid, const std::string& skin, const point3& Source, const point3& Target);
	// ������ ������ �������� ����
	void CreateMeatEffect(unsigned int eid, const point3& Explosion, const point3& Human);

	// ������� ������ ���������� ������
	void DestroyGlass(const std::string& SysName, const DynObject& DynObj, const point3& ExpPos);

	// ������� ������ ����������� ������� �������
	void DestroyObjectByExplosion(const point3& ExpPos, const std::string& ObjName);

	// ������� ������ ����������� ������� ����� ������
	void DestroyObjectByModel(const point3& ExpPos, const std::string& ObjName, ObjType obj_type, ObjEffectType effect_type);

	// ������ � ������ �������� (������� �� ������)
	virtual void MakeSaveLoad(SavSlot& slot);

	// �������� ������
private:
	// ����� ���������� �� �������
	EffectsMap effects;
	// ��������� �� ������ GraphPipe
	GraphPipe * pGraphPipe;
	// ������� ��������� ����� �������
	unsigned int next_id;
	// ������� ����� (����� ���������� NextTick() )
	float cur_time;
	// �������� ��������
	float quality;
	// ��������� �� ������, �������� ���������� � ���������� ��������
	EffectInfo * p_effect_info;
	// ��������� �� �������� �������� ����������
	DestructEffectManager * destruction_manager;
	// ��������� �� �������� �������� �������
	ModelManager* m_model_manager;

	struct MarkInfo
	{
		MARK_EFFECT_INFO m_info;
		float m_birthtime;
		point3 m_target;
	};
	typedef std::list<MarkInfo> Marks_t;
	// ������ �������, ������� ���������� ���������
	Marks_t m_marks;

	// �������� �������

	// �������� ��������� ����� �� ������� ����
	std::string& GetRandomWord(std::vector<std::string>& vec);

	// ������������ ��� ������� ������� �� ������
	void updateMarks();
	// ������������ ��� ������� ������� �� ������
	void restoreMarks();
};


#endif // !defined(__EFFECTMANAGER_H__)
