/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: интерфейс для скриптовых сцен со стороны основного проекта
				
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#include "scriptpch.h"
//	обработка данных
#include "../Common/DataMgr/DataMgr.h"
#include "../Common/saveload/saveload.h"
//	уровень
#include "../IWorld.h"
#include "../GameLevel/GameLevel.h"
// для логики
#include "../logic2/logicdefs.h"
#include "../logic2/Entity.h"
#include "../logic2/Activity.h"
#include "../logic2/PathUtils.h"
#include "../logic2/Graphic.h"
#include "../logic2/HexGrid.h"
#include "../logic2/hexutils.h"
#include "../logic2/DirtyLinks.h"
#include "../logic2/AIUtils.h"
#include "../logic2/Spawn.h"
#include "../logic2/EntityFactory.h"
#include "../logic2/Thing.h"
#include "../logic2/ThingFactory.h"
#include "../logic2/Form.h"
#include "../logic2/PhraseManager.h"
//	для квестов
#include "../logic2/questengine.h"
//	для камеры
#include "../Common/GraphPipe/Camera.h"
//	для анимации
#include "../Common/GraphPipe/GraphPipe.h"
#include "../Skin/AnimaLibrary.h"
#include "../Skin/SkAnim.h"
//	для интерфейса
#include "../Interface/MouseCursor.h"
#include "../Interface/GameScreen.h"
#include "../Interface/Interface.h"
#include "../Interface/Screens.h"
//	для эффектов
#include "../GameLevel/ExplosionManager.h"
//	для скриптовых сцен
#include "ScriptSceneAPI.h"

extern float SpeedMul;
extern float SkSpeed;

//**********************************************************************
//	class ScriptSceneAPI
ScriptSceneAPI::Deleter ScriptSceneAPI::m_Deleter;
ScriptSceneAPI::ScriptSceneAPI()
{
}

ScriptSceneAPI::~ScriptSceneAPI()
{
}

//************** Управление скриптовой сценой **************************
//	инициализация скриптовой сцены
void ScriptSceneAPI::StartScene(void)
{
	//	установим нормальную скорость анимации
	m_AnimationSpeed1 = SpeedMul;
	m_AnimationSpeed2 = SkSpeed;
	SpeedMul = 2.0f;
	SkSpeed  = 2.0f;
	//	включим крыши
    m_RoofVisible = IWorld::Get()->GetLevel()->RoofVisible;
    IWorld::Get()->GetLevel()->RoofVisible = true;
	//	подключим интерфейс
	Screens::Instance()->Game()->OpenScriptSceneScreen();
}
//	завершение скриптовой сцены
void ScriptSceneAPI::FinishScene(void)
{
	// установим скорость анимации, которую хотел пользователь
	SpeedMul = m_AnimationSpeed1;
	SkSpeed  = m_AnimationSpeed2;
	//	выключим крыши если надо
    IWorld::Get()->GetLevel()->RoofVisible = m_RoofVisible;
	//	отключим интерфейс
	Screens::Instance()->Game()->CloseScriptSceneScreen();
}
//	получить текущее время работы программы
float ScriptSceneAPI::GetTime(void) const
{
	::Timer::Update();
	return ::Timer::GetSeconds();
}
//	выйти на другой уровень
void ScriptSceneAPI::ExitFromLevel(const ipnt2_t& pt)
{
	std::string level_name;

	if(Spawner::GetInst()->CanExit(HexGrid::GetInst()->GetProp(pt).m_joints,&level_name))
	{
		//сохрание по выходе с уровня
//		Screens::Instance()->Game()->MakeSpecialSaveLoad(GameScreen::SST_AUTOSAVE, GameScreen::SSM_SAVE);
		//	выход с уровня
		Spawner::GetInst()->ExitLevel(level_name);
	}
}
//	получить фразу из xls'ки
std::string ScriptSceneAPI::GetSSPhrase(const char* phrase_name)
{
	const char* script_name = "scripts/ssphrases.txt";
	TxtFile script(DataMgr::Load(script_name));
	unsigned int i;

	DataMgr::Release(script_name);
	if(script.FindInCol(phrase_name,&i,0))
	{
		if(script.SizeX(i)>1)
		{
			return std::string(script(i,1));
		}
	}

	return std::string("<no phrase>");
}
//	включить/выключить крыши
void ScriptSceneAPI::SetRoofMode(bool visible)
{
	IWorld::Get()->GetLevel()->RoofVisible = visible;
}
//	проиграть мультик
void ScriptSceneAPI::PlayBink(const char* file_name)
{
	Interface::Instance()->PlayBink(file_name);
}
//	проверить, есть ли человек на уровне
bool ScriptSceneAPI::IsHumanExist(const char* name)
{
	EntityPool::iterator i = EntityPool::GetInst()->begin(ET_HUMAN);
	
	//	ищем такого человека на уровне
	while(i != EntityPool::GetInst()->end())
	{
		if(i->GetInfo()->GetRID() == name) break;
		++i;
	}

	return !(i == EntityPool::GetInst()->end());
}
//	проверить активирование определенного квеста
bool ScriptSceneAPI::IsQuestExecute(const char* name)
{
	return (QS_STARTED == QuestEngine::GetQuestState(name));
}
//	проверить успешное завершение определенного квеста
bool ScriptSceneAPI::IsQuestOk(const char* name)
{
	return (QS_COMPLETE == QuestEngine::GetQuestState(name)) ||
		   (QS_PASSED_OK == QuestEngine::GetQuestState(name));
}

