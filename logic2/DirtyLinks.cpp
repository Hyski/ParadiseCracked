#pragma warning(disable:4786)

#include "logicdefs.h"

#include "../game.h"
#include "../Common/3Deffects/EffectManager.h"
#include "../Common/CmdLine/CmdLine.h"
#include "../Common/Shell/Shell.h"
#include "../common/graphpipe/graphpipe.h"
#include "../gamelevel/LevelToLogic.h"
#include "../options/options.h"
#include "../GameLevel/GameLevel.h"
#include "../Iworld.h"
#include "../Interface/interface.h"
#include "../GameLevel/ExplosionManager.h"
#include "../sound/ISound.h"

#include "form.h"
#include "thing.h"
#include "entity.h"
#include "bureau.h"
#include "damager.h"
#include "Graphic.h"
#include "aiutils.h"
#include "HexUtils.h"
#include "xlsreader.h"
#include "DirtyLinks.h"

#include <algorithm>

namespace{

    const char* str_res_xls_name = "scripts/logic/str_res.txt";

    int Entity2GID(BaseEntity* entity)
    {
        return entity ? entity->GetGraph()->GetGID() : -1;
    }

    BaseEntity* GID2Entity(size_t gid)
    {
        EntityPool::iterator itor = EntityPool::GetInst()->begin();
        
        while(itor != EntityPool::GetInst()->end()){

            if(itor->GetGraph()->GetGID() == gid)
                return &*itor;

            ++itor;
        }

        return 0;
    }

    DynObjectPool* GetObjPool()
    {
        return &IWorld::Get()->GetLevel()->LevelObjects;
    }

    /*
    class profiler{
    public:
        
        profiler() : m_overal_time(0), m_counter(0){}
        
        ~profiler()
        {
            theLog("enter times %i\n", m_counter);
            theLog("overal work time %f\n", m_overal_time);
            
            if(m_counter) theLog("average work time %f\n", m_overal_time/m_counter);
        }
        
        void enter()
        {
            Timer::Update();
            m_enter_time = Timer::GetSeconds();
        }
        
        void leave(const std::string& label)
        {
            m_counter++;
            
            Timer::Update();
            float delta = Timer::GetSeconds() - m_enter_time;
            
            theLog("%i: %s %f\n", m_counter, label.c_str(), delta);
            m_overal_time += delta;
        }
        
        struct enter_leave{
            
            enter_leave(profiler* prof, const std::string& label = std::string()) : 
        m_profiler(prof), m_label(label) { m_profiler->enter(); }
        
        ~enter_leave() { m_profiler->leave(m_label); }
        
        std::string m_label;
        profiler*   m_profiler;
        };
        
    private:
            
        int   m_counter;
            
        float m_enter_time;
        float m_overal_time;
    };
    */

    //
    // оптимизация получения ресурсной строки
    //
    class StringCache{
    public:

        StringCache() { m_last_used = m_table.end(); }
        ~StringCache(){}

        bool GetString(const rid_t& rid, std::string* str)
        {
            if(TblFind(rid, str))
                return true;

            if(XlsFind(rid, str)){
                TblPush(rid,*str);
                return true;
            }

            return false;
        }

    private:

        //поиск в xls
        bool XlsFind(const rid_t& rid, std::string* str)
        {
            TxtFilePtr txt(str_res_xls_name);
            
            //найдем нужную строку
            for(int line = 0; line < txt->SizeY(); line++){
                txt->GetCell(line, 0, *str);
                
                if(rid == *str){
                    txt->GetCell(line, 1, *str);
                    return true;
                }
            }

            return false;
        }

        //поиск в таблице
        bool TblFind(const rid_t& rid, std::string* str)
        {
            //если строку недавно читали
            if(m_last_used != m_table.end() && m_last_used->first == rid){
                *str = m_last_used->second.m_str;
                m_last_used->second.m_use = Timer::GetSeconds();
                return true;
            }

            //ищем ее в таблице
            m_last_used = m_table.find(rid);

            if(m_last_used == m_table.end())
                return false;

            m_last_used->second.m_use++;
            *str = m_last_used->second.m_str;
            return true;
        } 
        
        //вставка в таблицу
        void TblPush(const rid_t& rid, const std::string& str)
        {
            m_table[rid] = str_info(str);
            m_last_used  = m_table.find(rid);

            if(m_table.size() < MAX_TABLE_SIZE)
                return;

            str_map_t::iterator itor       = m_table.begin();
            str_map_t::iterator erase_itor = m_table.begin();

            while(itor != m_table.end()){

                if(itor->second.m_use < erase_itor->second.m_use)
                    erase_itor = itor;

                ++itor;
            } 
            
            m_table.erase(erase_itor);
        }

    private:

        enum { MAX_TABLE_SIZE = 64 };

        struct str_info{
            float       m_use;
            std::string m_str;

            str_info(){}

            str_info(const std::string& str) : m_use(Timer::GetSeconds()), m_str(str){}
        };

        typedef std::map<rid_t, str_info> str_map_t;

        str_map_t           m_table;
        str_map_t::iterator m_last_used;
    };
}

//===========================================================

