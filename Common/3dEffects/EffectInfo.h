/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: содержит информацию о всех эффектах
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__EFFECTINFO_H__)
#define __EFFECTINFO_H__

#include "../DataMgr/TxtFile.h"

// структура для хранения информации о LineEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Length;
	float WidthFirst;
	float WidthSecond;
	float VelFactor;
} LINE_EFFECT_INFO;
// карта
typedef std::map<DWORD, LINE_EFFECT_INFO> LINE_EFFECT_MAP;

// структура для хранения информации о SpangleEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Length;
	float Width;
	float PartLifeTime;
	float VelFactor;
} LINE_BALL_EFFECT_INFO;
// карта
typedef std::map<DWORD, LINE_BALL_EFFECT_INFO> LINE_BALL_EFFECT_MAP;

// структура для хранения информации о GrenadeEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	point3 Vel;
	point3 Accel;
	float Size0;
	float SizeRnd;
	float SizeRange;
	float LifeTime;
} GRENADE_EFFECT_INFO;
// карта
typedef std::map<DWORD, GRENADE_EFFECT_INFO> GRENADE_EFFECT_MAP;

// структура для хранения информации о BombSparkles
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	point3 Vel;
	point3 Accel;
	float Size0;
	float SizeRnd;
	float SizeRange;
	float LifeTime;
	unsigned int PartNum;
	bool Full;
} BOMB_SPARKLES_INFO;
// карта
typedef std::map<DWORD, BOMB_SPARKLES_INFO> BOMB_SPARKLES_MAP;

// структура для хранения информации о SparkleEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	point3 Vel;
	point3 Accel;
	float Size0;
	float SizeRnd;
	float SizeRange;
	float LifeTime;
	unsigned int PartNum;
	bool Full;
} SPARKLE_EFFECT_INFO;
// карта
typedef std::map<DWORD, SPARKLE_EFFECT_INFO> SPARKLE_EFFECT_MAP;

// структура для хранения информации о SpotEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Size;
	float VelFactor;
} SPOT_EFFECT_INFO;
// карта для хранения информации о SpotEffect
typedef std::map<DWORD, SPOT_EFFECT_INFO> SPOT_EFFECT_MAP;
// карта для хранения информации о ShineSpotEffect
typedef std::map<DWORD, SPOT_EFFECT_INFO> SHINE_SPOT_EFFECT_MAP;

// структура для хранения информации о WaveSpot
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float StartSize;
	float SizeVel;
	float LifeTime;
} WAVE_SPOT_INFO;
// карта
typedef std::map<DWORD, WAVE_SPOT_INFO> WAVE_SPOT_MAP;
// карта для WaveSpot2
typedef std::map<DWORD, WAVE_SPOT_INFO> WAVE_SPOT2_MAP;

// структура для хранения информации о SpangleEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Size;
	float VelFactor;
	float LifeTime;
} SPANGLE_EFFECT_INFO;
// карта
typedef std::map<DWORD, SPANGLE_EFFECT_INFO> SPANGLE_EFFECT_MAP;

// структура для хранения информации о TailEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Size;
	float SizeEnd;
	float VelFactor;
	float LifeTime;
	int Frequency;
} TAIL_EFFECT_INFO;
// карта для хранения информации о TailEffect
typedef std::map<DWORD, TAIL_EFFECT_INFO> TAIL_EFFECT_MAP;
// карта для хранения информации о TailCircleEffect
typedef std::map<DWORD, TAIL_EFFECT_INFO> TAIL_CIRCLE_EFFECT_MAP;

// структура для хранения информации о TailParabolaEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Size;
	float SizeEnd;
	float VelFactor;
	float Gravitation;
	float LifeTime;
	int Frequency;
} TAIL_PARABOLA_EFFECT_INFO;
// карта для хранения информации о TailEffect
typedef std::map<DWORD, TAIL_PARABOLA_EFFECT_INFO> TAIL_PARABOLA_EFFECT_MAP;

// структура для хранения информации о RotateSpotEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float SizeFirst;
	float SizeLast;
	int SequenceNumber;
	float RadiusStart;
	float RadiusEnd;
	float VelFactor;
	float AngVel;
	int SequenceSize;
	float Delta;
	bool Disp;
} ROTATE_SPOT_EFFECT_INFO;
// карта
typedef std::map<DWORD, ROTATE_SPOT_EFFECT_INFO> ROTATE_SPOT_EFFECT_MAP;

