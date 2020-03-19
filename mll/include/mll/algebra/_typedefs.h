#if !defined(__TYPEDEFS_H_INCLUDED__)
#define __TYPEDEFS_H_INCLUDED__

#include <mll/_choose_lib.h>
#include <mll/_export_rules.h>

namespace mll
{

namespace algebra
{
	class base_identity {};
	class base_origin {};
	class base_null_vector {};

	const base_identity identity;
	const base_origin origin;
	const base_null_vector null_vector;


	
	// MLL_EXPORT extern base_identity identity;

//	MLL_EXPORT extern base_origin origin;

//	class base_null_vector {};
//	MLL_EXPORT extern base_null_vector null_vector;
}

}

#endif
