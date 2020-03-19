/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#include "precomp.h"
#include <ctime>
#include "Timer.h"

bool  Timer::m_fsuspend;
float Timer::m_counter;
float Timer::m_freq;
ULARGE_INTEGER Timer::m_initial;   

void Timer::Init(void)
{
    //сбросим счетчик
    m_counter = 0;
    
    //вычислим количество тиков процессора в сек.
    ULARGE_INTEGER a, b, s;
    
    int tp, pp;
	HANDLE t, p;

	t = GetCurrentThread();
	tp = GetThreadPriority(t);
	SetThreadPriority(t, THREAD_PRIORITY_TIME_CRITICAL);
	p = GetCurrentProcess();
	pp = GetPriorityClass(p);
	SetPriorityClass(p, REALTIME_PRIORITY_CLASS);

	Sleep(1);
	GetCPUTicks(&a);   // read the performance counter 
	Sleep(100);        // pause 100 ms 
	GetCPUTicks(&b);   // read the performance counter 
    
    //!!! ~~~~~~~~~ ВНИМАНИЕ ~~~~~~~~~~~ !!!
    // ситуация с переполнением обрабатывается автоматически
    // возможно потому, что все в выражении unsigned типа. 
    // Интересно а как на других компиляторах?
    //
  	s.QuadPart = b.QuadPart - a.QuadPart;
		
    SetThreadPriority(t, tp);
	SetPriorityClass(p, pp);

    //вычислим и сохраним значение чаcmоты
    m_freq = (__int64)(10 * s.QuadPart);

    //запомним текущее положение счетчика
    GetCPUTicks(&m_initial);	
}

void Timer::Update(void)
{
    ULARGE_INTEGER  cur, delta;

    GetCPUTicks(&cur);    

    //!!! ~~~~~~~~~ ВНИМАНИЕ ~~~~~~~~~~~ !!!
    // ситуация с переполнением обрабатывается автоматически
    // возможно потому, что все в выражении unsigned типа. 
    // Интересно а как на других компиляторах?
    //
    delta.QuadPart = cur.QuadPart - m_initial.QuadPart;
    
    //запомним текущее положение счетчика
    m_initial = cur;
    
    //если таймер остановлен то все
    if(m_fsuspend) return;

    m_counter += ((__int64) delta.QuadPart) / m_freq;
}

void Timer::Suspend()
{
    //запретить подсчет секунд
    m_fsuspend = true;
}

void Timer::Resume()
{
    //считать текущее положение счетчика
    Update();

    //разрешить подсчет секунд
    m_fsuspend = false;
}

tm* Timer::GetSysTime(void)
{
	time_t long_time;

	time(&long_time);          

	return localtime(&long_time);
}