const char* opt_names[DirtyLinks::MAX_OPTIONS] = 
{
    "game.mark_path",          // OT_SHOW_PATH
    "game.mark_wave_front",    // OT_SHOW_FRONT
    "game.mark_exit_zone",     // OT_SHOW_JOINTS
    "game.mark_lands",         // OT_SHOW_LANDS
    "game.show_fos",           // OT_SHOW_FOS,
    "game.show_banner",        // OT_SHOW_BANNER         
    "game.cursor_refreshtime", // OT_MOUSE_UPDATE_TIME
    "system.sound.themes",     // OT_ENABLE_THEMES
    "game.helper",             // OT_ENABLE_HELPER

    "game.hidden_movement.enemy_related.move",   //OT_HIDDEN_MOVEMENT_ENEMY_RELATED_MOVE,
    "game.hidden_movement.enemy_related.move" ,  //OT_HIDDEN_MOVEMENT_ENEMY_RELATED_SHOOT,
    "game.hidden_movement.friend_related.move",  //OT_HIDDEN_MOVEMENT_FRIEND_RELATED_MOVE,
    "game.hidden_movement.friend_related.shoot",  //OT_HIDDEN_MOVEMENT_FRIEND_RELATED_SHOOT,
    "game.hidden_movement.neutral_related.move",  //OT_HIDDEN_MOVEMENT_NEUTRAL_RELATED_MOVE,
    "game.hidden_movement.neutral_related.shoot", //OT_HIDDEN_MOVEMENT_NEUTRAL_RELATED_SHOOT,

    "game.hidden_movement.civilian_vehicles.move",         //OT_HIDDEN_MOVEMENT_CIVILIAN_VEHICLES_MOVE,
    "game.hidden_movement.civilian_vehicles.shoot",        //OT_HIDDEN_MOVEMENT_CIVILIAN_VEHICLES_SHOOT,
    "game.hidden_movement.civilian_humans_traders.move",   //OT_HIDDEN_MOVEMENT_CIVILIAN_HUMANS_TRADERS_MOVE,
    "game.hidden_movement.civilian_humans_traders.shoot" , //OT_HIDDEN_MOVEMENT_CIVILIAN_HUMANS_TRADERS_SHOOT,
};


bool DirtyLinks::IsGoodEntity(BaseEntity *ent)
	{
			for(EntityPool::iterator it=EntityPool::GetInst()->begin(); it!=EntityPool::GetInst()->end(); it++)
			{
				if(ent==&(*it))
					return true;
			}
			return false;
	}


GraphPipe* DirtyLinks::GetGraphPipe()
{
    return Shell::Instance()->GetGraphPipe();
}

void DirtyLinks::EnableRoofs(bool flag)
{
    IWorld::Get()->GetLevel()->RoofVisible = flag;
    GetGraphPipe()->GetCam()->Move(NULLVEC);    
}

GameLevel* DirtyLinks::GetGameLevel()
{
    return IWorld::Get()->GetLevel();
}

bool DirtyLinks::GetLevelPnt(point3* lev_pnt)
{
    point3  dir  = GetGraphPipe()->GetCam()->Pick(Input::MouseState().x ,Input::MouseState().y),
            from = GetGraphPipe()->GetCam()->GetPos(),
            norm;
    
    return GetGameLevel()->TraceRay(from, dir, lev_pnt,&norm);
}

void DirtyLinks::LoadLevel(const std::string& level)
{    
    if(level.empty())
		Game::UnloadGameLevel();
	else
		Interface::Instance()->LoadGameLevel(level.c_str());
}

const std::string& DirtyLinks::GetLevelSysName()
{
    return IWorld::Get()->GetLevel()->LevelName;
}

std::string DirtyLinks::GetLevelFullName(const std::string& sys_name)
{
    return IWorld::Get()->GetLevelName(sys_name);
}

float DirtyLinks::SwitchObjState(const std::string& obj_name)
{
	return GetObjPool()->SwitchState(obj_name,Timer::GetSeconds());
}

void DirtyLinks::DrawBBox(const BBox& box, unsigned color)
{
    Shell::Instance()->GetGraphPipe()->DrawBBox(box, color);
}

void DirtyLinks::DrawPrimi(const std::string& shader, const Primi& pr)
{
    Shell::Instance()->GetGraphPipe()->Chop(shader, &pr);
}

float DirtyLinks::GetLandHeight(const point3& pnt)
{
    return IWorld::Get()->GetLevel()->LevelGrid->Height(pnt);
}

void DirtyLinks::MoveCamera(const point3& pos, float time)
{
    IWorld::Get()->GetCamera()->FocusOn(pos, time);
}

bool DirtyLinks::IsInCameraCone(const point3& point)
{
    return IWorld::Get()->GetCamera()->Cone.TestPoint(point) == Frustum::VISIBLE;
}

const point3& DirtyLinks::GetCamPos()
{
    return IWorld::Get()->GetCamera()->GetPos();
}

point3 DirtyLinks::PickCam(float x, float y)
{
    return IWorld::Get()->GetCamera()->Pick(x, y);
}

int DirtyLinks::GetIntOpt(opt_type type)
{
    if(type == MAX_OPTIONS) return 0;
    return Options::GetInt(opt_names[type]);
}

float DirtyLinks::GetFloatOpt(opt_type type)
{
    if(type == MAX_OPTIONS) return 0;
    return Options::GetFloat(opt_names[type]);
}

extern bool ProcessCamera;

