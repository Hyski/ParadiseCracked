//
// ��������� �������� ������� � ������� ��� �����. �������
//

#ifndef _PUNCH_ENTITYFACTORY_H_
#define _PUNCH_ENTITYFACTORY_H_

class BaseThing;
class BaseEntity;

//
// ��������� - ����������� �������
//   
class EntityBuilder{
public:

    EntityBuilder(){}
    virtual ~EntityBuilder(){}
    
    //������� �������� (���������� 0 ���� �������� ������ �������)
    virtual BaseEntity* CreateHuman(const rid_t& rid);
    virtual BaseEntity* CreateTrader(const rid_t& rid);
    virtual BaseEntity* CreateVehicle(const rid_t& rid);
    
    //��������� ����������� � �������� ��������
    virtual void SendSpawnEvent(BaseEntity* entity);
    //���������� ������� ��������
    virtual void SetPlayer(BaseEntity* entity, player_type player);
    //���������� ����� ��������
    virtual void SetAIModel(BaseEntity* entity, const std::string& label);
    //���������� ���������� � ����������� ��������
    virtual void SetSpawnZone(BaseEntity* entity, const std::string& info);
    
    //������� ����� � ������� ����� ��������� ��������
    virtual bool GenerateSuitablePos(BaseEntity* entity, const pnt_vec_t& vec, ipnt2_t* pos);
    //����� �� ��������� �������� � ��� �����
    virtual bool IsSuitableLocation(BaseEntity* entity, const ipnt2_t& pos);
    //����� �������� � ������
    virtual void UnlinkEntity(BaseEntity* entity);
    //��������� �������� �� ������
    virtual void LinkEntity(BaseEntity* entity, const ipnt2_t& pos, float angle);

    //����� �� �������� ����� �������?
    virtual bool CanTake(BaseEntity* entity, BaseThing* thing, human_pack_type pack = HPK_BACKPACK);
    //�������� �������� ���������
    virtual void GiveThing(BaseEntity* entity, BaseThing* thing, human_pack_type pack = HPK_BACKPACK);  
};

#endif // _PUNCH_ENTITYFACTORY_H_