//**********************************************************************
//	class ScriptSceneAPI::Vehicle
ScriptSceneAPI::Vehicle::Vehicle(const char* name) : Object(name),m_Entity(0),m_Activity(0)
{
	EntityPool::iterator i = EntityPool::GetInst()->begin(ET_VEHICLE);
	
	//	ищем такого человека на уровне
	while(i != EntityPool::GetInst()->end())
	{
		if(i->GetInfo()->GetRID() == name)
		{
			break;
		}
		++i;
	}
	if(i == EntityPool::GetInst()->end())
		throw CASUS(std::string(std::string("На уровне нет техники <")+std::string(name)+">!"));
	//	инициализируем существо
	m_Entity = &(*i);
	//	запрещаем изменения свойств техники
	LockTraits(true);
	//	показываем персонаж
	Show(true);
}

ScriptSceneAPI::Vehicle::~Vehicle()
{
	//	разрешаем изменения свойств техники
	LockTraits(false);
	//	уничтожаем активность
	if(m_Activity)
	{
		m_Activity->Run(AC_STOP);
		delete m_Activity;
	}
}
//	существует ли такой объект
bool ScriptSceneAPI::Vehicle::IsExist(const char* name)
{
	for(EntityPool::iterator i=EntityPool::GetInst()->begin(ET_VEHICLE);i!=EntityPool::GetInst()->end();++i)
	{
		if(i->GetInfo()->GetRID() == name) return true;
	}

	return false;
}
//	объект думает (false - объект перестал думать)
bool ScriptSceneAPI::Vehicle::OnThink(void)
{
	// если человек уже давно не активен
	if(!m_Activity) return false;
	// здесь активность еще существует, проверим не закочилось ли действие
	if(m_Activity->Run(AC_TICK))
	{// действие не закончилось - продолжаем работу
		return true;
	}
	// действие только что завершилось - удалим его
	delete m_Activity;
	m_Activity = 0;
	// закончим работу
	return false;
}
//	показать/спрятать персонаж
void ScriptSceneAPI::Vehicle::Show(bool show)
{
	m_Entity->GetGraph()->Visible(show);
}
//	включить/выключить изменение свойств техники
void ScriptSceneAPI::Vehicle::LockTraits(bool lock)
{
	lock?m_Entity->RaiseFlags(EA_LOCK_TRAITS):m_Entity->DropFlags(EA_LOCK_TRAITS);
}
// получить трехмерные координаты техники
point3 ScriptSceneAPI::Vehicle::GetCoords3(void)
{
	point3  p = m_Entity->GetGraph()->GetPos3();
	ipnt2_t hex = m_Entity->GetGraph()->GetPos2();
	p.z = HexGrid::GetInst()->Get(hex).z + 1.0f;
	return p;
}
// получить двумерные (хексовые) координаты техники
ipnt2_t ScriptSceneAPI::Vehicle::GetCoords2(void)
{
	return m_Entity->GetGraph()->GetPos2();
}
// мгновенно переместить технику в нужную позицию
void ScriptSceneAPI::Vehicle::SetCoords(const ipnt2_t& pt,const float angle)
{
	EntityBuilder eb;

	eb.UnlinkEntity(m_Entity);
	eb.LinkEntity(m_Entity,pt,angle);
}
//	ехать в точку
void ScriptSceneAPI::Vehicle::MoveTo(const ipnt2_t& pnt)
{
	// если не прикреплен к человеку
	if(m_Activity) throw CASUS("ScriptSceneAPI::Vehicle::go vehicle already active!");
	// просчитаем путь
	pnt_vec_t path;
	if(HexGrid::GetInst()->IsOutOfRange(pnt))
	{
		throw CASUS("Мы вылетели за рамки хексового поля!");
	}
	PathUtils::GetInst()->CalcPassField(m_Entity);
	PathUtils::GetInst()->CalcPath(m_Entity, pnt, path);
	//	создаем активность
	m_Activity = ActivityFactory::GetInst()->CreateMove(m_Entity, path);
}
//	остановить технику
void ScriptSceneAPI::Vehicle::Stop(void)
{
	if(m_Activity) m_Activity->Run(AC_STOP);
}
//	сказать фразу
void ScriptSceneAPI::Vehicle::Talk(const std::string& phrase)
{
	Forms::GetInst()->ShowTalkDialog(m_Entity,phrase);
}
//	взорвать технику
void ScriptSceneAPI::Vehicle::ToBlast(void)
{
	m_Entity->GetEntityContext()->PlayDeath(0);
}

