/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: содержит информацию о всех эффектах
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "EffectInfo.h"

//#define EFFECT_INFO_LOG 1 // enable for log file


#if defined (EFFECT_INFO_LOG) //for log
CLog l;
#define F "Effects.log"
#endif // for log
#if !defined (EFFECT_INFO_LOG) //for log
#define l /##/
#endif // for log


//////////////////////////////////////////////////////////////////////////
//
// класс EffectInfo
//
//////////////////////////////////////////////////////////////////////////

EffectInfo::EffectInfo()
{
	m_next_id = 1;
}

EffectInfo::~EffectInfo()
{
	UnLoadInfo();
}

// получение типа эффекта по его названию
EffectInfo::ET EffectInfo::GetEffectType(std::string Name)
{
	if(Name == "LineEffect") return ET::ET_LINE;
	if(Name == "GrenadeEffect") return ET::ET_GRENADE;
	if(Name == "BombSparkles") return ET::ET_BOMBSPARKLES;
	if(Name == "SparkleEffect") return ET::ET_SPARKLE;
	if(Name == "SpotEffect") return ET::ET_SPOT;
	if(Name == "ShineSpotEffect") return ET::ET_SHINE_SPOT;
	if(Name == "SpangleEffect") return ET::ET_SPANGLE;
	if(Name == "TailEffect") return ET::ET_TAIL;
	if(Name == "TailParabolaEffect") return ET::ET_TAIL_PARABOLA;
	if(Name == "RotateSpotEffect") return ET::ET_ROTATE_SPOT;
	if(Name == "ExplosionSmokeEffect") return ET::ET_EXPLOSION_SMOKE;
	if(Name == "ExplosionSmokeRing") return ET::ET_SMOKE_RING;
	if(Name == "FireEffect") return ET::ET_FIRE;
	if(Name == "SmokeEffect") return ET::ET_SMOKE;
	if(Name == "HeavySmokeEffect") return ET::ET_HSMOKE;
	if(Name == "LensFlareEffect") return ET::ET_FLARE;
	if(Name == "OneUpsizingSpot") return ET::ET_ONE_UPSIZING_SPOT;
	if(Name == "TwoUpsizingSpot") return ET::ET_TWO_UPSIZING_SPOT;
	if(Name == "DirectSparkles") return ET::ET_DIRECT_SPARKLES;
	if(Name == "HaloEffect") return ET::ET_HALO;
	if(Name == "BlinkingHaloEffect") return ET::ET_BLINKING_HALO;
	if(Name == "SimpleHaloEffect") return ET::ET_SIMPLE_HALO;
	if(Name == "SelectionEffect") return ET::ET_SELECTION;
	if(Name == "LineDLightEffect") return ET::ET_LINE_DLIGHT;
	if(Name == "WaveSpot") return ET::ET_WAVE_SPOT;
	if(Name == "ShootSmokeEffect") return ET::ET_SHOOT_SMOKE;
	if(Name == "LineBallEffect") return ET::ET_LINE_BALL;
	if(Name == "TailCircleEffect") return ET::ET_TAIL_CIRCLE;
	if(Name == "FlashEffect") return ET::ET_FLASH;
	if(Name == "GlassDestruct") return ET::ET_GLASS_DESTRUCT;
	if(Name == "MarkEffect") return ET::ET_MARK;
	if(Name == "MultiTexFlashEffect") return ET::ET_MULTI_FLASH;
	if(Name == "RotateSpotEffect2") return ET::ET_ROTATE_SPOT2;
	if(Name == "FlyEffect") return ET::ET_FLY;
	if(Name == "DirectExplosion") return ET::ET_DIRECT_EXPLOSION;
	if(Name == "PlasmaBeamEffect") return ET::ET_PLASMA_BEAM;
	if(Name == "LightningEffect") return ET::ET_LIGHTNING;
	if(Name == "TracerLineModel") return ET::ET_TRACER_LINE_MODEL;
	if(Name == "TracerParabolaModel") return ET::ET_TRACER_PARABOLA_MODEL;
	if(Name == "ShieldEffect") return ET::ET_SHIELD;
	if(Name == "ModelDestructEffect") return ET::ET_MODEL_DESTRUCT;
	if(Name == "WaveSpot2") return ET::ET_WAVE_SPOT2;
	if(Name == "MeatEffect") return ET::ET_MEAT;
	if(Name == "FlyCylinderEffect") return ET::ET_FLY_CYLINDER;
	return ET::ET_ERROR;
}

// загрузить информацию о всех эффектах из файла
void EffectInfo::LoadInfo(const char * filename)
{
	UnLoadInfo();

	l[F]("EffectInfo::LoadInfo begin\n");
		//загрузить файл
	TxtFile f(DataMgr::Load(filename));
	if(f.SizeY()==0)throw CASUS("EffectInfo: Can't Load Effect Info File!");

	std::string str, strn;
	int num, j;
	EFFECT_INFO ei;
	EFFECT_INFO_LIST eil;

	if(f.SizeY() < 2)
	{
		l[F]("EffectInfo::LoadInfo: Info file \"%s\" is not found or empty!\n");
		return;
	}
	// цикл по всем строкам файла
	int i = 1;
	while( i < f.SizeY() )
	{
		// читаем System Name эффекта
		str = f(i, 0);
		l[F]("\tстрока %d: \"%s\"\n", i, str.c_str());
		if(str == "") { i++; continue; }

		// пробуем найти это имя в карте информации об эффектах
		// и если оно уже есть выдаем исключение
		if(m_effect_info_map.find(str) != m_effect_info_map.end())
		{
			// нашли
			l[F]("EffectInfo::LoadInfo: Duplicate System Name! (\"%s\")\n", str.c_str());
			return;
			//throw CASUS("EffectInfo: Duplicate System Name!");
		}

		// добавим в карту информацию о новом эффекте
		// а также добавим в соответствующие карты параметры составляющих эффектов
		strn = f(i, 2);
		num = atoi(strn.c_str());
		l[F]("\t\t эффект \"%s\" состоит из %d эффектов:\n", str.c_str(), num);
		i++;
		j = i + num;
		eil.clear();
		while(i < j)
		{
			// поочереди сохраним параметры составляющих
			strn = f(i, 3);
			l[F]("\t\t\t\"%s\"\n", strn.c_str());
			ei.type = GetEffectType(strn);
			ei.id = LoadEffectParams(f, i, 4, ei.type);
			eil.push_back(ei);
			i++;
		}
		m_effect_info_map[str] = eil;

	}

	// закроем файл
	DataMgr::Release(filename);

	l[F]("EffectInfo::LoadInfo end\n");
}

