#if !defined(___TYPE_TRAITS_HLP_H_INCLUDED_7552370787048739__)
#define ___TYPE_TRAITS_HLP_H_INCLUDED_7552370787048739__

namespace mll
{
namespace patterns
{

template<bool b> struct _not { enum { value = true }; };
template<>struct _not<true> { enum { value = false }; };

template<bool b1, bool b2, bool b3 = false, bool b4 = false, bool b5 = false, bool b6 = false, bool b7 = false>
struct _or;
template<bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7>
struct _or
{
	enum { value = true };
};
template<>
struct _or<false, false, false, false, false, false, false>
{
	enum { value = false };
};

template<bool b1, bool b2, bool b3 = true, bool b4 = true, bool b5 = true, bool b6 = true, bool b7 = true>
struct _and;
template<bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7>
struct _and
{
	enum { value = false };
};
template<>
struct _and<true, true, true, true, true, true, true>
{
	enum { value = true };
};

template<int b1, int b2>
struct _eq
{
	enum { value = (b1 == b2) };
};

template<int b1, int b2>
struct _ne
{
	enum { value = (b1 != b2) };
};

typedef char _yes_type;
typedef double _no_type;

struct _pointer_helper
{
	_pointer_helper(const volatile void *);
};

_yes_type _is_pointer_helper(_pointer_helper);
_no_type _is_pointer_helper(...);

_yes_type _is_array_helper(const volatile void*, const volatile void*);
template <class T>
_no_type _is_array_helper(T*const volatile*, const volatile void*);
_no_type _is_array_helper(...);

template <class R>
_yes_type _is_function_tester(R (*)(void));
template <class R, class A0>
_yes_type _is_function_tester(R (*)(A0));
template <class R, class A0, class A1>
_yes_type _is_function_tester(R (*)(A0, A1));
template <class R, class A0, class A1, class A2>
_yes_type _is_function_tester(R (*)(A0, A1, A2));
template <class R, class A0, class A1, class A2, class A3>
_yes_type _is_function_tester(R (*)(A0, A1, A2, A3));
template <class R, class A0, class A1, class A2, class A3, class A4>
_yes_type _is_function_tester(R (*)(A0, A1, A2, A3, A4));
template <class R, class A0, class A1, class A2, class A3, class A4, class A5>
_yes_type _is_function_tester(R (*)(A0, A1, A2, A3, A4, A5));
template <class R, class A0, class A1, class A2, class A3, class A4, class A5, class A6>
_yes_type _is_function_tester(R (*)(A0, A1, A2, A3, A4, A5, A6));
_no_type _is_function_tester(...);

_yes_type _is_const_helper(const volatile void*);
_no_type _is_const_helper(volatile void *);
_yes_type _is_volatile_helper(const volatile void*);
_no_type _is_volatile_helper(const void *);

template<typename T> struct is_void { enum { value = false }; };
template<> struct is_void<void> { enum { value = true }; };

template<class T> struct _is_ref_wrap {};

template<class T> T&(* _is_reference_helper1(_is_ref_wrap<T>) )(_is_ref_wrap<T>);
_yes_type _is_reference_helper1(...);

template<class T> _no_type _is_reference_helper2(T&(*)(_is_ref_wrap<T>));
_yes_type _is_reference_helper2(...);

struct _false_unary_metafunction
{
	template<typename T> struct apply {enum { value = false };};
};

template<bool maybe = false>
struct _is_pointer_select : ::mll::patterns::_false_unary_metafunction
{
};

template<>
struct _is_pointer_select<true>
{
	template <class T>
	struct apply
	{
		static T& make_t();
		enum
		{
			value =	(
					_or
						<
						_eq<sizeof(_yes_type),sizeof(_is_pointer_helper(make_t()))>::value,
						_eq<sizeof(_yes_type),sizeof(_is_function_tester(make_t()))>::value
						>::value
					)
		};
	};
};

template<bool x>
struct _reference_adder
{
	template <class T>
	struct rebind
	{
		typedef T& type;
	};
};

template <>
struct _reference_adder<true>
{
	template <class T>
	struct rebind
	{
		typedef T type;
	};
};

#pragma warning(push)
#pragma warning(disable:4244)
struct _from_not_void_conversion
{
	template <class From, class To>
	struct n_bind
	{
	private:
		static _no_type _m_check(...);
		static _yes_type _m_check(To);
	public:
		static From _m_from;
		enum { exists = sizeof( _m_check(_m_from) ) == sizeof(_yes_type) };
	};
};
#pragma warning(pop)

struct _from_is_void_conversion
{
	template <class From, class To>
	struct n_bind
	{
		enum { exists = is_void<To>::value };
	};
};

template <class From>
struct _conversion_helper
{
	typedef _from_not_void_conversion type;
};

template<>
struct _conversion_helper<void>
{
	typedef _from_is_void_conversion type;
};

} // namespace 'patterns'
} // namespace 'mll'

#endif // !defined(___TYPE_TRAITS_HLP_H_INCLUDED_7552370787048739__)