//**********************************************************************
//	class ScriptSceneAPI::Human
std::map<std::string,int> ScriptSceneAPI::Human::m_Registration;
ScriptSceneAPI::Human::Human(const char* name,CREATE_MODE cm) : Object(InitEntity(name,cm).c_str()),
																m_Activity(0),m_SystemName(name)
{
	//	запрещаем изменения свойств персонажа
	LockTraits(true);
	//	запомним режим передвижения
	m_RunMode = m_Entity->GetGraph()->Cast2Human()->IsRunMove();
	m_SitMode = m_Entity->GetGraph()->Cast2Human()->IsSitPose();
	//	ставим персонаж стоймя
	m_Entity->GetGraph()->Cast2Human()->ChangePose(GraphHuman::PT_STAND);
	//	показываем персонаж
	Show(true);
}
//	инициализировать сущность
std::string ScriptSceneAPI::Human::InitEntity(const char* system_name,CREATE_MODE cm)
{
	char buff[50];
	EntityBuilder eb;
	EntityPool::iterator i = EntityPool::GetInst()->begin(ET_HUMAN);
	
	//	ищем такого человека на уровне
	while(i != EntityPool::GetInst()->end())
	{
		if(i->GetInfo()->GetRID() == system_name) break;
		++i;
	}
	if(i == EntityPool::GetInst()->end())
	{//	нет такого существа на уровне
		switch(cm)
		{
		case CM_ALREADY_EXIST:
			throw CASUS(std::string(std::string("На уровне нет человека <")+std::string(system_name)+">!"));
		case CM_NEW:
			if(!(m_Entity = eb.CreateHuman(system_name)))
				throw CASUS(std::string(std::string("Нельзя создать человека <")+std::string(system_name)+">!"));
			break;
		}
	}
	else
	{//	есть такое существо на уровне
		switch(cm)
		{
		case CM_ALREADY_EXIST:
			m_Entity = &(*i);
			break;
		case CM_NEW:
			if(!(m_Entity = eb.CreateHuman(system_name)))
				throw CASUS(std::string(std::string("Нельзя создать человека <")+std::string(system_name)+">!"));
			break;
		}
	}
	//	генерим имя
	sprintf(buff,"%s%d",system_name,m_Registration[system_name]);
	//	увеличиваем счетчик
	m_Registration[system_name]++;

	return std::string(buff);
}