void DirtyLinks::EnableCameraControl(bool flag)
{
    ProcessCamera = flag;
}

void DirtyLinks::CalcObjectHexes(const rid_t& rid, pnt_vec_t* pnts)
{
    GetObjPool()->CollectHexesForObject(rid, pnts);
}

point3 DirtyLinks::GetObjCenter(const rid_t& rid)
{
    DynObjectPool* obj_pool = &IWorld::Get()->GetLevel()->LevelObjects;
    DynObjectPool::iterator itor = obj_pool->begin(rid); 
    
    int count = 0;
    point3 center=NULLVEC;

    while(itor != obj_pool->end()){

        center.x += itor->Bound.GetCenter().x;
        center.y += itor->Bound.GetCenter().y;
        center.z += itor->Bound.GetCenter().z;

        ++count;
        ++itor;
    }

    //вычислим среднее арифметическое
    center.x /= count;
    center.y /= count;
    center.z /= count;

    return center;        
}

void DirtyLinks::EnumerateAllLevelCases(BoxSpawner* spawner)
{
    DynObjectPool::iterator itor = GetObjPool()->begin("");
    
    for(; itor != GetObjPool()->end(); ++itor){
        if(itor->CaseInfo.IsDatabase())
            spawner->CreateDataBox(itor->CaseInfo.GetName());
        else if(itor->CaseInfo.IsStorage())
            spawner->CreateStoreBox(itor->CaseInfo.GetName());    
    }
}

bool DirtyLinks::IsElevatorObj(const rid_t& obj_rid)
{
    DynObjectPool::iterator itor = GetObjPool()->begin(obj_rid);

    for(; itor != GetObjPool()->end(); ++itor){
        if(itor->CaseInfo.IsElevator())
            return true;
    }    

    return false;
}

void DirtyLinks::EraseLevelObject(const rid_t& rid)
{
    GetObjPool()->EraseByName(rid);
}

bool DirtyLinks::CanUseObject(const rid_t& obj_rid, std::string* hint)
{
    DynObjectPool* obj_pool = &IWorld::Get()->GetLevel()->LevelObjects;
    DynObjectPool::iterator itor = obj_pool->begin(obj_rid);

    if(itor == obj_pool->end() || !itor->GetUseOptions().Usable)
        return false;

    *hint = itor->GetUseOptions().UseHint;
    return true;
}

bool DirtyLinks::GetObjState(const rid_t& rid)
{
    DynObjectPool* obj_pool = &IWorld::Get()->GetLevel()->LevelObjects;
    DynObjectPool::iterator itor = obj_pool->begin(rid);

    if(itor != obj_pool->end()) return itor->GetState();
    
    return false;
}

rid_t DirtyLinks::GetKey4UseObj(const rid_t& obj_rid)
{
    DynObjectPool* obj_pool = &IWorld::Get()->GetLevel()->LevelObjects;
    DynObjectPool::iterator itor = obj_pool->begin(obj_rid);

    if(itor != obj_pool->end()) return itor->GetUseOptions().UnlockKey;    

    return rid_t();
}

int DirtyLinks::GetWisdom4UseObj(const rid_t& obj_rid)
{
    DynObjectPool* obj_pool = &IWorld::Get()->GetLevel()->LevelObjects;
    DynObjectPool::iterator itor = obj_pool->begin(obj_rid);

    if(itor != obj_pool->end()) return itor->GetUseOptions().UnlockWisdom;    

    return -1;
}

void DirtyLinks::SetObjState(const rid_t& rid, bool state)
{
    DynObjectPool* obj_pool = &IWorld::Get()->GetLevel()->LevelObjects;
    DynObjectPool::iterator itor = obj_pool->begin(rid);

    while(itor != obj_pool->end()){
        itor->ChangeState(state, 0);

			if(state)
				{
				bool Locked=false; //Объекты однажды открытые не должны закрываться.
				if(Grid *LevelGrid=(Grid*)HexGrid::GetInst())
					{
					if(LevelGrid->ManDirect[itor->Name].size()) Locked=true;
					if(LevelGrid->ManInvert[itor->Name].size()) Locked=true;
					if(LevelGrid->HeightDirect[itor->Name].size()) Locked=false;
					}
				if(Locked) itor->UseOptions.Usable=false;
				}
        ++itor;
    }
}

std::string DirtyLinks::GetStrRes(const rid_t& rid)
{    
    static StringCache cache;

    std::string str;

    if(cache.GetString(rid, &str))
        return str;
        
#ifdef _HOME_VERSION
    return "string <" + rid + "> not found";
#else
    return "";
#endif
}

bool DirtyLinks::GetCmdLineArg(const std::string& arg_name, std::string* value)
{
    return CmdLine::IsKey(arg_name.c_str(), *value);
}

void DirtyLinks::EnableLevelRender(bool flag)
{
    Game::SetGameLevelRender(flag);
}

void DirtyLinks::EnableLevelAndEffectSound(bool flag)
{
    ISound::instance()->muteChannel(ISound::cEffects, !flag);
    ISound::instance()->muteChannel(ISound::cAmbient, !flag);
    ISound::instance()->muteChannel(ISound::cSpeech, !flag);
}

//================================================

namespace {

    class SimpleBusRoute : public BusRoute{
    public:
        
