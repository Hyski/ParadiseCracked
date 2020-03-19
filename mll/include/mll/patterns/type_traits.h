#if !defined(__TYPE_TRAITS_INCLUDED__)
#define __TYPE_TRAITS_INCLUDED__

#include <mll/patterns/_type_traits_hlp.h>

namespace mll
{
namespace patterns
{

//=====================================================================================//
//                                 struct is_integral                                  //
//=====================================================================================//
template<typename T> struct is_integral { enum { value = false }; };
template<> struct is_integral<unsigned char> { enum { value = true }; };
template<> struct is_integral<unsigned short> { enum { value = true }; };
template<> struct is_integral<unsigned int> { enum { value = true }; };
template<> struct is_integral<unsigned long> { enum { value = true }; };
template<> struct is_integral<signed char> { enum { value = true }; };
template<> struct is_integral<signed short> { enum { value = true }; };
template<> struct is_integral<signed int> { enum { value = true }; };
template<> struct is_integral<signed long> { enum { value = true }; };
template<> struct is_integral<char> { enum { value = true }; };
template<> struct is_integral<__wchar_t> { enum { value = true }; };
template<> struct is_integral<bool> { enum { value = true }; };
template<> struct is_integral<unsigned __int64> { enum { value = true }; };
template<> struct is_integral<__int64> { enum { value = true }; };

//=====================================================================================//
//                                      is_float                                       //
//=====================================================================================//
template<typename T> struct is_float { enum { value = false }; };
template<> struct is_float<float> { enum { value = true }; };
template<> struct is_float<double> { enum { value = true }; };
template<> struct is_float<long double> { enum { value = true }; };

//=====================================================================================//
//                                struct is_arithmetic                                 //
//=====================================================================================//
template<typename T> 
struct is_arithmetic
{ 
	enum
	{
		value = (
				_or
					<
					is_integral<T>::value,
					is_float<T>::value
					>::value
				)
	};
};

//=====================================================================================//
//                                   struct is_const                                   //
//=====================================================================================//
template<typename T>
struct is_const
{
private:
	static T t;

public:
	enum
	{
		value = (
				_eq
					<
					sizeof(_yes_type),
					sizeof(_is_const_helper(&t))
					>::value
				)
	};
};

template<> struct is_const<void> { enum {value = false}; };

//=====================================================================================//
//                                 struct is_volatile                                  //
//=====================================================================================//
template<typename T>
struct is_volatile
{
private:
	static T t;
	
public:
	enum
	{
		value = (
				_eq
					<
					sizeof(_yes_type),
					sizeof(_is_volatile_helper(&t))
					>::value
				)
	};
};

//=====================================================================================//
//                                 struct is_reference                                 //
//=====================================================================================//
template<typename T>
struct is_reference
{
    enum
	{
        value = (
				/*_eq
					<
					(int)sizeof(_is_reference_helper2(_is_reference_helper1(_is_ref_wrap<T>()))),
					(int)sizeof(_yes_type)
					>::value*/
				sizeof(_is_reference_helper2(_is_reference_helper1(_is_ref_wrap<T>()))) == sizeof(_yes_type)
				)
	};
};
    
template<> struct is_reference<void> { enum { value = false }; };

//=====================================================================================//
//                                   struct is_array                                   //
//=====================================================================================//
template<typename T>
struct is_array
{
private:
	static T t;

public:
	enum
	{
		value = (
				_and
					<
					_not<is_reference<T>::value>::value,
					_ne<sizeof(_yes_type),sizeof(_is_function_tester(t))>::value,
					_eq<sizeof(_yes_type),sizeof(_is_array_helper(&t,t))>::value
					>::value
				)
	};
};

template<> struct is_array<void> { enum { value = false }; };

template <class T>
struct _neither_array_nor_reference :	_not
											<
											_or
												<
												is_reference<T>::value,
												is_array<T>::value
												>::value
											>
{};

//=====================================================================================//
//                                  struct is_pointer                                  //
//=====================================================================================//
template<typename T>
struct is_pointer : _is_pointer_select	<
										_neither_array_nor_reference<T>::value
										>::template apply<T>
{};

template<> struct is_pointer<void> { enum { value = false }; };

//=====================================================================================//
//                                struct add_reference                                 //
//=====================================================================================//
template<typename T>
struct add_reference
{
private:
	typedef typename _reference_adder<is_reference<T>::value>::template rebind<T> binder;
public:
	typedef typename binder::type type;
};

//=====================================================================================//
//                                struct is_convertible                                //
//=====================================================================================//
template<class From, class To>
struct is_convertible
{
	typedef typename _conversion_helper<From>::type Selector;
	typedef typename Selector::template n_bind<From,To> Conversion;
public:
	enum { value = Conversion::exists };
};

struct _int_convertible
{
   _int_convertible(int);
};

//=====================================================================================//
//                                   struct is_enum                                    //
//=====================================================================================//
template <typename T>
struct is_enum
{
private:
	typedef typename add_reference<T>::type r_type;
public:
	enum
	{
		value = (
				_and
					<
					_not<is_arithmetic<T>::value>::value,
					_not<is_reference<T>::value>::value,
					is_convertible<r_type, _int_convertible>::value
					>::value
				)
	};
};

template<> struct is_enum<void> { enum { value = false }; };

} // namespace 'patterns'
} // namespace 'mll'

#endif