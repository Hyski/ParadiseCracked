/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ��������� ��� ������ �� ����������� �������
				
                                                                                
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
	//	�������� ������� ���������� �����
	struct SceneTraits
	{
		std::string m_Name;						//	������������ ���������� �����
		EVENT		m_Event;					//	�������, �� �������� ���������������� ����� 
		std::string m_Object;					//	��� ������� ������� ��� ����������
		std::string m_Level;					//	��� ������ �� ������� ��������� �������
		std::string m_ToLevel;					//	��� ���������� ������
		std::string m_Quest;					//	��� ������, �� ��������� �������� ������ ��������� ���������� �����
		bool		m_Phase[4];					//	����� ����
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
	std::map<int,std::list<SceneTraits> > m_Scenes;			//	������ ���� ���������� ����
	int m_Episode;											//	������� ������
	Eraser* m_Eraser;										//	��������� ���������� ����
public:
	//	���������, ��� �������� �����
	struct Params
	{
		std::string m_Object;					//	��� ������� ������� ��� ����������
		std::string m_Level;					//	��� ������ �� ������� ��������� �������
		std::string m_ToLevel;					//	��� ���������� ������
		std::string m_Quest;					//	��� ������, �� ��������� �������� ������ ��������� ���������� �����
		unsigned int m_Phase;					//	����� ����
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
	// ������� ���������� �����
	ScriptScene* CreateScene(const EVENT event,const Params& params);
	//	������� ���������� ����� �� ������
	void EraseScene(const EVENT event,const Params& params);
	// ��������� �������� � ������ ����� ����
	struct NewGameParams
	{
		int m_Episode;					//	����� �������� �������
	};
	void OnBegNewGame(const NewGameParams& ngp);
	//	����������, ���� �� ���������� ���������� ����� (������� ���� ����������� ��� ���������)
	bool IsScriptSceneFinished(int episode,const char* name);
	//	���������/��������� ����
	void MakeSaveLoad(SavSlot& slot);
private:
	//	������ ������ �� ������������� �����
	void ReadXlsScript(int episode);
	//	��������� ������ � �������
	ScriptSceneManager::EVENT IndexToEvent(int index) const;
	//	��������� ��� � ������-�����
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
// ��������������� ����� ��� ���������� �����
class ScriptScene
{
public:
	class Object;
	class Eraser;
	// ������������ ��� ������
	enum COMMAND {TICK,SKIP};
	//	������������ ��� ��������� ���������� �����
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
	// ���������� true ���� ����� � ������ ���������� �������� ��� �������
	bool Run(const COMMAND command);
	//	�������� ������ � �����
	bool AddObject(Object* object);
	//	�������� �� ����������� �����
	bool IsValid(void) const;
	//	���������� ���������
	void SetEraser(Eraser* eraser);
protected:
	//	������ ���������� �����
	virtual bool OnStart(void) = 0;
	//	��������� ���������� �����
	virtual bool OnFinish(void) = 0;
	//	���������� ���������� �����
	virtual bool OnSkip(void) = 0;
	//	��������� �������� ������� ���������� �����
	virtual bool OnObjectNoActive(Object* object) = 0;
	//	�������� ���������� ���������� �����
	virtual bool OnTick(void) = 0;
protected:
	//	������ ���������
	ScriptScene::STATE State(void) const;
	//	�������� ��������� �� ����� �������
	ScriptSceneAPI* GetApi(void);
private:
	//	��������
	bool OnThink(void);
};

//	������ ���������
inline ScriptScene::STATE ScriptScene::State(void) const
{
	return m_State;
}

//**************************************************************************************//
//	class ScriptScene::Object
class ScriptScene::Object
{
private:
	std::string m_Name;				//	��� �������
	bool m_Activated;				//	���� ���������� 
public:
	Object(const char* name) : m_Name(name),m_Activated(false) {}
	virtual ~Object() {}
public:
	//	������ ������ (false - ������ �������� ������)
	virtual bool OnThink(void) = 0;
	//	��� �������
	virtual const char* Type(void) const = 0;
	//	��� �������
	const char* Name(void) const {return m_Name.c_str();}
	//	������ ������� �� ������
	bool IsActivated(void) const {return m_Activated;}
	//	�������� ���������� �������
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
