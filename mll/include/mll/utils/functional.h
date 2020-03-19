#ifndef __FUNCTIONAL_H_INCLUDED_0686713605376210__
#define __FUNCTIONAL_H_INCLUDED_0686713605376210__

namespace mll
{

namespace utils
{

//=====================================================================================//
//                                   struct deleter                                    //
//=====================================================================================//
//	предназначен для безопасного удаления указателей из контейнеров
template<typename T>
struct deleter : public std::unary_function<T*,T*>
{
	T* operator()(T* ptr) const
	{
		delete ptr;
		return 0;
	}
};

}

}

#endif // __FUNCTIONAL_H_INCLUDED_0686713605376210__