//
// Грязные связи - связи от кот. нужно избавиться
//

#ifndef _PUNCH_DIRTYLINKS_H_
#define _PUNCH_DIRTYLINKS_H_

struct damage_s;

class GameLevel;
class GraphPipe;
class LevelCase;
class BaseEntity;

//
// интерфейс на автобусный маршрут 
// (его можно получить только через GameLevel)
//
class BusRoute{
public:

    virtual ~BusRoute(){}

    //абстрактное опред динамич конструктора
    DCTOR_ABS_DEF(BusRoute)
    //система сохранения/загрузки
    virtual void MakeSaveLoad(SavSlot& st) = 0;

    enum point_type{
        PT_STOP,    //остановка 
        PT_PATH,    //путевая точка
        PT_DEST,    //конечная остановка
        PT_LAST,    //последняя точка пути
    };

    struct pos_s{
        ipnt2_t m_pos;
        float   m_angle;
    };

    //узнать тип текущей точки
    virtual point_type GetPntType(const ipnt2_t& cur_pos) = 0;
    //перейти в начало маршрута
    virtual void GenLoc(bool first, pos_s* pos) = 0;
    //узнать путь для автобуса
    virtual void GetPath(const ipnt2_t& cur_pos, pnt_vec_t* path) = 0;
};

//
// интерейс на точки до кот нужно дойти
//
class CheckPoints{
public:

    virtual ~CheckPoints(){}

    //абстрактное опред динамич конструктора
    DCTOR_ABS_DEF(CheckPoints)
    //система сохранения/загрузки
    virtual void MakeSaveLoad(SavSlot& st) = 0;

    struct pos_s{
        ipnt2_t m_pos;
        float   m_angle;
        bool    m_fspecial;

        pos_s() : m_fspecial(false){}
    };

    //узнать точку, направление анимацию для проигрывания
    virtual void GetPos(pos_s* pos) const = 0;
    //это последняя точка в списке?
    virtual bool IsEnd() const = 0;

    //перейти к следующей точке
    virtual void Next() = 0;

    enum first_type{
        FT_USUAL,
        FT_SPAWN,
    };

    //перейти к первой точке
    virtual void First(first_type type = FT_USUAL) = 0;
};

//
// файл реализованный через Win32 API
//
class Win32File : public AbstractFile{
public:

    Win32File(const std::string& fname, unsigned create, unsigned access = GENERIC_READ|GENERIC_WRITE) :   
      m_hfile(CreateFile(fname.c_str(), access, 0, NULL, create, FILE_ATTRIBUTE_NORMAL, NULL)){}

    ~Win32File() { CloseHandle(m_hfile); }
        
    long GetPos(){ return SetFilePointer(m_hfile, 0, NULL, FILE_CURRENT); }

    void Seek(long offset, int origin)
    {
        switch(origin){
        case SEEK_CUR:
            SetFilePointer(m_hfile, offset, NULL, FILE_CURRENT);
            break;

        case SEEK_END:
            SetFilePointer(m_hfile, offset, NULL, FILE_END);
            break;

        case SEEK_SET:
            SetFilePointer(m_hfile, offset, NULL, FILE_BEGIN);
            break;
        }
    }

    size_t Read(void* ptr, size_t size)
    {
        DWORD bytes_read;
        ReadFile(m_hfile, ptr, size, &bytes_read, NULL);
        return bytes_read;
    }

    size_t Write(void* ptr, size_t size)
    {
        DWORD bytes_written;
        WriteFile(m_hfile, ptr, size, &bytes_written, NULL);
        return bytes_written;
    }

private:

    HANDLE m_hfile;
};

//
// класс для хранения данных связанных с тек.уровнем
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ВНИМАНИЕ:  при переходе на другой уровень
//            объект класса должен создаваться заново
//          
class LevelFile : public AbstractFile {
public:

    enum open_mode{
        OM_READ,     //чтение данных (аналог "r")
        OM_WRITE,    //запись данных (аналог "w")
    };

    LevelFile(AbstractFile* file, const std::string& level, open_mode om);
    ~LevelFile();

