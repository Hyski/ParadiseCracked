#ifndef _PUNCH_PROFILER_H_
#define _PUNCH_PROFILER_H_

extern CLog logFile;

class profiler{
public:
    
    profiler(const std::string& log_name, const std::string& label) :
      m_label(label), m_log_name(log_name), m_overal_time(0), m_counter(0){}
    
    ~profiler()
    {
        logFile[m_log_name.c_str()]("%s enter times %i\n", m_label.c_str(), m_counter);
        logFile[m_log_name.c_str()]("%s overal work time %f\n", m_label.c_str(), m_overal_time);
        
        if(m_counter) logFile[m_log_name.c_str()]("%s average work time %f\n", m_label.c_str(), m_overal_time/m_counter);
    }
    
    void enter()
    {
        Timer::Update();
        m_enter_time = Timer::GetSeconds();
    }
    
    void leave()
    {
        m_counter++;
        
        Timer::Update();
        float delta = Timer::GetSeconds() - m_enter_time;
        
        logFile[m_log_name.c_str()]("%i: %s %f\n", m_counter, m_label.c_str(), delta);
        m_overal_time += delta;
    }
    
    struct enter_leave{
        
        enter_leave(profiler* prof) :  m_profiler(prof) { m_profiler->enter(); }
        ~enter_leave() { m_profiler->leave(); }
    
        profiler*   m_profiler;
    };
    
private:
        
    int   m_counter;
       
    float m_enter_time;
    float m_overal_time;

    std::string m_label;
    std::string m_log_name;
};

class memory_state{
public:

    memory_state(const std::string& label = std::string()) :
      m_label(label) { _CrtMemCheckpoint(&m_state);}

    friend memory_state operator - (const memory_state& m1, const memory_state& m2)
    {
        memory_state ret;
        _CrtMemDifference(&ret.m_state, &m1.m_state, &m2.m_state);
        return ret;
    }

    void DumpStat() { _CrtMemDumpStatistics(&m_state); }

private:

    std::string  m_label;
    _CrtMemState m_state;
};

#endif // _PUNCH_PROFILER_H_