// структура для хранения информации о ExplosionSmokeEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	point3 Vel;
	point3 Accel;
	float Size0;
	float SizeRnd;
	float SizeRange;
	float LifeTime;
} EXPLOSION_SMOKE_INFO;
// карта
typedef std::map<DWORD, EXPLOSION_SMOKE_INFO> EXPLOSION_SMOKE_MAP;

// структура для хранения информации о ExplosionSmokeRing
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Size0;
	float SizeRange;
	float LifeTime;
} SMOKE_RING_INFO;
// карта
typedef std::map<DWORD, SMOKE_RING_INFO> SMOKE_RING_MAP;

// структура для хранения информации о FireEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float VelXY;
	float VelZ;
	float SizeBegin;
	float SizeRnd;
	float SizeRange;
	float LifeTime;
	int TurnsNum;
	float ReduceTime;
} FIRE_EFFECT_INFO;
// карта
typedef std::map<DWORD, FIRE_EFFECT_INFO> FIRE_EFFECT_MAP;

// структура для хранения информации о SmokeEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float BirthRange;
	point3 Vel;
	point3 Accel;
	float SizeBegin;
	float SizeRnd;
	float SizeRange;
	float LifeTime;
	int MaxPartNum;
	int TurnsNum;
	float ReduceTime;
	bool Exp;
} SMOKE_EFFECT_INFO;
// карта
typedef std::map<DWORD, SMOKE_EFFECT_INFO> SMOKE_EFFECT_MAP;

// структура для хранения информации о HeavySmokeEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float BirthRange;
	point3 Vel;
	point3 Accel;
	float SizeBegin;
	float SizeRnd;
	float SizeRange;
	float LifeTime;
	int MaxPartNum;
} HSMOKE_EFFECT_INFO;
// карта
typedef std::map<DWORD, HSMOKE_EFFECT_INFO> HSMOKE_EFFECT_MAP;

// структура для хранения информации о LensFlareEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Size;
	float Distance;
	float VelFactor;
} LENS_FLARE_EFFECT_INFO;
// карта
typedef std::map<DWORD, LENS_FLARE_EFFECT_INFO> LENS_FLARE_EFFECT_MAP;

// структура для хранения информации о OneUpsizingSpot и TwoUpsizingSpot
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float StartVel;
	float Accel;
	float WidthFirst;
	float WidthSecond;
	float WidthVel;
	float LifeTime;
} UPSIZING_SPOT_INFO;
// карты
typedef std::map<DWORD, UPSIZING_SPOT_INFO> ONE_UPSIZING_SPOT_MAP;
typedef std::map<DWORD, UPSIZING_SPOT_INFO> TWO_UPSIZING_SPOT_MAP;

// структура для хранения информации о DirectSparkles
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	unsigned int SeqNum;
	unsigned int PartNum;
	point3 VelDisp;
	float VelFactor;
	point3 Accel;
	float SizeBegin;
	float SizeRnd;
	float LifeTime;
} DIRECT_SPARKLES_INFO;
// карта
typedef std::map<DWORD, DIRECT_SPARKLES_INFO> DIRECT_SPARKLES_MAP;

// структура для хранения информации о HaloEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float SmallSize;
	float LargeSize;
	float NearDist;
	float FarDist;
	float NearColorFactor;
	float FarColorFactor;
} HALO_EFFECT_INFO;
// карта
typedef std::map<DWORD, HALO_EFFECT_INFO> HALO_EFFECT_MAP;

// структура для хранения информации о SimpleHaloEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Size;
} SIMPLE_HALO_EFFECT_INFO;
// карта
typedef std::map<DWORD, SIMPLE_HALO_EFFECT_INFO> SIMPLE_HALO_EFFECT_MAP;

// структура для хранения информации о BlinkingHaloEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Size;
	point3 Color;
	float SinA;
} BLINKING_HALO_EFFECT_INFO;
// карта
typedef std::map<DWORD, BLINKING_HALO_EFFECT_INFO> BLINKING_HALO_EFFECT_MAP;