ScriptSceneAPI::Human::~Human()
{
	if(m_Entity)
	{
		//	восстанавливаем режим перемещения
		m_RunMode?m_Entity->GetGraph()->Cast2Human()->ChangeMoveType(GraphHuman::MT_RUN):m_Entity->GetGraph()->Cast2Human()->ChangeMoveType(GraphHuman::MT_WALK);
		m_SitMode?m_Entity->GetGraph()->Cast2Human()->ChangePose(GraphHuman::PT_SIT):m_Entity->GetGraph()->Cast2Human()->ChangePose(GraphHuman::PT_STAND);
		//	разрешаем изменения свойств персонажа
		LockTraits(false);
	}
	//	вычеркиваем из списков
	m_Registration[m_SystemName]--;
	//	уничтожаем активность
	if(m_Activity)
	{
		m_Activity->Run(AC_STOP);
		delete m_Activity;
	}
}
//	объект думает (false - объект перестал думать)
bool ScriptSceneAPI::Human::OnThink(void)
{
	// если человек уже давно не активен
	if(!m_Activity) return false;
	// здесь активность еще существует, проверим не закочилось ли действие
	if(m_Activity->Run(AC_TICK))
	{// действие не закончилось - продолжаем работу
		return true;
	}
	// действие только что завершилось - удалим его
	delete m_Activity;
	m_Activity = 0;
	// закончим работу
	return false;
}
//	остановить персонажа
void ScriptSceneAPI::Human::Stop(void)
{
	if(m_Activity) m_Activity->Run(AC_STOP);
}
//	посадить/поставить персонажа
void ScriptSceneAPI::Human::SetSitMode(bool sit)
{
	m_SitMode = sit;
	m_SitMode?m_Entity->GetGraph()->Cast2Human()->ChangePose(GraphHuman::PT_SIT):m_Entity->GetGraph()->Cast2Human()->ChangePose(GraphHuman::PT_STAND);
}
// мгновенно переместить человека в нужную позицию
void ScriptSceneAPI::Human::SetCoords(const ipnt2_t& pt,const float angle)
{
	EntityBuilder eb;

	eb.UnlinkEntity(m_Entity);
	eb.LinkEntity(m_Entity,pt,angle);
}
// идти в точку
void ScriptSceneAPI::Human::WalkTo(const ipnt2_t& pnt)
{
	// если не прикреплен к человеку
	if(m_Activity) throw CASUS("ScriptSceneAPI::Human::go human already active!");
	// просчитаем путь
	pnt_vec_t path;
	if(HexGrid::GetInst()->IsOutOfRange(pnt))
	{
		throw CASUS("Мы вылетели за рамки хексового поля!");
	}
	PathUtils::GetInst()->CalcPassField(m_Entity);
	PathUtils::GetInst()->CalcPath(m_Entity, pnt, path);
	//	измяняем тип ходьбы
	m_Entity->GetGraph()->Cast2Human()->ChangeMoveType(GraphHuman::MT_WALK);
	//	создаем активность
	m_Activity = ActivityFactory::GetInst()->CreateMove(m_Entity,path,ActivityFactory::CT_USUAL_PASS);
}

void ScriptSceneAPI::Human::WalkTo(ScriptSceneAPI::Object* object)
{
	pnt_vec_t hexes;

	DirtyLinks::CalcObjectHexes(object->Name(),&hexes);
	PathUtils::GetInst()->CalcPassField(m_Entity/*,&front*/);
	WalkTo(PathUtils::GetInst()->GetNearPnt(hexes).m_pnt);
}

void ScriptSceneAPI::Human::WalkTo(ScriptSceneAPI::Human* human)
{
	PathUtils::GetInst()->CalcPassField(m_Entity);
	WalkTo(PathUtils::GetInst()->GetNearPnt(human->Entity()).m_pnt);
}

// бежать в точку
void ScriptSceneAPI::Human::RunTo(const ipnt2_t& pnt)
{
	// если не прикреплен к человеку
	if(m_Activity) throw CASUS("ScriptSceneAPI::Human::go human already active!");
	// просчитаем путь
	pnt_vec_t path;
	if(HexGrid::GetInst()->IsOutOfRange(pnt))
	{
		throw CASUS("Мы вылетели за рамки хексового поля!");
	}
	PathUtils::GetInst()->CalcPassField(m_Entity);
	PathUtils::GetInst()->CalcPath(m_Entity, pnt, path);
	//	измяняем тип ходьбы
	m_Entity->GetGraph()->Cast2Human()->ChangeMoveType(GraphHuman::MT_RUN);
	//	создаем активность
	m_Activity = ActivityFactory::GetInst()->CreateMove(m_Entity,path,ActivityFactory::CT_USUAL_PASS);
}

void ScriptSceneAPI::Human::RunTo(ScriptSceneAPI::Object* object)
{
	pnt_vec_t hexes;

	DirtyLinks::CalcObjectHexes(object->Name(),&hexes);
	PathUtils::GetInst()->CalcPassField(m_Entity);
	RunTo(PathUtils::GetInst()->GetNearPnt(hexes).m_pnt);
}

void ScriptSceneAPI::Human::RunTo(ScriptSceneAPI::Human* human)
{
	PathUtils::GetInst()->CalcPassField(m_Entity);
	RunTo(PathUtils::GetInst()->GetNearPnt(human->Entity()).m_pnt);
}
//	установить доступность точки
bool ScriptSceneAPI::Human::IsAccess(ScriptSceneAPI::Object* object)
{
	pnt_vec_t hexes;

	DirtyLinks::CalcObjectHexes(object->Name(),&hexes);
	PathUtils::GetInst()->CalcPassField(m_Entity);
	return !PathUtils::GetInst()->GetNearPnt(hexes).IsDefPnt();
}