        SimpleBusRoute(const rid_t& rid = rid_t()) : m_rid(rid) {}    
        ~SimpleBusRoute(){}
        
        DCTOR_DEF(SimpleBusRoute)
            
            void MakeSaveLoad(SavSlot& st)
        {
            if(st.IsSaving())
                st << m_rid;
            else
                st >> m_rid;
        }
        
        point_type GetPntType(const ipnt2_t& cur_pos)
        {
            BusPath* route = GetBusPath();
            
            //это конечная точка?
            for(size_t k = 0; k < route->Segments.size(); k++){
                if(cur_pos == route->Segments[k].back()){
                    
                    //если последний отрезок пути
                    if(k == route->Segments.size() - 1)
                        return PT_LAST;
                    
                    //если предпосл. отрезок пути
                    if(k == route->Segments.size() - 2)
                        return PT_DEST;
                    
                    return PT_STOP;
                }
            }
            
            return PT_PATH;
        }
        
        void GenLoc(bool first, pos_s* pos)
        {
            int seg_num = 0, pnt_num = 0;
            
            BusPath* route = GetBusPath();
            
            if(!first){
                
                seg_num = NormRand()*(route->Segments.size() - 1); 
                if(seg_num < 0) seg_num = 0;

                pnt_num = NormRand()*(route->Segments[seg_num].size() - 2);
                if(pnt_num < 0) pnt_num = 0;
            }
            
            pos->m_pos   = route->Segments[seg_num][pnt_num];
            pos->m_angle = Dir2Angle(HexGrid::GetInst()->Get(route->Segments[seg_num][pnt_num+1]) - HexGrid::GetInst()->Get(pos->m_pos));
        }
        
        void GetPath(const ipnt2_t& cur_pos, pnt_vec_t* path)
        {
            bool frecord = false;
            
            BusPath* route = GetBusPath();
            
            for(size_t k = 0; k < route->Segments.size(); k++){
                
                //если это конец пути
                if(cur_pos == route->Segments[k].back()){
                    *path = route->Segments[k+1];
                    std::reverse(path->begin(), path->end());
                    return;
                }
                
                //ищем точку
                for(size_t j = 0; j < route->Segments[k].size(); j++){
                    if(!frecord)
                        frecord = cur_pos == route->Segments[k][j];
                    else
                        path->push_back(route->Segments[k][j]);                
                }
                
                if(frecord){
                    std::reverse(path->begin(), path->end());
                    return;
                }
            }
            
            throw CASUS("SimpleBusRoute: автобус сбился с пути!!!");
        }
        
    private:
        
        BusPath* GetBusPath()
        {
            GameLevel::BusPaths::iterator itor = IWorld::Get()->GetLevel()->BPaths.find(m_rid);
            if(itor == IWorld::Get()->GetLevel()->BPaths.end())
                throw CASUS("CreateBusRoute: не такого маршрута!");
            
            return &itor->second;
        }
        
    private:
        
        rid_t    m_rid;
    };

    DCTOR_IMP(SimpleBusRoute)
}

//================================================

BusRoute* DirtyLinks::CreateBusRoute(const std::string& sys_name)
{
    return new SimpleBusRoute(sys_name);
}

//================================================

namespace{

    //
    // интерейс на точки до кот нужно дойти
    //
    class SimpleCheckPoints : public CheckPoints{
    public:
        
        SimpleCheckPoints(const rid_t rid = rid_t()) : m_rid(rid), m_cur(0){}
        
        ~SimpleCheckPoints(){}
        
        DCTOR_DEF(SimpleCheckPoints)
            
            void MakeSaveLoad(SavSlot& st)
        {
            if(st.IsSaving()){
                st << m_cur;
                st << m_rid;
            } else {
                st >> m_cur; 
                st >> m_rid;
            }
        }
        
        bool IsEnd() const { return !(m_cur < GetPoints()->Points.size()); }
        
        void GetPos(pos_s* pos) const
        {
            DestPoints* points = GetPoints();
            
            if(m_cur < points->Points.size()){
                pos->m_pos   = HexUtils::scr2hex(points->Points[m_cur].Pos);
                pos->m_angle = Dir2Angle(points->Points[m_cur].Dir);
                pos->m_fspecial = points->Points[m_cur].Special != 0;
            }else{
                pos->m_pos   = HexUtils::scr2hex(points->Points.back().Pos);
                pos->m_angle = Dir2Angle(points->Points.back().Dir);
                pos->m_fspecial = points->Points.back().Special != 0;
            }
        }
        
        void First(first_type type) 
        { 
            if(type == FT_USUAL){
                m_cur = 0;
                return;
            }
            
            if(type == FT_SPAWN){
                int rand_val = 10*NormRand();
                while(rand_val--) Next();
                return;
            }
        }
        
        void Next()
        { 
            DestPoints* points = GetPoints();
            
            if(++m_cur > points->Points.size()) 
                m_cur = points->Points.size(); 
        }
        
    private:
        
        DestPoints* GetPoints() const
        {
            GameLevel::PathPoints::iterator itor = IWorld::Get()->GetLevel()->PPoints.find(m_rid);
            if(itor == IWorld::Get()->GetLevel()->PPoints.end()) throw CASUS("CreateCheckPoints: нет такого маршрута!");
            return &itor->second;
        }
        
    private:
        
