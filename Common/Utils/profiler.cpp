#include "precomp.h"
#include "profiler.h"

#include "../../Interface/GraphMgr.h"

#define _DO_PROFILE_


std::map<std::string,CodeProfiler::Item> CodeProfiler::m_ProfData;

CodeProfiler::CodeProfiler(char* BlockName)
	{
#ifdef _DO_PROFILE_
	Timer::Update();
	m_StartTime=Timer::GetSeconds();
	strncpy(m_Name,BlockName,IDENT_LEN-1);
	m_Name[IDENT_LEN-1]=0;
#endif
	}

CodeProfiler::~CodeProfiler()
	{
#ifdef _DO_PROFILE_
	Timer::Update();
	m_StartTime=Timer::GetSeconds()-m_StartTime;
	Item &i=m_ProfData[m_Name];

	i.OverallTime += m_StartTime;
	i.FrameTime += m_StartTime;
	i.BlockNumber++;
#endif
	}


void CodeProfiler::DepictItem(const std::string &name,const Item& it)
{
	static const int SZx=100,SZy=50;
	static ipnt2_t place(D3DKernel::ResX()-SZx,0);

	const char *cname=name.c_str();

	if(!GraphMgr::Window(cname))
	{
		GraphWindow* wnd = GraphMgr::BuildWindow(cname,SZx,SZy);
		if(wnd)
			{
			wnd->Calibrate(10,0,0.05);
			wnd->MoveTo(place.x,place.y);
			place.y += SZy+5;
			if(place.y > D3DKernel::ResY())
				{
				place.y = 0;
				place.x -= SZx+5;
				}
			}
	}
	//	добавление значения графика
	if(GraphMgr::Window(cname))
	GraphMgr::Window(cname)->AddValue(Timer::GetSeconds(),it.FrameTime);
}

void CodeProfiler::FrameStart()   //для подсчета времени/кадр
	{
#ifdef _DO_PROFILE_
	std::map<std::string,Item>::iterator it,ite=m_ProfData.end();
	for(it=m_ProfData.begin();it!=ite;it++)
		{
			it->second.FrameNumber++;
			DepictItem(it->first,it->second);

			it->second.FrameTime = 0;
		}
#endif
	}


void CodeProfiler::Init()
	{
#ifdef _DO_PROFILE_
	m_ProfData.clear();
#endif
	}


void CodeProfiler::Close()
	{
#ifdef _DO_PROFILE_
	std::map<std::string,Item>::iterator it,ite=m_ProfData.end();
	for(it=m_ProfData.begin();it!=ite;it++)
		{

		}
#endif
	}
