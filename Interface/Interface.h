/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   13.04.2000

************************************************************************/
#ifndef _INTERFACE_H_
#define _INTERFACE_H_

class Interface
{
public:
	class ProgressBar;
private:
	struct Deleter 
	{
	public:
		Interface *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
	static bool m_IsInitialized;
private:
	ProgressBar* m_ProgressBar;
private:
	Interface();
	virtual ~Interface();
public:	
	//	реакци€ на смену видео режима
	void OnChangeVideoMode(void);	
	//	передача управлени€
	void Tick(void);
	//	отображение 2D интерфейса
	void Render2D(void);
	//	отображение 3D интерфейса
	void Render3D(void);
	//	освобождение всех ресурсов интерфейса
	//	об€зательно вызывать перед закрытием приложени€
	void Release(void);
public:	//****  логическое управление интерфейсом ****//
	//	создание меню
	void RunMenuFlow(void);
	//	загрузка уровн€
	void LoadGameLevel(const char* pLevelName);
	//	вызов диалогового окна yes/no
	int QuestionBox(const char* text);
	//	обновление прогресc бара «ј√–”« » ”–ќ¬Ќя
	void UpdateLoadingProgressBar(float value);
	//	установить заголовок прогресс бара «ј√–”« » ”–ќ¬Ќя
	void SetProgressBarTitle(const char* title);
	//	записать/считать данные необходимые интерфейсу
	void MakeSaveLoad(Storage& store);
	//	получить модуль управлени€ прогресс баром
	ProgressBar* ProBar(void) const;
	//	получить утверждение, что камеру можно перемещать
	bool IsCameraMoveable(void);
public:
	static Interface *Instance(void);
	static bool IsInitialized(void) {return m_IsInitialized;}
	//	предварительна€ инициализаци€ интерфейса
	static void PriorInitialize(void);
	//	проиграть мультик
	static void PlayBink(const char* file_name);		
	//	проиграть несколько мультиков
	static void PlayBink(std::vector<std::string>& file_names);		
};

inline Interface* Interface::Instance(void)
{
	if(m_Deleter.m_pInstance && !IsInitialized()) throw CASUS("»нтерфейс не проинициализирован!");

	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new Interface();
	return m_Deleter.m_pInstance;
}
//	получить модуль управлени€ прогресс баром
inline Interface::ProgressBar* Interface::ProBar(void) const
{
	return m_ProgressBar;
}

/////////////////////////////////////////////////////////////////////////
///////////////////    class Interface::ProgressBar    //////////////////
/////////////////////////////////////////////////////////////////////////
class Interface::ProgressBar
{
private:
	float m_From;
	float m_To;
public:
	ProgressBar();
	~ProgressBar();
public:
	//	установка интервала отображени€ значений 0..1 на интервал from..to
	void SetRange(float from, float to);
	//	установка процента бара в установленном интервале
	void SetPercent(float value); 
	//	установка процента по формуле m/n
	void SetMofN(int m, int n); 
	//	установка строки, описывающей текущие загружаемые данные
	void SetTitle(const char* title); 
};

#endif	//_INTERFACE_H_