        rid_t  m_rid;
        size_t m_cur;
    };
    
    DCTOR_IMP(SimpleCheckPoints)
        
    //
    // генерирует случайные CheckPoints
    //
    class CircleCheckPoints : public CheckPoints{
    public:
        
          CircleCheckPoints(const ipnt2_t& center = ipnt2_t(), int radius = 0) :
            m_radius(radius), m_center(center), m_angle(NormRand()*PIm2){}
          
          CircleCheckPoints(BaseEntity* entity, int radius = 0) :
            m_radius(radius),
            m_center(entity->GetGraph()->GetPos2()),
            m_angle(entity->GetGraph()->GetAngle()){}

          ~CircleCheckPoints() {}
          
          DCTOR_DEF(CircleCheckPoints)
              
          void MakeSaveLoad(SavSlot& st)
          {
              if(st.IsSaving()){
                  
                  st << m_rnd_pnt.x << m_rnd_pnt.y;
                  st << m_center.x  << m_center.y;

                  st << m_angle;
                  st << m_rnd_ang;
                  
                  st << m_radius;
              }else{
                  
                  st >> m_rnd_pnt.x >> m_rnd_pnt.y; 
                  st >> m_center.x  >> m_center.y;
                  
                  st >> m_angle;
                  st >> m_rnd_ang;

                  st >> m_radius;
              }
          }
          
          void GetPos(pos_s* pos) const
          {
              pos->m_pos   = m_rnd_pnt;
              pos->m_angle = m_rnd_ang; 
          }
          
          void Next()
          {
              while(true){
                  
                  int dx = NormRand() * m_radius,
                      dy = NormRand() * m_radius;
                  
                  m_rnd_pnt.x = m_center.x + (NormRand() > 0.5f ? -dx : dx);
                  m_rnd_pnt.y = m_center.y + (NormRand() > 0.5f ? -dy : dy);
                  
                  if(!HexGrid::GetInst()->IsOutOfRange(m_rnd_pnt)){
                      m_rnd_ang = NormRand()*PIm2;
                      return;
                  }
              }        
          }
          
          void First(first_type type)
          {
              if(type == FT_USUAL){
                  Next();
                  return;
              }
              
              if(type == FT_SPAWN){
                  m_rnd_pnt = m_center;
                  m_rnd_ang = m_angle;
                  return;
              }
          }
          
          bool IsEnd() const { return false; }
          
    private:
        
        float   m_rnd_ang; 
        ipnt2_t m_rnd_pnt;

        ipnt2_t m_center;
        float   m_angle;

        int     m_radius;
    };
    
    DCTOR_IMP(CircleCheckPoints)
}

//================================================

CheckPoints* DirtyLinks::CreateCheckPoints(const std::string& sys_name)
{
    return new SimpleCheckPoints(sys_name);
}

CheckPoints* DirtyLinks::CreateCirclePoints(const ipnt2_t& center, int radius)
{
    return new CircleCheckPoints(center, radius);
}

CheckPoints* DirtyLinks::CreateCirclePoints(BaseEntity* entity, int radius)
{
    return new CircleCheckPoints(entity, radius);
}

void DirtyLinks::SendTurnEvent(player_type player, bool fbegin)
{
    if(player != PT_PLAYER || fbegin)
        return;
        
    IWorld::Get()->GetEffects()->NewTurn();
    LevelAPI::GetAPI()->EndTurn(0);
/*
    typedef std::vector<point3>     pnt3_vec_t;
    typedef std::map<float, point3> dmg_map_t;
    
    pnt3_vec_t area_pnts;
    dmg_map_t  damage_map;
    
    HexGrid::cell_iterator first_cell = HexGrid::GetInst()->first_cell(),
                           last_cell  = HexGrid::GetInst()->last_cell();
    
    while(first_cell != last_cell){

        first_cell->DecreaseEffect();
        
        if(GroundEffect* effect = first_cell->GetEffect()){
            //соберем поврежденную область
            area_pnts.push_back(*first_cell);
            //соберем карут повреждений
            damage_map[effect->GetObjectFlameDmg()] = *first_cell;
        }
        
        ++first_cell;
    }

    rid_set_t area_objs;              

    //составим список объектов для повреждения
    GetObjPool()->CollectObjectsForPoints(area_pnts, &area_objs);

    //повредим все объекты в области
    dmg_map_t::reverse_iterator  itor = damage_map.rbegin();
    while(itor != damage_map.rend() && area_objs.size()){
        
        rid_set_t pnt_objs;

        area_pnts.clear();
        area_pnts.push_back(itor->second);
        
        //найдем объекты в этой точке
        GetObjPool()->CollectObjectsForPoints(area_pnts, &pnt_objs);
        
        rid_set_t::iterator obj = pnt_objs.begin();
        while(obj != pnt_objs.end() && area_objs.size()){
            
            if(area_objs.count(*obj)){
                
                //повредим объект
                ExplosionManager::Get()->BurnObject(*obj, itor->first);
                
                //удалим его из списка
                area_objs.erase(*obj);
            }
            
            ++obj;
        }
        
        ++itor;
    }
    */
}

//
//обработка событий на уровне
//
class LevelEvents : public LogicAPI{
public:

    void CaseDestroyed(const rid_t& rid)
    {
    }

