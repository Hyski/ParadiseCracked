#if !defined(__ANIMATION_H_INCLUDED_2903341426519653__)
#define __ANIMATION_H_INCLUDED_2903341426519653__

///todo:
/**
1. +один параметр при интстанцировании
2. +базовый интерполятор с типами traits
3. обдумать вариант (не)наследования key_track
4. +переделать интерполятор в функтор

*/
#include <vector>
#include <ostream>
#include <istream>
#include <mll/io/binstream.h>
#include <mll/algebra/quaternion.h>
#include <mll/algebra/point2.h>
#include <boost/utility.hpp>
#include <algorithm>

namespace mll
{
namespace utils
{


//=====================================================================================//
//                                   class _key_track                                   //
//=====================================================================================//
template<class T>
class _key_track
{

protected:
	typedef std::pair<float,T> _elem;

	struct Compare
	{
		bool operator()(const _elem &l, const _elem &r) const { return l.first < r.first; }
		bool operator()(float l, const _elem &r) const { return l < r.first; }
		bool operator()(const _elem &l, float r) const { return l.first < r; }
	};

	typedef std::vector<_elem> _cont;
	_cont m_keys;

public:
	typedef typename _cont::const_iterator const_iterator;//по ключам относительно текущего
	typedef typename _cont::iterator iterator;//по ключам относительно текущего

public:
	void reserve(unsigned int size_)
	{
		m_keys.reserve(size_);
	};
	void clear() { m_keys.clear(); }
	void set(float time, const T& val)
	{
		enum type{PUSHBACK,REPLASEBACK,FINDREPLACE_,FINDINSERT_};		
		
		type sost;
		_cont::iterator i;
		if (m_keys.begin()==m_keys.end())
		{
			sost=PUSHBACK;
		}
		else
		{
			if (m_keys.back().first<time)
			{
				 sost=PUSHBACK;
			}
			else
			{
			    if (m_keys.back().first==time)
				{
					sost=REPLASEBACK;
				}
				else
				{
				    i = std::lower_bound(m_keys.begin(),m_keys.end(),time,Compare());
					if(i == m_keys.end())
					{
					 sost=PUSHBACK;
					}
					else
					{
						if(i->first != time)
						{
							sost=FINDINSERT_;
						}
						else
						{
							sost=FINDREPLACE_;
						}
					
					}
					
				}
			}
		}

		//if((sost==PUSHBACK)||(sost==FINDINSERT_))
		//{
		//	if (m_keys.capacity()<=(m_keys.size()+1) )
		//	{
		//		m_keys.reserve(m_keys.size()+600);
		//	}

		//}

		switch(sost)
		{
		case PUSHBACK: m_keys.push_back(_elem(time,val));break;
		case REPLASEBACK: {	(m_keys.begin()+m_keys.size()-1)->second=val;break;	}
		case FINDREPLACE_: {	i->second = val;break;	}
		case FINDINSERT_: {	m_keys.insert(i,_elem(time,val));break;	}		
		default: assert("Такой ситуации  не должно быть");

		}
		
	}
	void push_back(float time, const T &value)
	{
		//if (m_keys.capacity()<=(m_keys.size()+1) )
		//{
		//	m_keys.reserve(m_keys.size()+600);
		//}

		assert( m_keys.empty()||time > m_keys.back().first || !"Неправильное использование mll::utils::_key_track::push_back()" );
		m_keys.push_back(_elem(time,value));
	}
	float length() const { return m_keys.size() ? m_keys.back().first - m_keys.front().first : 0.0f; }
	unsigned int size() const { return m_keys.size(); } ///< получить количество ключей
	void erase(iterator i) { m_keys.erase(i); }
	void erase(iterator first,iterator last) { m_keys.erase(first,last); }
	iterator begin() { return m_keys.begin(); }
	iterator end() { return m_keys.end(); }
	const_iterator begin() const { return m_keys.begin(); }
	const_iterator end() const { return m_keys.end(); }
	///	найти первый элемент время которого не меньше time
	iterator lower_bound(float time) { return std::lower_bound(m_keys.begin(),m_keys.end(),time,Compare());	}
	///	найти первый элемент время которого больше чем time
	iterator upper_bound(float time) { return std::upper_bound(m_keys.begin(),m_keys.end(),time,Compare());	}
	///	найти первый элемент время которого не меньше time
	const_iterator lower_bound(float time) const { return std::lower_bound(m_keys.begin(),m_keys.end(),time,Compare());	}
	///	найти первый элемент время которого больше чем time
	const_iterator upper_bound(float time) const { return std::upper_bound(m_keys.begin(),m_keys.end(),time,Compare());	}

};


//=====================================================================================//
//                        class animation: private _key_track<T>                        //
//=====================================================================================//
template<class I>
class animation	: public _key_track<typename I::value_type>
{
public: 