bool ScriptSceneAPI::Human::IsAccess(ScriptSceneAPI::Human* human)
{
	PathUtils::GetInst()->CalcPassField(m_Entity);
	return !PathUtils::GetInst()->GetNearPnt(human->Entity()).IsDefPnt();
}
//	установить свободна ли эта точка от каких-либо объектов
bool ScriptSceneAPI::Human::IsBusy(const ipnt2_t& pnt)
{
	return HexGrid::GetInst()->IsOutOfRange(pnt) ||
		   !PathUtils::GetInst()->IsEmptyPlace(m_Entity,pnt);
}
// заставить человека повернуться
void ScriptSceneAPI::Human::RotateToPoint(const point3& pnt)
{
	// если не прикреплен к человеку
	if(m_Activity)
		throw CASUS("ScriptSceneAPI::Human::rotateToPoint human already active!"); 
	// высчитаем угол, на который нужно повернуться, чтобы смотреть
	// в точку pnt и повернем на этот угол
	m_Activity = ActivityFactory::GetInst()->CreateRotate(m_Entity,
		Dir2Angle(pnt - GetCoords3()));
}
//	сказать фразу
void ScriptSceneAPI::Human::Talk(const std::string& phrase)
{
/*	static const std::string PREFIX		=	"$(";
	static const std::string POSTFIX	=	")";
	static const std::string SNDDIR		=	"sounds/";
	static const std::string SNDEXT		=	".wav";
	std::string::size_type start,finish;
	std::string cut_phrase = phrase;
	std::string sound_phrase;

	start = cut_phrase.find(PREFIX);
	if(start != cut_phrase.npos)
	{
		finish = cut_phrase.find(POSTFIX,start);
		if(finish == cut_phrase.npos) finish = cut_phrase.size();
		sound_phrase = SNDDIR+std::string(cut_phrase,start+PREFIX.size(),finish-start-PREFIX.size())+SNDEXT;
		cut_phrase = std::string(cut_phrase,0,start);
	}
*/
	MouseCursor::SetVisible(true);
	Forms::GetInst()->ShowTalkDialog(m_Entity,phrase_s(phrase));
	MouseCursor::SetVisible(false);
}
//	показать/спрятать персонаж
void ScriptSceneAPI::Human::Show(bool show)
{
	m_Entity->GetGraph()->Visible(show);
}
//	определить модель поведения
void ScriptSceneAPI::Human::SetBehaviourModel(const char* label)
{
	EntityBuilder eb;

	eb.SetAIModel(m_Entity,label);
    eb.SendSpawnEvent(m_Entity);
}
//	дать человеку предмет
//enum PACK_TYPE {PK_HEAD,PK_BODY,PK_HANDS,PK_LKNEE,PK_RKNEE,PK_IMPLANTS,PK_BACKPACK};
bool ScriptSceneAPI::Human::GiveWeapon(const char* weapon_name,const char* ammo_name,int ammo_count,PACK_TYPE pt)
{
	EntityBuilder eb;
	human_pack_type hpt = static_cast<human_pack_type>(PackTypeToHumanPackType(pt));
	BaseThing* thing = ThingFactory::GetInst()->CreateWeapon(weapon_name,ammo_name,ammo_count);

	if(!eb.CanTake(m_Entity,thing,hpt))
		return false;
	eb.GiveThing(m_Entity,thing,hpt);  

	return true;
}

bool ScriptSceneAPI::Human::GiveArmor(const char* armor_name,PACK_TYPE pt)
{
	EntityBuilder eb;
	human_pack_type hpt = static_cast<human_pack_type>(PackTypeToHumanPackType(pt));
	BaseThing* thing = ThingFactory::GetInst()->CreateArmor(armor_name);

	if(!eb.CanTake(m_Entity,thing,hpt))
		return false;
	eb.GiveThing(m_Entity,thing,hpt);  

	return true;
}

bool ScriptSceneAPI::Human::GiveAmmo(const char* ammo_name,int ammo_count,PACK_TYPE pt)
{
	EntityBuilder eb;
	human_pack_type hpt = static_cast<human_pack_type>(PackTypeToHumanPackType(pt));
	BaseThing* thing = ThingFactory::GetInst()->CreateAmmo(ammo_name,ammo_count);

	if(!eb.CanTake(m_Entity,thing,hpt))
		return false;
	eb.GiveThing(m_Entity,thing,hpt);  

	return true;
}

bool ScriptSceneAPI::Human::GiveGrenade(const char* grenade_name,PACK_TYPE pt)
{
	EntityBuilder eb;
	human_pack_type hpt = static_cast<human_pack_type>(PackTypeToHumanPackType(pt));
	BaseThing* thing = ThingFactory::GetInst()->CreateGrenade(grenade_name);

	if(!eb.CanTake(m_Entity,thing,hpt))
		return false;
	eb.GiveThing(m_Entity,thing,hpt);  

	return true;
}