// структура для хранения информации о SelectionEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Radius;
	unsigned int PartNum;
	float SmallSize;
	float LargeSize;
	float AngVel;
	float AngDelta;
} SELECTION_EFFECT_INFO;
// карта
typedef std::map<DWORD, SELECTION_EFFECT_INFO> SELECTION_EFFECT_MAP;

// структура для хранения информации о LineDLightEffect
typedef struct {
	float Radius;
	unsigned int Color;
	float VelFactor;
} LINE_DLIGHT_EFFECT_INFO;
// карта
typedef std::map<DWORD, LINE_DLIGHT_EFFECT_INFO> LINE_DLIGHT_EFFECT_MAP;

// структура для хранения информации о ShootSmokeEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	unsigned int PartNum;
	float StartSize;
	float SlowSizeVel;
	float FastSizeVel;
	point3 SlowVel;
	point3 FastVel;
	point3 Accel;
	float LifeTime;
} SHOOT_SMOKE_EFFECT_INFO;
// карта
typedef std::map<DWORD, SHOOT_SMOKE_EFFECT_INFO> SHOOT_SMOKE_EFFECT_MAP;

// структура для хранения информации о FlashEffect & MultiTexFlashEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float SizeBegin;
	float SizeEnd;
	float SwitchTime;
	float LifeTime;
} FLASH_EFFECT_INFO;
// карта
typedef std::map<DWORD, FLASH_EFFECT_INFO> FLASH_EFFECT_MAP;
// карта
typedef std::map<DWORD, FLASH_EFFECT_INFO> MULTI_FLASH_EFFECT_MAP;

// структура для хранения информации о DestructEffect
typedef struct {
	std::string Sound;
	float MinVelFactor;
	float MaxVelFactor;
	point3 Accel;
	point3 VelDisp;
	float MaxAngleVel;
	float FrictionFactor;
	float MaxSquare;
	float LifeTime;
} GLASS_DESTRUCT_EFFECT_INFO;
// карта
typedef std::map<DWORD, GLASS_DESTRUCT_EFFECT_INFO> GLASS_DESTRUCT_EFFECT_MAP;

// структура для хранения информации о MarkEffect
typedef struct {
	std::vector<std::string> Shaders;
	float Radius;
	std::string Type;
	point3 StartColor;
	float LifeTime;
	float SwitchTime;
} MARK_EFFECT_INFO;
// карта
typedef std::map<DWORD, MARK_EFFECT_INFO> MARK_EFFECT_MAP;

// структура для хранения информации о RotateSpotEffect2
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Size;
	int Number;
	float RadiusBegin;
	float RadiusEnd;
	float AngVel;
	point3 Vel;
	float LifeTime;
} ROTATE_SPOT_EFFECT2_INFO;
// карта
typedef std::map<DWORD, ROTATE_SPOT_EFFECT2_INFO> ROTATE_SPOT_EFFECT2_MAP;

// структура для хранения информации о FlyEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Size;
	float SizeRnd;
	int MaxPartNum;
	float MinRadius;
	float RadiusRange;
	point3 MinVel;
} FLY_EFFECT_INFO;
// карта
typedef std::map<DWORD, FLY_EFFECT_INFO> FLY_EFFECT_MAP;

// структура для хранения информации о DirectExplosion
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	int PartNum;
	float Size;
	float SizeRnd;
	point3 VelDisp;
	float VelFactor;
	point3 Accel;
	float LifeTime;
} DIRECT_EXPLOSION_INFO;
// карта
typedef std::map<DWORD, DIRECT_EXPLOSION_INFO> DIRECT_EXPLOSION_MAP;

// структура для хранения информации о PlasmaBeamEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float WidthSource;
	float WidthTarget;
	float LifeTime;
} PLASMA_BEAM_INFO;
// карта
typedef std::map<DWORD, PLASMA_BEAM_INFO> PLASMA_BEAM_MAP;

// структура для хранения информации о LightningEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	int BranchNum;
	float SegmentLength;
	float Width;
	float LifeTime;
} LIGHTNING_INFO;
// карта
typedef std::map<DWORD, LIGHTNING_INFO> LIGHTNING_MAP;

// структура для хранения информации о TracerLineModel
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float VelFactor;
} TRACER_LINE_MODEL_INFO;
// карта
typedef std::map<DWORD, TRACER_LINE_MODEL_INFO> TRACER_LINE_MODEL_MAP;