// выгрузить информацию из памяти
void EffectInfo::UnLoadInfo()
{
	EFFECT_INFO_MAP::iterator i;
	if( m_effect_info_map.empty() ) return;
	for(i = m_effect_info_map.begin(); i != m_effect_info_map.end(); i++)
	{
		if( (*i).second.empty() ) continue;
		(*i).second.clear();
	}
	m_effect_info_map.clear();
}

// загрузить параметры эффекта в соответствующую карту составляющих
DWORD EffectInfo::LoadEffectParams(TxtFile& f, int i, int j, ET type)
{
	std::string str;
	switch(type)
	{
	case ET::ET_LINE : 
		{
			LINE_EFFECT_INFO lei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			lei.Shader = str;

			str = f(i, j + 1);
			lei.Sound = str;

			str = f(i, j + 2);
			lei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			lei.Length = GetFloatFromString(str);

			str = f(i, j + 4);
			lei.WidthFirst = GetFloatFromString(str);

			str = f(i, j + 5);
			lei.WidthSecond = GetFloatFromString(str);

			str = f(i, j + 6);
			lei.VelFactor = GetFloatFromString(str);

			// добавляем структуру в карту
			m_line_effect_map[m_next_id] = lei;

			l[F]("\t\t\t ET_LINE loaded\n");
			break;
		}
	case ET::ET_LINE_BALL : 
		{
			LINE_BALL_EFFECT_INFO lbei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			lbei.Shader = str;

			str = f(i, j + 1);
			lbei.Sound = str;

			str = f(i, j + 2);
			lbei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			lbei.Length = GetFloatFromString(str);

			str = f(i, j + 4);
			lbei.Width = GetFloatFromString(str);

			str = f(i, j + 5);
			lbei.PartLifeTime = GetFloatFromString(str);

			str = f(i, j + 6);
			lbei.VelFactor = GetFloatFromString(str);

			// добавляем структуру в карту
			line_ball_effect_map[m_next_id] = lbei;

			l[F]("\t\t\t ET_LINE_BALL loaded\n");
			break;
		}
	case ET::ET_GRENADE : 
		{
			GRENADE_EFFECT_INFO gei;
			float x, y, z;

			// читаем информацию из файла в структуру
			str = f(i, j);
			gei.Shader = str;

			str = f(i, j + 1);
			gei.Sound = str;

			str = f(i, j + 2);
			gei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			gei.Vel = point3(x, y, z);

			str = f(i, j + 4);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			gei.Accel = point3(x, y, z);

			str = f(i, j + 5);
			gei.Size0 = GetFloatFromString(str);

			str = f(i, j + 6);
			gei.SizeRnd = GetFloatFromString(str);

			str = f(i, j + 7);
			gei.SizeRange = GetFloatFromString(str);

			str = f(i, j + 8);
			gei.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			m_grenade_effect_map[m_next_id] = gei;

			l[F]("\t\t\t ET_GRENADE loaded\n");
			break;
		}
	case ET::ET_BOMBSPARKLES : 
		{
			BOMB_SPARKLES_INFO bsi;
			float x, y, z;

			// читаем информацию из файла в структуру
			str = f(i, j);
			bsi.Shader = str;

			str = f(i, j + 1);
			bsi.Sound = str;

			str = f(i, j + 2);
			bsi.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			bsi.Vel = point3(x, y, z);

			str = f(i, j + 4);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			bsi.Accel = point3(x, y, z);

			str = f(i, j + 5);
			bsi.Size0 = GetFloatFromString(str);

			str = f(i, j + 6);
			bsi.SizeRnd = GetFloatFromString(str);

			str = f(i, j + 7);
			bsi.SizeRange = GetFloatFromString(str);

			str = f(i, j + 8);
			bsi.LifeTime = GetFloatFromString(str);

			str = f(i, j + 9);
			bsi.PartNum = GetIntFromString(str);

			str = f(i, j + 10);
			bsi.Full = GetBoolFromString(str);

			// добавляем структуру в карту
			m_bomb_sparkles_map[m_next_id] = bsi;

			l[F]("\t\t\t ET_BOMBSPARKLES loaded\n");
			break;
		}
	case ET::ET_SPARKLE : 
		{
			SPARKLE_EFFECT_INFO sei;
			float x, y, z;

			// читаем информацию из файла в структуру
			str = f(i, j);
			sei.Shader = str;

			str = f(i, j + 1);
			sei.Sound = str;

			str = f(i, j + 2);
			sei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			sei.Vel = point3(x, y, z);

			str = f(i, j + 4);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			sei.Accel = point3(x, y, z);

			str = f(i, j + 5);
			sei.Size0 = GetFloatFromString(str);

			str = f(i, j + 6);
			sei.SizeRnd = GetFloatFromString(str);

			str = f(i, j + 7);
			sei.SizeRange = GetFloatFromString(str);

			str = f(i, j + 8);
			sei.LifeTime = GetFloatFromString(str);

			str = f(i, j + 9);
			sei.PartNum = GetIntFromString(str);

			str = f(i, j + 10);
			sei.Full = GetBoolFromString(str);

			// добавляем структуру в карту
			m_sparkle_effect_map[m_next_id] = sei;

			l[F]("\t\t\t ET_SPARKLE loaded\n");
			break;
		}
	case ET::ET_SPOT : 
		{
			SPOT_EFFECT_INFO sei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			sei.Shader = str;

			str = f(i, j + 1);
			sei.Sound = str;

			str = f(i, j + 2);
			sei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			sei.Size = GetFloatFromString(str);

			str = f(i, j + 4);
			sei.VelFactor = GetFloatFromString(str);

			// добавляем структуру в карту
			m_spot_effect_map[m_next_id] = sei;

			l[F]("\t\t\t ET_SPOT loaded\n");
			break;
		}
	case ET::ET_WAVE_SPOT : 
		{
			WAVE_SPOT_INFO wsi;

			// читаем информацию из файла в структуру
			str = f(i, j);
			wsi.Shader = str;

			str = f(i, j + 1);
			wsi.Sound = str;

			str = f(i, j + 2);
			wsi.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			wsi.StartSize = GetFloatFromString(str);

			str = f(i, j + 4);
			wsi.SizeVel = GetFloatFromString(str);

			str = f(i, j + 5);
			wsi.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			wave_spot_map[m_next_id] = wsi;

			l[F]("\t\t\t ET_WAVE_SPOT loaded\n");
			break;
		}
	case ET::ET_SHINE_SPOT : 
		{
			SPOT_EFFECT_INFO sei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			sei.Shader = str;

			str = f(i, j + 1);
			sei.Sound = str;

			str = f(i, j + 2);
			sei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			sei.Size = GetFloatFromString(str);

			str = f(i, j + 4);
			sei.VelFactor = GetFloatFromString(str);

			// добавляем структуру в карту
			shine_spot_effect_map[m_next_id] = sei;

			l[F]("\t\t\t ET_SHINE_SPOT loaded\n");
			break;
		}
	case ET::ET_SPANGLE : 
		{
			SPANGLE_EFFECT_INFO sei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			sei.Shader = str;

			str = f(i, j + 1);
			sei.Sound = str;

			str = f(i, j + 2);
			sei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			sei.Size = GetFloatFromString(str);

			str = f(i, j + 4);
			sei.VelFactor = GetFloatFromString(str);

			str = f(i, j + 5);
			sei.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			m_spangle_effect_map[m_next_id] = sei;

			l[F]("\t\t\t ET_SPANGLE loaded\n");
			break;
		}
	case ET::ET_TAIL : 
		{
			TAIL_EFFECT_INFO tei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			tei.Shader = str;

			str = f(i, j + 1);
			tei.Sound = str;

			str = f(i, j + 2);
			tei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			tei.Size = GetFloatFromString(str);

			str = f(i, j + 4);
			tei.SizeEnd = GetFloatFromString(str);

			str = f(i, j + 5);
			tei.VelFactor = GetFloatFromString(str);

			str = f(i, j + 6);
			tei.LifeTime = GetFloatFromString(str);

			str = f(i, j + 7);
			tei.Frequency = GetIntFromString(str);

			// добавляем структуру в карту
			m_tail_effect_map[m_next_id] = tei;

			l[F]("\t\t\t ET_TAIL loaded\n");
			break;
		}
	case ET::ET_TAIL_PARABOLA : 
		{
			TAIL_PARABOLA_EFFECT_INFO tpei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			tpei.Shader = str;

			str = f(i, j + 1);
			tpei.Sound = str;

			str = f(i, j + 2);
			tpei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			tpei.Size = GetFloatFromString(str);

			str = f(i, j + 4);
			tpei.SizeEnd = GetFloatFromString(str);

			str = f(i, j + 5);
			tpei.VelFactor = GetFloatFromString(str);

			str = f(i, j + 6);
			tpei.Gravitation = GetFloatFromString(str);

			str = f(i, j + 7);
			tpei.LifeTime = GetFloatFromString(str);

			str = f(i, j + 8);
			tpei.Frequency = GetIntFromString(str);

			// добавляем структуру в карту
			tail_parabola_effect_map[m_next_id] = tpei;

			l[F]("\t\t\t ET_TAIL_BARABOLA loaded\n");
			break;
		}
	case ET::ET_ROTATE_SPOT : 
		{
			ROTATE_SPOT_EFFECT_INFO rsei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			rsei.Shader = str;

			str = f(i, j + 1);
			rsei.Sound = str;

			str = f(i, j + 2);
			rsei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			rsei.SizeFirst = GetFloatFromString(str);

			str = f(i, j + 4);
			rsei.SizeLast = GetFloatFromString(str);

			str = f(i, j + 5);
			rsei.SequenceNumber = GetIntFromString(str);

			str = f(i, j + 6);
			rsei.RadiusStart = GetFloatFromString(str);
			rsei.RadiusEnd = GetFloatFromString(str);

			str = f(i, j + 7);
			rsei.VelFactor = GetFloatFromString(str);

			str = f(i, j + 8);
			rsei.AngVel = GetFloatFromString(str);

			str = f(i, j + 9);
			rsei.SequenceSize = GetIntFromString(str);

			str = f(i, j + 10);
			rsei.Delta = GetFloatFromString(str);

			str = f(i, j + 11);
			rsei.Disp = GetBoolFromString(str);

			// добавляем структуру в карту
			m_rotate_spot_effect_map[m_next_id] = rsei;

			l[F]("\t\t\t ET_ROTATE_SPOT loaded\n");
			break;
		}
	case ET::ET_EXPLOSION_SMOKE : 
		{
			EXPLOSION_SMOKE_INFO esi;
			float x, y, z;

			// читаем информацию из файла в структуру
			str = f(i, j);
			esi.Shader = str;

			str = f(i, j + 1);
			esi.Sound = str;

			str = f(i, j + 2);
			esi.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			esi.Vel = point3(x, y, z);

			str = f(i, j + 4);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			esi.Accel = point3(x, y, z);

			str = f(i, j + 5);
			esi.Size0 = GetFloatFromString(str);

			str = f(i, j + 6);
			esi.SizeRnd = GetFloatFromString(str);

			str = f(i, j + 7);
			esi.SizeRange = GetFloatFromString(str);

			str = f(i, j + 8);
			esi.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			m_explosion_smoke_map[m_next_id] = esi;

			l[F]("\t\t\t ET_EXPLOSION_SMOKE loaded\n");
			break;
		}
	case ET::ET_SMOKE_RING : 
		{
			SMOKE_RING_INFO sri;

			// читаем информацию из файла в структуру
			str = f(i, j);
			sri.Shader = str;

			str = f(i, j + 1);
			sri.Sound = str;

			str = f(i, j + 2);
			sri.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			sri.Size0 = GetFloatFromString(str);

			str = f(i, j + 4);
			sri.SizeRange = GetFloatFromString(str);

			str = f(i, j + 5);
			sri.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			m_smoke_ring_map[m_next_id] = sri;

			l[F]("\t\t\t ET_SMOKE_RING loaded\n");
			break;
		}
	case ET::ET_FIRE : 
		{
			FIRE_EFFECT_INFO fei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			fei.Shader = str;

			str = f(i, j + 1);
			fei.Sound = str;

			str = f(i, j + 2);
			fei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			fei.VelXY = GetFloatFromString(str);

			str = f(i, j + 4);
			fei.VelZ = GetFloatFromString(str);

			str = f(i, j + 5);
			fei.SizeBegin = GetFloatFromString(str);

			str = f(i, j + 6);
			fei.SizeRnd = GetFloatFromString(str);

			str = f(i, j + 7);
			fei.SizeRange = GetFloatFromString(str);

			str = f(i, j + 8);
			fei.LifeTime = GetFloatFromString(str);

			str = f(i, j + 9);
			fei.TurnsNum = GetIntFromString(str);

			str = f(i, j + 10);
			fei.ReduceTime = GetFloatFromString(str);

			// добавляем структуру в карту
			fire_effect_map[m_next_id] = fei;

			l[F]("\t\t\t ET_FIRE loaded\n");
			break;
		}
	case ET::ET_SMOKE : 
		{
			float x, y, z;
			SMOKE_EFFECT_INFO sei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			sei.Shader = str;

			str = f(i, j + 1);
			sei.Sound = str;

			str = f(i, j + 2);
			sei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			sei.BirthRange = GetFloatFromString(str);

			str = f(i, j + 4);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			sei.Vel = point3(x, y, z);

			str = f(i, j + 5);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			sei.Accel = point3(x, y, z);

			str = f(i, j + 6);
			sei.SizeBegin = GetFloatFromString(str);

			str = f(i, j + 7);
			sei.SizeRnd = GetFloatFromString(str);

			str = f(i, j + 8);
			sei.SizeRange = GetFloatFromString(str);

			str = f(i, j + 9);
			sei.LifeTime = GetFloatFromString(str);

			str = f(i, j + 10);
			sei.MaxPartNum = GetIntFromString(str);

			str = f(i, j + 11);
			sei.TurnsNum = GetIntFromString(str);

			str = f(i, j + 12);
			sei.ReduceTime = GetFloatFromString(str);

			str = f(i, j + 13);
			sei.Exp = GetBoolFromString(str);

			// добавляем структуру в карту
			smoke_effect_map[m_next_id] = sei;

			l[F]("\t\t\t ET_SMOKE loaded\n");
			break;
		}
	case ET::ET_HSMOKE : 
		{
			float x, y, z;
			HSMOKE_EFFECT_INFO sei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			sei.Shader = str;

			str = f(i, j + 1);
			sei.Sound = str;

			str = f(i, j + 2);
			sei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			sei.BirthRange = GetFloatFromString(str);

			str = f(i, j + 4);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			sei.Vel = point3(x, y, z);

			str = f(i, j + 5);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			sei.Accel = point3(x, y, z);

			str = f(i, j + 6);
			sei.SizeBegin = GetFloatFromString(str);

			str = f(i, j + 7);
			sei.SizeRnd = GetFloatFromString(str);

			str = f(i, j + 8);
			sei.SizeRange = GetFloatFromString(str);

			str = f(i, j + 9);
			sei.LifeTime = GetFloatFromString(str);

			str = f(i, j + 10);
			sei.MaxPartNum = GetIntFromString(str);

			// добавляем структуру в карту
			hsmoke_effect_map[m_next_id] = sei;

			l[F]("\t\t\t ET_HSMOKE loaded\n");
			break;
		}
	case ET::ET_FLARE : 
		{
			LENS_FLARE_EFFECT_INFO lfei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			lfei.Shader = str;

			str = f(i, j + 1);
			lfei.Sound = str;

			str = f(i, j + 2);
			lfei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			lfei.Size = GetFloatFromString(str);

			str = f(i, j + 4);
			lfei.Distance = GetFloatFromString(str);

			str = f(i, j + 5);
			lfei.VelFactor = GetFloatFromString(str);

			// добавляем структуру в карту
			lens_flare_effect_map[m_next_id] = lfei;

			l[F]("\t\t\t ET_FLARE loaded\n");
			break;
		}
	case ET::ET_ONE_UPSIZING_SPOT : 
		{
			UPSIZING_SPOT_INFO usi;

			// читаем информацию из файла в структуру
			str = f(i, j);
			usi.Shader = str;

			str = f(i, j + 1);
			usi.Sound = str;

			str = f(i, j + 2);
			usi.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			usi.StartVel = GetFloatFromString(str);

			str = f(i, j + 4);
			usi.Accel = GetFloatFromString(str);

			str = f(i, j + 5);
			usi.WidthFirst = GetFloatFromString(str);

			str = f(i, j + 6);
			usi.WidthSecond = GetFloatFromString(str);

			str = f(i, j + 7);
			usi.WidthVel = GetFloatFromString(str);

			str = f(i, j + 8);
			usi.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			one_upsizing_spot_map[m_next_id] = usi;

			l[F]("\t\t\t ET_ONE_UPSIZING_SPOT loaded\n");
			break;
		}
	case ET::ET_TWO_UPSIZING_SPOT : 
		{
			UPSIZING_SPOT_INFO usi;

			// читаем информацию из файла в структуру
			str = f(i, j);
			usi.Shader = str;

			str = f(i, j + 1);
			usi.Sound = str;

			str = f(i, j + 2);
			usi.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			usi.StartVel = GetFloatFromString(str);

			str = f(i, j + 4);
			usi.Accel = GetFloatFromString(str);

			str = f(i, j + 5);
			usi.WidthFirst = GetFloatFromString(str);

			str = f(i, j + 6);
			usi.WidthSecond = GetFloatFromString(str);

			str = f(i, j + 7);
			usi.WidthVel = GetFloatFromString(str);

			str = f(i, j + 8);
			usi.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			two_upsizing_spot_map[m_next_id] = usi;

			l[F]("\t\t\t ET_TWO_UPSIZING_SPOT loaded\n");
			break;
		}
	case ET::ET_DIRECT_SPARKLES : 
		{
			DIRECT_SPARKLES_INFO dsi;
			float x, y, z;

			// читаем информацию из файла в структуру
			str = f(i, j);
			dsi.Shader = str;

			str = f(i, j + 1);
			dsi.Sound = str;

			str = f(i, j + 2);
			dsi.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			dsi.SeqNum = GetIntFromString(str);

			str = f(i, j + 4);
			dsi.PartNum = GetIntFromString(str);

			str = f(i, j + 5);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			dsi.VelDisp = point3(x, y, z);

			str = f(i, j + 6);
			dsi.VelFactor = GetFloatFromString(str);

			str = f(i, j + 7);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			dsi.Accel = point3(x, y, z);

			str = f(i, j + 8);
			dsi.SizeBegin = GetFloatFromString(str);

			str = f(i, j + 9);
			dsi.SizeRnd = GetFloatFromString(str);

			str = f(i, j + 10);
			dsi.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			direct_sparkles_map[m_next_id] = dsi;

			l[F]("\t\t\t ET_DIRECT_SPARKLES loaded\n");
			break;
		}
	case ET::ET_HALO : 
		{
			HALO_EFFECT_INFO hei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			hei.Shader = str;

			str = f(i, j + 1);
			hei.Sound = str;

			str = f(i, j + 2);
			hei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			hei.SmallSize = GetFloatFromString(str);

			str = f(i, j + 4);
			hei.LargeSize = GetFloatFromString(str);

			str = f(i, j + 5);
			hei.NearDist = GetFloatFromString(str);

			str = f(i, j + 6);
			hei.FarDist = GetFloatFromString(str);

			str = f(i, j + 7);
			hei.NearColorFactor = GetFloatFromString(str);

			str = f(i, j + 8);
			hei.FarColorFactor = GetFloatFromString(str);


			// добавляем структуру в карту
			halo_effect_map[m_next_id] = hei;

			l[F]("\t\t\t ET_HALO loaded\n");
			break;
		}
	case ET::ET_SIMPLE_HALO : 
		{
			SIMPLE_HALO_EFFECT_INFO shei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			shei.Shader = str;

			str = f(i, j + 1);
			shei.Sound = str;

			str = f(i, j + 2);
			shei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			shei.Size = GetFloatFromString(str);

			// добавляем структуру в карту
			simple_halo_effect_map[m_next_id] = shei;

			l[F]("\t\t\t ET_SIMPLE_HALO loaded\n");
			break;
		}
	case ET::ET_BLINKING_HALO : 
		{
			BLINKING_HALO_EFFECT_INFO bhei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			bhei.Shader = str;

			str = f(i, j + 1);
			bhei.Sound = str;

			str = f(i, j + 2);
			bhei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			bhei.Size = GetFloatFromString(str);

			str = f(i, j + 4);
			float x, y, z;
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			bhei.Color = point3(x, y, z);

			str = f(i, j + 5);
			bhei.SinA = GetFloatFromString(str);


			// добавляем структуру в карту
			blinking_halo_map[m_next_id] = bhei;

			l[F]("\t\t\t ET_BLINKING_HALO loaded\n");
			break;
		}
	case ET::ET_SELECTION : 
		{
			SELECTION_EFFECT_INFO sei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			sei.Shader = str;

			str = f(i, j + 1);
			sei.Sound = str;

			str = f(i, j + 2);
			sei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			sei.Radius = GetFloatFromString(str);

			str = f(i, j + 4);
			sei.PartNum = GetIntFromString(str);

			str = f(i, j + 5);
			sei.SmallSize = GetFloatFromString(str);

			str = f(i, j + 6);
			sei.LargeSize = GetFloatFromString(str);

			str = f(i, j + 7);
			sei.AngVel = GetFloatFromString(str);

			str = f(i, j + 8);
			sei.AngDelta = GetFloatFromString(str);

			// добавляем структуру в карту
			selection_effect_map[m_next_id] = sei;

			l[F]("\t\t\t ET_SELECTION loaded\n");
			break;
		}
	case ET::ET_LINE_DLIGHT : 
		{
			LINE_DLIGHT_EFFECT_INFO ldei;
			unsigned int r, g, b;

			// читаем информацию из файла в структуру
			str = f(i, j + 3);
			ldei.Radius = GetFloatFromString(str);

			str = f(i, j + 4);
			r = GetIntFromString(str);
			if(r > 255) r = 255;

			str = f(i, j + 5);
			g = GetIntFromString(str);
			if(g > 255) g = 255;

			str = f(i, j + 6);
			b = GetIntFromString(str);
			if(b > 255) b = 255;

			ldei.Color = RGB_MAKE(r, g, b);

			str = f(i, j + 7);
			ldei.VelFactor = GetFloatFromString(str);

			// добавляем структуру в карту
			line_dlight_effect_map[m_next_id] = ldei;

			l[F]("\t\t\t ET_LINE_DLIGHT loaded\n");
			break;
		}
	case ET::ET_SHOOT_SMOKE : 
		{
			SHOOT_SMOKE_EFFECT_INFO ssei;
			float x, y, z;

			// читаем информацию из файла в структуру
			str = f(i, j);
			ssei.Shader = str;

			str = f(i, j + 1);
			ssei.Sound = str;

			str = f(i, j + 2);
			ssei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			ssei.PartNum = GetIntFromString(str);

			str = f(i, j + 4);
			ssei.StartSize = GetFloatFromString(str);

			str = f(i, j + 5);
			ssei.SlowSizeVel = GetFloatFromString(str);

			str = f(i, j + 6);
			ssei.FastSizeVel = GetFloatFromString(str);

			str = f(i, j + 7);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			ssei.SlowVel = point3(x, y, z);

			str = f(i, j + 8);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			ssei.FastVel = point3(x, y, z);

			str = f(i, j + 9);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			ssei.Accel = point3(x, y, z);

			str = f(i, j + 10);
			ssei.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			shoot_smoke_effect_map[m_next_id] = ssei;

			l[F]("\t\t\t ET_SHOOT_SMOKE loaded\n");
			break;
		}
	case ET::ET_TAIL_CIRCLE : 
		{
			TAIL_EFFECT_INFO tei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			tei.Shader = str;

			str = f(i, j + 1);
			tei.Sound = str;

			str = f(i, j + 2);
			tei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			tei.Size = GetFloatFromString(str);

			str = f(i, j + 4);
			tei.SizeEnd = GetFloatFromString(str);

			str = f(i, j + 5);
			tei.VelFactor = GetFloatFromString(str);

			str = f(i, j + 6);
			tei.LifeTime = GetFloatFromString(str);

			str = f(i, j + 7);
			tei.Frequency = GetIntFromString(str);

			// добавляем структуру в карту
			tail_circle_effect_map[m_next_id] = tei;

			l[F]("\t\t\t ET_TAIL_CIRCLE loaded\n");
			break;
		}
	case ET::ET_FLASH : 
		{
			FLASH_EFFECT_INFO fei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			fei.Shader = str;

			str = f(i, j + 1);
			fei.Sound = str;

			str = f(i, j + 2);
			fei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			fei.SizeBegin = GetFloatFromString(str);

			str = f(i, j + 4);
			fei.SizeEnd = GetFloatFromString(str);

			str = f(i, j + 5);
			fei.SwitchTime = GetFloatFromString(str);

			str = f(i, j + 6);
			fei.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			flash_effect_map[m_next_id] = fei;

			l[F]("\t\t\t ET_FLASH loaded\n");
			break;
		}
	case ET::ET_GLASS_DESTRUCT : 
		{
			GLASS_DESTRUCT_EFFECT_INFO gdei;
			float x, y, z;

			// читаем информацию из файла в структуру
			str = f(i, j + 1);
			gdei.Sound = str;

			str = f(i, j + 3);
			gdei.MinVelFactor = GetFloatFromString(str);

			str = f(i, j + 4);
			gdei.MaxVelFactor = GetFloatFromString(str);

			str = f(i, j + 5);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			gdei.Accel = point3(x, y, z);

			str = f(i, j + 6);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			gdei.VelDisp = point3(x, y, z);

			str = f(i, j + 7);
			gdei.MaxAngleVel = GetFloatFromString(str);

			str = f(i, j + 8);
			gdei.FrictionFactor = GetFloatFromString(str);

			str = f(i, j + 9);
			gdei.MaxSquare = GetFloatFromString(str);

			str = f(i, j + 10);
			gdei.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			glass_destruct_effect_map[m_next_id] = gdei;

			l[F]("\t\t\t ET_GLASS_DESTRUCT loaded\n");
			break;
		}
	case ET::ET_MARK : 
		{
			MARK_EFFECT_INFO mei;
			float x, y, z;

			str = f(i, j);
			SetShadersVector(str, mei.Shaders);
			//mei.Shader = GetRandomStringFromString(str);

			str = f(i, j + 3);
			mei.Radius = GetFloatFromString(str);

			str = f(i, j + 4);
			mei.Type = str;

			str = f(i, j + 5);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			mei.StartColor = point3(x, y, z);

			str = f(i, j + 6);
			mei.LifeTime = GetFloatFromString(str);

			str = f(i, j + 7);
			mei.SwitchTime = GetFloatFromString(str);

			// добавляем структуру в карту
			mark_effect_map[m_next_id] = mei;

			l[F]("\t\t\t ET_MARK loaded\n");
			break;
		}
	case ET::ET_MULTI_FLASH : 
		{
			FLASH_EFFECT_INFO fei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			fei.Shader = str;

			str = f(i, j + 1);
			fei.Sound = str;

			str = f(i, j + 2);
			fei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			fei.SizeBegin = GetFloatFromString(str);

			str = f(i, j + 4);
			fei.SizeEnd = GetFloatFromString(str);

			str = f(i, j + 5);
			fei.SwitchTime = GetFloatFromString(str);

			str = f(i, j + 6);
			fei.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			multi_flash_effect_map[m_next_id] = fei;

			l[F]("\t\t\t ET_MULTI_FLASH loaded\n");
			break;
		}
	case ET::ET_ROTATE_SPOT2 : 
		{
			ROTATE_SPOT_EFFECT2_INFO rsei;
			float x, y, z;

			// читаем информацию из файла в структуру
			str = f(i, j);
			rsei.Shader = str;

			str = f(i, j + 1);
			rsei.Sound = str;

			str = f(i, j + 2);
			rsei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			rsei.Size = GetFloatFromString(str);

			str = f(i, j + 4);
			rsei.Number = GetIntFromString(str);

			str = f(i, j + 5);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			rsei.RadiusBegin = x;
			rsei.RadiusEnd = y;

			str = f(i, j + 6);
			rsei.AngVel = GetFloatFromString(str);

			str = f(i, j + 7);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			rsei.Vel = point3(x, y, z);

			str = f(i, j + 8);
			rsei.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			rotate_spot_effect2_map[m_next_id] = rsei;

			l[F]("\t\t\t ET_ROTATE_SPOT2 loaded\n");
			break;
		}
	case ET::ET_FLY : 
		{
			FLY_EFFECT_INFO fei;
			float x, y, z;

			// читаем информацию из файла в структуру
			str = f(i, j);
			fei.Shader = str;

			str = f(i, j + 1);
			fei.Sound = str;

			str = f(i, j + 2);
			fei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			fei.Size = GetFloatFromString(str);

			str = f(i, j + 4);
			fei.SizeRnd = GetFloatFromString(str);

			str = f(i, j + 5);
			fei.MaxPartNum = GetIntFromString(str);

			str = f(i, j + 6);
			fei.MinRadius = GetFloatFromString(str);

			str = f(i, j + 7);
			fei.RadiusRange = GetFloatFromString(str);

			str = f(i, j + 8);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			fei.MinVel = point3(x, y, z);

			// добавляем структуру в карту
			fly_effect_map[m_next_id] = fei;

			l[F]("\t\t\t ET_FLY loaded\n");
			break;
		}
	case ET::ET_DIRECT_EXPLOSION : 
		{
			DIRECT_EXPLOSION_INFO dei;
			float x, y, z;

			// читаем информацию из файла в структуру
			str = f(i, j);
			dei.Shader = str;

			str = f(i, j + 1);
			dei.Sound = str;

			str = f(i, j + 2);
			dei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			dei.PartNum = GetIntFromString(str);

			str = f(i, j + 4);
			dei.Size = GetFloatFromString(str);

			str = f(i, j + 5);
			dei.SizeRnd = GetFloatFromString(str);

			str = f(i, j + 6);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			dei.VelDisp = point3(x, y, z);

			str = f(i, j + 7);
			dei.VelFactor = GetFloatFromString(str);

			str = f(i, j + 8);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			dei.Accel = point3(x, y, z);

			str = f(i, j + 9);
			dei.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			direct_explosion_map[m_next_id] = dei;

			l[F]("\t\t\t ET_DIRECT_EXPLOSION loaded\n");
			break;
		}
	case ET::ET_PLASMA_BEAM : 
		{
			PLASMA_BEAM_INFO pbi;

			// читаем информацию из файла в структуру
			str = f(i, j);
			pbi.Shader = str;

			str = f(i, j + 1);
			pbi.Sound = str;

			str = f(i, j + 2);
			pbi.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			pbi.WidthSource = GetFloatFromString(str);

			str = f(i, j + 4);
			pbi.WidthTarget = GetFloatFromString(str);

			str = f(i, j + 5);
			pbi.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			plasma_beam_map[m_next_id] = pbi;

			l[F]("\t\t\t ET_PLASMA_BEAM loaded\n");
			break;
		}
	case ET::ET_LIGHTNING : 
		{
			LIGHTNING_INFO li;

			// читаем информацию из файла в структуру
			str = f(i, j);
			li.Shader = str;

			str = f(i, j + 1);
			li.Sound = str;

			str = f(i, j + 2);
			li.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			li.BranchNum = GetIntFromString(str);

			str = f(i, j + 4);
			li.SegmentLength = GetFloatFromString(str);

			str = f(i, j + 5);
			li.Width = GetFloatFromString(str);

			str = f(i, j + 6);
			li.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			lightning_map[m_next_id] = li;

			l[F]("\t\t\t ET_LIGHTNING loaded\n");
			break;
		}
	case ET::ET_TRACER_LINE_MODEL : 
		{
			TRACER_LINE_MODEL_INFO tlmi;

			// читаем информацию из файла в структуру
			str = f(i, j);
			tlmi.Shader = str;

			str = f(i, j + 1);
			tlmi.Sound = str;

			str = f(i, j + 2);
			tlmi.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			tlmi.VelFactor = GetFloatFromString(str);

			// добавляем структуру в карту
			tracer_line_model_map[m_next_id] = tlmi;

			l[F]("\t\t\t ET_TRACER_LINE_MODEL loaded\n");
			break;
		}
	case ET::ET_TRACER_PARABOLA_MODEL : 
		{
			TRACER_PARABOLA_MODEL_INFO tpmi;

			// читаем информацию из файла в структуру
			str = f(i, j);
			tpmi.Shader = str;

			str = f(i, j + 1);
			tpmi.Sound = str;

			str = f(i, j + 2);
			tpmi.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			tpmi.VelFactor = GetFloatFromString(str);

			str = f(i, j + 4);
			tpmi.Gravitation = GetFloatFromString(str);

			// добавляем структуру в карту
			tracer_parabola_model_map[m_next_id] = tpmi;

			l[F]("\t\t\t ET_TRACER_PARABOLA_MODEL loaded\n");
			break;
		}
	case ET::ET_SHIELD : 
		{
			SHIELD_INFO si;

			// читаем информацию из файла в структуру
			str = f(i, j);
			si.Shader = str;

			str = f(i, j + 1);
			si.Sound = str;

			str = f(i, j + 2);
			si.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			si.PointsNum = GetIntFromString(str);

			str = f(i, j + 4);
			si.CirclesNum = GetIntFromString(str);

			str = f(i, j + 5);
			si.DX = GetFloatFromString(str);

			str = f(i, j + 6);
			si.ParabolaFactor = GetFloatFromString(str);

			str = f(i, j + 7);
			si.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			shield_map[m_next_id] = si;

			l[F]("\t\t\t ET_SHIELD loaded\n");
			break;
		}
	case ET::ET_MODEL_DESTRUCT : 
		{
			MODEL_DESTRUCT_INFO mdi;
			float x, y, z;

			// читаем информацию из файла в структуру

			str = f(i, j + 3);
			mdi.MinVelFactor = GetFloatFromString(str);

			str = f(i, j + 4);
			mdi.MaxVelFactor = GetFloatFromString(str);

			str = f(i, j + 5);
			mdi.ExplVelFactor = GetFloatFromString(str);

			str = f(i, j + 6);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			mdi.Accel = point3(x, y, z);

			str = f(i, j + 7);
			mdi.MaxAngleVel = GetFloatFromString(str);

			str = f(i, j + 8);
			mdi.Density = GetFloatFromString(str);

			str = f(i, j + 9);
			mdi.ScaleFactor = GetFloatFromString(str);

			str = f(i, j + 10);
			mdi.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			model_destruct_map[m_next_id] = mdi;

			l[F]("\t\t\t ET_MODEL_DESTRUCT loaded\n");
			break;
		}
	case ET::ET_WAVE_SPOT2 : 
		{
			WAVE_SPOT_INFO wsi;

			// читаем информацию из файла в структуру
			str = f(i, j);
			wsi.Shader = str;

			str = f(i, j + 1);
			wsi.Sound = str;

			str = f(i, j + 2);
			wsi.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			wsi.StartSize = GetFloatFromString(str);

			str = f(i, j + 4);
			wsi.SizeVel = GetFloatFromString(str);

			str = f(i, j + 5);
			wsi.LifeTime = GetFloatFromString(str);

			// добавляем структуру в карту
			wave_spot2_map[m_next_id] = wsi;

			l[F]("\t\t\t ET_WAVE_SPOT2 loaded\n");
			break;
		}
	case ET::ET_MEAT : 
		{
			MEAT_EFFECT_INFO mei;

			// читаем информацию из файла в структуру
			str = f(i, j);
			mei.Shader = str;

			str = f(i, j + 1);
			mei.Sound = str;

			str = f(i, j + 2);
			mei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			mei.Size = GetFloatFromString(str);

			str = f(i, j + 4);
			mei.SizeEnd = GetFloatFromString(str);

			str = f(i, j + 5);
			mei.LifeTime = GetFloatFromString(str);

			str = f(i, j + 6);
			mei.Frequency = GetIntFromString(str);

			// добавляем структуру в карту
			meat_effect_map[m_next_id] = mei;

			l[F]("\t\t\t ET_MEAT loaded\n");
			break;
		}
	case ET::ET_FLY_CYLINDER : 
		{
			FLY_CYLINDER_EFFECT_INFO ei;
			float x, y, z;

			// читаем информацию из файла в структуру
			str = f(i, j);
			ei.Shader = str;

			str = f(i, j + 1);
			ei.Sound = str;

			str = f(i, j + 2);
			ei.SndMov = GetBoolFromString(str);

			str = f(i, j + 3);
			ei.Radius = GetFloatFromString(str);

			str = f(i, j + 4);
			ei.Height = GetFloatFromString(str);

			str = f(i, j + 5);
			ei.MaxPartNum = GetIntFromString(str);

			str = f(i, j + 6);
			ei.TailPartNum = GetIntFromString(str);

			str = f(i, j + 7);
			ei.PartSize = GetFloatFromString(str);

			str = f(i, j + 8);
			ei.VelH = GetFloatFromString(str);

			str = f(i, j + 9);
			ei.VelAlpha = GetFloatFromString(str);

			str = f(i, j + 10);
			ei.PartDeltat = GetFloatFromString(str);

			str = f(i, j + 11);
			x = GetFloatFromString(str);
			y = GetFloatFromString(str);
			z = GetFloatFromString(str);
			ei.Color = point3(x, y, z);

			// добавляем структуру в карту
			m_fly_cylinder_map[m_next_id] = ei;

			l[F]("\t\t\t ET_FLY_CYLINDER loaded\n");
			break;
		}
	default : 
		{
			l[F]("\t\t\t ET_ERROR (неправильный тип эффекта!)\n");
			return 0;
			//throw CASUS("EffectInfo::LoadEffectParams: Invalid Effect Type!");
		}
	}
	m_next_id++;
	return (m_next_id-1);
}