bool ScriptSceneAPI::Human::GiveImplant(const char* implant_name,PACK_TYPE pt)
{
	EntityBuilder eb;
	human_pack_type hpt = static_cast<human_pack_type>(PackTypeToHumanPackType(pt));
	BaseThing* thing = ThingFactory::GetInst()->CreateImplant(implant_name);

	if(!eb.CanTake(m_Entity,thing,hpt))
		return false;
	eb.GiveThing(m_Entity,thing,hpt);  

	return true;
}

unsigned int ScriptSceneAPI::Human::PackTypeToHumanPackType(PACK_TYPE pt)
{
	human_pack_type hpt = HPK_NONE;

	switch(pt)
	{
	case PK_HEAD:		hpt = HPK_HEAD;		break;
	case PK_BODY:		hpt = HPK_BODY;		break;
	case PK_HANDS:		hpt = HPK_HANDS;	break;
	case PK_LKNEE:		hpt = HPK_LKNEE;	break;
	case PK_RKNEE:		hpt = HPK_RKNEE;	break;
	case PK_IMPLANTS:	hpt = HPK_IMPLANTS;	break;
	case PK_BACKPACK:	hpt = HPK_BACKPACK;	break;
	}

	return hpt;
}
//	установить команду для человека
//	enum PLAYER_TYPE {PT_PLAYER,PT_ENEMY};
void ScriptSceneAPI::Human::SetTeam(TEAM_TYPE tt)
{
	player_type pt = PT_NONE;
	EntityBuilder eb;

	switch(tt)
	{
	case TT_PLAYER:	pt = PT_PLAYER;	break;
	case TT_ENEMY:	pt = PT_ENEMY;	break;
	}

	eb.SetPlayer(m_Entity,pt);
}
//	поменять команду для человека
void ScriptSceneAPI::Human::ChangeTeam(TEAM_TYPE tt)
{
	player_type pt = PT_NONE;

	switch(tt)
	{
	case TT_PLAYER:	pt = PT_PLAYER;	break;
	case TT_ENEMY:	pt = PT_ENEMY;	break;
	}

	AIUtils::ChangeEntityPlayer(m_Entity,pt);
}

// выстрелить в точку
void ScriptSceneAPI::Human::Shoot(const point3& target, const float accuracy, const SHOT_TYPE st)
{
	shot_type st4logic;

	if(m_Activity)
		throw CASUS("ScriptSceneAPI::Human::shoot human already active!"); 
	switch(st)
	{
	case ST_AIMSHOT : st4logic = SHT_AIMSHOT;  break;
	case ST_SNAPSHOT: st4logic = SHT_SNAPSHOT; break;
	case ST_AUTOSHOT: st4logic = SHT_AUTOSHOT; break;

	}
	m_Entity->Cast2Human()->GetEntityContext()->SetShotType(st4logic);
	m_Activity = ActivityFactory::GetInst()->CreateShoot(m_Entity,target,accuracy);
}
//	включить/выключить изменение свойств персонажа
void ScriptSceneAPI::Human::LockTraits(bool lock)
{
	lock?m_Entity->RaiseFlags(EA_LOCK_TRAITS):m_Entity->DropFlags(EA_LOCK_TRAITS);
}
// получить трехмерные координаты человека
point3 ScriptSceneAPI::Human::GetCoords3(void)
{
	point3  p = m_Entity->GetGraph()->GetPos3();
	ipnt2_t hex = m_Entity->GetGraph()->GetPos2();
	p.z = HexGrid::GetInst()->Get(hex).z + 1.0f;
	return p;
}
// получить двумерные (хексовые) координаты человека
ipnt2_t ScriptSceneAPI::Human::GetCoords2(void)
{
	return m_Entity->GetGraph()->GetPos2();
}
//	уничтожить существо
void ScriptSceneAPI::Human::Destroy(void)
{
	EntityBuilder eb;

	eb.UnlinkEntity(m_Entity);
    DeadList::GetHeroesList()->Insert(m_Entity);
	m_Entity->GetGraph()->Destroy();
	EntityPool::GetInst()->Remove(m_Entity);
	m_Entity = 0;
}
//	сбросить все вещи на землю
void ScriptSceneAPI::Human::ItemsToGround(const ipnt2_t& pt)
{
	HumanEntity* human = m_Entity->Cast2Human();
	GraphHuman* graph = human->GetGraph();
	HumanContext* context = human->GetEntityContext();
	//очистим поле имлантов
	HumanContext::iterator itor = context->begin(HPK_IMPLANTS);
	while(itor != context->end())
	{
		BaseThing* thing = &*itor;
		context->RemoveThing(thing);
		delete thing;
		++itor;
	}
	
	//сбросить все items на землю
	itor = context->begin(HPK_ALL);
	while(itor != context->end())
	{
		
		BaseThing* thing = &*itor;
		
		context->RemoveThing(thing);
		Depot::GetInst()->Push(pt, thing);
		
		++itor;
	}
}