// структура для хранения информации о TracerParabolaModel
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float VelFactor;
	float Gravitation;
} TRACER_PARABOLA_MODEL_INFO;
// карта
typedef std::map<DWORD, TRACER_PARABOLA_MODEL_INFO> TRACER_PARABOLA_MODEL_MAP;

// структура для хранения информации о ShieldEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	int PointsNum;
	int CirclesNum;
	float DX;
	float ParabolaFactor;
	float LifeTime;
} SHIELD_INFO;
// карта
typedef std::map<DWORD, SHIELD_INFO> SHIELD_MAP;

// структура для хранения информации о ModelDestructEffect
typedef struct {
	float MinVelFactor;
	float MaxVelFactor;
	float ExplVelFactor;
	point3 Accel;
	float MaxAngleVel;
	float Density;
	float ScaleFactor;
	float LifeTime;
} MODEL_DESTRUCT_INFO;
// карта
typedef std::map<DWORD, MODEL_DESTRUCT_INFO> MODEL_DESTRUCT_MAP;

// структура для хранения информации о MeatEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Size;
	float SizeEnd;
	float LifeTime;
	int Frequency;
} MEAT_EFFECT_INFO;
// карта для хранения информации о MeatEffect
typedef std::map<DWORD, MEAT_EFFECT_INFO> MEAT_EFFECT_MAP;

// структура для хранения информации о FlyCylinderEffect
typedef struct {
	std::string Shader;
	std::string Sound;
	bool SndMov;
	float Radius;
	float Height;
	unsigned int MaxPartNum;
	unsigned int TailPartNum;
	float PartSize;
	float VelH;
	float VelAlpha;
	float PartDeltat;
	point3 Color;
} FLY_CYLINDER_EFFECT_INFO;
// карта
typedef std::map<DWORD, FLY_CYLINDER_EFFECT_INFO> FLY_CYLINDER_EFFECT_MAP;

//////////////////////////////////////////////////////////////////////////
//
// класс EffectInfo
//
//////////////////////////////////////////////////////////////////////////

class EffectInfo
{
public:

	// перечисление всех типов эффектов
	typedef enum
	{
		ET_ERROR,
		ET_LINE,
		ET_GRENADE,
		ET_BOMBSPARKLES,
		ET_SPARKLE,
		ET_SPOT,
		ET_WAVE_SPOT,
		ET_SHINE_SPOT,
		ET_SPANGLE,
		ET_TAIL,
		ET_TAIL_PARABOLA,
		ET_ROTATE_SPOT,
		ET_EXPLOSION_SMOKE,
		ET_SMOKE_RING,
		ET_FIRE,
		ET_SMOKE,
		ET_HSMOKE,
		ET_FLARE,
		ET_ONE_UPSIZING_SPOT,
		ET_TWO_UPSIZING_SPOT,
		ET_DIRECT_SPARKLES,
		ET_HALO,
		ET_SIMPLE_HALO,
		ET_BLINKING_HALO,
		ET_SELECTION,
		ET_LINE_DLIGHT,
		ET_SHOOT_SMOKE,
		ET_LINE_BALL,
		ET_TAIL_CIRCLE,
		ET_FLASH,
		ET_GLASS_DESTRUCT,
		ET_MARK,
		ET_MULTI_FLASH,
		ET_ROTATE_SPOT2,
		ET_FLY,
		ET_DIRECT_EXPLOSION,
		ET_PLASMA_BEAM,
		ET_LIGHTNING,
		ET_TRACER_LINE_MODEL,
		ET_TRACER_PARABOLA_MODEL,
		ET_SHIELD,
		ET_MODEL_DESTRUCT,
		ET_WAVE_SPOT2,
		ET_MEAT,
		ET_FLY_CYLINDER,
	} ET;

	////////////
	//
	// открытые функции
	//
	////////////

	EffectInfo();
	~EffectInfo();

	// получение типа эффекта по его названию
	EffectInfo::ET GetEffectType(std::string strName);

	// загрузить информацию о всех эффектах из файла
	void LoadInfo(const char * filename);

	// выгрузить информацию из памяти
	void UnLoadInfo();

	// загрузить параметры эффекта в соответствующую карту составляющих
	DWORD LoadEffectParams(TxtFile& f, int i, int j, ET type);

