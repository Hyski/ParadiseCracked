#if !defined(__OBSERVER_TEMPLATE_INCLUDED__)
#define __OBSERVER_TEMPLATE_INCLUDED__

#include <set>

///базовое пространство имен
namespace mll
{

/// пространство имен для различных шаблонных подходов к организации структуры программы
namespace patterns
{

template<typename Event>
class observer;

template<typename Event>
class adv_observer;

template<typename E>
class base_subject;

template<typename Event>
struct observer_traits;

template<typename Event>
struct adv_observer_traits;

///класс наблюдателя 
/**
если удобно наследоваться для получения оповещений, используйте observer
 или можно использовать adaptor
\param Event тип события
*/
template<typename Event>
class observer
{
public:
	typedef Event event_type;						 ///< тип получаемого сообщения
	typedef observer_traits<event_type> traits_type; ///< класс описаия взаимодействия между subject и observer

	virtual void on_notify(const event_type &) = 0;	 ///< собсвенно прием сообщения.
};

/// продвинутый наблюдатель
/**
	добавлены 2 метода оповещения
	при подключении и при отключении от объекта
\param Event тип события
*/
template<typename Event>
class adv_observer
{
public:
	typedef Event event_type;							///< тип получаемого сообщения
	typedef adv_observer_traits<event_type> traits_type;///< класс описаия взаимодействия между subject и observer
	typedef base_subject<event_type> subject_type;		///< базовый тип наблюдаемого объекта

	virtual void on_notify(const event_type &) = 0;		///< оповещение о сообщении
	virtual void on_attach(subject_type *) = 0;			///< оповещение о подключении к объекту
	virtual void on_detach(subject_type *) = 0;			///< оповещение об отключении от объекта
};

///базовый тип наблюдаемого объекта
/**
	наблюдатели могут получить от него только сообщение параметризованного типа.
\param Event тип события
*/
template<typename Event>
class base_subject
{
protected:
	virtual ~base_subject() {}

public:
	typedef Event event_type;

	virtual void notify(const event_type &) = 0;
};

/// интерфейс класса-доставщика событий
/**
 к интерфейсу класса-родителя добавлены функции оповещения наблюдателя о подключении/отключении
 к потоку событий, проверка подключения к потоку событий.
\param E тип события
\param O тип наблюдателя
*/
template<typename E,
		 typename O = observer<E> >
class isubject : public base_subject<E>
{
protected:
	virtual ~isubject() {}

public:
	typedef O observer_type;

	virtual void attach(observer_type *) = 0;	///< подключить наблюдателя к потоку событий
	virtual void detach(observer_type *) = 0;   ///<отключить наблюдателя от потока событий
	virtual bool is_attached(observer_type *) const = 0; ///<проверить подключен ли наблюдатель к потоку событий
};

///реализация простого наблюдаемого
/**
\param E тип события
\param O тип наблюдателя
*/
template<typename E,
		 typename O = observer<E>,
		 typename T = typename O::traits_type>
class simple_subject : public isubject<E,O>
{
protected:
	typedef std::set<observer_type *> observers_type;
	typedef typename observers_type::iterator iterator_type;
	observers_type m_observers;

	bool m_destructing;

public:
	typedef T observer_traits_type;

	simple_subject() : m_destructing(false) {}
	virtual ~simple_subject()
	{
		m_destructing = true;
		while(!m_observers.empty())
		{
			detach(*m_observers.begin());
		}
	}

	virtual void attach(observer_type *o)
	{
		if(!m_destructing && m_observers.find(o) == m_observers.end())
		{
			m_observers.insert(o);
			observer_traits_type::on_attach(o,this);
		}
	}

	virtual void detach(observer_type *o)
	{
		observer_traits_type::on_detach(o,this);
		m_observers.erase(o);
	}

	virtual void notify(const event_type &event)
	{
		for(observers_type::iterator itor = m_observers.begin(); itor != m_observers.end(); ++itor)
		{
			observer_traits_type::on_notify(event,*itor,this);
		}
	}

	virtual bool is_attached(observer_type *obs) const
	{
		return m_observers.find(obs) != m_observers.end();
	}
};

///класс-наблюдаемый
/** он ответственнен за рассылку сообщений.
класс, рассылающий события наследуется от subject или делегирует subject
\param E тип события
\param O тип наблюдателя
\param T ???
*/
template<typename E,
		 typename O = observer<E>,
		 typename T = typename O::traits_type>
class subject : public simple_subject<E,O,T>
{
public:
	virtual ~subject() {}

	virtual void notify(const event_type &event)
	{
		observers_type tmp(m_observers);

		for(observers_type::iterator itor = tmp.begin(); itor != tmp.end(); ++itor)
		{
			observer_traits_type::on_notify(event,*itor,this);
		}
	}
};

/// посредник между наблюдателем и наблюдаемым. 
/**
позволяет отделить наблюдаемого от реализации наблюдателя

*/
template<typename Event>
struct observer_traits
{
	typedef Event event_type;
	typedef observer<Event> observer_type;
	typedef base_subject<Event> subject_type;
///диспетчерная функция для вызова сообщения у наблюдателя
	static void on_notify(const event_type &event, observer_type *o, subject_type *subj)
	{
		o->on_notify(event); 
	}

