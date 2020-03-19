//
// Основные определения константы и т.д.
//

#ifndef _PUNCH_LOGICDEFS_H_
#define _PUNCH_LOGICDEFS_H_

//предотврашаем повторное включение
#ifndef _PRECOMP_H_
#include "../common/precomp.h"
#endif 

#include "observer.h" //система наблюдателей
#include "DynUtils.h" //система создания объектов runtime

#ifdef _HOME_VERSION
    extern CLog logFile;
    #define theLog logFile["logic.log"]
#else
    #define theLog /##/
#endif

//перечисление - ограничения игры
enum game_limits{
    MAX_BUSES     = 32,   //максимальное кол-во автобувос на уровне  
    MAX_TEAMS     = 3,    //максимальное кол-во комманд в игре
    MAX_ENTITIES  = 128,  //максимальное кол-во существ в игре
    MAX_JOINTS    = 8,    //максимальное кол-во выходов с уровня
    MAX_CREW      = 8,    //максимальное кол-во людей в технике 
    MAX_TEAMMATES = 6,    //максимальное кол-во unit'в в команде игрока

    DMG_DURATION  = 3,   //продолжит воздействия  повреждения

    MAX_FRONT_RADIUS = 40, //максимальный радиус видения

    GROUND_FIRE_DURATION    = 3, //продолжит грения земли
    GROUND_FIRE_PATH_LENGTH = 5, //длина пути кот. надо пройти по огню чтобы повредить существо 

    BUS_AND_CAR_STOP_DURATION = 5, //максимальная величина задержки для автобуса и машины 

    USUAL_PANIC_DURATION   = 0,  //продолжит паники  
    SHOCK_PANIC_DURATION   = 4,  //продолжит паники  
    BERSERK_PANIC_DURATION = 0,  //продолжит паники  

    MPS_FOR_HACK        = 10, //кол-во ходов для взлома 
    MPS_FOR_ROTATE      = 1,  //кол-во ходов на поворот
    MPS_FOR_LANDING     = 20, //кол-во ходов на выгрузку
    MPS_FOR_POSE_CHANGE = 3,  //кол-во ходов на смену позы
};

//определения полезных типов
typedef std::string rid_t;  //resource identifier
typedef unsigned    eid_t;  //entity identifier         
typedef unsigned    gid_t;  //graphic id (id графичесуой оболочки)
typedef unsigned    tid_t;  //thing id

typedef std::set<rid_t>      rid_set_t;
typedef std::vector<ipnt2_t> pnt_vec_t;
typedef std::vector<point3>  pnt3_vec_t;

//нужно практически везде для работы с вероятностью
inline float NormRand(){return static_cast<float>(rand())/static_cast<float>(RAND_MAX);}
//перевод направления в угол поворота существа
inline float Dir2Angle(const point3& dir){return atan2(dir.x, -dir.y);}
//сгенерировать случайное целое от 0 до range
inline int RangeRand(int range)
{
    double index = (range - 1) * NormRand();
    return modf(index, &index) < 0.5 ? index : index + 1;    
}

//
//  ВНИМАНИЕ:  Не существует entity с id == 0
//

//
// Перечисление всех возможных типов игроков
// должно занимать биты с 0 - 7
//
enum player_type{
    PT_NONE     = 1 << 0,     

    PT_ENEMY    = 1 << 1, //противник
    PT_PLAYER   = 1 << 2, //настоящий игрок
    PT_CIVILIAN = 1 << 3, //команда мирных жителей

    PT_ALL_PLAYERS = PT_ENEMY | PT_PLAYER | PT_NONE | PT_CIVILIAN,
};

//
// Перечисление всех возможных типов существ 
// должно занимать биты с 8-15
//
enum entity_type{
    ET_NONE    = 0,

    ET_HUMAN   = 1 << 8, //человек
    ET_VEHICLE = 1 << 9, //техника
    ET_TRADER  = 1 << 10,
 
    ET_ALL_ENTS = ET_HUMAN | ET_VEHICLE | ET_TRADER,
};