	// из строки-параметра читает float и отрезает от строки все лишнее
	float GetFloatFromString(std::string& str);

	// из строки-параметра читает bool и отрезает от строки все лишнее
	bool GetBoolFromString(std::string& str);

	// из строки-параметра читает int и отрезает от строки все лишнее
	int GetIntFromString(std::string& str);

	// из строки-параметра читает слова и сохраняет их в вектор
	void SetShadersVector(std::string& str, std::vector<std::string>& vec);

	////////////
	//
	// открытые данные
	//
	////////////

	// тип - список типов эффектов и их идентификаторов
	typedef struct { DWORD id; ET type; } EFFECT_INFO;
	typedef std::list<EFFECT_INFO> EFFECT_INFO_LIST; 
	// тип - карта всех эффектов
	typedef std::map<std::string, EFFECT_INFO_LIST> EFFECT_INFO_MAP;

	LINE_EFFECT_MAP m_line_effect_map;
	GRENADE_EFFECT_MAP m_grenade_effect_map;
	BOMB_SPARKLES_MAP m_bomb_sparkles_map;
	SPARKLE_EFFECT_MAP m_sparkle_effect_map;
	SPOT_EFFECT_MAP m_spot_effect_map;
	SHINE_SPOT_EFFECT_MAP shine_spot_effect_map;
	SPANGLE_EFFECT_MAP m_spangle_effect_map;
	TAIL_EFFECT_MAP m_tail_effect_map;
	TAIL_PARABOLA_EFFECT_MAP tail_parabola_effect_map;
	ROTATE_SPOT_EFFECT_MAP m_rotate_spot_effect_map;
	EXPLOSION_SMOKE_MAP m_explosion_smoke_map;
	SMOKE_RING_MAP m_smoke_ring_map;
	FIRE_EFFECT_MAP fire_effect_map;
	SMOKE_EFFECT_MAP smoke_effect_map;
	HSMOKE_EFFECT_MAP hsmoke_effect_map;
	LENS_FLARE_EFFECT_MAP lens_flare_effect_map;
	ONE_UPSIZING_SPOT_MAP one_upsizing_spot_map;
	TWO_UPSIZING_SPOT_MAP two_upsizing_spot_map;
	DIRECT_SPARKLES_MAP direct_sparkles_map;
	HALO_EFFECT_MAP halo_effect_map;
	SIMPLE_HALO_EFFECT_MAP simple_halo_effect_map;
	SELECTION_EFFECT_MAP selection_effect_map;
	LINE_DLIGHT_EFFECT_MAP line_dlight_effect_map;
	WAVE_SPOT_MAP wave_spot_map;
	SHOOT_SMOKE_EFFECT_MAP shoot_smoke_effect_map;
	LINE_BALL_EFFECT_MAP line_ball_effect_map;
	TAIL_CIRCLE_EFFECT_MAP tail_circle_effect_map;
	FLASH_EFFECT_MAP flash_effect_map;
	GLASS_DESTRUCT_EFFECT_MAP glass_destruct_effect_map;
	MARK_EFFECT_MAP mark_effect_map;
	MULTI_FLASH_EFFECT_MAP multi_flash_effect_map;
	ROTATE_SPOT_EFFECT2_MAP rotate_spot_effect2_map;
	FLY_EFFECT_MAP fly_effect_map;
	DIRECT_EXPLOSION_MAP direct_explosion_map;
	PLASMA_BEAM_MAP plasma_beam_map;
	LIGHTNING_MAP lightning_map;
	TRACER_LINE_MODEL_MAP tracer_line_model_map;
	TRACER_PARABOLA_MODEL_MAP tracer_parabola_model_map;
	SHIELD_MAP shield_map;
	MODEL_DESTRUCT_MAP model_destruct_map;
	BLINKING_HALO_EFFECT_MAP blinking_halo_map;
	WAVE_SPOT2_MAP wave_spot2_map;
	MEAT_EFFECT_MAP meat_effect_map;
	FLY_CYLINDER_EFFECT_MAP m_fly_cylinder_map;

	// карта всех эффектов
	EFFECT_INFO_MAP m_effect_info_map;

	////////////
	//
	// закрытые данные
	//
	////////////
private:
	DWORD m_next_id;
};

#endif // !defined(__EFFECTINFO_H__)
