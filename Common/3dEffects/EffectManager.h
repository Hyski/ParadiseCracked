/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: менеджер эффектов
                                                                                
                                                                                
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
// класс менеджера эффектов
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define INFO_FILE_NAME "scripts/Effects/3DEffects.txt"

// тип карты для эффектов
typedef std::map<long, PBaseEffect> EffectsMap;
// итератор для карты эффектов
typedef EffectsMap::iterator Effects_Map_Iterator;

class EffectManager
{
public:
	// перечисления
	enum ObjType          // тип разрушаемого объекта
	{
		OT_WOOD,
		OT_METAL,
		OT_STONE
	};
	enum ObjEffectType    // тип дополнительного эффекта при разрушении объекта
	{
		OET_DUST,
		OET_BLACKSMOKE,
		OET_FLAME
	};
	// открытые методы
public:
	// очистить все эффекты
	void clear();
	// конструктор по умолчанию
	EffectManager();
	// деструктор
	~EffectManager();
	// проинициализировать менеджер (получить указатель на graphpipe)
	void Init(GraphPipe * const pGP);
	// следующий тик времени для всех эффектов
	void NextTick(float time);
	// подготовка массивов готовых к отображению частиц
	// для всех эффектов
	void Update();
	// отрисовка всех эффектов
	void DrawEffects();

	// получение сообщения о начале нового турна
	void NewTurn();

	// получить качество эффектов
	const float GetEffectsQuality() { return quality; } const
	// установить качество эффектов
	void SetEffectsQuality(float q) { quality = q; }

	// уничтожить эффект
	void DestroyEffect(long id);

	// изменить позицию эффекта
	void SetPosition(unsigned int id, const point3& Pos);

	// изменить интенсивность эффекта
	void SetActivity(unsigned int id, const float Activity);

	// плавно погасить и уничтожить эффект
	void FinishEffect(unsigned int id);

	/////////////////
	// создание эффектов по имени
	/////////////////

	// вспышки при выстреле из оружия
	void CreateFlashEffect(const std::string& strSysName, const point3& Source, const point3& Target);
	// трассеры
	float CreateTracerEffect(const std::string& strSysName, const point3& Source, const point3& Target);
	// эффект попадания
	void CreateHitEffect(const std::string& strSysName, const point3& Source, const point3& Target, const point3& Ground, const float Growth);
	// вечные фоновые эффекты
	unsigned int CreateAmbientEffect(const std::string& strSysName, const point3& Target, const point3& Color);
	// эффект выделения
	unsigned int CreateSelectionEffect(const std::string& strSysName, const point3& Target);
	// эффект, который можно передвинуть и погасить извне
	unsigned int CreateMovableEffect(const std::string& strSysName, const point3& Target, float Radius = 0.0f);
	// эффект броска гранаты
	void CreateGrenadeEffect(const KeyAnimation& ka, const std::string& skin);
	// эффект вылета гильзы
	void CreateShellEffect(unsigned int eid, const std::string& skin, const point3& Source, const point3& Target);
	// эффект вылета ошметков мяса
	void CreateMeatEffect(unsigned int eid, const point3& Explosion, const point3& Human);

	// создать эффект разбивания стекла
	void DestroyGlass(const std::string& SysName, const DynObject& DynObj, const point3& ExpPos);

	// создать эффект уничтожения объекта взрывом
	void DestroyObjectByExplosion(const point3& ExpPos, const std::string& ObjName);

	// создать эффект уничтожения объекта через модель
	void DestroyObjectByModel(const point3& ExpPos, const std::string& ObjName, ObjType obj_type, ObjEffectType effect_type);

	// запись и чтение эффектов (отметин на уровне)
	virtual void MakeSaveLoad(SavSlot& slot);

	// закрытые данные
private:
	// карта указателей на эффекты
	EffectsMap effects;
	// указатель на объект GraphPipe
	GraphPipe * pGraphPipe;
	// текущий свободный номер эффекта
	unsigned int next_id;
	// текущее время (время последнего NextTick() )
	float cur_time;
	// качество эффектов
	float quality;
	// указатель на объект, хранящий информацию о параметрах эффектов
	EffectInfo * p_effect_info;
	// указатель на менеджер эффектов разрушения
	DestructEffectManager * destruction_manager;
	// указатель на менеджер эффектов моделек
	ModelManager* m_model_manager;

	struct MarkInfo
	{
		MARK_EFFECT_INFO m_info;
		float m_birthtime;
		point3 m_target;
	};
	typedef std::list<MarkInfo> Marks_t;
	// список отметин, которые необходимо сохранять
	Marks_t m_marks;

	// закрытые функции

	// получить случайное слово из вектора слов
	std::string& GetRandomWord(std::vector<std::string>& vec);

	// проапдейтить все эффекты отметин на уровне
	void updateMarks();
	// восстановить все эффекты отметин на уровне
	void restoreMarks();
};


#endif // !defined(__EFFECTMANAGER_H__)