    void OptionsChanged()
    {
        GraphGrid::GetInst()->HandleOptionsChange();
    }

    ~LevelEvents(){}

private:

    LevelEvents(){SetAPI(this);}

    static LevelEvents m_obj;
};

LevelEvents LevelEvents::m_obj;

//================================================

namespace{

    //
    // информация о попадании
    //
    class HitInfoAdapter : public hit_s, public hit_info_s{
    public:
        
        HitInfoAdapter(const hit_s& hit)
        {
            hit_info_s::m_to = hit.m_to;
            hit_info_s::m_from = hit.m_from;
            hit_info_s::m_radius = hit.m_radius;
            
            hit_info_s::m_dmg[hit_info_s::BW_DMG].m_val = hit.m_dmg[hit_s::BW_DMG].Value;
            hit_info_s::m_dmg[hit_info_s::BW_DMG].m_type = Hit2Dmg(hit.m_dmg[hit_s::BW_DMG].Type);
            
            hit_info_s::m_dmg[hit_info_s::AW_DMG].m_val = hit.m_dmg[hit_s::AW_DMG].Value;
            hit_info_s::m_dmg[hit_info_s::AW_DMG].m_type = Hit2Dmg(hit.m_dmg[hit_s::AW_DMG].Type);
            
            hit_info_s::m_dmg[hit_info_s::AA_DMG].m_val = hit.m_dmg[hit_s::AA_DMG].Value;        
            hit_info_s::m_dmg[hit_info_s::AA_DMG].m_type = Hit2Dmg(hit.m_dmg[hit_s::AA_DMG].Type);
        }
        
        HitInfoAdapter(const hit_info_s& hit) 
        {
            hit_s::m_to = hit.m_to;
            hit_s::m_from = hit.m_from;
            hit_s::m_radius = hit.m_radius;
            
            hit_s::m_dmg[hit_s::BW_DMG].Value = hit.m_dmg[hit_info_s::BW_DMG].m_val;
            hit_s::m_dmg[hit_s::BW_DMG].Type = Dmg2Hit(hit.m_dmg[hit_info_s::BW_DMG].m_type);
            
            hit_s::m_dmg[hit_s::AW_DMG].Value = hit.m_dmg[hit_info_s::AW_DMG].m_val;
            hit_s::m_dmg[hit_s::AW_DMG].Type = Dmg2Hit(hit.m_dmg[hit_info_s::AW_DMG].m_type);
            
            hit_s::m_dmg[hit_s::AA_DMG].Value = hit.m_dmg[hit_info_s::AA_DMG].m_val;        
            hit_s::m_dmg[hit_s::AA_DMG].Type = Dmg2Hit(hit.m_dmg[hit_info_s::AA_DMG].m_type);
        }
        
    private:
        
        ::damage_type Hit2Dmg(hit_s::damage_type type)
        {
            switch(type){
            case DT_NONE: return ::DT_NONE;
            case DT_SHOCK: return ::DT_SHOCK;
            case DT_FLAME: return ::DT_FLAME;
            case DT_STRIKE: return ::DT_STRIKE;
            case DT_ENERGY: return ::DT_ENERGY;
            case DT_ELECTRIC: return ::DT_ELECTRIC;
            case DT_EXPLOSIVE: return ::DT_EXPLOSIVE;
            }
            
            return ::DT_NONE;
        }
        
        hit_s::damage_type Dmg2Hit(::damage_type type)
        {
            switch(type){
            case ::DT_NONE: return DT_NONE;
            case ::DT_SHOCK: return DT_SHOCK;
            case ::DT_FLAME: return DT_FLAME;
            case ::DT_STRIKE: return DT_STRIKE;
            case ::DT_ENERGY: return DT_ENERGY;
            case ::DT_ELECTRIC: return DT_ELECTRIC;
            case ::DT_EXPLOSIVE: return DT_EXPLOSIVE;
            }
            
            return DT_NONE;
        }
    };
        
    //
    // реализация менеджера повреждений
    //
    class DamagerImp : public Damager{
    public:

        void HandleAirHit(BaseEntity* actor, const hit_t& hit, unsigned flags)
        {
            DSBridge bridge(actor, flags);
            ExplosionManager::Get()->OnAirHit(Entity2GID(actor), HitInfoAdapter(hit));
        }

        //уведомление о попадании в уровень
        void HandleLevelHit(BaseEntity* actor, const hit_t& hit, unsigned flags)
        {
            DSBridge bridge(actor, flags);
            ExplosionManager::Get()->OnLevelHit(Entity2GID(actor), HitInfoAdapter(hit));
        }

        //уведомление о попадании в щит
        void HandleShieldHit(BaseEntity* actor, const hit_t& hit, unsigned flags)
        {
            DSBridge bridge(actor, flags);
            ExplosionManager::Get()->OnShieldHit(Entity2GID(actor), HitInfoAdapter(hit));
        }

        //уведомление о попадании в объект
        void HandleObjectHit(BaseEntity* actor, const rid_t& obj, const hit_t& hit, unsigned flags)
        {
            DSBridge bridge(actor, flags);
            ExplosionManager::Get()->OnObjectHit(Entity2GID(actor), obj, HitInfoAdapter(hit));
        }