    //записать/восстановить данные
    void MakeSaveLoad(SavSlot& slot);

    //узнать текущее положение указателя в файле
    long GetPos();
    //установить указатель файла на опред позицию
    void Seek(long offset, int origin);

    //запись / чтение из файла
    size_t Read(void* ptr, size_t size);
    size_t Write(void* ptr, size_t size);

private:

    typedef std::map<std::string,size_t> index_map_t;

    //читать/писать таблицу (файловый указатель находится в позиции чтения/записи)
    void ReadIndexMap(AbstractFile* file, index_map_t& indexes);
    void WriteIndexMap(AbstractFile* file, index_map_t& indexes);
    
private:

    enum { BUFF_SIZE = 256 };

    long        m_pos;
    open_mode   m_mode;
    data_buff_t m_data;
    std::string m_level;

    AbstractFile* m_file;    
};

//
// обертка вокруг указателя на свойства ящика
//
class BoxInfo{
public:

    BoxInfo(const rid_t& rid = rid_t());

	bool IsStorage() const;
	//это база данных
	bool IsDatabase() const;
	
	//узнать символьную метку ящика
	const std::string& GetName() const;
	//узнать название набора предметов
	const std::string& GetItems() const;
	
	int GetWeight() const;
	int GetRespawn() const;
	int GetExperience() const;

private:
    
    const LevelCase* m_case;
};

//
// интерфейсный класс для создания всех ящиков с помощью активного итератора
//
class BoxSpawner{
public:

    virtual ~BoxSpawner(){}

    //создать информационную консоль
    virtual void CreateDataBox(const rid_t& rid) = 0;
    //создать ящик с предметами
    virtual void CreateStoreBox(const rid_t& rid) = 0;
};

//
// грязные связи
//
namespace DirtyLinks{
	bool IsGoodEntity(BaseEntity *ent);
    //получить ссылку на GraphPipe
    GraphPipe* GetGraphPipe();
    //получить ссылку на GameLevel
    GameLevel* GetGameLevel();

    //узнать имя текущего уровня
    const std::string& GetLevelSysName();
    std::string GetLevelFullName(const std::string& sys_name);

    //получить строковый ресурс
    std::string GetStrRes(const rid_t& rid); 

    //получить состояние объекта 
    bool GetObjState(const rid_t& rid);
    //установить состояние объекта
    void SetObjState(const rid_t& rid, bool state);
    //получить название ключа для use на объекте
    rid_t GetKey4UseObj(const rid_t& obj_rid);
    //получить wisdom для use на объекте
    int GetWisdom4UseObj(const rid_t& obj_rid);
    //это лифт?
    bool IsElevatorObj(const rid_t& obj_rid);

    //уничтожить объект на уровне
    void EraseLevelObject(const rid_t& rid);
    //обойти все ящики на уровне
    void EnumerateAllLevelCases(BoxSpawner* spawner);
    //вычислить точки HexGrid'а кот занимает объект
    void CalcObjectHexes(const rid_t& rid, pnt_vec_t* pnts);

    //можно ли объект иcпользовать
    bool CanUseObject(const rid_t& obj_rid, std::string* hint);        

    //получить точку на уровне, исходя из коорд. мыши
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //возврат:
    //  true    -   успех
    //  flase   -   неудалось
    //
    bool GetLevelPnt(point3* lev_pnt);

    //включить/отключить крыши зданий
    void EnableRoofs(bool flag);

    // загрузить уровень
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //  при передаче пустой строки происходит
    //  выгрузка игрового уровня.
    // 
    void LoadLevel(const std::string& level);

    enum button_type{
        BT_OK     =   1 << 0,
        BT_CANCEL =   1 << 1,
    };

    //установить значение bar'a при загрузке
    void SetLoadBarPos(float val);

    //узнaть координаты центра объекта
    point3 GetObjCenter(const rid_t& rid); 

    enum message_type{
        MT_USUAL,   //обычное сообщение
        MT_DENIAL,  //отказ в чем - либо
    };

