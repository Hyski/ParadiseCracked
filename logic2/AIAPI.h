/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� API ��� ������������� ������� � ������� ��������
                                                                                
                                                                                
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
// ����� ��� ��������� ���� ����� (�����������)
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
// ����� ��� ��������� ���� ����� ��� ������������� ���� �����
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
// ����� ��� ��������� ���� ����� ��� ����������� ���� �����
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
// ����� ��� ��������� ���� ����� ��� ���������� ���� �����
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
// ����� ��� ��������� ���� ������ 
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class EnemyComparator
{
public:
	// �������� ������������ - id ��������, �� ��������� � ��������
	// ������������ ��������
	EnemyComparator(eid_t id) { m_id = id; }
	// ���������� true, ���� ������ ������� ������� (human&vehicle)
	virtual bool operator() (eid_t id1, eid_t id2) const;
protected:
	// ���������� �������� ��������� ��������� �������� (human&vehicle)
	virtual float getEntityDanger(eid_t entity) const;
private:
	eid_t m_id;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ (�����������)
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class HexComparator
{
public:
	// ����������� � id �������� � id �����
	HexComparator(eid_t im, eid_t enemy);// (human&vehicle)
	// ���������� true, ���� ������ ����� �������
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
// ����� ��� ��������� ���� ������ ��� ������������� ���� �����
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class FixedHexComparator : public HexComparator
{
public:
	// ����������� � id �������� � id �����
	// (human&vehicle)
	FixedHexComparator(eid_t im, eid_t enemy);
	// ���������� true, ���� ������ ����� �������
	// (human&vehicle)
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ����������� ���� �����
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class PatrolHexComparator : public HexComparator
{
public:
	// ����������� � id �������� � id �����
	// (human&vehicle)
	PatrolHexComparator(eid_t im, eid_t enemy);
	// ���������� true, ���� ������ ����� �������
	// (human&vehicle)
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ���������� ���� �����
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class AssaultHexComparator : public HexComparator
{
public:
	// ����������� � id �������� � id �����
	// (human&vehicle)
	AssaultHexComparator(eid_t im, eid_t enemy);
	// (human&vehicle)
	// ���������� true, ���� ������ ����� �������
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ������� �������
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class WalkingTechHexComparator : public HexComparator
{
public:
	// ����������� � id �������� � id �����
	// (human&vehicle)
	WalkingTechHexComparator(eid_t im, eid_t enemy);
	// (human&vehicle)
	// ���������� true, ���� ������ ����� �������
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ������� � �����
// � ������ ���������� ����� ��������
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
class NoLineOfFireHexComparator : public HexComparator
{
public:
	// ����������� � id �������� � id �����
	// (human&vehicle)
	NoLineOfFireHexComparator(eid_t im, eid_t enemy);
	// (human&vehicle)
	// ���������� true, ���� ������ ����� �������
	virtual bool operator() (const ipnt2_t& p1, const ipnt2_t& p2) const;
};

/////////////////////////////////////////////////////////////////////////////////
//
// �����, ��������������� API (������ ��������)
//
/////////////////////////////////////////////////////////////////////////////////
class AIAPI
{
public:
	// �������� ��������� �� ������������ ������ ������
	static AIAPI * getInst();

	// ����������
	~AIAPI();

	// �������� ������� �� ������������� ��������, ����������� � ��������
	float getHealthPercent(eid_t id) const; // (human&vehicle)
	// �������� ������� �� ����������, ���������� � ��������
	float getMovePointsPercent(eid_t id) const; // (human&vehicle)
	// �������� ���������� ����������, ���������� � ��������
	int getMovePoints(eid_t id) const; // (human&vehicle)
	// �������� ���������� �����, ���������� � ��������
	int getSteps(eid_t id) const; // (human&vehicle)
	// �������� ���������� �����, ������� ����� ������� ��������
	// � �������� ����������� ����������
	int getSteps(eid_t id, int movepoints) const; // (human&vehicle)

	// ���������� ��� �������� � �������� (����������� ����������� aim,
	// � snap ����� ��������)
	void setShootType(eid_t id, float aim, float snap); // (human only)

	// ����� � ���� ������ ���������� ������ (���� ���������� �����
	// ���, ������ false) �� ���� ���� ����������� ����� ����� ����������
	// ���������� ��������� � ������ ������ - ������������
	// ������ �������� - ����� ��������� ���� �����
	bool takeBestWeapon(eid_t id, const WeaponComparator& wc); // (human only)
	// ����� � ���� ������� (���� ��� - ������ false)
	bool takeGrenade(eid_t id); // (human only)
	// ����� � ���� ���� (���� ��� - ������ false)
	bool takeShield(eid_t id); // (human only)
	// ���� �� � ������� ���������� ������
	bool haveTechWeapon(eid_t id); // (vehicle only)
	// �������� ���������� ����� ������������� ���� � ��������
	int getThingCount(eid_t id, thing_type tt); // (human only)

	// �������� ���������� ���������� ����������� �� �������
	// ������ 0 ���� ��� ������
	int getShootMovePoints(eid_t id, shot_type st); // (human&vehicle)

	// ������ �������
	typedef std::list<eid_t> entity_list_t;

	// ������ �������
	typedef std::vector<eid_t> entity_vector_t;

	// �������� ���� ������� ������ ������
	// ���������� true ���� ���� ���� �� ���� ����
	// (human&vehicle)
	bool getEnemiesVisibleByEntity(eid_t id, entity_list_t* enemies);

	// �������� ���� ������� ������ �������
	// ���������� true ���� ���� ���� �� ����� ��������
	// ��������� �������� - ��� �������
	// (���� 0 - � ����� �������)
	// (human&vehicle)
	bool getEntitiesVisibleByEntity(eid_t id, entity_vector_t* entities, entity_type et);

	// �������� ���� ������� ������ �������
	// ���������� true ���� ���� ���� �� ����� ��������
	// ������������� �������� - ��� �������
	// ��������� �������� - ����, � ������� ������ ���������� ��������
	// (���� 0 - � ����� �������)
	// (human&vehicle)
	bool getEntitiesVisibleByEntity(eid_t id, entity_vector_t* entities, entity_type et, const pnt_vec_t& field);

	// �������� ���� ������� ����������� ������
	// ���������� true ���� ���� ���� �� ���� ����
	// (human&vehicle)
	bool getEnemiesVisibleBySubteam(const std::string& subteam, entity_list_t* enemies);

	// ��������� ���� �� � ������ ��������
	// (human&vehicle)
	bool isEntityVisible(eid_t im, eid_t entity);

	// �������� id ������ �������� �� ������� ������
	// ������ �������� - ����� ��� ��������� ���� ������ �� ������� ������
	// ���������� 0 ���� ��� ������� ������
	// (human&vehicle)
	eid_t getMostDangerousEnemy(const entity_list_t& enemies, const EnemyComparator& ec);

	// �������� id ������ ���������� �� ������� ������
	// ������ �������� - ����� ��� ��������� ���� ������ �� ������� ������
	// ���������� 0 ���� ��� ������� ������
	// (human&vehicle)
	eid_t getLeastDangerousEnemy(const entity_list_t& enemies, const EnemyComparator& ec);

	// �������� id ���������� �������� �� ������
	// ���������� 0 ���� ��� �������
	// (human&vehicle)
	eid_t getRandomEntity(const entity_vector_t& entities);

	// ��������� ���� �� � ������� ��������� ����� ������ ��� �������
	// ���� ���� - ���������� true � ��������� �����
	bool getWeaponLocation(eid_t id, ipnt2_t *pnt); // (human&vehicle)
	// ��������� ���� �� � ������� ��������� ����� ���� ���������������� ����
	// ���� ���� - ���������� true � ��������� �����
	bool getThingLocation(eid_t id, ipnt2_t *pnt, int type); // (human&vehicle)
	// ������� ��� �������, ������� ����� � ������
	void pickupAllNearAmmo(eid_t id); // (human only)
	// ������� ��� �������, ������� ����� � ������
	void pickupAllNearMedikit(eid_t id); // (human only)
	// ������� ��� ������, ������� ����� � ������
	void pickupAllNearWeapon(eid_t id); // (human only)
	// �������� ��� ������ ������ � �������� �� ����� ���� ������ �����
	void dropUselessWeapon(eid_t id, const WeaponComparator& wc); // (human only)
	// ������� ������ ���������� �����, ������� ����� � ������
	// ���������� true ���� ������� ������� ���-������
	bool pickupNearWeapon(eid_t id); // (human only)

	// ��������� �������� �����
	void setSitPose(eid_t id); // (human only)
	// ��������� �������� ������
	void setStandPose(eid_t id); // (human only)
	// ����������� �� ���
	void setRun(eid_t id); // (human only)
	// ����������� �� ������
	void setWalk(eid_t id); // (human only)
	// ���������, ����� �� �������
	bool isSitPose(eid_t id); // (human only)
	// ���������, ����� �� �������
	bool isStandPose(eid_t id); // (human only)

	// ���������, �������� �� ������� ������������ ����������
	// (human&vehicle)
	bool isSafeShot(eid_t id, WeaponThing* weapon, const point3& target);
	// ���������, �������� �� ������ ������� ������������ ����������
	// (human only)
	bool isSafeThrow(eid_t id, GrenadeThing* grenade, const point3& target);
	// ���������� �� �������� ���� ��� ��������
	// (human only)
	bool shoot(eid_t im, eid_t enemy, Activity** activity, std::string* reason);
	// (vehicle only)
	bool shootByVehicle(eid_t im, eid_t enemy, Activity** activity, std::string* reason);
	bool shootByVehicle(eid_t im, const point3& target, Activity** activity, std::string* reason);
	// ���� ��������� �������� ����� ����� ���� �����
	// (human only)
	bool canAttackByGrenadeMultipleTarget(eid_t im, eid_t enemy, const entity_list_t& enemies);
	// ���������� � ��������� �����, ���� ��� ��������
	// (human only)
	bool shootRandom(eid_t im, Activity** activity);
	// ������ ���� ������ �����, ���� ��� ��������
	// (human only)
	bool throwShield(eid_t im, eid_t enemy, Activity** activity);

	// �������� ������ ������� ������, ����������� �� ���������� �� �����
	// ������� �� ����� �����������
	// (human&vehicle)
	void getBaseField(const ipnt2_t &center, int radius, pnt_vec_t *vector);
	// �������� �� ������� ������ ������ ������, � ������� ����� �����
	// (human&vehicle)
	void getReachableField(const pnt_vec_t &invector, pnt_vec_t *outvector, bool busaware = false);
	// �������� �� ������� ������ ������ ������, � ������� ����� �������� � ������
	// (human&vehicle)
	void getPanicReachableField(const pnt_vec_t &invector, pnt_vec_t *outvector);
	// �������� ��������� ���� �� ������� ������
	ipnt2_t getRandomPoint(const pnt_vec_t &vector);// (human&vehicle)
	// �������� ������ ���� �� ������� ������
	// (human&vehicle)
	ipnt2_t getBestPoint(const pnt_vec_t &vector, const HexComparator& hc);
	// ���������, ��������� �� �������� � ������� ����
	bool isEntityInBaseField(eid_t id, const pnt_vec_t &vector);// (human&vehicle)
	// �������� ������� �������� �� ���������
	ipnt2_t getPos2(eid_t id); // (human&vehicle)
	ipnt2_t getPos2(BaseEntity* be); // (human&vehicle)
	// �������� ���������� ������� ��������
	point3 getPos3(eid_t id); // (human&vehicle)
	// �������� ����, �� ������� ��������� ��������
	float getAngle(eid_t id); // (human&vehicle)

	// ���� �� � �������� �������
	bool haveMedikit(eid_t id); // (human only)
	// ����� � ���� �������
	void takeMedikit(eid_t id); // (human only)
	// ������� �����
	void cure(eid_t doctor, eid_t sick); // (human only)

	// �������� ���������� ���������� �� ��������
	point3 convertPos2ToPos3(const ipnt2_t& pnt);
	// �������� ��������� �� ��������
	BaseEntity* getPtr(eid_t id);
	// ������� � ������� ���������
	void print(eid_t id, const std::string& str);

private:
	
	// ���������, �������� �� ����� � ���� � ��� ������ ������ - �������� ��
	bool isWeaponLoaded(WeaponThing* wth); // (human&vehicle)
	// �������� ������ ���� ��� ����� � �������� (���������� false ����
	// ������ �������� ������������)
	bool reloadWeapon(eid_t id, WeaponThing* wth); // (human only)

	// ������ ����������� ����� ����� �� ��� ������� ������ ������ ���
	AIAPI();

	// ����������� ��������� ��� �������� � �����������
	// ��������, ����������� � ����������� ����������
	struct Container
	{
		// ��������� �� ������������ ������
		AIAPI * m_instance;

		~Container()
		{
			delete m_instance;
		}
	};

	static Container m_container;
};

#endif // __AI_API_H__