	typedef typename I::value_type value_type;

public:
	///
	value_type get(float time) const;
	value_type get2(float time, const_iterator &it) const;

public:
	///	записать анимацию в поток
	static std::ostream& _write_to_stream(std::ostream &o, const animation<I> &a);
	///	прочитать анимацию из поток
	static std::istream& _read_from_stream(std::istream &o, animation<I> &a);
	///	записать анимацию в бинарный поток
	static mll::io::obinstream& _write_to_binstream(mll::io::obinstream &o, const animation<I> &a);
	///	прочитать анимацию из бинарного поток
	static mll::io::ibinstream& _read_from_binstream(mll::io::ibinstream &o, animation<I> &a);

private:
	bool is_valid_cache(float time, const_iterator it) const;
};

//-+-+-+-+-+-+-+-+-+-+- НОВЫЙ КОД -+-+-+-+-+-+-+-+-+-+-

//=====================================================================================//
///< Этот класс по идее должен быть просто в mll
///< тип разности элементов нужен в разных местах
template<class T>
struct interpolator_traits
{
	typedef T difference_type; ///< тип разности
};
template<>
struct interpolator_traits<mll::algebra::point2>
{
	typedef mll::algebra::vector2 difference_type; ///< тип разности
};
template<>
struct interpolator_traits<mll::algebra::point3>
{
	typedef mll::algebra::vector3 difference_type; ///< тип разности
};

//=====================================================================================//
//! базовый класс для функтора который вычисляет промежуточное значение между ключами
/**
от него необходимо наследоваться и реализовывать operator()
*/
template<class T>
class base_interpolator
{
public:
	typedef T value_type;														///< тип значений, хранящихся в анимации
	typedef typename _key_track<T>::const_iterator iter;						///< тип итератора по ключам анимации
	typedef typename interpolator_traits<T>::difference_type difference_type;	///< тип разности
public:
	//! пример реализации - дискретная интерполяция
	T operator()(float time, const iter &it_before, const iter &it_after, const iter &beg, const iter &end) const
	{
		 return it_before->second;
	}
    T err_value() const {return T();};

};

//=====================================================================================//
// Линейная интерполяция
//=====================================================================================//
template<class T>
struct l_interpolator: public base_interpolator<T>
{
	T operator()(float time, const iter &it_before, const iter &it_after, const iter &beg, const iter &end) const
	{
		if(it_before==it_after) return it_before->second;

		assert( (it_after->first-it_before->first) > 0 );

		float alpha = (time-it_before->first)/(it_after->first-it_before->first);
		return it_before->second + (it_after->second-it_before->second)*alpha;
	}
};

template<>
struct l_interpolator<mll::algebra::quaternion>: public base_interpolator<mll::algebra::quaternion>
{
	typedef mll::algebra::quaternion val;
	mll::algebra::quaternion operator()(float time, const iter &it_before, const iter &it_after, const iter &beg, const iter &end) const
	{
		if(it_before==it_after) return it_before->second;

		float alpha = (time-it_before->first)/(it_after->first-it_before->first);
		return it_before->second.slerp(it_after->second,alpha);
	}
};

//=====================================================================================//
// Кубическая интерполяция
//=====================================================================================//
template<class T>
struct q_interpolator: public base_interpolator<T>
	{
	 public:
		 T operator()(float time, const iter &a, const iter &b, const iter &beg, const iter &end) const
			 {
			 if(a==b) return b->second;
			 float t=(time-a->first)/(b->first-a->first);
			 const value_type &p0 = a->second;
			 const value_type &p1 = b->second;
			 difference_type n0,n1,dif(p1-p0);
			 iter it;
			 if(a==beg)
				 {
				 n0=dif;
				 }
			 else
				 {
				 it = a; it--;
				 n0 = p1-it->second;
				 }
			 it=b; it++;
			 
			 if(it==end)
				 {
				 n1=dif;
				 }
			 else 
				 {
				 n1=it->second-p0;
				 }

			 float t2=t*t,t3=t2*t;

			 return  p0+(t-2*t2+t3)*n0+(t3-t2)*n1+(3*t2-2*t3)*dif;
			 }
	};
template<>
struct q_interpolator<mll::algebra::quaternion>: public l_interpolator<mll::algebra::quaternion>
{
};

//=====================================================================================//
//                           inline T animation<T,I>::get()                            //
//=====================================================================================//
template<class I>
inline typename I::value_type animation<I>::get(float time) const
{
	const_iterator it = end();
	return get2(time,it);
}

//=====================================================================================//
//                      bool animation<I>::is_valid_cache() const                      //
//=====================================================================================//
template<typename I>
bool animation<I>::is_valid_cache(float time, const_iterator it) const
{
	if(it == m_keys.end())
	{
		return boost::prior(it)->first <= time;
	}
	else
	{
		return it->first > time && (it == begin() || boost::prior(it)->first <= time);
	}
}

//=====================================================================================//
//              inline typename I::value_type animation<I>::get2() const               //
//=====================================================================================//
template<class I>
inline typename I::value_type animation<I>::get2(float time, const_iterator &it2) const
{
	if(!m_keys.size())
	{
		return I().err_value();
	}

	if(!is_valid_cache(time,it2))
	{
		if(it2 != end() && it2->first <= time)
		{
			if(!is_valid_cache(time,++it2))
			{
				it2 = std::upper_bound(m_keys.begin(),m_keys.end(),time,Compare());
			}
		}
		else
		{
			it2 = std::upper_bound(m_keys.begin(),m_keys.end(),time,Compare());
		}
	}

	const_iterator it1;

	if(it2 == m_keys.end())
	{
		it2 = it1 = boost::prior(m_keys.end());
	}
	else
	{
		it1 = it2;
		if(it2 != m_keys.begin())
		{
			--it1;
		}
	}
	return I()(time, it1, it2, m_keys.begin(), m_keys.end());
}

//=====================================================================================//
//                inline std::ostream& animation<I>::_write_to_stream()                //
//=====================================================================================//
template<class I>
inline std::ostream& animation<I>::_write_to_stream(std::ostream &o, const animation<I> &a)
{
	animation<I>::_cont::const_iterator b,e;

	o<<"animation{"<<std::endl;
	o<<"keynum:"<<a.m_keys.size()<<std::endl;
	b=a.m_keys.begin();
	e=a.m_keys.end();
	for(;b!=e;b++)
	{
		o<<"("<<b->first<<"->"<<b->second<<")"<<std::endl;
	}
	o<<"}"<<std::endl;
	return o;
}

//=====================================================================================//
//               inline std::ostream& animation<I>::_read_from_stream()                //
//=====================================================================================//
template<class I>
inline std::istream& animation<I>::_read_from_stream(std::istream &o, animation<I> &a)
{
	int i;
	o >> std::ws >> io::verify("animation{");
	o >> std::ws >> io::verify("keynum:") >> i;
	while(i--)
	{
		float time;
		I::value_type  val;

		o >> std::ws >> io::verify("(") >> time;
		o >> std::ws >> io::verify("->") >> val >> std::ws >> io::verify(")");
		a.set(time,val);
	}
	o >> std::ws >> io::verify("}");
	return o;
}

//=====================================================================================//
//              inline std::ostream& animation<I>::_write_to_binstream()               //
//=====================================================================================//
template<class I>
inline mll::io::obinstream& animation<I>::_write_to_binstream(mll::io::obinstream &o, const animation<I> &a)
{
	animation<I>::_cont::const_iterator b,e;

	o<<a.m_keys.size();
	b=a.m_keys.begin();
	e=a.m_keys.end();
	for(;b!=e;b++)
	{
		o<<b->first<<b->second;
	}		


	return o;
}

//=====================================================================================//
//              inline std::ostream& animation<I>::_read_from_binstream()              //
//=====================================================================================//
template<class I>
inline mll::io::ibinstream& animation<I>::_read_from_binstream(mll::io::ibinstream &o, animation<I> &a)
{
	int i;
	o>>i;
	while(i--)
	{
		float time;
		I::value_type val;
		o>>time>>val;
		a.set(time,val);
	}		 
	return o;
}


//=====================================================================================//
//                          inline std::ostream& operator<<()                          //
//=====================================================================================//
template<class I>
inline std::ostream& operator<<(std::ostream &o, const animation<I> &a)
{
	return animation<I>::_write_to_stream(o,a);
}

template<class I>
inline std::istream& operator>>(std::istream &o, animation<I> &a)
{
	return animation<I>::_read_from_stream(o,a);
}

template<class I>
inline mll::io::obinstream& operator<<(mll::io::obinstream &o, const animation<I> &a)
{
	return animation<I>::_write_to_binstream(o,a);
}

template<class I>
inline mll::io::ibinstream& operator>>(mll::io::ibinstream &o, animation<I> &a)
{
	return animation<I>::_read_from_binstream(o,a);
}

}	//	namespace utils
}	//	namespace mll

#endif // !defined(__ANIMATION_H_INCLUDED_2903341426519653__)