	static void on_attach(observer_type *o, subject_type *subj) {}
	static void on_detach(observer_type *o, subject_type *subj) {}
};

/// посредник между наблюдателем и наблюдаемым. 
template<typename Event>
struct adv_observer_traits
{
	typedef Event event_type;
	typedef adv_observer<Event> observer_type;
	typedef base_subject<Event> subject_type;

	static void on_notify(const event_type &event, observer_type *o, subject_type *subj)
	{
		o->on_notify(event); 
	}

	static void on_attach(observer_type *o, subject_type *subj)
	{
		o->on_attach(subj);
	}

	static void on_detach(observer_type *o, subject_type *subj)
	{
		o->on_detach(subj);
	}
};
/// посредник между наблюдателем и наблюдаемым. 
/**
	сообщение приходит единоды. после этого наблюдатель отключается.
*/
template<typename Traits>
struct detach_on_notify
{
	typedef typename Traits::event_type event_type;
	typedef typename Traits::observer_type observer_type;
	typedef isubject<event_type,observer_type> subject_type;
	typedef Traits parent_traits_type;

	static void on_notify(const event_type &event, observer_type *o, subject_type *subj)
	{
		subj->detach(o);
		parent_traits_type::on_notify(event,o,subj);
	}

	static void on_attach(observer_type *o, subject_type *subj)
	{
		parent_traits_type::on_attach(o,subj);
	}

	static void on_detach(observer_type *o, subject_type *subj)
	{
		parent_traits_type::on_detach(o,subj);
	}
};
///адаптирование наблюдателя к вызову определенного метода объекта
/**
	этот класс преобразовывает вызов определенной функции наблюдателя (обычно on_notify)
	в вызов указаной функции указанного объекта.
	   \param R тип объекта-получателя
	   \param E тип получаемого события
*/
template<typename R, typename E, typename P = const E &>
class adaptor : public observer<E>
{
public:
	typedef R receiver_type;
	typedef E event_type;
	typedef P param_type;
	typedef observer<E> observer_type;

	typedef void (R::*notify_func_type)(param_type);

private:
	receiver_type *m_recv;
	notify_func_type m_func;

public:
	adaptor() : m_recv(0), m_func(0) {}
	adaptor(receiver_type *recv, notify_func_type func) : m_recv(recv), m_func(func) {}
	adaptor(const adaptor &rhs) : m_recv(rhs.m_recv), m_func(rhs.m_func) {}

	adaptor &operator=(const adaptor &rhs)
	{
		this->m_recv = rhs.m_recv;
		this->m_func = rhs.m_func;
		return *this;
	}

	virtual void on_notify(const event_type &event)
	{
		(m_recv->*m_func)(event);
	}
};

/// адаптор для наблюдателя
/** класс-наблюдатель должен включать adaptor в себя, инстанцируя его функцией, которую
 нужно вызывать при событии
	   \param R тип объекта-получателя
	   \param E тип получаемого события
*/
template<typename R, typename E, typename P = const E &>
class adv_adaptor : public adv_observer<E>
{
public:
	typedef R receiver_type;
	typedef E event_type;
	typedef P param_type;
	typedef adv_observer<E> observer_type;
	typedef isubject<event_type,observer_type> subject_type;

	typedef void (R::*notify_func_type)(param_type);
	typedef void (R::*tach_func_type)(subject_type *);

private:
	receiver_type *m_recv;			///< указателя на объект-приемник
	notify_func_type m_func;		///<функция оповещения
	tach_func_type m_attach_func;	///<событие подключения к наблюдаемому
	tach_func_type m_detach_func;	///<событие отключения от наблюдаемого

public:
	adv_adaptor() : m_recv(0), m_func(0), m_attach_func(0), m_detach_func(0) {}
/**
\brief конструктор

создает объект с указанным объктом-получателеми функциями-обработчиками сообщений.
	\param recv объект-получатель
	\param func функция события
	\param attach_func функция для обработки события подключения к наблюдаемому
	\param detach_func функция для обработки события отключения от наблюдаемого.
*/
	adv_adaptor(receiver_type *recv, notify_func_type func, 
			tach_func_type attach_func = 0, tach_func_type detach_func = 0)
	:	m_recv(recv),
		m_func(func),
		m_attach_func(attach_func),
		m_detach_func(detach_func)
	{}

	adv_adaptor(const adv_adaptor &rhs)
	:	m_recv(rhs.m_recv),
		m_func(rhs.m_func),
		m_attach_func(rhs.m_attach_func),
		m_detach_func(rhs.m_detach_func)
	{}