    //вывести текст консоль
    void Print(const std::string& str, message_type type = MT_USUAL);

    //узнать высоту ланшафта в т. pnt.x, pnt.y
    float GetLandHeight(const point3& pnt);
    //отрисовать BBox
    void DrawBBox(const BBox& box, unsigned color);
    //отрисовать примитив
    void DrawPrimi(const std::string& shader, const Primi& pr);
    //переключить состояние объекта
    float SwitchObjState(const std::string& obj_name);

    //послать собщение о передаче хода
    void SendTurnEvent(player_type player, bool fbegin);

    //узнать входит ли точка в конус видимости камеры
    bool IsInCameraCone(const point3& point);
    //подвинуть камеру в позицию
    void MoveCamera(const point3& pos, float time = 0.2f);

    //узнать позицию камеры
    const point3& GetCamPos();
    //перевод точки на экране в направление
    point3 PickCam(float x, float y);

    enum opt_type{

        OT_SHOW_PATH,         //показывать путь для unit'а
        OT_SHOW_FRONT,        //показывать предел поля проходимости
        OT_SHOW_JOINTS,       //показывать зоны выхода      
        OT_SHOW_LANDS,        //показывать точки высадки из техники
        OT_SHOW_FOS,          //постоянно показывать FOS 
        OT_SHOW_BANNER,       //постоянно показывать Banner

        OT_MOUSE_UPDATE_TIME, //время обновления мыши
        OT_ENABLE_THEMES,     //отключить проигрывние мызыкальных тем

        OT_ENABLE_HELPER,     //показывать helper?

        OT_HIDDEN_MOVEMENT_ENEMY_RELATED_MOVE,      //показывать перемещение враждебных персонажей
        OT_HIDDEN_MOVEMENT_ENEMY_RELATED_SHOOT,     //показывать стрельбу враждебных персонажей
        OT_HIDDEN_MOVEMENT_FRIEND_RELATED_MOVE,     //показывать перемещение дружественных персонажей
        OT_HIDDEN_MOVEMENT_FRIEND_RELATED_SHOOT,    //показывать стрельбу дружественных персонажей
        OT_HIDDEN_MOVEMENT_NEUTRAL_RELATED_MOVE,    //показывать перемещение нейтральных персонажей
        OT_HIDDEN_MOVEMENT_NEUTRAL_RELATED_SHOOT,   //показывать стрельбу нейтральных персонажей

        OT_HIDDEN_MOVEMENT_CIVILIAN_VEHICLES_MOVE,          //показывать перемещение мирной техники
        OT_HIDDEN_MOVEMENT_CIVILIAN_VEHICLES_SHOOT,         //показывать стрельбу мирной техники
        OT_HIDDEN_MOVEMENT_CIVILIAN_HUMANS_TRADERS_MOVE,    //показывать перемещение мирных жителей
        OT_HIDDEN_MOVEMENT_CIVILIAN_HUMANS_TRADERS_SHOOT,   //показывать стрельбу мирных жителей

        MAX_OPTIONS,
    };

    //работа с опциями
    int GetIntOpt(opt_type type);
    float GetFloatOpt(opt_type type);

    //получить параметр командной строки
    bool GetCmdLineArg(const std::string& arg_name, std::string* value);

    //разрешить/запретить рендеринг уровня 
    void EnableLevelRender(bool flag);
    //разрешить обработку управление камерой
    void EnableCameraControl(bool flag);
    //отключить/включить звук на уровне
    void EnableLevelAndEffectSound(bool flag);

    //произвести quick Save/quick Load
    void MakeQuickSaveLoad(Storage::storage_mode mode);

    //создать автобусный маршрут
    BusRoute* CreateBusRoute(const std::string& sys_name);
    //создать набор CheckPoints
    CheckPoints* CreateCheckPoints(const std::string& sys_name);
    //создать набор точек для движения в пределах окружности
    CheckPoints* CreateCirclePoints(BaseEntity* entity, int radius);
    CheckPoints* CreateCirclePoints(const ipnt2_t& center, int radius);
}

#endif // _PUNCH_DIRTYLINKS_H_