//**********************************************************************
//	class ScriptSceneAPI::Trader
ScriptSceneAPI::Trader::Trader(const char* name) : Object(name),m_Entity(0)
{
	EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_TRADER);
	
	//	ищем такого человека на уровне
	while(itor != EntityPool::GetInst()->end())
	{
		if(itor->GetInfo()->GetRID() == name)
		{
			break;
		}
		++itor;
	}
	if(itor == EntityPool::GetInst()->end())
		throw CASUS(std::string(std::string("На уровне нет торговца <")+std::string(name)+">!"));
	m_Entity = &(*itor);
	//	запрещаем изменения свойств персонажа
	LockTraits(true);
	//	показываем персонаж
	Show(true);
}

ScriptSceneAPI::Trader::~Trader()
{
	if(m_Entity)
	{
		//	разрешаем изменения свойств персонажа
		LockTraits(false);
	}
}
//	объект думает (false - объект перестал думать)
bool ScriptSceneAPI::Trader::OnThink(void)
{
	return false;
}
// мгновенно переместить человека в нужную позицию
void ScriptSceneAPI::Trader::SetCoords(const ipnt2_t& pt,const float angle)
{
	EntityBuilder eb;

	eb.UnlinkEntity(m_Entity);
	eb.LinkEntity(m_Entity,pt,angle);

//	m_Entity->GetGraph()->SetLoc(pt,angle);
}
//	показать/спрятать персонаж
void ScriptSceneAPI::Trader::Show(bool show)
{
	m_Entity->GetGraph()->Visible(show);
}
//	включить/выключить изменение свойств персонажа
void ScriptSceneAPI::Trader::LockTraits(bool lock)
{
	lock?m_Entity->RaiseFlags(EA_LOCK_TRAITS):m_Entity->DropFlags(EA_LOCK_TRAITS);
}
// получить трехмерные координаты человека
point3 ScriptSceneAPI::Trader::GetCoords3(void)
{
	point3  p = m_Entity->GetGraph()->GetPos3();
	ipnt2_t hex = m_Entity->GetGraph()->GetPos2();
	p.z = HexGrid::GetInst()->Get(hex).z + 1.0f;
	return p;
}
// получить двумерные (хексовые) координаты человека
ipnt2_t ScriptSceneAPI::Trader::GetCoords2(void)
{
	return m_Entity->GetGraph()->GetPos2();
}
//	уничтожить существо
void ScriptSceneAPI::Trader::Destroy(void)
{
	EntityBuilder eb;

	eb.UnlinkEntity(m_Entity);
    DeadList::GetTradersList()->Insert(m_Entity);
	m_Entity->GetGraph()->Destroy();
	EntityPool::GetInst()->Remove(m_Entity);
	m_Entity = 0;
}

//**********************************************************************
//	class ScriptSceneAPI::Camera
ScriptSceneAPI::Camera::Camera() : Object("camera")
{
}

ScriptSceneAPI::Camera::~Camera()
{
}
//	объект думает (false - объект перестал думать)
bool ScriptSceneAPI::Camera::OnThink(void)
{
	return IWorld::Get()->GetCamera()->IsLinked();
}
// запустить камеру по сплайну
void ScriptSceneAPI::Camera::MoveBySpline(const std::string& spline_name)
{
	IWorld::Get()->GetCamera()->LinkToSpline(IWorld::Get()->GetLevel()->CamPaths[spline_name.c_str()],ScriptSceneAPI::Instance()->GetTime());
}
// сфокусировать камеру на определенную точку
void ScriptSceneAPI::Camera::FocusOn(const point3& pt)
{
	IWorld::Get()->GetCamera()->FocusOn(pt);
}
// сфокусировать камеру на определенную точку
void ScriptSceneAPI::Camera::FocusOn(const point3& pt, const float time)
{
	IWorld::Get()->GetCamera()->FocusOn(pt,time);
}

//**********************************************************************
//	class ScriptSceneAPI::Timer
ScriptSceneAPI::Timer::Timer(const char* name) : Object(name),m_SecondsStart(0),m_SecondsLeft(0)
{
}

