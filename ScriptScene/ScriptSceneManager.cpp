/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ��������� ��� ������ �� ����������� �������
				
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#include "scriptpch.h"
#include "../Common/Precomp.h"
#include "../Common/DataMgr/DataMgr.h"
#include "../Common/DataMgr/TxtFile.h"
#include "ScriptSceneAPI.h"
#include "ScriptSceneManager.h"

#include "SSEpisode1.h"
#include "SSEpisode2.h"
#include "SSEpisode3.h"
#include "SSEpisode4.h"

//**********************************************************************//
//	class ScriptSceneManager::Eraser
class ScriptSceneManager::Eraser : public ScriptScene::Eraser
{
private:
	ScriptSceneManager::EVENT m_Event;
	ScriptSceneManager::Params m_Params;
public:
	Eraser() {}
	~Eraser() {}
public:
	void Set(const EVENT event,const Params& params)
	{
		m_Event = event;
		m_Params = params;
	}
	void Do(void)
	{
		ScriptSceneManager::Instance()->EraseScene(m_Event,m_Params);
	}
};

ScriptSceneManager::Deleter ScriptSceneManager::m_Deleter;
const char* ScriptSceneManager::m_CommonScriptName = "scripts/sscenes/episode";
ScriptSceneManager::ScriptSceneManager() : m_Episode(0),m_Eraser(0)
{
	m_Eraser = new Eraser();
}

ScriptSceneManager::~ScriptSceneManager()
{
	delete m_Eraser;
}

// ������� ���������� �����
ScriptScene* ScriptSceneManager::CreateScene(const EVENT event,const Params& params)
{
	SceneTraits st;
	ScriptScene* rss = 0;

	if(params.m_Phase < 4)
	{
		std::map<int,std::list<SceneTraits> >::iterator im = m_Scenes.find(m_Episode);

		if(im != m_Scenes.end())
		{
			for(std::list<SceneTraits>::iterator i=im->second.begin();i!=im->second.end();i++)
			{
				st = *i;
				//	������������ �������
				if(st.m_Event == event)
				{
					//	����������� �������, ������, ����
					if(((st.m_Object.size()  && (st.m_Object == params.m_Object))	|| !st.m_Object.size())  &&
						((st.m_Level.size()	 && (st.m_Level == params.m_Level))		|| !st.m_Level.size())	 &&
						((st.m_ToLevel.size() && (st.m_ToLevel == params.m_ToLevel)) || !st.m_ToLevel.size()) &&
						((st.m_Quest.size() && (st.m_Quest == params.m_Quest)) || !st.m_Quest.size()) &&
						st.m_Phase[params.m_Phase])
					{
						rss = NameToScene(m_Episode,st.m_Name.c_str());
						if(rss)
						{//	������������� ���������� ����������� �����
							m_Eraser->Set(event,params);
							rss->SetEraser(m_Eraser);
						}
						break;
					}
				}
			}
		}
	}

	return rss;
}
//	������� ���������� ����� �� ������
void ScriptSceneManager::EraseScene(const EVENT event,const Params& params)
{
	SceneTraits st;
	std::map<int,std::list<SceneTraits> >::iterator im = m_Scenes.find(m_Episode);
	
	if(im != m_Scenes.end())
	{
		for(std::list<SceneTraits>::iterator i=im->second.begin();i!=im->second.end();i++)
		{
			st = *i;
			//	������������ �������
			if(st.m_Event == event)
			{
				//	����������� �������, ������, ����
				if(((st.m_Object.size()  && (st.m_Object == params.m_Object))	|| !st.m_Object.size())  &&
					((st.m_Level.size()	 && (st.m_Level == params.m_Level))		|| !st.m_Level.size())	 &&
					((st.m_ToLevel.size() && (st.m_ToLevel == params.m_ToLevel)) || !st.m_ToLevel.size()) &&
					((st.m_Quest.size() && (st.m_Quest == params.m_Quest)) || !st.m_Quest.size()) &&
					st.m_Phase[params.m_Phase])
				{
					im->second.erase(i);
					break;
				}
			}
		}
	}
}

