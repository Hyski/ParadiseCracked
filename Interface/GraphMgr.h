/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   23.05.2001

************************************************************************/
#ifndef _GRAPH_MGR_H_
#define _GRAPH_MGR_H_

#include "../Common/UI/Widget.h"

class GLayer;
class Text;

class GraphWindow : public Widget
{
public:
	static HPEN m_BasePen;
	static HBRUSH m_BaseBrush;
private:
	//	флаг того что окошко перетаскивается вслед за мышью
	bool m_MouseCapture;
	int m_xMouse,m_yMouse;
private:
	Text* m_minText;
	Text* m_maxText;
	//	технологическая часть
	GLayer* m_Layer;
	float m_Time;				//	промежуток времени от последнего назад
	float m_MinValue;
	float m_MaxValue;
private:
	std::map<float,float> m_Values;
public:
	GraphWindow(const char* name,int width,int height);
	virtual ~GraphWindow();
public:
	//	интерфейс с пользователем
	void Calibrate(float time,float min_value,float max_value);
	void AddValue(float time,float value);
	void Clear(void);
	void AutoScale(void);
	void Show(void);
	void Hide(void);
private:
	//	обработка мышинного ввода
	bool ProcessMouseInput(VMouseState* pMS);
	//	производим какие-то действия
	bool Tick(void);
	//	уведомление об изменении режима работы системы
	void OnSystemChange(void);
	//	составляем график
	void Build(void);
public:
	const char* Type(void) const {return "graphwindow";};
};


class GraphMgr
{
	class Console;
private:
	//	сам объект менеджера 
	static GraphMgr* m_Instance;
	//	флаг присутсвия менеджера в игре
	static bool m_IsAvailable;
	//	родительское окно для виджетов
	static Widget* m_Owner;
	//	контролер консоль
	Console* m_Console;
public:
	GraphMgr();
	virtual ~GraphMgr();
public:	//	интерфейс инициализации менеджера графиков
	//	создать менеджер
	static GraphMgr* SetAvailable(Widget* pOwner); 
	//	проверить присутствует ли менеджер
	static bool IsAvailable(void);
	//	уничтожить менеджер
	static void Release(void);
public:
	//	создать окно для вывод графика
	static GraphWindow* BuildWindow(const char* name,int width,int height); 
	//	получить окно по имени
	static GraphWindow* Window(const char* name);
	//	уничтожить окно
	static void DestroyWindow(const char* name);
};

inline bool GraphMgr::IsAvailable(void) 
{
	return m_IsAvailable;
}

#endif	//_GRAPH_MGR_H_