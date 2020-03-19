/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: интерфейс для работы со скриптовыми сценами
				
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#if !defined(__SCRIPT_SCENE_MANAGER_H__)
#define __SCRIPT_SCENE_MANAGER_H__

class ScriptScene;
class ScriptSceneAPI;

class ScriptSceneManager
{
	class Eraser;
private:
	struct Deleter 
	{
	public:
		ScriptSceneManager* m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	static const char* m_CommonScriptName;
private:
	ScriptSceneManager();
	virtual ~ScriptSceneManager();
public:
	enum EVENT {E_NONE,E_STARTLEVEL,E_EXITTOLEVEL,E_KILL,E_USE,E_QUESTFINISHED};
private:
	//	описание свойств скриптовой сцены
	struct SceneTraits
	{
		std::string m_Name;						//	наименование скриптовой сцены
		EVENT		m_Event;					//	событие, по которому инициализируется сцена 
		std::string m_Object;					//	имя объекта убитого или поюзанного
		std::string m_Level;					//	имя уровня на котором произошло событие
		std::string m_ToLevel;					//	имя следующего уровня
		std::string m_Quest;					//	имя квеста, по окончании которого должна сработать скриптовая сцена
		bool		m_Phase[4];					//	номер фазы
		const SceneTraits& operator=(const SceneTraits& st)
		{
			m_Name	  = st.m_Name;
			m_Event   = st.m_Event;		m_Phase[0] = st.m_Phase[0];
			m_Object  = st.m_Object;	m_Phase[1] = st.m_Phase[1];
			m_Level   = st.m_Level;		m_Phase[2] = st.m_Phase[2];
			m_ToLevel = st.m_ToLevel;	m_Phase[3] = st.m_Phase[3];
			m_Quest	  = st.m_Quest;
			return *this;
		}
	};
	std::map<int,std::list<SceneTraits> > m_Scenes;			//	список всех скриптовых сцен
	int m_Episode;											//	текущий эпизод
	Eraser* m_Eraser;										//	стиратель скриптовых сцен
public:
	//	параметры, для создания сцены
	struct Params
	{
		std::string m_Object;					//	имя объекта убитого или поюзанного
		std::string m_Level;					//	имя уровня на котором произошло событие
		std::string m_ToLevel;					//	имя следующего уровня
		std::string m_Quest;					//	имя квеста, по окончании которого должна сработать скриптовая сцена
		unsigned int m_Phase;					//	номер фазы
		const Params& operator = (const Params& p)
		{
			m_Object	= p.m_Object;
			m_Level		= p.m_Level;
			m_ToLevel	= p.m_ToLevel;
			m_Phase		= p.m_Phase;
			m_Quest		= p.m_Quest;

			return *this;
		}
	};
	// создать скриптовую сцену
	ScriptScene* CreateScene(const EVENT event,const Params& params);
	//	удалить скриптовую сцену из списка
	void EraseScene(const EVENT event,const Params& params);
	// уведомить менеджер о начале новой игры
	struct NewGameParams
	{
		int m_Episode;					//	номер текущего эпизода
	};
	void OnBegNewGame(const NewGameParams& ngp);
	//	определить, были ли проигранна скриптовая сцена (регистр букв учитывается при сравнении)
	bool IsScriptSceneFinished(int episode,const char* name);
	//	сохранить/прочитать игру
	void MakeSaveLoad(SavSlot& slot);
private:
	//	читаем скрипт на проигрываемые сцены
	void ReadXlsScript(int episode);
	//	переводим индекс в событие
	ScriptSceneManager::EVENT IndexToEvent(int index) const;
	//	переводим имя в объект-сцену
	ScriptScene* NameToScene(int episode,const char* scene_name) const;
public:
	static ScriptSceneManager *Instance(void);
};

inline ScriptSceneManager* ScriptSceneManager::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new ScriptSceneManager();
	return m_Deleter.m_pInstance;
}

//**************************************************************************************//
// полуабстрактный класс для скриптовой сцены
class ScriptScene
{
public:
	class Object;
	class Eraser;
	// перечисление для команд
	enum COMMAND {TICK,SKIP};
	//	перечисление для состояний скриптовой сцены
	enum STATE {S_START,S_RUN,S_FINISH};
private:
	std::map<std::string,Object*> m_Objects;
	STATE m_State;
	Eraser* m_Eraser;
protected:
	bool m_Erase;
public:
	ScriptScene();
	virtual ~ScriptScene();
	// возвращает true если нужно и дальше продолжать вызывать эту функцию
	bool Run(const COMMAND command);
	//	добавить объект в карту
	bool AddObject(Object* object);
	//	проверка на целостность сцены
	bool IsValid(void) const;
	//	усиановить стиратель
	void SetEraser(Eraser* eraser);
protected:
	//	начало скриптовой сцены
	virtual bool OnStart(void) = 0;
	//	окончание скриптовой сцены
	virtual bool OnFinish(void) = 0;
	//	прерывание скриптовой сцены
	virtual bool OnSkip(void) = 0;
	//	окончание действия объекта скриптовой сцены
	virtual bool OnObjectNoActive(Object* object) = 0;
	//	передача управления скриптовой сцене
	virtual bool OnTick(void) = 0;
protected:
	//	узнать состояние
	ScriptScene::STATE State(void) const;
	//	получить указатель на набор функций
	ScriptSceneAPI* GetApi(void);
private:
	//	думатель
	bool OnThink(void);
};

//	узнать ссстояние
inline ScriptScene::STATE ScriptScene::State(void) const
{
	return m_State;
}

//**************************************************************************************//
//	class ScriptScene::Object
class ScriptScene::Object
{
private:
	std::string m_Name;				//	имя объекта
	bool m_Activated;				//	флаг активности 
public:
	Object(const char* name) : m_Name(name),m_Activated(false) {}
	virtual ~Object() {}
public:
	//	объект думает (false - объект перестал думать)
	virtual bool OnThink(void) = 0;
	//	тип объекта
	virtual const char* Type(void) const = 0;
	//	имя объекта
	const char* Name(void) const {return m_Name.c_str();}
	//	узнать активен ли объект
	bool IsActivated(void) const {return m_Activated;}
	//	изменить активность объекта
	void Activated(bool flag) {m_Activated = flag;}
};

//**************************************************************************************//
//	class ScriptScene::Eraser
class ScriptScene::Eraser
{
public:
	virtual ~Eraser() {}
	virtual void Do(void) = 0;
};

#endif // !defined(__SCRIPT_SCENE_MANAGER_H__)
