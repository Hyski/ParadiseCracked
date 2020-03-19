/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#ifndef _GAME_H_
#define _GAME_H_

class GraphPipe; 

namespace Game
{
	//	игровое время
	extern float m_fGameTime;
	//	инициализация
	void Init(GraphPipe *lpGraphPipe);
	//	создание игрового интерфейса
	void PriorInterfaceInitialize(void);
	void CreateInterface(void);
	//	проигрывание мультика
	void PlayBink(const char *pFileName);
	//	смена видеорежима
	void Tune(void);
	void Close(void);
	//	передача управления игре
	void Tick(void);
	void ProcessInput(void);
	void Render(void);
	//	управление игрой
	void SetGameLevelRender(bool render);
	void SetGameLevelUpdate(bool update);
	void LoadGameLevel(const char *pLevelName);
	void UnloadGameLevel(void);
	//	получение скриншота
	void MakeScreenShot(void);
	//	вызов после Alt-Tab'а
	void AdjustHW(void);
	//	изменение качества эффектов
	void SetEffectsQuality(float q);
	//	сохранить/прочитать игру
	void MakeSaveLoad(const char* pFileName,bool bSave);
	bool RenderEnabled();
}

#endif