ScriptSceneAPI::Timer::~Timer()
{
}
//	объект думает (false - объект перестал думать)
bool ScriptSceneAPI::Timer::OnThink(void)
{
//	m_SecondsLeft = std::max(0.f,m_SecondsStart-(::Timer::GetSeconds()-m_Seconds));
	m_SecondsLeft = m_SecondsStart-(::Timer::GetSeconds()-m_Seconds);
	return (m_SecondsLeft>0)?true:false;
}
//	установить время в секундах	
void ScriptSceneAPI::Timer::SetTime(float seconds)
{
	m_SecondsStart = seconds;
	m_SecondsLeft = 0;
}
//	запустить таймер
void ScriptSceneAPI::Timer::Start(void)
{
	m_SecondsLeft = m_SecondsStart;
	m_Seconds = ::Timer::GetSeconds();
}

//**********************************************************************
//	class ScriptSceneAPI::Animation
const char* ScriptSceneAPI::Animation::m_AnimaPath = "animations/anims/scriptscenes/";
const char* ScriptSceneAPI::Animation::m_SkinPath = "animations/skins/scriptscenes/";
ScriptSceneAPI::Animation::Animation(const char* name) : Object(name),m_Animation(0),m_Skin(0),m_AnimaData(0)
{
	const std::string anima_name = std::string(m_AnimaPath)+std::string(name)+".skel";
	const std::string skin_name  = std::string(m_SkinPath)+std::string(name)+".skin";

	if(m_Animation = AnimaLibrary::GetInst()->GetSkAnimation(anima_name))
	{
		if(m_Skin = AnimaLibrary::GetInst()->GetSkSkin(skin_name))
		{
			m_Skin->ValidateLinks(m_Animation);
			m_AnimaData = new AnimaData();
		}
		else
		{
			throw CASUS(std::string(std::string("Не найдена кожа <")+skin_name+">."));
		}
	}
	else
	{
		throw CASUS(std::string(std::string("Не найдена анимация <")+anima_name+">."));
	}
}

ScriptSceneAPI::Animation::~Animation()
{
	if(m_Skin) delete m_Skin;
	if(m_AnimaData) delete m_AnimaData;
}
//	объект думает (false - объект перестал думать)
bool ScriptSceneAPI::Animation::OnThink(void)
{
	m_Skin->Update(&m_Animation->Get(m_AnimaData,::Timer::GetSeconds()));
	IWorld::Get()->GetPipe()->Chop(m_Skin->GetMesh());

	return true;
}

void ScriptSceneAPI::Animation::Start(void)
{
	(*m_AnimaData) = m_Animation->Start(::Timer::GetSeconds());
}

//**********************************************************************
//	class ScriptSceneAPI::Object
ScriptSceneAPI::Object::Object(const char* name) : ScriptScene::Object(name)
{
}

ScriptSceneAPI::Object::~Object()
{
}
//	объект думает (false - объект перестал думать)
bool ScriptSceneAPI::Object::OnThink(void)
{
	return false;
}
//	переключить сотояние объекта
float ScriptSceneAPI::Object::SwitchState(void)
{
	GameObjectsMgr::GetInst()->MarkAsUsed(Name());

	return DirtyLinks::SwitchObjState(Name());
}
//	узнать состояние объекта
bool ScriptSceneAPI::Object::GetState(void)
{
	return DirtyLinks::GetObjState(Name());
}
//	уничтожить объект
void ScriptSceneAPI::Object::ToErase(void)
{
	DirtyLinks::EraseLevelObject(Name());
}
//	взорвать объект
void ScriptSceneAPI::Object::ToBlast(float damage)
{
	ExplosionManager::Get()->BurnObject(Name(),damage);
}
//	получить координаты сентра объекта
point3 ScriptSceneAPI::Object::GetCoords3(void)
{
	return DirtyLinks::GetObjCenter(Name());
}

/////////////////////////////////////////////////////////////////////////
//////////////////////    class VideoScriptScene    //////////////////////
/////////////////////////////////////////////////////////////////////////
VideoScriptScene::VideoScriptScene(const char* video) : ScriptScene(),m_Video(video)
{
}

VideoScriptScene::~VideoScriptScene()
{
}
//	начало скриптовой сцены
bool VideoScriptScene::OnStart(void)
{
	GetApi()->PlayBink(m_Video.c_str());

	return false;
}
//	окончание скриптовой сцены
bool VideoScriptScene::OnFinish(void)
{
	return true;
}
//	прерывание скриптовой сцены
bool VideoScriptScene::OnSkip(void)
{
	return true;
}
//	окончание действия объекта скриптовой сцены
bool VideoScriptScene::OnObjectNoActive(Object* object)
{
	return false;
}

