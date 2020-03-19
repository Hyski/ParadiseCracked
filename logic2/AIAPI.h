/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: класс API для использования врагами и мирными жителями
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__AI_API_H__)
#define __AI_API_H__

class BaseEntity;
class WeaponThing;
class GrenadeThing;
class Activity;

/////////////////////////////////////////////////////////////////////////////////
//
// класс для сравнения двух пушек (абстрактный)
//
/////////////////////////////////////////////////////////////////////////////////
class WeaponComparator
{
public:
	// (human&vehicle)
	virtual bool operator() (const WeaponThing&, const WeaponThing&) const = 0;
};

/////////////////////////////////////////////////////////////////////////////////
//
// класс для сравнения двух пушек для стационарного типа юнита
//
/////////////////////////////////////////////////////////////////////////////////
class FixedWeaponComparator : public WeaponComparator
{
public:
	// (human&vehicle)
	virtual bool operator() (const WeaponThing&, const WeaponThing&) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// класс для сравнения двух пушек для патрульного типа юнита
//
/////////////////////////////////////////////////////////////////////////////////
class PatrolWeaponComparator : public WeaponComparator
{
public:
	// (human&vehicle)
	virtual bool operator() (const WeaponThing&, const WeaponThing&) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// класс для сравнения двух пушек для штурмового типа юнита
//
/////////////////////////////////////////////////////////////////////////////////
class AssaultWeaponComparator : public WeaponComparator
{
public:
	// (human&vehicle)
	virtual bool operator() (const WeaponThing&, const WeaponThing&) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// класс для сравнения двух врагов 
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class EnemyComparator
{
public:
	// параметр конструктора - id существа, по отношению к которому
	// сравниваются существа
	EnemyComparator(eid_t id) { m_id = id; }
	// возвращает true, если первый опаснее второго (human&vehicle)
	virtual bool operator() (eid_t id1, eid_t id2) const;
protected:
	// возвращает значение суммарной опасности существа (human&vehicle)
	virtual float getEntityDanger(eid_t entity) const;
private:
	eid_t m_id;
};

/////////////////////////////////////////////////////////////////////////////////
//
// класс для сравнения двух хексов (абстрактный)
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class HexComparator
{
public:
	// конструктор с id существа и id врага
	HexComparator(eid_t im, eid_t enemy);// (human&vehicle)
	// возвращает true, если первый лучше второго
	// (human&vehicle)
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const = 0;
	eid_t getIm() const { return m_im; }// (human&vehicle)
	eid_t getEnemy() const { return m_enemy; }// (human&vehicle)
	point3 getImPos() const { return m_im_pos; }// (human&vehicle)
	point3 getEnemyPos() const { return m_enemy_pos; }// (human&vehicle)
private:
	eid_t m_im;
	eid_t m_enemy;
	point3 m_im_pos;
	point3 m_enemy_pos;
};

/////////////////////////////////////////////////////////////////////////////////
//
// класс для сравнения двух хексов для стационарного типа юнита
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class FixedHexComparator : public HexComparator
{
public:
	// конструктор с id существа и id врага
	// (human&vehicle)
	FixedHexComparator(eid_t im, eid_t enemy);
	// возвращает true, если первый лучше второго
	// (human&vehicle)
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// класс для сравнения двух хексов для патрульного типа юнита
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class PatrolHexComparator : public HexComparator
{
public:
	// конструктор с id существа и id врага
	// (human&vehicle)
	PatrolHexComparator(eid_t im, eid_t enemy);
	// возвращает true, если первый лучше второго
	// (human&vehicle)
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// класс для сравнения двух хексов для штурмового типа юнита
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class AssaultHexComparator : public HexComparator
{
public:
	// конструктор с id существа и id врага
	// (human&vehicle)
	AssaultHexComparator(eid_t im, eid_t enemy);
	// (human&vehicle)
	// возвращает true, если первый лучше второго
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// класс для сравнения двух хексов для ходящей техники
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class WalkingTechHexComparator : public HexComparator
{
public:
	// конструктор с id существа и id врага
	// (human&vehicle)
	WalkingTechHexComparator(eid_t im, eid_t enemy);
	// (human&vehicle)
	// возвращает true, если первый лучше второго
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// класс для сравнения двух хексов для подхода к врагу
// в случае отсутствия линии стрельбы
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class NoLineOfFireHexComparator : public HexComparator
{
public:
	// конструктор с id существа и id врага
	// (human&vehicle)
	NoLineOfFireHexComparator(eid_t im, eid_t enemy);
	// (human&vehicle)
	// возвращает true, если первый лучше второго
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// класс, предоставляющий API (шаблон одиночка)
//
/////////////////////////////////////////////////////////////////////////////////
class AIAPI
{
public:
	// получить указатель на единственный объект класса
	static AIAPI * getInst();

	// деструктор
	~AIAPI();

	// получить процент от максимального здоровья, оставшегося у существа
	float getHealthPercent(eid_t id) const; // (human&vehicle)
	// получить процент от мувпоинтов, оставшихся у существа
	float getMovePointsPercent(eid_t id) const; // (human&vehicle)
	// получить количество мувпоинтов, оставшихся у существа
	int getMovePoints(eid_t id) const; // (human&vehicle)
	// получить количество шагов, оставшихся у существа
	int getSteps(eid_t id) const; // (human&vehicle)
	// получить количество шагов, которое может сделать существо
	// с заданным количеством мувпоинтов
	int getSteps(eid_t id, int movepoints) const; // (human&vehicle)

	// установить тип стрельбы у человека (указывается вероятность aim,
	// и snap типов стрельбы)
	void setShootType(eid_t id, float aim, float snap); // (human only)

	// взять в руки лучшее заряженное оружие (если заряженных пушек
	// нет, вернет false) по ходу дела разряженные пушки будут заряжаться
	// имеющимися патронами а пустые обоймы - выкидываться
	// второй параметр - класс сравнения двух пушек
	bool takeBestWeapon(eid_t id, const WeaponComparator& wc); // (human only)
	// взять в руки гранату (если нет - вернет false)
	bool takeGrenade(eid_t id); // (human only)
	// взять в руки шилд (если нет - вернет false)
	bool takeShield(eid_t id); // (human only)
	// есть ли у техники заряженное оружие
	bool haveTechWeapon(eid_t id); // (vehicle only)
	// получить количество вещей определенного типа у существа
	int getThingCount(eid_t id, thing_type tt); // (human only)

	// получить количество мувпоинтов необходимых на выстрел
	// вернет 0 если нет оружия
	int getShootMovePoints(eid_t id, shot_type st); // (human&vehicle)

	// список существ
	typedef std::list<eid_t> entity_list_t;

	// массив существ
	typedef std::vector<eid_t> entity_vector_t;

	// получить всех видимых юнитом врагов
	// возвращает true если есть хотя бы один враг
	// (human&vehicle)
	bool getEnemiesVisibleByEntity(eid_t id, entity_list_t* enemies);

	// получить всех видимых юнитом существ
	// возвращает true если есть хотя бы одино существо
	// последний параметр - тип существ
	// (если 0 - в любом радиусе)
	// (human&vehicle)
	bool getEntitiesVisibleByEntity(eid_t id, entity_vector_t* entities, entity_type et);

	// получить всех видимых юнитом существ
	// возвращает true если есть хотя бы одино существо
	// предпоследний параметр - тип существ
	// последний параметр - поле, в котором должны находиться существа
	// (если 0 - в любом радиусе)
	// (human&vehicle)
	bool getEntitiesVisibleByEntity(eid_t id, entity_vector_t* entities, entity_type et, const pnt_vec_t& field);

	// получить всех видимых подкомандой врагов
	// возвращает true если есть хотя бы один враг
	// (human&vehicle)
	bool getEnemiesVisibleBySubteam(const std::string& subteam, entity_list_t* enemies);

	// проверить вижу ли я данное существо
	// (human&vehicle)
	bool isEntityVisible(eid_t im, eid_t entity);

	// получить id самого опасного из видимых врагов
	// второй параметр - класс для сравнения двух врагов по степени угрозы
	// возвращает 0 если нет видимых врагов
	// (human&vehicle)
	eid_t getMostDangerousEnemy(const entity_list_t& enemies, const EnemyComparator& ec);

	// получить id самого неопасного из видимых врагов
	// второй параметр - класс для сравнения двух врагов по степени угрозы
	// возвращает 0 если нет видимых врагов
	// (human&vehicle)
	eid_t getLeastDangerousEnemy(const entity_list_t& enemies, const EnemyComparator& ec);

	// получить id случайного существа из списка
	// возвращает 0 если нет существ
	// (human&vehicle)
	eid_t getRandomEntity(const entity_vector_t& entities);

	// проверить есть ли в области видимости юнита оружие или патроны
	// если есть - возвращает true и заполняет точку
	bool getWeaponLocation(eid_t id, ipnt2_t *pnt); // (human&vehicle)
	// проверить есть ли в области видимости юнита вещь соответствующего типа
	// если есть - возвращает true и заполняет точку
	bool getThingLocation(eid_t id, ipnt2_t *pnt, int type); // (human&vehicle)
	// поднять все патроны, лежащие рядом с юнитом
	void pickupAllNearAmmo(eid_t id); // (human only)
	// поднять все аптечки, лежащие рядом с юнитом
	void pickupAllNearMedikit(eid_t id); // (human only)
	// поднять все оружие, лежащее рядом с юнитом
	void pickupAllNearWeapon(eid_t id); // (human only)
	// выкинуть все лишнее оружие и оставить не более двух лучших пушек
	void dropUselessWeapon(eid_t id, const WeaponComparator& wc); // (human only)
	// поднять первую попавшуюся пушку, лежащую рядом с юнитом
	// возвращает true если удалось поднять что-нибудь
	bool pickupNearWeapon(eid_t id); // (human only)

	// заставить человека сесть
	void setSitPose(eid_t id); // (human only)
	// заставить человека встать
	void setStandPose(eid_t id); // (human only)
	// переключить на бег
	void setRun(eid_t id); // (human only)
	// переключить на ходьбу
	void setWalk(eid_t id); // (human only)
	// проверить, сидит ли человек
	bool isSitPose(eid_t id); // (human only)
	// проверить, стоит ли человек
	bool isStandPose(eid_t id); // (human only)

	// проверить, является ли выстрел потенциально безопасным
	// (human&vehicle)
	bool isSafeShot(eid_t id, WeaponThing* weapon, const point3& target);
	// проверить, является ли бросок гранаты потенциально безопасным
	// (human only)
	bool isSafeThrow(eid_t id, GrenadeThing* grenade, const point3& target);
	// выстрелить по существу если это возможно
	// (human only)
	bool shoot(eid_t im, eid_t enemy, Activity** activity, std::string* reason);
	// (vehicle only)
	bool shootByVehicle(eid_t im, eid_t enemy, Activity** activity, std::string* reason);
	bool shootByVehicle(eid_t im, const point3& target, Activity** activity, std::string* reason);
	// могу атаковать гранатой более одной цели сразу
	// (human only)
	bool canAttackByGrenadeMultipleTarget(eid_t im, eid_t enemy, const entity_list_t& enemies);
	// выстрелить в случайную точку, если это возможно
	// (human only)
	bool shootRandom(eid_t im, Activity** activity);
	// кинуть шилд против врага, если это возможно
	// (human only)
	bool throwShield(eid_t im, eid_t enemy, Activity** activity);

	// получить вектор центров хексов, находящихся на расстоянии не более
	// радиуса по обоим координатам
	// (human&vehicle)
	void getBaseField(const ipnt2_t &center, int radius, pnt_vec_t *vector);
	// получить из вектора хексов вектор хексов, в которые можно дойти
	// (human&vehicle)
	void getReachableField(const pnt_vec_t &invector, pnt_vec_t *outvector, bool busaware = false);
	// получить из вектора хексов вектор хексов, в которые можно добежать в панике
	// (human&vehicle)
	void getPanicReachableField(const pnt_vec_t &invector, pnt_vec_t *outvector);
	// получить случайный хекс из вектора хексов
	ipnt2_t getRandomPoint(const pnt_vec_t &vector);// (human&vehicle)
	// получить лучший хекс из вектора хексов
	// (human&vehicle)
	ipnt2_t getBestPoint(const pnt_vec_t &vector, const HexComparator& hc);
	// проверить, находится ли существо в базовом поле
	bool isEntityInBaseField(eid_t id, const pnt_vec_t &vector);// (human&vehicle)
	// получить позицию существа на хексгриде
	ipnt2_t getPos2(eid_t id); // (human&vehicle)
	ipnt2_t getPos2(BaseEntity* be); // (human&vehicle)
	// получить трехмерную позицию существа
	point3 getPos3(eid_t id); // (human&vehicle)
	// получить угол, на который повернуто существо
	float getAngle(eid_t id); // (human&vehicle)

	// есть ли у существа аптечка
	bool haveMedikit(eid_t id); // (human only)
	// взять в руки аптечку
	void takeMedikit(eid_t id); // (human only)
	// полечим юнита
	void cure(eid_t doctor, eid_t sick); // (human only)

	// получить трехмерные координаты по хексовым
	point3 convertPos2ToPos3(const ipnt2_t& pnt);
	// получить указатель на существо
	BaseEntity* getPtr(eid_t id);
	// вывести в консоль сообщение
	void print(eid_t id, const std::string& str);

private:
	
	// проверить, заряжена ли пушка и если в ней пустая обойма - выкинуть ее
	bool isWeaponLoaded(WeaponThing* wth); // (human&vehicle)
	// зарядить оружие если это нужно и возможно (возвращает false если
	// оружие осталось незаряженным)
	bool reloadWeapon(eid_t id, WeaponThing* wth); // (human only)

	// скроем конструктор чтобы никто не мог создать объект класса сам
	AIAPI();

	// специальная структура для хранения и уничтожения
	// объектов, находящихся в статических указателях
	struct Container
	{
		// указатель на единственный объект
		AIAPI * m_instance;

		~Container()
		{
			delete m_instance;
		}
	};

	static Container m_container;
};

#endif // __AI_API_H__