	adv_adaptor &operator=(const adv_adaptor &rhs)
	{
		this->m_recv = rhs.m_recv;
		this->m_func = rhs.m_func;
		this->m_attach_func = rhs.m_attach_func;
		this->m_detach_func = rhs.m_detach_func;
		return *this;
	}

	virtual void on_notify(const event_type &event)
	{
		(m_recv->*m_func)(event);
	}

	virtual void on_attach(subject_type *subj)
	{
		if(m_attach_func)
		{
			(m_recv->*m_attach_func)(subj);
		}
	}

	virtual void on_detach(subject_type *subj)
	{
		if(m_detach_func)
		{
			(m_recv->*m_detach_func)(subj);
		}
	}
};
/**
\brief получение сообщений в пределах области существования (scope) объекта

этот класс обеспечивает автоматическое 
подключение/отключение от наблюдаемого в пределах области видимости
*/
template<typename Observer>
class auto_attach
{
public:
	typedef Observer observer_type;
	typedef isubject<typename observer_type::event_type,observer_type> subject_type;

private:
	subject_type * const m_subject;
	mutable observer_type * m_observer;

public:

	auto_attach(subject_type *subj, observer_type *obs)
	:	m_subject(subj),
		m_observer(obs)
	{
		m_subject->attach(m_observer);
	}

	auto_attach(const auto_attach &t)
	:	m_subject(t.m_subject),
		m_observer(t.release())
	{
	}

	~auto_attach()
	{
		if(m_observer && m_subject->is_attached(m_observer))
		{
			m_subject->detach(m_observer);
		}
	}

	observer_type *get()
	{
		return m_observer;
	}

	observer_type *release()
	{
		observer_type *store = m_observer;
		return store;
	}

	void reset(observer_type *obs)
	{
		if(m_observer != obs)
		{
			this->~auto_attach();
			new(this) auto_attach(m_subject,obs);
		}
	}
};

///class single_subject : public isubject<E,O>
/**
	работает с одним наблюдателем
*/
template<typename E,
		 typename O = observer<E>,
		 typename T = typename O::traits_type>
class single_subject : public isubject<E,O>
{
protected:
	observer_type* m_observer;
	bool m_destructing;
public:
	typedef T observer_traits_type;

	single_subject() : m_destructing(false),m_observer(0) {}
	virtual ~single_subject()
	{
		m_destructing = true;
		if(m_observer) detach(m_observer);
	}

	//	установить наблюдатель
	virtual void attach(observer_type* o)
	{
		if(!m_destructing && m_observer!=o)
		{
			if(m_observer) detach(m_observer);
			m_observer = o;
			observer_traits_type::on_attach(o,this);
		}
	}
	//	удалить наблюдатель (o - неважно чему равно)
	virtual void detach(observer_type *o = 0)
	{
		observer_traits_type::on_detach(o,this);
		m_observer = 0;
	}
	//	уведомить 
	virtual void notify(const event_type &event)
	{
		if(m_observer) observer_traits_type::on_notify(event,m_observer,this);
	}

	virtual bool is_attached(observer_type* o) const
	{
		return m_observer == o;
	}
};

template<typename Event>
struct feedback_observer_traits;


template<typename E>
class feedback_observer
{
public:
	typedef E event_type;
	typedef base_subject<event_type> subject_type;
	typedef feedback_observer_traits<event_type> traits_type;

	virtual void on_notify(const event_type&,subject_type*) = 0;
};

template<typename Event>
struct feedback_observer_traits
{
	typedef Event event_type;
	typedef feedback_observer<Event> observer_type;
	typedef base_subject<event_type> subject_type;

	static void on_notify(const event_type &event, observer_type *o, subject_type *subj)
	{
		o->on_notify(event,subj); 
	}

	static void on_attach(observer_type *o, subject_type *subj) {}
	static void on_detach(observer_type *o, subject_type *subj) {}
};

//=====================================================================================//
//                               class feedback_adaptor                                //
//=====================================================================================//
template<typename R, typename E>
class feedback_adaptor : public feedback_observer<E>
{
public:
	typedef R receiver_type;
	typedef E event_type;
	typedef feedback_observer<E> observer_type;

	typedef void (R::*notify_func_type)(const event_type &,subject_type *);

private:
	receiver_type *m_recv;
	notify_func_type m_func;

public:
	feedback_adaptor() : m_recv(0), m_func(0) {}
	feedback_adaptor(receiver_type *recv, notify_func_type func)
	: m_recv(recv), m_func(func) {}
	feedback_adaptor(const feedback_adaptor &rhs) : m_recv(rhs.m_recv), m_func(rhs.m_func) {}

	feedback_adaptor &operator=(const feedback_adaptor &rhs)
	{
		this->m_recv = rhs.m_recv;
		this->m_func = rhs.m_func;
		return *this;
	}

	virtual void on_notify(const event_type& event,subject_type* subj)
	{
		(m_recv->*m_func)(event,subj);
	}
};

} //namespace patterns

} //namespace mll
#endif