//	��������� ��� � ������-�����
ScriptScene* ScriptSceneManager::NameToScene(int episode,const char* scene_name) const
{
	switch(episode)
	{
	case 0:
		if(!stricmp(scene_name,"Scene1"))  return new Episode1::Scene1();
		if(!stricmp(scene_name,"Scene2"))  return new Episode1::Scene2();
		if(!stricmp(scene_name,"Scene3"))  return new Episode1::Scene3();
		if(!stricmp(scene_name,"Scene4"))  return new Episode1::Scene4();
		if(!stricmp(scene_name,"Scene5"))  return new Episode1::Scene5();
		if(!stricmp(scene_name,"Scene6"))  return new Episode1::Scene6();
		if(!stricmp(scene_name,"Scene7a")) return new Episode1::Scene7();
		if(!stricmp(scene_name,"Scene7b")) return new Episode1::Scene7();
		if(!stricmp(scene_name,"FinalVideo"))  return Episode1::FinalEpisodeVideo();
		break;
	case 1:
		if(!stricmp(scene_name,"Scene1")) return new Episode2::Scene1();
		if(!stricmp(scene_name,"Scene2")) return new Episode2::Scene2();
		if(!stricmp(scene_name,"Scene3")) return new Episode2::Scene3();
		if(!stricmp(scene_name,"FinalVideo1"))  return Episode2::FinalEpisodeVideo1();
		if(!stricmp(scene_name,"FinalVideo2"))  return Episode2::FinalEpisodeVideo2();
		break;
	case 2:
		if(!stricmp(scene_name,"Scene1")) return new Episode3::Scene1();
		if(!stricmp(scene_name,"Scene2")) return new Episode3::Scene2();
		if(!stricmp(scene_name,"Scene3")) return new Episode3::Scene3();
		if(!stricmp(scene_name,"Scene4")) return new Episode3::Scene4();
		if(!stricmp(scene_name,"FinalVideo"))  return Episode3::FinalEpisodeVideo();
		break;
	case 3:
		if(!stricmp(scene_name,"Scene1")) return new Episode4::Scene1();
		break;
	}

	return 0;
}
//	������ ������ �� ������������� �����
void ScriptSceneManager::ReadXlsScript(int episode)
{
	SceneTraits st;
	char script_name[100];
	std::string tmp;

	//	��������� �� ������������� ���������� ���� ��� ������� �������
	std::map<int,std::list<SceneTraits> >::iterator im = m_Scenes.find(episode);
	if(im != m_Scenes.end()) m_Scenes.erase(im);
	
	//	��� ������� ������� ���� xls'��
	sprintf(script_name,"%s%d.txt",m_CommonScriptName,episode);
	TxtFile script(DataMgr::Load(script_name));
	DataMgr::Release(script_name);
	
	//	��������� ��������� ���������� ����
	for(int i=1;i<script.SizeY();i++)
	{
		//	��������� ��� ���������� �����
		st.m_Name = script(i,0);
		if(st.m_Name.size())
		{
			//	��������� ��������� ���������
			st.m_Object = script(i,6);
			st.m_Level = script(i,7);
			st.m_ToLevel = script(i,8);
			st.m_Quest = script(i,9);
			//	��������� �������
			int j;
			for(j=1;j<6;j++)
			{
				tmp = script(i,j);
				if(tmp.size()) break;
			}
			st.m_Event = IndexToEvent(j-1);
			//	��������� ����
			tmp = script(i,10);
			unsigned int count,phase[4];
			
			for(j=0;j<4;j++) st.m_Phase[j] = false;
			
			count = sscanf(tmp.c_str(),"%d.%d.%d.%d",&phase[0],&phase[1],&phase[2],&phase[3]);
			
			if(count != EOF && count)
			{
				for(;count;count--)
				{
					if(phase[count-1] < 4)
						st.m_Phase[phase[count-1]] = true;
				}
			}
			else
			{
				for(j=0;j<4;j++) st.m_Phase[j] = true;
			}
			
			//	������� �������� ���������� ����� � �����
			m_Scenes[episode].push_back(st);
		}
	}
}
//	��������� ������ � �������
ScriptSceneManager::EVENT ScriptSceneManager::IndexToEvent(int index) const
{
	switch(index)
	{
	case 0 :  return E_STARTLEVEL;
	case 1 :  return E_EXITTOLEVEL;
	case 2 :  return E_KILL;
	case 3 :  return E_USE;
	case 4 :  return E_QUESTFINISHED;
	default:  return E_NONE;
	}
}
// ��������� �������� � ������ ����� ����
void ScriptSceneManager::OnBegNewGame(const NewGameParams& ngp)
{
	m_Episode = ngp.m_Episode;
	//	������ ������
	ReadXlsScript(ngp.m_Episode);
}
//	���������/��������� ����
void ScriptSceneManager::MakeSaveLoad(SavSlot& slot)
{
	unsigned int ui;

	if(slot.IsSaving())
	{
		slot << m_Episode;				//	���������� ����� �������� �������
		ui = m_Scenes.size();
		slot << ui;						//	���������� ���-�� ��������
		for(std::map<int,std::list<SceneTraits> >::iterator im = m_Scenes.begin();im != m_Scenes.end();im++)
		{
			slot << im->first;			//	���������� ����� �������
			ui = im->second.size();
			slot << ui;					//	���������� ���-�� ���� � ������� �������
			for(std::list<SceneTraits>::iterator il = im->second.begin();il != im->second.end();il++)
			{
				//	���������� ��������� �����
				slot << (*il).m_Name;
				slot << static_cast<unsigned int>((*il).m_Event);		
				slot << (*il).m_Object;	
				slot << (*il).m_Level;		
				slot << (*il).m_ToLevel;	
				slot << (*il).m_Quest;	
				slot << (*il).m_Phase[0];
				slot << (*il).m_Phase[1];
				slot << (*il).m_Phase[2];
				slot << (*il).m_Phase[3];
			}
		}
	}
	else
	{
		unsigned int num_of_episodes;

		m_Scenes.clear();			//	������� ������� ������ ����
		slot >> m_Episode;			//	��������� ����� �������
		slot >> num_of_episodes;	//	��������� ���-�� �������� �� ����������� �������
		for(int i=0;i<num_of_episodes;i++)
		{
			int episode_number;
			unsigned int num_of_scenes;

			slot >> episode_number;		//	��������� ����� �������
			slot >> num_of_scenes;		//	��������� ���-�� ����
			for(int j=0;j<num_of_scenes;j++)
			{
				const unsigned int version = slot.GetStore()->GetVersion();
				SceneTraits st;

				//	��������� ��������� �����
				slot >> st.m_Name;
				slot >> ui;
				st.m_Event = static_cast<EVENT>(ui);		
				slot >> st.m_Object;	
				slot >> st.m_Level;		
				slot >> st.m_ToLevel;	
				if(version > 0) slot >> st.m_Quest;	//	��������� ������������ ������, �� ���������� �������� ����������� ���������� �����
				slot >> st.m_Phase[0];
				slot >> st.m_Phase[1];
				slot >> st.m_Phase[2];
				slot >> st.m_Phase[3];

				m_Scenes[episode_number].push_back(st);
			}
		}
	}
}
//	����������, ���� �� ���������� ���������� �����
bool ScriptSceneManager::IsScriptSceneFinished(int episode,const char* name)
{
	std::map<int,std::list<SceneTraits> >::iterator i = m_Scenes.find(episode);

	if(i != m_Scenes.end())
	{
		for(std::list<SceneTraits>::iterator j=i->second.begin();j!=i->second.end();j++)
		{
			if((*j).m_Name == name) return false;
		}
	}

	return true;
}

