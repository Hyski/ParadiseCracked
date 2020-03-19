#if !defined(__STATIC_ASSERTION_INCLUDED__)
#define __STATIC_ASSERTION_INCLUDED__

namespace mll
{
namespace debug
{

template<bool> struct _static_assertion_failure;
template<> struct _static_assertion_failure<true> {};

template<int> struct _static_assertion_test {};

#define MLL_STATIC_ASSERT(X)													\
		typedef ::mll::debug::_static_assertion_test							\
			<sizeof( ::mll::debug::_static_assertion_failure< (bool)(X) > ) >	\
			_static_assertion_typedef_

} // namespace 'debug'
} // namespace 'mll'

#endif