        //уведомление о попадании в существо
        void HandleEntityHit(BaseEntity* actor, BaseEntity* victim, const hit_t& hit, unsigned flags)
        {
            DSBridge bridge(actor, flags);
            ExplosionManager::Get()->OnEntityHit(Entity2GID(actor), Entity2GID(victim), HitInfoAdapter(hit));
        }

        class DSBridge : private EntityObserver,
                         public  LogicDamageStrategy
        {
        public:

            DSBridge(BaseEntity* actor, unsigned flags) : 
                m_actor(actor), m_flags(flags)
            {
                if(m_actor) m_actor->Attach(this, EV_PREPARE_DEATH);
                ExplosionManager::Get()->SetLogicDamageStrategy(this);
            }

            ~DSBridge()
            {
                if(m_actor) m_actor->Detach(this);
                ExplosionManager::Get()->SetLogicDamageStrategy(0);
            }

            void DamageEntity(int gid, const hit_s& hit)
            {       
                if(BaseEntity* victim = GID2Entity(gid)) EntityDamager::GetInst()->Damage(m_actor, victim, HitInfoAdapter(hit), m_flags);
            }
            
            void DestroyObject(const std::string& rid)
            {
                Bureau::GetInst()->HandleObjectDamage(rid);
                GameObjectsMgr::GetInst()->MarkAsDestroyed(rid);        
            }
            
            void LightFire(const point3 &pos, float radius, float damage)
            {
                RiskArea::GetInst()->InsertFlame(pos, radius, damage);
            }

        private:

            void Update(BaseEntity* entity, event_t event, info_t info)
            {
                m_actor->Detach(this);
                m_actor = 0;
            }

        private:

            unsigned    m_flags;
            BaseEntity* m_actor;
        };
    };
}

Damager* Damager::GetInst()
{
    static DamagerImp imp;
    return &imp;
}

//==================================================================================

/*
  файл организован так:

  начало файла

        unsigned - смещение от начала файла до первой index_record
        unsigned - размер индексной табл. + размер данных (пригодится при save/load)
            ...
        chunk
            ...
        chunk
            ...
        chunk
            ...            
        index_record
            ...
        index_record            

  конец файла 
  
  chunk это:

        unsigned - размер данных в байтах
        char[]   - массив данных

  index_record это:

        unsigned - смещение от начала файла до chunk
        c_string - название (послед. символов оканчивающаяся 0)
*/

LevelFile::LevelFile(AbstractFile* file, const std::string& level, open_mode mode) :
    m_pos(0), m_file(file), m_level(level), m_mode(mode)
{
    m_file->Seek(0, SEEK_END);

    if(m_mode == OM_READ && m_file->GetPos()){
        
        //прочтем индексную таблицу
        size_t index_off;
        m_file->Seek(0, SEEK_SET);
        m_file->Read(&index_off, sizeof(index_off));

        index_map_t indexes;
        m_file->Seek(index_off, SEEK_SET);
        ReadIndexMap(m_file, indexes);

        if(size_t file_ptr = indexes[m_level]){
            
            size_t chunk_size;
            m_file->Seek(file_ptr, SEEK_SET);
            m_file->Read(&chunk_size, sizeof(chunk_size));

            chunk_size -= sizeof(chunk_size);

            while(chunk_size--){
                unsigned char ch;
                m_file->Read(&ch, sizeof(ch));
                m_data.push_back(ch);
            }
        }
    }
}

LevelFile::~LevelFile()
{
    //пишем данные в файл
    if(m_mode != OM_WRITE) return;

    size_t index_off = 0;
    index_map_t indexes;
    unsigned char buff[BUFF_SIZE];

    m_file->Seek(0, SEEK_END);

    if(m_file->GetPos()){

        //найдем таблицу индексов
        m_file->Seek(0, SEEK_SET);
        m_file->Read(&index_off, sizeof(index_off));

        //читаем таблицу индексов
        m_file->Seek(index_off, SEEK_SET);
        ReadIndexMap(m_file, indexes);

        if(size_t file_ptr = indexes[m_level]){
                
            m_file->Seek(file_ptr, SEEK_SET);
            
            //прочтем размер блока
            size_t chunk_size = 0;
            m_file->Read(&chunk_size, sizeof(chunk_size));
            
            //вычислим конец данных
            index_off -= chunk_size;
           
            //сместим данные на размер блока
            while(file_ptr < index_off){
                
                //читаем
                m_file->Seek(file_ptr + chunk_size, SEEK_SET);
                size_t read_size = m_file->Read(buff, BUFF_SIZE);
                
                //пишем
                m_file->Seek(file_ptr, SEEK_SET);
                m_file->Write(buff, read_size);
                
                file_ptr += read_size;
            }

            file_ptr = indexes[m_level];

            //подкорректируем таблицу индексов
            for(index_map_t::iterator itor = indexes.begin(); itor != indexes.end(); ++itor)
                if(itor->second > file_ptr) itor->second -= chunk_size;
        }

        //установим файловый указатель в позицию записи данных
        m_file->Seek(index_off, SEEK_SET);

    }else{

        //пропустим место под смещение таблицы индексов
        m_file->Write(&index_off, sizeof(index_off));
        m_file->Write(&index_off, sizeof(index_off));
    }

    //запомним наш индекс в таблице
    indexes[m_level] = m_file->GetPos();

    //пропишем размер
    size_t data_chunk_size = sizeof(size_t) + m_data.size();
    m_file->Write(&data_chunk_size, sizeof(data_chunk_size));

    //пишем данные
    for(size_t k = 0; k < m_data.size(); k++){
        data_buff_t::value_type val = m_data[k];
        m_file->Write(&val, sizeof(val));
    }

    //запомним смещение таблицы индексов
    index_off = m_file->GetPos();
    m_file->Seek(0, SEEK_SET);
    int tmp = m_file->GetPos();
    m_file->Write(&index_off, sizeof(index_off));

    //пишем таблицу индексов
    m_file->Seek(index_off, SEEK_SET);
    WriteIndexMap(m_file, indexes);

    //пишем размер файла
    size_t file_size = m_file->GetPos();
    m_file->Seek(sizeof(index_off), SEEK_SET);
    m_file->Write(&file_size, sizeof(file_size));
}

