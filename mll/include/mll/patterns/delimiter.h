#if !defined(__DELIMITER_H_INCLUDED_2903341426519653__)
#define __DELIMITER_H_INCLUDED_2903341426519653__

namespace mll
{

namespace patterns
{

//=====================================================================================//
//                           struct _container_const_helper                            //
//=====================================================================================//
template<typename C>
struct _container_const_helper
{
	template<int IsConst>
	struct holder
	{
		typedef C container_type;
		typedef const container_type &container_ref_type;
		typedef typename C::const_iterator iterator;
		typedef typename C::const_reference reference;
	};

	template<>
	struct holder<0>
	{
		typedef C container_type;
		typedef container_type &container_ref_type;
		typedef typename C::iterator iterator;
		typedef typename C::reference reference;
	};
};

template<typename C, int IsConst>
struct _cch // container_const_helper
{
	typedef typename _container_const_helper<C>::holder<IsConst> _hlp;
	typedef typename _hlp::container_type container_type;
	typedef typename _hlp::container_ref_type container_ref_type;
	typedef typename _hlp::iterator iterator;
	typedef typename _hlp::reference reference;
};

//=====================================================================================//
//                                     struct _sct                                     //
//=====================================================================================//
template<typename CH>
struct _sct	// stl_container_typedefs
{
	typedef typename CH::container_type container_type;
	typedef typename CH::container_ref_type container_ref_type;

	typedef typename CH::iterator iterator;
	typedef typename CH::reference reference;
	typedef typename container_type::value_type value_type;
	typedef typename container_type::pointer pointer;
	typedef typename container_type::difference_type difference_type;
	typedef typename container_type::size_type size_type;
};

//=====================================================================================//
//                          class _sequence_helper : public T                          //
//=====================================================================================//
template<typename T>
class _sequence_helper : public T
{
	typename T::container_ref_type m_cont;

public:
	_sequence_helper(typename T::container_ref_type cont) : m_cont(cont) {}

	typename T::iterator begin() const { return m_cont.begin(); }
	typename T::iterator end() const { return m_cont.end(); }

	typename T::size_type size() const { return m_cont.size(); }
};

//=====================================================================================//
//              class _random_access_helper : public _sequence_helper<T>               //
//=====================================================================================//
template<typename T>
class _random_access_helper : public _sequence_helper<T>
{
public:
	_random_access_helper(typename T::container_ref_type cont) : _sequence_helper<T>(cont) {}

	typename T::reference operator[](typename T::size_type n) const { return m_cont[n]; }
};

//=====================================================================================//
//                                   class sequence                                    //
//=====================================================================================//
template<typename C>
class sequence : public _sequence_helper< _sct< _cch<C,0> > >
{
	typedef _sequence_helper< _sct< _cch<C,0> > > base;

public:
	sequence(typename base::container_ref_type cont) : base(cont) {}
};

//=====================================================================================//
//                                class const_sequence                                 //
//=====================================================================================//
template<typename C>
class const_sequence : public _sequence_helper< _sct< _cch<C,1> > >
{
	typedef _sequence_helper< _sct< _cch<C,1> > > base;

public:
	const_sequence(typename base::container_ref_type cont) : base(cont) {}
};

}	//	namespace patterns

}	//	namespace mll

#endif // !defined(__DELIMITER_H_INCLUDED_2903341426519653__)