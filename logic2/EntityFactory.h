//
// механизмы создания существ и утилиты для расст. существ
//

#ifndef _PUNCH_ENTITYFACTORY_H_
#define _PUNCH_ENTITYFACTORY_H_

class BaseThing;
class BaseEntity;

//
// интерфейс - конструктор существ
//   
class EntityBuilder{
public:

    EntityBuilder(){}
    virtual ~EntityBuilder(){}
    
    //создать существо (возвращает 0 если существо нельзя создать)
    virtual BaseEntity* CreateHuman(const rid_t& rid);
    virtual BaseEntity* CreateTrader(const rid_t& rid);
    virtual BaseEntity* CreateVehicle(const rid_t& rid);
    
    //разослать уведомление о рождении существа
    virtual void SendSpawnEvent(BaseEntity* entity);
    //установить команду существа
    virtual void SetPlayer(BaseEntity* entity, player_type player);
    //установить метку существа
    virtual void SetAIModel(BaseEntity* entity, const std::string& label);
    //установить информацию о расстановке существа
    virtual void SetSpawnZone(BaseEntity* entity, const std::string& info);
    
    //выбрать точку в которую можно поставить сущетсво
    virtual bool GenerateSuitablePos(BaseEntity* entity, const pnt_vec_t& vec, ipnt2_t* pos);
    //можно ли поставить существо в эту точку
    virtual bool IsSuitableLocation(BaseEntity* entity, const ipnt2_t& pos);
    //снять существо с уровня
    virtual void UnlinkEntity(BaseEntity* entity);
    //поставить существо на уровне
    virtual void LinkEntity(BaseEntity* entity, const ipnt2_t& pos, float angle);

    //может ли существо взять предмет?
    virtual bool CanTake(BaseEntity* entity, BaseThing* thing, human_pack_type pack = HPK_BACKPACK);
    //снабдить существо предметом
    virtual void GiveThing(BaseEntity* entity, BaseThing* thing, human_pack_type pack = HPK_BACKPACK);  
};

#endif // _PUNCH_ENTITYFACTORY_H_