//
// Свойства существ: должно занимать биты с 16 - 23
//
enum entity_attribute{
    EA_NOT_INCREW    = 1 << 16,
    EA_INCREW        = 1 << 17, 
    EA_WAS_IN_INV    = 1 << 19, //человек был в inventory
    EA_LOCK_TRAITS   = 1 << 20, //запретить изменение свойств существа
    EA_EXPLOSIVE_DMG = 1 << 21, //существу был нанесено взрывное ранение

    EA_ALL_ATTRIBUTES = EA_NOT_INCREW|EA_INCREW|EA_WAS_IN_INV|EA_LOCK_TRAITS|EA_EXPLOSIVE_DMG,
};

//
// состояние хода
//
enum state_type{
    ST_INCOMPLETE,   //неокончен
    ST_FINISHED,     //закончен
    ST_DESTROYED,    //только для узла AI - узел уничтожен
};

//
// Типы элементов в Inventory
//
enum thing_type{
    TT_WEAPON  = 1 << 0,
    TT_AMMO    = 1 << 1,
    TT_GRENADE = 1 << 2,
    TT_IMPLANT = 1 << 3,
    TT_ARMOUR  = 1 << 4,
    TT_MEDIKIT = 1 << 5,
    TT_MONEY   = 1 << 6,
    TT_KEY     = 1 << 7,
    TT_CAMERA  = 1 << 8,
    TT_SHIELD  = 1 << 9,
    TT_SCANNER = 1 << 10, 

    MAX_THINGS,   

    TT_ALL_ITEMS = TT_WEAPON|TT_MONEY|TT_GRENADE|TT_KEY|TT_SHIELD|TT_ARMOUR|TT_AMMO|TT_MEDIKIT|TT_CAMERA|TT_IMPLANT|TT_SCANNER,

    //предметы кот для активации надо бросать как гранату
    TT_THROWABLE = TT_GRENADE|TT_CAMERA|TT_SHIELD,
};

//
// части тела
//
enum body_parts_type{
    BPT_NONE,
    BPT_HEAD,
    BPT_BODY,
    BPT_HANDS,
    BPT_LEGS,

    MAX_BODY_PARTS,
};

//
// стойка для человека
//
enum hold_type{
    HT_NONE,
    HT_RIFLE,
    HT_PISTOL,
    HT_CANNON,
    HT_AUTOCANNON,
    HT_ROCKET,
    HT_SPECIAL,
    HT_GRENADE,
    HT_FREEHANDS,
};

//
// тип поражения
//
enum damage_type{
    DT_NONE,
    DT_STRIKE,
    DT_SHOCK,
    DT_ENERGY,
    DT_ELECTRIC,
    DT_EXPLOSIVE,
    DT_FLAME,
};

//
// поражение 
//
struct damage_s{
    int         m_val;
    damage_type m_type;

    damage_s() : m_val(0), m_type(DT_NONE) {}
};

//
// типы стрельбы 
//
enum shot_type {
    SHT_AIMSHOT,
    SHT_SNAPSHOT,
    SHT_AUTOSHOT,
};

//
// перечисление слотов на человеке
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ВНИМАНИЕ: 
//      порядок следования констант важен, при его изменении
//      необх изменить карту слотов у человека (pack_map)
//
enum human_pack_type{
    HPK_NONE,
    HPK_HEAD,
    HPK_BODY,
    HPK_HANDS,
    HPK_LKNEE,
    HPK_RKNEE,
    HPK_IMPLANTS,
    HPK_BACKPACK,
    HPK_ALL,
        
    MAX_PACKS,
};

//
// типы паники
//
enum human_panic_type{
    HPT_NONE,
    HPT_USUAL,
    HPT_SHOCK,
    HPT_BERSERK,

    MAX_PANICS,
};

//
// типы бронежилетов
//
enum armor_type{
    ARM_NONE,
    ARM_NORM,
    ARM_SUIT1,
    ARM_SUIT2, 
    ARM_SUIT3,
    ARM_SUIT4,
    ARM_SCUBA,
};

//
// перечисление типов отношений одной подкоманды к другой
//
enum RelationType
{
	RT_NEUTRAL,
	RT_ENEMY,
	RT_FRIEND
};

#endif //  _PUNCH_LOGICDEFS_H_