//**************************************************************************************//
// class ScriptScene
ScriptScene::ScriptScene() : m_State(S_START),m_Eraser(0),m_Erase(true)
{
}

ScriptScene::~ScriptScene()
{
	for(std::map<std::string,Object*>::iterator i=m_Objects.begin();i!=m_Objects.end();i++)
		delete i->second;
	if(m_Erase && m_Eraser) m_Eraser->Do();
	//	������� ������������ ����
	Input::Update();
}
//	���������� ���������
void ScriptScene::SetEraser(Eraser* eraser)
{
	m_Eraser = eraser;
}
// ���������� true ���� ����� � ������ ���������� �������� ��� �������
bool ScriptScene::Run(const COMMAND command)
{
	switch(command)
	{
	case SKIP:
		if(OnSkip())
		{
			GetApi()->FinishScene();
			return false;
		}
		break;
	case TICK:
		switch(m_State)
		{
		case S_START:
			GetApi()->StartScene();
			if(!OnStart()) m_State = S_FINISH;
			else m_State = S_RUN;
			break;
		case S_RUN:
			if(!OnThink()) m_State = S_FINISH;
			break;
		case S_FINISH:
			if(OnFinish())
			{
				GetApi()->FinishScene();
				return false;
			}
			break;
		}
	}

	return true;
}
//	�������� ������ � �����
bool ScriptScene::AddObject(Object* object)
{
	if(object)
	{
		std::map<std::string,Object*>::iterator i = m_Objects.find(object->Name());
		if(i == m_Objects.end())
		{
			m_Objects[object->Name()] = object;
		}
	}

	return false;
}
//	��������
bool ScriptScene::OnThink(void)
{
	bool flag = true;

	if(!OnTick()) return false;
	for(std::map<std::string,Object*>::iterator i=m_Objects.begin();i!=m_Objects.end();i++)
	{
		if(i->second->IsActivated())
		{
			if(!i->second->OnThink())
			{
				if(!OnObjectNoActive(i->second))
					flag = false;
			}
		}
	}

	return flag;
}

ScriptSceneAPI* ScriptScene::GetApi(void)
{
	return ScriptSceneAPI::Instance();
}
//	�������� �� ����������� �����
bool ScriptScene::IsValid(void) const
{
	return m_Objects.size();
}
