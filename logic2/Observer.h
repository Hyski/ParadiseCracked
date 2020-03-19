//
//  шаблонный класс на наблюдателей
//

#ifndef _PUNCH_OBSERVER_H_
#define _PUNCH_OBSERVER_H_

#pragma warning(disable : 4503)

typedef void* void_ptr;

//
// базовый класс самого наблюдателя
//
template<class _Subject, class _Event = int, class _Info = void_ptr>
class Observer{
public:

    virtual ~Observer(){}

    //тип информации
    typedef _Info info_t;
    //тип события
    typedef _Event event_t;
    //тип наблюдаемого объекта
    typedef _Subject subject_t;

    //функция для уведомления наблюдателя
    virtual void Update(subject_t subj, event_t event, info_t info) = 0;
};

//
// сервер для подписки и рассылки событий
//
template<class _Observer>
class ObserverManager{
public:

    //тип наблюдателя
    typedef _Observer observer_t;

    //тип пересылаемой информации о событии
    typedef typename observer_t::info_t info_t;
    //тип события
    typedef typename observer_t::event_t event_t;
    //тип наблюдаемого объекта
    typedef typename observer_t::subject_t subject_t;

    //сняться с подписки
    void Detach(observer_t* observer)
    {
        for(size_t k = 0; k < m_observers.size(); k++){
            if(m_observers[k].m_observer == observer)
                m_observers[k].m_observer = 0;            
        }
    }

    //подписаться на событие
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

    //отправить уведомление
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