// из строки-параметра читает float и отрезает от строки все лишнее
float EffectInfo::GetFloatFromString(std::string& str)
{
	float f;
	str.erase(0, str.find_first_of("0123456789.-"));
	f = atof(str.c_str());
	str.erase(0, str.find_first_not_of("0123456789.-"));
	return f;
}

// из строки-параметра читает bool и отрезает от строки все лишнее
bool EffectInfo::GetBoolFromString(std::string& str)
{
	bool b;
	str.erase(0, str.find_first_of("01"));
	if(atoi(str.c_str()) == 1) b = true;
	else b = false;
	str.erase(0, str.find_first_not_of("01"));
	return b;
}

// из строки-параметра читает int и отрезает от строки все лишнее
int EffectInfo::GetIntFromString(std::string& str)
{
	int i;
	str.erase(0, str.find_first_of("0123456789-"));
	i = atoi(str.c_str());
	str.erase(0, str.find_first_not_of("0123456789-"));
	return i;
}

// из строки-параметра читает слова и сохраняет их в вектор
void EffectInfo::SetShadersVector(std::string& str, std::vector<std::string>& vec)
{
	std::string s;

	while(!str.empty())
	{
		// удалить ведущие пробелы и запятые
		str.erase(0, str.find_first_not_of(" ,"));

		if(str.empty()) break;
		// найти следующую запятую и взять слово от начала до нее, удалить слово
		// из строки
		s.clear();
		s.append(str, 0, str.find_first_of(" ,"));
		vec.push_back(s);
		str.erase(0, str.find_first_of(" ,"));
	}
}
