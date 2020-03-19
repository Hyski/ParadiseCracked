//
//  ��������� ����� �� ������������
//

#ifndef _PUNCH_OBSERVER_H_
#define _PUNCH_OBSERVER_H_

#pragma warning(disable : 4503)

typedef void* void_ptr;

//
// ������� ����� ������ �����������
//
template<class _Subject, class _Event = int, class _Info = void_ptr>
class Observer{
public:

    virtual ~Observer(){}

    //��� ����������
    typedef _Info info_t;
    //��� �������
    typedef _Event event_t;
    //��� ������������ �������
    typedef _Subject subject_t;

    //������� ��� ����������� �����������
    virtual void Update(subject_t subj, event_t event, info_t info) = 0;
};

//
// ������ ��� �������� � �������� �������
//
template<class _Observer>
class ObserverManager{
public:

    //��� �����������
    typedef _Observer observer_t;

    //��� ������������ ���������� � �������
    typedef typename observer_t::info_t info_t;
    //��� �������
    typedef typename observer_t::event_t event_t;
    //��� ������������ �������
    typedef typename observer_t::subject_t subject_t;

    //������� � ��������
    void Detach(observer_t* observer)
    {
        for(size_t k = 0; k < m_observers.size(); k++){
            if(m_observers[k].m_observer == observer)
                m_observers[k].m_observer = 0;            
        }
    }

    //����������� �� �������
    void Attach(observer_t* observer, event_t event)
    {
        for(size_t k = 0; k < m_observers.size(); k++){
            if(m_observers[k].m_observer == 0){
                m_observers[k].m_event = event;
                m_observers[k].m_observer = observer;
                return;
            }
        }

        m_observers.push_back(obs_s(observer, event));
    }

    //��������� �����������
    void Notify(subject_t subj, event_t event, info_t info)
    {
        for(size_t k = 0; k < m_observers.size(); k++){
            if(m_observers[k].m_observer && m_observers[k].m_event == event)
							{
							obs_s &o = m_observers[k];
              o.m_observer->Update(subj, event, info);
							}
        }
    }
    bool isAttached(observer_t *subj)
    {
        for(size_t k = 0; k < m_observers.size(); k++){
            if(m_observers[k].m_observer && m_observers[k].m_observer == subj)
							{
							return true;
							}
        }
				return false;
    }

private:
    
    struct obs_s{
        event_t     m_event;
        observer_t* m_observer;

        obs_s(observer_t* obs, event_t event) : m_observer(obs), m_event(event) {} 
    };

    typedef std::vector<obs_s> obs_vec_t;
    obs_vec_t m_observers;
};

#endif // _PUNCH_OBSERVER_H_