long LevelFile::GetPos()
{
    return m_pos;
}

void LevelFile::Seek(long offset, int origin)
{
    switch(origin){
    case SEEK_CUR:
        m_pos += offset;
        break;
    case SEEK_END:
		m_pos = m_data.size() + offset;
        break;
    case SEEK_SET:
	    m_pos = offset;
        break;
    }
}

size_t LevelFile::Read(void* ptr, size_t size)
{
    if(m_mode != OM_READ) return 0;

    if(m_pos + size > m_data.size()) size = m_data.size() - m_pos;
    
    std::copy(   m_data.begin() + m_pos, 
                 m_data.begin() + m_pos + size,
                 static_cast<data_buff_t::value_type*>(ptr));
    
    m_pos += size;
    return size;
} 

size_t LevelFile::Write(void* ptr, size_t size)
{
    if(m_mode != OM_WRITE) return 0;

    if(m_pos + size > m_data.size()) m_data.resize(m_pos + size);

    std::copy(  static_cast<data_buff_t::value_type*>(ptr),
                static_cast<data_buff_t::value_type*>(ptr) + size,
                m_data.begin() + m_pos);

    m_pos += size;
    return size;
}

void LevelFile::ReadIndexMap(AbstractFile* file, index_map_t& indexes)
{
    size_t index;
    std::string str;

    //читаем индекс
    m_file->Read(&index, sizeof(index));
    while(index){

        char ch = 0;
        str.clear();

        m_file->Read(&ch, sizeof(ch));
        while(ch){
            str += ch;
            m_file->Read(&ch, sizeof(ch));
        }

        //вставим прочитаное в таблицу
        indexes[str] = index;

        m_file->Read(&index, sizeof(index));
    }
}

void LevelFile::WriteIndexMap(AbstractFile* file, index_map_t& indexes)
{
    index_map_t::iterator itor = indexes.begin();

    while(itor != indexes.end()){
     
        //пишем индекс
		m_file->Write(&itor->second, sizeof(index_map_t::value_type));
        //пишем ключ
        m_file->Write(const_cast<char*>(itor->first.c_str()), itor->first.size()+1);
        
        ++itor;
    }

    //пишем признак конца таблицы
    size_t null_index = 0;
    m_file->Write(&null_index, sizeof(null_index));
}

void LevelFile::MakeSaveLoad(SavSlot& slot)
{
    if(slot.IsSaving()){

        size_t file_size = 0;
       
        //узнаем размер данных
        m_file->Seek(0, SEEK_END);
        if(m_file->GetPos()){
            m_file->Seek(sizeof(size_t), SEEK_SET);
            m_file->Read(&file_size, sizeof(file_size));
        }

        slot << file_size;

        size_t bytes_read;
        unsigned char buff[BUFF_SIZE];

        //пишем данные в save
        m_file->Seek(0, SEEK_SET);
        while(file_size){

            bytes_read = m_file->Read(buff, BUFF_SIZE);

            //сохраним только то, что нужно
            if(bytes_read > file_size) bytes_read = file_size;                        

            slot.Save(buff, bytes_read);
            file_size -= bytes_read;
        }

    }else{

        //восст. данные из Save'а
        size_t file_size;
        slot >> file_size;

        size_t bytes_read;
        unsigned char buff[BUFF_SIZE];

        m_file->Seek(0, SEEK_SET);
        while(file_size){
            
            bytes_read = file_size > BUFF_SIZE ? BUFF_SIZE : file_size;
            
            slot.Load(buff, bytes_read);
            m_file->Write(buff, bytes_read);           

            file_size -= bytes_read;
        }
    }
}

//===========================================================================

BoxInfo::BoxInfo(const rid_t& rid) :
    m_case(0)
{
    if(rid.size()) m_case = GetObjPool()->GetCase(rid);
}

bool BoxInfo::IsStorage() const
{
    return m_case->IsStorage();
}

bool BoxInfo::IsDatabase() const
{
    return m_case->IsDatabase();
}
  
const std::string& BoxInfo::GetName() const
{
    return m_case->GetName();
}

const std::string& BoxInfo::GetItems() const
{
    return m_case->GetItems();
}

int BoxInfo::GetWeight() const
{
    return m_case->GetWeight();
}
 
int BoxInfo::GetRespawn() const
{
    return m_case->GetRespawn();
}

int BoxInfo::GetExperience() const
{
    return m_case->GetExperience();
}

